/*
 *
 * Elemental-IRCd: the ircd of the people
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

#include <stdbool.h>

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
#include "supported.h"
#include "inline/stringops.h"

static void block_invalid_utf8_process(hook_data_privmsg_channel *);

static int
is_utf8(const char* data)
{
    int remaining_continuations = 0;
    int i = 0;

    while (data[i]) {
        if(remaining_continuations > 0) {
            if((data[i] & 0x80) > 0 && (~data[i] & 0x40) > 0)
                remaining_continuations--;
            else return false;
        } else {
            if((data[i] & 0x80) > 0) {
                int ones = __builtin_clz(((int)~data[i]) & 0xFF) - __builtin_clz(0x000000FF);
                if(ones == 1 || ones > 4) return false;
                remaining_continuations = ones - 1;
            }
        }

        i++;
    }
    return remaining_continuations == 0;
}

mapi_hfn_list_av1 block_invalid_utf8_hfnlist[] = {
    { "privmsg_channel", (hookfn) block_invalid_utf8_process },
    { NULL, NULL }
};

static void
block_invalid_utf8_process(hook_data_privmsg_channel *data)
{
    /* don't waste CPU if message is already blocked */
    if (data->approved) {
        return;
    }

    if(!is_utf8(data->text)) {
        sendto_one_numeric(data->source_p,
                           ERR_CANNOTSENDTOCHAN,
                           form_str(ERR_CANNOTSENDTOCHAN),
                           data->chptr->chname,
                           "your message was badly formatted UTF-8 and this network enforces valid UTF-8");
        data->approved = ERR_CANNOTSENDTOCHAN;
        return;
    }
}

static int
_modinit(void)
{
    add_isupport("CHARSET", isupport_string, "UTF-8");

    return 0;
}

static void
_moddeinit(void)
{
    delete_isupport("CHARSET");
}

DECLARE_MODULE_AV1(block_invalid_utf8, _modinit, _moddeinit, NULL, NULL, block_invalid_utf8_hfnlist, "$Revision$");
