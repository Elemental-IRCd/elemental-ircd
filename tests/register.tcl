begin {Test client registration}

client new

# Must receive RPL_WELCOME
<< $RPL_WELCOME

# Check that ISUPPORT is sent
<< $RPL_ISUPPORT

# Check the motd is sent
<< $RPL_MOTDSTART
<< $RPL_MOTD
<< $RPL_ENDOFMOTD
