begin {Test echo-message}

set test_channel #echo-message

client talker -caps echo-message

if {![have echo-message]} {
    skip {No echo-message support}
}

client listener

set message "[listener nick] should receive this, as well as [talker nick]"
talker   >> PRIVMSG [listener nick] $message
         << PRIVMSG [listener nick] $message
listener << PRIVMSG [listener nick] $message

set message "${test_channel} should see this, as should [talker nick]"
talker   >> PRIVMSG $test_channel $message
         << PRIVMSG $test_channel $message
listener << PRIVMSG $test_channel $message

set message "[listener nick] should receive this, as well as [talker nick]"
talker   >> NOTICE [listener nick] $message
         << NOTICE [listener nick] $message
listener << NOTICE [listener nick] $message

set message "${test_channel} should see this, as should [talker nick]"
talker   >> NOTICE $test_channel $message
         << NOTICE $test_channel $message
listener << NOTICE $test_channel $message

