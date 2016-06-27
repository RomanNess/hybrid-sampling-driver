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
Target=$PGOE_TARGET_EXE.vanilla

CheckFile() {
	if [ ! -f $1 ]; then (>&2 echo "Cannot find $2 file: $1"); exit; fi
}

Driver=$LIBSAMPLING_BASE/lib/libflat.so
CheckFile $Driver "driver"

# compile target binary
make vanilla-no-inline -j 8
CheckFile $Target "target"

rm -f flat_profile

python $LIBSAMPLING_BASE/py/gen.py $Target

echo "taskset -c 13 monitor-run -i $Driver $Target $(< ref)"
if [ "$1" = "go" ]; then
	monitor-run -i $Driver $Target $(< ref)
	python $LIBSAMPLING_BASE/py/flat.py .
	cp samples_file.txt $PGOE_BASE/spec-centos/$Bench.samples
fi

if [ "$1" = "t420" ]; then
	monitor-run -i $Driver -i $PAPI_BASE/lib/libpapi.so $PGOE_TARGET_EXE.vanilla $(< ref)
	python $LIBSAMPLING_BASE/py/flat.py .
fi

