/*----------------------------------------------------------------------
  ipsec_policy.c:    IP Security Policy engine routines for both input
                     and output processing.  

		     Note that actual AH/ESP processing is NOT done here.  

Copyright 1995 by Randall Atkinson, Bao Phan, & Dan McDonald, All
Rights Reserved.  All Rights are assigned to the US Naval Research
Laboratory (NRL).  The NRL Copyright Notice & License Agreement governs
Distribution and Use of this software.
----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
#	@(#)COPYRIGHT	1.1a (NRL) 17 August 1995

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
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/route.h>
#include <sys/protosw.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>

#include <netinet6/in6.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>

#include <netinet6/in6_debug.h>

#include <netinet6/ipsec.h>
#include <netinet6/key.h>
#include <netinet6/ipsec_policy.h>


#ifndef min
#define min(x, y) (x < y) ? x : y
#endif /* min(x, y) */
#ifndef max
#define max(x, y) (x > y) ? x : y
#endif /* max(x, y) */


/* nullkeynode is a special NULL "Key Table Node" of the Key Engine
   that lives in file netinet6/key.c;
   IF getassocbysocket() returns 0 && the ktn ptr == &(nullkeynode),
   THEN this socket is authorised to bypass the spec'd security service. */
extern struct key_tblnode nullkeynode; 

#define INIT if (!ipsec_initted) ipsec_init()


/*----------------------------------------------------------------------
 socket_input_policy_check:  Given the security level for the local socket, 
                  return whether the flagged security level of packet is OK.

----------------------------------------------------------------------*/
static int socket_input_policy_check(socketlevel, type, flagged)
     int socketlevel[4];
     int type;
     int flagged;
{
  int themin = min(socketlevel[type], ipsec_system_sec_level[type]);
  int themax = max(socketlevel[type], ipsec_system_sec_level[type]);

  /* Formerly, a negative security level was used to mark input bypass allowed.
     A better approach is needed. rja */
  if (themin < -1)
    return flagged;

  if (themin == -1)
    return 0;

  if (themax >= IPSEC_LEVEL_REQUIRE) 
    return (!flagged);

  return 0;
}


/*----------------------------------------------------------------------
  ipsec_input_policy:
             This routine checks the security properties of an incoming
	     packet against the security requirements of the receiving
	     socket and the system security policy to determine whether
	     this packet can be accepted or should instead be dropped.

  NOTES:     This is just a place-holder quick hack that needs to
             be thoroughly rewritten or replaced after the "alpha"
	     release.  Time doesn't permit rewriting just now.  
	     Suggestions for improvement are very welcome.  rja
----------------------------------------------------------------------*/
int ipsec_input_policy(incoming, socket, next_header)
     struct mbuf *incoming;
     struct socket *socket;
     int next_header;
{
  int socketlevel[4];
  int result = 0;

  if (socket)
    bcopy(socket->so_securitylevel, socketlevel, sizeof(socketlevel));
  else
    bzero(socketlevel, sizeof(socketlevel));

  if (socket_input_policy_check(socketlevel, SS_AUTHENTICATION,
				incoming->m_flags & M_AUTHENTIC)) {
    ah_stats.ah_missing++;
    result = -1;
  }

  /* A problem with the next 2 tests is that one can't detect the case
     where the received packet was encrypted with ESP_TRANSPORT 
     when ESP_TUNNEL was really what was needed.  Finer granularity
     of input policy checks are needed.  rja */
  if (socket_input_policy_check(socketlevel, SS_ENCRYPTION_TRANSPORT,
				incoming->m_flags & M_DECRYPTED)) {
    esp_stats.esp_trans_missing++;
    result = -1;
  }

  if (socket_input_policy_check(socketlevel, SS_ENCRYPTION_NETWORK,
				incoming->m_flags & M_DECRYPTED)) {
    esp_stats.esp_tunnel_missing++;
    result = -1;
  }

  DPRINTF(IDL_EVENT, ("ipsec_input_policy: protocol=%d, result=%s\n",
		      next_header, result ? "drop" : "OK"));

  if (result != 0)
    DPRINTF(IDL_CRITICAL, 
	    ("IPsec_input_policy: rejected packet, proto = %d\n", 
	     next_header));

  return (result);
}


/*----------------------------------------------------------------------
  ipsec_output_policy():  Called by ip*_output() to determine which 
                          IPsec services are needed for an outbound 
			  socket, which is passed in.
			  Source and Destination sockaddr ptrs are
			  passed in for the Key Engine to use.

		  Returns:
		     "needed" services in a flags short
		     "sa_ready" status indicating whether 
		         all needed Sec Assocs are ready,
			 OR the packet should be queued until SAs are ready,
			 OR whether the SAs can't be obtained at all.
		     "ah_sa" points to Key Table Node with AH's SA
		     "esp_t" points to Key Table Node with ESP Xport's SA
		     "esp_n" points to Key Table Node with ESP Tunnel's SA
		Any Sec Assoc that is not needed will be returned as NULL.

 Notes:
        This routine is WAY too slow considering that it is called
	at least once for each outgoing packet.

	If the socket ptr passed in is NULL, then this policy function
	will assign the system security level BUT will ensure that
	unique per-socket keying is NOT requested.  One can only have 
	unique per-socket keying from an actual application layer socket.  
	When the socket ptr is NULL, then the packet originated inside
	the kernel (e.g. ICMP, ND, TCP response to port not being
	listed to).

	This function is responsible for actually calling getassocbysocket()
	and passing the KTNs back to the caller of ipsec_output_policy()
	so that the Key Engine knows that those SAs are in use and so
	won't accidentally delete the SA.  This reduces the potential 
	race condition of the Policy Engine saying that the SA exists 
	and is ready and then the Key Engine deleting that SA _before_ 
	ip*_output() can obtain all the needed SAs.  This implies that
	all callers of ipsec_output_policy() MUST call key_free() for
	any non-NULL KTN pointers that they are given.  We are passing
	around KTN pointers rather than (struct ipsec_assoc *) pointers
	because key_free() takes a KTN pointer as its input parameter.

	In future this function will be enhanced into a more sophisticated
	and useful determinant of output security policy, but this is
	sufficient for the alpha release.  Suggestions on policies that
	would be operationally useful are solicited.  rja
----------------------------------------------------------------------*/
void ipsec_output_policy(socket, src, dst, needed, SA_ready, 
			 ah_ktn, esp_t_ktn, esp_n_ktn)
     struct socket        *socket;
     struct sockaddr      *src;
     struct sockaddr      *dst;
     u_int                *needed;
     u_int                *SA_ready;
     struct key_tblnode  **ah_ktn; 
     struct key_tblnode  **esp_t_ktn; 
     struct key_tblnode  **esp_n_ktn; 
{
  struct inpcb       *inp;
  u_short             sport;
  u_short             dport;
  short            esp_n_status;
  short            esp_t_status;
  short            ah_status;

  DPRINTF(IDL_GROSS_EVENT, ("Entering ipsec_output_policy()\n"));

  *needed = 0;
  *SA_ready = 0;

  if (socket == NULL) {
    esp_t_status = min(ipsec_system_sec_level[SS_ENCRYPTION_TRANSPORT],
		 IPSEC_LEVEL_REQUIRE);
  } else { 
    /* socket ptr is real, so just apply system security level */
    esp_t_status = max(socket->so_securitylevel[SS_ENCRYPTION_TRANSPORT],
		 ipsec_system_sec_level[SS_ENCRYPTION_TRANSPORT]);
  }
  if (esp_t_status > IPSEC_LEVEL_DEFAULT) {
    DPRINTF(IDL_GROSS_EVENT, ("ESP Transport encryption is needed.\n"));
    esp_t_status = getassocbysocket(SS_ENCRYPTION_TRANSPORT, src, dst, 
		      socket, ((esp_t_status == IPSEC_LEVEL_UNIQUE) ? 1 : 0), 
		      esp_t_ktn);

    if (esp_t_status == 1) 
      esp_stats.esp_sa_delayed++;
    else if (esp_t_status == -1)
      esp_stats.esp_sa_unavailable++;

    /* now, if not an authorised security bypass, then set needed flag */
    if (*esp_t_ktn != &(nullkeynode))  
      *needed |= IPSEC_ESP_TRANSPORT_NEEDED;
  } else {
    *esp_t_ktn = NULL;
  }

  if (socket == NULL) {
    esp_n_status = min(ipsec_system_sec_level[SS_ENCRYPTION_NETWORK],
		 IPSEC_LEVEL_REQUIRE);
  } else { 
    /* socket ptr is real, so just apply system security level */
    esp_n_status = max(socket->so_securitylevel[SS_ENCRYPTION_NETWORK],
		 ipsec_system_sec_level[SS_ENCRYPTION_NETWORK]);
  }
  if (esp_n_status > IPSEC_LEVEL_DEFAULT) {
    DPRINTF(IDL_GROSS_EVENT, ("ESP Tunnel encryption is needed.\n"));
    esp_n_status = getassocbysocket(SS_ENCRYPTION_NETWORK, src, dst,
		      socket, ((esp_n_status == IPSEC_LEVEL_UNIQUE) ? 1 : 0), 
		      esp_n_ktn);

    if (esp_n_status == 1) 
      esp_stats.esp_sa_delayed++;
    else if (esp_n_status == -1)
      esp_stats.esp_sa_unavailable++;

    /* now, if not an authorised security bypass, set needed flag */
    if (*esp_n_ktn != &(nullkeynode))
      *needed |= IPSEC_ESP_TUNNEL_NEEDED;
  } else {
    *esp_n_ktn = NULL;
  }

  if (socket == NULL) {
    ah_status = min(ipsec_system_sec_level[SS_AUTHENTICATION],
		 IPSEC_LEVEL_REQUIRE);
  } else { 
    /* socket ptr is real, so just apply system security level */
    ah_status = max(socket->so_securitylevel[SS_AUTHENTICATION],
		 ipsec_system_sec_level[SS_AUTHENTICATION]);
  }
  if (ah_status > IPSEC_LEVEL_DEFAULT) {
    DPRINTF(IDL_GROSS_EVENT, ("AH is needed.\n"));
    ah_status = getassocbysocket(SS_AUTHENTICATION, src, dst,
		      socket, (ah_status == IPSEC_LEVEL_UNIQUE) ? 1 : 0,
		       ah_ktn);
    if (ah_status == 1)
      ah_stats.ah_sa_delayed++;
    else if (ah_status == -1)
      ah_stats.ah_sa_unavailable++;

    /* now, if not an authorised security bypass, set needed flag */
    if (*ah_ktn != &(nullkeynode))
	*needed |= IPSEC_AH_NEEDED;
  } else {
    *ah_ktn = NULL;
  }

  /* set return value for ipsec_output_policy()
   * If anything is a lost cause, this whole packet is a goner.
   * elseif anything is delayed, this packet needs to be delayed
   * else life is good.
   */ 
  if ((ah_status == -1) || (esp_n_status == -1) || (esp_t_status == -1)) {
    *SA_ready = -1;
  }
  else if ((ah_status == 1) || (esp_n_status == 1) || (esp_t_status == 1)) {
    *SA_ready = 1;
  }
  else
    *SA_ready = 0;
    
    /* if one or more SAs aren't yet ready but are needed,
       then ensure that all ktn pointers are freed up because
       ip*_output() will just take the packet and put it on
       the ipsec waiting-for-SA queue. */
  if (*SA_ready != 0) {
    if (*ah_ktn != NULL) {
      DPRINTF(IDL_GROSS_EVENT, ("ipsec_output_policy freeing AH ktn.\n"));
      key_free(*ah_ktn);
      *ah_ktn = NULL;
    }

    if (*esp_t_ktn != NULL) {
      DPRINTF(IDL_GROSS_EVENT, ("ipsec_output_policy freeing ESP_T ktn.\n"));
      key_free(*esp_t_ktn);
      *esp_t_ktn = NULL;
    }

    if (*esp_n_ktn != NULL) {
      DPRINTF(IDL_GROSS_EVENT, ("ipsec_output_policy freeing ESP_N ktn.\n"));
      key_free(*esp_n_ktn);
      *esp_n_ktn = NULL;
    }
  } /* endif SA_ready != 0 */

  DPRINTF(IDL_GROSS_EVENT, ("Leaving ipsec_output_policy()\n"));

}
/* EOF */
