begin {Monitor tests}

client watcher

set joiner_nick [get_nick]

# Adding a nick while the target is offline
watcher >> MONITOR + $joiner_nick
watcher << RPL_MONOFFLINE * $joiner_nick

client joiner -nick $joiner_nick
watcher << RPL_MONONLINE * [joiner hostmask]

watcher >> MONITOR S
        << RPL_MONONLINE * [joiner hostmask]

# Adding a nick while the target is online
watcher >> MONITOR - $joiner_nick
        >> MONITOR + $joiner_nick
        << RPL_MONONLINE * [joiner hostmask]

joiner  >> QUIT
watcher << RPL_MONOFFLINE * $joiner_nick

watcher >> MONITOR S
        << RPL_MONOFFLINE * $joiner_nick

watcher >> MONITOR L
        << RPL_MONLIST * $joiner_nick
        << RPL_ENDOFMONLIST *
