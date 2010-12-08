#include "stdinc.h"
#include "ircd.h"
#include "client.h"
#include "modules.h"
#include "send.h"
#include "numeric.h"
#include "hash.h"
#include "s_serv.h"
#include "inline/stringops.h"
#include "chmode.h"

static int m_scene(struct Client *client_p, struct Client *source_p, int parc, const char *parv[]);
static int m_fsay(struct Client *client_p, struct Client *source_p, int parc, const char *parv[]);
static int m_faction(struct Client *client_p, struct Client *source_p, int parc, const char *parv[]);
static int m_npc(struct Client *client_p, struct Client *source_p, int parc, const char *parv[]);
static int m_npca(struct Client *client_p, struct Client *source_p, int parc, const char *parv[]);
static int m_displaymsg(struct Client *source_p, const char *channel, int underline, int action, const char *nick, const char *text);
static int me_roleplay(struct Client *client_p, struct Client *source_p, int parc, const char *parv[]);
static unsigned int mymode;

static int
_modinit(void)
{
	/* initalize the +x and +d cmodes */
	mymode = cflag_add('x', chm_simple);
	if (mymode == 0)
		return -1;

	mymode = cflag_add('d', chm_simple);
	if (mymode == 0)
		return -1;

	return 0;
}

static void
_moddeinit(void)
{
	/* orphan the +x and +d cmodes on modunload */
	cflag_orphan('x');

	cflag_orphan('d');
}


struct Message scene_msgtab = {
	"SCENE", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_scene, 3}, mg_ignore, mg_ignore, mg_ignore, {m_scene, 3}}
};

struct Message ambiance_msgtab = {
	"AMBIANCE", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_scene, 3}, mg_ignore, mg_ignore, mg_ignore, {m_scene, 3}}
};  

struct Message fsay_msgtab = {
	"FSAY", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_npc, 4}, mg_ignore, mg_ignore, mg_ignore, {m_fsay, 4}}
};  

struct Message faction_msgtab = {
	"FACTION", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_npca, 4}, mg_ignore, mg_ignore, mg_ignore, {m_faction, 4}}
};  

struct Message npc_msgtab = {
	"NPC", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_npc, 4}, mg_ignore, mg_ignore, mg_ignore, {m_npc, 4}}
};  

struct Message npca_msgtab = {
	"NPCA", 0, 0, 0, MFLG_SLOW,
	{mg_unreg, {m_npca, 4}, mg_ignore, mg_ignore, mg_ignore, {m_npca, 4}}
};  

struct Message roleplay_msgtab = {
	"ROLEPLAY", 0, 0, 0, MFLG_SLOW,
	{mg_ignore, mg_ignore, mg_ignore, mg_ignore, {me_roleplay, 5}, mg_ignore}
};  

mapi_clist_av1 roleplay_clist[] = { &scene_msgtab, &ambiance_msgtab, &fsay_msgtab, &faction_msgtab, &npc_msgtab, &npca_msgtab, &roleplay_msgtab, NULL };

DECLARE_MODULE_AV1(roleplay, _modinit, _moddeinit, roleplay_clist, NULL, NULL, "$m_roleplay 1.0 - Taros $");

static int
m_scene(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	m_displaymsg(source_p, parv[1], 0, 0, "=Scene=", parv[2]);
	return 0;
}

static int
m_fsay(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	m_displaymsg(source_p, parv[1], 0, 0, parv[2], parv[3]);
	return 0;
}

static int
m_faction(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	m_displaymsg(source_p, parv[1], 0, 1, parv[2], parv[3]);
	return 0;
}

static int
m_npc(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	m_displaymsg(source_p, parv[1], 1, 0, parv[2], parv[3]);
	return 0;
}

static int
m_npca(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	m_displaymsg(source_p, parv[1], 1, 1, parv[2], parv[3]);
	return 0;
}

static int
m_displaymsg(struct Client *source_p, const char *channel, int underline, int action, const char *nick, const char *text)
{
	struct Channel *chptr;
	struct membership *msptr;
	char nick2[109];
	char *nick3 = rb_strdup(nick);
	char text2[BUFSIZE];

	if((chptr = find_channel(channel)) == NULL)
	{
		sendto_one(source_p, form_str(ERR_NOSUCHCHANNEL), channel);
		return 0;
	}

	if(!(msptr = find_channel_membership(chptr, source_p)))
	{
		sendto_one_numeric(source_p, ERR_NOTONCHANNEL,
				   form_str(ERR_NOTONCHANNEL), chptr->chname);
		return 0;
	}

	if(chptr->mode.mode & chmode_flags['d'])
	{
		sendto_one_numeric(source_p, 573, "%s :Roleplay commands are disabled on this channel (+d)", chptr->chname);
		return 0;
	}

	if(!IsOper(source_p) || chptr->mode.mode & chmode_flags['x'])
	{
		if(chptr->mode.mode & chmode_flags['x'])
		{
			if(!is_chanop_voiced(msptr))
			{
				sendto_one(source_p, ":%s 482 %s %s :You are not a channel operator or voice, and thus cannot use roleplay commands on this channel.",
						me.name, source_p->name, chptr->chname);
				return 0;
			}
		}
		else if(!is_any_op(msptr))
		{
			sendto_one(source_p, ":%s 482 %s %s :You are not a channel operator, and thus cannot use roleplay commands on this channel.",
					me.name, source_p->name, chptr->chname);	
			return 0;
		}
	}

	if(underline)
		rb_snprintf(nick2, sizeof(nick2), "\x1F%s\x1F", strip_unprintable(nick3));
	else
		rb_snprintf(nick2, sizeof(nick2), "%s", strip_unprintable(nick3));

	if(EmptyString(nick3))
	{
		sendto_one_numeric(source_p, 573, "%s :No visible non-stripped characters in nick.", chptr->chname);
		return 0;
	}

	if(action)
		rb_snprintf(text2, sizeof(text2), "\1ACTION %s", text);
	else
		rb_snprintf(text2, sizeof(text2), "%s", text);

	sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@npc.fakeuser.invalid PRIVMSG %s :%s", nick2, source_p->name, channel, text2); 
	sendto_match_servs(&me, "*", CAP_ENCAP, NOCAPS, "ENCAP * ROLEPLAY %s %s %s :%s",
			source_p->name, channel, nick2, text2);
	return 0;
}

static int
me_roleplay(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	struct Channel *chptr;

	/* Don't segfault if we get ENCAP * ROLEPLAY with an invalid channel.
	 * This shouldn't happen but it's best to be on the safe side. */
	if((chptr = find_channel(parv[2])) == NULL)
		return 0;

	sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@npc.fakeuser.invalid PRIVMSG %s :%s", parv[3], parv[1], parv[2], parv[4]); 
	return 0;
}
