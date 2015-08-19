#!/usr/bin/expect -f

source lib/lib.tcl

begin test register {Test client registration}

client new

# Client constructor checks for RPL_WELCOME

# Check that ISUPPORT is sent
expect_rpl RPL_ISUPPORT

# Check the motd is sent
expect_rpl RPL_MOTDSTART
expect_rpl RPL_MOTD
expect_rpl RPL_ENDOFMOTD

quit
