/*
 * A rewrite of Darren Reeds original res.c As there is nothing
 * left of Darrens original code, this is now licensed by the hybrid group.
 * (Well, some of the function names are the same, and bits of the structs..)
 * You can use it where it is useful, free even. Buy us a beer and stuff.
 *
 * The authors takes no responsibility for any damage or loss
 * of property which results from the use of this software.
 *
 *
 * July 1999 - Rewrote a bunch of stuff here. Change hostent builder code,
 *     added callbacks and reference counting of returned hostents.
 *     --Bleep (Thomas Helvey <tomh@inxpress.net>)
 *
 * This was all needlessly complicated for irc. Simplified. No more hostent
 * All we really care about is the IP -> hostname mappings. Thats all.
 *
 * Apr 28, 2003 --cryogen and Dianora
 *
 * DNS server flooding lessened, AAAA-or-A lookup removed, ip6.int support
 * removed, various robustness fixes
 *
 * 2006 --jilles and nenolod
 */

#include "stdinc.h"
#include "ircd_defs.h"
#include "common.h"
#include "ircd.h"
#include "res.h"
#include "reslib.h"
#include "match.h"
#include "numeric.h"
#include "client.h" /* SNO_* */

#if (CHAR_BIT != 8)
#error this code needs to be able to address individual octets
#endif

static PF res_readreply;

#define MAXPACKET      1024	/* rfc sez 512 but we expand names so ... */
#define RES_MAXALIASES 35	/* maximum aliases allowed */
#define RES_MAXADDRS   35	/* maximum addresses allowed */
#define AR_TTL         600	/* TTL in seconds for dns cache entries */

/* RFC 1104/1105 wasn't very helpful about what these fields
 * should be named, so for now, we'll just name them this way.
 * we probably should look at what named calls them or something.
 */
#define TYPE_SIZE         (size_t)2
#define CLASS_SIZE        (size_t)2
#define TTL_SIZE          (size_t)4
#define RDLENGTH_SIZE     (size_t)2
#define ANSWER_FIXED_SIZE (TYPE_SIZE + CLASS_SIZE + TTL_SIZE + RDLENGTH_SIZE)

struct reslist {
    rb_dlink_node node;
    int id;
    time_t ttl;
    char type;
    char queryname[IRCD_RES_HOSTLEN + 1]; /* name currently being queried */
    char retries;		/* retry counter */
    char sends;		/* number of sends (>1 means resent) */
    time_t sentat;
    time_t timeout;
    unsigned int lastns;	/* index of last server sent to */
    struct rb_sockaddr_storage addr;
    char *name;
    struct DNSQuery *query;	/* query callback for this request */
};

static rb_fde_t *res_fd;
static rb_dlink_list request_list = { NULL, NULL, 0 };
static int ns_timeout_count[IRCD_MAXNS];

static void rem_request(struct reslist *request);
static struct reslist *make_request(struct DNSQuery *query);
static void do_query_name(struct DNSQuery *query, const char *name, struct reslist *request, int);
static void do_query_number(struct DNSQuery *query, const struct rb_sockaddr_storage *,
                            struct reslist *request);
static void query_name(struct reslist *request);
static int send_res_msg(const char *buf, int len, int count);
static void resend_query(struct reslist *request);
static int check_question(struct reslist *request, HEADER * header, char *buf, char *eob);
static int proc_answer(struct reslist *request, HEADER * header, char *, char *);
static struct reslist *find_id(int id);
static struct DNSReply *make_dnsreply(struct reslist *request);

/*
 * int
 * res_ourserver(inp)
 *      looks up "inp" in irc_nsaddr_list[]
 * returns:
 *      0  : not found
 *      >0 : found
 * author:
 *      paul vixie, 29may94
 *      revised for ircd, cryogen(stu) may03
 */
static int res_ourserver(const struct rb_sockaddr_storage *inp)
{
#ifdef RB_IPV6
    const struct sockaddr_in6 *v6;
    const struct sockaddr_in6 *v6in = (const struct sockaddr_in6 *)inp;
#endif
    const struct sockaddr_in *v4;
    const struct sockaddr_in *v4in = (const struct sockaddr_in *)inp;
    int ns;

    for (ns = 0; ns < irc_nscount; ns++) {
        const struct rb_sockaddr_storage *srv = &irc_nsaddr_list[ns];
#ifdef RB_IPV6
        v6 = (const struct sockaddr_in6 *)srv;
#endif
        v4 = (const struct sockaddr_in *)srv;

        /* could probably just memcmp(srv, inp, srv.ss_len) here
         * but we'll air on the side of caution - stu
         */
        switch (srv->ss_family) {
#ifdef RB_IPV6
        case AF_INET6:
            if (srv->ss_family == inp->ss_family)
                if (v6->sin6_port == v6in->sin6_port)
                    if ((memcmp(&v6->sin6_addr.s6_addr, &v6in->sin6_addr.s6_addr,
                                sizeof(struct in6_addr)) == 0) ||
                        (memcmp(&v6->sin6_addr.s6_addr, &in6addr_any,
                                sizeof(struct in6_addr)) == 0)) {
                        ns_timeout_count[ns] = 0;
                        return 1;
                    }
            break;
#endif
        case AF_INET:
            if (srv->ss_family == inp->ss_family)
                if (v4->sin_port == v4in->sin_port)
                    if ((v4->sin_addr.s_addr == INADDR_ANY)
                        || (v4->sin_addr.s_addr == v4in->sin_addr.s_addr)) {
                        ns_timeout_count[ns] = 0;
                        return 1;
                    }
            break;
        default:
            break;
        }
    }

    return 0;
}

/*
 * timeout_query_list - Remove queries from the list which have been
 * there too long without being resolved.
 */
static time_t timeout_query_list(time_t now)
{
    rb_dlink_node *ptr;
    rb_dlink_node *next_ptr;
    struct reslist *request;
    time_t next_time = 0;
    time_t timeout = 0;

    RB_DLINK_FOREACH_SAFE(ptr, next_ptr, request_list.head) {
        request = ptr->data;
        timeout = request->sentat + request->timeout;

        if (now >= timeout) {
            if (--request->retries <= 0) {
                (*request->query->callback) (request->query->ptr, NULL);
                rem_request(request);
                continue;
            } else {
                ns_timeout_count[request->lastns]++;
                request->sentat = now;
                request->timeout += request->timeout;
                resend_query(request);
            }
        }

        if ((next_time == 0) || timeout < next_time) {
            next_time = timeout;
        }
    }

    return (next_time > now) ? next_time : (now + AR_TTL);
}

/*
 * timeout_resolver - check request list
 */
static void timeout_resolver(void *notused)
{
    timeout_query_list(rb_current_time());
}

static struct ev_entry *timeout_resolver_ev = NULL;

/*
 * start_resolver - do everything we need to read the resolv.conf file
 * and initialize the resolver file descriptor if needed
 */
static void start_resolver(void)
{
    int i;

    irc_res_init();
    for (i = 0; i < irc_nscount; i++)
        ns_timeout_count[i] = 0;

    if (res_fd == NULL) {
        if ((res_fd = rb_socket(irc_nsaddr_list[0].ss_family, SOCK_DGRAM, 0,
                                "UDP resolver socket")) == NULL)
            return;

        /* At the moment, the resolver FD data is global .. */
        rb_setselect(res_fd, RB_SELECT_READ, res_readreply, NULL);
        timeout_resolver_ev = rb_event_add("timeout_resolver", timeout_resolver, NULL, 1);
    }
}

/*
 * init_resolver - initialize resolver and resolver library
 */
void init_resolver(void)
{
#ifdef HAVE_SRAND48
    srand48(rb_current_time());
#endif
    start_resolver();
}

/*
 * restart_resolver - reread resolv.conf, reopen socket
 */
void restart_resolver(void)
{
    rb_close(res_fd);
    res_fd = NULL;
    rb_event_delete(timeout_resolver_ev);	/* -ddosen */
    start_resolver();
}

/*
 * add_local_domain - Add the domain to hostname, if it is missing
 * (as suggested by eps@TOASTER.SFSU.EDU)
 */
void add_local_domain(char *hname, size_t size)
{
    /* try to fix up unqualified names */
    if (strchr(hname, '.') == NULL) {
        if (irc_domain[0]) {
            size_t len = strlen(hname);

            if ((strlen(irc_domain) + len + 2) < size) {
                hname[len++] = '.';
                strcpy(hname + len, irc_domain);
            }
        }
    }
}

/*
 * rem_request - remove a request from the list.
 * This must also free any memory that has been allocated for
 * temporary storage of DNS results.
 */
static void rem_request(struct reslist *request)
{
    rb_dlinkDelete(&request->node, &request_list);
    rb_free(request->name);
    rb_free(request);
}

/*
 * make_request - Create a DNS request record for the server.
 */
static struct reslist *make_request(struct DNSQuery *query)
{
    struct reslist *request = rb_malloc(sizeof(struct reslist));

    request->sentat = rb_current_time();
    request->retries = 3;
    request->timeout = 4;	/* start at 4 and exponential inc. */
    request->query = query;

    rb_dlinkAdd(request, &request->node, &request_list);

    return request;
}

/*
 * delete_resolver_queries - cleanup outstanding queries
 * for which there no longer exist clients or conf lines.
 */
void delete_resolver_queries(const struct DNSQuery *query)
{
    rb_dlink_node *ptr;
    rb_dlink_node *next_ptr;
    struct reslist *request;

    RB_DLINK_FOREACH_SAFE(ptr, next_ptr, request_list.head) {
        if ((request = ptr->data) != NULL) {
            if (query == request->query)
                rem_request(request);
        }
    }
}

/*
 * retryfreq - determine how many queries to wait before resending
 * if there have been that many consecutive timeouts
 */
static int retryfreq(int timeouts)
{
    switch (timeouts) {
    case 1:
        return 3;
    case 2:
        return 9;
    case 3:
        return 27;
    case 4:
        return 81;
    default:
        return 243;
    }
}

/*
 * send_res_msg - sends msg to a nameserver.
 * This should reflect /etc/resolv.conf.
 * Returns number of nameserver successfully sent to
 * or -1 if no successful sends.
 */
static int send_res_msg(const char *msg, int len, int rcount)
{
    int i;
    int ns;
    static int retrycnt;

    retrycnt++;
    /* First try a nameserver that seems to work.
     * Every once in a while, try a possibly broken one to check
     * if it is working again.
     */
    for (i = 0; i < irc_nscount; i++) {
        ns = (i + rcount - 1) % irc_nscount;
        if (ns_timeout_count[ns] && retrycnt % retryfreq(ns_timeout_count[ns]))
            continue;
        if (sendto(rb_get_fd(res_fd), msg, len, 0,
                   (struct sockaddr *)&(irc_nsaddr_list[ns]),
                   GET_SS_LEN(&irc_nsaddr_list[ns])) == len)
            return ns;
    }

    /* No known working nameservers, try some broken one. */
    for (i = 0; i < irc_nscount; i++) {
        ns = (i + rcount - 1) % irc_nscount;
        if (!ns_timeout_count[ns])
            continue;
        if (sendto(rb_get_fd(res_fd), msg, len, 0,
                   (struct sockaddr *)&(irc_nsaddr_list[ns]),
                   GET_SS_LEN(&irc_nsaddr_list[ns])) == len)
            return ns;
    }

    return -1;
}

/*
 * find_id - find a dns request id (id is determined by dn_mkquery)
 */
static struct reslist *find_id(int id)
{
    rb_dlink_node *ptr;
    struct reslist *request;

    RB_DLINK_FOREACH(ptr, request_list.head) {
        request = ptr->data;

        if (request->id == id)
            return (request);
    }

    return (NULL);
}

/*
 * gethost_byname_type - get host address from name
 *
 */
void gethost_byname_type(const char *name, struct DNSQuery *query, int type)
{
    assert(name != 0);
    do_query_name(query, name, NULL, type);
}

/*
 * gethost_byaddr - get host name from address
 */
void gethost_byaddr(const struct rb_sockaddr_storage *addr, struct DNSQuery *query)
{
    do_query_number(query, addr, NULL);
}

/*
 * do_query_name - nameserver lookup name
 */
static void do_query_name(struct DNSQuery *query, const char *name, struct reslist *request,
                          int type)
{
    char host_name[IRCD_RES_HOSTLEN + 1];

    rb_strlcpy(host_name, name, IRCD_RES_HOSTLEN + 1);
    add_local_domain(host_name, IRCD_RES_HOSTLEN);

    if (request == NULL) {
        request = make_request(query);
        request->name = (char *)rb_malloc(strlen(host_name) + 1);
        strcpy(request->name, host_name);
    }

    rb_strlcpy(request->queryname, host_name, sizeof(request->queryname));
    request->type = type;
    query_name(request);
}

/*
 * do_query_number - Use this to do reverse IP# lookups.
 */
static void do_query_number(struct DNSQuery *query, const struct rb_sockaddr_storage *addr,
                            struct reslist *request)
{
    const unsigned char *cp;

    if (request == NULL) {
        request = make_request(query);
        memcpy(&request->addr, addr, sizeof(struct rb_sockaddr_storage));
        request->name = (char *)rb_malloc(IRCD_RES_HOSTLEN + 1);
    }

    if (addr->ss_family == AF_INET) {
        const struct sockaddr_in *v4 = (const struct sockaddr_in *)addr;
        cp = (const unsigned char *)&v4->sin_addr.s_addr;

        sprintf(request->queryname, "%u.%u.%u.%u.in-addr.arpa", (unsigned int)(cp[3]),
                   (unsigned int)(cp[2]), (unsigned int)(cp[1]), (unsigned int)(cp[0]));
    }
#ifdef RB_IPV6
    else if (addr->ss_family == AF_INET6) {
        const struct sockaddr_in6 *v6 = (const struct sockaddr_in6 *)addr;
        cp = (const unsigned char *)&v6->sin6_addr.s6_addr;

        (void)sprintf(request->queryname, "%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x."
                      "%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.%x.ip6.arpa",
                      (unsigned int)(cp[15] & 0xf), (unsigned int)(cp[15] >> 4),
                      (unsigned int)(cp[14] & 0xf), (unsigned int)(cp[14] >> 4),
                      (unsigned int)(cp[13] & 0xf), (unsigned int)(cp[13] >> 4),
                      (unsigned int)(cp[12] & 0xf), (unsigned int)(cp[12] >> 4),
                      (unsigned int)(cp[11] & 0xf), (unsigned int)(cp[11] >> 4),
                      (unsigned int)(cp[10] & 0xf), (unsigned int)(cp[10] >> 4),
                      (unsigned int)(cp[9] & 0xf), (unsigned int)(cp[9] >> 4),
                      (unsigned int)(cp[8] & 0xf), (unsigned int)(cp[8] >> 4),
                      (unsigned int)(cp[7] & 0xf), (unsigned int)(cp[7] >> 4),
                      (unsigned int)(cp[6] & 0xf), (unsigned int)(cp[6] >> 4),
                      (unsigned int)(cp[5] & 0xf), (unsigned int)(cp[5] >> 4),
                      (unsigned int)(cp[4] & 0xf), (unsigned int)(cp[4] >> 4),
                      (unsigned int)(cp[3] & 0xf), (unsigned int)(cp[3] >> 4),
                      (unsigned int)(cp[2] & 0xf), (unsigned int)(cp[2] >> 4),
                      (unsigned int)(cp[1] & 0xf), (unsigned int)(cp[1] >> 4),
                      (unsigned int)(cp[0] & 0xf), (unsigned int)(cp[0] >> 4));
    }
#endif

    request->type = T_PTR;
    query_name(request);
}

/*
 * query_name - generate a query based on class, type and name.
 */
static void query_name(struct reslist *request)
{
    char buf[MAXPACKET];
    int request_len = 0;
    int ns;

    memset(buf, 0, sizeof(buf));

    if ((request_len =
             irc_res_mkquery(request->queryname, C_IN, request->type, (unsigned char *)buf, sizeof(buf))) > 0) {
        HEADER *header = (HEADER *) buf;
#ifndef HAVE_LRAND48
        int k = 0;
        struct timeval tv;
#endif
        /*
         * generate an unique id
         * NOTE: we don't have to worry about converting this to and from
         * network byte order, the nameserver does not interpret this value
         * and returns it unchanged
         */
#ifdef HAVE_LRAND48
        do {
            header->id = (header->id + lrand48()) & 0xffff;
        } while (find_id(header->id));
#else
        gettimeofday(&tv, NULL);
        do {
            header->id = (header->id + k + tv.tv_usec) & 0xffff;
            k++;
        } while (find_id(header->id));
#endif /* HAVE_LRAND48 */
        request->id = header->id;
        ++request->sends;

        ns = send_res_msg(buf, request_len, request->sends);
        if (ns != -1)
            request->lastns = ns;
    }
}

static void resend_query(struct reslist *request)
{
    switch (request->type) {
    case T_PTR:
        do_query_number(NULL, &request->addr, request);
        break;
    case T_A:
#ifdef RB_IPV6
    case T_AAAA:
#endif
        do_query_name(NULL, request->name, request, request->type);
        break;
    default:
        break;
    }
}

/*
 * check_question - check if the reply really belongs to the
 * name we queried (to guard against late replies from previous
 * queries with the same id).
 */
static int check_question(struct reslist *request, HEADER * header, char *buf, char *eob)
{
    char hostbuf[IRCD_RES_HOSTLEN + 1];	/* working buffer */
    unsigned char *current;	/* current position in buf */
    int n;			/* temp count */

    current = (unsigned char *)buf + sizeof(HEADER);
    if (header->qdcount != 1)
        return 0;
    n = irc_dn_expand((unsigned char *)buf, (unsigned char *)eob, current, hostbuf,
                      sizeof(hostbuf));
    if (n <= 0)
        return 0;
    if (strcasecmp(hostbuf, request->queryname))
        return 0;
    return 1;
}

/*
 * proc_answer - process name server reply
 */
static int proc_answer(struct reslist *request, HEADER * header, char *buf, char *eob)
{
    char hostbuf[IRCD_RES_HOSTLEN + 100];	/* working buffer */
    unsigned char *current;	/* current position in buf */
    int query_class;	/* answer class */
    int type;		/* answer type */
    int n;			/* temp count */
    int rd_length;
    struct sockaddr_in *v4;	/* conversion */
#ifdef RB_IPV6
    struct sockaddr_in6 *v6;
#endif
    current = (unsigned char *)buf + sizeof(HEADER);

    for (; header->qdcount > 0; --header->qdcount) {
        if ((n = irc_dn_skipname(current, (unsigned char *)eob)) < 0)
            return 0;

        current += (size_t) n + QFIXEDSZ;
    }

    /*
     * process each answer sent to us blech.
     */
    while (header->ancount > 0 && (char *)current < eob) {
        header->ancount--;

        n = irc_dn_expand((unsigned char *)buf, (unsigned char *)eob, current, hostbuf,
                          sizeof(hostbuf));

        if (n < 0) {
            /*
             * broken message
             */
            return (0);
        } else if (n == 0) {
            /*
             * no more answers left
             */
            return (0);
        }

        hostbuf[IRCD_RES_HOSTLEN] = '\0';

        /* With Address arithmetic you have to be very anal
         * this code was not working on alpha due to that
         * (spotted by rodder/jailbird/dianora)
         */
        current += (size_t) n;

        if (!(((char *)current + ANSWER_FIXED_SIZE) < eob))
            break;

        type = irc_ns_get16(current);
        current += TYPE_SIZE;

        query_class = irc_ns_get16(current);
        current += CLASS_SIZE;

        request->ttl = irc_ns_get32(current);
        current += TTL_SIZE;

        rd_length = irc_ns_get16(current);
        current += RDLENGTH_SIZE;

        /*
         * Wait to set request->type until we verify this structure
         */
        switch (type) {
        case T_A:
            if (request->type != T_A)
                return (0);

            /*
             * check for invalid rd_length or too many addresses
             */
            if (rd_length != sizeof(struct in_addr))
                return (0);
            v4 = (struct sockaddr_in *)&request->addr;
            SET_SS_LEN(&request->addr, sizeof(struct sockaddr_in));
            v4->sin_family = AF_INET;
            memcpy(&v4->sin_addr, current, sizeof(struct in_addr));
            return (1);
            break;
#ifdef RB_IPV6
        case T_AAAA:
            if (request->type != T_AAAA)
                return (0);
            if (rd_length != sizeof(struct in6_addr))
                return (0);
            SET_SS_LEN(&request->addr, sizeof(struct sockaddr_in6));
            v6 = (struct sockaddr_in6 *)&request->addr;
            v6->sin6_family = AF_INET6;
            memcpy(&v6->sin6_addr, current, sizeof(struct in6_addr));
            return (1);
            break;
#endif
        case T_PTR:
            if (request->type != T_PTR)
                return (0);
            n = irc_dn_expand((unsigned char *)buf, (unsigned char *)eob, current,
                              hostbuf, sizeof(hostbuf));
            if (n < 0)
                return (0);	/* broken message */
            else if (n == 0)
                return (0);	/* no more answers left */

            rb_strlcpy(request->name, hostbuf, IRCD_RES_HOSTLEN + 1);

            return (1);
            break;
        case T_CNAME:
            /* real answer will follow */
            current += rd_length;
            break;

        default:
            /* XXX I'd rather just throw away the entire bogus thing
             * but its possible its just a broken nameserver with still
             * valid answers. But lets do some rudimentary logging for now...
             */
            ilog(L_MAIN, "irc_res.c bogus type %d", type);
            break;
        }
    }

    return (1);
}

/*
 * res_read_single_reply - read a dns reply from the nameserver and process it.
 * Return value: 1 if a packet was read, 0 otherwise
 */
static int res_read_single_reply(rb_fde_t *F, void *data)
{
    char buf[sizeof(HEADER) + MAXPACKET]
    /* Sparc and alpha need 16bit-alignment for accessing header->id
     * (which is uint16_t). Because of the header = (HEADER*) buf;
     * lateron, this is neeeded. --FaUl
     */
#if defined(__sparc__) || defined(__alpha__)
    __attribute__ ((aligned(16)))
#endif
    ;
    HEADER *header;
    struct reslist *request = NULL;
    struct DNSReply *reply = NULL;
    int rc;
    int answer_count;
    rb_socklen_t len = sizeof(struct rb_sockaddr_storage);
    struct rb_sockaddr_storage lsin;

    rc = recvfrom(rb_get_fd(F), buf, sizeof(buf), 0, (struct sockaddr *)&lsin, &len);

    /* No packet */
    if (rc == 0 || rc == -1)
        return 0;

    /* Too small */
    if (rc <= (int)(sizeof(HEADER)))
        return 1;

    /*
     * convert DNS reply reader from Network byte order to CPU byte order.
     */
    header = (HEADER *) buf;
    header->ancount = ntohs(header->ancount);
    header->qdcount = ntohs(header->qdcount);
    header->nscount = ntohs(header->nscount);
    header->arcount = ntohs(header->arcount);

    /*
     * response for an id which we have already received an answer for
     * just ignore this response.
     */
    if (0 == (request = find_id(header->id)))
        return 1;

    /*
     * check against possibly fake replies
     */
    if (!res_ourserver(&lsin))
        return 1;

    if (!check_question(request, header, buf, buf + rc))
        return 1;

    if ((header->rcode != NO_ERRORS) || (header->ancount == 0)) {
        if (NXDOMAIN == header->rcode) {
            (*request->query->callback) (request->query->ptr, NULL);
            rem_request(request);
        } else {
            /*
             * If a bad error was returned, we stop here and dont send
             * send any more (no retries granted).
             */
            (*request->query->callback) (request->query->ptr, NULL);
            rem_request(request);
        }
        return 1;
    }
    /*
     * If this fails there was an error decoding the received packet,
     * give up. -- jilles
     */
    answer_count = proc_answer(request, header, buf, buf + rc);

    if (answer_count) {
        if (request->type == T_PTR) {
            if (request->name == NULL) {
                /*
                 * got a PTR response with no name, something bogus is happening
                 * don't bother trying again, the client address doesn't resolve
                 */
                (*request->query->callback) (request->query->ptr, reply);
                rem_request(request);
                return 1;
            }

            /*
             * Lookup the 'authoritative' name that we were given for the
             * ip#.
             *
             */
#ifdef RB_IPV6
            if (request->addr.ss_family == AF_INET6)
                gethost_byname_type(request->name, request->query, T_AAAA);
            else
#endif
                gethost_byname_type(request->name, request->query, T_A);
            rem_request(request);
        } else {
            /*
             * got a name and address response, client resolved
             */
            reply = make_dnsreply(request);
            (*request->query->callback) (request->query->ptr, reply);
            rb_free(reply);
            rem_request(request);
        }
    } else {
        /* couldn't decode, give up -- jilles */
        (*request->query->callback) (request->query->ptr, NULL);
        rem_request(request);
    }
    return 1;
}

static void res_readreply(rb_fde_t *F, void *data)
{
    while (res_read_single_reply(F, data))
        ;
    rb_setselect(F, RB_SELECT_READ, res_readreply, NULL);
}

static struct DNSReply *make_dnsreply(struct reslist *request)
{
    struct DNSReply *cp;
    s_assert(request != 0);

    cp = (struct DNSReply *)rb_malloc(sizeof(struct DNSReply));

    cp->h_name = request->name;
    memcpy(&cp->addr, &request->addr, sizeof(cp->addr));
    return (cp);
}

void report_dns_servers(struct Client *source_p)
{
    int i;
    char ipaddr[128];

    for (i = 0; i < irc_nscount; i++) {
        if (!rb_inet_ntop_sock((struct sockaddr *)&(irc_nsaddr_list[i]),
                               ipaddr, sizeof ipaddr))
            rb_strlcpy(ipaddr, "?", sizeof ipaddr);
        sendto_one_numeric(source_p, RPL_STATSDEBUG,
                           "A %s %d", ipaddr, ns_timeout_count[i]);
    }
}
