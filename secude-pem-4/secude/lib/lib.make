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
#    Make shared libraries:

DD =

all: secudevrsn.c

clean:
	rm -f libsecude* *.o secudevrsn.c *% core

secudevrsn.c:    libsecude.a
	./version.sh secude > $@
	$(CC) -c secudevrsn.c
	ar ruv libsecude.a secudevrsn.o
	ranlib libsecude.a
	touch secudevrsn.c
	ar x libsecude.a
	@make-lib.sh -major `cat version.major``cat version.minor` \
	-minor `cat version.local` -options "$(SL_OPTIONS)" -appendix "$(SL_APPENDIX)"
	rm -f *.o *SYM*

