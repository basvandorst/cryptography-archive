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
#CP=cp
CP=ln

OUT=/dev/null

while :; do
	case $# in
	0)	break;;
	esac
	case "$1" in
	CC=)	CC="`expr '$1' : '/CC=\(.*\)/'`";;
	RM=)	RM="`expr '$1' : '/RM=\(.*\)/'`";;
	-o)     shift
		case $# in
		0)      exit 1;;
		esac
		OUT="$1";;
	esac
	shift
done


if eval ${CC} -E test/signal.c | grep -s 'void[ 	\*()]*signal' >/dev/null 2>&1; then
	SIGRETURN='#define SIGRETURN void'
	RETURN_SIGFUNC='#define RETURN_SIGFUNC(s) return'
else
	SIGRETURN='#define SIGRETURN int'
	RETURN_SIGFUNC='#define RETURN_SIGFUNC(s) return(s)'
fi

if ${CC} -E test/termios.c >/dev/null 2>&1; then
	FAKE_TERMIOS='/* #define FAKE_TERMIOS */'
else
	FAKE_TERMIOS='#define FAKE_TERMIOS'
	eval ${CP} fake/termios.h termios.h
	echo "${CP} fake/termios.h termios.h"
fi

if ${CC} -E test/stdlib.c >/dev/null 2>&1; then
	FAKE_STDLIB='/* #define FAKE_STDLIB */'
else
	FAKE_STDLIB='#define FAKE_STDLIB'
	eval ${CP} fake/stdlib.h stdlib.h
	echo "${CP} fake/stdlib.h stdlib.h"
fi

if ${CC} -E test/string.c | grep -s memcpy >/dev/null 2>&1; then
	MEMORY='/* #include <memory.h> */'
	MEMCPY='/* #define memcpy(s1,s2,n) bcopy(s2,s1,n) */'
	MEMCMP='/* #define memcmp(s1,s2,n) bcmp(s2,s1,n) */'
else
	if ${CC} -E test/memory.c | grep -s memcpy >/dev/null 2>&1; then
		MEMORY='#include <memory.h>'
		MEMCPY='/* #define memcpy(s1,s2,n) bcopy(s2,s1,n) */'
		MEMCMP='/* #define memcmp(s1,s2,n) bcmp(s2,s1,n) */'
	else
		MEMORY='/* #include <memory.h> */'
		MEMCPY='#define memcpy(s1,s2,n) bcopy(s2,s1,n)'
		MEMCMP='#define memcmp(s1,s2,n) bcmp(s2,s1,n)'
	fi
fi

if ${CC} -E test/times.c | grep -s 'clock_t[ 	]*times('; then
	HAS_TIMES='#define HAS_TIMES'
	HAS_GETTIMEOFDAY=''
else
	HAS_TIMES='/* #define HAS_TIMES */'
	if ${CC} -E test/timeval.c | grep -s 'struct[ 	]*timeval'; then
		HAS_GETTIMEOFDAY='# define HAS_GETTIMEOFDAY'
	else
		HAS_GETTIMEOFDAY='/* # define HAS_GETTIMEOFDAY */'
	fi
fi

cat > "${OUT}" << END_OF_TEXT
/*
 * This file is automaticly generated, do not change.
 */

$SIGRETURN
$RETURN_SIGFUNC

#ifdef SIGHUP
# if !defined(SA_NOCLDSTOP) && defined(NEED_SIGACTION)
#  include	"fake/signal.h"
# endif /* !SA_NOCLDSTOP && NEED_SIGACTION */
#endif /* SIGHUP */

$FAKE_TERMIOS
$FAKE_STDLIB

$MEMORY
$MEMCPY
$MEMCMP

#ifndef NULL
# define NULL 0
#endif  /* NULL */

$HAS_TIMES
#if defined(NEED_TIMES) && !defined(HAS_TIMES)
# include	"fake/times.c"
$HAS_GETTIMEOFDAY
#endif /* NEED_TIMES */

END_OF_TEXT
