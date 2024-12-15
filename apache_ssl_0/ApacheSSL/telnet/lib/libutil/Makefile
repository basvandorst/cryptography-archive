#	@(#)Makefile	5.10 (Berkeley) 5/6/91

LIB=	util
#CFLAGS= -DLIBC_SCCS -I/sys -I../libbsd/include 
CFLAGS = -DLIBC_SCCS -I../libbsd/include 
OBJS=	daemon.o getloadavg.o login.o login_tty.o logout.o logwtmp.o \
	pwcache.o

lib${LIB}.a: ${OBJS}
	echo building standard ${LIB} library 
	rm -f lib${LIB}.a 
	if [ -f /usr/bin/lorder ]; then \
		ar cq lib${LIB}.a `lorder ${OBJS} | tsort`  ; \
	else \
		ar cq lib${LIB}.a ${OBJS}; \
	fi;
	if [ -f /usr/bin/ranlib ]; then \
		ranlib lib${LIB}.a; \
	fi;

clean:
	rm -f *.o lib${LIB}.a
