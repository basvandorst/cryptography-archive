/*
 * tcp.h  interface to MCS's TCP routines
 */
 
#include <Dialogs.h>
#ifdef SUPPORT_OPENTRANSPORT
#include <OpenTransport.h>
#include <OpenTptInternet.h>
#endif /* SUPPORT_OPENTRANSPORT */
#include <MacTCP.h>

#ifndef TCP_BUFSIZ
#define TCP_BUFSIZ	8192
#endif /* TCP_BUFSIZ */

typedef struct tcpstream {
	StreamPtr			tcps_sptr;			/* stream pointer for MacTCP TCP PB calls */
#ifdef SUPPORT_OPENTRANSPORT
	EndpointRef			tcps_ep;			/* OpenTransport end point */
#endif /* SUPPORT_OPENTRANSPORT */
	short				tcps_data;			/* count of packets on read queue */
	short				drefnum;			/* driver ref num, for convenience */
	Boolean				tcps_connected;		/* true if connection was made */
	Boolean				tcps_terminated;	/* true if connection no longer exists */
#ifdef SUPPORT_OPENTRANSPORT
	InetHost			tcps_remoteaddr;	/* Address of our peer */
	InetPort			tcps_remoteport;	/* Port number of our peer */
#endif /* SUPPORT_OPENTRANSPORT */
	unsigned char		*tcps_buffer;		/* buffer given over to system to use */
	struct tcpstream	*tcps_next;			/* next one in chain */
	TCPNotifyUPP		tcps_notifyupp;		/* universal proc pointer for notify routine */
} tcpstream, *tcpstreamptr;

/*
 * the Unix-y struct timeval
 */
struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};

typedef		UInt8 byte;

#define TCP_IS_TERMINATED( tsp )		(tsp)->tcps_terminated

/*
 * function prototypes
 */
#ifdef SUPPORT_OPENTRANSPORT
OSStatus	tcp_init(void);
void		tcp_shutdown(void);
Boolean		tcp_have_opentransport( void );
#endif /* SUPPORT_OPENTRANSPORT */
tcpstream 	*tcpopen( unsigned char * buf, long buflen );
tcp_port	tcpconnect( tcpstream *s, ip_addr addr, tcp_port port );
short		tcpclose( tcpstream *s );
long		tcpread(  tcpstream *s, byte timeout, unsigned char * rbuf,
				unsigned short rbuflen, DialogPtr dlp );
long		tcpwrite( tcpstream *s, unsigned char * wbuf, unsigned short wbuflen );
short		tcpselect( tcpstream *s, struct timeval * timeout );
short		tcpreadready( tcpstream *tsp );
short		tcpgetpeername( tcpstream * tsp, ip_addr *addrp, tcp_port *portp );

#ifdef SUPPORT_OPENTRANSPORT
short		gethostinfobyname( char *host, InetHostInfo *hip );
short		gethostinfobyaddr(InetHost addr, InetHostInfo *hip );
short		ipaddr2str( InetHost ipaddr, char *addrstr );
#else /* SUPPORT_OPENTRANSPORT */
short		gethostinfobyname( char *host, struct hostInfo *hip );
short		gethostinfobyaddr( ip_addr addr, struct hostInfo *hip );
short		ipaddr2str( ip_addr ipaddr, char *addrstr );
#endif /* SUPPORT_OPENTRANSPORT */
