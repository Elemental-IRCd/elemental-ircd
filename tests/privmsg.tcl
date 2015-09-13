begin test privmsg {Check privmsg works and is routed}

set test_channel #privmsg

client client1
client client2

set message "This is sent by [client1 nick], [client2 nick] should see it"
client1 >> PRIVMSG $test_channel $message
client2 << PRIVMSG $test_channel $message

set message "This is sent by [client2 nick], [client1 nick] should see it"
client2 >> PRIVMSG $test_channel $message
client1 << PRIVMSG $test_channel $message

client client3
client client4

set message "This is sent by [client3 nick], everyone else should see it"
client3 >> PRIVMSG $test_channel $message
client1 << PRIVMSG $test_channel $message
client2 << PRIVMSG $test_channel $message
client4 << PRIVMSG $test_channel $message

set message "This is sent by [client4 nick], everyone else should see it"
client4 >> PRIVMSG $test_channel $message
client1 << PRIVMSG $test_channel $message
client2 << PRIVMSG $test_channel $message
client3 << PRIVMSG $test_channel $message
