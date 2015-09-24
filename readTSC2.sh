#!/bin/bash
for path in "$@"; do
	echo "  $path"
	cat $path | grep target | awk '{ print $5 }'
done
