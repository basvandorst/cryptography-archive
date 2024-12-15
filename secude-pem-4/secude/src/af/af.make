#
#  SecuDE Release 4.3 (GMD)
#
#
#    Copyright (C) 1994, GMD Darmstadt
#
#                         NOTICE
#
#    Acquisition, use, and distribution of this module 
#    and related materials are subject to restrictions 
#    mentioned in each volume of the documentation.
#

#.SUFFIXES:	.py .c

#.py.c:	;	pepy -o $@	$*.py


ICQUIPU     = -I$(ICINCLPATH)/quipu
ICASN1      = -I$(ICINCLPATH)/asn1
ICLL        = -I$(ICINCLPATH)/ll
ICUL        = -I$(ICINCLPATH)/ul
ICCOMPAT    = -I$(ICINCLPATH)/compat

INCLISODE   = -I$(ICINCLPATH) $(ICQUIPU) $(ICASN1) $(ICLL) $(ICUL) $(ICCOMPAT)
DFLAGS      = $(DX500) $(DSTRONG) $(DCOSINE) $(DAFDB) $(DSCA) $(DISODE80) $(DICR1)

OBJS    =       SECIF_tables.o CADB_tables.o SECAF_tables.o STR_tables.o \
	        af_init.o str_init.o af_ed.o af_cadb.o cadb_ed.o \
		af_crypt.o af_util.o af_pse.o \
                af_dir.o af_dirop.o af_sntx.o if_init.o \
		str_int.o str_ed.o str_print.o str_free.o

all: $(LIBSECUDE)

STR_tables.o: STRONG_tables.c
	$(CC) STRONG_tables.c -c -o STR_tables.o $(CFLAGS) $(DFLAGS) $(IFLAGS) $(CPPFLAGS)

af_crypt.o: af_crypt.c
	$(CC) af_crypt.c -c $(CFLAGS) $(DFLAGS) $(IFLAGS) $(DTIME)

af_dir.o:	af_dir.c
	$(CC) af_dir.c -c $(CFLAGS) $(DFLAGS) $(IFLAGS) -DPEPYPATH

af_sntx.o:	af_sntx.c
	$(CC) af_sntx.c -c $(CFLAGS) $(DFLAGS) $(IFLAGS) -DPEPYPATH

af_cadb.o:	af_cadb.c
	$(CC) af_cadb.c -c $(CFLAGS) $(DFLAGS) $(IFLAGS) $(DBMVERS) 

# Strong Authentication
str_int.o:	str_int.c
	$(CC) str_int.c -c $(CFLAGS) $(DFLAGS) $(IFLAGS) -DPEPYPATH
