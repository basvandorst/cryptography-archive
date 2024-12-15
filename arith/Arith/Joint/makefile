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

# This is the @(#)makefile.dst 1.1 created on 3/29/90.

ARCH = ${TARGET_ARCH:-%=%}
LIB = ../Library/${ARCH}
INC = ../Include

CFLAGS = -g
CPPFLAGS = -I${INC}
LDFLAGS = -L${LIB}

Qtools.o :    ${INC}/normal.h ${INC}/Ztools.h ${INC}/storage.h

libQ.a : Qtools.o
	ar rcv libQ.a Qtools.o
	ranlib libQ.a

install : libQ.a
	install -c -m 444 libQ.a ${LIB}

lint : Qtools.c
	${LINT.c} Qtools.c

clean :
	rm -f *.o *~ *.ln core

veryclean : clean
	rm -f libQ.a

