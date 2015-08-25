#!/bin/bash

# This just reruns autogen.sh, ./configure and make -j4 to
# automate automake.
#
# Yo dawg.

./autogen.sh
./configure --prefix=$HOME/prefix/ircd

make -j4
