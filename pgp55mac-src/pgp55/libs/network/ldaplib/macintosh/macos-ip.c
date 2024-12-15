/*
 *  Copyright (c) 1995 Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  macos-ip.c -- Macintosh platform-specific TCP & UDP related code
 */

#ifndef lint 
static char copyright[] = "@(#) Copyright (c) 1995 Regents of the University of Michigan.\nAll rights reserved.\n";
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Memory.h>
#include "macos.h"
#include "lber.h"
#include "ldap.h"
#include "ldap-int.h"

#include "portable.h"
#include "lber.h"
#include "ldap.h"

#include "PGPSockets.h"


#ifdef MACOS
PGPSocketRef *	gCallbackDataSocket = NULL;
#endif

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

#ifdef MACOS
#define tcp_close( s )		PGPCloseSocket( s )
#else /* MACOS */
#ifdef DOS
#ifdef PCNFS
#define tcp_close( s )		close( s )
#endif /* PCNFS */
#ifdef NCSA
#define tcp_close( s )		netclose( s ); netshut()
#endif /* NCSA */
#if defined(WINSOCK) || defined(WIN32)		/* marcd */
#define tcp_close( s )		closesocket( s ); WSACleanup();
#endif /* WINSOCK */
#else /* DOS */
#define tcp_close( s )		close( s )
#endif /* DOS */
#endif /* MACOS */


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
		*addr = htonl(val);
	return (1);
}

#define	INADDR_NONE	0xffffffff		/* -1 return */

/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
static long
inet_addr(cp)
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
	PGPHostEntry				*hp;
	PGPSocketRef				socketRef;
	short						connectResult;
	
	(void) async;
	
#ifdef notyet
#ifdef LDAP_REFERRALS
	int			status;	/* for ioctl call */
#endif /* LDAP_REFERRALS */
#endif /* notyet */

	Debug( LDAP_DEBUG_TRACE, "connect_to_host: %s:%d\n",
	    ( host == NULL ) ? "(by address)" : host, ntohs( port ), 0 );

	connected = use_hp = 0;

	if ( host != NULL && ( address = inet_addr( host )) == -1 ) {
		if ( (hp = PGPGetHostByName( host )) == NULL ) {
			return( -1 );
		}
		use_hp = 1;
	}

	rc = -1;
	for ( i = 0; !use_hp || ( hp->h_addr_list[ i ] != 0 ); i++ )
	{
		socketRef = PGPOpenSocket( kPGPAddressFamilyInternet, kPGPSocketTypeStream, kPGPTCPProtocol );
		if( socketRef == kPGPInvalidSocket )
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

#ifdef MACOS
		if (gCallbackDataSocket != NULL) {
			*gCallbackDataSocket = socketRef;
		}
#endif
		connectResult = PGPConnect( socketRef, (PGPSocketAddress *)&sin,
		    sizeof( sin ));
#ifdef MACOS
		if (gCallbackDataSocket != NULL) {
			*gCallbackDataSocket = NULL;
		}
#endif
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
	sb->sb_options |= LBER_NO_READ_AHEAD;

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
    tcp_close( sb->sb_sd );
}


#ifdef KERBEROS
char *
host_connected_to( Sockbuf *sb )
{
	struct hostent		*hp;
	char			*p;
	int			len;
	struct sockaddr_in	sin;

	(void)memset( (char *)&sin, 0, sizeof( struct sockaddr_in ));
	len = sizeof( sin );
	if ( getpeername( sb->sb_sd, (struct sockaddr *)&sin, &len ) == -1 ) {
		return( NULL );
	}

	/*
	 * do a reverse lookup on the addr to get the official hostname.
	 * this is necessary for kerberos to work right, since the official
	 * hostname is used as the kerberos instance.
	 */
	if (( hp = gethostbyaddr( (char *) &sin.sin_addr,
	    sizeof( sin.sin_addr ), kPGPAddressFamilyInternet )) != NULL ) {
		if ( hp->h_name != NULL ) {
			return( strdup( hp->h_name ));
		}
	}

	return( NULL );
}
#endif /* KERBEROS */


#ifdef LDAP_REFERRALS
/* for UNIX */
struct selectinfo {
	fd_set	si_readfds;
	fd_set	si_writefds;
	fd_set	si_use_readfds;
	fd_set	si_use_writefds;
};


void
mark_select_write( LDAP *ld, Sockbuf *sb )
{
	struct selectinfo	*sip;

	sip = (struct selectinfo *)ld->ld_selectinfo;

	if ( !FD_ISSET( sb->sb_sd, &sip->si_writefds )) {
		FD_SET( sb->sb_sd, &sip->si_writefds );
	}
}


void
mark_select_read( LDAP *ld, Sockbuf *sb )
{
	struct selectinfo	*sip;

	sip = (struct selectinfo *)ld->ld_selectinfo;

	if ( !FD_ISSET( sb->sb_sd, &sip->si_readfds )) {
		FD_SET( sb->sb_sd, &sip->si_readfds );
	}
}


void
mark_select_clear( LDAP *ld, Sockbuf *sb )
{
	struct selectinfo	*sip;

	sip = (struct selectinfo *)ld->ld_selectinfo;

	FD_CLR( sb->sb_sd, &sip->si_writefds );
	FD_CLR( sb->sb_sd, &sip->si_readfds );
}


int
is_write_ready( LDAP *ld, Sockbuf *sb )
{
	struct selectinfo	*sip;

	sip = (struct selectinfo *)ld->ld_selectinfo;

	return( FD_ISSET( sb->sb_sd, &sip->si_use_writefds ));
}


int
is_read_ready( LDAP *ld, Sockbuf *sb )
{
	struct selectinfo	*sip;

	sip = (struct selectinfo *)ld->ld_selectinfo;

	return( FD_ISSET( sb->sb_sd, &sip->si_use_readfds ));
}


void *
new_select_info()
{
	struct selectinfo	*sip;

	if (( sip = (struct selectinfo *)calloc( 1,
	    sizeof( struct selectinfo ))) != NULL ) {
		FD_ZERO( &sip->si_readfds );
		FD_ZERO( &sip->si_writefds );
	}

	return( (void *)sip );
}


void
free_select_info( void *sip )
{
	free( sip );
}


int
do_ldap_select( LDAP *ld, struct timeval *timeout )
{
	struct selectinfo	*sip;
	static int		tblsize;

	Debug( LDAP_DEBUG_TRACE, "do_ldap_select\n", 0, 0, 0 );

	if ( tblsize == 0 ) {
#ifdef USE_SYSCONF
		tblsize = sysconf( _SC_OPEN_MAX );
#else /* USE_SYSCONF */
		tblsize = getdtablesize();
#endif /* USE_SYSCONF */
	}

	sip = (struct selectinfo *)ld->ld_selectinfo;
	sip->si_use_readfds = sip->si_readfds;
	sip->si_use_writefds = sip->si_writefds;
	
	return( select( tblsize, &sip->si_use_readfds, &sip->si_use_writefds,
	    NULL, timeout ));
}

#if 0
		if ( sip->si_streaminfo[ i ].tcpsi_stream == (tcpstream *)sb->sb_sd ) {
#ifdef LDAP_DEBUG
				if ( sip->si_streaminfo[ i ].tcpsi_is_read_ready ) {
					Debug( LDAP_DEBUG_TRACE, "is_read_ready: stream %x READY\n",
							(tcpstream *)sb->sb_sd, 0, 0 );
				} else {
					Debug( LDAP_DEBUG_TRACE, "is_read_ready: stream %x Not Ready\n",
							(tcpstream *)sb->sb_sd, 0, 0 );
				}
#endif /* LDAP_DEBUG */
				return( sip->si_streaminfo[ i ].tcpsi_is_read_ready ? 1 : 0 );
			}
		}
	}

	Debug( LDAP_DEBUG_TRACE, "is_read_ready: stream %x: NOT FOUND\n", (tcpstream *)sb->sb_sd, 0, 0 );
	return( 0 );
}
#endif


void *
new_select_info()
{
	return( (void *)calloc( 1, sizeof( struct selectinfo )));
}


void
free_select_info( void *sip )
{
	if ( sip != NULL )
	{
		// Fixes a leak:
		if( ((struct selectinfo *) sip )->si_streaminfo != NULL )
		{
			free( ((struct selectinfo *) sip )->si_streaminfo );
		}
		
		free( sip );
	}
}


int
do_ldap_select( LDAP *ld, struct timeval *timeout )
{
	struct selectinfo	*sip;
	Boolean				ready, gotselecterr;
	long				ticks, endticks;
	short				i, err;

	Debug( LDAP_DEBUG_TRACE, "do_ldap_select\n", 0, 0, 0 );

	if (( sip = (struct selectinfo *)ld->ld_selectinfo ) == NULL ) {
		return( -1 );
	}

	if ( sip->si_count == 0 ) {
		return( 1 );
	}

	if ( timeout != NULL ) {
		endticks = 60 * timeout->tv_sec + ( 60 * timeout->tv_usec ) / 1000000 + TickCount();
	}

	for ( i = 0; i < sip->si_count; ++i ) {
		if ( sip->si_streaminfo[ i ].tcpsi_check_read ) {
			sip->si_streaminfo[ i ].tcpsi_is_read_ready = false;
		}
	}

	ready = gotselecterr = false;
	do {
		for ( i = 0; i < sip->si_count; ++i ) {
			if ( sip->si_streaminfo[ i ].tcpsi_check_read && !sip->si_streaminfo[ i ].tcpsi_is_read_ready ) {
				if (( err = tcpreadready( sip->si_streaminfo[ i ].tcpsi_stream )) > 0 ) {
					sip->si_streaminfo[ i ].tcpsi_is_read_ready = ready = true;
				} else if ( err < 0 ) {
					gotselecterr = true;
				}
			}
		}
		if ( !ready && !gotselecterr ) {
			Delay( 2L, &ticks );
			SystemTask();
		}
	} while ( !ready && !gotselecterr && ( timeout == NULL || ticks < endticks ));

	Debug( LDAP_DEBUG_TRACE, "do_ldap_select returns %d\n", ready ? 1 : ( gotselecterr ? -1 : 0 ), 0, 0 );
	return( ready ? 1 : ( gotselecterr ? -1 : 0 ));
}
#endif /* LDAP_REFERRALS */