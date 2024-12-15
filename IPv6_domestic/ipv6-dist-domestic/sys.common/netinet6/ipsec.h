/*----------------------------------------------------------------------
  ipsec.h:  Global declarations for IP Security services, not including
            PF_KEY stuff.  rja

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

#ifndef _IPSEC_H

#define _IPSEC_H

/* Symbolic names for System Security Levels */
/* Note: the concept of system security levels needs revisiting. rja */
#define IPSEC_LEVEL_DEFAULT	0 /* send plaintext, accept any */
#define IPSEC_LEVEL_USE		1 /* use outbound, don't require inbound */
#define IPSEC_LEVEL_REQUIRE	2 /* use both directions */
#define IPSEC_LEVEL_UNIQUE	3 /* use both directions, 
				     and outbound uses a unique key */

/* Symbolic names for Security Types */ 
#define SECURITY_TYPE_NONE		0

#define SECURITY_TYPE_AUTH_AH           1
#define SECURITY_TYPE_AUTH_RSVP         2
#define SECURITY_TYPE_AUTH_OSPF         3
#define SECURITY_TYPE_AUTH_RIPV2        4
#define SECURITY_TYPE_AUTH_MIPV4        5
#define SECURITY_TYPE_AUTH_MIPV6        6

#define SECURITY_TYPE_ENCR_ESPTRANSPORT	8
#define SECURITY_TYPE_ENCR_ESPTUNNEL	9

#define SECURITY_TYPE_MAX		9

/* Symbolic names for algorithms */
#define IPSEC_ALGTYPE_AUTH_MD5          0
#define IPSEC_ALGTYPE_ESP_DES_CBC       0
/* 3DES is not yet coded; once coded, it will be renumbered 1
   and ESP_DUMMY will be numbered 2 within the #ifdef DEBUG. 
   The symbol was added early for the benefit of netinet6/key.c rja */
#define IPSEC_ALGTYPE_ESP_3DES          3  

#ifdef DEBUG
  /* "dummy" and "cksum" algorithms provide no security and are
     only used for testing purposes... */
  #define IPSEC_ALGTYPE_AUTH_DUMMY        1 
  #define IPSEC_ALGTYPE_AUTH_CKSUM        2 

  #define IPSEC_ALGTYPE_ESP_DUMMY         1
#endif

/* Security association data for IP Security */
struct ipsec_assoc {
  u_int8  len;                  /* Length of the data (for radix) */
  u_int8  type;                 /* Type of association */
  u_int8  state;                /* State of the association */
  u_int8  label;                /* Sensitivity label (unused) */
  u_int32 spi;                  /* SPI */
  u_int8  keylen;		/* Key length */
  u_int8  ivlen;		/* Initialization vector length */
  u_int8  algorithm;		/* Algorithm switch index */
  u_int8  lifetype;             /* Type of lifetime */
  caddr_t iv;			/* Initialization vector */
  caddr_t key;			/* Key */
  u_int32 lifetime1;            /* Lifetime value 1 */
  u_int32 lifetime2;            /* Lifetime value 2 */
  struct sockaddr_in6 src;      /* Source host address */
  struct sockaddr_in6 dst;      /* Destination host address */
  struct sockaddr_in6 from;     /* Originator of association */
};

struct ah_stat_table {
  /* kernel statistics relating to IP Authentication Header */
  u_long    ah_sent;
  u_long    ah_accepted;
  u_long    ah_in_errs;
  u_long    ah_out_errs;
  u_long    ah_missing;
  u_long    ah_spoof;

  u_long    ah_sa_unavailable;
  u_long    ah_sa_invalid;
  u_long    ah_sa_delayed;
};

struct esp_stat_table {
  u_long    esp_transport_sent; 
  u_long    esp_tunnel_sent; 
  u_long    esp_accepted;
  u_long    esp_trans_missing;
  u_long    esp_tunnel_missing;
  u_long    esp_in_errs;
  u_long    esp_trans_out_errs;
  u_long    esp_tunnel_out_errs;
  u_long    esp_tunnel_spoof;

  u_long    esp_sa_unavailable;
  u_long    esp_sa_invalid;
  u_long    esp_sa_delayed;
};

struct  ah_stat_table  ah_stats;  /* lives and initialised in ipsec.c */
struct esp_stat_table esp_stats;  /* lives and initialised in ipsec.c */


#ifdef KERNEL
/* The array below SHOULD be modifyable using a sysctl or similar 
   so that one can tune the system security without building a new
   kernel, but support for that is not yet implemented. */
u_int ipsec_system_sec_level[4];  /* used in ipsec_policy.c */

/* lives in ipsec.c */
void        ipsec_init __P((void));

#endif /* KERNEL */

#endif /* _IPSEC_H */
