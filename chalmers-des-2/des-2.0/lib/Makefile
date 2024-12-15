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
.SUFFIXES: .h

#        CC = gcc
#DEFINE_TTY = -DTTY=\"/dev/tty\"
  DEFINES = ${DEFINE_TTY}
 INCLUDES = -I. -I../include -I../fips -I../config
#    CCOPT = -g
    CCOPT = -O
   CFLAGS = ${CCOPT} ${DEFINES} ${INCLUDES}
  LDFLAGS =

  INSTALL = install -c
   RANLIB = ranlib

       RM = rm -f

   LIBDIR = 

#
# quad_cksum is a separate part from now on.
#
#OLD_SRCS = quad_cksum.c
#OLD_OBJS = quad_cksum.o

EXT_SRCS = encrypt.c
EXT_OBJS = encrypt.o

    SRCS = des_dea.c des_dea3.c des_sched.c \
	   des_ecb.c des_ecb2.c des_cbc.c des_pcbc.c des_cfb8.c des_ofb8.c \
	   des_cksum.c \
	   ivec.c \
	   des_rblock.c des_rkey.c \
	   parity_tab.c des_checkkp.c des_fixkp.c \
	   des_isweak.c des_issweak.c des_nkey.c \
	   des_ksched.c \
	   des_hexblock.c des_print.c \
	   des_strkey.c \
	   prompt.c read.c sig.c tty.c \
	   des_passwd.c des_readhex.c des_read.c \
	   ${EXT_SRCS} \
	   ${OLD_SRCS}

    OBJS = des_dea.o des_dea3.o des_sched.o \
	   des_ecb.o des_ecb2.o des_cbc.o des_pcbc.o des_cfb8.o des_ofb8.o \
	   des_cksum.o \
	   ivec.o \
	   des_rblock.o des_rkey.o \
	   parity_tab.o des_checkkp.o des_fixkp.o \
	   des_isweak.o des_issweak.o des_nkey.o \
	   des_ksched.o \
	   des_hexblock.o des_print.o \
	   des_strkey.o \
	   prompt.o read.o sig.o tty.o \
	   des_passwd.o des_readhex.o des_read.o \
	   ${EXT_OBJS} \
	   ${OLD_OBJS}


   TSRCS = tab_IP.c tab_E.c tab_S_and_P.c tab_IPinv.c \
	   tab_parity.c tab_PC1.c tab_PC2.c

   TOBJS = tab_IP.o tab_E.o tab_S_and_P.o tab_IPinv.o \
	   tab_parity.o tab_PC1.o tab_PC2.o

  TPROGS = tab_IP tab_E tab_S_and_P tab_IPinv \
	   tab_parity tab_PC1 tab_PC2

   THDRS = tab_IP.h tab_E.h tab_S_and_P.h tab_IPinv.h \
	   tab_parity.h tab_PC1.h tab_PC2.h


  DESLIB = libdes.a


 REMOVES = ${TPROGS} ${THDRS} dea_bench bench.data tmp.h dea_conf.h


# This rule should be defined in all make programs.
#
#.c.o:
#	${CC} ${CFLAGS} -c $* $<

.c.h:
	${CC} ${CFLAGS} -o $* $< ${LDFLAGS}
	./$* > tmp.h
	mv tmp.h $@

.sh.h:
	./$< 'MFLAGS=${MFLAGS}' 'CC=${CC}' 'CFLAGS=${CFLAGS}' 'RM=${RM}' -o $@


all	: ${THDRS} ${DESLIB}


${DESLIB} : ${OBJS}
	ar r $@ $?
	ranlib $@


install	: all
	@set -x; if [ "${LIBDIR}" ]; then \
		${INSTALL} ${DESLIB} ${LIBDIR}; \
		${RANLIB} ${LIBDIR}/${DESLIB}; \
	else \
		exit 1; \
	fi


clean	:
	${RM} ${DESLIB} ${REMOVES} *.o Makefile.bak core a.out *.NEW *.BAK


depend  : ${THDRS}
	../config/mkdepend.sh -CC '${CC}' -CFLAGS '${CFLAGS}' ${SRCS}


tab_IP.h	: tab_IP.c ../include/des.h ../config/config.h \
		  ../fips/fips_IP.h
tab_S_and_P.h	: tab_S_and_P.c ../config/config.h ../fips/fips_S.h \
		  ../fips/fips_P.h
tab_E.h		: tab_E.c ../config/config.h ../fips/fips_E.h
tab_IPinv.h	: tab_IPinv.c ../include/des.h ../config/config.h \
		  ../fips/fips_IPinv.h

tab_parity.h	: tab_parity.c ../include/des.h ../config/config.h
tab_PC1.h	: tab_PC1.c ../include/des.h ../config/config.h \
		  ../fips/fips_PC1.h
tab_PC2.h	: tab_PC2.c ../config/config.h ../fips/fips_PC2.h

dea_conf.h	: dea_conf.sh des_dea.c dea.h


bench.o		: ../verify/bench.c
	${CC} ${CFLAGS} -c ../verify/bench.c

# To keep old make programs happy.
des_cbc.o	: des_cbc.c
des_cfb8.o	: des_cfb8.c
des_cksum.o	: des_cksum.c
des_dea.o	: des_dea.c dea_conf.h
des_dea3.o	: des_dea3.c dea_conf.h
des_ecb.o	: des_ecb.c
des_ecb2.o	: des_ecb2.c
des_hexblock.o	: des_hexblock.c
des_issweak.o	: des_issweak.c
des_isweak.o	: des_isweak.c
des_sched.o	: des_sched.c
des_nkey.o	: des_nkey.c
des_ofb8.o	: des_ofb8.c
des_passwd.o	: des_passwd.c
des_pcbc.o	: des_pcbc.c
des_print.o	: des_print.c
des_rblock.o	: des_rblock.c
des_read.o	: des_read.c
des_readhex.o	: des_readhex.c
des_rkey.o	: des_rkey.c
des_ksched.o	: des_ksched.c
des_fixkp.o	: des_fixkp.c
des_strkey.o	: des_strkey.c
des_checkkp.o	: des_checkkp.c
ivec.o		: ivec.c
parity_tab.o	: parity_tab.c
prompt.o	: prompt.c
encrypt.o	: encrypt.c
quad_cksum.o	: quad_cksum.c
read.o		: read.c
sig.o		: sig.c
tab_E.o		: tab_E.c
tab_IP.o	: tab_IP.c
tab_IPinv.o	: tab_IPinv.c
tab_P.o		: tab_P.c
tab_PC1.o	: tab_PC1.c
tab_PC2.o	: tab_PC2.c
tab_S.o		: tab_S.c
tab_S_and_P.o	: tab_S_and_P.c
tab_parity.o	: tab_parity.c
tty.o		: tty.c


# DEPEND
#
# Don not change the lines below, run make depend
#
