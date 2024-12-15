#	@(#)Makefile	8.1 (Berkeley) 6/5/93

PROG=	ifconfig
MAN8=	ifconfig.0
CFLAGS+= -DINET6 -g -I../../sys
LDADD= -L/usr/inet6/lib -linet6

.include <bsd.prog.mk>
