/*
 *  ssld.c: The ircd-ratbox ssl helper daemon thingy
 *  Copyright (C) 2007 Aaron Sethman <androsyn@ratbox.org>
 *  Copyright (C) 2007 ircd-ratbox development team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 *  $Id$
 */

/**
 * IPC spec
 *
 * a dgram socket is created and inherited by to ssld.
 *
 * s_ctlfd = getenv("CTL_FD");
 *
 *----------------------------------------------------------------------
 * Commands passed via ctl fd to ssld:
 *
 * Accept and Connect:
 *  u8   u32
 * ['A'|connection id]
 * ['C'|connection id]
 *
 * Descriptors:
 * [0]: mod_fd, ssl socket
 * [1]: plain_fd, socket to the ircd for plaintext
 *
 *
 * New keys:
 *  u8  char[]
 * ['K'|cert\0priv key\0(opt)dh params\0]
 *
 * Certificates and keys are passed as concantenated null terminated
 * ascii (probably openssl format).
 * dh params are optional
 *
 * Init PRNG:
 *  u8  u8        char[]
 * ['I'|seed_type|path'\0']
 *
 * This is intended to be passed to rb_init_prng pretty much unmodified
 * seed_type is of type prng_seed_t, defined in rb_commio.h
 *
 *----------------------------------------------------------------------
 * Commands passed via ctl fd from the ssld:
 *
 * Dead fd:
 *  u8  u32           char[]
 * ['D'|connection id|reason\0]
 *
 * Indicates a session has disconnected
 *
 * Cert fp:
 *  u8  u32           u8[RB_SSL_CERTFP_LEN]
 * ['F'|connection id|fingerprint]
 *
 * Certificate fingerprint for sasl authentication
 *
 * I am useless/cannot initialize:
 *  u8
 * ['U']
 * ['I']
 *
 * indicates that ssld can't do ssl (?)
 * 'U' libratbox reports no ssl support
 * 'I' configuration error
 **/

#include "stdinc.h"

#define MAXPASSFD 4
#ifndef READBUF_SIZE
#define READBUF_SIZE 16384
#endif

static void setup_signals(void);
static pid_t ppid;

static inline int32_t
buf_to_int32(char *buf)
{
    int32_t x;
    memcpy(&x, buf, sizeof(x));
    return x;
}

static inline void
int32_to_buf(char *buf, int32_t x)
{
    memcpy(buf, &x, sizeof(x));
    return;
}

static inline uint16_t
buf_to_uint16(char *buf)
{
    uint16_t x;
    memcpy(&x, buf, sizeof(x));
    return x;
}

static inline void
uint16_to_buf(char *buf, uint16_t x)
{
    memcpy(buf, &x, sizeof(x));
    return;
}


static char inbuf[READBUF_SIZE];

typedef struct _mod_ctl_buf {
    rb_dlink_node node;
    char *buf;
    size_t buflen;
    rb_fde_t *F[MAXPASSFD];
    int nfds;
} mod_ctl_buf_t;

typedef struct _mod_ctl {
    rb_dlink_node node;
    int cli_count;
    rb_fde_t *F;
    rb_dlink_list readq;
    rb_dlink_list writeq;
} mod_ctl_t;

static mod_ctl_t *mod_ctl;


typedef struct _conn {
    rb_dlink_node node;
    mod_ctl_t *ctl;
    rawbuf_head_t *modbuf_out;
    rawbuf_head_t *plainbuf_out;

    int32_t id;

    rb_fde_t *mod_fd;
    rb_fde_t *plain_fd;
    unsigned long long mod_out;
    unsigned long long mod_in;
    unsigned long long plain_in;
    unsigned long long plain_out;
    uint8_t flags;
    void *stream;
} conn_t;

#define FLAG_CORK          (1 << 0)
#define FLAG_DEAD          (1 << 1)
#define FLAG_SSL_W_WANTS_R (1 << 2)   /* output needs to wait until input possible */
#define FLAG_SSL_R_WANTS_W (1 << 3)   /* input needs to wait until output possible */

#define IsCork(x) ((x)->flags & FLAG_CORK)
#define IsDead(x) ((x)->flags & FLAG_DEAD)
#define IsSSLWWantsR(x) ((x)->flags & FLAG_SSL_W_WANTS_R)
#define IsSSLRWantsW(x) ((x)->flags & FLAG_SSL_R_WANTS_W)

#define SetCork(x) ((x)->flags |= FLAG_CORK)
#define SetDead(x) ((x)->flags |= FLAG_DEAD)
#define SetSSLWWantsR(x) ((x)->flags |= FLAG_SSL_W_WANTS_R)
#define SetSSLRWantsW(x) ((x)->flags |= FLAG_SSL_R_WANTS_W)

#define ClearCork(x) ((x)->flags &= ~FLAG_CORK)
#define ClearDead(x) ((x)->flags &= ~FLAG_DEAD)
#define ClearSSLWWantsR(x) ((x)->flags &= ~FLAG_SSL_W_WANTS_R)
#define ClearSSLRWantsW(x) ((x)->flags &= ~FLAG_SSL_R_WANTS_W)

#define NO_WAIT 0x0
#define WAIT_PLAIN 0x1

#define HASH_WALK_SAFE(i, max, ptr, next, table) for(i = 0; i < max; i++) { RB_DLINK_FOREACH_SAFE(ptr, next, table[i].head)
#define HASH_WALK_END }
#define CONN_HASH_SIZE 2000
#define connid_hash(x)	(&connid_hash_table[(x % CONN_HASH_SIZE)])



static rb_dlink_list connid_hash_table[CONN_HASH_SIZE];
static rb_dlink_list dead_list;

static void conn_mod_read_cb(rb_fde_t *fd, void *data);
static void conn_mod_write_sendq(rb_fde_t *, void *data);
static void conn_plain_write_sendq(rb_fde_t *, void *data);
static void mod_write_ctl(rb_fde_t *, void *data);
static void conn_plain_read_cb(rb_fde_t *fd, void *data);
static void conn_plain_read_shutdown_cb(rb_fde_t *fd, void *data);
static void mod_cmd_write_queue(mod_ctl_t * ctl, const void *data, size_t len);
static const char *remote_closed = "Remote host closed the connection";
static int ssl_ok;

static void send_i_am_useless(mod_ctl_t * ctl);
static void send_config_error(mod_ctl_t * ctl);


static conn_t *
conn_find_by_id(int32_t id)
{
    rb_dlink_node *ptr;
    conn_t *conn;

    RB_DLINK_FOREACH(ptr, (connid_hash(id))->head) {
        conn = ptr->data;
        if(conn->id == id && !IsDead(conn))
            return conn;
    }
    return NULL;
}

static void
conn_add_id_hash(conn_t * conn, int32_t id)
{
    conn->id = id;
    rb_dlinkAdd(conn, &conn->node, connid_hash(id));
}

static void
free_conn(conn_t * conn)
{
    rb_free_rawbuffer(conn->modbuf_out);
    rb_free_rawbuffer(conn->plainbuf_out);
    rb_free(conn);
}

static void
clean_dead_conns(void *unused)
{
    conn_t *conn;
    rb_dlink_node *ptr, *next;
    RB_DLINK_FOREACH_SAFE(ptr, next, dead_list.head) {
        conn = ptr->data;
        free_conn(conn);
    }
    dead_list.tail = dead_list.head = NULL;
}


/* Close a connection and send a human readable reason */
static void
close_conn(conn_t * conn, int wait_plain, const char *fmt, ...)
{
    va_list ap;
    char reason[128];	/* must always be under 250 bytes */
    char buf[256];
    int len;
    if(IsDead(conn))
        return;

    rb_rawbuf_flush(conn->modbuf_out, conn->mod_fd);
    rb_rawbuf_flush(conn->plainbuf_out, conn->plain_fd);
    rb_close(conn->mod_fd);
    SetDead(conn);

    if(conn->id >= 0)
        rb_dlinkDelete(&conn->node, connid_hash(conn->id));

    if(!wait_plain || fmt == NULL) {
        rb_close(conn->plain_fd);
        rb_dlinkAdd(conn, &conn->node, &dead_list);
        return;
    }
    rb_setselect(conn->plain_fd, RB_SELECT_READ, conn_plain_read_shutdown_cb, conn);
    rb_setselect(conn->plain_fd, RB_SELECT_WRITE, NULL, NULL);
    va_start(ap, fmt);
    rb_vsnprintf(reason, sizeof(reason), fmt, ap);
    va_end(ap);

    /* Send dead fd message */
    buf[0] = 'D';
    int32_to_buf(&buf[1], conn->id);
    strcpy(&buf[5], reason);
    len = (strlen(reason) + 1) + 5;
    mod_cmd_write_queue(conn->ctl, buf, len);
}

/* Create a new connection from file descriptors */
static conn_t *
make_conn(mod_ctl_t * ctl, rb_fde_t *mod_fd, rb_fde_t *plain_fd)
{
    conn_t *conn = rb_malloc(sizeof(conn_t));
    conn->ctl = ctl;
    conn->modbuf_out = rb_new_rawbuffer();
    conn->plainbuf_out = rb_new_rawbuffer();
    conn->mod_fd = mod_fd;
    conn->plain_fd = plain_fd;
    conn->id = -1;
    conn->stream = NULL;
    rb_set_nb(mod_fd);
    rb_set_nb(plain_fd);
    return conn;
}

static void
check_handshake_flood(void *unused)
{
    conn_t *conn;
    rb_dlink_node *ptr, *next;
    unsigned int count;
    int i;
    HASH_WALK_SAFE(i, CONN_HASH_SIZE, ptr, next, connid_hash_table) {
        conn = ptr->data;

        count = rb_ssl_handshake_count(conn->mod_fd);
        /* nothing needs to do this more than twice in ten seconds i don't think */
        if(count > 2)
            close_conn(conn, WAIT_PLAIN, "Handshake flooding");
        else
            rb_ssl_clear_handshake_count(conn->mod_fd);
    }
    HASH_WALK_END
}

static void
conn_mod_write_sendq(rb_fde_t *fd, void *data)
{
    conn_t *conn = data;
    const char *err;
    int retlen;
    if(IsDead(conn))
        return;

    if(IsSSLWWantsR(conn)) {
        ClearSSLWWantsR(conn);
        conn_mod_read_cb(conn->mod_fd, conn);
        if(IsDead(conn))
            return;
    }

    while((retlen = rb_rawbuf_flush(conn->modbuf_out, fd)) > 0)
        conn->mod_out += retlen;

    if(retlen == 0 || (retlen < 0 && !rb_ignore_errno(errno))) {
        if(retlen == 0)
            close_conn(conn, WAIT_PLAIN, "%s", remote_closed);
        if(retlen == RB_RW_SSL_ERROR)
            err = rb_get_ssl_strerror(conn->mod_fd);
        else
            err = strerror(errno);
        close_conn(conn, WAIT_PLAIN, "Write error: %s", err);
        return;
    }
    if(rb_rawbuf_length(conn->modbuf_out) > 0) {
        if(retlen != RB_RW_SSL_NEED_READ)
            rb_setselect(conn->mod_fd, RB_SELECT_WRITE, conn_mod_write_sendq, conn);
        else {
            rb_setselect(conn->mod_fd, RB_SELECT_READ, conn_mod_write_sendq, conn);
            rb_setselect(conn->mod_fd, RB_SELECT_WRITE, NULL, NULL);
            SetSSLWWantsR(conn);
        }
    } else
        rb_setselect(conn->mod_fd, RB_SELECT_WRITE, NULL, NULL);

    if(IsCork(conn) && rb_rawbuf_length(conn->modbuf_out) == 0) {
        ClearCork(conn);
        conn_plain_read_cb(conn->plain_fd, conn);
    }

}

static void
conn_mod_write(conn_t * conn, void *data, size_t len)
{
    if(IsDead(conn))	/* no point in queueing to a dead man */
        return;
    rb_rawbuf_append(conn->modbuf_out, data, len);
}

static void
conn_plain_write(conn_t * conn, void *data, size_t len)
{
    if(IsDead(conn))	/* again no point in queueing to dead men */
        return;
    rb_rawbuf_append(conn->plainbuf_out, data, len);
}

/* Queue a control message */
static void
mod_cmd_write_queue(mod_ctl_t * ctl, const void *data, size_t len)
{
    mod_ctl_buf_t *ctl_buf;
    ctl_buf = rb_malloc(sizeof(mod_ctl_buf_t));
    ctl_buf->buf = rb_malloc(len);
    ctl_buf->buflen = len;
    memcpy(ctl_buf->buf, data, len);
    ctl_buf->nfds = 0;
    rb_dlinkAddTail(ctl_buf, &ctl_buf->node, &ctl->writeq);
    mod_write_ctl(ctl->F, ctl);
}

static int
plain_check_cork(conn_t * conn)
{
    if(rb_rawbuf_length(conn->modbuf_out) >= 4096) {
        /* if we have over 4k pending outbound, don't read until
         * we've cleared the queue */
        SetCork(conn);
        rb_setselect(conn->plain_fd, RB_SELECT_READ, NULL, NULL);
        /* try to write */
        conn_mod_write_sendq(conn->mod_fd, conn);
        return 1;
    }
    return 0;
}


/* Read plaintext from the ircd and send to the client */
static void
conn_plain_read_cb(rb_fde_t *fd, void *data)
{
    conn_t *conn = data;
    int length = 0;
    if(conn == NULL)
        return;

    if(IsDead(conn))
        return;

    if(plain_check_cork(conn))
        return;

    while(1) {
        if(IsDead(conn))
            return;

        length = rb_read(conn->plain_fd, inbuf, sizeof(inbuf));

        if(length == 0 || (length < 0 && !rb_ignore_errno(errno))) {
            close_conn(conn, NO_WAIT, NULL);
            return;
        }

        if(length < 0) {
            rb_setselect(conn->plain_fd, RB_SELECT_READ, conn_plain_read_cb, conn);
            conn_mod_write_sendq(conn->mod_fd, conn);
            return;
        }
        conn->plain_in += length;
        conn_mod_write(conn, inbuf, length);

        if(IsDead(conn))
            return;
        if(plain_check_cork(conn))
            return;
    }
}

static void
conn_plain_read_shutdown_cb(rb_fde_t *fd, void *data)
{
    conn_t *conn = data;
    int length = 0;

    if(conn == NULL)
        return;

    while(1) {
        length = rb_read(conn->plain_fd, inbuf, sizeof(inbuf));

        if(length == 0 || (length < 0 && !rb_ignore_errno(errno))) {
            rb_close(conn->plain_fd);
            rb_dlinkAdd(conn, &conn->node, &dead_list);
            return;
        }

        if(length < 0) {
            rb_setselect(conn->plain_fd, RB_SELECT_READ, conn_plain_read_shutdown_cb, conn);
            return;
        }
    }
}

/* Read ciphertext from the client and forward to the ircd */
static void
conn_mod_read_cb(rb_fde_t *fd, void *data)
{
    conn_t *conn = data;
    const char *err;
    int length;
    if(conn == NULL)
        return;
    if(IsDead(conn))
        return;

    if(IsSSLRWantsW(conn)) {
        ClearSSLRWantsW(conn);
        conn_mod_write_sendq(conn->mod_fd, conn);
        if(IsDead(conn))
            return;
    }

    while(1) {
        if(IsDead(conn))
            return;

        length = rb_read(conn->mod_fd, inbuf, sizeof(inbuf));

        if(length == 0 || (length < 0 && !rb_ignore_errno(errno))) {
            if(length == 0) {
                close_conn(conn, WAIT_PLAIN, "%s", remote_closed);
                return;
            }

            if(length == RB_RW_SSL_ERROR)
                err = rb_get_ssl_strerror(conn->mod_fd);
            else
                err = strerror(errno);
            close_conn(conn, WAIT_PLAIN, "Read error: %s", err);
            return;
        }
        if(length < 0) {
            if(length != RB_RW_SSL_NEED_WRITE)
                rb_setselect(conn->mod_fd, RB_SELECT_READ, conn_mod_read_cb, conn);
            else {
                rb_setselect(conn->mod_fd, RB_SELECT_READ, NULL, NULL);
                rb_setselect(conn->mod_fd, RB_SELECT_WRITE, conn_mod_read_cb, conn);
                SetSSLRWantsW(conn);
            }
            conn_plain_write_sendq(conn->plain_fd, conn);
            return;
        }
        conn->mod_in += length;
        conn_plain_write(conn, inbuf, length);
    }
}

static void
conn_plain_write_sendq(rb_fde_t *fd, void *data)
{
    conn_t *conn = data;
    int retlen;

    if(IsDead(conn))
        return;

    while((retlen = rb_rawbuf_flush(conn->plainbuf_out, fd)) > 0) {
        conn->plain_out += retlen;
    }
    if(retlen == 0 || (retlen < 0 && !rb_ignore_errno(errno))) {
        close_conn(data, NO_WAIT, NULL);
        return;
    }

    if(rb_rawbuf_length(conn->plainbuf_out) > 0)
        rb_setselect(conn->plain_fd, RB_SELECT_WRITE, conn_plain_write_sendq, conn);
    else
        rb_setselect(conn->plain_fd, RB_SELECT_WRITE, NULL, NULL);
}

static int
maxconn(void)
{
#if defined(RLIMIT_NOFILE) && defined(HAVE_SYS_RESOURCE_H)
    struct rlimit limit;

    if(!getrlimit(RLIMIT_NOFILE, &limit)) {
        return limit.rlim_cur;
    }
#endif /* RLIMIT_FD_MAX */
    return MAXCONNECTIONS;
}

static void
ssl_process_accept_cb(rb_fde_t *F, int status, struct sockaddr *addr, rb_socklen_t len, void *data)
{
    conn_t *conn = data;
    char buf[5 + RB_SSL_CERTFP_LEN];

    if(status == RB_OK) {
        if(rb_get_ssl_certfp(F, &buf[5])) {
            /* Send client certificate fingerprint */
            buf[0] = 'F';
            int32_to_buf(&buf[1], conn->id);
            mod_cmd_write_queue(conn->ctl, buf, sizeof(buf));
        }
        conn_mod_read_cb(conn->mod_fd, conn);
        conn_plain_read_cb(conn->plain_fd, conn);
        return;
    }
    /* ircd doesn't care about the reason for this */
    close_conn(conn, NO_WAIT, 0);
    return;
}

static void
ssl_process_connect_cb(rb_fde_t *F, int status, void *data)
{
    conn_t *conn = data;
    char buf[5 + RB_SSL_CERTFP_LEN];

    if(status == RB_OK) {
        if(rb_get_ssl_certfp(F, &buf[5])) {
            buf[0] = 'F';
            int32_to_buf(&buf[1], conn->id);
            mod_cmd_write_queue(conn->ctl, buf, sizeof buf);
        }
        conn_mod_read_cb(conn->mod_fd, conn);
        conn_plain_read_cb(conn->plain_fd, conn);
    } else if(status == RB_ERR_TIMEOUT)
        close_conn(conn, WAIT_PLAIN, "SSL handshake timed out");
    else if(status == RB_ERROR_SSL)
        close_conn(conn, WAIT_PLAIN, "%s", rb_get_ssl_strerror(conn->mod_fd));
    else
        close_conn(conn, WAIT_PLAIN, "SSL handshake failed");
}

static void
ssl_process_accept(mod_ctl_t * ctl, mod_ctl_buf_t * ctlb)
{
    conn_t *conn;
    int32_t id;

    conn = make_conn(ctl, ctlb->F[0], ctlb->F[1]);

    id = buf_to_int32(&ctlb->buf[1]);

    if(id >= 0)
        conn_add_id_hash(conn, id);

    if(rb_get_type(conn->mod_fd) & RB_FD_UNKNOWN)
        rb_set_type(conn->mod_fd, RB_FD_SOCKET);

    if(rb_get_type(conn->mod_fd) & RB_FD_UNKNOWN)
        rb_set_type(conn->plain_fd, RB_FD_SOCKET);

    rb_ssl_start_accepted(ctlb->F[0], ssl_process_accept_cb, conn, 10);
}

static void
ssl_process_connect(mod_ctl_t * ctl, mod_ctl_buf_t * ctlb)
{
    conn_t *conn;
    int32_t id;

    conn = make_conn(ctl, ctlb->F[0], ctlb->F[1]);

    id = buf_to_int32(&ctlb->buf[1]);

    if(id >= 0)
        conn_add_id_hash(conn, id);

    if(rb_get_type(conn->mod_fd) & RB_FD_UNKNOWN)
        rb_set_type(conn->mod_fd, RB_FD_SOCKET);

    if(rb_get_type(conn->mod_fd) & RB_FD_UNKNOWN)
        rb_set_type(conn->plain_fd, RB_FD_SOCKET);

    rb_ssl_start_connected(ctlb->F[0], ssl_process_connect_cb, conn, 10);
}

static void
init_prng(mod_ctl_t * ctl, mod_ctl_buf_t * ctl_buf)
{
    char *path;
    prng_seed_t seed_type;

    seed_type = (prng_seed_t) ctl_buf->buf[1];
    path = &ctl_buf->buf[2];
    rb_init_prng(path, seed_type);
}


static void
ssl_new_keys(mod_ctl_t * ctl, mod_ctl_buf_t * ctl_buf)
{
    char *buf;
    char *cert, *key, *dhparam;

    buf = &ctl_buf->buf[2];
    cert = buf;
    buf += strlen(cert) + 1;
    key = buf;
    buf += strlen(key) + 1;
    dhparam = buf;
    if(strlen(dhparam) == 0)
        dhparam = NULL;

    if(!rb_setup_ssl_server(cert, key, dhparam))
        send_config_error(ctl);
}

static void
send_i_am_useless(mod_ctl_t * ctl)
{
    static const char *useless = "U";
    mod_cmd_write_queue(ctl, useless, strlen(useless));
}

static void
send_config_error(mod_ctl_t * ctl)
{
    static const char invalid[] = "I";
    mod_cmd_write_queue(ctl, invalid, strlen(invalid));
}

/* Pass queued control message to the apropriate handler */
static void
mod_process_cmd_recv(mod_ctl_t * ctl)
{
    rb_dlink_node *ptr, *next;
    mod_ctl_buf_t *ctl_buf;

    RB_DLINK_FOREACH_SAFE(ptr, next, ctl->readq.head) {
        ctl_buf = ptr->data;

        switch (*ctl_buf->buf) {

        case 'A':
            ssl_process_accept(ctl, ctl_buf);
            break;

        case 'C':
            ssl_process_connect(ctl, ctl_buf);
            break;

        case 'K':
            ssl_new_keys(ctl, ctl_buf);
            break;

        case 'I':
            init_prng(ctl, ctl_buf);
            break;

        default:
            break;
            /* Log unknown commands */
        }
        rb_dlinkDelete(ptr, &ctl->readq);
        rb_free(ctl_buf->buf);
        rb_free(ctl_buf);
    }

}

/* Accept and queue a control message and its descriptors */
static void
mod_read_ctl(rb_fde_t *F, void *data)
{
    mod_ctl_buf_t *ctl_buf;
    mod_ctl_t *ctl = data;
    int retlen;
    int i;

    do {
        ctl_buf = rb_malloc(sizeof(mod_ctl_buf_t));
        ctl_buf->buf = rb_malloc(READBUF_SIZE);
        ctl_buf->buflen = READBUF_SIZE;
        retlen = rb_recv_fd_buf(ctl->F, ctl_buf->buf, ctl_buf->buflen, ctl_buf->F,
                                MAXPASSFD);
        if(retlen <= 0) {
            rb_free(ctl_buf->buf);
            rb_free(ctl_buf);
        } else {
            ctl_buf->buflen = retlen;
            rb_dlinkAddTail(ctl_buf, &ctl_buf->node, &ctl->readq);
            for (i = 0; i < MAXPASSFD && ctl_buf->F[i] != NULL; i++)
                ;
            ctl_buf->nfds = i;
        }
    } while(retlen > 0);

    if(retlen == 0 || (retlen < 0 && !rb_ignore_errno(errno)))
        exit(0);

    mod_process_cmd_recv(ctl);
    rb_setselect(ctl->F, RB_SELECT_READ, mod_read_ctl, ctl);
}

/* Attempt to write a control message */
static void
mod_write_ctl(rb_fde_t *F, void *data)
{
    mod_ctl_t *ctl = data;
    mod_ctl_buf_t *ctl_buf;
    rb_dlink_node *ptr, *next;
    int retlen, x;

    RB_DLINK_FOREACH_SAFE(ptr, next, ctl->writeq.head) {
        ctl_buf = ptr->data;
        retlen = rb_send_fd_buf(ctl->F, ctl_buf->F, ctl_buf->nfds, ctl_buf->buf,
                                ctl_buf->buflen, ppid);
        if(retlen > 0) {
            rb_dlinkDelete(ptr, &ctl->writeq);
            for(x = 0; x < ctl_buf->nfds; x++)
                rb_close(ctl_buf->F[x]);
            rb_free(ctl_buf->buf);
            rb_free(ctl_buf);

        }
        if(retlen == 0 || (retlen < 0 && !rb_ignore_errno(errno)))
            exit(0);

    }
    if(rb_dlink_list_length(&ctl->writeq) > 0)
        rb_setselect(ctl->F, RB_SELECT_WRITE, mod_write_ctl, ctl);
}

int
main(int argc, char **argv)
{
    const char *s_ctlfd, *s_pid;
    int ctlfd, x, maxfd;
    maxfd = maxconn();

    s_ctlfd = getenv("CTL_FD");
    s_pid = getenv("CTL_PPID");

    if(s_ctlfd == NULL || s_pid == NULL) {
        fprintf(stderr,
                "This is ircd-ratbox ssld.  You know you aren't supposed to run me directly?\n");
        fprintf(stderr,
                "You get an Id tag for this: $Id$\n");
        fprintf(stderr, "Have a nice life\n");
        exit(1);
    }

    ctlfd = atoi(s_ctlfd);
    ppid = atoi(s_pid);

#ifndef _WIN32
    for(x = 0; x < maxfd; x++) {
        if(x != ctlfd && x > 2)
            close(x);
    }
    x = open("/dev/null", O_RDWR);

    if(x >= 0) {
        if(ctlfd != 0)
            dup2(x, 0);
        if(ctlfd != 1)
            dup2(x, 1);
        if(ctlfd != 2)
            dup2(x, 2);
        if(x > 2)
            close(x);
    }
#endif
    setup_signals();
    rb_lib_init(NULL, NULL, NULL, 0, maxfd, 1024, 4096);
    rb_init_rawbuffers(1024);
    ssl_ok = rb_supports_ssl();
    mod_ctl = rb_malloc(sizeof(mod_ctl_t));
    mod_ctl->F = rb_open(ctlfd, RB_FD_SOCKET, "ircd control socket");
    rb_set_nb(mod_ctl->F);
    rb_event_addish("clean_dead_conns", clean_dead_conns, NULL, 10);
    rb_event_add("check_handshake_flood", check_handshake_flood, NULL, 10);
    mod_read_ctl(mod_ctl->F, mod_ctl);
    if(!ssl_ok) {
        /* this is really useless... */
        send_i_am_useless(mod_ctl);
        /* sleep until the ircd kills us */
        rb_sleep(2 << 30, 0);
        exit(1);
    }

    rb_lib_loop(0);
    return 0;
}


#ifndef _WIN32
static void
dummy_handler(int sig)
{
    return;
}
#endif

static void
setup_signals()
{
#ifndef _WIN32
    struct sigaction act;

    act.sa_flags = 0;
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGPIPE);
    sigaddset(&act.sa_mask, SIGALRM);
#ifdef SIGTRAP
    sigaddset(&act.sa_mask, SIGTRAP);
#endif

#ifdef SIGWINCH
    sigaddset(&act.sa_mask, SIGWINCH);
    sigaction(SIGWINCH, &act, 0);
#endif
    sigaction(SIGPIPE, &act, 0);
#ifdef SIGTRAP
    sigaction(SIGTRAP, &act, 0);
#endif

    act.sa_handler = dummy_handler;
    sigaction(SIGALRM, &act, 0);
#endif
}
