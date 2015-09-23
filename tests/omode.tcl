begin {Check OMODE changes}

set test_channel #omode

client oper

oper :
    # set up m_omode.so
    >> OPER god testsuite
    >> MODLOAD extensions/m_omode.so
    << NOTICE * "*** Notice -- Module m_omode.so \[version: \$Revision\$; MAPI version: 1\] loaded at"

    >> MODE [oper nick] +p
    << MODE [oper nick] +p
    >> MODE $test_channel +h [oper nick]
    << MODE $test_channel +h [oper nick]
    >> OMODE $test_channel +y [oper nick]
    << MODE $test_channel +y [oper nick]

    >> MODUNLOAD m_omode.so
