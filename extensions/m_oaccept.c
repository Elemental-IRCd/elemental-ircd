#include "stdinc.h"
#include "ircd.h"
#include "client.h"
#include "modules.h"
#include "send.h"
#include "numeric.h"
#include "hash.h"

void mo_oaccept(struct Client *client_p, struct Client *source_p, int parc, const char *parv[]);

struct Message oaccept_msgtab = {
	"OACCEPT", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, mg_not_oper, mg_ignore, mg_ignore, mg_ignore, {mo_oaccept, 2}}
};  

mapi_clist_av1 oaccept_clist[] = { &oaccept_msgtab, NULL };

DECLARE_MODULE_AV1(omode, NULL, NULL, oaccept_clist, NULL, NULL, "$Id $");

void
mo_oaccept(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Metadata *md;
	struct Client *target_p;

	if(!(target_p = find_client(parv[1])))
	{
		sendto_one(source_p, form_str(ERR_NOSUCHNICK), parv[1]);
		return;
	}

	/* Don't allow someone to pointlessly fill up someone's metadata
	 * with identical OACCEPT entries. */
	if((md = user_metadata_find(target_p, "OACCEPT")))
		if(!strcmp(source_p->id, md->value))
		{
			sendto_one_notice(source_p, ":You're already on %s's OACCEPT list", target_p->name);
			return;
		}

	user_metadata_add(target_p, "OACCEPT", source_p->id, 1);

	sendto_wallops_flags(UMODE_WALLOP, &me,
			     "OACCEPT called for %s by %s!%s@%s",
			     target_p->name,
			     source_p->name, source_p->username, source_p->host);
}
