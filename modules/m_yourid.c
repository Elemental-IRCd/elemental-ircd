/*
 * Elemental-IRCd: Internet Relay Chat Program Server
 * m_yourid.c: show a client's unique ID on connect, ircd 2.11 style
 */

#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"
#include "hash.h"
#include "s_conf.h"
#include "s_user.h"
#include "s_serv.h"
#include "numeric.h"

static void check_new_user(void *data);

mapi_hfn_list_av1 m_yourip_hfnlist[] = {
    { "new_local_user", (hookfn) check_new_user },
    { NULL, NULL }
};

DECLARE_MODULE_AV1(m_yourip, NULL, NULL, NULL, NULL,
                   m_yourip_hfnlist, "$Revision$");

static void
check_new_user(void *vdata)
{
    struct Client *source_p = (void *)vdata;
    sendto_one_numeric(source_p, RPL_YOURID, form_str(RPL_YOURID), source_p->id);
}
