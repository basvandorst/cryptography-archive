/*----------------------------------------------------------------------
  ipsec_policy.h:  Declarations for the Policy Engine.

  This software is Copyright 1995 by Randall Atkinson, Bao Phan, &
  Dan McDonald, All Rights Reserved.  All Rights have been assigned
  to the US Naval Research Laboratory.  The NRL Copyright Notice and
  License Agreement below applies to this software.
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

#ifndef _IPSEC_POLICY_H

#define _IPSEC_POLICY_H

#ifdef KERNEL
/* Symbolic names for SA_ready flag of ipsec_output_policy() */
#define IPSEC_SA_FAIL                  -1
#define IPSEC_SA_READY                  0
#define IPSEC_SA_PENDING                1

/* Symbolic names for the needed flag of ipsec_output_policy();
   These are deliberately chosen to be a bit field format. */
#define IPSEC_AH_NEEDED                 1
#define IPSEC_ESP_TUNNEL_NEEDED         2
#define IPSEC_ESP_TRANSPORT_NEEDED      4

/* service routines that live in ipsec_policy.c */
int      ipsec_input_policy __P((struct mbuf *, struct socket *, int));
void     ipsec_output_policy __P((struct socket *, struct sockaddr *, 
			  struct sockaddr *, u_int *, u_int *,
			  struct key_tblnode **, struct key_tblnode **, 
			  struct key_tblnode **));
#endif /* KERNEL */

#endif /* _IPSEC_POLICY_H */
