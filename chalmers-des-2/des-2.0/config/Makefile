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

#
# All make programs dont have the .h suffix defined
#
.SUFFIXES: .h .sh

      SUBDIRS = fake test

#           CC = gcc
      DEFINES = -DCONFIG
     INCLUDES = -I.
#        CCOPT = -g
        CCOPT =
       CFLAGS = ${CCOPT} ${DEFINES} ${INCLUDES}
      LDFLAGS =

           RM = rm -f
         ECHO = echo
         MAKE = make

 MAKE_SUBDIRS = set -x; for D in ${SUBDIRS}; do \
		    ( cd $$D; ${MAKE} ${MFLAGS} $@ ); \
		done


     INCDIR = ../include

     LIBDIR = 


      SRCS = hrdwr.c

      OBJS = hrdwr.o

     PROGS = hrdwr

      HDRS = cc.h sys.h hrdwr.h

     REMOVES = ${PROGS} ${HDRS} tmp.h stdlib.h termios.h


.sh.h:
	./$< 'CC=${CC}' 'RM=${RM}' -o $@

all	:: ${HDRS}
	@ ${MAKE_SUBDIRS}

hrdwr	: hrdwr.c config.h cc.h sys.h
	${CC} ${CFLAGS} -o $@ hrdwr.c ${LDFLAGS}

hrdwr.h	: hrdwr
	./$? > tmp.h
	mv tmp.h $@

install	:
	@ ${MAKE_SUBDIRS}

clean	:
	${RM} ${REMOVES} *.o Makefile.bak core a.out *.NEW *.BAK
	@ ${MAKE_SUBDIRS}


depend  : ${THDR}
	../util/mkdepend.sh -CC '${CC}' -CFLAGS '${CFLAGS}' ${SRCS}
	@ ${MAKE_SUBDIRS}

cc.h	: cc.sh
sys.h	: sys.sh

# DEPEND
#
# Don not change the lines below, run make depend
#
