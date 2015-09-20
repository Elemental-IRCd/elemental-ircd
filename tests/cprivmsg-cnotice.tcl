begin {Ensure that CNOTICE and CPRIVMSG are gone}

client cprivmsg

cprivmsg >> CPRIVMSG
cprivmsg << $ERR_UNKNOWNCOMMAND

cprivmsg >> CNOTICE
cprivmsg << $ERR_UNKNOWNCOMMAND
