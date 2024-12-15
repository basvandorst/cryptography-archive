/*
 * ascii2addr.c  --  Implements translation of typed-in :: notation to
 *                   IPv6.  Also works for IP, and others.
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
#include <sys/param.h>
#include <sys/socket.h>
#include <machine/endian.h>

#include <netinet/in.h>
#include <netinet6/in6.h>
#include <netns/ns.h>
#include <netiso/iso.h>
#include <net/if_dl.h>

#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

/*
   Convert:

   xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
   xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:ddd.ddd.ddd.ddd
   xxxx::xxxx:xxxx
   xxxx::ddd.ddd.ddd.ddd
   ::xxxx:xxxx
   ::ddd.ddd.ddd.ddd 
*/

#define UNKNOWN 0
#define COLON 1
#define DOT 2

/* Parse a IPv6 address in string form (probably human-entered) into a binary,
   network-order address. Be fairly strict about syntax. Note: handles the
   double colon case by building a ``left'' and ``right'' side address, then
   effectively shifting over the right side to the far right of its buffer
   and merging that with the left side. */
static int _inet6_aton(cp, ap)
char *cp;
char *ap;
{
  char left[sizeof(struct in_addr6)], right[sizeof(struct in_addr6)];
  char *leftpos = left, *rightpos = right;
  char **pos = &leftpos;

  int state = UNKNOWN;
  char temp[5];
  int tempi = 0;
  int bytesdone = 0;
  int dotsdone = 0;

  bzero(left, sizeof(struct in_addr6));
  bzero(right, sizeof(struct in_addr6));

  while (*cp) {
    switch (*cp) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      if (state == DOT)
	return 0;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      /* Buffer digits */
      temp[tempi++] = *(cp++);
      break;
    case ':': /* Handle (hex) digits before a colon */
      if (tempi) {
	if (state == DOT)
	  return 0;
	tempi = temp[tempi] = 0;
	{
	  int buf;
	  /* Performance/finishing: get rid of sscanf */
	  sscanf(temp, "%x", &buf);
	  *((short *)(*pos)) = htons((short)buf);
	}
	*pos += sizeof(short);
	bytesdone += 2;
      } else /* Handle the icky double-colon case */
	if (state == COLON) {
	  if (rightpos == right)
	    pos = &rightpos;
	  else
	    return 0;
	}
		
      state = COLON;
      cp++;
      break;
    case '.': /* Handle (decimal) digits before a dot. Does not handle
	         case where there are hex digits in there properly, so
		 an address like f00:dead:beef:fe.1.1.9 right */
      if (!tempi)
	return 0;

      tempi = temp[tempi] = 0;
      {
	int buf;
	/* Performance/finishing: get rid of sscanf */
	sscanf(temp, "%d", &buf);
	*((char *)(*pos)) = (char)buf;
      }
      ++*pos;
      bytesdone++;
      dotsdone++;
      state = DOT;
      cp++;
      break;
    default:
      return 0;
    }
  }
  /* Finish up the last block of digits */
  if (tempi) 
    switch(state) {
    case COLON:
      tempi = temp[tempi] = 0;
      {
	int buf;
	sscanf(temp, "%x", &buf);
	*((short *)(*pos)) = htons((short)buf);
      }
      *pos += sizeof(short);
      bytesdone += 2;
      break;
    case DOT:
      tempi = temp[tempi] = 0;
      {
	int buf;
	sscanf(temp, "%d", &buf);
	*((char *)(*pos)) = (char)buf;
      }
      ++*pos;
      bytesdone++;
      dotsdone++;
      break;
    default:
      return 0;
    }

  /* Check to see if it looks like we handled a reasonable number of tokens */
  if ((dotsdone && (dotsdone != 4)) || ((rightpos == right) && 
      (bytesdone != sizeof(struct in_addr6))))
    return 0;

  /* Copy the new address to the buffer given, merging the left and right
     sides created by the double colon case */
  bzero(ap, sizeof(struct in_addr6));
  bcopy(left, ap, leftpos - left);
  bcopy(right, ap + sizeof(struct in_addr6) - (rightpos - right), 
	rightpos - right);

  return 1;
}

/* Standard API entry point. In order to bring in support for as many
   protocols as possible, use the appropriate existing parsers. */
int ascii2addr(af, cp, ap)
int af;
/* const */ char *cp;
void *ap;
{
  char addr[40], *addrptr;
  int addrlen;
  struct ns_addr addrns;

  addrlen = 0;
  addrptr = addr;
  switch(af) {
  case AF_INET:
    if (inet_aton(cp, (struct in_addr *)addr))
      addrlen = sizeof(struct in_addr);
    break;
  case AF_INET6:
    if (_inet6_aton(cp, (struct in_addr6 *)addr))
      addrlen = sizeof(struct in_addr6);
    break;
#ifdef AF_ISO
  case AF_ISO:
    addrptr = (char *)iso_addr(cp); 
    if (addrptr) 
      addrlen = sizeof(struct iso_addr);
    break;
#endif /* AF_ISO */
#ifdef AF_NS
  case AF_NS:
    addrns = ns_addr(cp);
    addrptr = (char *)&addrns;
    addrlen = sizeof(struct ns_addr);
    break;
#endif /* AF_NS */
#ifdef AF_LINK
  case AF_LINK:
    link_addr(cp, (struct sockaddr_dl *)addr);
    addrlen = sizeof(struct sockaddr_dl);
    break;
#endif /* AF_LINK */
  }
  if (addrlen) {
    memcpy(ap, addrptr, addrlen);
    return addrlen;
  } else
    return -1;
}

