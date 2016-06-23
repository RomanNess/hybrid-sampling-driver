#!/bin/bash

CUR_DIR=${PWD##*/}
Bench=${CUR_DIR/spec_cpu./}
Compiler=${CC:-cc}

export PGOE_TARGET_EXE=./$Bench-$Compiler

Driver=$LIBSAMPLING_BASE/lib/libflat.so
if [ ! -f $Driver ]; then echo "Cannot find driver: $Driver"; exit; fi

# compile target binary
echo "make vanilla -j 8"
make vanilla -j 8

rm -f flat_profile

python $LIBSAMPLING_BASE/py/gen.py $PGOE_TARGET_EXE.vanilla

echo "taskset -c 13 monitor-run -i $Driver $PGOE_TARGET_EXE.vanilla $(< ref)"
if [ "$1" = "go" ]; then
	monitor-run -i $Driver $PGOE_TARGET_EXE.vanilla $(< ref)
	python $LIBSAMPLING_BASE/py/flat.py .
fi

if [ "$1" = "t420" ]; then
	monitor-run -i $Driver -i $PAPI_BASE/lib/libpapi.so $PGOE_TARGET_EXE.vanilla $(< ref)
	python $LIBSAMPLING_BASE/py/flat.py .
fi

