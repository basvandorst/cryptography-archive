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



./desvrfy -v

TOTAL_ERRORS=$?

./desvrfy3 -v

TOTAL_ERRORS=`expr $TOTAL_ERRORS + $?`

echo

echo "Encrypt then decrypt and compare with original data."
N=0
ERRORS=0
exec < /usr/dict/words
while read KEY; do
	echo -n "	Key: $KEY ... "
	if ./destest $KEY; then
		echo Ok.
	else
		echo Error.
		ERRORS=`exec expr $ERRORS + 1`
	fi
	N=`exec expr $N + 1`
	case $N in
	200)	break;;
	esac
done
echo "The implementation passed $N test(s) with $ERRORS error(s)."
TOTAL_ERRORS=`exec expr $TOTAL_ERRORS + $ERRORS`

echo

./ctest.sh
TOTAL_ERRORS=`exec expr $TOTAL_ERRORS + $?`

echo
echo

echo "The implementation passed all test with $TOTAL_ERRORS error(s)."

exit $TOTAL_ERRORS
