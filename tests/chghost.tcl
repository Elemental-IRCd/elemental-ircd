begin test chghost {Check chghost}

set test_channel #chghost

client oper
>> OPER oper testsuite

client target
client observer

oper     >> CHGHOST [target nick] chghost.test
         << QUIT {Changing host}
         >> "[target nick]!*@chghost.test" JOIN $test_channel

observer << QUIT {Changing host}
         >> "[target nick]!*@chghost.test" JOIN $test_channel

target   << $RPL_HOSTHIDDEN [target nick] chghost.test "*is now your hidden host*"
