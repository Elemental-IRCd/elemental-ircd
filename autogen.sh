#!/bin/sh

set -e

autoreconf --install -v --force
./configure $@
