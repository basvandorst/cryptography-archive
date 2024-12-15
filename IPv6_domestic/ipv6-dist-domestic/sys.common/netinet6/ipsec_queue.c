/*
 * ipsec_queue.c  --  Routines for queueing up packets which
 *                    don't have keys yet.  Will hold packets while user-
 *                    level key management programs fetch keys.
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
#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/time.h>
#include <sys/kernel.h>

#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

#include <netinet6/in6.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>

#include <netinet6/key.h>
#include <netinet6/ipsec.h>
#include <netinet6/ipsec_policy.h>
#include <netinet6/ipsec_queue.h>

/*
 * Globals
 */

struct ipsecq ipsecq;
/* Following variables should be sysctl-tweakable. */
int ipsec_qtint = IPSEC_QTINT;
int ipsec_qlifetime = IPSEC_QLIFETIME;

/*
 * Functions
 */

/*----------------------------------------------------------------------
 * ipsec_qtimer:   See if any queued up packets can be dequeued yet.
 *
 ----------------------------------------------------------------------*/

void
ipsec_qtimer(whocares)
     void *whocares;
{
  struct ipsecq *ipsq;
  int s = splnet();    
  struct key_tblnode *ah_ktn, *esp_n_ktn, *esp_t_ktn;
  u_int needed, SA_ready;
  struct ip *ip;
  struct ipv6 *ipv6;

  ipsq = ipsecq.ipsq_next;
  while (ipsq != &ipsecq)
    {
      struct ipsecq *current = ipsq;
      struct sockaddr_in6 src6, dst6;  /* Make sockaddr_in6 actual storage,
					  because it's the biggest one. */
      struct sockaddr_in *src4 = (struct sockaddr_in *)&src6,
                         *dst4 = (struct sockaddr_in *)&dst6;
      struct sockaddr *src = (struct sockaddr *)&src6,
                      *dst = (struct sockaddr *)&dst6;

      ipsq = ipsq->ipsq_next;   /* Before any munging of current item. */

      /*
       * Assume IP/IPv6 header is contiguous in the first mbuf of chain.
       */
      ip = mtod(current->ipsq_packet, struct ip *);
      ipv6 = mtod(current->ipsq_packet, struct ipv6 *);

      /*
       * Ports are position-invariant across sockaddr_in/in6, therefore blank
       * them out regardless.
       */
      src4->sin_port = 0;
      dst4->sin_port = 0;

      if (ip->ip_v == 4)
	{
	  src4->sin_family = AF_INET;
	  dst4->sin_family = AF_INET;
	  src4->sin_len = sizeof(*src4);
	  dst4->sin_len = sizeof(*dst4);
	  src4->sin_addr = ip->ip_src;
	  dst4->sin_addr = ip->ip_dst;
	}
      else
	{
	  src6.sin6_family = AF_INET6;
	  dst6.sin6_family = AF_INET6;
	  src6.sin6_len = sizeof(src6);
	  dst6.sin6_len = sizeof(dst6);
	  src6.sin6_addr = ipv6->ipv6_src;
	  dst6.sin6_addr = ipv6->ipv6_dst;
	}

      /*
       * Check to see if all needed keys are ready 
       * by consulting ipsec_output_policy().  Only the output policy
       * function really knows which keys are needed.
       */
      ipsec_output_policy((struct socket *)
			  current->ipsq_packet->m_pkthdr.rcvif, src, dst,
			  &needed, &SA_ready, &ah_ktn, &esp_t_ktn, &esp_n_ktn);

      if ((needed == 0) || (SA_ready == IPSEC_SA_READY))
	{
	  /*
	   * If I make it in here, dequeue current, and send packet out on
	   * its merry way by calling either ip_output() or ipv6_output().
	   * (Don't forget to free ktn pointers!)
	   *
	   * (I'd prefer not to store the v4/v6 decision in struct ipsecq.
	   *  Inspecting the first byte (actually 4 bits) of the packet
	   *  should be enough, at least for v4/v6 decisions.  If we
	   *  enlarge this to be beyond AF_INET{,6}, we'll have to
	   *  put more state in ipsecq.)
	   */

	  /*
	   * If needed is 0, none of these checks will pass, therefore,
	   * no key_free() calls are made.
	   */
	  if (SA_ready == 0)
	    {
	      if (needed & IPSEC_ESP_TRANSPORT_NEEDED)
		if (esp_t_ktn != NULL)
		  key_free(esp_t_ktn);
	      if (needed & IPSEC_AH_NEEDED)
		if (ah_ktn)
		  key_free(ah_ktn);
	      if (needed & IPSEC_ESP_TUNNEL_NEEDED)
		if (esp_n_ktn)
		  key_free(esp_n_ktn);
	    }

	  remque(current);

	  /*
	   * NOTE:  There is a small chance that when I call ip/ipv6_output,
	   *        that the packet will get put right back on the queue
	   *        I'm traversing.  The good news is that it'll be put on
	   *        the queue's front, so I won't inspect it until
	   *        the next time I'm around.
	   */
	  if (ip->ip_v == 4)
	    ip_output(current->ipsq_packet, NULL, NULL, IP_RAWOUTPUT, NULL);
	  else ipv6_output(current->ipsq_packet, NULL, IPV6_RAWOUTPUT, NULL);
	  
	  free(current, M_TEMP);
	}
      else if (SA_ready == -1 ||
	       current->ipsq_expire <= time.tv_sec)
	{
	  /*
	   * Age entry and remove if expired or something went seriously
	   * wrong with the policy engine.
	   *
	   * ipsq_expire is the (absolute) time which the entry expires.
	   * If current time (time.tv_sec) has past expiration time, nuke it.
	   */
	  
	  m_freem(current->ipsq_packet);
	  remque(current);
	  free(current,M_TEMP);
	  
	} /* endif SA_ready or needed */
    } /* end while */

  timeout(ipsec_qtimer,NULL,ipsec_qtint * hz);
  splx(s);
}

/*----------------------------------------------------------------------
 * ipsec_qinit():    Initialises IPsec queue structures prior to use.
 *
 * Called from ipsec_init() in netinet6/ipsec.c.
 ----------------------------------------------------------------------*/
void
ipsec_qinit()
{
  ipsecq.ipsq_prev = ipsecq.ipsq_next = &ipsecq;
  timeout(ipsec_qtimer,NULL,hz);
}


/*----------------------------------------------------------------------
 * ipsec_enqueue:     Enqueue a packet because SAs/keys aren't ready yet.  
 *                    If a failure occurs in this function, dump the mbufs.
 *
 * Called by {ip,ipv6}_output(), if keys are unavailable.
 ----------------------------------------------------------------------*/

void
ipsec_enqueue(packet)
     struct mbuf *packet;
{
  struct ipsecq *newbie = malloc(sizeof(struct ipsecq), M_TEMP, M_NOWAIT);
  int s;

  if (newbie == NULL) {
    /* if can't get memory, free packet and give up */
    m_freem(packet);
    return ;
  }

  newbie->ipsq_packet = packet;
  newbie->ipsq_expire = time.tv_sec + ipsec_qlifetime;
  s = splnet();   /* See comment by splnet() in ipsec_qtimer(). */
  insque(newbie,&ipsecq);
  splx(s);
  return ;
}

/*----------------------------------------------------------------------
 * ipsec_remsock:    Scan the queue of waiting packets. 
 *                   Remove all entries spawned by the passed-in socket.
 *
 * Called by sofree() or someplace close to that when a socket is
 * about to go away.
 ----------------------------------------------------------------------*/

void
ipsec_remsock(so)
     struct socket *so;
{
  struct ipsecq *ipsq;
  int s = splnet();

  ipsq = ipsecq.ipsq_next;
  while (ipsq != &ipsecq)
    {
      struct ipsecq *current = ipsq;

      ipsq = ipsq->ipsq_next;   /* Before any munging of current item. */
      if (so == (struct socket *)current->ipsq_packet->m_pkthdr.rcvif)
	{
	  m_freem(current->ipsq_packet);
	  remque(current);
	  free(current,M_TEMP);
	}
    }

  splx(s);
}

/* EOF */
