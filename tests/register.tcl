begin {Test client registration}

client new

# Client constructor checks for RPL_WELCOME

# Check that ISUPPORT is sent
<< $RPL_ISUPPORT

# Check the motd is sent
<< $RPL_MOTDSTART
<< $RPL_MOTD
<< $RPL_ENDOFMOTD
