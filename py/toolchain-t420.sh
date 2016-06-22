#!/bin/bash

# with default values
Bench=${PGOE_BENCHMARK:-462.libquantum}.clang
Phase=${PGOE_PHASE:-hybrid-dyn}
Compiler=${CC:-cc}

export PGOE_TARGET_EXE=./$Bench-$Compiler

UNW_NODES=$PGOE_BASE/out/unw-$Bench-$Phase.txt
if [ ! -f $UNW_NODES ]; then echo "Cannot find unwound nodes: $UNW_NODES"; exit; fi
INSTR_NODES=$PGOE_BASE/out/instrumented-$Bench-$Phase.txt
if [ ! -f $INSTR_NODES ]; then echo "Cannot find unwound nodes: $INSTR_NODES"; exit; fi

if [ ! -s $INSTR_NODES ]; then
	INSTR_NODES=$PGOE_BASE/wl-dummy.txt # replace empty wl with dummy because of clang bug
fi

Driver=$LIBSAMPLING_BASE/lib/libflat.so
if [ ! -f $Driver ]; then echo "Cannot find driver: $Driver"; exit; fi

# compile target binary
echo "make vanilla -j 8"
make vanilla -j 8

rm -f flat_profile

python $LIBSAMPLING_BASE/py/gen.py $PGOE_TARGET_EXE.vanilla

#echo "taskset -c 13 monitor-run -i $Driver -i $PAPI_BASE/lib/libpapi.so $PGOE_TARGET_EXE.vanilla $(< ref)"
echo "taskset -c 13 monitor-run -i $Driver $PGOE_TARGET_EXE.vanilla $(< ref)"


#python $LIBSAMPLING_BASE/py/flat.py .
