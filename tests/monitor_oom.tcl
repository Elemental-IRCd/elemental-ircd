begin {Test for monitor oom}

client watcher

set count 0
while {1} {
    set joiner_nick "monbug-$count"
    >> MONITOR + $joiner_nick
    << RPL_MONOFFLINE * $joiner_nick
    >> MONITOR - $joiner_nick
    incr count
}
