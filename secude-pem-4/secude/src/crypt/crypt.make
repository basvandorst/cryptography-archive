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

inst-all: all

all:
	cd des; ./make
	cd idea; ./make
	cd md; ./make
	cd sha; ./make
	cd rsa; ./make
	cd dsa; ./make
	cd dh; ./make
	cd arithmetic; ./make

clean:
	cd des; ./make clean
	cd idea; ./make clean
	cd md; ./make clean
	cd sha; ./make clean
	cd rsa; ./make clean
	cd dsa; ./make clean
	cd dh; ./make clean
	cd arithmetic; ./make clean

lib:
	cd des; ./make lib
	cd idea; ./make lib
	cd md; ./make lib
	cd sha; ./make lib
	cd rsa; ./make lib
	cd dsa; ./make lib
	cd dh; ./make lib
	cd arithmetic; ./make lib

