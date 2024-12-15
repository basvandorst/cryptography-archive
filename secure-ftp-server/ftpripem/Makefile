#	not even close to @(#)Makefile	5.15 (Berkeley) 7/1/90

CC = cc
CFLAGS= -I. -DSETPROCTITLE -O
SRCS=	ftpd.c ftpcmd.c glob.c logwtmp.c popen.c vers.c parsit.c normstr.c strerror.c strcasecmp.c
OBJS=	ftpd.o ftpcmd.o glob.o logwtmp.o popen.o vers.o parsit.o normstr.o strerror.o strcasecmp.o

all: ftpd

ftpd: ${OBJS}
	${CC} -o $@ $(CFLAGS) ${OBJS} 
	strip $@
 
#	${CC} -s -object -o $@ ${OBJS} -lsys_s

install: ftpd
	rm -f /usr/etc/in.ftpd
	cp ftpd /usr/etc/in.ftpd
	
#	rm -f /usr/man/man8/ftpd.8
#	cp ftpd.8 /usr/man/man8

clean:
	rm -f ${OBJS} ftpd ftpcmd.c y.tab.h
