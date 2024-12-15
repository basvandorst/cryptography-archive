/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
*/

#ifndef PGPSOCKETS_H
#define PGPSOCKETS_H

#include "pgpTypes.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *			PGPSocketRef;
typedef long			PGPSocketsError;

typedef struct PGPInternetAddress {
	union {
		struct {
			uchar	s_b1;
			uchar	s_b2;
			uchar	s_b3;
			uchar	s_b4;
		}					S_un_b;
		struct {
			ushort	s_w1;
			ushort	s_w2;
		}					S_un_w;
		ulong				S_addr;
	} S_un;
#define s_addr	S_un.S_addr
} PGPInternetAddress;


typedef struct PGPSocketAddressInternet {
	short				sin_family;
	ushort				sin_port;
	PGPInternetAddress	sin_addr;
	char				sin_zero[8];
} PGPSocketAddressInternet;

typedef struct PGPSocketAddress {
	ushort	sa_family;
	char	sa_data[14];
} PGPSocketAddress;

typedef struct PGPHostEntry {
	char *	h_name;
	char **	h_aliases;		/* Not currently supported */
	short	h_addrtype;
	short	h_length;
	char **	h_addr_list;
#define h_addr	h_addr_list[0]
} PGPHostEntry;


typedef struct PGPIOVector {
	void *	iovbase;
	size_t	iov_len;
} PGPIOVector;

/*
typedef struct PGPMessageHeader {
	void *			msg_name;
	short			msg_namelen;
	PGPIOVector *	msg_iov;
	short			msg_iovlen;
	void *			msg_control;
	short			msg_controllen;
	short			msg_flags;
} PGPMessageHeader;
*/


/* Select types and defines */
#ifndef PGPSOCKETSET_SETSIZE
#define PGPSOCKETSET_SETSIZE	64
#endif

typedef struct PGPSocketSet {
	ushort			fd_count;
	PGPSocketRef	fd_array[PGPSOCKETSET_SETSIZE];
} PGPSocketSet;

#define PGPSOCKETSET_CLEAR(socketRef, set)	do {											\
						ushort	__i;														\
						for (__i = 0; __i < ((PGPSocketSet * (set))->fd_count; __i++) {		\
							if (((PGPSocketSet *) (set))->fd_array[__i] == socketRef) {		\
								while (__i < (((PGPSocketSet *) (set))->fd_count - 1)) {	\
									((PGPSocketSet *) (set))->fd_array[__i] =				\
										((PGPSocketSet *) (set))->fd_array[__i + 1];		\
									__i++;													\
								}															\
								((PGPSocketSet *) (set))->fd_count--;						\
								break;														\
							}																\
						}																	\
											} while (0)
										
#define PGPSOCKETSET_SET(socketRef, set)	do {											\
if (((PGPSocketSet *) (set))->fd_count < PGPSOCKETSET_SETSIZE) {							\
	((PGPSocketSet *) (set))->fd_array[((PGPSocketSet *) (set))->fd_count++] = (socketRef);	\
}																							\
											} while (0)

#define PGPSOCKETSET_ZERO(set)	(((PGPSocketSet *) (set))->fd_count = 0)

short __PGPSocketsIsSet(PGPSocketRef inSocketRef, PGPSocketSet * inSocketSet);
#define PGPSOCKETSET_ISSET(socketRef, set)	__PGPSocketsIsSet((socketRef), (set))

typedef struct PGPSocketsTimeValue {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
}PGPSocketsTimeValue;


/*
	Many Unix applications check errno and h_errno insert this into your code
	if necessary

	#ifndef errno
	#define errno	(PGPGetLastSocketsError())
	#define h_errno	(PGPGetLastHostError())
	#endif
*/


/* Special socket values */
#define	kPGPInvalidSocket	NULL

/* Address families */
enum {
	kPGPAddressFamilyUnspecified	=	0,
	kPGPAddressFamilyUnix			=	1,
	kPGPAddressFamilyInternet		=	2
};



/* Types */
enum {
	kPGPSocketTypeUnspecified		=	0,
	kPGPSocketTypeStream			=	1,
	kPGPSocketTypeDatagram			=	2
};



/* Protocols */
enum {
	kPGPIPProtocol	=	0, 
	kPGPTCPProtocol	=	6, 
	kPGPUDPProtocol	=	22
};



/* Send flags */
enum {
	kPGPSendFlagNone		=	0,
	kPGPSendFlagDontRoute	=	1,	/* Not supported */
	kPGPSendFlagOOB			=	2
};




/* Receive flags */
enum {
	kPGPReceiveFlagNone		=	0
};



/* Internet Addresses */
enum {
	kPGPInternetAddressAny	=	0x00000000
};

extern PGPSocketAddressInternet	kPGPAddressAny;



/* Errors */
enum
{
	kPGPSocketsErrorUnknown			=	(PGPSocketsError)10000,
	kPGPSocketsErrorProtoNoSupport	=	(PGPSocketsError)10001,
	kPGPSocketsErrorAFNoSupport		=	(PGPSocketsError)10002,
	kPGPSocketsErrorBadF			=	(PGPSocketsError)10003,
	kPGPSocketsErrorNotSock			=	(PGPSocketsError)10004,
	kPGPSocketsErrorFault			=	(PGPSocketsError)10005,
	kPGPSocketsErrorInval			=	(PGPSocketsError)10006,
	kPGPSocketsErrorNetDown			=	(PGPSocketsError)10007,
	kPGPSocketsErrorNotConn			=	(PGPSocketsError)10008,
	kPGPSocketsErrorOpNotSupp		=	(PGPSocketsError)10009,
	kPGPSocketsErrorMFile			=	(PGPSocketsError)10010,
	kPGPSocketsErrorConnRefused		=	(PGPSocketsError)10011,
	kPGPSocketsErrorAddrNotAvail	=	(PGPSocketsError)10012,
	kPGPSocketsErrorMsgSize			=	(PGPSocketsError)10013,
	kPGPSocketsErrorBadQLen			=	(PGPSocketsError)10014,
	kPGPSocketsErrorAddrInUse		=	(PGPSocketsError)10015,
	kPGPSocketsErrorNoBufs			=	(PGPSocketsError)10016,
	kPGPSocketsErrorDestAddReq		=	(PGPSocketsError)10017,
	kPGPSocketsErrorUserCanceled	=	(PGPSocketsError)10018,

	kPGPSocketsErrorHostNotFound	=	(PGPSocketsError)11000
};

/* Callback functions */
typedef void (* PGPSocketsCallback)(void * inData);
void			PGPSetDefaultCallback(PGPSocketsCallback inCallback, void * inData);

short			PGPSetCallback(PGPSocketRef inSocketRef, PGPSocketsCallback inCallback,
					void * inData);

/* Initialization and termination headers */
short			PGPSocketsInit(void);
void			PGPSocketsCleanup(void);

/* Socket creation and destruction */
PGPSocketRef	PGPOpenSocket(short inAddressFamily, short inSocketType,
						short inSocketProtocol);
short			PGPCloseSocket(PGPSocketRef inSocketRef);

/* Endpoint binding */
short			PGPBindSocket(PGPSocketRef inSocketRef,
						const PGPSocketAddress * inAddress, short inAddressLength);
short			PGPConnect(PGPSocketRef inSocketRef,
						const PGPSocketAddress * inServerAddress, short inAddressLength);

/* Send functions */
short  			PGPSend(PGPSocketRef inSocketRef, const void * inBuffer,
						short inBufferLength, ushort inFlags);
size_t			PGPWrite(PGPSocketRef inSocketRef, const void * inBuffer,
						size_t inBufferLength);
short			PGPGatherWrite(PGPSocketRef inSocketRef, const PGPIOVector * inVectors,
                 		size_t inCount);
short			PGPSendTo(PGPSocketRef inSocketRef, const void * inBuffer,
						short inBufferLength, ushort inFlags,
						PGPSocketAddress * inAddress, short inAddressLength);
/*				We currently do not support the sendmsg call
short			PGPSendMessage(PGPSocketRef inSocketRef,
						const PGPMessageHeader * inMessage, ushort inFlags);
*/

/* Receive functions */
short			PGPReceive(PGPSocketRef inSocketRef, void * outBuffer, short inBufferSize,
						ushort inFlags);
short			PGPRead(PGPSocketRef inSocketRef, void * outBuffer, size_t inBufferSize);
short			PGPScatterRead(PGPSocketRef inSocketRef, const PGPIOVector * inVectors,
						size_t inCount);
short			PGPReceiveFrom(PGPSocketRef inSocketRef, void * outBuffer,
						short inBufferSize, ushort inFlags,
						PGPSocketAddress * outAddress, short * ioAddressLength);
/*				We currently do not support the recvmsg call
short			PGPReceiveMessage(PGPSocketRef inSocketRef,
						const PGPMessageHeader * outMessage, ushort inFlags);
*/

/* Server functions */
short			PGPListen(PGPSocketRef inSocketRef, short inMaxBacklog);
PGPSocketRef	PGPAccept(PGPSocketRef inSocketRef, PGPSocketAddress * outAddress,
						short * ioAddressLength);

/* Select */
/* Note that inNumSetCount is not used under the Mac and Windows */
short 			PGPSelect(short inNumSetCount, PGPSocketSet * ioReadSet,
						PGPSocketSet * ioWriteSet, PGPSocketSet * ioErrorSet,
						const PGPSocketsTimeValue * inTimeout);

/* DNS services */
PGPHostEntry *	PGPGetHostByName(const char * inName);
PGPHostEntry* PGPGetHostByAddress(const char* addr, int len, int type);


/* Net byte ordering macros */
#ifdef PGP_MACINTOSH
#define PGPHostToNetLong(x)			(x)
#define PGPHostToNetShort(x)		(x)
#define PGPNetToHostLong(x)			(x)
#define PGPNetToHostShort(x)		(x)
#else
long	PGPHostToNetLong(long x);
short	PGPHostToNetShort(short x);
long	PGPNetToHostLong(long x);
short	PGPNetToHostShort(short x);
#endif

/* Error reporting */
PGPSocketsError	PGPGetLastSocketsError(void);
PGPSocketsError	PGPGetLastHostError(void);


short PGPGetPeerName(PGPSocketRef s, PGPSocketAddress *name, int* namelen);
ulong PGPDottedToInternetAddress(const char *cp);
char *PGPInternetAddressToDottedString(PGPInternetAddress addr);

/* Not implemented yet */
short PGPGetSocketName(PGPSocketRef s, PGPSocketAddress *name, int* namelen);
short PGPGetSockOpt(PGPSocketRef s, int level, int optname, char* optval, int* optlen);
short PGPSetSockOpt(PGPSocketRef s, int level, int optname,const char* optval, int optlen);
short PGPShutdown(PGPSocketRef s, int how);


#ifdef __cplusplus
}
#endif

#endif /* PGPSOCKETS_H */



