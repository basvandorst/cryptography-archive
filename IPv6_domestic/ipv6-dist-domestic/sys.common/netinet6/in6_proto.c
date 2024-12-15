/*
 * in6_proto.c  --  Protocol switch entries for IPv6.
 *
 *
 * Copyright 1995 by Dan McDonald, Bao Phan, and Randall Atkinson,
 *	All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 */

/*----------------------------------------------------------------------
#       @(#)COPYRIGHT   1.1a (NRL) 17 August 1995

COPYRIGHT NOTICE

All of the documentation and software included in this software
distribution from the US Naval Research Laboratory (NRL) are
copyrighted by their respective developers.

This software and documentation were developed at NRL by various
people.  Those developers have each copyrighted the portions that they
developed at NRL and have assigned All Rights for those portions to
NRL.  Outside the USA, NRL also has copyright on the software
developed at NRL. The affected files all contain specific copyright
notices and those notices must be retained in any derived work.

NRL LICENSE

NRL grants permission for redistribution and use in source and binary
forms, with or without modification, of the software and documentation
created at NRL provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:

        This product includes software developed at the Information
        Technology Division, US Naval Research Laboratory.

4. Neither the name of the NRL nor the names of its contributors
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

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/mbuf.h>

#include <net/if.h>
#include <net/route.h>
#include <net/radix.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/in_pcb.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_debug.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h>
#include <netinet6/ipsec.h>
#include <netinet6/ipsec_ah.h>
#include <netinet6/ipsec_esp.h>

extern struct domain inet6domain;

struct protosw inet6sw[] = {
  
  {
    0, &inet6domain, 0, 0,    /* NOTE:  This 0 is the same as NEXTHDR_HOP,
				 but we specially demux NEXTHDR_HOP
				 in ipv6_input(). */
    ipv6_hop, ipv6_output, 0, 0, /* Watch for hop-by-hop input! */
    0,
    ipv6_init, 0, ipv6_slowtimo, ipv6_drain, ipv6_sysctl
  },

  /* ICMPv6 entry */

  {
    SOCK_RAW, &inet6domain, NEXTHDR_ICMP, PR_ATOMIC|PR_ADDR,
    ipv6_icmp_input, /* ripv6_output*/ ipv6_icmp_output, 0, ripv6_ctloutput,
    /*ripv6_usrreq*/ ipv6_icmp_usrreq,
    0, 0, 0, 0, 0
  },

  /* IPv6-in-IPv6 entry */

  {
    SOCK_RAW, &inet6domain, NEXTHDR_IPV6, PR_ATOMIC|PR_ADDR,
    ipv6_input, ripv6_output, 0, ripv6_ctloutput,
    ripv6_usrreq,
    0, 0, 0, 0, 0
  },

  /* Fragment entry 

     This should never actually be called.  See ipv6_input() for details.

  {
    SOCK_RAW, &inet6domain, NEXTHDR_FRAGMENT, PR_ATOMIC|PR_ADDR,
    ipv6_reasm, 0, 0, 0,
    0,
    0, 0, 0, 0, 0
  },
  */

  /* UDP entry */

  /*
   * Eventually, that ipv6_ctloutput() will have to be replaced with a
   * udp_ctloutput(), which knows whether or not to redirect things down to
   * IP or IPv6 appropriately.
   */

  {
    SOCK_DGRAM, &inet6domain, NEXTHDR_UDP, PR_ATOMIC|PR_ADDR,
    udp_input, 0, udp_ctlinput, ipv6_ctloutput,
    udp_usrreq,
    0, 0, 0, 0, 0
  },

  /* TCP entry */

  {
    SOCK_STREAM, &inet6domain, NEXTHDR_TCP, PR_CONNREQUIRED|PR_WANTRCVD,
    tcp_input, 0, tcp_ctlinput, tcp_ctloutput,
    tcp_usrreq,
    0, 0, 0, 0, 0  /* init, fasttimo, etc. in v4 protosw already! */
  },

  /* IPv6 & IPv4 Authentication Header */
  {
    SOCK_RAW, &inet6domain, NEXTHDR_AUTH, PR_ATOMIC|PR_ADDR,
    ipsec_ah_input, 0, 0, 0,
    0,
    0, 0, 0, 0, 0
  },

  /* IPv6 & IPv4 Encapsulating Security Payload Header */
  {
    SOCK_RAW, &inet6domain, NEXTHDR_ESP, PR_ATOMIC|PR_ADDR,
    ipsec_esp_input, 0, 0, 0,
    0,
    0, 0, 0, 0, 0
  },

  /* Unknown header. */

  {
    SOCK_RAW, &inet6domain, NEXTHDR_RAW, PR_ATOMIC|PR_ADDR,
    ripv6_input, ripv6_output, 0, ripv6_ctloutput,
    ripv6_usrreq,
    0,0,0,0,0
  },

  /* Raw wildcard */
  {
    SOCK_RAW, &inet6domain, 0, PR_ATOMIC|PR_ADDR,
    ripv6_input, ripv6_output, 0, ripv6_ctloutput,
    ripv6_usrreq,
    ripv6_init,0,0,0,0
  },
};

struct domain inet6domain =
{
  PF_INET6, "IPv6", 0, 0, 0,
  inet6sw, &inet6sw[sizeof(inet6sw)/sizeof(inet6sw[0])], 0,
  /*
   * FreeBSD's IPv4 replaces rn_inithead() with an IPv4-specific function.
   * Our IPv6 uses the ifa->ifa_rtrequest() function pointer to intercept
   * rtrequest()s.  The consequence of this is that we use the generic
   * rn_inithead().
   */
  rn_inithead, 64, sizeof(struct sockaddr_in6)
};
