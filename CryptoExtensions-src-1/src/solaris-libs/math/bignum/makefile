#
# Copyright (C) 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

LIB_NAME = bignum
JAVA_CLASS_NAME = java.math.BigNum
CLASS_NAME = java_math_BigNum
EXTRA_LIB_NAME = lib/bignum.a
SHARED_LIB_NAME = lib$(LIB_NAME).so
JAVA_ROOT = /home/apps/java

FILES=$(CLASS_NAME).o $(CLASS_NAME)_class.o $(EXTRA_LIB_NAME)

CC = gcc
CC_INCLUDE = -I$(JAVA_ROOT)/include -I$(JAVA_ROOT)/include/solaris
LINK = gcc
LINK_OPTIONS = -shared

all: $(SHARED_LIB_NAME)

.c.o :
	$(CC) -c $(CC_INCLUDE) $< 

$(CLASS_NAME).c : $(CLASS_NAME).h
	javah -stubs $(JAVA_CLASS_NAME)

$(CLASS_NAME).h :
	javah $(JAVA_CLASS_NAME)

$(SHARED_LIB_NAME): $(FILES)
	$(LINK) $(LINK_OPTIONS) -o $(SHARED_LIB_NAME) $(FILES)
clean:
	rm *.o *.so $(CLASS_NAME).c $(CLASS_NAME).h
