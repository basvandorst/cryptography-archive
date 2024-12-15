#! /bin/sh
#                                        _
# Copyright (c) 1990,1991,1992,1993 Stig Ostholm.
# All Rights Reserved
#
#
# The author takes no responsibility of actions caused by the use of this
# software and does not guarantee the correctness of the functions.
#
# This software may be freely distributed and modified for non-commercial use
# as long as the copyright notice is kept. If you modify any of the files,
# pleas add a comment indicating what is modified and who made the
# modification.
#
# If you intend to use this software for commercial purposes, contact the
# author.
#
# If you find any bugs or porting problems, please inform the author about
# the problem and fixes (if there are any).
#
#
# Additional restrictions due to national laws governing the use, import or
# export of cryptographic software is the responsibility of the software user,
# importer or exporter to follow.
#
#
#                                              _
#                                         Stig Ostholm
#                                         Chalmers University of Technology
#                                         Department of Computer Engineering
#                                         S-412 96 Gothenburg
#                                         Sweden
#                                         ----------------------------------
#                                         Email: ostholm@ce.chalmers.se
#                                         Phone: +46 31 772 1703
#                                         Fax:   +46 31 772 3663
# 

#
# This script test different variants of the DEA and generates a configuration
# file for the variant with best performance.
#

MAKE=make
MFLAGS=make
CC=cc
CFLAGS='-O -I. -I../include -I../fips -I../config'
RM='rm -f'

OUT=/dev/null

while :; do
	case $# in
	0)	break;;
	esac
	case "$1" in
	MAKE=)	MAKE="`expr '$1' : '/MAKE=\(.*\)/'`";;
	MFLAGS=) MFLAGS="`expr '$1' : '/MFLAGS=\(.*\)/'`";;
	CC=)	CC="`expr '$1' : '/CC=\(.*\)/'`";;
	CFLAGS=) CFLAGS="`expr '$1' : '/CFLAGS=\(.*\)/'`";;
	RM=)	RM="`expr '$1' : '/RM=\(.*\)/'`";;
	-o)	shift
		case $# in
		0)	exit 1;;
		esac
		OUT="$1";;
	esac
	shift
done


OBJS='bench.o des_ksched.o des_sched.o des_isweak.o des_checkkp.o parity_tab.o'

BRM='dea_bench bench.data'

PROG=bench

DATA_FILE=bench.data
LOOPS=100000


if ${MAKE} ${MFLAGS} ${OBJS}; then
	:
else
	exit $?
fi

set -x

eval ${RM} $DATA_FILE

for DEFINES in	'' \
		'-DUSE_LOOP' \
		'-DUSE_INC' \
		'-DUSE_INC -DUSE_LOOP'; \
do
	if eval ${CC} ${CFLAGS} -DDEA_TEST ${DEFINES} -c des_dea.c; then
		if eval ${CC} ${CFLAGS} -o ${PROG} ${OBJS} des_dea.o; then
			TIME="`time ./${PROG} ${LOOPS} 2>&1`"
			echo "${TIME} # ${DEFINES}" >> $DATA_FILE
			eval ${RM} dea_bench des_dea.o
		else
			exit $?
		fi
	else
		exit $?
	fi
done

#
# Known time formats:
#
#	"0.010u 0.060s 0:02.01 3.4% 0+49k 0+0io 0pf+0w" (builtin csh)
#	"       10.9 real        10.8 user         0.0 sys"
#

FASTEST="`sed \
		-e 's/^.* \([0-9.]*\) user [^#]*#/\1/' \
		-e 's/^\([0-9.]*\)u [^#]*#/\1/' \
			$DATA_FILE | sort -n +0 -1 | head -1`"
CONFIG=
for D in $FASTEST; do
	case "${D}" in
	-D*)	CONFIG="${CONFIG}
#define	`expr $D : '-D\(.*\)$'`"
	esac
done

eval ${RM} $DATA_FILE

cat > "${OUT}" << END_OF_TEXT
/*
 * This file is automaticly generated, do not change.
 */
${CONFIG}
END_OF_TEXT

exit 0
