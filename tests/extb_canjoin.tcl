begin {tests extb_canjoin.so ($j:extban)}

set test_channel #extb_canjoin
set target_channel #extb_canjoin_ops

client chanop
    >> JOIN $target_channel
    << JOIN $target_channel
    after 500
    >> MODE $test_channel +b "\$~j:#extb_canjoin_ops"
    << MODE $test_channel +b

client peon
    << JOIN $test_channel
