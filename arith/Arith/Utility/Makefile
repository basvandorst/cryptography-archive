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

# This is the @(#)Makefile	1.1 for the utility library, created 3/29/90.

ARCH = ${TARGET_ARCH:-%=%}
LIB = ../Library/${ARCH}
INC = ../Include

CFLAGS = -g
CPPFLAGS = -I${INC}
LINTFLAGS = -abch

SOURCE = bailout.c \
         blurb.c \
         capture.c \
         release.c \
         makewarehouse.c \
         constant.c \
         ratconst.c \
         more.c \
         inratl.c \
         collect.c \
         negate.c \
         shcopy.c \
         ratcopy.c \
         showme.c \
         ratshowme.c \
         size.c

OBJECT = bailout.o \
         blurb.o \
         capture.o \
         release.o \
         makewarehouse.o \
         constant.o \
         ratconst.o \
         more.o \
         inratl.o \
         collect.o \
         negate.o \
         shcopy.o \
         ratcopy.o \
         showme.o \
         ratshowme.o \
         size.o

LINTOUT = bailout.ln \
         blurb.ln \
         capture.ln \
         release.ln \
         makewarehouse.ln \
         constant.ln \
         ratconst.ln \
         more.ln \
         inratl.ln \
         collect.ln \
         negate.ln \
         shcopy.ln \
         ratcopy.ln \
         showme.ln \
         ratshowme.ln \
         size.ln

trial : trial.o libU.a
	cc -L. trial.o -lU -o trial

libU.a : ${OBJECT}
	ar rcv libU.a ${OBJECT}
	ranlib libU.a

install : libU.a
	install -c -m444 libU.a ${LIB}

picky : ${LINTOUT}

lint : ${SOURCE}
	${LINT.c} ${SOURCE}

clean :
	rm -f *.o *~ core *.ln

veryclean : clean
	rm -f libU.a trial

capture.o :    ${INC}/storage.h
release.o :    ${INC}/storage.h
makewarehouse.o : ${INC}/storage.h
constant.o :   ${INC}/normal.h ${INC}/Imacro.h ${INC}/Pmacro.h
ratconst.o :   ${INC}/normal.h ${INC}/Qstruct.h
ratcopy.o :    ${INC}/normal.h ${INC}/Qstruct.h
more.o :       ${INC}/normal.h
inratl.o :     ${INC}/normal.h ${INC}/Qstruct.h ${INC}/myinput.h
collect.o :    ${INC}/normal.h ${INC}/Imacro.h ${INC}/Pmacro.h ${INC}/myinput.h
negate.o :     ${INC}/normal.h ${INC}/Imacro.h
showme.o :     ${INC}/normal.h ${INC}/Imacro.h
ratshowme.o :  ${INC}/normal.h ${INC}/Qstruct.h
size.o :       ${INC}/normal.h ${INC}/Imacro.h

trial.o :      ${INC}/integer.h ${INC}/storage.h
