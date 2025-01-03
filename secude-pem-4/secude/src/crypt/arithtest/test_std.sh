#!/bin/sh
set -e
#
# SecuDE Release 4.3 (GMD) 
#

if [ "$2" != "" ]; then
	echo "test [number_of_steps]"
	exit
fi

if [ "$1" = "-h" ]; then
	echo "test [number_of_steps]"
	exit
fi

steps=100

if [ "$1" != "" ]; then
	steps=$1
fi

echo "Creating $steps-std.trc"

mk_trace -n $steps -o $steps-std.trc

if cmp $steps-std.def $steps-std.trc; then
	echo "O.K.: No difference detected"
else
	echo "Difference between $steps-std.def and $steps-std.trc detected"
fi
