#!/bin/bash
for path in "$@"; do
	cat $path | grep target | awk '{ print $17 }'
done
