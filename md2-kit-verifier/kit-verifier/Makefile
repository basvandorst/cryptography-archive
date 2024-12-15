DBG=-O
DEBUG = -DDEBUG
DEFINES	= -DBSD42 -DULTRIX
CFLAGS=$(DBG) $(DEBUG) $(INCLUDE) $(DEFINES)
LINTFLAGS=$(INCLUDE) $(DEFINES)

SRCTOP=..

DESTDIR=

SVRDIR= /usr/etc
LINTLIBDIR = /usr/lib/lint
BINDIR= /usr/bin
UCBDIR= /usr/ucb
ETCDIR= /etc
DAEMDIR= /usr/etc
SLASHBINDIR= /bin
INCLDIR= /usr/include
MANDIR= /usr/man

CC=cc

ARCHIVE= ar cru
RANLIB= ranlib

MAKE=make
RM=rm -f
MV=mv
CP=cp
LN=ln -s
CHMOD=chmod

SRCS	= \
	md2.c \
	md2-file.c

OBJECTS	= \
	md2.o \
	md2-file.o

.c.o:
	$(CC) -p -c $(CFLAGS) $*.c

all:: md2-file

md2-file:  md2-file.o md2.o
	$(CC) -o $@ $@.o md2.o -lc

# DO NOT DELETE THIS LINE

