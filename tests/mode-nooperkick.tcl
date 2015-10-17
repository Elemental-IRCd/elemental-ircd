begin {test MODE_NOOPERKICK (+M)}

set test_channel #modenooperkick

client god -oper god
    >> MODE [god nick] +p
    << MODE [god nick] +p

    >> MODE $test_channel +o [god nick]
    << MODE $test_channel +o [god nick]

    >> MODE $test_channel +M
    << MODE $test_channel +M

client peon

god >> MODE $test_channel +o [peon nick]

peon :
    << MODE $test_channel +o [peon nick]
    >> KICK $test_channel [god nick] {No irc cops allowed}
    << ERR_ISCHANSERVICE
