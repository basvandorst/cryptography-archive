#!/bin/sh
L3PATH=.
PREFIX=.
export L3PATH
echo hit return when prompted for a password
$PREFIX/decrypt test <sample.L3 >sample.dec
echo you should see no differences here
diff sample sample.dec
$PREFIX/decrypt test <sample.L32 >sample.dec
echo you should see no differences here
diff sample sample.dec
echo "type enter to continue"
read i
echo hit return when prompted for a password
$PREFIX/encrypt test <ssk.c >ssk.L3
$PREDIX/decrypt test <ssk.L3 >foo
echo you should see no differences here
diff foo ssk.c
read i
echo checking signature
$PREFIX/vsign markh@wimsey.bc.ca MANIFEST.sig <MANIFEST
echo done
