begin {Test for a regression in issue #164}

set test_channel #test164

client oper
client peon
client otherpeon

oper :
    >> OPER god testsuite
    << $RPL_YOUREOPER
    >> MODE [oper nick] +p
    << MODE [oper nick] +p
    >> MODE $test_channel +a [peon nick]
    << MODE $test_channel +a [peon nick]
    >> MODE $test_channel +y [otherpeon nick]
    << MODE $test_channel +y [otherpeon nick]
