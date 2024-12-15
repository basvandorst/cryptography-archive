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

    SRCS = vry.c test.c bench.c

#      CC = gcc
INCLUDES = -I../include -I../config
#   CCOPT = -g
   CCOPT = -O
  CFLAGS = ${CCOPT} ${DEFINES} ${INCLUDES}
 LDFLAGS = ${DESLIB}

 INSTALL = install -c

      RM = rm -f
    ECHO = echo

  BINDIR =


   PROGS = desvrfy desvrfy3 destest desbench


all	: ${PROGS}


desvrfy : vrfy.c ${DESLIB}
	${CC} ${CFLAGS} -o $@ vrfy.c ${LDFLAGS}

desvrfy3: vrfy.c ${DESLIB}
	${CC} ${CFLAGS} -DDES_DEA3 -o $@ vrfy.c ${LDFLAGS}

destest	: test.c ${DESLIB}
	${CC} ${CFLAGS} -o $@ test.c ${LDFLAGS}

desbench: bench.c ${DESLIB}
	${CC} ${CFLAGS} -o $@ bench.c ${LDFLAGS}


install	:
	${ECHO} 'Nothing to install'


clean	:
	${RM} ${PROGS} ${TEST_PROGS} *.o Makefile.bak core a.out *.NEW *.BAK


depend	:
	../util/mkdepend.sh -CC '${CC}' -CFLAGS '${CFLAGS}' ${SRCS}

# DEPEND
#
# Don not change the lines below, run make depend
#
