begin {snomask for klines}

# XXX HACK
after 750

client god
    oper god
    << RPL_YOUREOPER
    >> MODE [god nick] +s +K

    # Kline bansworth
    >> KLINE 5 ~bansworth@* ON * {Test}
    << NOTICE * {*K-Line*}

after 150

# ensure bansworth is banned (shocker!)
client klined -user bansworth
    << ERR_YOUREBANNEDCREEP
    << ERROR {*Banned*}

god :
    >> UNKLINE ~bansworth@* ON *
    << NOTICE * {*** * has removed the temporary K-Line *}

    # Now test an X-Line
    >> XLINE 5 eggdrop?bot ON * {no bots}
    << NOTICE * {*** * added temporary 5 min. X-Line for *}

# ensure the xlined gecos doesn't work with the right error
client xlined -gecos "eggdrop bot"
    << ERROR {*Bad user info*}

god :
    >> UNXLINE eggdrop?bot ON *
    << NOTICE * {X-Line for * is removed}
