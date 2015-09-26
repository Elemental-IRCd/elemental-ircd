begin {Extban based on account names}

skip {Services logins are not yet functional}

set test_channel #extb_account

client chanop
    >> MODE $test_channel +b {$a:foobar}

client peon
    >> NS id foobar baz
    << ERR_BANNEDFROMCHAN
