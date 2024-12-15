#	@(#)Makefile	8.1 (Berkeley) 6/4/93

PROG=	tftpd
SRCS=	tftpd.c tftpsubs.c
MAN8=	tftpd.0
CFLAGS+=-g -DDEBUG -DINET6 -I${.CURDIR}/../../usr.bin/tftp
.PATH:	${.CURDIR}/../../usr.bin/tftp
LDADD = -L/usr/inet6/lib -linet6

.include <bsd.prog.mk>
