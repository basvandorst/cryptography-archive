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

 TEXT=/usr/dict/words
CRYPT=/usr/tmp/CRYPT_FILE
CLEAR=/usr/tmp/CLEAR_FILE

ALGORITHMS='CBC ECB CFB8 OFB8 PCBC'

CRYPT_PROG=../prog/descrypt
KEY_PROG=../prog/deskey
VERIFY_PROG=./desverify


echo "Test the ${CRYPT_PROG} program:"

echo

echo "	Test all encryption modes against ${TEXT}"

trap 'rm -f $CRYPT $CLEAR; exit 1' 1 2 3 15

N=0
ERRORS=0

for I in 1 2 3 4 5; do

	HEXKEY=`$KEY_PROG -r`

	for A in $ALGORITHMS; do

		echo -n "		$I: Test of algorithm '${A}': ... "
		$CRYPT_PROG -e -h -k $HEXKEY -m $A $TEXT  $CRYPT
		$CRYPT_PROG -d -h -k $HEXKEY -m $A $CRYPT $CLEAR
		if cmp -s $TEXT $CLEAR; then
			echo Ok.
		else
			echo Error.
			ERRORS=`expr $ERRORS + 1`
		fi
		N=`expr $N + 1`
		rm -f $CRYPT $CLEAR

		echo -n "		$I: Test of algorithm '${A}' (ascii): ... "
		$CRYPT_PROG -e -a -h -k $HEXKEY -m $A $TEXT  $CRYPT
		$CRYPT_PROG -d -a -h -k $HEXKEY -m $A $CRYPT $CLEAR
		if cmp -s $TEXT $CLEAR; then
			echo Ok.
		else
			echo Error.
			ERRORS=`expr $ERRORS + 1`
		fi
		N=`expr $N + 1`
		rm -f $CRYPT $CLEAR

	done

done

echo "	$CRYPT_PROG passed $N test(s) with $ERRORS error(s)"

echo

echo "	Encrypt then decrypt and compare all files in the directory."

trap 'rm -f $KEY_FILE $CRYPT_FILE $DECRYPT_FILE; exit 1' 1 2 3 15
N=0
ERRORS=0
HEXKEY_FILE=key.$$

$KEY_PROG -r > $HEXKEY_FILE
HEXKEY=`exec cat $HEXKEY_FILE`

for FILE in *; do
	CRYPT_FILE="/usr/tmp/${FILE}.C_$$"
	DECRYPT_FILE="/usr/tmp/${FILE}.D_$$"
	echo -n "	File $FILE  ... "
	if $CRYPT_PROG -e -h -f $HEXKEY_FILE $FILE $CRYPT_FILE; then
		if $CRYPT_PROG -d -h -k $HEXKEY $CRYPT_FILE $DECRYPT_FILE; then
			if cmp $FILE $DECRYPT_FILE; then
				echo Ok.
			else
				echo Error.
				ls -l $F $CRYPT_FILE $DECRYPT_FILE
				ERRORS=`exec expr $ERRORS + 1`
			fi
		else
			ls -l $F $CRYPT_FILE $DECRYPT_FILE
			ERRORS=`exec expr $ERRORS + 1`
		fi
		rm -f $CRYPT_FILE $DECRYPT_FILE
	else
		rm -f $CRYPT_FILE
	fi
	N=`exec expr $N + 1`
done
rm -f $HEXKEY_FILE

echo "	$CRYPT_PROG passed $N test(s) with $ERRORS error(s)."

exit $ERRORS
