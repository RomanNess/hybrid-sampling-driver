#!/bin/bash

CUR_DIR=${PWD##*/}
tmp=${PGOE_BENCHMARK:-${CUR_DIR/spec_cpu./}}
if [[ $tmp == *"povray"* ]]; then
    Bench=$tmp.gcc
else
    Bench=$tmp.clang
fi
Compiler=${CC:-cc}

export PGOE_TARGET_EXE=./$Bench-$Compiler

DriverVanilla=$LIBSAMPLING_BASE/lib/libempty-monitor.so
if [ ! -f $DriverVanilla ]; then (>&2 echo echo "Cannot find driver: $DriverVanilla"); exit; fi

DriverPAPI=$LIBSAMPLING_BASE/lib/libsampling.$Compiler.$HOSTNAME.so
if [ ! -f $DriverPAPI ]; then (>&2 echo echo "Cannot find driver: $DriverPAPI"); exit; fi

fullBinaryName=$PGOE_TARGET_EXE.vanilla

RunBenchmark() {
    outFile=out-vanilla/$3

    if [ "$2" = "log" ]; then

        timestamp=`date --rfc-3339=seconds`
        echo " ================ $timestamp ================ " >> $outFile
        echo "taskset -c 13 monitor-run -i $1 $fullBinaryName $(< ref)" &>> $outFile
        echo "" >> $outFile

        taskset -c 13 monitor-run -i $1 $fullBinaryName $(< ref)  2>&1 &>> $outFile
        echo "" >> $outFile
    else
        echo "taskset -c 13 monitor-run -i $1 $fullBinaryName $(< ref)"
        taskset -c 13 monitor-run -i $1 $fullBinaryName $(< ref)
    fi
}


# compile target binary
make vanilla -j &> /dev/null
if [ ! -f $fullBinaryName ]; then (>&2 echo echo "Cannot find binary: $fullBinaryName"); exit; fi

for i in {1..10}; do
    echo -e " $i\c"
	RunBenchmark $DriverVanilla "log" "vanilla"
	echo -e " $i\c"
	RunBenchmark $DriverPAPI "log" "papi"
done
echo ""



