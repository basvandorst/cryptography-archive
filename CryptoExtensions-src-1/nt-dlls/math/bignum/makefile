# this was written for watcom 'wmake'.

#
# Copyright (C) 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

LIB_NAME = bignum
JAVA_CLASS_NAME = java.math.BigNum
CLASS_NAME = java_math_BigNum
EXTRA_LIB_NAME=bignum.lib

CC = wcc386
CC_INCLUDE = C:\apps\watcom\h;C:\apps\watcom\h\nt;c:\apps\java\include;c:\apps\java\include\win32
CC_OPTIONS = -i=$(CC_INCLUDE) -bd -mf -3s -bt=nt
LINK = wlink
LINK_FLAGS = SYS nt_dll op m d a op st=8192 op maxe=25 op q op symf 
LINK_EXTRAS = NAME $(LIB_NAME) FIL $(CLASS_NAME)_4watcom, $(CLASS_NAME)_class, $(EXTRA_LIB_NAME)
LINK_OPTIONS = $(LINK_FLAGS) $(LINK_EXTRAS)

$(LIB_NAME).dll: $(CLASS_NAME)_4watcom.obj $(CLASS_NAME)_class.obj $(EXTRA_LIB_NAME)
    $(LINK) $(LINK_OPTIONS)

$(CLASS_NAME)_4watcom.obj : $(CLASS_NAME)_4watcom.c
    $(CC) $(CC_OPTIONS) $(CLASS_NAME)_4watcom.c 

$(CLASS_NAME)_class.obj : $(CLASS_NAME)_class.c
    $(CC) $(CC_OPTIONS) $(CLASS_NAME)_class.c

$(CLASS_NAME)_4watcom.c : $(CLASS_NAME).c $(CLASS_NAME).h
	perl conv4watcom.pl $(CLASS_NAME).c > $(CLASS_NAME)_4watcom.c

$(CLASS_NAME).h :
	javah $(JAVA_CLASS_NAME)

$(CLASS_NAME).c :
	javah -stubs $(JAVA_CLASS_NAME)

