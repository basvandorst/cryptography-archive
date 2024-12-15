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

# This is the @(#)makefile.dst 1.1 for the programs that use Dtools to look for curves
# with everywhere good reduction over quadratic fields.

ARCH = ${TARGET_ARCH:-%=%}
LIB = ../Library/${ARCH}
INC = ../Include

CFLAGS = -g
CPPFLAGS = -I${INC}
LDFLAGS = -L${LIB}

Dtools.o :       ${INC}/Dstruct.h

libD.a : Dtools.o
	ar rcv libD.a Dtools.o
	ranlib libD.a

install :
	install -c -m 444 libD.a ${LIB}

lint :
	${LINT.c} Dtools.c

clean :
	rm -f *.o *~ core

veryclean : clean
	rm -f libD.a
