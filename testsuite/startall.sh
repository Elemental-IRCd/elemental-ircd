#!/bin/sh
cd `dirname $0`
testdir=`pwd`
prefix=`sed -n -e 's/^#define IRCD_PREFIX "\(.*\)"/\1/p' "$testdir/../include/setup.h"`
[ -d $prefix ] || { echo Unable to find installation prefix; exit 1; }

$prefix/bin/ircd -configfile $testdir/ircd.conf.1 -pidfile $testdir/ircd.pid.1 -logfile logs/ircd.log.1 &
$prefix/bin/ircd -configfile $testdir/ircd.conf.2 -pidfile $testdir/ircd.pid.2 -logfile logs/ircd.log.2 &
$prefix/bin/ircd -configfile $testdir/ircd.conf.3 -pidfile $testdir/ircd.pid.3 -logfile logs/ircd.log.3 &
