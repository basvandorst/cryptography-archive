/*
** Copyright (C) 1997 Network Associates Inc. and affiliated companies.
** All rights reserved.
*/

#include "pgpTypes.h"
#include "PGPSockets.h"


#define SocketNumToSocketRef( num ) ( (PGPSocketRef)(long)(num) )
#define SocketRefToSocketNum( ref ) ( (int)(long)(ref) )


short PGPSocketsInit(void)
{
	return 0; 
}

void PGPSocketsCleanup(void)
{
	return;
}

void PGPSetDefaultCallback(PGPSocketsCallback inCallback, void * inData)
{
	return;
}

short PGPSetCallback(PGPSocketRef s, PGPSocketsCallback inCallback,
						void * inData)
{
	return kPGPSocketsErrorUnknown;
}

PGPSocketRef PGPOpenSocket(short af, short type, short protocol)
{
	int s;

	s = socket(af, type, protocol);

	return SocketNumToSocketRef( s );
}

short PGPCloseSocket(PGPSocketRef s)
{
	short returnValue;

	returnValue = close( SocketRefToSocketNum( s ) );

	return returnValue;
}

short PGPBindSocket(PGPSocketRef s, const PGPSocketAddress* my_addr,
			short addrlen)
{
	short returnValue;

	returnValue = bind( SocketRefToSocketNum( s ),
		(const struct sockaddr*)my_addr, addrlen);

	return returnValue;
}

short PGPConnect(PGPSocketRef s, const PGPSocketAddress* serv_addr,
			short addrlen)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue =  connect(	SocketRefToSocketNum( s ), 
						(const struct sockaddr*)serv_addr, 
						addrlen);

	return returnValue;
}

short PGPListen(PGPSocketRef s, short backlog)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue = listen(SocketRefToSocketNum( s ), backlog);

	return returnValue;
}

short PGPSelect(short nfds, PGPSocketSet* readfds, PGPSocketSet* writefds,
			PGPSocketSet* exceptfds, const PGPSocketsTimeValue* timeout)
{
	return select(nfds, (fd_set*) readfds, (fd_set*) writefds,
				(fd_set*) exceptfds, (const struct timeval *) timeout);
}
					
short PGPSend(PGPSocketRef s, const void* msg, short len, ushort flags)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue =  send(SocketRefToSocketNum( s ), (const char*) msg, len,
							flags);

	return returnValue;
}

short PGPReceive(PGPSocketRef s, void* buf, short len, ushort flags)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue =  recv(SocketRefToSocketNum( s ), (char*) buf, len, flags);

	return returnValue;
}

short PGPReceiveFrom(PGPSocketRef s, void* buf, short len, ushort flags,
			PGPSocketAddress* from, short* fromlen)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue =  recvfrom(SocketRefToSocketNum( s ), 
						(char*) buf, 
						len, 
						flags, 
						(struct sockaddr*)from, 
						(int*)fromlen);

	return returnValue;
}
										
short PGPSendTo(PGPSocketRef s, const void* buf, short len, ushort flags,
			PGPSocketAddress* to, short tolen)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue =  sendto(SocketRefToSocketNum( s ), 
						(char*) buf, 
						len, 
						flags, 
						(struct sockaddr*)to, 
						tolen);
	return returnValue;
}

PGPSocketRef PGPAccept(PGPSocketRef s, PGPSocketAddress* addr, short* addrlen)
{
	int returnValue = -1;

	returnValue =  accept(SocketRefToSocketNum( s ), 
						(struct sockaddr *) addr,
						(int*)addrlen);

	return SocketNumToSocketRef( returnValue );

}
					
PGPHostEntry* PGPGetHostByName(const char* name)
{
	PGPHostEntry* ReturnValue = 0;

	ReturnValue =  (PGPHostEntry*) gethostbyname( name );

	return ReturnValue;
}

PGPHostEntry* PGPGetHostByAddress(const char* addr, int len, int type)
{
	PGPHostEntry* ReturnValue = 0;

	ReturnValue =  (PGPHostEntry*) gethostbyaddr( addr, len, type);
	return ReturnValue;
}

ulong	PGPDottedToInternetAddress(const char* cp)
{
	return inet_addr(cp);
}

char* PGPInternetAddressToDottedString(PGPInternetAddress addr)
{
	struct in_addr* p_address;

	p_address = (struct in_addr*)&addr;

/*	return inet_ntoa((const struct in_addr)*p_address);*/
	return NULL;
}

short PGPGetPeerName(PGPSocketRef s, PGPSocketAddress *name, int* namelen)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue = getpeername(SocketRefToSocketNum( s ),
						(struct sockaddr *) name, namelen);

	return returnValue;
}
					
short PGPGetSocketName(PGPSocketRef s, PGPSocketAddress *name, int* namelen)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue = getsockname(SocketRefToSocketNum( s ),
						(struct sockaddr *) name, namelen);

	return returnValue;
}
					
short PGPGetSockOpt(PGPSocketRef s, int level, int optname, char* optval,
			int* optlen)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue = getsockopt(SocketRefToSocketNum( s ), level, optname,
						optval, optlen);

	return returnValue;
}
										
short PGPSetSockOpt(PGPSocketRef s, int level, int optname,
			const char* optval, int optlen)
{
	short returnValue = kPGPSocketsErrorUnknown;

	returnValue = setsockopt(SocketRefToSocketNum( s ), level, optname,
						optval, optlen);

	return returnValue;
}
					
short PGPShutdown(PGPSocketRef s, int how)
{
	return shutdown(SocketRefToSocketNum( s ), how);
}
					
PGPSocketsError	PGPGetLastSocketsError()
{
	return (PGPSocketsError) 0;
}

PGPSocketsError	PGPGetLastHostError()
{
	return (PGPSocketsError) 0;
}

long PGPHostToNetLong(long x)
{
	return htonl(x);
}

short PGPHostToNetShort(short x)
{
	return htons(x);
}

long PGPNetToHostLong(long x)
{
	return ntohl(x);
}

short PGPNetToHostShort(short x)
{
	return ntohs(x);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/