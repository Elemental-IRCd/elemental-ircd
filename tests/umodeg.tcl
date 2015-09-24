begin {Check umode +g functionality and presence of help msg}

set test_channel #umodeg

client target
client sender

set message {This message is sent unencumbered}
sender >> PRIVMSG [target nick] $message
target << PRIVMSG [target nick] $message

target >> MODE [target nick] +g
target << MODE [target nick] +g

sender >> PRIVMSG [target nick] {This message will be filtered by +g}
target << RPL_UMODEGMSG [target nick] [sender nick] * "*is messaging you*+g*/ACCEPT [sender nick]*"
