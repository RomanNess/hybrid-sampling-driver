#!/bin/bash

if [ ! -f $1 ]; then
	echo "$1 not found"
	exit
fi

LD_PRELOAD="$LIBSAMPLING_BASE/$1 $LIBMONITOR_BASE/lib/libmonitor.so" ${@:2}
