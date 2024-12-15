#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	tftp
CFLAGS= -g -DINET6
#CFLAGS= -g 
LDADD = -L/usr/inet6/lib -linet6

SRCS=	main.c tftp.c tftpsubs.c

.include <bsd.prog.mk>
