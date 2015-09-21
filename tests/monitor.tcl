begin {Monitor tests}

client watcher

set joiner_nick [get_nick]

watcher >> MONITOR + $joiner_nick
watcher << RPL_MONOFFLINE * $joiner_nick

client joiner -nick $joiner_nick
watcher << RPL_MONONLINE * "${joiner_nick}!*@*"

watcher >> MONITOR S
        << RPL_MONONLINE * "${joiner_nick}!*@*"

joiner  >> QUIT
watcher << RPL_MONOFFLINE * $joiner_nick

watcher >> MONITOR S
        << RPL_MONOFFLINE * $joiner_nick

watcher >> MONITOR L
        << RPL_MONLIST * $joiner_nick
        << RPL_ENDOFMONLIST *
