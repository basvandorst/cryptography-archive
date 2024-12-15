#	BSDI	Makefile,v 2.1 1995/02/03 18:13:30 polk Exp

PROG=	tcpdump
SRCS=	tcpdump.c addrtoname.c \
	print-ether.c print-ip.c print-arp.c print-tcp.c print-udp.c \
	print-atalk.c print-domain.c print-tftp.c print-bootp.c print-nfs.c \
	print-icmp.c print-sl.c print-ppp.c print-rip.c \
	print-snmp.c print-ntp.c print-null.c print-egp.c print-ospf.c \
	print-fddi.c print-llc.c print-sunrpc.c \
	print-wb.c print-decnet.c print-isoclns.c print-ipx.c \
	util.c bpf_dump.c parsenfsfh.c version.c print-ipv6.c print-icmpv6.c
CFLAGS+=-I. -I${.CURDIR} -I${.CURDIR}/../libpcap -DINET6 -g 
CFLAGS+=-DCSLIP -DPPP -DFDDI
LDADD+=	${.CURDIR}/../libpcap/obj/libpcap.a -L/usr/inet6/lib -linet6
DPADD+=	${.CURDIR}/../libpcap/obj/libpcap.a
CLEANFILES+=os.h md.h

os.h:
	-rm -f os.h
	ln -s ${.CURDIR}/os-bsd.h os.h

md.h:
	-rm -f md.h
	ln -s ${.CURDIR}/md-${MACHINE}.h md.h

.depend: os.h md.h

.include <bsd.prog.mk>
