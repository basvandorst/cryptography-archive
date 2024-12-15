#	BSDI	Makefile,v 2.1 1995/02/03 18:11:35 polk Exp

LIB=	pcap
SRCS=	bpf_filter.c bpf_image.c etherent.c gencode.c \
	grammar.y inet.c nametoaddr.c optimize.c pcap-bpf.c pcap.c \
	savefile.c scanner.l
CFLAGS+=-DINET6 -I${.CURDIR} -I. -g
CLEANFILES+=tokdefs.h grammar.c
NOPROFILE=noprofile

install:

tokdefs.h: grammar.c

grammar.c: grammar.y
	rm -f grammar.c tokdefs.h
	yacc -d $<
	mv y.tab.c grammar.c
	mv y.tab.h tokdefs.h

grammar.o: grammar.c
	$(CC) -c $(CFLAGS) -Dyylval=pcap_lval grammar.c

.depend: tokdefs.h

.include <bsd.lib.mk>
