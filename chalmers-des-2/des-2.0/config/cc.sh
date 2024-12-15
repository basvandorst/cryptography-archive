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

CC=cc
RM='rm -f'

OUT=/dev/null

while :; do
	case $# in
	0)	break;;
	esac
	case "$1" in
	CC=)	CC="`expr '$1' : '/CC=\(.*\)/'`";;
	RM=)	RM="`expr '$1' : '/RM=\(.*\)/'`";;
	-o)	shift
		case $# in
		0)	exit 1;;
		esac
		OUT="$1";;
	esac
	shift
done


if eval ${CC} test/error.c; then
	echo 'This compilation should yield an error, aborting' 1>&2
	exit 1
else
	echo 'Compilation error ignored, continuing ...'
fi

if eval ${CC} test/void.c; then
	VOID='#define VOID (void)'
else
	echo 'Compilation error ignored, continuing ...'
	VOID='#define VOID'
fi
eval $RM a.out void.o core

if eval ${CC} test/const.c; then
	CONST='/* #define const */'
else
	echo 'Compilation error ignored, continuing ...'
	CONST='#define const'
fi
eval $RM a.out const.o core

cat > "${OUT}" << END_OF_TEXT
/*
 * This file is automaticly generated, do not change.
 */

$VOID
$CONST

END_OF_TEXT
