#!/bin/bash
LD_PRELOAD="$LIBSAMPLING_BASE/lib/liboverhead.$HOSTNAME.so $LIBMONITOR_BASE/lib/libmonitor.so" $@
