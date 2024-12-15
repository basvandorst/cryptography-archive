# this was written for watcom 'wmake'.

#
# Copyright (C) 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

LIB_NAME = des
JAVA_CLASS_NAME = java.crypt.DES
CLASS_NAME = java_crypt_DES
CODE_NAME = des
HEADER_NAME = des.h

CC = wcc386
CC_INCLUDE = C:\apps\watcom\h;C:\apps\watcom\h\nt;c:\apps\java\include;c:\apps\java\include\win32
CC_OPTIONS = -i=$(CC_INCLUDE) -bd -mf -3s -bt=nt
LINK = wlink
LINK_FLAGS = SYS nt_dll op m d a op st=8192 op maxe=25 op q op symf 
LINK_EXTRAS = NAME $(LIB_NAME) FIL $(CLASS_NAME)_4watcom, $(CODE_NAME), $(CLASS_NAME)_class
LINK_OPTIONS = $(LINK_FLAGS) $(LINK_EXTRAS)

$(LIB_NAME).dll: $(CLASS_NAME)_4watcom.obj $(CODE_NAME).obj $(CLASS_NAME)_class.obj
    $(LINK) $(LINK_OPTIONS)

$(CLASS_NAME)_4watcom.obj : $(CLASS_NAME)_4watcom.c
    $(CC) $(CC_OPTIONS) $(CLASS_NAME)_4watcom.c 

$(CODE_NAME).obj : $(CODE_NAME).c $(HEADER_NAME)
    $(CC) $(CC_OPTIONS) $(CODE_NAME).c

$(CLASS_NAME)_class.obj : $(CLASS_NAME)_class.c $(HEADER_NAME)
    $(CC) $(CC_OPTIONS) $(CLASS_NAME)_class.c

$(CLASS_NAME)_4watcom.c : $(CLASS_NAME).c $(CLASS_NAME).h
	perl conv4watcom.pl $(CLASS_NAME).c > $(CLASS_NAME)_4watcom.c

$(CLASS_NAME).h :
	javah $(JAVA_CLASS_NAME)

$(CLASS_NAME).c :
	javah -stubs $(JAVA_CLASS_NAME)

