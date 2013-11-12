## Elemental-ircd specific deviations from the normal TS6 protocol

As a general rule of thumb and for compatibility reasons, protocol changes are
made as infrequently and minimally as possible. However there are some things
that have been changed in how elemental handles the server to server protocol
that are worth mentioning.

For readability's sake, any lines that a server sends are prefixed by `>>>` and
any lines it expects to recieve are prefixed by `<<<`.

### Changes

#### Channel owner mode

If enabled, a new mode will be added to the channel prefix list. This mode (+y)
has a prefix of "~" and in most IRC services packages is used to signify the
owner of a specific channel.  If enabled, it will show up in SJOIN lines as such:

    >>> :[sid] SJOIN [channel-ts] #opers + :~[ts6-uid]

A more practical example would be:

    >>> :420 SJOIN 13840003504 #opers + :~@42000000A

As the mode for this channel is `+y`, all `TMODE` changes adding or removing `+y`
from a client in a channel should set and unset this mode accordingly. An example
of adding the mode follows:

    >>> :47GAAAAAB TMODE 13840003504 #opers +y 42000000A

Handling removal is similar.

Currently, the only way to detect if a remote server supports these extended
modes is to remotely query a server's `VERSION` and read out the `005` reply.

### Things added to existing commands

#### Server notice mask broadcasts

Currently the NEEDSASL, DNSBL hit, hunted command flooding and `m_post` rejections
are globally broadcast server notices.  They will show up in TS6 lines such as:

    <<< :[sid] ENCAP * SNOTE [snomask] :[server name] :[message]

For a more practical example:

    <<< :47G ENCAP * SNOTE b :rarity.shadownet.int :Client floodbot (~a@127.0.0.1) is attempting to flood ADMIN

These broadcasts are there for IRC operators on more centrally managed networks
to better see what is going on on other ends of the network without having to hop
servers and check.  Bots or pseudoservices may also uses these lines to perform
additional actions (such as `AKILL`s or logging to channels) as needed by the
bot author.

