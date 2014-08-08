#include "stdinc.h"

#include "send.h"
#include "channel.h"
#include "client.h"
#include "common.h"
#include "config.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_newconf.h"
#include "s_serv.h"
#include "hash.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "whowas.h"
#include "monitor.h"

static int me_metadata(struct Client *, struct Client *, int, const char **);

struct Message metadata_msgtab = {
    "METADATA", 0, 0, 0, MFLG_SLOW,
    {mg_ignore, mg_ignore, mg_ignore, mg_ignore, {me_metadata, 3}, mg_ignore}
};

mapi_clist_av1 metadata_clist[] = {
    &metadata_msgtab, NULL
};

DECLARE_MODULE_AV1(metadata, NULL, NULL, metadata_clist, NULL, NULL, "$Revision$");

/*
 * Follows the specification for IRCv3 METADATA
 * http://ircv3.org/specification/metadata-3.2
 */

static int
me_metadata(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
    if(parv[2][0] == '#') {
        struct Channel *chptr;

        if((chptr = find_channel(parv[2])) == NULL)
            return 0;

        //XXX need to support the old, nonstandard verbs for compatability reasons
        if(!strcmp(parv[1], "ADD") && parv[4] != NULL)
            channel_metadata_add(chptr, parv[3], parv[4], 0);
        if(!strcmp(parv[1], "DELETE") && parv[3] != NULL)
            channel_metadata_delete(chptr, parv[3], 0);

        //Now moving on to the standard ones
        if(!strcmp(parv[1], "SET") && parv[4] != NULL)
            channel_metadata_add(chptr, parv[3], parv[4], 0);
        if(!strcmp(parv[1], "CLEAR") && parv[3] != NULL)
            channel_metadata_delete(chptr, parv[3], 0);
    }

    else {
        struct Client *target_p;

        if((target_p = find_id(parv[2])) == NULL)
            return 0;

        if(!target_p->user)
            return 0;

        //XXX need to support the old, nonstandard verbs for compatability reasons
        if(!strcmp(parv[1], "ADD") && parv[4] != NULL)
            user_metadata_add(target_p, parv[3], parv[4], 0);
        if(!strcmp(parv[1], "DELETE") && parv[3] != NULL)
            user_metadata_delete(target_p, parv[3], 0);

        //Now moving on to the standard ones
        if(!strcmp(parv[1], "SET") && parv[4] != NULL)
            user_metadata_add(target_p, parv[3], parv[4], 0);
        if(!strcmp(parv[1], "CLEAR") && parv[3] != NULL)
            user_metadata_delete(target_p, parv[3], 0);
    }
    return 0;
}
