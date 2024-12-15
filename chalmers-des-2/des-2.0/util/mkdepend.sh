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
CFLAGS=
 MFILE=Makefile

while :; do
	case "$1" in
	-CC)
		case $# in
		1)	
			exit 1
			;;
		esac
		shift
		CC="$1"
		;;
	-CFLAGS)
		case $# in
		1)	
			exit 1
			;;
		esac
		shift
		CFLAGS="$1"
		;;
	*)	
		break
		;;
	esac
	shift
done

(
	awk '/^# DEPEND/ {
		print;
		exit;
	}
	{
		print;
	}
	END {
		print "#";
		print "# Don not change the lines below, run make depend";
		print "#";
	}' $MFILE

	${CC} -M ${CFLAGS} $@ | \
	awk -F: 'BEGIN {
		target = "";
		depend = "";
	}
	$1 != target {
		if (depend != "") {
			print ptarget depend;
			depend = "";
		}
		target = $1;
		if (length(target) < 8) {
			ptarget = target "		:";
		} else {
			ptarget = target "	:";
		}
	}
	{
		tmp = depend " " $2;
		if (length(tmp) > 66) {
			print ptarget depend;
			depend = $2;
		} else {
			depend = tmp;
		}
	}
	END {
		if (depend != "") {
			print ptarget depend;
		}
	}' - | sed 's/  */ /g'
) > $MFILE.new

case $? in
0)	
	if mv $MFILE $MFILE.bak; then
		if mv $MFILE.new $MFILE; then
			exit 0
		else
			mv $MFILE.bak $MFILE
			exit 1
		fi
	else
		exit 1
	fi
	;;
*)
	exit 1
	;;
esac
