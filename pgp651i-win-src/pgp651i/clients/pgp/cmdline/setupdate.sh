#!/bin/ksh/	-v
# setupdate.sh - a simple script to control the
#
# removes all offensive files except for the build executables
#
# Solaris x86 hint: successful build requires GNU make and GNU as.
# make sure these are on your path _before_ the ones in /usr/ccs/bin

SOURCE=clients/pgp/
DESTINATION=~/clients/pgp/cmdline/

mywd=`pwd`
MODULES="clients/pgp/shared clients/pgp/shared/JPEG clients/pgp/shared/JPEG/win32 clients/pgp/shared/JPEG/common clients/pgp/cmdline clients/pgp/cmdline/patches clients/pgp/cmdline/test clients/pgp/cmdline/unix clients/pgp/cmdline/unix/config "
CVSDIRS="shared shared/JPEG shared/JPEG/win32 shared/JPEG/common cmdline/ cmdline/patches cmdline/test cmdline/unix cmdline/unix/config "

for x in $CVSDIRS ; do
        cd $SOURCE$x;
        rm -Rf CVS 
        echo Source: $SOURCE$x
	echo Destination: $DESTINATION$x
        echo Command:
        echo cp * $DESTINATION$x
        echo "***************************************" 
        cd $mywd
done