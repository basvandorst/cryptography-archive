#	@(#)Makefile	8.2 (Berkeley) 12/15/93

PROG=	telnetd
CFLAGS+=-g -I../../sys -DINET6
CFLAGS+=-DLINEMODE -DKLUDGELINEMODE -DUSE_TERMIO -DDIAGNOSTICS
CFLAGS+=-DOLD_ENVIRON -DENV_HACK
#CFLAGS+=-DAUTHENTICATION -DENCRYPTION -I${.CURDIR}/../../lib
CFLAGS+=-DAUTHENTICATION -DENCRYPTION -I/usr/src/lib 
CFLAGS+=-DDEFAULT_IM="\"\r\n\r\n4.4 BSD UNIX (%h) (%t) (IPv6 v0.1)\r\n\r\r\n\r\""
SRCS=	authenc.c global.c slc.c state.c sys_term.c telnetd.c \
	termstat.c utility.c
DPADD=	${LIBUTIL} ${LIBTERM}
LDADD=	-g -lutil -ltermcap -ltelnet
LDADD+=	-lkrb -ldes
# temporary until libc get updated to correct bugs in ipv6 support functions
LDADD+= -L/usr/inet6/lib -linet6
MAN8=	telnetd.0

# These are the sources that have encryption stuff in them.
CRYPT_SRC= authenc.c ext.h state.c telnetd.c termstat.c
CRYPT_SRC+= utility.c Makefile
NOCRYPT_DIR=${.CURDIR}/Nocrypt

.include <bsd.prog.mk>

nocrypt:
#ifdef	ENCRYPTION
	@for i in ${CRYPT_SRC}; do \
	    if [ ! -d ${NOCRYPT_DIR} ]; then \
		echo Creating subdirectory ${NOCRYPT_DIR}; \
		mkdir ${NOCRYPT_DIR}; \
	    fi; \
	    echo ${NOCRYPT_DIR}/$$i; \
	    unifdef -UENCRYPTION ${.CURDIR}/$$i | \
		sed "s/ || defined(ENCRYPTION)//" > ${NOCRYPT_DIR}/$$i; \
	done

placeholder:
#else	/* ENCRYPTION */
	@echo "Encryption code already removed."
#endif	/* ENCRYPTION */
