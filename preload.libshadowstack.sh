#!/bin/bash
LD_PRELOAD="$LIBSAMPLING_BASE/lib/libshadowstack.so $LIBMONITOR_BASE/lib/libmonitor.so" $@
