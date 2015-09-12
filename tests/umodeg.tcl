source lib/lib.tcl

begin test umodeg {Check umode +g functionality and presence of help msg}

client umodegtarget hub
client umodegsender hub

set message {This message is sent unencumbered}
umodegsender send_cmd PRIVMSG umodegtarget $message
umodegtarget expect  ":$message"

umodegtarget send_cmd MODE umodegtarget +g

set message {This message will be filtered by +g}
umodegsender send_cmd PRIVMSG umodegtarget $message
umodegtarget expect "is messaging you"
umodegtarget expect "/ACCEPT"
umodegsender expect "has been informed that you messaged them."

umodegsender quit
umodegtarget quit
