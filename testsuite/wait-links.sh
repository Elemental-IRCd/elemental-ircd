#!/bin/bash

set -e

cd tests

tclsh ./lib/runtest.tcl ../testsuite/ensure-links.tcl
