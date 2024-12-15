#!/bin/sh
# killem.sh - a simple script to control the
#
# removes all offensive files except for the build executables
#
# Solaris x86 hint: successful build requires GNU make and GNU as.
# make sure these are on your path _before_ the ones in /usr/ccs/bin



mywd=`pwd`
MODULES="libs/pfl libs/pgpcdk clients/pgp/shared clients/pgp/cmdline"


for x in $MODULES ; do
        cd $x;
        make distclean || make clean
        cd $mywd
done