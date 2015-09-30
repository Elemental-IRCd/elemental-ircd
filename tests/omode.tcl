begin {Check OMODE changes}

set test_channel #omode

client myoper

myoper :
    # set up m_omode.so
    oper god
    << RPL_YOUREOPER
    >> MODLOAD extensions/m_omode.so

    # sleep a little just in case
    after 125

    >> MODE [myoper nick] +p
    << MODE [myoper nick] +p
    >> MODE $test_channel +h [myoper nick]
    << MODE $test_channel +h [myoper nick]
    >> OMODE $test_channel +y [myoper nick]
    << MODE $test_channel +y [myoper nick]

    >> MODUNLOAD m_omode.so
