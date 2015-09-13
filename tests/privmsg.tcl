begin test privmsg {Check privmsg works and is routed}

client client1
>> JOIN #privmsg

client client2
>> JOIN #privmsg

set message "This is sent by [client1 nick], [client2 nick] should see it"
client1 >> PRIVMSG #privmsg $message
client2 << PRIVMSG #privmsg $message

set message "This is sent by [client2 nick], [client1 nick] should see it"
client2 >> PRIVMSG #privmsg $message
client1 << PRIVMSG #privmsg $message

client client3
>> JOIN #privmsg

client client4
>> JOIN #privmsg

set message "This is sent by [client3 nick], everyone else should see it"
client3 >> PRIVMSG #privmsg $message
client1 << PRIVMSG #privmsg $message
client2 << PRIVMSG #privmsg $message
client4 << PRIVMSG #privmsg $message

set message "This is sent by [client4 nick], everyone else should see it"
client4 >> PRIVMSG #privmsg $message
client1 << PRIVMSG #privmsg $message
client2 << PRIVMSG #privmsg $message
client3 << PRIVMSG #privmsg $message
