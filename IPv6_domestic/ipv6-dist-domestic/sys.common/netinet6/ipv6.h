/*
 * ipv6.h  --  Definitions for internet protocol version 6.
 *
 * Copyright 1995 by Dan McDonald, Bao Phan, and Randall Atkinson,
 *	All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 */

/*----------------------------------------------------------------------
#       @(#)COPYRIGHT   1.1 (NRL) 17 January 1995

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

#ifndef _IPV6_H
#define _IPV6_H

#define IPV6VERSION 6

/*
 * Header structures.
 */

#include <netinet6/in6_types.h>  /* Need data types. */

struct ipv6
{
  u_int32 ipv6_versfl;      /* Version and flow label word. */
 
  u_int16 ipv6_length;      /* Datagram length (not including the length
			       of this header). */
  u_int8 ipv6_nexthdr;      /* Next header type. */
  u_int8 ipv6_hoplimit;     /* Hop limit. */
 
  struct in_addr6 ipv6_src; /* Source address. */
  struct in_addr6 ipv6_dst; /* Destination address. */
};

/*
 * Macros and defines for header fields, and values thereof.
 * Assume things are in host order for these three macros.
 */

#define IPV6_VERSION(h) ((h)->ipv6_versfl >> 28)
#define IPV6_PRIORITY(h)  (((h)->ipv6_versfl & 0x0f000000) >> 24)
#define IPV6_FLOWID(h)  ((h)->ipv6_versfl & 0x00ffffff)

#define MAXHOPLIMIT 255
#define IPV6_MINMTU 576

/*
 * Other IPv6 header definitions.
 */

/* Fragmentation header & macros for it.  NOTE:  Host order assumption. */

struct ipv6_fraghdr
{
  u_int8 frag_nexthdr;      /* Next header type. */
  u_int8 frag_reserved;
  u_int16 frag_bitsoffset;  /* More bit and fragment offset. */
  u_int32 frag_id;          /* Fragment identifier. */
};

#define FRAG_MOREMASK 0x1
#define FRAG_OFFMASK 0xFFF8
#define FRAG_MORE_BIT(fh)       ((fh)->frag_bitsoffset & FRAG_MOREMASK)
#define FRAG_OFFSET(fh)         ((fh)->frag_bitsoffset & FRAG_OFFMASK)

/* Source routing header.  Host order assumption for macros. */

struct ipv6_srcroute0

{
  u_int8 i6sr_nexthdr;           /* Next header type. */
  u_int8 i6sr_type;              /* Routing type, for loose source
				    route, should be 0. */
  u_int8 i6sr_numaddrs;          /* Number of addresses. */
  u_int8 i6sr_nextaddr;          /* Next address in list. */
  u_int8 i6sr_reserved;          /* 8 bits of reserved padding. */
  u_int8 i6sr_mask[3];           /* 8 bits of reserved padding. */
};

#define I6SR_BITMASK(i6sr)      ((i6sr)->i6sr_reserved & 0xffffff)

/* Options header.  For "ignoreable" options. */

struct ipv6_opthdr
{
  u_int8 oh_nexthdr;        /* Next header type. */
  u_int8 oh_extlen;         /* Header extension length. */
  u_int8 oh_data[6];        /* Option data, may be reserved for
			       alignment purposes. */
};

#define OPT_PAD1 0
#define OPT_PADN 1
#define OPT_JUMBO 194

struct ipv6_option
{
  u_int8 opt_type;      /* Option type. */
  u_int8 opt_datalen;   /* Option data length. */
  u_int8 opt_data[1];   /* Option data. */
};

struct ipv6_auth
{
  u_int8 auth_nexthdr;          /* Next header type. */
  u_int8 auth_datalen;          /* Length of authentication data in 8 byte
                                   chunks. */
  u_int16 reserved;             /* 16-bits of reserved padding. */
  u_int32 auth_spi;             /* Security paramaters index */
};

#endif /* _IPV6_H */
