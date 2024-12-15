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
#

# The file @(#)Makefile 1.1, created on 3/29/90,
# is the makefile for the integer library of the multi-precision
# Arithmetic package.

ARCH = ${TARGET_ARCH:-%=%}
LIB = ../Library/${ARCH}
INC = ../Include

CFLAGS = -g
CPPFLAGS = -I${INC}

SOURCE = Ztools.c Zdivalg.c Zgcd.c Zinitial.c

OBJECT = Ztools.o Zdivalg.o Zgcd.o Zinitial.o

libZ.a : ${OBJECT}
	ar rcv libZ.a ${OBJECT}
	ranlib libZ.a

install : libZ.a
	install -c -m 444 libZ.a ${LIB}

lint : ${SOURCE}
	${LINT.c} ${SOURCE}

clean :
	rm -f *.o *~ core

veryclean : clean
	rm -f libZ.a

Ztools.o :   ${INC}/integer.h ${INC}/storage.h
Zdivalg.o :  ${INC}/integer.h ${INC}/storage.h
Zgcd.o :     ${INC}/integer.h ${INC}/storage.h
Zinitial.o : ${INC}/storage.h
