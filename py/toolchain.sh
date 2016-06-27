#!/bin/bash

# wit default values
CUR_DIR=${PWD##*/}
tmp=${PGOE_BENCHMARK:-${CUR_DIR/spec_cpu./}}
if [[ $tmp == *"povray"* ]]; then
    Bench=$tmp.gcc
else
    Bench=$tmp.clang
fi

Phase=${PGOE_PHASE:-hybrid-dyn}
Compiler=${CC:-cc}

export PGOE_TARGET_EXE=./$Bench-$Phase-$Compiler

UNW_NODES=$PGOE_BASE/out/unw-$Bench-$Phase.txt
if [ ! -f $UNW_NODES ]; then (>&2 echo "Cannot find unwound nodes: $UNW_NODES"); exit; fi
INSTR_NODES=$PGOE_BASE/out/instrumented-$Bench-$Phase.txt
if [ ! -f $INSTR_NODES ]; then (>&2 echo "Cannot find unwound nodes: $INSTR_NODES"); exit; fi

if [ ! -s $INSTR_NODES ]; then
	INSTR_NODES=$PGOE_BASE/wl-dummy.txt # replace empty wl with dummy because of clang bug
fi

Driver=$LIBSAMPLING_BASE/lib/libsampling.$Compiler.$HOSTNAME.so

if [ "$CC" = "clang" ]; then
    # compile target binary
    echo "WL_FILE=$INSTR_NODES make sel-instr -j 8"
    WL_FILE=$INSTR_NODES make sel-instr -j 8  &> /dev/null

    fullBinaryName=$PGOE_TARGET_EXE.sel
else
    (>&2 echo "WARNING: CC is: $CC, CXX is: $CXX")
    make instr  &> /dev/null

    fullBinaryName=$PGOE_TARGET_EXE.instr
fi

# create nm_file and regions_file
python $LIBSAMPLING_BASE/py/gen.py $fullBinaryName $UNW_NODES

if [ "$1" = "go" ]; then
    outFile=out-sampling/$Phase

    if [ "$2" = "log" ]; then

        timestamp=`date --rfc-3339=seconds`
        echo " ================ $timestamp ================ " >> $outFile
        echo "taskset -c 13 monitor-run -i $Driver $fullBinaryName $(< ref)" &>> $outFile
        echo "" >> $outFile

        taskset -c 13 monitor-run -i $Driver $fullBinaryName $(< ref)  2>&1 &>> $outFile
        echo "" >> $outFile
    else
        echo "taskset -c 13 monitor-run -i $Driver $fullBinaryName $(< ref)"
        taskset -c 13 monitor-run -i $Driver $fullBinaryName $(< ref)
    fi
else
    echo "taskset -c 13 monitor-run -i $Driver $fullBinaryName $(< ref)"
fi
