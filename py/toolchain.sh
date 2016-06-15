#!/bin/bash

# wit default values
Bench=${PGOE_BENCHMARK:-462.libquantum.clang}
Phase=${PGOE_PHASE:-hybrid-dyn}
Compiler=${CC:-cc}

export PGOE_TARGET_EXE=./$Bench-$Phase-$Compiler

UNW_NODES=$PGOE_BASE/out/unw-$Bench-$Phase.txt
if [ ! -f $UNW_NODES ]; then echo "Cannot find unwound nodes: $UNW_NODES"; exit; fi
INSTR_NODES=$PGOE_BASE/out/instrumented-$Bench-$Phase.txt
if [ ! -f $INSTR_NODES ]; then echo "Cannot find unwound nodes: $INSTR_NODES"; exit; fi

if [ ! -s $INSTR_NODES ]; then
	INSTR_NODES=$PGOE_BASE/wl-dummy.txt # replace empty wl with dummy because of clang bug
fi

Driver=$LIBSAMPLING_BASE/lib/libsampling.$Compiler.$HOSTNAME.so

# compile target binary
echo "WL_FILE=$INSTR_NODES make sel-instr -j 8"
#WL_FILE=$INSTR_NODES make sel-instr
WL_FILE=$INSTR_NODES make sel-instr -j 8 # this is just for testing on my laptop without the clang fork

# create nm_file and regions_file
python $LIBSAMPLING_BASE/py/gen.py $PGOE_TARGET_EXE.sel $UNW_NODES


echo "taskset -c 13 monitor-run -i $Driver $PGOE_TARGET_EXE.sel $(< ref)"

