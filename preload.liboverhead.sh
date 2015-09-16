#!/bin/bash
LD_PRELOAD="$LIBSAMPLING_BASE/lib/liboverhead.so $LIBMONITOR_BASE/lib/libmonitor.so" $@
