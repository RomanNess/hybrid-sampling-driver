#!/bin/bash
LD_PRELOAD="$LIBSAMPLING_BASE/lib/libshadowstack.serial.$HOSTNAME.so $LIBMONITOR_BASE/lib/libmonitor.so" $@
