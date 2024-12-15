/*
** Copyright (C)1997 Pretty Good Privacy, Inc.
** All rights reserved.
*/

#include <windows.h>
#include <winsock.h>


#include "PGPTypes.h"
#include "../../pub/PGPsockets.h"

#define PGPSOCKETSMAGIC	0xdeadbeef

typedef struct _CALLBACKSTRUCT
{
	// magic number used to test for integrity of structure
	long magic;

	// actual socket for this call
	SOCKET s;

	// Callback Function for Async Calls
	PGPSocketsCallback function;

	// Callback Data for Async Calls
	void* data;

	// Signal Mutex
	HANDLE mutex;

	struct _CALLBACKSTRUCT* next;
	struct _CALLBACKSTRUCT* last;

}CALLBACKSTRUCT, *PCALLBACKSTRUCT;

CALLBACKSTRUCT g_defaultCallback = {PGPSOCKETSMAGIC, 0, NULL, NULL, NULL, NULL, NULL};
PCALLBACKSTRUCT g_callbackList = NULL;
CRITICAL_SECTION g_callbackListLock;
CRITICAL_SECTION g_CloseLock;
INT g_icallbackListLockRefCount	= 0;


DWORD WINAPI CallbackThreadRoutine( LPVOID lpvoid );
void AddToList( PCALLBACKSTRUCT pcbs);
void RemoveFromList( PCALLBACKSTRUCT pcbs );
BOOL FindInList( PCALLBACKSTRUCT pcbs );

short PGPSocketsInit()
{
	PGPError ReturnValue = kPGPSocketsErrorUnknown;
	WSADATA wsaData;
	int err;

	err = WSAStartup(MAKEWORD(1,1), &wsaData);

	if ( err == 0 ) 
	{
		/* we could find a usable WinSock DLL. */
		ReturnValue = 0;

		if (g_icallbackListLockRefCount == 0) 
		{ 
			g_icallbackListLockRefCount = 1;
			InitializeCriticalSection(&g_callbackListLock);
			InitializeCriticalSection(&g_CloseLock);
		}
		else ++g_icallbackListLockRefCount;
	}

    return ReturnValue; 
}

void PGPSocketsCleanup()
{
	--g_icallbackListLockRefCount;
	if (g_icallbackListLockRefCount == 0) {
		DeleteCriticalSection(&g_callbackListLock);
		DeleteCriticalSection(&g_CloseLock);
	}
	WSACleanup();
}

void PGPSetDefaultCallback(PGPSocketsCallback inCallback, void * inData)
{
	g_defaultCallback.function = inCallback;
	g_defaultCallback.data = inData;
}

short PGPSetCallback(PGPSocketRef s, PGPSocketsCallback inCallback,
						void * inData)
{
	PCALLBACKSTRUCT pcbs = NULL;
	short returnValue = 0;

	pcbs = (PCALLBACKSTRUCT)s;

	if(pcbs && !pcbs->mutex)
	{
		pcbs->function = inCallback;
		pcbs->data = inData;
	}
	else
	{
		returnValue = kPGPSocketsErrorUnknown;
	}

	return returnValue;
}

PGPSocketRef PGPOpenSocket(short af, short type, short protocol)
{
	PCALLBACKSTRUCT pcbs = NULL;

	pcbs = (PCALLBACKSTRUCT)malloc(sizeof(CALLBACKSTRUCT));

	if(pcbs)
	{
		memset(pcbs, 0x00, sizeof(CALLBACKSTRUCT));

		pcbs->magic = PGPSOCKETSMAGIC;
		pcbs->s = socket(af, type, protocol);
		pcbs->function = g_defaultCallback.function;
		pcbs->data = g_defaultCallback.data;
		pcbs->mutex = CreateMutex(NULL, FALSE, NULL);

		AddToList(pcbs);
	}

	return(PGPSocketRef)pcbs;
}

short PGPCloseSocket(PGPSocketRef s)
{
	PCALLBACKSTRUCT pcbs = NULL;
	short returnValue = kPGPSocketsErrorUnknown;

	EnterCriticalSection(&g_CloseLock);

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	pcbs->magic = 0;

	if(FindInList(pcbs))
	{
		returnValue = closesocket( pcbs->s );

		if(pcbs->mutex)
		{

			WaitForSingleObject(pcbs->mutex, INFINITE);
			CloseHandle( pcbs->mutex );
			pcbs->mutex = NULL;

		}

		RemoveFromList(pcbs);
	}

	LeaveCriticalSection(&g_CloseLock);

	return returnValue;
}

short PGPBindSocket(PGPSocketRef s, const PGPSocketAddress* my_addr,short addrlen)
{
	PCALLBACKSTRUCT pcbs = NULL;
	short returnValue = kPGPSocketsErrorUnknown;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	if(pcbs)
	{
		returnValue = bind(pcbs->s, (const struct sockaddr*)my_addr, addrlen);
	}

	return returnValue;
}

short PGPConnect(PGPSocketRef s, const PGPSocketAddress* serv_addr, short addrlen)
{
	DWORD threadId = 0;
	HANDLE handle;
	short returnValue = kPGPSocketsErrorUnknown;
	PCALLBACKSTRUCT pcbs = NULL;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	if( pcbs && pcbs->function )
	{
		if( pcbs->mutex )
		{
			WaitForSingleObject(pcbs->mutex, INFINITE);

			handle = CreateThread(	NULL, 
									0, 
									CallbackThreadRoutine, 
									(void*)pcbs, 
									0, 
									&threadId);
			if( handle )
			{
				returnValue =  connect(	pcbs->s, 
									(const struct sockaddr*)serv_addr, 
									addrlen);
			}

			ReleaseMutex( pcbs->mutex );
		}
	}
	else if( pcbs )
	{
		returnValue =  connect(	pcbs->s, 
								(const struct sockaddr*)serv_addr, 
								addrlen);
	}

	return returnValue;
}

short PGPListen(PGPSocketRef s, short backlog)
{
	PCALLBACKSTRUCT pcbs = NULL;
	short returnValue = kPGPSocketsErrorUnknown;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	if(pcbs)
	{
		returnValue = listen(pcbs->s, backlog);
	}

	return returnValue;
}

short PGPSelect(short nfds, PGPSocketSet* readfds, PGPSocketSet* writefds, PGPSocketSet* exceptfds, const PGPSocketsTimeValue* timeout)
{
	return select(nfds, (fd_set*) readfds, (fd_set*) writefds, (fd_set*) exceptfds, (const struct timeval*) timeout);
}
					
short PGPSend(PGPSocketRef s, const void* msg, short len, ushort flags)
{
	DWORD threadId = 0;
	HANDLE handle;
	short returnValue = kPGPSocketsErrorUnknown;
	PCALLBACKSTRUCT pcbs = NULL;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return SOCKET_ERROR;
	if (pcbs->magic != PGPSOCKETSMAGIC) return SOCKET_ERROR;

	if( pcbs && pcbs->function )
	{
		if( pcbs->mutex )
		{

			WaitForSingleObject(pcbs->mutex, INFINITE);

			handle = CreateThread(	NULL, 
									0, 
									CallbackThreadRoutine, 
									(void*)pcbs, 
									0, 
									&threadId);
			if( handle )
			{
				returnValue =  send(pcbs->s, (const char*) msg, len, flags);
			}

			ReleaseMutex( pcbs->mutex );
		}
	}
	else if( pcbs )
	{
		returnValue =  send(pcbs->s, (const char*) msg, len, flags);
	}

	return returnValue;
}

short PGPReceive(PGPSocketRef s, void* buf, short len, ushort flags)
{
	DWORD threadId = 0;
	HANDLE handle;
	short returnValue = kPGPSocketsErrorUnknown;
	PCALLBACKSTRUCT pcbs = NULL;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return SOCKET_ERROR;
	if (pcbs->magic != PGPSOCKETSMAGIC) return SOCKET_ERROR;

	if( pcbs && pcbs->function )
	{
		if( pcbs->mutex )
		{
			WaitForSingleObject(pcbs->mutex, INFINITE);

			handle = CreateThread(	NULL, 
									0, 
									CallbackThreadRoutine, 
									(void*)pcbs, 
									0, 
									&threadId);
			if( handle )
			{
				returnValue =  recv(pcbs->s, (char*) buf, len, flags);
			}

			ReleaseMutex( pcbs->mutex );
		}
	}
	else if( pcbs )
	{
		returnValue =  recv(pcbs->s, (char*) buf, len, flags);
	}

	return returnValue;
}

short PGPReceiveFrom(PGPSocketRef s, void* buf, short len, ushort flags, PGPSocketAddress* from, short* fromlen)
{
	DWORD threadId = 0;
	HANDLE handle;
	short returnValue = kPGPSocketsErrorUnknown;
	PCALLBACKSTRUCT pcbs = NULL;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return SOCKET_ERROR;
	if (pcbs->magic != PGPSOCKETSMAGIC) return SOCKET_ERROR;

	if( pcbs && pcbs->function )
	{
		if( pcbs->mutex )
		{
			WaitForSingleObject(pcbs->mutex, INFINITE);

			handle = CreateThread(	NULL, 
									0, 
									CallbackThreadRoutine, 
									(void*)pcbs, 
									0, 
									&threadId);
			if( handle )
			{
				returnValue =  recvfrom(pcbs->s, 
										(char*) buf, 
										len, 
										flags, 
										(struct sockaddr*)from, 
										(int*)fromlen);
			}

			ReleaseMutex( pcbs->mutex );
		}
	}
	else if( pcbs )
	{
		returnValue =  recvfrom(pcbs->s, 
								(char*) buf, 
								len, 
								flags, 
								(struct sockaddr*)from, 
								(int*)fromlen);
	}

	return returnValue;
}
										
short PGPSendTo(PGPSocketRef s, void* buf, short len, ushort flags, PGPSocketAddress* to, short tolen)
{
	DWORD threadId = 0;
	HANDLE handle;
	short returnValue = kPGPSocketsErrorUnknown;
	PCALLBACKSTRUCT pcbs = NULL;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return SOCKET_ERROR;
	if (pcbs->magic != PGPSOCKETSMAGIC) return SOCKET_ERROR;

	if( pcbs && pcbs->function )
	{
		if( pcbs->mutex )
		{
			WaitForSingleObject(pcbs->mutex, INFINITE);

			handle = CreateThread(	NULL, 
									0, 
									CallbackThreadRoutine, 
									(void*)pcbs, 
									0, 
									&threadId);
			if( handle )
			{
				returnValue =  sendto(	pcbs->s, 
										(char*) buf, 
										len, 
										flags, 
										(struct sockaddr*)to, 
										tolen);
			}

			ReleaseMutex( pcbs->mutex );
		}
	}
	else if( pcbs )
	{
		returnValue =  sendto(	pcbs->s, 
								(char*) buf, 
								len, 
								flags, 
								(struct sockaddr*)to, 
								tolen);
	}

	return returnValue;
}

PGPSocketRef PGPAccept(PGPSocketRef s, PGPSocketAddress* addr, short* addrlen)
{
	DWORD threadId = 0;
	HANDLE handle;
	int returnValue = INVALID_SOCKET;
	PCALLBACKSTRUCT pcbs = NULL;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return NULL;
	if (pcbs->magic != PGPSOCKETSMAGIC) return NULL;

	if( pcbs && pcbs->function )
	{
		if( pcbs->mutex )
		{
			WaitForSingleObject(pcbs->mutex, INFINITE);

			handle = CreateThread(	NULL, 
									0, 
									CallbackThreadRoutine, 
									(void*)pcbs, 
									0, 
									&threadId);
			if( handle )
			{
				returnValue =  accept(	pcbs->s, 
										(struct sockaddr FAR*) addr,
										(int*)addrlen);
			}

			ReleaseMutex( pcbs->mutex );
		}
	}
	else if( pcbs )
	{
		returnValue =  accept(	pcbs->s, 
								(struct sockaddr FAR*) addr, 
								(int*)addrlen);
	}

	pcbs = kPGPInvalidSocket;

	if(INVALID_SOCKET != returnValue)
	{
		pcbs = (PCALLBACKSTRUCT)malloc(sizeof(CALLBACKSTRUCT));

		if(pcbs)
		{
			memset(pcbs, 0x00, sizeof(CALLBACKSTRUCT));

			pcbs->magic = PGPSOCKETSMAGIC;
			pcbs->s = returnValue;
			pcbs->function = g_defaultCallback.function;
			pcbs->data = g_defaultCallback.data;
		}
	}

	return (PGPSocketRef)pcbs;

}
					
PGPHostEntry* PGPGetHostByName(const char* name)
{
	DWORD threadId = 0;
	HANDLE handle;
	PGPHostEntry* ReturnValue = 0;
	CALLBACKSTRUCT localCallback = g_defaultCallback;

	if( localCallback.function )
	{
		localCallback.mutex = CreateMutex(NULL, TRUE, NULL);

		if( localCallback.mutex )
		{

			handle = CreateThread(	NULL, 
									0, 
									CallbackThreadRoutine, 
									(void*)&localCallback, 
									0, 
									&threadId);
			if( handle )
			{
				ReturnValue =  (PGPHostEntry*) gethostbyname( name );
			}

			ReleaseMutex( localCallback.mutex );
			CloseHandle( localCallback.mutex );
		}
	}
	else
	{
		ReturnValue =  (PGPHostEntry*) gethostbyname( name );
	}

	return ReturnValue;
}

PGPHostEntry* PGPGetHostByAddress(const char* addr, int len, int type)
{
	DWORD threadId = 0;
	HANDLE handle;
	PGPHostEntry* ReturnValue = 0;
	CALLBACKSTRUCT localCallback = g_defaultCallback;

	if( localCallback.function )
	{
		localCallback.mutex = CreateMutex(NULL, TRUE, NULL);

		if( localCallback.mutex )
		{

			handle = CreateThread(	NULL, 
									0, 
									CallbackThreadRoutine, 
									(void*)&localCallback, 
									0, 
									&threadId);
			if( handle )
			{
				ReturnValue =  (PGPHostEntry*) gethostbyaddr( addr, len, type);
			}

			ReleaseMutex( localCallback.mutex );
			CloseHandle( localCallback.mutex );
		}
	}
	else
	{
		ReturnValue =  (PGPHostEntry*) gethostbyaddr( addr, len, type);
	}

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

	return inet_ntoa(*p_address);
}

short PGPGetPeerName(PGPSocketRef s, PGPSocketAddress *name, int* namelen)
{
	PCALLBACKSTRUCT pcbs = NULL;
	short returnValue = kPGPSocketsErrorUnknown;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	if(pcbs)
	{
		returnValue = getpeername(pcbs->s, (struct sockaddr FAR*) name, namelen);
	}

	return returnValue;
}
					
short PGPGetSocketName(PGPSocketRef s, PGPSocketAddress *name, int* namelen)
{
	PCALLBACKSTRUCT pcbs = NULL;
	short returnValue = kPGPSocketsErrorUnknown;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	if(pcbs)
	{
		returnValue = getsockname(pcbs->s, (struct sockaddr FAR*) name, namelen);
	}

	return returnValue;
}
					
short PGPGetSockOpt(PGPSocketRef s, int level, int optname, char* optval, int* optlen)
{
	PCALLBACKSTRUCT pcbs = NULL;
	short returnValue = kPGPSocketsErrorUnknown;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	if(pcbs)
	{
		returnValue = getsockopt(pcbs->s, level, optname, optval, optlen);
	}

	return returnValue;
}
										
short PGPSetSockOpt(PGPSocketRef s, int level, int optname,const char* optval, int optlen)
{
	PCALLBACKSTRUCT pcbs = NULL;
	short returnValue = kPGPSocketsErrorUnknown;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	if(pcbs)
	{
		returnValue = setsockopt(pcbs->s, level, optname, optval, optlen);
	}

	return returnValue;
}
					
short PGPShutdown(PGPSocketRef s, int how)
{
	PCALLBACKSTRUCT pcbs = NULL;
	short returnValue = kPGPSocketsErrorUnknown;

	pcbs = (PCALLBACKSTRUCT)s;
	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	if(pcbs)
	{
		returnValue = shutdown(pcbs->s, how);
	}

	return returnValue;
}
					
PGPSocketsError	PGPGetLastSocketsError()
{
	int error;

	error = WSAGetLastError();

	if(error == WSAENOTSOCK)
	{
		error = kPGPSocketsErrorUserCanceled;
	}
	return (PGPSocketsError) error;
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

DWORD WINAPI CallbackThreadRoutine( LPVOID lpvoid)
{
	PCALLBACKSTRUCT pcbs = (PCALLBACKSTRUCT)lpvoid;
	DWORD waitResult = 0;
	HANDLE blockingMutex = NULL;

	if (!pcbs) return kPGPSocketsErrorNotSock;
	if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

	blockingMutex = pcbs->mutex;

	while( 1 )
	{
		waitResult = WaitForSingleObject(blockingMutex, 500);

		if (!pcbs) return kPGPSocketsErrorNotSock;
		if (pcbs->magic != PGPSOCKETSMAGIC) return kPGPSocketsErrorNotSock;

		if ((waitResult == WAIT_TIMEOUT) && (pcbs->function))
		{
			pcbs->function( pcbs->data );
		}
		else
		{
			return 0;
		}
	}
}

void AddToList( PCALLBACKSTRUCT pcbs)
{
	EnterCriticalSection(&g_callbackListLock);

	pcbs->next = g_callbackList;

	if(g_callbackList)
	{
		g_callbackList->last = pcbs;
	}

	g_callbackList	= pcbs;

	LeaveCriticalSection(&g_callbackListLock);
}

void RemoveFromList( PCALLBACKSTRUCT pcbs )
{
	PCALLBACKSTRUCT iter = NULL;

	EnterCriticalSection(&g_callbackListLock);

	iter = g_callbackList;

	// Go through the list of sockets
	while(iter)
	{
		// is this the socket we are looking for
		if( iter == pcbs )
		{

			if(iter->last)
			{
				iter->last->next = iter->next;
			}
			else // new first item
			{
				g_callbackList = iter->next;

				if(g_callbackList)
				{
					g_callbackList->last = NULL;
				}
			}

			if(iter->next)
			{
				iter->next->last = iter->last;
			}
			
			
			iter->s = 0;
			iter->function = NULL;
			iter->data = NULL;

			free(iter);

			break;
		}

		iter = iter->next;
	}

	LeaveCriticalSection(&g_callbackListLock);
}

BOOL FindInList( PCALLBACKSTRUCT pcbs )
{
	PCALLBACKSTRUCT iter = NULL;

	EnterCriticalSection(&g_callbackListLock);

	iter = g_callbackList;

	// Go through the list of sockets
	while(iter)
	{
		// is this the socket we are looking for
		if( iter == pcbs )
		{
			LeaveCriticalSection(&g_callbackListLock);
			return TRUE;
		}
		iter = iter->next;
	}

	LeaveCriticalSection(&g_callbackListLock);

	return FALSE;
}






