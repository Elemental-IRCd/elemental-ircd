#include "stdinc.h"
#include "modules.h"
#include "client.h"
#include "hook.h"
#include "ircd.h"
#include "send.h"
#include "s_conf.h"
#include "s_newconf.h"
#include "hash.h"
#include "chmode.h"

/* Because I'm lazy we don't specify the letters we want in the config but directly here
 * Can be "+" for every snomask
 * Note: If chm_operonly is loaded, the channel has to be +O
 */
static const char snomasks[] = "+";
static const char channel[] = "#snomask";

static void h_cl_snomask(hook_data_snomask *);

mapi_hfn_list_av1 nl_clnlist[] = {
    { "on_snomask", (hookfn)h_cl_snomask },
    { NULL, NULL }
};

DECLARE_MODULE_AV1(m_chanlog, NULL, NULL, NULL, NULL, nl_clnlist, "$Revision$");

static void
h_cl_snomask(hook_data_snomask *hd)
{
    struct Channel *chan;
    buf_head_t linebuf;
    rb_dlink_node *ptr;
    buf_line_t *line;

    if (!*snomasks || strchr(snomasks, hd->flag))
        return;

    chan = find_channel(channel);
    if (!chan)
        return;

    if (findmodule_byname("chm_operonly.so") && !(chan->mode.mode & chmode_flags['O']))
        return;

    rb_linebuf_newbuf(&linebuf);
    rb_linebuf_putmsg(&linebuf, "%s", NULL,
        ":%s PRIVMSG %s :%s",
        me.name, chan->chname, hd->buf);

    RB_DLINK_FOREACH(ptr, linebuf.list.head) {
       sendto_channel_local(ALL_MEMBERS, chan, "%s", ptr->data);
    }
    rb_linebuf_donebuf(&linebuf);
}
