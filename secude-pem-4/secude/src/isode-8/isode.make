###############################################################################
#
# Revision 7.0  89/11/23  21:25:38  mrose
# Release 6.0
# 
###############################################################################

###############################################################################
#
#				 NOTICE
#
#    Acquisition, use, and distribution of this module and related
#    materials are subject to the restrictions of a license agreement.
#    Consult the Preface in the User's Manual for the full terms of
#    this agreement.
#
###############################################################################

INCLISODE   = -I$(DD)../include/isode -I$(DD)../include/isode/quipu -I$(DD)../include/isode/pepsy

###############################################################################
# Options
###############################################################################


#OPTIONS	=	-I. -I../include/isode


###############################################################################
# Programs and Libraries
###############################################################################

MAKE	=	./make DESTDIR=$(DESTDIR) $(MFLAGS) -k
SHELL	=	/bin/sh

###############################################################################
# Generation Rules for library modules
###############################################################################

.c.o:;		$(CC) $*.c $(CFLAGS) $(OPTIONS) -c 


OBJS    = pe_cpy.o pe_alloc.o pe_free.o pe_error.o pe2ps.o pe2pl.o pe2text.o qbuf2pe_f.o \
	  ps_alloc.o ps_free.o ps_get_abs.o ps_error.o ps_prime.o ps2pe.o ps_io.o \
          ps_flush.o str2ps.o chrcnv.o oid_cpy.o seq_add.o set_add.o set_find.o \
          set_addon.o enc.o dec.o prim2num.o num2prim.o prim2bit.o bit2prim.o prim2str.o \
          str2prim.o prim2qb.o qb2prim.o prim2oid.o obj2prim.o prim2flag.o flag2prim.o \
          bitstr2strb.o strb2bitstr.o bit_ops.o qb_free.o str2qb.o qb2str.o util.o pl_tables.o \
          UNIV_tables.o py_advise.o asprintf.o rcmd_srch.o serror.o smalloc.o


all: $(LIBSECUDE)

