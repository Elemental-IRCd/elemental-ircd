#!/bin/sh

set -e

autoreconf --version
autoreconf --install -v --force
./configure $@
