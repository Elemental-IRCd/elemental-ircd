#!/usr/bin/expect -f

source lib/lib.tcl

begin test privmsg {Check privmsg works and is routed}

set hub1  [client new hub]
set hub2  [client new hub]

$hub1 join_channel #privmsg
$hub2 join_channel #privmsg

set message {This is sent by hub1, hub2 should see it}
$hub1 send_cmd PRIVMSG #privmsg  $message
$hub2 expect  "PRIVMSG #privmsg :$message"

set message {This is sent by hub2, hub1 should see it}
$hub2 send_cmd PRIVMSG #privmsg  $message
$hub1 expect  "PRIVMSG #privmsg :$message"

set leaf1 [client new leaf1]
set leaf2 [client new leaf2]

$leaf1 join_channel #privmsg
$leaf2 join_channel #privmsg

# wait for joins to propagate
sleep 3

set message {This is sent by hub1, everyone else should see it}
$hub1  send_cmd PRIVMSG #privmsg  $message
$hub2  expect  "PRIVMSG #privmsg :$message"
$leaf1 expect  "PRIVMSG #privmsg :$message"
$leaf2 expect  "PRIVMSG #privmsg :$message"

set message {This is sent by leaf1, everyone else should see it}
$leaf1 send_cmd PRIVMSG #privmsg  $message
$hub1  expect  "PRIVMSG #privmsg :$message"
$hub2  expect  "PRIVMSG #privmsg :$message"
$leaf2 expect  "PRIVMSG #privmsg :$message"

$hub1 quit
$hub2 quit
$leaf1 quit
$leaf2 quit
