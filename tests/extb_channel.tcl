begin {tests channel-based extbans}

skip {something is broken with this}

set test_channel #extb_channel
set target_channel #extb_channel_ops

client chanop
    >> JOIN $target_channel
    << JOIN $target_channel

client foo
    >> JOIN $target_channel
    << JOIN $target_channel

client bar
    >> JOIN $target_channel
    << JOIN $target_channel

chanop :
    >> MODE $target_channel +i
    << MODE $target_channel +i
    after 500 ; #hack
    >> MODE $target_channel +I "\$c:#extb_channel"
    << MODE $target_channel +I "\$c:#extb_channel"

after 500 ; #hack

client peon
    >> JOIN $target_channel
    << JOIN $target_channel
