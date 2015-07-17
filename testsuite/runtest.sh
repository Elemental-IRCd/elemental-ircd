#!/bin/bash

cd $(dirname "${BASH_SOURCE[0]}")

#TODO: Test more than startup

source ./startall.sh

sleep 5

ircd1=`<ircd.pid.1`
ircd2=`<ircd.pid.2`
ircd3=`<ircd.pid.3`

kill $ircd1 $ircd2 $ircd3

wait "0${ircd1}"
ecode1=$?

wait "0${ircd2}"
ecode2=$?

wait "0${ircd3}"
ecode3=$?

if [ x"000" != x"${ecode1}${ecode2}${ecode3}" ]; then
    echo "Non-zero exit code"
    echo "ircd 1: ${ecode1}"
    echo "ircd 2: ${ecode2}"
    echo "ircd 3: ${ecode3}"
    exit 1
fi

echo "Test ok!"

exit 0
