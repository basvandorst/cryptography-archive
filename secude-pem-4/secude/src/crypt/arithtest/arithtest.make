#
#  SecuDE Release 4.3 (GMD)
#
#
#    Copyright (C) 1993, GMD Darmstadt
#
#                         NOTICE
#
#    Acquisition, use, and distribution of this module 
#    and related materials are subject to restrictions 
#    mentioned in each volume of the documentation.
#
CPPFLAGS     = $(DARITHTEST) $(DWLNG)
PGM     = mk_trace
DD      = ../


all: mk_trace


#mk_trace.o: mk_trace.c
#	$(CC) mk_trace.c -c $(CFLAGS) $(DARITHTEST) $(DWLNG)

mk_trace: mk_trace.o
	$(CC) mk_trace.o -o $@  $(LFLAGS) $(LIB)
