#!/bin/bash

if [ ! -f $1 ]; then
	echo "$1 not found"
	exit
fi

if [ ! -x $1 ]; then
	echo "$1 not executable"
	exit
fi

LD_PRELOAD="$PWD/$1 $LIBMONITOR_BASE/lib/libmonitor.so" ${@:2}
