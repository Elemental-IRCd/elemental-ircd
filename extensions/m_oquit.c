/*
 *  elemental-ircd: A slightly useful ircd.
 *  m_oquit.c: Allows an oper to quit from IRC with no quit prefix.
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2002-2005 ircd-ratbox development team
 *  Copyright (C) 2015 elemental-ircd development team
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
 */

#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"
#include "inline/stringops.h"

static int m_oquit(struct Client *, struct Client *, int, const char **);

struct Message oquit_msgtab = {
    "OQUIT", 0, 0, 0, MFLG_SLOW | MFLG_UNREG,
    {{m_oquit, 0}, {m_oquit, 0}, mg_ignore, mg_ignore, mg_ignore, {m_oquit, 0}}
};

mapi_clist_av1 oquit_clist[] = { &oquit_msgtab, NULL };

DECLARE_MODULE_AV1(oquit, NULL, NULL, oquit_clist, NULL, NULL, "$Revision: 1333 $");

/*
** m_oquit
**      parv[1] = comment
*/
static int
m_oquit(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
    char *comment = LOCAL_COPY((parc > 1 && parv[1]) ? parv[1] : client_p->name);
    char reason[REASONLEN + 1];

    source_p->flags |= FLAGS_NORMALEX;

    if(strlen(comment) > (size_t) REASONLEN)
        comment[REASONLEN] = '\0';

    if(!IsOper(source_p)) {
        sendto_one(source_p, form_str(ERR_NOPRIVS), me.name, source_p->name, "oquit");
        return 0;
    }

    exit_client(client_p, source_p, source_p, comment);

    return 0;
}
