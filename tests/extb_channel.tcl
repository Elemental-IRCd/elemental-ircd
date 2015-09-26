begin {tests channel-based extbans}

set test_channel #extb_channel
set target_channel #extb_channel_ops

client chanop
    >> JOIN $target_channel
    << JOIN $target_channel
    >> MODE $target_channel +i
    << MODE $target_channel +i
    >> MODE $target_channel +I "\$c:$test_channel"
    << MODE $target_channel +I "\$c:$test_channel"

client peon
    >> JOIN $target_channel
    << JOIN $target_channel
