#
# Arithmetic in Global Fields
# Copyright (C) 1990 Kevin R. Coombes and David R. Grant
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 1, or (at your option)
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

# This is the makefile for the test functions.
#  @(#)Makefile	1.1  3/29/90

ARCH = ${TARGET_ARCH:-%=%}
LIB = ../Library/${ARCH}
INC = ../Include

CFLAGS = -g
CPPFLAGS = -I${INC}
LDFLAGS = -L${LIB}

.SUFFIXES : .tZ .tP .oZ .oP

.c.oZ :
	${COMPILE.c} -DINTEGRAL $<
	mv $*.o $*_Z.o
	touch $*.oZ

.oZ.tZ :
	${LINK.c} -DINTEGRAL $*_Z.o -lZ -lU -o $@

.c.oP :
	${COMPILE.c} -DPOLYNOMIAL $<
	mv $*.o $*_P.o
	touch $*.oP

.oP.tP :
	${LINK.c} -DPOLYNOMIAL $*_P.o -lP -lU -o $@

all : testP testZ

testP : add.tP mult.tP sub.tP div.tP euc.tP der.tP

testZ : add.tZ mult.tZ sub.tZ div.tZ euc.tZ minus.tZ

clean :
	rm -f *.oP *.oZ *_P.o *_Z.o *~ core

veryclean : clean
	rm -f *.tP *.tZ

add.oP :    ${INC}/polynomial.h
sub.oP :    ${INC}/polynomial.h
mult.oP :   ${INC}/polynomial.h
div.oP :    ${INC}/polynomial.h
euc.oP :    ${INC}/polynomial.h
minus.oP :  ${INC}/polynomial.h
der.oP :    ${INC}/polynomial.h

add.oZ :    ${INC}/integer.h
sub.oZ :    ${INC}/integer.h
mult.oZ :   ${INC}/integer.h
div.oZ :    ${INC}/integer.h
euc.oZ :    ${INC}/integer.h
minus.oZ :  ${INC}/integer.h
