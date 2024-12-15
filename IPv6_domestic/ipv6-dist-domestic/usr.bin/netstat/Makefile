#	@(#)Makefile	8.1 (Berkeley) 6/12/93

PROG=	netstat
SRCS=	if.c inet.c iso.c main.c mbuf.c mroute.c ns.c route.c \
	tp_astring.c unix.c
CFLAGS+=-I../../sys
CFLAGS+=-g -DINET6 
.PATH:	${.CURDIR}/../../sys/netiso
BINGRP=	kmem
BINMODE=2555
LDADD=	-lkvm -L/usr/inet6/lib -linet6
DPADD=	${LIBKVM}

.include <bsd.prog.mk>

