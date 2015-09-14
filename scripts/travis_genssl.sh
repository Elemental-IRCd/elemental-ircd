#!/bin/bash

echo "Using the pregenned cert"

cp testsuite/ssl* ~/ircd/etc
cp testsuite/dh.pem ~/ircd/etc
