/*
 * Elemental-IRCd
 * chm_nocaps: blocks capsing messages (+G mode).
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

static unsigned int mode_nocaps;

static void chm_nocaps_process(hook_data_privmsg_channel *);

mapi_hfn_list_av1 chm_nocaps_hfnlist[] = {
    { "privmsg_channel", (hookfn) chm_nocaps_process },
    { NULL, NULL }
};

static void
chm_nocaps_process(hook_data_privmsg_channel *data)
{
    char text2[BUFSIZE];
    size_t contor;
    int caps = 0;
    int len = 0;

    struct membership *msptr = find_channel_membership(data->chptr, data->source_p);

    /* don't waste CPU if message is already blocked */
    if (data->approved) {
        return;
    }

    if (strlen(data->text) > 10 &&
        data->chptr->mode.mode & mode_nocaps &&
        (!ConfigChannel.exempt_cmode_G || !is_any_op(msptr))) {
        rb_strlcpy(text2, data->text, BUFSIZE);
        strip_unprintable(text2);

        // Don't count the "ACTION" part of action as part of the message --SnoFox
        if (data->msgtype != MESSAGE_TYPE_NOTICE && *data->text == '\001' &&
            !strncasecmp(data->text + 1, "ACTION ", 7)) {
            contor = 7;
        } else {
            contor = 0;
        }
        for(; contor < strlen(text2); contor++) {
            if(IsUpper(text2[contor]) && !isdigit(text2[contor]))
                caps++;
            len++;
        }
        /* Added divide by 0 check --alxbl */
        if(len != 0 && ((caps*100)/(len)) >= 50) {
            sendto_one_numeric(data->source_p, ERR_CANNOTSENDTOCHAN,
                               form_str(ERR_CANNOTSENDTOCHAN),
                               data->chptr->chname,
                               "your message contains mostly capital letters (+G set)");
            return;
        }
    }
}

static int
_modinit(void)
{
    mode_nocaps = cflag_add('G', chm_simple);
    if (mode_nocaps == 0)
        return -1;

    return 0;
}

static void
_moddeinit(void)
{
    cflag_orphan('G');
}

DECLARE_MODULE_AV1(chm_nocaps, _modinit, _moddeinit, NULL, NULL, chm_nocaps_hfnlist, "$Revision$");
