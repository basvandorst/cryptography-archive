/*
** Copyright (C)1997 Pretty Good Privacy, Inc.
** All rights reserved.
*/

#include <windows.h>
#include <winsock.h>


#include "PGPTypes.h"
#include "../../pub/PGPsockets.h"


PGPBoolean PGPSocketsInit()
{
	PGPBoolean ReturnValue = FALSE;
	WSADATA wsaData;
	int err;

	err = WSAStartup(MAKEWORD(1,1), &wsaData);

	if ( !err ) 
	{
		/* we could find a usable WinSock DLL. */
		ReturnValue = TRUE;
	}

    return ReturnValue; 
}

void PGPSocketsCleanup()
{
	WSACleanup();
}

PGPSocketRef PGPOpenSocket(int af, int type, int protocol)
{
	return(PGPSocketRef) socket(af, type, protocol);
}

short PGPCloseSocket(PGPSocketRef fd)
{
	return closesocket( (SOCKET)fd );
}

short PGPBindSocket(PGPSocketRef sockfd, const PGPSocketAddress* my_addr,int addrlen)
{
	return bind((SOCKET)sockfd, (const struct sockaddr*)my_addr, addrlen);
}

short PGPConnect(PGPSocketRef sockfd, const PGPSocketAddress* serv_addr, int addrlen)
{
	return connect((SOCKET)sockfd, (const struct sockaddr*)serv_addr, addrlen);
}

short PGPListen(PGPSocketRef s, int backlog)
{
	return listen((SOCKET)s, backlog);
}

short PGPSelect(int nfds, PGPSocketSet* readfds, PGPSocketSet* writefds, PGPSocketSet* exceptfds, const PGPSocketsTimeValue* timeout)
{
	return select(nfds, (fd_set*) readfds, (fd_set*) writefds, (fd_set*) exceptfds, (const timeval*) timeout);
}
					
short PGPSend(PGPSocketRef s, const void* msg, int len, unsigned int flags)
{
	return send((SOCKET)s, (const char*) msg, len, flags);
}

short	PGPReceive(PGPSocketRef s, void* buf, int len, unsigned int flags)
{
	return recv((SOCKET)s, (char*) buf, len, flags);
}

short PGPReceiveFrom(PGPSocketRef s, void* buf, int len, int flags, PGPSocketAddress* from, int* fromlen)
{
	return recvfrom((SOCKET)s, (char*) buf, len, flags, (struct sockaddr*)from, fromlen);
}
										
short PGPSendTo(PGPSocketRef s, void* buf, int len, int flags, PGPSocketAddress* to, int tolen)
{
	return sendto((SOCKET)s, (char*) buf, len, flags, (struct sockaddr*)to, tolen);
}

PGPSocketRef PGPAccept(PGPSocketRef s, PGPSocketAddress* addr, int* addrlen)
{
	return (PGPSocketRef) accept((SOCKET)s, (struct sockaddr FAR*) addr, addrlen);
}
					
PGPHostEntry* PGPGetHostByName(const char* name)
{
	return (PGPHostEntry*) gethostbyname( name );
}

PGPHostEntry* PGPGetHostByAddress(const char* addr, int len, int type)
{
	return (PGPHostEntry*) gethostbyaddr( addr, len, type);
}

ulong	PGPDottedToInternetAddress(const char* cp)
{
	return inet_addr(cp);
}

char* PGPInternetAddressToDottedString(PGPInternetAddress addr)
{
	struct in_addr* p_address;

	p_address = (struct in_addr*)&addr;

	return inet_ntoa(*p_address);
}

short PGPGetPeerName(PGPSocketRef s, PGPSocketAddress *name, int* namelen)
{
	return getpeername((SOCKET)s, (struct sockaddr FAR*) name, namelen);
}
					
short PGPGetSocketName(PGPSocketRef s, PGPSocketAddress *name, int* namelen)
{
	return getsockname((SOCKET)s, (struct sockaddr FAR*) name, namelen);
}
					
short PGPGetSockOpt(PGPSocketRef s, int level, int optname, char* optval, int* optlen)
{
	return getsockopt((SOCKET)s, level, optname, optval, optlen);
}
										
short PGPSetSockOpt(PGPSocketRef s, int level, int optname,const char* optval, int optlen)
{
	return setsockopt((SOCKET) s, level, optname, optval, optlen);
}
					
short PGPShutdown(PGPSocketRef s, int how)
{
	return shutdown((SOCKET) s, how);
}
					
PGPSocketsError	PGPGetLastSocketsError()
{
	return (PGPSocketsError) WSAGetLastError();
}

PGPSocketsError	PGPGetLastHostError()
{
	return (PGPSocketsError) WSAGetLastError();
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
