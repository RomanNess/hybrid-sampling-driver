#!/bin/bash
LD_PRELOAD="$LIBSAMPLING_BASE/lib/libempty-monitor.so $LIBMONITOR_BASE/lib/libmonitor.so" $@
