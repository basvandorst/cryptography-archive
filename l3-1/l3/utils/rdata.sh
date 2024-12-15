#!/bin/sh
(/usr/ucb/netstat -i
/usr/ucb/netstat -a
/usr/ucb/netstat -A
/usr/ucb/netstat -m
/usr/ucb/netstat -s
/usr/etc/pstat -ap
/usr/etc/pstat -f
/usr/etc/pstat -i
/usr/etc/pstat -S
/usr/etc/pstat -s
/usr/etc/pstat -T
/bin/iostat
/bin/iostat -d
/bin/iostat -D
/bin/iostat -I
/usr/etc/nfsstat -c
/usr/etc/nfsstat -n
/usr/etc/nfsstat -m
/usr/etc/nfsstat -r
/usr/etc/nfsstat -s
/usr/etc/crash <<HERE
buf
c
ctx
dbfree
dblock
inode
mbfree
pment
pmgrp
s
seg
stream
u
vfs
HERE
) | haval
