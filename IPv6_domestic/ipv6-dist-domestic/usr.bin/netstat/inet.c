/*----------------------------------------------------------------------
 inet.c:    netstat subroutines for AF_INET/AF_INET6.

 Originally from 4.4-Lite BSD.  Modifications to support IPv6 and IPsec
 copyright by Bao Phan, Randall Atkinson, & Dan McDonald, All Rights Reserved.
 All rights under this copyright are assigned to NRL.
----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
#	@(#)COPYRIGHT	1.1 (NRL) 17 January 1995

COPYRIGHT NOTICE

All of the documentation and software included in this software
distribution from the US Naval Research Laboratory (NRL) are
copyrighted by their respective developers.

Portions of the software are derived from the Net/2 and 4.4 Berkeley
Software Distributions (BSD) of the University of California at
Berkeley and those portions are copyright by The Regents of the
University of California. All Rights Reserved.  The UC Berkeley
Copyright and License agreement is binding on those portions of the
software.  In all cases, the NRL developers have retained the original
UC Berkeley copyright and license notices in the respective files in
accordance with the UC Berkeley copyrights and license.

Portions of this software and documentation were developed at NRL by
various people.  Those developers have each copyrighted the portions
that they developed at NRL and have assigned All Rights for those
portions to NRL.  Outside the USA, NRL has copyright on some of the
software developed at NRL. The affected files all contain specific
copyright notices and those notices must be retained in any derived
work.

NRL LICENSE

NRL grants permission for redistribution and use in source and binary
forms, with or without modification, of the software and documentation
created at NRL provided that the following conditions are met:

1. All terms of the UC Berkeley copyright and license must be followed.
2. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
3. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
4. All advertising materials mentioning features or use of this software
   must display the following acknowledgements:

	This product includes software developed by the University of
	California, Berkeley and its contributors.

	This product includes software developed at the Information
	Technology Division, US Naval Research Laboratory.

5. Neither the name of the NRL nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THE SOFTWARE PROVIDED BY NRL IS PROVIDED BY NRL AND CONTRIBUTORS ``AS
IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL NRL OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation
are those of the authors and should not be interpreted as representing
official policies, either expressed or implied, of the US Naval
Research Laboratory (NRL).

----------------------------------------------------------------------*/


/*
 * Copyright (c) 1983, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)inet.c	8.4 (Berkeley) 4/20/94";
#endif /* not lint */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>

#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>

#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/igmp_var.h>
#include <netinet/ip_var.h>

#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_seq.h>
#define TCPSTATES
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_debug.h>

#include <netinet/udp.h>
#include <netinet/udp_var.h>

#ifdef INET6
#include <netinet6/in6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h>
#include <netinet6/icmpv6_var.h>

#include <netinet6/ipsec.h>
#endif INET6

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "netstat.h"

struct	inpcb inpcb;
struct	tcpcb tcpcb;
struct	socket sockb;

char	*inetname __P((struct in_addr *));

#ifdef INET6
void	inetprint __P((int, struct in_addr *, int, char *));
char	*inet6name __P((struct in_addr6 *));
#else
void	inetprint __P((struct in_addr *, int, char *));
#endif /* INET6 */

/*
 * Print a summary of connections related to an Internet
 * protocol.  For TCP, also give state of connection.
 * Listening processes (aflag) are suppressed unless the
 * -a (all) flag is specified.
 */
void
protopr(off, name)
	u_long off;
	char *name;
{
	struct inpcb cb; 
	register struct inpcb *prev, *next;
	int istcp;
	static int first = 1;
#ifdef INET6
	int v6 = 0;
#endif /* INET6 */

	if (off == 0)
		return;
	istcp = strcmp(name, "tcp") == 0;
	kread(off, (char *)&cb, sizeof (struct inpcb));
	inpcb = cb;
	prev = (struct inpcb *)off;
	if (inpcb.inp_next == (struct inpcb *)off)
		return;
	while (inpcb.inp_next != (struct inpcb *)off) {
		next = inpcb.inp_next;
		kread((u_long)next, (char *)&inpcb, sizeof (inpcb));
		if (inpcb.inp_prev != prev) {
			printf("???\n");
			break;
		}
#ifdef INET6
		v6 = (inpcb.inp_flags & INP_IPV6) && 
		  !(inpcb.inp_flags & INP_IPV6_MAPPED);

		if (!aflag && ((v6 && IS_IN6_UNSPEC(inpcb.inp_laddr6))
		  || (!v6 && (inet_lnaof(inpcb.inp_laddr) == INADDR_ANY)))) {
#else
		if (!aflag &&
		  inet_lnaof(inpcb.inp_laddr) == INADDR_ANY) { 
#endif /* INET6 */
			prev = next;
			continue;
		}
		kread((u_long)inpcb.inp_socket, (char *)&sockb, sizeof (sockb));
		if (istcp) {
			kread((u_long)inpcb.inp_ppcb,
			    (char *)&tcpcb, sizeof (tcpcb));
		}
		if (first) {
			printf("Active Internet connections");
			if (aflag)
				printf(" (including servers)");
			putchar('\n');
			if (Aflag)
				printf("%-8.8s ", "PCB");
			printf(Aflag ?
				"%-6.6s %-6.6s %-6.6s  %-18.18s %-18.18s %s\n" :
				"%-6.6s %-6.6s %-6.6s  %-22.22s %-22.22s %s\n",
				"Proto", "Recv-Q", "Send-Q",
				"Local Address", "Foreign Address", "(state)");
			first = 0;
		}
		if (Aflag)
			if (istcp)
				printf("%8x ", inpcb.inp_ppcb);
			else
				printf("%8x ", next);
#ifdef INET6
		if (v6) {
		  printf("%-3.3s(6) %6d %6d ", name, sockb.so_rcv.sb_cc,
			sockb.so_snd.sb_cc);
		} else
#endif
		printf("%-6.6s %6d %6d ", name, sockb.so_rcv.sb_cc,
			sockb.so_snd.sb_cc);
#ifdef INET6
		inetprint(v6 ? AF_INET6 : AF_INET, v6 ? 
		      (struct in_addr *)&inpcb.inp_laddr6 : 
		      &inpcb.inp_laddr, (int)inpcb.inp_lport, name);
		inetprint(v6 ? AF_INET6 : AF_INET, v6 ? 
		      (struct in_addr *)&inpcb.inp_faddr6 : 
		      &inpcb.inp_faddr, (int)inpcb.inp_fport, name);
#else
		inetprint(&inpcb.inp_laddr, (int)inpcb.inp_lport, name);
		inetprint(&inpcb.inp_faddr, (int)inpcb.inp_fport, name);
#endif /* INET6 */
		if (istcp) {
			if (tcpcb.t_state < 0 || tcpcb.t_state >= TCP_NSTATES)
				printf(" %d", tcpcb.t_state);
			else
				printf(" %s", tcpstates[tcpcb.t_state]);
		}
		putchar('\n');
		prev = next;
	}
}

/*
 * Dump TCP statistics structure.
 */
void
tcp_stats(off, name)
	u_long off;
	char *name;
{
	struct tcpstat tcpstat;

	if (off == 0)
		return;
	printf ("%s:\n", name);
	kread(off, (char *)&tcpstat, sizeof (tcpstat));

#define	p(f, m) if (tcpstat.f || sflag <= 1) \
    printf(m, tcpstat.f, plural(tcpstat.f))
#define	p2(f1, f2, m) if (tcpstat.f1 || tcpstat.f2 || sflag <= 1) \
    printf(m, tcpstat.f1, plural(tcpstat.f1), tcpstat.f2, plural(tcpstat.f2))
#define	p3(f, m) if (tcpstat.f || sflag <= 1) \
    printf(m, tcpstat.f, plurales(tcpstat.f))

	p(tcps_sndtotal, "\t%d packet%s sent\n");
	p2(tcps_sndpack,tcps_sndbyte,
		"\t\t%d data packet%s (%d byte%s)\n");
	p2(tcps_sndrexmitpack, tcps_sndrexmitbyte,
		"\t\t%d data packet%s (%d byte%s) retransmitted\n");
	p2(tcps_sndacks, tcps_delack,
		"\t\t%d ack-only packet%s (%d delayed)\n");
	p(tcps_sndurg, "\t\t%d URG only packet%s\n");
	p(tcps_sndprobe, "\t\t%d window probe packet%s\n");
	p(tcps_sndwinup, "\t\t%d window update packet%s\n");
	p(tcps_sndctrl, "\t\t%d control packet%s\n");
	p(tcps_rcvtotal, "\t%d packet%s received\n");
	p2(tcps_rcvackpack, tcps_rcvackbyte, "\t\t%d ack%s (for %d byte%s)\n");
	p(tcps_rcvdupack, "\t\t%d duplicate ack%s\n");
	p(tcps_rcvacktoomuch, "\t\t%d ack%s for unsent data\n");
	p2(tcps_rcvpack, tcps_rcvbyte,
		"\t\t%d packet%s (%d byte%s) received in-sequence\n");
	p2(tcps_rcvduppack, tcps_rcvdupbyte,
		"\t\t%d completely duplicate packet%s (%d byte%s)\n");
	p(tcps_pawsdrop, "\t\t%d old duplicate packet%s\n");
	p2(tcps_rcvpartduppack, tcps_rcvpartdupbyte,
		"\t\t%d packet%s with some dup. data (%d byte%s duped)\n");
	p2(tcps_rcvoopack, tcps_rcvoobyte,
		"\t\t%d out-of-order packet%s (%d byte%s)\n");
	p2(tcps_rcvpackafterwin, tcps_rcvbyteafterwin,
		"\t\t%d packet%s (%d byte%s) of data after window\n");
	p(tcps_rcvwinprobe, "\t\t%d window probe%s\n");
	p(tcps_rcvwinupd, "\t\t%d window update packet%s\n");
	p(tcps_rcvafterclose, "\t\t%d packet%s received after close\n");
	p(tcps_rcvbadsum, "\t\t%d discarded for bad checksum%s\n");
	p(tcps_rcvbadoff, "\t\t%d discarded for bad header offset field%s\n");
	p(tcps_rcvshort, "\t\t%d discarded because packet too short\n");
	p(tcps_connattempt, "\t%d connection request%s\n");
	p(tcps_accepts, "\t%d connection accept%s\n");
	p(tcps_connects, "\t%d connection%s established (including accepts)\n");
	p2(tcps_closed, tcps_drops,
		"\t%d connection%s closed (including %d drop%s)\n");
	p(tcps_conndrops, "\t%d embryonic connection%s dropped\n");
	p2(tcps_rttupdated, tcps_segstimed,
		"\t%d segment%s updated rtt (of %d attempt%s)\n");
	p(tcps_rexmttimeo, "\t%d retransmit timeout%s\n");
	p(tcps_timeoutdrop, "\t\t%d connection%s dropped by rexmit timeout\n");
	p(tcps_persisttimeo, "\t%d persist timeout%s\n");
	p(tcps_keeptimeo, "\t%d keepalive timeout%s\n");
	p(tcps_keepprobe, "\t\t%d keepalive probe%s sent\n");
	p(tcps_keepdrops, "\t\t%d connection%s dropped by keepalive\n");
	p(tcps_predack, "\t%d correct ACK header prediction%s\n");
	p(tcps_preddat, "\t%d correct data packet header prediction%s\n");
	p3(tcps_pcbcachemiss, "\t%d PCB cache miss%s\n");
#undef p
#undef p2
#undef p3
}

/*
 * Dump UDP statistics structure.
 */
void
udp_stats(off, name)
	u_long off;
	char *name;
{
	struct udpstat udpstat;
	u_long delivered;

	if (off == 0)
		return;
	kread(off, (char *)&udpstat, sizeof (udpstat));
	printf("%s:\n", name);
#define	p(f, m) if (udpstat.f || sflag <= 1) \
    printf(m, udpstat.f, plural(udpstat.f))
	p(udps_ipackets, "\t%u datagram%s received\n");
	p(udpps_pcbcachemiss, "\t%u datagram%s missing pcb cache\n");
	p(udps_hdrops, "\t%u with incomplete header\n");
	p(udps_badlen, "\t%u with bad data length field\n");
	p(udps_badsum, "\t%u with bad checksum\n");
	p(udps_noport, "\t%u dropped due to no socket\n");
	p(udps_noportbcast, "\t%u broadcast/multicast datagram%s dropped due to no socket\n");
	p(udps_fullsock, "\t%u dropped due to full socket buffers\n");
	delivered = udpstat.udps_ipackets -
		    udpstat.udps_hdrops -
		    udpstat.udps_badlen -
		    udpstat.udps_badsum -
		    udpstat.udps_noport -
		    udpstat.udps_noportbcast -
		    udpstat.udps_fullsock;
	if (delivered || sflag <= 1)
		printf("\t%u delivered\n", delivered);
	p(udps_opackets, "\t%u datagram%s output\n");
#undef p
}

/*
 * Dump IP statistics structure.
 */
void
ip_stats(off, name)
	u_long off;
	char *name;
{
        struct ipstat ipstat;

	if (off == 0)
		return;
        kread(off, (char *)&ipstat, sizeof (ipstat));
	printf("%s:\n", name);

#define	p(f, m) if (ipstat.f || sflag <= 1) \
    printf(m, ipstat.f, plural(ipstat.f))

	p(ips_total, "\t%u total packet%s received\n");
	p(ips_badsum, "\t%u bad header checksum%s\n");
	p(ips_toosmall, "\t%u with size smaller than minimum\n");
	p(ips_tooshort, "\t%u with data size < data length\n");
	p(ips_badhlen, "\t%u with header length < data size\n");
	p(ips_badlen, "\t%u with data length < header length\n");
	p(ips_badoptions, "\t%u with bad options\n");
        p(ips_badvers, "\t%u with incorrect version number\n");
	p(ips_fragments, "\t%u fragment%s received\n");
	p(ips_fragdropped, "\t%u fragment%s dropped (dup or out of space)\n");
	p(ips_fragtimeout, "\t%u fragment%s dropped after timeout\n");
	p(ips_reassembled, "\t%u packet%s reassembled ok\n");
	p(ips_delivered, "\t%u packet%s for this host\n");
	p(ips_noproto, "\t%u packet%s for unknown/unsupported protocol\n");
	p(ips_forward, "\t%u packet%s forwarded\n");
	p(ips_cantforward, "\t%u packet%s not forwardable\n");
	p(ips_redirectsent, "\t%u redirect%s sent\n");
	p(ips_localout, "\t%u packet%s sent from this host\n");
	p(ips_rawout, "\t%u packet%s sent with fabricated ip header\n");
	p(ips_odropped, "\t%u output packet%s dropped due to no bufs, etc.\n");
	p(ips_noroute, "\t%u output packet%s discarded due to no route\n");
	p(ips_fragmented, "\t%u output datagram%s fragmented\n");
	p(ips_ofragments, "\t%u fragment%s created\n");
	p(ips_cantfrag, "\t%u datagram%s that can't be fragmented\n");
#undef p
}

#ifdef INET6
/*
 * Dump IPv6 statistics structure.
 *
 * Almost identical to IPv4 except for ips_badvers 
 * field which is not available in IPv6 and ips_badlen/ips_badhlen
 * which don't make sense given the absence of a Header Length field
 * from the IPv6 header.
 */
void
ipv6_stats(off, name)
	u_long off;
	char *name;
{
        struct ipv6stat ipv6stat;

	if (off == 0)
		return;
        kread(off, (char *)&ipv6stat, sizeof (ipv6stat));
	printf("%s:\n", name);

#define	p(f, m) if (ipv6stat.f || sflag <= 1) \
    printf(m, ipv6stat.f, plural(ipv6stat.f))

	p(ips_total, "\t%u total packet%s received\n");
	p(ips_delivered, "\t%u packet%s for this host\n");
	p(ips_forward, "\t%u packet%s forwarded\n");
	p(ips_redirectsent, "\t%u redirect%s sent\n");
	p(ips_cantforward, "\t%u packet%s not forwardable\n");

	p(ips_toosmall, "\t%u with size smaller than minimum\n");
	p(ips_tooshort, "\t%u with data size < data length\n");
	p(ips_noproto, "\t%u packet%s for unknown/unsupported protocol\n");
	p(ips_badvers, "\t%u with invalid IP version number\n");	
	p(ips_badoptions, "\t%u with bad options\n");

	p(ips_fragments, "\t%u fragment%s received\n");
	p(ips_fragdropped, "\t%u fragment%s dropped (dup or out of space)\n");
	p(ips_fragtimeout, "\t%u fragment%s dropped after timeout\n");
	p(ips_reassembled, "\t%u packet%s reassembled ok\n");

	p(ips_localout, "\t%u packet%s sent from this host\n");
	p(ips_rawout, "\t%u packet%s sent with fabricated ip header\n");
	p(ips_odropped, "\t%u output packet%s dropped due to no bufs, etc.\n");
	p(ips_noroute, "\t%u output packet%s discarded due to no route\n");
	p(ips_fragmented, "\t%u output datagram%s fragmented\n");
	p(ips_ofragments, "\t%u fragment%s created\n");
	p(ips_cantfrag, "\t%u datagram%s that can't be fragmented\n");
#undef p
}



/*
 * Dump ESP statistics structure.
 *
 * Derived from ipv6_stats() routine.
 * Includes statistics from ipsec_esp.c (ESP)
 *                      and ipsec_policy.c (In/Out policy checking).
 */
void
ESP_stat(off, name)
	u_long off;
	char *name;
{
        struct esp_stat_table ESP_stats;

	if (off == 0)
		return;
        kread(off, (char *)&ESP_stats, sizeof (ESP_stats));
	printf("%s:\n", name);

#define	p(f, m) if (ESP_stats.f || sflag <= 1) \
	printf(m, ESP_stats.f, plural(ESP_stats.f))

	p(esp_sa_unavailable, "\t%u unavailable ESP Security Associations\n");
	p(esp_sa_invalid,     "\t%u invalid     ESP Security Associations\n");
	p(esp_sa_delayed,     "\t%u delayed     ESP Security Associations\n");

	p(esp_transport_sent, "\t%u successful ESP transport-mode encryptions (packets sent)\n");
	p(esp_tunnel_sent, "\t%u successful ESP tunnel-mode encryptions sent (packets sent)\n");
	p(esp_accepted, "\t%u successful ESP decryptions performed (packets accepted)\n");
	p(esp_in_errs, "\t%u failed ESP decryption%s (packets dropped)\n");
	p(esp_trans_out_errs, "\t%u outgoing ESP transport-mode encryption%s failed (packets dropped)\n");
	p(esp_tunnel_out_errs, "\t%u outgoing ESP tunnel-mode encryption%s failed (packets dropped)\n");
	p(esp_trans_missing, "\t%u incoming packets needed and were missing transport-mode ESP\n");
	p(esp_tunnel_missing, "\t%u incoming packets needed and were missing tunnel-mode ESP\n");
	p(esp_tunnel_spoof, "\t%u received packet%s attempted ESP tunnel spoof attack\n");


#undef p
}






/*
 * Dump AH statistics structure.
 *
 * Derived from ipv6_stats() routine.
 * Includes statistics from ipsec_ah.c (AH),
 *                      and ipsec_policy.c (In/Out policy checking).
 */
void
AH_stat(off, name)
	u_long off;
	char *name;
{
        struct ah_stat_table AH_stats;

	if (off == 0)
		return;
        kread(off, (char *)&AH_stats, sizeof (AH_stats));
	printf("%s:\n", name);

#define	p(f, m) if (AH_stats.f || sflag <= 1) \
	printf(m, AH_stats.f, plural(AH_stats.f))

	p(ah_sa_unavailable, "\t%u unavailable  AH Security Associations\n");
	p(ah_sa_invalid,     "\t%u invalid      AH Security Associations\n");
	p(ah_sa_delayed,     "\t%u delayed      AH Security Associations\n");


	p(ah_sent, "\t%u successful outgoing authentication%s (packets sent)\n");
	p(ah_accepted, "\t%u successful incoming authentication%s\n (packets accepted)");
	p(ah_in_errs, "\t%u failed incoming authentication%s (packets dropped)\n");
	p(ah_out_errs, "\t%u outgoing authentication%s failed (packets dropped)\n");
	p(ah_missing, "\t%u packet%s needed and were missing AH\n");
	p(ah_spoof, "\t%u packet%s attempted an AH tunnel spoof attack\n");

#undef p
}

#endif /* INET6 */

static	char *icmpnames[] = {
	"echo reply",
	"#1",
	"#2",
	"destination unreachable",
	"source quench",
	"routing redirect",
	"#6",
	"#7",
	"echo",
	"#9",
	"#10",
	"time exceeded",
	"parameter problem",
	"time stamp",
	"time stamp reply",
	"information request",
	"information request reply",
	"address mask request",
	"address mask reply",
};

/*
 * Dump ICMP statistics.
 */
void
icmp_stats(off, name)
	u_long off;
	char *name;
{
	struct icmpstat icmpstat;
	register int i, first;

	if (off == 0)
		return;
	kread(off, (char *)&icmpstat, sizeof (icmpstat));
	printf("%s:\n", name);

#define	p(f, m) if (icmpstat.f || sflag <= 1) \
    printf(m, icmpstat.f, plural(icmpstat.f))
  
        p(icps_oldicmp,
	  "\t%u error%s not generated 'cuz old message was icmp\n");
	p(icps_error, "\t%u call%s to icmp_error\n");
	for (first = 1, i = 0; i < ICMP_MAXTYPE + 1; i++)
		if (icmpstat.icps_outhist[i] != 0) {
			if (first) {
				printf("\tOutput histogram:\n");
				first = 0;
			}
			printf("\t\t%s: %u\n", icmpnames[i],
				icmpstat.icps_outhist[i]);
		}
	p(icps_badcode, "\t%u message%s with bad code fields\n");
	p(icps_tooshort, "\t%u message%s < minimum length\n");
	p(icps_checksum, "\t%u bad checksum%s\n");
	p(icps_badlen, "\t%u message%s with bad length\n");
	for (first = 1, i = 0; i < ICMP_MAXTYPE + 1; i++)
		if (icmpstat.icps_inhist[i] != 0) {
			if (first) {
				printf("\tInput histogram:\n");
				first = 0;
			}
			printf("\t\t%s: %u\n", icmpnames[i],
				icmpstat.icps_inhist[i]);
		}
	p(icps_reflect, "\t%u message response%s generated\n");
#undef p
}


#ifdef INET6
/* Because the ICMPv6 message numbers are more sparse, a single
   static table is no longer sensible.  So the function below
   is a slower replacement, but netstat doesn't need to be that
   fast... */
char *icmpv6_name(int message_number)
{
static char outbuffer[40];

  switch (message_number) {

  /* Error and Diagnostic Message Types */
  case ICMPV6_UNREACH:
    strcpy(outbuffer, "destination unreachable\0");
    break;
    
  case ICMPV6_TOOBIG:
    strcpy(outbuffer, "packet too big\0");
    break;
    
  case ICMPV6_TIMXCEED:
    strcpy(outbuffer, "time exceeded\0");
    break;
    
  case ICMPV6_PARAMPROB:
    strcpy(outbuffer, "parameter problem\0");
    break;

  case ICMPV6_REDIRECT:
    strcpy(outbuffer, "redirect\0");
    break;

  /* Informational and Multicast Message Types */ 
  case ICMPV6_ECHO:
    strcpy(outbuffer, "echo request\0");
    break;
    
  case ICMPV6_ECHOREPLY:
    strcpy(outbuffer, "echo reply\0");
    break;
    
  case ICMPV6_GRPQUERY:
    strcpy(outbuffer, "group query\0");
    break;
    
  case ICMPV6_GRPREPORT:
    strcpy(outbuffer, "group report\0");
    break;
    
  case ICMPV6_GRPTERM:
    strcpy(outbuffer, "group leave\0");
    break;
    
  case ICMPV6_ROUTERSOL:
    strcpy(outbuffer, "router solicit\0");
    break;
    
  case ICMPV6_ROUTERADV:
    strcpy(outbuffer, "router advert\0");
    break;
    
  case ICMPV6_NEIGHBORSOL:
    strcpy(outbuffer, "node solicit\0");
    break;
    
  case ICMPV6_NEIGHBORADV:
    strcpy(outbuffer, "node advert\0");
    break;
    
  default:
    strcpy(outbuffer, "unrecognised\0");
    break;
  } /* end switch */

  return (outbuffer);
}; 


/*
 * Dump ICMP statistics for IPv6.
 */
void
icmpv6_stats(off, name)
	u_long off;
	char *name;
{
	struct icmpv6stat icmpv6stat;
	register int i, first;

	if (off == 0)
		return;
	kread(off, (char *)&icmpv6stat, sizeof (icmpv6stat));
	printf("%s:\n", name);

#define	p(f, m) if (icmpv6stat.f || sflag <= 1) \
    printf(m, icmpv6stat.f, plural(icmpv6stat.f))

	p(icps_error, "\t%u call%s to icmp_error\n");
	for (first = 1, i = 0; i < ICMPV6_MAXTYPE + 1; i++)
		if (icmpv6stat.icps_outhist[i] != 0) {
			if (first) {
				printf("\tOutput histogram:\n");
				first = 0;
			}
			printf("\t\t%s: %u\n", icmpv6_name(i),
				icmpv6stat.icps_outhist[i]);
		}
	p(icps_badcode, "\t%u message%s with bad code fields\n");
	p(icps_tooshort, "\t%u message%s < minimum length\n");
	p(icps_checksum, "\t%u bad checksum%s\n");
	p(icps_badlen, "\t%u message%s with bad length\n");
	for (first = 1, i = 0; i < ICMP_MAXTYPE + 1; i++)
		if (icmpv6stat.icps_inhist[i] != 0) {
			if (first) {
				printf("\tInput histogram:\n");
				first = 0;
			}
			printf("\t\t%s: %u\n", icmpv6_name(i),
				icmpv6stat.icps_inhist[i]);
		}
	p(icps_reflect, "\t%u message response%s generated\n");
#undef p
}

#endif /* INET6 */

/*
 * Dump IGMP statistics structure.
 */
void
igmp_stats(off, name)
	u_long off;
	char *name;
{
	struct igmpstat igmpstat;

	if (off == 0)
		return;
	kread(off, (char *)&igmpstat, sizeof (igmpstat));
	printf("%s:\n", name);

#define	p(f, m) if (igmpstat.f || sflag <= 1) \
    printf(m, igmpstat.f, plural(igmpstat.f))
#define	py(f, m) if (igmpstat.f || sflag <= 1) \
    printf(m, igmpstat.f, igmpstat.f != 1 ? "ies" : "y")
	p(igps_rcv_total, "\t%u message%s received\n");
        p(igps_rcv_tooshort, "\t%u message%s received with too few bytes\n");
        p(igps_rcv_badsum, "\t%u message%s received with bad checksum\n");
        py(igps_rcv_queries, "\t%u membership quer%s received\n");
        py(igps_rcv_badqueries, "\t%u membership quer%s received with invalid field(s)\n");
        p(igps_rcv_reports, "\t%u membership report%s received\n");
        p(igps_rcv_badreports, "\t%u membership report%s received with invalid field(s)\n");
        p(igps_rcv_ourreports, "\t%u membership report%s received for groups to which we belong\n");
        p(igps_snd_reports, "\t%u membership report%s sent\n");
#undef p
#undef py
}

/*
 * Pretty print an Internet address (net address + port).
 * If the nflag was specified, use numbers instead of names.
 */
void
#ifdef INET6
inetprint(af, in, port, proto)
        int af;
#else
inetprint(in, port, proto)
#endif /* INET6 */
	register struct in_addr *in;
	int port;
	char *proto;
{
	struct servent *sp = 0;
	char line[160], *cp;
	int width;

#ifdef INET6
	if ((af == AF_INET6) && nflag)
	  sprintf(line, " %s.", inet6name((struct in_addr6*)in));
	else 
	  sprintf(line, "%.*s.", (Aflag && !nflag) ? 12 : 16, 
	    (af == AF_INET6) ? inet6name((struct in_addr6 *)in) 
	     : inetname(in));
#else 
	sprintf(line, "%.*s.", (Aflag && !nflag) ? 12 : 16, inetname(in));
#endif /* INET6 */
	cp = index(line, '\0');
	if (!nflag && port)
		sp = getservbyport((int)port, proto);
	if (sp || port == 0)
		sprintf(cp, "%.8s", sp ? sp->s_name : "*");
	else
		sprintf(cp, "%d", ntohs((u_short)port));

	width = Aflag ? 18 : 22;
#ifdef INET6
	if ((af == AF_INET6) && nflag)
	  printf("%-*s ", width,line);
	else
#endif /* INET6 */	  
	printf(" %-*.*s", width, width, line);
}

#ifdef INET6
static char domain[MAXHOSTNAMELEN + 1];
static int first = 1;
#endif /* INET6 */

/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give
 * numeric value, otherwise try for symbolic name.
 */
char *
inetname(inp)
	struct in_addr *inp;
{
	register char *cp;
	static char line[50];
	struct hostent *hp;
	struct netent *np;
#ifndef INET6
	static char domain[MAXHOSTNAMELEN + 1];
	static int first = 1;
#endif /* INET6 */

	if (first && !nflag) {
		first = 0;
		if (gethostname(domain, MAXHOSTNAMELEN) == 0 &&
		    (cp = index(domain, '.')))
			(void) strcpy(domain, cp + 1);
		else
			domain[0] = 0;
	}
	cp = 0;
	if (!nflag && inp->s_addr != INADDR_ANY) {
		int net = inet_netof(*inp);
		int lna = inet_lnaof(*inp);

		if (lna == INADDR_ANY) {
			np = getnetbyaddr(net, AF_INET);
			if (np)
				cp = np->n_name;
		}
		if (cp == 0) {
			hp = gethostbyaddr((char *)inp, sizeof (*inp), AF_INET);
			if (hp) {
				if ((cp = index(hp->h_name, '.')) &&
				    !strcmp(cp + 1, domain))
					*cp = 0;
				cp = hp->h_name;
			}
		}
	}
	if (inp->s_addr == INADDR_ANY)
		strcpy(line, "*");
	else if (cp)
		strcpy(line, cp);
	else {
		inp->s_addr = ntohl(inp->s_addr);
#define C(x)	((x) & 0xff)
		sprintf(line, "%u.%u.%u.%u", C(inp->s_addr >> 24),
		    C(inp->s_addr >> 16), C(inp->s_addr >> 8), C(inp->s_addr));
	}
	return (line);
}


#ifdef INET6
char *
inet6name(in6p)
        struct in_addr6 *in6p;
{
	register char *cp;
	static char line[50];
	struct hostent *hp;

	if (first && !nflag) {
		first = 0;
		if (gethostname(domain, MAXHOSTNAMELEN) == 0 &&
		    (cp = index(domain, '.')))
			(void) strcpy(domain, cp + 1);
		else
			domain[0] = 0;
	}
	cp = 0;
	if (!nflag && !IS_IN6_UNSPEC(*in6p)) {
	         hp = (struct hostent *)addr2hostname(in6p, 
		      sizeof(struct in_addr6), AF_INET6);
		 if (hp) {
		         if ((cp = index(hp->h_name, '.')) &&
			     !strcmp(cp + 1, domain))
			         *cp = 0;
                         cp = hp->h_name;
		 }

	}
	if (IS_IN6_UNSPEC(*in6p))
	         strcpy(line, "*");
	else if (cp)
	         strcpy(line, cp);
	else 
	         addr2ascii(AF_INET6, in6p, sizeof(struct in_addr6), line);
	return(line);
}
#endif INET6
