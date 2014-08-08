/*
 * m_cycle.c: server-side CYCLE
 *
 * Copyright (c) 2010 The ShadowIRCd team
 */
#include "stdinc.h"
#include "common.h"
#include "channel.h"
#include "client.h"
#include "hash.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "s_conf.h"
#include "s_serv.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "packet.h"
#include "hook.h"

extern struct module **modlist;

static int m_cycle(struct Client *, struct Client *, int, const char **);

struct Message cycle_msgtab = {
    "CYCLE", 0, 0, 0, MFLG_SLOW,
    {mg_unreg, {m_cycle, 2}, {m_cycle, 2}, mg_ignore, mg_ignore, {m_cycle, 2}}
};
mapi_clist_av1 cycle_clist[] = { &cycle_msgtab, NULL };

DECLARE_MODULE_AV1(cycle, NULL, NULL, cycle_clist, NULL, NULL, "$Revision$");

static int
m_cycle(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
    char *p, *name;
    char *s = LOCAL_COPY(parv[1]);
    struct Channel *chptr;
    struct membership *msptr;

    name = rb_strtok_r(s, ",", &p);

    /* Finish the flood grace period... */
    if(MyClient(source_p) && !IsFloodDone(source_p))
        flood_endgrace(source_p);

    while(name) {
        if((chptr = find_channel(name)) == NULL) {
            sendto_one_numeric(source_p, ERR_NOSUCHCHANNEL, form_str(ERR_NOSUCHCHANNEL), name);
            return 0;
        }

        msptr = find_channel_membership(chptr, source_p);
        if(msptr == NULL) {
            sendto_one_numeric(source_p, ERR_NOTONCHANNEL, form_str(ERR_NOTONCHANNEL), name);
            return 0;
        }

        if(MyConnect(source_p) && !IsOper(source_p) && !IsExemptSpambot(source_p))
            check_spambot_warning(source_p, NULL);

        if((is_any_op(msptr) || !MyConnect(source_p) ||
            ((can_send(chptr, source_p, msptr) > 0 &&
              (source_p->localClient->firsttime +
               ConfigFileEntry.anti_spam_exit_message_time) < rb_current_time())))) {
            sendto_server(client_p, chptr, CAP_TS6, NOCAPS,
                          ":%s PART %s :Cycling", use_id(source_p), chptr->chname);
            sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@%s PART %s :Cycling",
                                 source_p->name, source_p->username,
                                 source_p->host, chptr->chname);
        } else {
            sendto_server(client_p, chptr, CAP_TS6, NOCAPS,
                          ":%s PART %s", use_id(source_p), chptr->chname);
            sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@%s PART %s",
                                 source_p->name, source_p->username,
                                 source_p->host, chptr->chname);
        }

        remove_user_from_channel(msptr);

        chptr = NULL;
        msptr = NULL;

        name = rb_strtok_r(NULL, ",", &p);
    }

    user_join(client_p, source_p, parv[1], parc > 2 ? parv[2] : NULL);

    return 0;
}
