#!/bin/bash

set -e

# This just reruns autogen.sh, ./configure and make -j4 to
# automate automake. Useful for `git bisect` automation.
#
# Yo dawg.

./autogen.sh
./configure --prefix=$HOME/prefix/ircd

my_uname=$(uname)

case my_uname in
    Darwin*)
        cores=$(sysctl -n hw.ncpu) ;;
    Linux*)
        cores=$(awk '/^processor/ {++n} END {print n+1}' /proc/cpuinfo) ;;
esac

make -j"$cores"
