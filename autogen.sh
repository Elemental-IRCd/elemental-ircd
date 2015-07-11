#!/bin/sh

set -e

autoreconf --version
autoreconf --install --verbose --force --warnings=all
scripts/version.sh
