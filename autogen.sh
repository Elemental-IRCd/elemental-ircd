#!/bin/sh

set -e

autoreconf --version
autoreconf --install --verbose --warnings=portability,no-unsupported
scripts/version.sh
