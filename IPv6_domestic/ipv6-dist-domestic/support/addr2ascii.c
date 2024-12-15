/*
 * addr2ascii.c  --  Implements translation of addresses to human-readable
 *                   form.
 *
 * Copyright 1995 by Randall Atkinson, Bao Phan, and Dan McDonald
 *	All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 */

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

#include <netinet/in.h>
#include <netinet6/in6.h>
#include <netns/ns.h>
#include <netiso/iso.h>
#include <net/if_dl.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

#include "support.h"

static char addr2ascii_buffer[ADDR2ASCII_BUFSZ_MAX+1];

/*
 * Convert network-format IN6/IPv6 address
 * to canonical hexadecimal user format of h:h:h:h
 ... perfect for unrolling ...
 */
static char *_inet6_ntoa(addr)
     struct in_addr6 addr;
{
  static char b[40];
  char *a = (char *)&addr;
  register char i, k;
  int j;
  char table[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

#define PARTS                   8   /* # of parts in printable IPv6 address */
#define BINARY_DIGITS_PER_PART  2   /* # of "*a" chars in a single part */

  for (i = j = 0; i < PARTS; i++) {
    for (k = 0; k < BINARY_DIGITS_PER_PART; k++) {
      b[j++] = table[((*a >> 4) & 0x0f)];
      b[j++] = table[(*a & 0x0f)];
      a++;
    } /* end for k */

    /* now compress out leading zeros; 
       there must be a better way to code this... */
    if (b[j - 4] == '0') {
      if (b[j - 3] == '0') {
	if (b[j - 2] == '0') {
	  if (b[j - 1] == '0') { 
	    /* all 4 are zero, so just compress out completely */
	    j -= 4;
	  } else {
	    /* first 3 are zero and last is not, so remove 3 leading zeros */ 
	    b[j - 4] = b[j - 1];
	    b[j - 3] = b[j - 2] = b[j - 1] = '\0'; 
	    j -= 3;
	  } /* endif (b[j - 1] = '0') */
	} else {
	  /* first 2 are zero and last 2 aren't, so remove 2 leading zeros */
	  b[j - 4] = b[j - 2];
	  b[j - 3] = b[j - 1];
	  b[j - 2] = b[j - 1] = '\0'; 
	  j -= 2;
	} /* endif (b[j - 2] = '0') */
      } else {
	/* first 1 is zero, so remove one leading zero */
	  b[j - 4] = b[j - 3];
	  b[j - 3] = b[j - 2];
	  b[j - 2] = b[j - 1];
	  b[j - 1] = '\0'; 
	  j--;
      } /* endif (b[j - 3] = '0') */
    } /* endif (b[j - 4] = '0') */

    /* If not at end of the address,  append colon separator  */
    if (i < (PARTS - 1)) 
	b[j++] = ':';

    /* now compress out redundant colons */
    if ((b[j - 3] == ':') && (b[j - 2] == ':') && (b[j - 1] == ':')) 
      j--;


    /* since buffer b[] is reused and we now compress the IPv6 address
       where practical, we need to ensure we have a NUL at the
       end of the part of the buffer that we are currently using. */
    if (j < 39)
      b[j] = '\0';
    else
      b[39] = '\0';

  } /* end for i,j */

  return (b);
}

/* Standard API entry point. In the interest of moving other AFs to use this
   routine, try to include support for as much as possible by calling the
   existing conversion functions. */
char *addr2ascii(af, ap, len, cp)
int af;
/* const */ void *ap;
int len;
char *cp;
{
  char *ptr;
  int maxlen;

  ptr = NULL;

  switch(af) {

  case AF_INET:
    if (len != sizeof(struct in_addr))
      break;
    ptr = inet_ntoa(*((struct in_addr *)ap));
    maxlen = 16;
    break;

  case AF_INET6:
    if (len != sizeof(struct in_addr6))
      break;
    ptr = _inet6_ntoa(*((struct in_addr6 *)ap));
    maxlen = 46;
    break;

#ifdef AF_ISO
  case AF_ISO:
    ptr = iso_ntoa((struct iso_addr *)ap);
    maxlen = 42; /*?*/
    break;
#endif /* AF_ISO */

#ifdef AF_NS
  case AF_NS:
    ptr = ns_ntoa(*((struct ns_addr *)ap));
    maxlen = 42; /*?*/
    break;
#endif /* AF_NS */

#ifdef AF_LINK
  case AF_LINK:
    ptr = link_ntoa((struct sockaddr_dl *)ap);
    maxlen = 42; /*?*/
    break;
#endif /* AF_LINK */

  }

  if (!cp)
    cp = addr2ascii_buffer;

  if (ptr) {
    strncpy(cp, ptr, maxlen);
    cp[maxlen] = 0;
    return cp;
  }

  return NULL;
}
