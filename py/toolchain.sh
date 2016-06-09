#!/bin/bash

# wit default values
Bench=${PGOE_BENCHMARK:-462.libquantum.clang}
Phase=${PGOE_PHASE:-UnwindSample}
Compiler=${CC:-cc}

export PGOE_TARGET_EXE=./$Bench-$Phase-$Compiler

UNW_NODES=$PGOE_BASE/out/unw-$Bench-$Phase.txt
INSTR_NODES=$PGOE_BASE/out/instrumented-$Bench-$Phase.txt

Driver=$LIBSAMPLING_BASE/lib/libsampling.$Compiler.$HOSTNAME.so

# compile target binary
echo "WL_FILE=$INSTR_NODES make sel-instr -j 8"
#WL_FILE=$INSTR_NODES make sel-instr
WL_FILE=$INSTR_NODES make sel-instr -j 8 # this is just for testing on my laptop without the clang fork

# create nm_file and regions_file
python $LIBSAMPLING_BASE/py/gen.py $PGOE_TARGET_EXE.sel $UNW_NODES

ls $Driver