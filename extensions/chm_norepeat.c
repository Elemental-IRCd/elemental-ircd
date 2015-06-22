/*
 * Elemental-IRCd
 * chm_norepeat: blocks repeating messages (+K mode).
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
#include "inline/stringops.h"

static unsigned int mode_norepeat;

static void chm_norepeat_process(hook_data_privmsg_channel *);

mapi_hfn_list_av1 chm_norepeat_hfnlist[] = {
    { "privmsg_channel", (hookfn) chm_norepeat_process },
    { NULL, NULL }
};

static void
chm_norepeat_process(hook_data_privmsg_channel *data)
{
    char text2[BUFSIZE];
    struct Metadata *md;
    struct membership *msptr = find_channel_membership(data->chptr, data->source_p);

    /* don't waste CPU if message is already blocked */
    if (data->approved) {
        return;
    }

    if(data->chptr->mode.mode & mode_norepeat) {
        rb_strlcpy(text2, data->text, BUFSIZE);
        strip_unprintable(text2);
        md = channel_metadata_find(data->chptr, "NOREPEAT");

        if(md && (!ConfigChannel.exempt_cmode_K || !is_any_op(msptr))) {
            if(!(strcmp(md->value, text2))) {
                if(data->msgtype != MESSAGE_TYPE_NOTICE) {
                    sendto_one_numeric(data->source_p, ERR_CANNOTSENDTOCHAN,
                                       "%s :Cannot send to channel - Message blocked due to repeating (+K set)",
                                       data->chptr->chname);
                    data->approved = ERR_CANNOTSENDTOCHAN;
                }
            }
        }

        channel_metadata_delete(data->chptr, "NOREPEAT", 0);
        channel_metadata_add(data->chptr, "NOREPEAT", text2, 0);
    }
}

static int
_modinit(void)
{
    mode_norepeat = cflag_add('K', chm_simple);
    if (mode_norepeat == 0)
        return -1;

    return 0;
}

static void
_moddeinit(void)
{
    cflag_orphan('K');
}

DECLARE_MODULE_AV1(chm_norepeat, _modinit, _moddeinit, NULL, NULL, chm_norepeat_hfnlist, "$Revision$");
