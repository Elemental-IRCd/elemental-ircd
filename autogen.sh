#!/bin/sh

set -e

autoreconf --version
autoreconf --install --verbose --force --warnings=portability,no-unsupported
scripts/version.sh
