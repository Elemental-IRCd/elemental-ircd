/*
 *  ircd-ratbox: A slightly useful ircd.
 *  channel.c: Controls channels.
 *
 * Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center 
 * Copyright (C) 1996-2002 Hybrid Development Team 
 * Copyright (C) 2002-2005 ircd-ratbox development team 
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *  $Id: channel.c 3580 2007-11-07 23:45:14Z jilles $
 */

#include "stdinc.h"
#include "channel.h"
#include "chmode.h"
#include "client.h"
#include "common.h"
#include "hash.h"
#include "hook.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"		/* captab */
#include "s_user.h"
#include "send.h"
#include "whowas.h"
#include "s_conf.h"		/* ConfigFileEntry, ConfigChannel */
#include "s_newconf.h"
#include "logger.h"
#include "packet.h"
#include "irc_dictionary.h"

struct config_channel_entry ConfigChannel;
rb_dlink_list global_channel_list;
static rb_bh *channel_heap;
static rb_bh *ban_heap;
static rb_bh *topic_heap;
static rb_bh *member_heap;

static int channel_capabs[] = { CAP_EX, CAP_IE,
	CAP_SERVICE,
	CAP_TS6
};

#define NCHCAPS         (sizeof(channel_capabs)/sizeof(int))
#define NCHCAP_COMBOS   (1 << NCHCAPS)

static struct ChCapCombo chcap_combos[NCHCAP_COMBOS];

static void free_topic(struct Channel *chptr);

static int h_can_join;
static int h_can_create_channel;
static int h_channel_join;

/* init_channels()
 *
 * input	-
 * output	-
 * side effects - initialises the various blockheaps
 */
void
init_channels(void)
{
	channel_heap = rb_bh_create(sizeof(struct Channel), CHANNEL_HEAP_SIZE, "channel_heap");
	ban_heap = rb_bh_create(sizeof(struct Ban), BAN_HEAP_SIZE, "ban_heap");
	topic_heap = rb_bh_create(TOPICLEN + 1 + USERHOST_REPLYLEN, TOPIC_HEAP_SIZE, "topic_heap");
	member_heap = rb_bh_create(sizeof(struct membership), MEMBER_HEAP_SIZE, "member_heap");

	h_can_join = register_hook("can_join");
	h_channel_join = register_hook("channel_join");
	h_can_create_channel = register_hook("can_create_channel");
}

/*
 * allocate_channel - Allocates a channel
 */
struct Channel *
allocate_channel(const char *chname)
{
	struct Channel *chptr;
	struct Dictionary *metadata;
	chptr = rb_bh_alloc(channel_heap);
	chptr->chname = rb_strdup(chname);
	
	metadata = irc_dictionary_create(irccmp);
	chptr->metadata = metadata;
	return (chptr);
}

void
free_channel(struct Channel *chptr)
{
	channel_metadata_clear(chptr);
	rb_free(chptr->chname);
	rb_bh_free(channel_heap, chptr);
}

struct Ban *
allocate_ban(const char *banstr, const char *who)
{
	struct Ban *bptr;
	bptr = rb_bh_alloc(ban_heap);
	bptr->banstr = rb_strdup(banstr);
	bptr->who = rb_strdup(who);

	return (bptr);
}

void
free_ban(struct Ban *bptr)
{
	rb_free(bptr->banstr);
	rb_free(bptr->who);
	rb_bh_free(ban_heap, bptr);
}


/* find_channel_membership()
 *
 * input	- channel to find them in, client to find
 * output	- membership of client in channel, else NULL
 * side effects	-
 */
struct membership *
find_channel_membership(struct Channel *chptr, struct Client *client_p)
{
	struct membership *msptr;
	rb_dlink_node *ptr;

	if(!IsClient(client_p))
		return NULL;

	/* Pick the most efficient list to use to be nice to things like
	 * CHANSERV which could be in a large number of channels
	 */
	if(rb_dlink_list_length(&chptr->members) < rb_dlink_list_length(&client_p->user->channel))
	{
		RB_DLINK_FOREACH(ptr, chptr->members.head)
		{
			msptr = ptr->data;

			if(msptr->client_p == client_p)
				return msptr;
		}
	}
	else
	{
		RB_DLINK_FOREACH(ptr, client_p->user->channel.head)
		{
			msptr = ptr->data;

			if(msptr->chptr == chptr)
				return msptr;
		}
	}

	return NULL;
}

/* find_channel_status()
 *
 * input	- membership to get status for, whether we can combine flags
 * output	- flags of user on channel
 * side effects -
 */
const char *
find_channel_status(struct membership *msptr, int combine)
{
	static char buffer[5];
	char *p;

	p = buffer;

	if(is_admin(msptr))
	{
		if(!combine)
			return "!";
		*p++ = '!';
	}

	if(is_chanop(msptr))
	{
		if(!combine)
			return "@";
		*p++ = '@';
	}

	if(is_halfop(msptr))
	{
		if(!combine)
			return "%";
		*p++ = '%';
	}

	if(is_voiced(msptr))
		*p++ = '+';

	*p = '\0';
	return buffer;
}

/* is_halfop()
 *
 * input    - membership to check for halfops
 * output   - 1 if the user is halfopped, 0 if the user is not or halfop 
 * is disabled.
 * side effects - 
 *
 */
int
is_halfop(struct membership *msptr)
{
	if(!ConfigChannel.use_halfop)
		return 0;
	if(is_chmode_h(msptr))
		return 1;
	else
		return 0;
}

/* is_admin()
 *
 * input    - membership to check for admin
 * output   - 1 if the user is an admin, 0 if the user is not or admin 
 * is disabled.
 * side effects - 
 *
 */
int
is_admin(struct membership *msptr)
{
	if(!ConfigChannel.use_admin)
		return 0;
	if(is_chmode_a(msptr))
		return 1;
	else
		return 0;
}

/* is_any_op()
 *
 * input	- membership to check for ops
 * output	- 1 if the user is op, halfop, or admin, 0 elsewise
 * side effects - 
 */
int
is_any_op(struct membership *msptr)
{
	if(is_chanop(msptr) || is_halfop(msptr) || is_admin(msptr))
		return 1;
	else
		return 0;
}

/* is_chanop_voiced()
 *
 * input	- memebership to check for status
 * output	- 1 if the user is op, halfop, admin, or voice, 0 elsewise
 * side effects -
 */
int
is_chanop_voiced(struct membership *msptr)
{
	if(is_chanop(msptr) || is_voiced(msptr) || is_halfop(msptr) || is_admin(msptr))
		return 1;
	else
		return 0;
}

/* can_kick_deop()
 *
 * input	- two memeberships
 * output	- 1 if the first memebership can kick/deop the second, 0 elsewise
 * side effects -
 */
int
can_kick_deop(struct membership *source, struct membership *target)
{
	if(is_chanop(source) && !is_admin(target))
		return 1;
	else if(is_halfop(source) && !is_any_op(target))
		return 1;
	else if(is_admin(source))
		return 1;

	return 0;
}

/* add_user_to_channel()
 *
 * input	- channel to add client to, client to add, channel flags
 * output	- 
 * side effects - user is added to channel
 */
void
add_user_to_channel(struct Channel *chptr, struct Client *client_p, int flags)
{
	struct membership *msptr;

	s_assert(client_p->user != NULL);
	if(client_p->user == NULL)
		return;

	msptr = rb_bh_alloc(member_heap);

	msptr->chptr = chptr;
	msptr->client_p = client_p;
	msptr->flags = flags;

	rb_dlinkAdd(msptr, &msptr->usernode, &client_p->user->channel);
	rb_dlinkAdd(msptr, &msptr->channode, &chptr->members);

	if(MyClient(client_p))
		rb_dlinkAdd(msptr, &msptr->locchannode, &chptr->locmembers);
}

/* remove_user_from_channel()
 *
 * input	- membership pointer to remove from channel
 * output	-
 * side effects - membership (thus user) is removed from channel
 */
void
remove_user_from_channel(struct membership *msptr)
{
	struct Client *client_p;
	struct Channel *chptr;
	s_assert(msptr != NULL);
	if(msptr == NULL)
		return;

	client_p = msptr->client_p;
	chptr = msptr->chptr;

	rb_dlinkDelete(&msptr->usernode, &client_p->user->channel);
	rb_dlinkDelete(&msptr->channode, &chptr->members);

	if(client_p->servptr == &me)
		rb_dlinkDelete(&msptr->locchannode, &chptr->locmembers);

	if(!(chptr->mode.mode & MODE_PERMANENT) && rb_dlink_list_length(&chptr->members) <= 0)
		destroy_channel(chptr);

	rb_bh_free(member_heap, msptr);

	return;
}

/* remove_user_from_channels()
 *
 * input        - user to remove from all channels
 * output       -
 * side effects - user is removed from all channels
 */
void
remove_user_from_channels(struct Client *client_p)
{
	struct Channel *chptr;
	struct membership *msptr;
	rb_dlink_node *ptr;
	rb_dlink_node *next_ptr;

	if(client_p == NULL)
		return;

	RB_DLINK_FOREACH_SAFE(ptr, next_ptr, client_p->user->channel.head)
	{
		msptr = ptr->data;
		chptr = msptr->chptr;

		rb_dlinkDelete(&msptr->channode, &chptr->members);

		if(client_p->servptr == &me)
			rb_dlinkDelete(&msptr->locchannode, &chptr->locmembers);

		if(!(chptr->mode.mode & MODE_PERMANENT) && rb_dlink_list_length(&chptr->members) <= 0)
			destroy_channel(chptr);

		rb_bh_free(member_heap, msptr);
	}

	client_p->user->channel.head = client_p->user->channel.tail = NULL;
	client_p->user->channel.length = 0;
}

/* invalidate_bancache_user()
 *
 * input	- user to invalidate ban cache for
 * output	-
 * side effects - ban cache is invalidated for all memberships of that user
 *                to be used after a nick change
 */
void
invalidate_bancache_user(struct Client *client_p)
{
	struct membership *msptr;
	rb_dlink_node *ptr;

	if(client_p == NULL)
		return;

	RB_DLINK_FOREACH(ptr, client_p->user->channel.head)
	{
		msptr = ptr->data;
		msptr->bants = 0;
		msptr->flags &= ~CHFL_BANNED;
	}
}

/* check_channel_name()
 *
 * input	- channel name
 * output	- 1 if valid channel name, else 0
 * side effects -
 */
int
check_channel_name(const char *name)
{
	s_assert(name != NULL);
	if(name == NULL)
		return 0;

	for (; *name; ++name)
	{
		if(!IsChanChar(*name))
			return 0;
	}

	return 1;
}

/* free_channel_list()
 *
 * input	- rb_dlink list to free
 * output	-
 * side effects - list of b/e/I modes is cleared
 */
void
free_channel_list(rb_dlink_list * list)
{
	rb_dlink_node *ptr;
	rb_dlink_node *next_ptr;
	struct Ban *actualBan;

	RB_DLINK_FOREACH_SAFE(ptr, next_ptr, list->head)
	{
		actualBan = ptr->data;
		free_ban(actualBan);
	}

	list->head = list->tail = NULL;
	list->length = 0;
}

/* destroy_channel()
 *
 * input	- channel to destroy
 * output	-
 * side effects - channel is obliterated
 */
void
destroy_channel(struct Channel *chptr)
{
	rb_dlink_node *ptr, *next_ptr;

	RB_DLINK_FOREACH_SAFE(ptr, next_ptr, chptr->invites.head)
	{
		del_invite(chptr, ptr->data);
	}

	/* free all bans/exceptions/denies */
	free_channel_list(&chptr->banlist);
	free_channel_list(&chptr->exceptlist);
	free_channel_list(&chptr->invexlist);
	free_channel_list(&chptr->quietlist);

	/* Free the topic */
	free_topic(chptr);

	rb_dlinkDelete(&chptr->node, &global_channel_list);
	del_from_channel_hash(chptr->chname, chptr);
	free_channel(chptr);
}

/* channel_pub_or_secret()
 *
 * input	- channel
 * output	- "=" if public, "@" if secret, else "*"
 * side effects	-
 */
static const char *
channel_pub_or_secret(struct Channel *chptr)
{
	if(PubChannel(chptr))
		return ("=");
	else if(SecretChannel(chptr))
		return ("@");
	return ("*");
}

/* channel_member_names()
 *
 * input	- channel to list, client to list to, show endofnames
 * output	-
 * side effects - client is given list of users on channel
 */
void
channel_member_names(struct Channel *chptr, struct Client *client_p, int show_eon)
{
	struct membership *msptr;
	struct Client *target_p;
	rb_dlink_node *ptr;
	char lbuf[BUFSIZE];
	char *t;
	int mlen;
	int tlen;
	int cur_len;
	int is_member;
	int stack = IsCapable(client_p, CLICAP_MULTI_PREFIX);

	if(ShowChannel(client_p, chptr))
	{
		is_member = IsMember(client_p, chptr);

		cur_len = mlen = rb_sprintf(lbuf, form_str(RPL_NAMREPLY),
					    me.name, client_p->name,
					    channel_pub_or_secret(chptr), chptr->chname);

		t = lbuf + cur_len;

		RB_DLINK_FOREACH(ptr, chptr->members.head)
		{
			msptr = ptr->data;
			target_p = msptr->client_p;

			if(IsInvisible(target_p) && !is_member)
				continue;

			/* space, possible "@+" prefix */
			if(cur_len + strlen(target_p->name) + 3 >= BUFSIZE - 3)
			{
				*(t - 1) = '\0';
				sendto_one(client_p, "%s", lbuf);
				cur_len = mlen;
				t = lbuf + mlen;
			}

			tlen = rb_sprintf(t, "%s%s ", find_channel_status(msptr, stack),
					  target_p->name);

			cur_len += tlen;
			t += tlen;
		}

		/* The old behaviour here was to always output our buffer,
		 * even if there are no clients we can show.  This happens
		 * when a client does "NAMES" with no parameters, and all
		 * the clients on a -sp channel are +i.  I dont see a good
		 * reason for keeping that behaviour, as it just wastes
		 * bandwidth.  --anfl
		 */
		if(cur_len != mlen)
		{
			*(t - 1) = '\0';
			sendto_one(client_p, "%s", lbuf);
		}
	}

	if(show_eon)
		sendto_one(client_p, form_str(RPL_ENDOFNAMES),
			   me.name, client_p->name, chptr->chname);
}

/* del_invite()
 *
 * input	- channel to remove invite from, client to remove
 * output	-
 * side effects - user is removed from invite list, if exists
 */
void
del_invite(struct Channel *chptr, struct Client *who)
{
	rb_dlinkFindDestroy(who, &chptr->invites);
	rb_dlinkFindDestroy(chptr, &who->user->invited);
}

/* is_banned()
 *
 * input	- channel to check bans for, user to check bans against
 *                optional prebuilt buffers
 * output	- 1 if banned, else 0
 * side effects -
 */
int
is_banned(struct Channel *chptr, struct Client *who, struct membership *msptr,
	  const char *s, const char *s2)
{
	char src_host[NICKLEN + USERLEN + HOSTLEN + 6];
	char src_iphost[NICKLEN + USERLEN + HOSTLEN + 6];
	char src_althost[NICKLEN + USERLEN + HOSTLEN + 6];
	char *s3 = NULL;
	rb_dlink_node *ptr;
	struct Ban *actualBan = NULL;
	struct Ban *actualExcept = NULL;

	if(!MyClient(who))
		return 0;

	/* if the buffers havent been built, do it here */
	if(s == NULL)
	{
		rb_sprintf(src_host, "%s!%s@%s", who->name, who->username, who->host);
		rb_sprintf(src_iphost, "%s!%s@%s", who->name, who->username, who->sockhost);

		s = src_host;
		s2 = src_iphost;
	}
	if(who->localClient->mangledhost != NULL)
	{
		/* if host mangling mode enabled, also check their real host */
		if(!strcmp(who->host, who->localClient->mangledhost))
		{
			rb_sprintf(src_althost, "%s!%s@%s", who->name, who->username, who->orighost);
			s3 = src_althost;
		}
		/* if host mangling mode not enabled and no other spoof,
		 * also check the mangled form of their host */
		else if (!IsDynSpoof(who))
		{
			rb_sprintf(src_althost, "%s!%s@%s", who->name, who->username, who->localClient->mangledhost);
			s3 = src_althost;
		}
	}

	RB_DLINK_FOREACH(ptr, chptr->banlist.head)
	{
		actualBan = ptr->data;
		if(match(actualBan->banstr, s) ||
		   match(actualBan->banstr, s2) ||
		   match_cidr(actualBan->banstr, s2) ||
		   match_extban(actualBan->banstr, who, chptr, CHFL_BAN) ||
		   (s3 != NULL && match(actualBan->banstr, s3)))
			break;
		else
			actualBan = NULL;
	}

	if((actualBan != NULL) && ConfigChannel.use_except)
	{
		RB_DLINK_FOREACH(ptr, chptr->exceptlist.head)
		{
			actualExcept = ptr->data;

			/* theyre exempted.. */
			if(match(actualExcept->banstr, s) ||
			   match(actualExcept->banstr, s2) ||
			   match_cidr(actualExcept->banstr, s2) ||
			   match_extban(actualExcept->banstr, who, chptr, CHFL_EXCEPTION) ||
			   (s3 != NULL && match(actualExcept->banstr, s3)))
			{
				/* cache the fact theyre not banned */
				if(msptr != NULL)
				{
					msptr->bants = chptr->bants;
					msptr->flags &= ~CHFL_BANNED;
				}

				return CHFL_EXCEPTION;
			}
		}
	}

	/* cache the banned/not banned status */
	if(msptr != NULL)
	{
		msptr->bants = chptr->bants;

		if(actualBan != NULL)
		{
			msptr->flags |= CHFL_BANNED;
			return CHFL_BAN;
		}
		else
		{
			msptr->flags &= ~CHFL_BANNED;
			return 0;
		}
	}

	return ((actualBan ? CHFL_BAN : 0));
}

/* is_quieted()
 *
 * input	- channel to check bans for, user to check bans against
 *                optional prebuilt buffers
 * output	- 1 if banned, else 0
 * side effects -
 */
int
is_quieted(struct Channel *chptr, struct Client *who, struct membership *msptr,
	   const char *s, const char *s2)
{
	char src_host[NICKLEN + USERLEN + HOSTLEN + 6];
	char src_iphost[NICKLEN + USERLEN + HOSTLEN + 6];
	char src_althost[NICKLEN + USERLEN + HOSTLEN + 6];
	char *s3 = NULL;
	rb_dlink_node *ptr;
	struct Ban *actualBan = NULL;
	struct Ban *actualExcept = NULL;

	if(!MyClient(who))
		return 0;

	/* if the buffers havent been built, do it here */
	if(s == NULL)
	{
		rb_sprintf(src_host, "%s!%s@%s", who->name, who->username, who->host);
		rb_sprintf(src_iphost, "%s!%s@%s", who->name, who->username, who->sockhost);

		s = src_host;
		s2 = src_iphost;
	}
	if(who->localClient->mangledhost != NULL)
	{
		/* if host mangling mode enabled, also check their real host */
		if(!strcmp(who->host, who->localClient->mangledhost))
		{
			rb_sprintf(src_althost, "%s!%s@%s", who->name, who->username, who->orighost);
			s3 = src_althost;
		}
		/* if host mangling mode not enabled and no other spoof,
		 * also check the mangled form of their host */
		else if (!IsDynSpoof(who))
		{
			rb_sprintf(src_althost, "%s!%s@%s", who->name, who->username, who->localClient->mangledhost);
			s3 = src_althost;
		}
	}

	RB_DLINK_FOREACH(ptr, chptr->quietlist.head)
	{
		actualBan = ptr->data;
		if(match(actualBan->banstr, s) ||
		   match(actualBan->banstr, s2) ||
		   match_cidr(actualBan->banstr, s2) ||
		   match_extban(actualBan->banstr, who, chptr, CHFL_QUIET) ||
		   (s3 != NULL && match(actualBan->banstr, s3)))
			break;
		else
			actualBan = NULL;
	}

	if((actualBan != NULL) && ConfigChannel.use_except)
	{
		RB_DLINK_FOREACH(ptr, chptr->exceptlist.head)
		{
			actualExcept = ptr->data;

			/* theyre exempted.. */
			if(match(actualExcept->banstr, s) ||
			   match(actualExcept->banstr, s2) ||
			   match_cidr(actualExcept->banstr, s2) ||
			   match_extban(actualExcept->banstr, who, chptr, CHFL_EXCEPTION) ||
			   (s3 != NULL && match(actualExcept->banstr, s3)))
			{
				/* cache the fact theyre not banned */
				if(msptr != NULL)
				{
					msptr->bants = chptr->bants;
					msptr->flags &= ~CHFL_BANNED;
				}

				return CHFL_EXCEPTION;
			}
		}
	}

	/* cache the banned/not banned status */
	if(msptr != NULL)
	{
		msptr->bants = chptr->bants;

		if(actualBan != NULL)
		{
			msptr->flags |= CHFL_BANNED;
			return CHFL_BAN;
		}
		else
		{
			msptr->flags &= ~CHFL_BANNED;
			return 0;
		}
	}

	return ((actualBan ? CHFL_BAN : 0));
}

/* can_join()
 *
 * input	- client to check, channel to check for, key
 * output	- reason for not being able to join, else 0
 * side effects -
 */
int
can_join(struct Client *source_p, struct Channel *chptr, char *key)
{
	rb_dlink_node *invite = NULL;
	rb_dlink_node *ptr;
	struct Ban *invex = NULL;
	char src_host[NICKLEN + USERLEN + HOSTLEN + 6];
	char src_iphost[NICKLEN + USERLEN + HOSTLEN + 6];
	char src_althost[NICKLEN + USERLEN + HOSTLEN + 6];
	char *text = rb_strdup("");
	int use_althost = 0;
	int i = 0;
	hook_data_channel moduledata;
	struct Metadata *md;
	struct DictionaryIter iter;

	s_assert(source_p->localClient != NULL);

	rb_sprintf(src_host, "%s!%s@%s", source_p->name, source_p->username, source_p->host);
	rb_sprintf(src_iphost, "%s!%s@%s", source_p->name, source_p->username, source_p->sockhost);
	if(source_p->localClient->mangledhost != NULL)
	{
		/* if host mangling mode enabled, also check their real host */
		if(!strcmp(source_p->host, source_p->localClient->mangledhost))
		{
			rb_sprintf(src_althost, "%s!%s@%s", source_p->name, source_p->username, source_p->orighost);
			use_althost = 1;
		}
		/* if host mangling mode not enabled and no other spoof,
		 * also check the mangled form of their host */
		else if (!IsDynSpoof(source_p))
		{
			rb_sprintf(src_althost, "%s!%s@%s", source_p->name, source_p->username, source_p->localClient->mangledhost);
			use_althost = 1;
		}
	}

	if((is_banned(chptr, source_p, NULL, src_host, src_iphost)) == CHFL_BAN)
		return (ERR_BANNEDFROMCHAN);

	rb_sprintf(text, "K%s", source_p->id);

	DICTIONARY_FOREACH(md, &iter, chptr->metadata)
	{
		if(!strcmp(md->value, "KICKNOREJOIN") && !strcmp(md->name, text) && (md->timevalue + ConfigChannel.kick_no_rejoin_time > rb_current_time()))
			return ERR_KICKNOREJOIN;
		/* cleanup any stale KICKNOREJOIN metadata we find while we're at it */
		if(!strcmp(md->value, "KICKNOREJOIN") && !(md->timevalue + ConfigChannel.kick_no_rejoin_time > rb_current_time()))  
			channel_metadata_delete(chptr, md->name, 0);
	}

	if(chptr->mode.mode & MODE_INVITEONLY)
	{
		RB_DLINK_FOREACH(invite, source_p->user->invited.head)
		{
			if(invite->data == chptr)
				break;
		}
		if(invite == NULL)
		{
			if(!ConfigChannel.use_invex)
				return (ERR_INVITEONLYCHAN);
			RB_DLINK_FOREACH(ptr, chptr->invexlist.head)
			{
				invex = ptr->data;
				if(match(invex->banstr, src_host)
				   || match(invex->banstr, src_iphost)
				   || match_cidr(invex->banstr, src_iphost)
			   	   || match_extban(invex->banstr, source_p, chptr, CHFL_INVEX)
				   || (use_althost && match(invex->banstr, src_althost)))
					break;
			}
			if(ptr == NULL)
				return (ERR_INVITEONLYCHAN);
		}
	}

	if(*chptr->mode.key && (EmptyString(key) || irccmp(chptr->mode.key, key)))
		return (ERR_BADCHANNELKEY);

	if(chptr->mode.limit &&
	   rb_dlink_list_length(&chptr->members) >= (unsigned long) chptr->mode.limit)
		i = ERR_CHANNELISFULL;
	if(chptr->mode.mode & MODE_REGONLY && EmptyString(source_p->user->suser))
		i = ERR_NEEDREGGEDNICK;
	/* join throttling stuff --nenolod */
	else if(chptr->mode.join_num > 0 && chptr->mode.join_time > 0)
	{
		if ((rb_current_time() - chptr->join_delta <= 
			chptr->mode.join_time) && (chptr->join_count >=
			chptr->mode.join_num))
			i = ERR_THROTTLE;
	}

	/* allow /invite to override +l/+r/+j also -- jilles */
	if (i != 0 && invite == NULL)
	{
		RB_DLINK_FOREACH(invite, source_p->user->invited.head)
		{
			if(invite->data == chptr)
				break;
		}
		if (invite == NULL)
			return i;
	}

	moduledata.client = source_p;
	moduledata.chptr = chptr;
	moduledata.approved = 0;

	call_hook(h_can_join, &moduledata);

	return moduledata.approved;
}

/* can_send()
 *
 * input	- user to check in channel, membership pointer
 * output	- whether can explicitly send or not, else CAN_SEND_NONOP
 * side effects -
 */
int
can_send(struct Channel *chptr, struct Client *source_p, struct membership *msptr)
{
	if(IsServer(source_p) || IsService(source_p))
		return CAN_SEND_OPV;

	if(MyClient(source_p) && hash_find_resv(chptr->chname) &&
	   !IsOper(source_p) && !IsExemptResv(source_p))
		return CAN_SEND_NO;

	if(msptr == NULL)
	{
		msptr = find_channel_membership(chptr, source_p);

		if(msptr == NULL)
		{
			/* if its +m or +n and theyre not in the channel,
			 * they cant send.  we dont check bans here because
			 * theres no possibility of caching them --fl
			 */
			if(chptr->mode.mode & MODE_NOPRIVMSGS || chptr->mode.mode & MODE_MODERATED)
				return CAN_SEND_NO;
			else
				return CAN_SEND_NONOP;
		}
	}

	if(is_chanop_voiced(msptr))
		return CAN_SEND_OPV;

	if(chptr->mode.mode & MODE_MODERATED)
		return CAN_SEND_NO;

	if(MyClient(source_p))
	{
		/* cached can_send */
		if(msptr->bants == chptr->bants)
		{
			if(can_send_banned(msptr))
				return CAN_SEND_NO;
		}
		else if(is_banned(chptr, source_p, msptr, NULL, NULL) == CHFL_BAN
			|| is_quieted(chptr, source_p, msptr, NULL, NULL) == CHFL_BAN)
			return CAN_SEND_NO;
	}

	return CAN_SEND_NONOP;
}

/* find_bannickchange_channel()
 * Input: client to check
 * Output: channel preventing nick change
 */
struct Channel *
find_bannickchange_channel(struct Client *client_p)
{
	struct Channel *chptr;
	struct membership *msptr;
	rb_dlink_node *ptr;
	char src_host[NICKLEN + USERLEN + HOSTLEN + 6];
	char src_iphost[NICKLEN + USERLEN + HOSTLEN + 6];

	if (!MyClient(client_p) || IsOverride(client_p))
		return NULL;

	rb_sprintf(src_host, "%s!%s@%s", client_p->name, client_p->username, client_p->host);
	rb_sprintf(src_iphost, "%s!%s@%s", client_p->name, client_p->username, client_p->sockhost);

	RB_DLINK_FOREACH(ptr, client_p->user->channel.head)
	{
		msptr = ptr->data;
		chptr = msptr->chptr;
		if (is_chanop_voiced(msptr))
			continue;
		/* cached can_send */
		if (msptr->bants == chptr->bants)
		{
			if (can_send_banned(msptr))
				return chptr;
		}
		else if (is_banned(chptr, client_p, msptr, src_host, src_iphost) == CHFL_BAN
			|| is_quieted(chptr, client_p, msptr, src_host, src_iphost) == CHFL_BAN)
			return chptr;
	}
	return NULL;
}

/* find_nonickchange_channel()
 * Input: client to check
 * Output: channel preventing nick change
 */
struct Channel *
find_nonickchange_channel(struct Client *client_p)
{
	struct Channel *chptr;
	struct membership *msptr;
	rb_dlink_node *ptr;

	if (!MyClient(client_p))
		return NULL;

	RB_DLINK_FOREACH(ptr, client_p->user->channel.head)
	{
		msptr = ptr->data;
		chptr = msptr->chptr;
		if (chptr->mode.mode & MODE_NONICK && (!ConfigChannel.exempt_cmode_N || !is_any_op(msptr)))
			return chptr;
	}
	return NULL;
}

/* void check_spambot_warning(struct Client *source_p)
 * Input: Client to check, channel name or NULL if this is a part.
 * Output: none
 * Side-effects: Updates the client's oper_warn_count_down, warns the
 *    IRC operators if necessary, and updates join_leave_countdown as
 *    needed.
 */
void
check_spambot_warning(struct Client *source_p, const char *name)
{
	int t_delta;
	int decrement_count;
	if((GlobalSetOptions.spam_num &&
	    (source_p->localClient->join_leave_count >= GlobalSetOptions.spam_num)))
	{
		if(source_p->localClient->oper_warn_count_down > 0)
			source_p->localClient->oper_warn_count_down--;
		else
			source_p->localClient->oper_warn_count_down = 0;
		if(source_p->localClient->oper_warn_count_down == 0 &&
				name != NULL)
		{
			/* Its already known as a possible spambot */
			sendto_realops_snomask(SNO_BOTS, L_NETWIDE,
					     "User %s (%s@%s) trying to join %s is a possible spambot",
					     source_p->name,
					     source_p->username, source_p->orighost, name);
			source_p->localClient->oper_warn_count_down = OPER_SPAM_COUNTDOWN;
		}
	}
	else
	{
		if((t_delta =
		    (rb_current_time() - source_p->localClient->last_leave_time)) >
		   JOIN_LEAVE_COUNT_EXPIRE_TIME)
		{
			decrement_count = (t_delta / JOIN_LEAVE_COUNT_EXPIRE_TIME);
			if(name != NULL)
				;
			else if(decrement_count > source_p->localClient->join_leave_count)
				source_p->localClient->join_leave_count = 0;
			else
				source_p->localClient->join_leave_count -= decrement_count;
		}
		else
		{
			if((rb_current_time() -
			    (source_p->localClient->last_join_time)) < GlobalSetOptions.spam_time)
			{
				/* oh, its a possible spambot */
				source_p->localClient->join_leave_count++;
			}
		}
		if(name != NULL)
			source_p->localClient->last_join_time = rb_current_time();
		else
			source_p->localClient->last_leave_time = rb_current_time();
	}
}

/* check_splitmode()
 *
 * input	-
 * output	-
 * side effects - compares usercount and servercount against their split
 *                values and adjusts splitmode accordingly
 */
void
check_splitmode(void *unused)
{
	if(splitchecking && (ConfigChannel.no_join_on_split || ConfigChannel.no_create_on_split))
	{
		/* not split, we're being asked to check now because someone
		 * has left
		 */
		if(!splitmode)
		{
			if(eob_count < split_servers || Count.total < split_users)
			{
				splitmode = 1;
				sendto_realops_snomask(SNO_GENERAL, L_ALL,
						     "Network split, activating splitmode");
				check_splitmode_ev = rb_event_addish("check_splitmode", check_splitmode, NULL, 2);
			}
		}
		/* in splitmode, check whether its finished */
		else if(eob_count >= split_servers && Count.total >= split_users)
		{
			splitmode = 0;

			sendto_realops_snomask(SNO_GENERAL, L_ALL,
					     "Network rejoined, deactivating splitmode");

			rb_event_delete(check_splitmode_ev);
			check_splitmode_ev = NULL;
		}
	}
}


/* allocate_topic()
 *
 * input	- channel to allocate topic for
 * output	- 1 on success, else 0
 * side effects - channel gets a topic allocated
 */
static void
allocate_topic(struct Channel *chptr)
{
	void *ptr;

	if(chptr == NULL)
		return;

	ptr = rb_bh_alloc(topic_heap);

	/* Basically we allocate one large block for the topic and
	 * the topic info.  We then split it up into two and shove it
	 * in the chptr 
	 */
	chptr->topic = ptr;
	chptr->topic_info = (char *) ptr + TOPICLEN + 1;
	*chptr->topic = '\0';
	*chptr->topic_info = '\0';
}

/* free_topic()
 *
 * input	- channel which has topic to free
 * output	-
 * side effects - channels topic is free'd
 */
static void
free_topic(struct Channel *chptr)
{
	void *ptr;

	if(chptr == NULL || chptr->topic == NULL)
		return;

	/* This is safe for now - If you change allocate_topic you
	 * MUST change this as well
	 */
	ptr = chptr->topic;
	rb_bh_free(topic_heap, ptr);
	chptr->topic = NULL;
	chptr->topic_info = NULL;
}

/* set_channel_topic()
 *
 * input	- channel, topic to set, topic info and topic ts
 * output	-
 * side effects - channels topic, topic info and TS are set.
 */
void
set_channel_topic(struct Channel *chptr, const char *topic, const char *topic_info, time_t topicts)
{
	if(strlen(topic) > 0)
	{
		if(chptr->topic == NULL)
			allocate_topic(chptr);
		rb_strlcpy(chptr->topic, topic, TOPICLEN + 1);
		rb_strlcpy(chptr->topic_info, topic_info, USERHOST_REPLYLEN);
		chptr->topic_time = topicts;
	}
	else
	{
		if(chptr->topic != NULL)
			free_topic(chptr);
		chptr->topic_time = 0;
	}
}

/* has_common_channel()
 * 
 * input	- pointer to client
 *			- pointer to another client
 * output	- 1 if the two have a channel in common, 0 elsewise
 * side effects - none
 */
int
has_common_channel(struct Client *client1, struct Client *client2)
{
	rb_dlink_node *ptr;

	RB_DLINK_FOREACH(ptr, client1->user->channel.head)
	{
		if(IsMember(client2, ((struct membership *)ptr->data)->chptr))
			return 1;
	}
	return 0;
}

/* channel_modes()
 *
 * inputs       - pointer to channel
 *              - pointer to client
 * output       - string with simple modes
 * side effects - result from previous calls overwritten
 *
 * Stolen from ShadowIRCd 4 --nenolod
 */
const char *
channel_modes(struct Channel *chptr, struct Client *client_p)
{
	int i;
	char buf1[BUFSIZE];
	char buf2[BUFSIZE];
	static char final[BUFSIZE];
	char *mbuf = buf1;
	char *pbuf = buf2;

	*mbuf++ = '+';
	*pbuf = '\0';

	for (i = 0; i < 256; i++)
	{
		if(chmode_table[i].set_func == chm_hidden && !IsOper(client_p) && IsClient(client_p))
			continue;
		if(chptr->mode.mode & chmode_flags[i])
			*mbuf++ = i;
	}

	if(chptr->mode.limit)
	{
		*mbuf++ = 'l';

		if(!IsClient(client_p) || IsMember(client_p, chptr))
			pbuf += rb_sprintf(pbuf, " %d", chptr->mode.limit);
	}

	if(*chptr->mode.key)
	{
		*mbuf++ = 'k';

		if(pbuf > buf2 || !IsClient(client_p) || IsMember(client_p, chptr))
			pbuf += rb_sprintf(pbuf, " %s", chptr->mode.key);
	}

	if(chptr->mode.join_num)
	{
		*mbuf++ = 'j';

		if(pbuf > buf2 || !IsClient(client_p) || IsMember(client_p, chptr))
			pbuf += rb_sprintf(pbuf, " %d:%d", chptr->mode.join_num,
					   chptr->mode.join_time);
	}

	if(*chptr->mode.forward && (ConfigChannel.use_forward || !IsClient(client_p)))
	{
		*mbuf++ = 'f';

		if(pbuf > buf2 || !IsClient(client_p) || IsMember(client_p, chptr))
			pbuf += rb_sprintf(pbuf, " %s", chptr->mode.forward);
	}

	*mbuf = '\0';

	rb_strlcpy(final, buf1, sizeof final);
	rb_strlcat(final, buf2, sizeof final);
	return final;
}

/* Now lets do some stuff to keep track of what combinations of
 * servers exist...
 * Note that the number of combinations doubles each time you add
 * something to this list. Each one is only quick if no servers use that
 * combination, but if the numbers get too high here MODE will get too
 * slow. I suggest if you get more than 7 here, you consider getting rid
 * of some and merging or something. If it wasn't for irc+cs we would
 * probably not even need to bother about most of these, but unfortunately
 * we do. -A1kmm
 */

/* void init_chcap_usage_counts(void)
 *
 * Inputs	- none
 * Output	- none
 * Side-effects	- Initialises the usage counts to zero. Fills in the
 *                chcap_yes and chcap_no combination tables.
 */
void
init_chcap_usage_counts(void)
{
	unsigned long m, c, y, n;

	memset(chcap_combos, 0, sizeof(chcap_combos));

	/* For every possible combination */
	for (m = 0; m < NCHCAP_COMBOS; m++)
	{
		/* Check each capab */
		for (c = y = n = 0; c < NCHCAPS; c++)
		{
			if((m & (1 << c)) == 0)
				n |= channel_capabs[c];
			else
				y |= channel_capabs[c];
		}
		chcap_combos[m].cap_yes = y;
		chcap_combos[m].cap_no = n;
	}
}

/* void set_chcap_usage_counts(struct Client *serv_p)
 * Input: serv_p; The client whose capabs to register.
 * Output: none
 * Side-effects: Increments the usage counts for the correct capab
 *               combination.
 */
void
set_chcap_usage_counts(struct Client *serv_p)
{
	int n;

	for (n = 0; n < NCHCAP_COMBOS; n++)
	{
		if(IsCapable(serv_p, chcap_combos[n].cap_yes) &&
		   NotCapable(serv_p, chcap_combos[n].cap_no))
		{
			chcap_combos[n].count++;
			return;
		}
	}

	/* This should be impossible -A1kmm. */
	s_assert(0);
}

/* void set_chcap_usage_counts(struct Client *serv_p)
 *
 * Inputs	- serv_p; The client whose capabs to register.
 * Output	- none
 * Side-effects	- Decrements the usage counts for the correct capab
 *                combination.
 */
void
unset_chcap_usage_counts(struct Client *serv_p)
{
	int n;

	for (n = 0; n < NCHCAP_COMBOS; n++)
	{
		if(IsCapable(serv_p, chcap_combos[n].cap_yes) &&
		   NotCapable(serv_p, chcap_combos[n].cap_no))
		{
			/* Hopefully capabs can't change dynamically or anything... */
			s_assert(chcap_combos[n].count > 0);

			if(chcap_combos[n].count > 0)
				chcap_combos[n].count--;
			return;
		}
	}

	/* This should be impossible -A1kmm. */
	s_assert(0);
}

/* void send_cap_mode_changes(struct Client *client_p,
 *                        struct Client *source_p,
 *                        struct Channel *chptr, int cap, int nocap)
 * Input: The client sending(client_p), the source client(source_p),
 *        the channel to send mode changes for(chptr)
 * Output: None.
 * Side-effects: Sends the appropriate mode changes to capable servers.
 *
 * Reverted back to my original design, except that we now keep a count
 * of the number of servers which each combination as an optimisation, so
 * the capabs combinations which are not needed are not worked out. -A1kmm
 */
void
send_cap_mode_changes(struct Client *client_p, struct Client *source_p,
		      struct Channel *chptr, struct ChModeChange mode_changes[], int mode_count)
{
	static char modebuf[BUFSIZE];
	static char parabuf[BUFSIZE];
	int i, mbl, pbl, nc, mc, preflen, len;
	char *pbuf;
	const char *arg;
	int dir;
	int j;
	int cap;
	int nocap;
	int arglen;

	/* Now send to servers... */
	for (j = 0; j < NCHCAP_COMBOS; j++)
	{
		if(chcap_combos[j].count == 0)
			continue;

		mc = 0;
		nc = 0;
		pbl = 0;
		parabuf[0] = 0;
		pbuf = parabuf;
		dir = MODE_QUERY;

		cap = chcap_combos[j].cap_yes;
		nocap = chcap_combos[j].cap_no;

		mbl = preflen = rb_sprintf(modebuf, ":%s TMODE %ld %s ",
					   use_id(source_p), (long) chptr->channelts,
					   chptr->chname);

		/* loop the list of - modes we have */
		for (i = 0; i < mode_count; i++)
		{
			/* if they dont support the cap we need, or they do support a cap they
			 * cant have, then dont add it to the modebuf.. that way they wont see
			 * the mode
			 */
			if((mode_changes[i].letter == 0) ||
			   ((cap & mode_changes[i].caps) != mode_changes[i].caps)
			   || ((nocap & mode_changes[i].nocaps) != mode_changes[i].nocaps))
				continue;

			if(!EmptyString(mode_changes[i].id))
				arg = mode_changes[i].id;
			else
				arg = mode_changes[i].arg;

			if(arg)
			{
				arglen = strlen(arg);

				/* dont even think about it! --fl */
				if(arglen > MODEBUFLEN - 5)
					continue;
			}

			/* if we're creeping past the buf size, we need to send it and make
			 * another line for the other modes
			 * XXX - this could give away server topology with uids being
			 * different lengths, but not much we can do, except possibly break
			 * them as if they were the longest of the nick or uid at all times,
			 * which even then won't work as we don't always know the uid -A1kmm.
			 */
			if(arg && ((mc == MAXMODEPARAMSSERV) ||
				   ((mbl + pbl + arglen + 4) > (BUFSIZE - 3))))
			{
				if(nc != 0)
					sendto_server(client_p, chptr, cap, nocap,
						      "%s %s", modebuf, parabuf);
				nc = 0;
				mc = 0;

				mbl = preflen;
				pbl = 0;
				pbuf = parabuf;
				parabuf[0] = 0;
				dir = MODE_QUERY;
			}

			if(dir != mode_changes[i].dir)
			{
				modebuf[mbl++] = (mode_changes[i].dir == MODE_ADD) ? '+' : '-';
				dir = mode_changes[i].dir;
			}

			modebuf[mbl++] = mode_changes[i].letter;
			modebuf[mbl] = 0;
			nc++;

			if(arg != NULL)
			{
				len = rb_sprintf(pbuf, "%s ", arg);
				pbuf += len;
				pbl += len;
				mc++;
			}
		}

		if(pbl && parabuf[pbl - 1] == ' ')
			parabuf[pbl - 1] = 0;

		if(nc != 0)
			sendto_server(client_p, chptr, cap, nocap, "%s %s", modebuf, parabuf);
	}
}

void 
resv_chan_forcepart(const char *name, const char *reason, int temp_time)
{
	rb_dlink_node *ptr;
	rb_dlink_node *next_ptr;
	struct Channel *chptr;
	struct membership *msptr;
	struct Client *target_p;

	if(!ConfigChannel.resv_forcepart)
		return;

	/* for each user on our server in the channel list
	 * send them a PART, and notify opers.
	 */
	chptr = find_channel(name);
	if(chptr != NULL)
	{
		RB_DLINK_FOREACH_SAFE(ptr, next_ptr, chptr->locmembers.head)
		{
			msptr = ptr->data;
			target_p = msptr->client_p;

			if(IsExemptResv(target_p))
				continue;

			sendto_server(target_p, chptr, CAP_TS6, NOCAPS,
			              ":%s PART %s", target_p->id, chptr->chname);

			sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@%s PART %s :%s",
			                     target_p->name, target_p->username,
			                     target_p->host, chptr->chname, target_p->name);

			remove_user_from_channel(msptr);

			/* notify opers & user they were removed from the channel */
			sendto_realops_snomask(SNO_GENERAL, L_ALL,
			                     "Forced PART for %s!%s@%s from %s (%s)",
			                     target_p->name, target_p->username, 
			                     target_p->host, name, reason);

			if(temp_time > 0)
				sendto_one_notice(target_p, ":*** Channel %s is temporarily unavailable on this server.",
				           name);
			else
				sendto_one_notice(target_p, ":*** Channel %s is no longer available on this server.",
				           name);
		}
	}
}

/* Check what we will forward to, without sending any notices to the user
 * -- jilles
 */
struct Channel *
check_forward(struct Client *source_p, struct Channel *chptr,
		char *key)
{
	int depth = 0, i;

	/* User is +Q */
	if (IsNoForward(source_p))
		return NULL;

	while (depth < 16)
	{
		chptr = find_channel(chptr->mode.forward);
		/* Can only forward to existing channels */
		if (chptr == NULL)
			return NULL;
		/* Already on there, show original error message */
		if (IsMember(source_p, chptr))
			return NULL;
		/* Juped. Sending a warning notice would be unfair */
		if (hash_find_resv(chptr->chname))
			return NULL;
		/* Don't forward to +Q channel */
		if (chptr->mode.mode & MODE_DISFORWARD)
			return NULL;
		i = can_join(source_p, chptr, key);
		if (i == 0)
			return chptr;
		if (i != ERR_INVITEONLYCHAN && i != ERR_NEEDREGGEDNICK && i != ERR_THROTTLE && i != ERR_CHANNELISFULL)
			return NULL;
		depth++;
	}

	return NULL;
}

/*
 * do_join_0
 *
 * inputs	- pointer to client doing join 0
 * output	- NONE
 * side effects	- Use has decided to join 0. This is legacy
 *		  from the days when channels were numbers not names. *sigh*
 */
void
do_join_0(struct Client *client_p, struct Client *source_p)
{
	struct membership *msptr;
	struct Channel *chptr = NULL;
	rb_dlink_node *ptr;

	/* Finish the flood grace period... */
	if(MyClient(source_p) && !IsFloodDone(source_p))
		flood_endgrace(source_p);

	sendto_server(client_p, NULL, CAP_TS6, NOCAPS, ":%s JOIN 0", use_id(source_p));

	while((ptr = source_p->user->channel.head))
	{
	if(source_p->user->channel.head && MyConnect(source_p) &&
	   !IsOper(source_p) && !IsExemptSpambot(source_p))
		check_spambot_warning(source_p, NULL);

		msptr = ptr->data;
		chptr = msptr->chptr;
		sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@%s PART %s",
				     source_p->name,
				     source_p->username, source_p->host, chptr->chname);
		remove_user_from_channel(msptr);
	}
}

int
check_channel_name_loc(struct Client *source_p, const char *name)
{
	const char *p;

	s_assert(name != NULL);
	if(EmptyString(name))
		return 0;

	if(ConfigFileEntry.disable_fake_channels && !IsOper(source_p))
	{
		for(p = name; *p; ++p)
		{
			if(!IsChanChar(*p) || IsFakeChanChar(*p))
				return 0;
		}
	}
	else
	{
		for(p = name; *p; ++p)
		{
			if(!IsChanChar(*p))
				return 0;
		}
	}

	if(ConfigChannel.only_ascii_channels)
    {
    	for(p = name; *p; ++p)
			if(*p < 33 || *p > 126)
    			return 0;
    }


	return 1;
}

void user_join(struct Client * client_p, struct Client * source_p, const char * channels, const char * keys)
{
	static char jbuf[BUFSIZE];
	struct Channel *chptr = NULL;
	struct ConfItem *aconf;
	char *name;
	char *key = NULL;
	const char *modes;
	int i, flags = 0;
	char *p = NULL, *p2 = NULL;
	char *chanlist;
	char *mykey;

	jbuf[0] = '\0';

	if(channels == NULL)
		return;

	/* rebuild the list of channels theyre supposed to be joining.
	 * this code has a side effect of losing keys, but..
	 */
	chanlist = LOCAL_COPY(channels);
	for(name = rb_strtok_r(chanlist, ",", &p); name; name = rb_strtok_r(NULL, ",", &p))
	{
		/* check the length and name of channel is ok */
		if(!check_channel_name_loc(source_p, name) || (strlen(name) > LOC_CHANNELLEN))
		{
			sendto_one_numeric(source_p, ERR_BADCHANNAME,
					   form_str(ERR_BADCHANNAME), (unsigned char *) name);
			continue;
		}

		/* join 0 parts all channels */
		if(*name == '0' && (name[1] == ',' || name[1] == '\0') && name == chanlist)
		{
			(void) strcpy(jbuf, "0");
			continue;
		}

		/* check it begins with # or &, and local chans are disabled */
                else if(!IsChannelName(name) ||
                        ( !ConfigChannel.use_local_channels && name[0] == '&'))
		{
			sendto_one_numeric(source_p, ERR_NOSUCHCHANNEL,
					   form_str(ERR_NOSUCHCHANNEL), name);
			continue;
		}

		/* see if its resv'd */
		if(!IsExemptResv(source_p) && (aconf = hash_find_resv(name)))
		{
			sendto_one_numeric(source_p, ERR_BADCHANNAME,
					   form_str(ERR_BADCHANNAME), name);

			/* dont warn for opers */
			if(!IsExemptJupe(source_p) && !IsOper(source_p))
				sendto_realops_snomask(SNO_SPY, L_NETWIDE,
						     "User %s (%s@%s) is attempting to join locally juped channel %s (%s)",
						     source_p->name, source_p->username,
						     source_p->orighost, name, aconf->passwd);
			/* dont update tracking for jupe exempt users, these
			 * are likely to be spamtrap leaves
			 */
			else if(IsExemptJupe(source_p))
				aconf->port--;

			continue;
		}

		if(splitmode && !IsOper(source_p) && (*name != '&') &&
		   ConfigChannel.no_join_on_split)
		{
			sendto_one(source_p, form_str(ERR_UNAVAILRESOURCE),
				   me.name, source_p->name, name);
			continue;
		}

		if(*jbuf)
			(void) strcat(jbuf, ",");
		(void) rb_strlcat(jbuf, name, sizeof(jbuf));
	}

	if(keys != NULL)
	{
		mykey = LOCAL_COPY(keys);
		key = rb_strtok_r(mykey, ",", &p2);
	}

	for(name = rb_strtok_r(jbuf, ",", &p); name;
	    key = (key) ? rb_strtok_r(NULL, ",", &p2) : NULL, name = rb_strtok_r(NULL, ",", &p))
	{
		hook_data_channel_activity hook_info;

		/* JOIN 0 simply parts all channels the user is in */
		if(*name == '0' && !atoi(name))
		{
			if(source_p->user->channel.head == NULL)
				continue;

			do_join_0(&me, source_p);
			continue;
		}

		/* look for the channel */
		if((chptr = find_channel(name)) != NULL)
		{
			if(IsMember(source_p, chptr))
				continue;

			flags = 0;
		}
		else
		{
			hook_data_client_approval moduledata;

			moduledata.client = source_p;
			moduledata.approved = 0;

			call_hook(h_can_create_channel, &moduledata);

			if(moduledata.approved != 0)
			{
				sendto_one(source_p, form_str(moduledata.approved),
					   me.name, source_p->name, name);
				continue;
			}

			if(splitmode && !IsOper(source_p) && (*name != '&') &&
			   ConfigChannel.no_create_on_split)
			{
				sendto_one(source_p, form_str(ERR_UNAVAILRESOURCE),
					   me.name, source_p->name, name);
				continue;
			}

			flags = CHFL_CHANOP;
		}

		if((rb_dlink_list_length(&source_p->user->channel) >=
		    (unsigned long) ConfigChannel.max_chans_per_user) &&
		   (!IsOper(source_p) ||
		    (rb_dlink_list_length(&source_p->user->channel) >=
		     (unsigned long) ConfigChannel.max_chans_per_user * 3)))
		{
			sendto_one(source_p, form_str(ERR_TOOMANYCHANNELS),
				   me.name, source_p->name, name);
			return;
		}

		if(chptr == NULL)	/* If I already have a chptr, no point doing this */
		{
			chptr = get_or_create_channel(source_p, name, NULL);

			if(chptr == NULL)
			{
				sendto_one(source_p, form_str(ERR_UNAVAILRESOURCE),
					   me.name, source_p->name, name);
				continue;
			}
		}

		/* can_join checks for +i key, bans etc */
		if((i = can_join(source_p, chptr, key)))
		{
			if(IsOverride(source_p))
			{
				sendto_wallops_flags(UMODE_WALLOP, &me,
						"%s is overriding JOIN to [%s]",
					get_oper_name(source_p), chptr->chname);
				sendto_server(NULL, chptr, NOCAPS, NOCAPS,
						":%s WALLOPS :%s is overriding JOIN to [%s]",
						me.name, get_oper_name(source_p), chptr->chname);
			}
			else if ((i != ERR_NEEDREGGEDNICK && i != ERR_THROTTLE && i != ERR_INVITEONLYCHAN && i != ERR_CHANNELISFULL) ||
			    (!ConfigChannel.use_forward || (chptr = check_forward(source_p, chptr, key)) == NULL))
			{
				/* might be wrong, but is there any other better location for such?
				 * see extensions/chm_operonly.c for other comments on this
				 * -- dwr
				 */
				if(i != ERR_CUSTOM)
					sendto_one(source_p, form_str(i), me.name, source_p->name, name);

				continue;
			}
			else
				sendto_one_numeric(source_p, ERR_LINKCHANNEL, form_str(ERR_LINKCHANNEL), name, chptr->chname);
		}
		
		if(flags == 0 &&
     					!IsOper(source_p) && !IsExemptSpambot(source_p))
				check_spambot_warning(source_p, name);

		/* add the user to the channel */
		add_user_to_channel(chptr, source_p, flags);
		if (chptr->mode.join_num &&
			rb_current_time() - chptr->join_delta >= chptr->mode.join_time)
		{
			chptr->join_count = 0;
			chptr->join_delta = rb_current_time();
		}
		chptr->join_count++;

		/* we send the user their join here, because we could have to
		 * send a mode out next.
		 */
		sendto_channel_local(ALL_MEMBERS, chptr, ":%s!%s@%s JOIN :%s",
				     source_p->name,
				     source_p->username, source_p->host, chptr->chname);

		/* its a new channel, set +nt and burst. */
		if(flags & CHFL_CHANOP)
		{
			chptr->channelts = rb_current_time();

			/* autochanmodes stuff */
			if(ConfigChannel.autochanmodes)
			{
				char * ch;
				for(ch = ConfigChannel.autochanmodes; *ch; *ch++)
				{
					chptr->mode.mode |= chmode_table[*ch].mode_type;
				}
			}
			else
			{
				chptr->mode.mode |= MODE_TOPICLIMIT;
				chptr->mode.mode |= MODE_NOPRIVMSGS;
			}

			modes = channel_modes(chptr, &me);

			sendto_channel_local(ONLY_CHANOPS, chptr, ":%s MODE %s %s",
					     me.name, chptr->chname, modes);

			sendto_server(client_p, chptr, CAP_TS6, NOCAPS,
				      ":%s SJOIN %ld %s %s :@%s",
				      me.id, (long) chptr->channelts,
				      chptr->chname, modes, source_p->id);
		}
		else
		{
			sendto_server(client_p, chptr, CAP_TS6, NOCAPS,
				      ":%s JOIN %ld %s +",
				      use_id(source_p), (long) chptr->channelts,
				      chptr->chname);
		}

		del_invite(chptr, source_p);

		if(chptr->topic != NULL)
		{
			sendto_one(source_p, form_str(RPL_TOPIC), me.name,
				   source_p->name, chptr->chname, chptr->topic);

			sendto_one(source_p, form_str(RPL_TOPICWHOTIME),
				   me.name, source_p->name, chptr->chname,
				   chptr->topic_info, chptr->topic_time);
		}

		channel_member_names(chptr, source_p, 1);

		hook_info.client = source_p;
		hook_info.chptr = chptr;
		hook_info.key = key;
		call_hook(h_channel_join, &hook_info);
	}

	return;
}

/*
 * channel_metadata_add
 * 
 * inputs	- pointer to channel struct
 *		- name of metadata item you wish to add
 *		- value of metadata item
 *		- 1 if metadata should be propegated, 0 if not
 * output	- none
 * side effects - metadata is added to the channel in question
 *		- metadata is propegated if propegate is set.
 */
struct Metadata *
channel_metadata_add(struct Channel *target, const char *name, const char *value, int propegate)
{
	struct Metadata *md;

	md = rb_malloc(sizeof(struct Metadata));
	md->name = rb_strdup(name);
	md->value = rb_strdup(value);

	irc_dictionary_add(target->metadata, md->name, md);
	
	if(propegate)
		sendto_match_servs(&me, "*", CAP_ENCAP, NOCAPS, "ENCAP * METADATA ADD %s %s :%s",
				target->chname, name, value);

	return md;
}

/*
 * channel_metadata_time_add
 * 
 * inputs	- pointer to channel struct
 *		- name of metadata item you wish to add
 *		- time_t you wish to add
 *		- value you wish to add
 * output	- none
 * side effects - metadata is added to the channel in question
 */
struct Metadata *
channel_metadata_time_add(struct Channel *target, const char *name, time_t timevalue, const char *value)
{
	struct Metadata *md;

	md = rb_malloc(sizeof(struct Metadata));
	md->name = rb_strdup(name);
	md->value = rb_strdup(value);
	md->timevalue = timevalue;

	irc_dictionary_add(target->metadata, md->name, md);

	return md;
}

/*
 * channel_metadata_delete
 * 
 * inputs	- pointer to channel struct
 *		- name of metadata item you wish to delete
 * output	- none
 * side effects - metadata is deleted from the channel in question
 * 		- deletion is propegated if propegate is set
 */
void
channel_metadata_delete(struct Channel *target, const char *name, int propegate)
{
	struct Metadata *md = channel_metadata_find(target, name);

	if(!md)
		return;

	irc_dictionary_delete(target->metadata, md->name);

	rb_free(md);

	if(propegate)
		sendto_match_servs(&me, "*", CAP_ENCAP, NOCAPS, "ENCAP * METADATA DELETE %s %s",
				target->chname, name);
}

/*
 * channel_metadata_find
 * 
 * inputs	- pointer to channel struct
 *		- name of metadata item you wish to read
 * output	- the requested metadata, if it exists, elsewise null.
 * side effects - 
 */
struct Metadata *
channel_metadata_find(struct Channel *target, const char *name)
{
	if(!target)
		return NULL;

	if(!target->metadata)
		return NULL;

	return irc_dictionary_retrieve(target->metadata, name);
}

/*
 * channel_metadata_clear
 * 
 * inputs	- pointer to channel struct
 * output	- none
 * side effects - metadata is cleared from the channel in question
 */
void
channel_metadata_clear(struct Channel *chptr)
{
	struct Metadata *md;
	struct DictionaryIter iter;
	
	DICTIONARY_FOREACH(md, &iter, chptr->metadata)
	{
		channel_metadata_delete(chptr, md->name, 0);
	}
}
