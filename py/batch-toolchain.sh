#!/bin/bash

for i in {1..10}; do
    echo -e "$i \c"

#    for phase in "hybrid-dyn" "hybrid-st" "ss-min" "unw-min"; do
    for phase in "ss-min"; do
        echo -e "$phase \c"
        PGOE_PHASE=$phase toolchain.sh go log > /dev/null
    done
    echo ""

done

for i in {1..10}; do
    echo -e "$i \c"

#    for phase in "ss-cpd" "unw-all"; do
    for phase in "ss-cpd"; do
        echo -e "$phase \c"
        PGOE_PHASE=$phase toolchain.sh go log > /dev/null
    done
    echo ""

done
