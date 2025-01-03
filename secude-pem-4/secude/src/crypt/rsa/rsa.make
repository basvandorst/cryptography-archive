#
#  SecuDE Release 4.3 (GMD)
#
#
#    Copyright (C) 1991, GMD Darmstadt
#
#                         NOTICE
#
#    Acquisition, use, and distribution of this module 
#    and related materials are subject to restrictions 
#    mentioned in each volume of the documentation.
#

DFLAGS      =


OBJS    = rsablock.o rsa.o rsagen.o 
DAFDB =
DD      = ../
DFLAGS  = $(DWLNG) $(DARITHTEST) -DPAD $(DISODE80) $(DICR1) # -DTEST

all: $(LIBSECUDE)

trace:	; ./make "OPT=$(OPT) -DTRACE"

rsablock.o:     rsa.h rsablock.c
rsa.o:          rsa.h rsa.c
rsagen.o:       rsa.h rsa_debug.h rsagen.c

