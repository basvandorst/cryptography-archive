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

#.py.c:	; pepy	-o $@	$*.py



ICASN1      = -I$(ICINCLPATH)/asn1
ICUL        = -I$(ICINCLPATH)/ul
INCLISODE   = -I$(ICINCLPATH) $(ICASN1) $(ICUL)


DAFDB =
DINITDEVSET  = -DINITDEVSET
DSECSCTEST   = #-DSECSCTEST

OBJS    =       SEC_tables.o sec_init.o secure.o sec_ed.o sec_rand.o  \
                secsc_init.o secsc.o readconf.o               
                              

all: $(LIBSECUDE)


sec_init.o: sec_init.c
	$(CC) -c sec_init.c $(CFLAGS) $(DFLAGS) $(IFLAGS) $(DVERSION) 

secsc.o: secsc.c
	$(CC) -c secsc.c $(CFLAGS) $(DFLAGS) $(IFLAGS) $(DINITDEVSET)

sec_rand.o: sec_rand.c
	$(CC) -c sec_rand.c $(CFLAGS) $(DFLAGS) $(IFLAGS) -DWLNG32

readconf.o: readconf.c
	$(CC) -c readconf.c $(CFLAGS) $(DFLAGS) $(IFLAGS) $(DSCINIT) 

