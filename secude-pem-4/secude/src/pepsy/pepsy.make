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

#.SUFFIXES:	.py .c

#.py.c:	;	pepy -o $@	$*.py


all: SECAF_tables.c SECIF_tables.c CADB_tables.c STRONG_tables.c SEC_tables.c


SECAF_tables.c:  af.py
	env PEPSYPATH=.:../secure pepsy -A -f -h af.py
	mv SECAF-types.h ../include
	cp SECAF_tables.c ../af

SECIF_tables.c:  if.py
	env PEPSYPATH=.:../secure pepsy -A -f -h if.py
	mv SECIF-types.h ../include
	cp SECIF_tables.c ../af

CADB_tables.c:  cadb.py
	env PEPSYPATH=.:../secure pepsy -A -f -h cadb.py
	mv CADB-types.h ../include
	cp CADB_tables.c ../af

STRONG_tables.c:  strong.py
	env PEPSYPATH=.:../secure pepsy -A -f -h strong.py
	/bin/sed -f sedscript <STRONG_tables.c >STRONG_tables.c.sed
	cp STRONG_tables.c.sed STRONG_tables.c
	rm STRONG_tables.c.sed
	mv STRONG-types.h ../include
	cp STRONG_tables.c ../af

SEC_tables.c:  sec.py
	env PEPSYPATH=.:../af pepsy -A -f -h sec.py
	mv SEC-types.h ../include
	cp SEC_tables.c ../secure
	
