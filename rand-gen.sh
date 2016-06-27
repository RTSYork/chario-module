#!/bin/sh

dd if=/dev/urandom of=randomdata bs=10M count=1

dd if=randomdata of=/dev/charfs bs=4k

# SIZE=128k; dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=132k; dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=136k; dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=140k; dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=256k; dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=260k; dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=264k; dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=512k; dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=1M;   dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=2M;   dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=3M;   dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
# SIZE=4M;   dd if=randomdata of=randomdata$SIZE bs=$SIZE count=1
