/*
 * elemental-ircd: it's a privilege, not a right
 * chm_nooperkick: block opers from being kicked (+M mode).
 *
 * Copyright (c) 2015 Christine Dodrill <xena@yolo-swag.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice is present in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "stdinc.h"
#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"
#include "s_conf.h"
#include "s_user.h"
#include "s_serv.h"
#include "numeric.h"
#include "chmode.h"

static unsigned int mode_nooperkick;

static void chm_nooperkick_process(hook_data_channel_approval *);

mapi_hfn_list_av1 chm_nooperkick_hfnlist[] = {
    { "can_kick", (hookfn) chm_nooperkick_process },
    { NULL, NULL }
};

static void
chm_nooperkick_process(hook_data_channel_approval *data)
{
    if (!data->approved) {
        return;
    }

    if (data->chptr->mode.mode & mode_nooperkick && IsOper(data->target)) {
        sendto_realops_snomask(SNO_GENERAL, L_NETWIDE,
                       "Overriding KICK from %s on %s in %s (channel is +M)",
                       data->client->name, data->target->name, data->chptr->chname);
        sendto_one_numeric(data->client, ERR_ISCHANSERVICE,
                       "%s %s :Cannot kick IRC operators from that channel.",
                       data->target->name, data->chptr->chname);
        data->approved = 0;
    }
}

static int
_modinit(void)
{
    mode_nooperkick = cflag_add('M', chm_hidden);
    if (mode_nooperkick == 0) {
        return -1;
    }

    return 0;
}

static void
_moddeinit(void)
{
    cflag_orphan('M');
}

DECLARE_MODULE_AV1(chm_nooperkick, _modinit, _moddeinit, NULL, NULL, chm_nooperkick_hfnlist, "$Revision$");
