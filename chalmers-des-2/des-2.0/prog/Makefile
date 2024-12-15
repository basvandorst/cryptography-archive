#
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

  LIBDIR = ../lib
  DESLIB = ${LIBDIR}/libdes.a

    SRCS = crypt.c key.c filekey.c strutil.c

#      CC = gcc
 DEFINES =
INCLUDES = -I../include -I../config
#   CCOPT = -g
   CCOPT = -O
  CFLAGS = ${CCOPT} ${DEFINES} ${INCLUDES}
 LDFLAGS = ${DESLIB}

 INSTALL = install -c

      RM = rm -f
    ECHO = echo

  BINDIR =


     PROGS = descrypt deskey


all	: ${PROGS}


descrypt: crypt.o filekey.o strutil.o ${DESLIB}
	${CC} ${CCOPT} -o $@ crypt.o filekey.o strutil.o ${LDFLAGS}

deskey	: key.o filekey.o strutil.o ${DESLIB}
	${CC} ${CCOPT} -o $@ key.o filekey.o strutil.o ${LDFLAGS}


install	: ${PROGS}
	@if [ "${BINDIR}" ]; then \
		for P in ${PROGS}; do \
			${ECHO} ${INSTALL} -s $$P ${BINDIR}; \
			${INSTALL} -s $$P ${BINDIR}; \
		done \
	else \
		${ECHO} 'Define destination directory (BINDIR = destination)'; \
		exit 1; \
	fi


clean	:
	${RM} ${PROGS} *.o Makefile.bak core a.out *.NEW *.BAK


depend	:
	../util/mkdepend.sh -CC '${CC}' -CFLAGS '${CFLAGS}' ${SRCS}

# To keep old make programs happy.
crypt.o	: crypt.c
key.o	: key.c
filekey.o : filekey.c
strutil.o : strutil.c

# DEPEND
#
# Don not change the lines below, run make depend
#
