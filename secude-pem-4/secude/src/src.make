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

inst-all: all

clean:
	rm -f *% *.bak
	cd config; rm -f *% *.bak
	cd af; ./make clean
	cd secure; ./make clean
	cd aux; ./make clean
	cd crypt; make clean
	cd sca; ./make clean $(DSCA)
	cd isode; ./make clean $(DISODE)
	cd pem; ./make clean
	cd crypt/arithtest; ./make clean $(DARITHTEST)
	cd util; ./make clean $(DXPIPE) $(DSECXLOCK) $(DMOTIFSECTOOL)
	cd ../lib; ./make clean
	cd ../bin; rm -f [a-z]*

all:
	cd pepsy; ./make
	cd secure; ./make
	cd af; ./make
	cd aux; ./make
	cd crypt; make
	cd sca; ./make all $(DSCA)
	cd isode; ./make all $(DISODE)
	cd pem; ./make
	cd ../lib; ./make all $(LIBIMPORTS)
	cd crypt/arithtest; ./make $(DARITHTEST)
	cd util; ./make all $(DXPIPE) $(DSECXLOCK) $(DMOTIFSECTOOL)

lib:
	cd secure; ./make lib
	cd af; ./make lib
	cd aux; ./make lib
	cd crypt; make lib
	cd sca; ./make lib $(DSCA)
	cd isode; ./make lib $(DISODE)
	cd pem; ./make lib
	cd ../lib; ./make all $(LIBIMPORTS)


