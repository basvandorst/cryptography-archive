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

# This is the @(#)makefile.dst 1.1, created on 3/29/90, for the polynomial library
# of the Arithmetic package.

ARCH = ${TARGET_ARCH:-%=%}
LIB = ../Library/${ARCH}
INC = ../Include

CFLAGS = -g
CPPFLAGS = -I${INC}
LDFLAGS = -L. -L${LIB}

SOURCE = Ptools.c Pdivalg.c Pgcd.c Pinitial.c Pderiv.c Proot.c

OBJECT = Ptools.o Pdivalg.o Pgcd.o Pinitial.o Pderiv.o Proot.o

libP.a : ${OBJECT}
	ar rcv libP.a ${OBJECT}
	ranlib libP.a

install : libP.a
	install -c -m 444 libP.a ${LIB}

lint : ${SOURCE}
	${LINT.c} ${SOURCE}

clean :
	rm -f *.o *~ core

veryclean : clean
	rm -f libP.a

Ptools.o :   ${INC}/polynomial.h ${INC}/storage.h
Pdivalg.o :  ${INC}/polynomial.h ${INC}/storage.h
Pgcd.o :     ${INC}/polynomial.h ${INC}/storage.h
Pinitial.o : ${INC}/polynomial.h ${INC}/storage.h
Pderiv.o :   ${INC}/polynomial.h ${INC}/storage.h
Proot.o :    ${INC}/polynomial.h ${INC}/storage.h

