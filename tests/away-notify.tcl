begin {away-notify Capability}

set test_channel #away-notify

client listener -caps away-notify

if {![have away-notify]} {
    skip "No away-notify support"
}

client leaver

set message "[listener nick] should receive an away-notify"
# Should see on set with common channel
leaver   >> AWAY $message
listener << AWAY $message

# Should see on join
leaver   >> PART $test_channel
leaver   >> JOIN $test_channel
listener << AWAY $message

# And when unset
leaver   >> AWAY
listener << AWAY
