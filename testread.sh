#!/bin/sh

if [ "$#" -ne 1 ]; then
    echo "Bad number of parameters"
    exit
fi

SIZE=$1

dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1

dd if=/dev/chardisk0 of=chario-out-$SIZE bs=$SIZE count=1

diff randomdata$SIZE chario-out-$SIZE
