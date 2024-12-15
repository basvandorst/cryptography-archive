/*----------------------------------------------------------------------
  ipsec.c:    Initialisation for IPsec and possible future home of
              any future routines shared between AH and ESP.

  Copyright 1995 by Randall Atkinson, Bao Phan, & Dan McDonald, All
  Rights Reserved.  All Rights have been assigned to the US Naval Research
  Laboratory (NRL).  The NRL Copyright Notice and License governs
  distribution and use of this software.
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

#include <netinet6/ipsec.h>
#include <netinet6/ipsec_queue.h>

#include <netinet6/in6_debug.h>

#ifndef min
#define min(x, y) (x < y) ? x : y
#endif /* min(x, y) */
#ifndef max
#define max(x, y) (x > y) ? x : y
#endif /* max(x, y) */

u_int  ipsec_system_sec_level[4];

struct esp_stat_table esp_stats; 
struct  ah_stat_table  ah_stats; 

int ipsec_initted = 0;


/*----------------------------------------------------------------------
  ipsec_init:   This is the master initialisation function for all
                of the IP security services.

		It isn't clear to me whether PF_KEY initialisation
		should be done here or not, but the use of PF_KEY for
		things unrelated to IP security (e.g. OSPF, RIP) argues
		that PF_KEY probably ought to be initialised separately. 
		rja
----------------------------------------------------------------------*/
void ipsec_init()
{
  if (ipsec_initted == 0) {

    /* Initialise IPsec statistics counters to zero */
    bzero(&esp_stats, sizeof(esp_stats));
    bzero(&ah_stats, sizeof(ah_stats));


    /* Initialise IPsec queue that lives in ipsec_queue.c and 
       is used for outbound packets waiting for their SA(s) to be ready */
    ipsec_qinit();
 
   
    /* The stuff below here stays, but needs to be reworked
       into a sysctl or some other more easily reconfigured scheme. rja */
    bzero(&ipsec_system_sec_level, sizeof(ipsec_system_sec_level));
    
    ipsec_system_sec_level[SS_AUTHENTICATION]       = IPSEC_LEVEL_DEFAULT;
    ipsec_system_sec_level[SS_ENCRYPTION_TRANSPORT] = IPSEC_LEVEL_DEFAULT;
    ipsec_system_sec_level[SS_ENCRYPTION_NETWORK]   = IPSEC_LEVEL_DEFAULT;

    
    ipsec_initted++;
  }  /* if not yet initialised... */
}

/* EOF */
