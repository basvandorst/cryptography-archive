#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	inetd
MAN8=	inetd.0
MLINKS=	inetd.8 inetd.5
CFLAGS+=-g -DOFFSETPORT -DINET6 -I../../sys
LDFLAGS+=-L/usr/inet6/lib -linet6
TARGETS=inetd

inetd: inetd.c
	cc $(CFLAGS) inetd.c $(LDFLAGS) -o inetd

.include <bsd.prog.mk>
