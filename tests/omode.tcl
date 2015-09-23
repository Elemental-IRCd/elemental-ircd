begin {Check OMODE changes}

set test_channel #omode

client oper

oper :
    >> OPER god testsuite
    >> MODLOAD extensions/m_omode.so
    << omode.so

    >> MODE [oper nick] +p
    << MODE
    >> MODE $test_channel +h [oper nick]
    << MODE
    >> OMODE $test_channel +y [oper nick]
    << MODE
