#!/bin/sh
cd `dirname $0`
testdir=`pwd`

kill $(cat $testdir/ircd.pid.1)
kill $(cat $testdir/ircd.pid.2)
kill $(cat $testdir/ircd.pid.3)
