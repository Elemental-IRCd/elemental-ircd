begin {Check chghost}

set test_channel #chghost

client oper
>> OPER oper testsuite

client target
client observer
client observerv3 -caps {chghost}

oper :
 >> CHGHOST [target nick] chghost.test
 << QUIT {Changing host}
 << ":[target nick]!*@chghost.test" JOIN $test_channel

observer :
 << QUIT {Changing host}
 << ":[target nick]!*@chghost.test" JOIN $test_channel

observerv3 :
if {[have chghost]} {
 << ":[target nick]!*@*" CHGHOST * chghost.test
}

target :
 << $RPL_HOSTHIDDEN [target nick] chghost.test "*is now your hidden host*"
