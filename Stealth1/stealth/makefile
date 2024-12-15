#
# Stealth V1.2
#

OFILES = stealth.o frig.o conv.o
TFILES = test.o frig.o conv.o
CFILES = create.o

LDFLAGS= 

#
# Currently only unix supported, define UNIX for a unix machine, USE_TMP
# to create temporary files in /tmp, and USE_PGPPATH to create them in
# the directory pointed to by the PGPPATH environment variable. Otherwise
# the will be created in the current directory.
#
# Define FRIG if you want the bit-frequency munging frig code to be
# compiled in.
#
# For MS-DOS with Microsoft C compiler, use :
#
# cl /DDOS stealth.c
#

CPPFLAGS=-DUNIX -DUSE_PGPPATH -DFRIG
CFLAGS= -O

#
# On a machine using gcc, uncomment the CC=gcc line (i.e. remove the #),
# and put a # before the CC=cc line, or take it out altogether.
#

#CC=gcc
CC=cc

stealth:	$(OFILES)
	$(CC) $(CFLAGS) $(OFILES) -o stealth $(LDFLAGS)

frigtest:	$(TFILES)
	$(CC) $(CFLAGS) $(TFILES) -o frigtest $(LDFLAGS)

create:	$(CFILES)
	$(CC) $(CFLAGS) $(CFILES) -o create $(LDFLAGS)

check:	check.o
	$(CC) $(CFLAGS) check.o -o check -lm

all:	stealth frigtest create check

conv.c:	create
	./create > conv.c

.c.o:	
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

clean:
	-rm *.o stealth create frigtest check

