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

    MAN1 = deskey.1 descrypt.1
    MAN3 = des.3 encrypt.3
    MAN8 = desbench.8 destest.8 desvrfy.8

 INSTALL = install -c

      RM = rm -f
    ECHO = echo

  MANDIR =


all	:

install	: 
	@if [ "${MANDIR}" ]; then \
		for M in ${MAN1}; do \
			${ECHO} ${INSTALL} $$M ${MANDIR}/man1; \
			${INSTALL} $$M ${MANDIR}/man1; \
		done; \
		for M in ${MAN3}; do \
			${ECHO} ${INSTALL} $$M ${MANDIR}/man3; \
			${INSTALL} $$M ${MANDIR}/man3; \
		done; \
	else \
		${ECHO} 'Define destination directory (MANDIR = destination)'; \
		exit 1; \
	fi


clean	:
	${RM} *.o Makefile.bak core a.out *.NEW *.BAK

depend	:
	${ECHO} 'Nothing to depend'


# DEPEND
