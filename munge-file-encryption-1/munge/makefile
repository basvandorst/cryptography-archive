# -DHIGHFIRST for big endian 32 bit machines. For little endian 32 bit
#  machines leave this out.
STRIP = -s
# AIX
#CFLAGS = -g -DUSE_TERMIOS -DHIGHFIRST -DUNIX -D_AIX
CFLAGS = -O -Q=75 -DUSE_TERMIOS -DHIGHFIRST -DUNIX -D_AIX
INSTDIR = /usr/local/bin

# Irix 4.x
#CFLAGS = -cckr -O -DUSE_TERMIOS -DHIGHFIRST -DUNIX -Dsgi
#INSTDIR = /usr/local/bin

# sun os 4.x
# build unproto first!
#CFLAGS = -O4 -DUSE_TERMIOS -DHIGHFIRST -DUNIX -Qpath unproto
#INSTDIR = /usr/local/bin

# hpux 8.x
#CC = c89 -D_HPUX_SOURCE -D_POSIX_SOURCE
#CFLAGS = +O3 -DUSE_TERMIOS -DHIGHFIRST -DUNIX -Dhpux
#INSTDIR = /usr/local/bin

# linux
OPT = -O6 -fomit-frame-pointer
#DEBUG = -g
#CFLAGS = $(OPT) $(DEBUG) -DUSE_TERMIOS -DUNIX -DSHORTNAMES
#INSTDIR = /usr/local/bin

munge: idea.o md5.o crypto.o main.o passwd.o d3des.o 
	$(CC) $(STRIP) -o munge $(CFLAGS) idea.o md5.o crypto.o main.o passwd.o d3des.o

rgr: md5.o rgrun.o
	$(CC) -s -o rgr $(CFLAGS) md5.o rgrun.o

magic.aix: magic
	te @magic.tec

#do make clean first
sign: 
	rm -f SIGN SIGN.raw
	md5 * >SIGN.raw
	date >>SIGN.raw
	ripem -e -m mic-clear <SIGN.raw >SIGN
	rm -f SIGN.raw

test: munge
	sh test.sh

md5.o: md5.h md5.c
	$(CC) -c $(CFLAGS) md5.c

rgrun.o: rgrun.c
	$(CC) -c $(CFLAGS) rgrun.c

d3des.o: d3des.h d3des.c
	$(CC) -c $(CFLAGS) d3des.c

idea.o: idea.c
	$(CC) -c $(CFLAGS) idea.c

crypto.o: crypto.c 
	$(CC) -c $(CFLAGS) crypto.c

main.o: main.c
	$(CC) -c $(CFLAGS) main.c

passwd.o: passwd.c
	$(CC) -c $(CFLAGS) passwd.c

clean:
	rm -f *.o core idea cscope.out test munge m666
	(cd unproto; make clean)

install:
	rm -f $(INSTDIR)/munge $(INSTDIR)/mcat $(INSTDIR)/unmunge
	cp munge $(INSTDIR)
	chmod 755 $(INSTDIR)/munge
	ln $(INSTDIR)/munge $(INSTDIR)/mcat
	ln $(INSTDIR)/munge $(INSTDIR)/unmunge
