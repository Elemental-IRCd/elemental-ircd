source lib/lib.tcl

begin test umodeg {Check umode +g functionality and presence of help msg}

client umodegtarget hub
client umodegsender hub

umodegtarget join_channel #privmsg
umodegsender join_channel #privmsg

set message {This message is sent unencumbered}
umodegsender send_cmd PRIVMSG umodegtarget $message
umodegtarget expect  "PRIVMSG umodegtarget :$message"

umodegtarget send_cmd MODE umodegtarget +g

set message {This message will be filtered by +g}
umodegsender send_cmd PRIVMSG umodegtarget $message
umodegtarget expect "is messaging you"
umodegtarget expect "ACCEPT umodegsender"
umodegsender expect "has been informed that you messaged them."

umodegsender quit
umodegtarget quit
