
CC      = gcc
AR	= ar
RANLIB	= ranlib

CFLAGS  = -I. -O #-Wall
LDFLAGS = $(CFLAGS) -s
ARFLAGS = rc

LIB     = mdigest.a
LIB_OBJS= mdigest.o utility.o \
	  md4.o md4-if.o \
	  md5.o md5-if.o \
	  shs.o shs-if.o \
	  haval.o haval-if.o

# ---

all: $(LIB) mfile mtest

# ---

$(LIB): $(LIB_OBJS)
	rm -f $(LIB)
	$(AR) $(ARFLAGS) $(LIB) $(LIB_OBJS)
	$(RANLIB) $(LIB)

mfile: $(LIB) mfile.o
	$(CC) $(LDFLAGS) -o mfile mfile.o $(LIB)

mtest: $(LIB) mtest.o
	$(CC) $(LDFLAGS) -o mtest mtest.o $(LIB)

# ---

clean:
	rm -f $(LIB_OBJS) mfile.o mtest.o a.out core

clobber: clean
	rm -f $(LIB) mfile mtest

