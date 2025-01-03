/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
	
	$Id: pgpSockets-ip.c,v 1.8.6.1 1999/06/13 21:11:49 heller Exp $
____________________________________________________________________________*/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Memory.h>
#include "pgpSocketsLDAP.h"
#include "lber.h"
#include "ldap.h"
#include "ldap-int.h"

#include "portable.h"

#include "PGPSockets.h"
#include "pgpTypes.h"

#ifdef LDAP_REFERRALS
#ifdef USE_SYSCONF
#include <unistd.h>
#endif /* USE_SYSCONF */
#ifdef notyet
#ifdef NEED_FILIO
#include <sys/filio.h>
#else /* NEED_FILIO */
#include <sys/ioctl.h>
#endif /* NEED_FILIO */
#endif /* notyet */
#endif /* LDAP_REFERRALS */



/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */

static inet_aton(cp, addr)
	register char *cp;
	ulong *addr;
{
	register ulong val, base, n;
	register char c;
	ulong parts[4], *pp = parts;

	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, other=decimal.
		 */
		val = 0; base = 10;
		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}
		while ((c = *cp) != '\0') {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				cp++;
				continue;
			}
			if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) +
					(c + 10 - (islower(c) ? 'a' : 'A'));
				cp++;
				continue;
			}
			break;
		}
		if (*cp == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16-bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3 || val > 0xff)
				return (0);
			*pp++ = val, cp++;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp && (!isascii(*cp) || !isspace(*cp)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		*addr = PGPHostToNetLong(val);
	return (1);
}

#define	INADDR_NONE	0xffffffff		/* -1 return */

/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 *
 * I changed the name since the definition conflicted with the system
 *   supplied function.  marcd 4/3/98
 */
static long
inet_addr_internal(cp)
	register const char *cp;
{
	ulong val;

	if (inet_aton(cp, &val))
		return (val);

	return (INADDR_NONE);
}



int
connect_to_host( Sockbuf *sb, char *host, unsigned long address,
	int port, int async )
/*
 * if host == NULL, connect using address
 * "address" and "port" must be in network byte order
 * zero is returned upon success, -1 if fatal error, -2 EINPROGRESS
 * async is only used ifdef LDAP_REFERRALS (non-0 means don't wait for connect)
 * XXX async is not used yet!
 */
{
	int							rc, i, connected, use_hp;
	PGPSocketAddressInternet	sin;
	PGPHostEntry				*hp = NULL;
	PGPSocketRef				socketRef;
	PGPInt32					connectResult;
	PGPError					pgpErr = kPGPError_NoErr;
	
	(void) async;
	
#ifdef notyet
#ifdef LDAP_REFERRALS
	int			status;	/* for ioctl call */
#endif /* LDAP_REFERRALS */
#endif /* notyet */

	Debug( LDAP_DEBUG_TRACE, "connect_to_host: %s:%d\n",
	    ( host == NULL ) ? "(by address)" : host,
		PGPNetToHostShort( (short)port ), 0 );

	connected = use_hp = 0;

	if ( host != NULL && ( address = inet_addr_internal( host )) == -1 ) {
#if PGP_WIN32
		pgpErr = PGPStartAsyncGetHostByName(host, &sb->ref);
		if (pgpErr != kPGPError_NoErr) {
			return ( -1 );
		}
		pgpErr = PGPWaitForGetHostByName(sb->ref, &address);
		sb->ref = kPGPInvalidAsyncHostEntryRef;
		if (pgpErr != kPGPError_NoErr) {
			return ( -1 );
		}
#else
		if ( (hp = PGPGetHostByName( host )) == NULL ) {
			return( -1 );
		}
		use_hp = 1;
#endif
	}

	rc = -1;
	for ( i = 0; !use_hp || ( hp->h_addr_list[ i ] != 0 ); i++ )
	{
		socketRef = PGPOpenSocket( kPGPAddressFamilyInternet, kPGPSocketTypeStream, kPGPTCPProtocol );
		if( socketRef == kInvalidPGPSocketRef )
		{
			return( -1 );
		}
#ifdef notyet
#ifdef LDAP_REFERRALS
		status = 1;
		if ( async && ioctl( s, FIONBIO, (caddr_t)&status ) == -1 ) {
			Debug( LDAP_DEBUG_ANY, "FIONBIO ioctl failed on %d\n",
			    s, 0, 0 );
		}
#endif /* LDAP_REFERRALS */
#endif /* notyet */
		(void)memset( (char *)&sin, 0, sizeof( sin ));
		sin.sin_family = kPGPAddressFamilyInternet;
		sin.sin_port = port;
		SAFEMEMCPY( (char *) &sin.sin_addr.s_addr,
		    ( use_hp ? (char *) hp->h_addr_list[ i ] :
		    (char *) &address ), sizeof( sin.sin_addr.s_addr) );

		connectResult = PGPConnect( socketRef, (PGPSocketAddress *)&sin,
		    sizeof( sin ));

		if ( connectResult >= 0 ) {
			connected = 1;
			rc = 0;
			break;
		} else {
#ifdef notyet
#ifdef LDAP_REFERRALS
#ifdef EAGAIN
			if ( errno == EINPROGRESS || errno == EAGAIN ) {
#else /* EAGAIN */
			if ( errno == EINPROGRESS ) {
#endif /* EAGAIN */
				Debug( LDAP_DEBUG_TRACE,
					"connect would block...\n", 0, 0, 0 );
				rc = -2;
				break;
			}
#endif /* LDAP_REFERRALS */
#endif /* notyet */

			PGPCloseSocket( socketRef );
			if ( !use_hp ) {
				break;
			}
		}
	}

	sb->sb_sd = socketRef;
//	sb->sb_options |= LBER_NO_READ_AHEAD; /* jason */

	if ( connected ) {
#ifdef notyet
#ifdef LDAP_REFERRALS
		status = 0;
		if ( !async && ioctl( s, FIONBIO, (caddr_t)&on ) == -1 ) {
			Debug( LDAP_DEBUG_ANY, "FIONBIO ioctl failed on %d\n",
			    s, 0, 0 );
		}
#endif /* LDAP_REFERRALS */
#endif /* notyet */

	}

	return( rc );
}



void
close_connection( Sockbuf *sb )
{
    PGPCloseSocket( sb->sb_sd );
}


