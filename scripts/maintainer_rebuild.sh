#!/bin/bash

set -e

# This just reruns autogen.sh, ./configure and make -j4 to
# automate automake. Useful for `git bisect` automation.
#
# Yo dawg.

./autogen.sh
./configure --prefix=$HOME/prefix/ircd

make -j4
