/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: pgpSockets.c,v 1.18.10.1 1998/11/12 03:22:21 heller Exp $
____________________________________________________________________________*/

#if PGP_WIN32
# include <windows.h>
# include <winsock.h>
# include "PGPsdkNetworkLibDLLMain.h"
#else
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <errno.h>
# include <unistd.h>
# include <stdlib.h>
#endif

#include "pgpConfig.h"
#include "pgpPubTypes.h"
#include "pgpMem.h"
#include "pgpEncode.h"
#include "pgpBinaryTree.h"
#include "pgpRMWOLock.h"

#include "pgpSockets.h"

/* Unix specific things */
#if PGP_UNIX
# define FAR
# define SOCKET_ERROR -1
# define closesocket(x) close((x))
# define ioctlsocket(a,b,c) ioctl((a),(b),(c))
typedef int		SOCKET;
#if PGP_UNIX_LINUX
int gethostname (char * name, size_t namelen);
#else
int gethostname (char * name, int namelen);
#endif /* PGP_UNIX_LINUX */
#endif

/* Windows specific things */
#if PGP_WIN32
 static PGPInt32		sIdleEventHandlerIndex = 0;
 static PGPInt32		sIdleEventHandlerDataIndex = 0;
 BOOL WINAPI SocketsBlockingHook();
 static PGPRMWOLock		sIdleEventHandlerLock;
#endif

PGPSocketAddressInternet kPGPAddressAny = {
	kPGPAddressFamilyInternet,
	0,
	{{{kPGPInternetAddressAny}}},
	{0,0,0,0,0,0,0,0}
};
												
static PGPInt32				sNumberOfInits = 0;
static PGPMemoryMgrRef		sMemoryMgr = kInvalidPGPMemoryMgrRef;

static PGPBinaryTreeRef		sErrorMap = kInvalidPGPBinaryTreeRef;
static PGPBinaryTreeRef		sTLSMap = kInvalidPGPBinaryTreeRef;

static PGPRMWOLock			sTLSMapLock;

void		InitErrorMap(void);
PGPError	MapPGPSocketsError(PGPError inErr);
PGPInt32	PGPSocketsTLSReceive(void * inData, void * outBuffer,
					 PGPInt32 inBufferSize);
PGPInt32	PGPSocketsTLSSend(void * inData, const void * inBuffer,
				  PGPInt32 inBufferLength);


PGPError
PGPSocketsInit()
{
	PGPError	err = kPGPError_NoErr;

#if PGP_WIN32
	WSADATA	wsaData;
	EnterCriticalSection(&gSocketsInitMutex);
#endif
	
	if (sNumberOfInits == 0) {
#if PGP_WIN32	
		InitializePGPRMWOLock(&sIdleEventHandlerLock);
#endif	
		InitializePGPRMWOLock(&sTLSMapLock);
		err = PGPNewMemoryMgr(0, &sMemoryMgr);
		if (err != kPGPError_NoErr) {
			goto done;
		}
		err = PGPNewBinaryTree(sMemoryMgr, &sErrorMap);
		if (err != kPGPError_NoErr) {
			goto done;
		}
		err = PGPNewBinaryTree(sMemoryMgr, &sTLSMap);
		if (err != kPGPError_NoErr) {
			goto done;
		}
	
#if PGP_WIN32	
		(void) PGPThreadKeyCreate(&sIdleEventHandlerIndex, NULL);
		if (PGPThreadSetSpecific(sIdleEventHandlerIndex, NULL)) {
			err = kPGPError_UnknownError;
			goto done;
		}
		(void) PGPThreadKeyCreate(&sIdleEventHandlerDataIndex, NULL);
		if (PGPThreadSetSpecific(sIdleEventHandlerDataIndex, NULL)) {
			err = kPGPError_UnknownError;
			goto done;
		}
#endif
	
	InitErrorMap();
	
#if PGP_WIN32	
	err = WSAStartup(MAKEWORD(1,1), &wsaData);
	if (err != 0) {
		err = MapPGPSocketsError(err);
		goto done;
	}
#endif
	
	}
	sNumberOfInits++;
	
 done:
#if PGP_WIN32
	LeaveCriticalSection(&gSocketsInitMutex);
#endif	
	return err;
}



void
PGPSocketsCleanup()
{
#if PGP_WIN32
	EnterCriticalSection(&gSocketsInitMutex);
#endif
	
	sNumberOfInits--;
	if (sNumberOfInits == 0) {
#if PGP_WIN32
		WSACleanup();
#endif	
		if (PGPBinaryTreeRefIsValid(sTLSMap)) {
			PGPRMWOLockStartWriting(&sTLSMapLock);
			PGPDisposeBinaryTree(sTLSMap);
			sTLSMap = kInvalidPGPBinaryTreeRef;
			PGPRMWOLockStopWriting(&sTLSMapLock);
		}
		if (PGPBinaryTreeRefIsValid(sErrorMap)) {
			PGPDisposeBinaryTree(sErrorMap);
			sErrorMap = kInvalidPGPBinaryTreeRef;
		}
		if (PGPMemoryMgrRefIsValid(sMemoryMgr)) {
			PGPFreeMemoryMgr(sMemoryMgr);
			sMemoryMgr = kInvalidPGPMemoryMgrRef;
		}
	
#if PGP_WIN32	
		PGPThreadKeyDelete(sIdleEventHandlerIndex);
		sIdleEventHandlerIndex = 0;
		PGPThreadKeyDelete(sIdleEventHandlerDataIndex);
		sIdleEventHandlerDataIndex = 0;
		DeletePGPRMWOLock(&sIdleEventHandlerLock);
#endif
	
		DeletePGPRMWOLock(&sTLSMapLock);
	} else if (sNumberOfInits < 0) {
		sNumberOfInits = 0;
	}

#if PGP_WIN32
	LeaveCriticalSection(&gSocketsInitMutex);
#endif	
}



PGPSocketRef
PGPOpenSocket(
	PGPInt32	inAddressFamily,
	PGPInt32	inSocketType,
	PGPInt32	inSocketProtocol)
{
	return (PGPSocketRef) socket(inAddressFamily, inSocketType,
								 inSocketProtocol);
}




PGPInt32
PGPCloseSocket(
	PGPSocketRef	inSocketRef)
{
	PGPUserValue	unused;
	
	PGPRMWOLockStartWriting(&sTLSMapLock);
	PGPDisposeNode(sTLSMap, (PGPInt32) inSocketRef, &unused);
	PGPRMWOLockStopWriting(&sTLSMapLock);
	return closesocket((SOCKET) inSocketRef);
}



PGPInt32
PGPBindSocket(
	PGPSocketRef				inSocketRef,
	const PGPSocketAddress *	inAddress,
	PGPInt32					inAddressLength)
{
	return bind((SOCKET) inSocketRef,
				(struct sockaddr *) inAddress,
				inAddressLength);
}



PGPInt32
PGPConnect(
	PGPSocketRef				inSocketRef,
	const PGPSocketAddress *	inServerAddress,
	PGPInt32					inAddressLength)
{
	return connect((SOCKET) inSocketRef,
				   (struct sockaddr *) inServerAddress,
				   inAddressLength);
}



PGPInt32
PGPSend(
	PGPSocketRef	inSocketRef,
	const void *	inBuffer,
	PGPInt32		inBufferLength,
	PGPInt32		inFlags)
{
	PGPInt32			result = kPGPSockets_Error;
	PGPError			pgpError;
	PGPtlsSessionRef	tlsSession;
	
	PGPRMWOLockStartReading(&sTLSMapLock);
	pgpError = PGPFindNode(sTLSMap, (PGPInt32) inSocketRef,
						   (PGPUserValue*)&tlsSession);
	PGPRMWOLockStopReading(&sTLSMapLock);
	if (IsntPGPError(pgpError)) {
		pgpError = PGPtlsSend(tlsSession,
							  inBuffer,
							  inBufferLength);
		if (IsntPGPError(pgpError)) {
			result = inBufferLength;
		} else if (pgpError != kPGPError_TLSUnexpectedClose) {
#if PGP_WIN32		
			WSASetLastError(pgpError);
#else
			errno = pgpError;
#endif		
		}
	} else {
		result = send((SOCKET) inSocketRef,
					  (char*)inBuffer,
					  inBufferLength,
					  inFlags);
	}
	
	return result;
}



PGPInt32
PGPWrite(
	PGPSocketRef	inSocketRef,
	const void *	inBuffer,
	PGPInt32		inBufferLength)
{
	return PGPSend(inSocketRef, inBuffer, inBufferLength, kPGPSendFlagNone);
}



PGPInt32
PGPSendTo(
	PGPSocketRef		inSocketRef,
	const void *		inBuffer,
	PGPInt32			inBufferLength,
	PGPInt32			inFlags,
	PGPSocketAddress *	inAddress,
	PGPInt32			inAddressLength)
{
	return sendto((SOCKET) inSocketRef,
				  (char *) inBuffer,
				  inBufferLength,
				  inFlags,
				  (struct sockaddr *) inAddress,
				  inAddressLength);
}



PGPInt32
PGPReceive(
	PGPSocketRef	inSocketRef,
	void *			outBuffer,
	PGPInt32		inBufferSize,
	PGPInt32		inFlags)
{
	PGPInt32			result = kPGPSockets_Error;
	PGPError			pgpError;
	PGPtlsSessionRef	tlsSession;
	PGPSize				bufferSize = inBufferSize;
	
	PGPRMWOLockStartReading(&sTLSMapLock);
	pgpError = PGPFindNode(sTLSMap, (PGPInt32) inSocketRef,
						   (PGPUserValue*) &tlsSession);
	PGPRMWOLockStopReading(&sTLSMapLock);
	if (IsntPGPError(pgpError)) {
		pgpError = PGPtlsReceive(tlsSession,
								 outBuffer,
								 &bufferSize);
		if (IsntPGPError(pgpError)) {
			result = bufferSize;
		} else if (pgpError != kPGPError_TLSUnexpectedClose) {
#if PGP_WIN32		
			WSASetLastError(pgpError);
#else
			errno = pgpError;
#endif		
		}
	} else {
		result = recv((SOCKET) inSocketRef,
					  (char *) outBuffer,
					  inBufferSize,
					  inFlags);
	}
	
	return result;
}



PGPInt32
PGPRead(
	PGPSocketRef	inSocketRef,
	void *			outBuffer,
	PGPInt32		inBufferSize)
{
	return PGPReceive(inSocketRef,
					  outBuffer,
					  inBufferSize,
					  kPGPReceiveFlagNone);
}



PGPInt32
PGPReceiveFrom(
	PGPSocketRef		inSocketRef,
	void *				outBuffer,
	PGPInt32			inBufferSize,
	PGPInt32			inFlags,
	PGPSocketAddress *	outAddress,
	PGPInt32 *			ioAddressLength)
{
	return recvfrom(	(SOCKET) inSocketRef,
						(char *) outBuffer,
						inBufferSize,
						inFlags,
						(struct sockaddr *) outAddress,
						ioAddressLength);
}



PGPInt32
PGPListen(
	PGPSocketRef	inSocketRef,
	PGPInt32		inMaxBacklog)
{
	return listen((SOCKET) inSocketRef, inMaxBacklog);
}



PGPSocketRef
PGPAccept(
	PGPSocketRef		inSocketRef,
	PGPSocketAddress *	outAddress,
	PGPInt32 *			ioAddressLength)
{
	return (PGPSocketRef) accept(	(SOCKET) inSocketRef,
									(struct sockaddr FAR *) outAddress,
									ioAddressLength);
}


PGPInt32
PGPSelect(
	PGPInt32			inNumSetCount,
	PGPSocketSet *		ioReadSet,
	PGPSocketSet *		ioWriteSet,
	PGPSocketSet *		ioErrorSet,
	const PGPSocketsTimeValue *	inTimeout)
{
	return select(inNumSetCount,
				  (fd_set *) ioReadSet,
				  (fd_set *) ioWriteSet,
				  (fd_set *) ioErrorSet,
				  (struct timeval *) inTimeout);
}


#if PGP_WIN32
PGPInt32
__PGPSocketsIsSet(
	PGPSocketRef	inSocketRef,
	PGPSocketSet *	inSocketSet)
{
	PGPInt16 	result = 0;
	PGPUInt16	i;
	
	for (i = 0; i < inSocketSet->fd_count; i++) {
		if (inSocketSet->fd_array[i] == (SOCKET) inSocketRef) {
			result = 1;
			break;
		}
	}
	
	return result;
}
#endif


PGPHostEntry *
PGPGetHostByName(
	const char * inName)
{
	struct hostent *hjet = 0;
	
	hjet = gethostbyname(inName);
	
	return (PGPHostEntry *) hjet;
}



PGPHostEntry *
PGPGetHostByAddress(
	const char *	inAddress,
	PGPInt32		inLength,
	PGPInt32		inType)
{
	return (PGPHostEntry *) gethostbyaddr(inAddress, inLength, inType);
}



PGPInt32
PGPGetHostName(
	char *		outName,
	PGPInt32	inNameLength)
{
	return gethostname(outName, inNameLength);
}



PGPProtocolEntry *
PGPGetProtocolByName(
	const char *	inName)
{
	return (PGPProtocolEntry *) getprotobyname(inName);
}



PGPProtocolEntry *
PGPGetProtocolByNumber(
	PGPInt32	inNumber)
{
	return (PGPProtocolEntry *) getprotobynumber(inNumber);
}



PGPServiceEntry *
PGPGetServiceByName(
	const char *	inName,
	const char *	inProtocol)
{
	return (PGPServiceEntry *) getservbyname(inName, inProtocol);
}



PGPServiceEntry *
PGPGetServiceByPort(
	PGPInt32		inPort,
	const char *	inProtocol)
{
	return (PGPServiceEntry *) getservbyport(inPort, inProtocol);
}


PGPInt32
PGPGetSocketName(
	PGPSocketRef		inSocketRef,
	PGPSocketAddress *	outName,
	PGPInt32 *			ioNameLength)
{
	return getsockname(	(SOCKET) inSocketRef,
						(struct sockaddr *) outName,
						ioNameLength);
}

PGPInt32
PGPGetPeerName(
	PGPSocketRef		inSocketRef,
	PGPSocketAddress *	outName,
	PGPInt32 *			ioNameLength)
{
	return getpeername(	(SOCKET) inSocketRef,
						(struct sockaddr *) outName,
						ioNameLength);
}

PGPUInt32
PGPDottedToInternetAddress(
	const char *	inAddress)
{
	return inet_addr(inAddress);
}

char *
PGPInternetAddressToDottedString(
	PGPInternetAddress	inAddress)
{
	return inet_ntoa(*((struct in_addr *) &inAddress));
}

PGPInt32
PGPIOControlSocket(
	PGPSocketRef	inSocketRef,
	PGPInt32		inCommand,
	PGPUInt32 *		ioParam)
{
	return ioctlsocket((SOCKET) inSocketRef, 
					   inCommand, 
					   (unsigned long*)ioParam);
}

PGPInt32
PGPGetSocketOptions(
	PGPSocketRef	inSocketRef,
	PGPInt32		inLevel,
	PGPInt32		inOptionName,
	char *			outOptionValue,
	PGPInt32 *		ioOptionLength)
{
	return getsockopt(	(SOCKET) inSocketRef,
						inLevel,
						inOptionName,
						outOptionValue,
						ioOptionLength);
}

PGPInt32
PGPSetSocketOptions(
	PGPSocketRef	inSocketRef,
	PGPInt32		inLevel,
	PGPInt32		inOptionName,
	const char *	inOptionValue,
	PGPInt32		inOptionLength)
{
	return setsockopt((SOCKET) inSocketRef,
					  inLevel,
					  inOptionName,
					  inOptionValue,
					  inOptionLength);
}


PGPError
PGPSocketsEstablishTLSSession(
	PGPSocketRef	inSocketRef,
	PGPtlsSessionRef	inTLSSession)
{
	PGPError	err;
	int			optval;
	int			optlen = sizeof(optval);
	
	if (getsockopt((SOCKET) inSocketRef, 
				   SOL_SOCKET, 
				   SO_TYPE, 
				   (char *) &optval,
				   &optlen) == SOCKET_ERROR) {
		err = PGPGetLastSocketsError();
	} else if (optval != SOCK_STREAM) {
		err = kPGPError_SocketsOperationNotSupported;
	} else {
		err = PGPtlsSetReceiveCallback(	inTLSSession,
										PGPSocketsTLSReceive,
										inSocketRef);
		if (err == kPGPError_NoErr) {
			err = PGPtlsSetSendCallback(inTLSSession,
										PGPSocketsTLSSend,
										inSocketRef);
			if (err == kPGPError_NoErr) {
				PGPSocketAddressInternet address;
				int  addressLen = sizeof(address);
				
				err = getpeername((SOCKET) inSocketRef,
								  (struct sockaddr *) &address,
								  &addressLen);
				if (err == SOCKET_ERROR) {
					err = PGPGetLastSocketsError();
				} else {
					err = PGPtlsSetRemoteUniqueID(inTLSSession,
												  address.sin_addr.s_addr);
					if (err == kPGPError_NoErr) {
						err = PGPtlsHandshake(inTLSSession);
						if (err == kPGPError_NoErr) {
							PGPRMWOLockStartWriting(&sTLSMapLock);
							err = PGPAddNode(sTLSMap,
											 (PGPInt32) inSocketRef,
											 inTLSSession);
							PGPRMWOLockStopWriting(&sTLSMapLock);
						} else if (err == kPGPError_TLSUnexpectedClose) {
							err = PGPGetLastSocketsError();
						}
					}
				}
			}
		}
	}
	
	return err;
}



PGPInt32
PGPSocketsTLSReceive(
	void *	inData,
	void *	outBuffer,
	PGPInt32	inBufferSize)
{
	return recv((SOCKET) inData,
				(char *) outBuffer,
				inBufferSize,
				kPGPReceiveFlagNone);
}



PGPInt32
PGPSocketsTLSSend(
	void *		inData,
	const void *	inBuffer,
	PGPInt32		inBufferLength)
{
	return send((SOCKET) inData,
				(const char *) inBuffer,
				inBufferLength,
				kPGPSendFlagNone);
}



PGPError
PGPGetLastSocketsError(void)
{
#if PGP_WIN32	
	return MapPGPSocketsError(WSAGetLastError());
#else
	return MapPGPSocketsError(errno);
#endif
}


#if PGP_WIN32		
PGPError
PGPSetSocketsIdleEventHandler(
	PGPEventHandlerProcPtr	inCallback,
	PGPUserValue		inUserData)
{
	PGPRMWOLockStartWriting(&sIdleEventHandlerLock);
	if (!PGPThreadSetSpecific(sIdleEventHandlerIndex, inCallback)) {
		if (!PGPThreadSetSpecific(sIdleEventHandlerDataIndex, inUserData)) {
			if (inCallback != NULL) {
				WSASetBlockingHook(SocketsBlockingHook);
			} else {
				WSAUnhookBlockingHook();
			}
		} else {
			(void) PGPThreadSetSpecific(sIdleEventHandlerIndex, NULL);
		}
	}
	PGPRMWOLockStopWriting(&sIdleEventHandlerLock);

	return kPGPError_NoErr;
}

PGPError
PGPGetSocketsIdleEventHandler(
	PGPEventHandlerProcPtr *	outCallback,
	PGPUserValue *		outUserData)
{
	PGPRMWOLockStartReading(&sIdleEventHandlerLock);
	(void) PGPThreadGetSpecific(sIdleEventHandlerIndex, (void *) outCallback);
	(void) PGPThreadGetSpecific(sIdleEventHandlerDataIndex,
				outUserData);
	PGPRMWOLockStopReading(&sIdleEventHandlerLock);
	
	return kPGPError_NoErr;
}

struct SThreadContext {
	PGPEventHandlerProcPtr	callback;
	PGPUserValue			callbackData;
};

PGPError
PGPSocketsCreateThreadStorage(
	PGPSocketsThreadStorageRef *	outPreviousStorage)
{
	PGPError				err = kPGPError_NoErr;
	struct SThreadContext *	context = NULL;

	PGPValidatePtr(outPreviousStorage);
	*outPreviousStorage = NULL;

	context = malloc(sizeof(struct SThreadContext));
	if (context != NULL) {
		err = PGPGetSocketsIdleEventHandler(&context->callback,
					&context->callbackData);
	} else {
		err = kPGPError_OutOfMemory;
	}

	if (IsPGPError(err) && (context != NULL)) {
		free(context);
	} else {
		*outPreviousStorage = (PGPSocketsThreadStorageRef) context;
	}
	
	return err;
}


PGPError
PGPSocketsDisposeThreadStorage(
	PGPSocketsThreadStorageRef	inPreviousStorage)
{
	PGPError	err = kPGPError_NoErr;

	if (inPreviousStorage != NULL) {
		err = PGPSetSocketsIdleEventHandler(
				((struct SThreadContext *) inPreviousStorage)->callback,
				((struct SThreadContext *) inPreviousStorage)->callbackData);
		free(inPreviousStorage);
	}

	return err;
}

#else /* !PGP_Win32 */

PGPError
PGPSetSocketsIdleEventHandler(
	PGPEventHandlerProcPtr	inCallback,
	PGPUserValue		inUserData)
{
	(void) inCallback;
	(void) inUserData;
	
	return kPGPError_NoErr;
}

PGPError
PGPGetSocketsIdleEventHandler(
	PGPEventHandlerProcPtr *	outCallback,
	PGPUserValue *		outUserData)
{
	*outUserData = NULL;
	*outCallback = NULL;
	return kPGPError_NoErr;
}

PGPError
PGPSocketsCreateThreadStorage(
	PGPSocketsThreadStorageRef *	outPreviousStorage)
{
	PGPValidatePtr(outPreviousStorage);
	*outPreviousStorage = NULL;
	return kPGPError_NoErr;
}


PGPError
PGPSocketsDisposeThreadStorage(
	PGPSocketsThreadStorageRef	inPreviousStorage)
{
	(void) inPreviousStorage;
	return kPGPError_NoErr;
}

#endif /* PGP_WIN32 */


PGPError
MapPGPSocketsError(
	PGPError	inErr)
{
	PGPError	result = kPGPError_UnknownError;
	PGPError	pgpError;
	PGPError	tempError;
	
	if ((inErr > kPGPError_FirstError) && (inErr < kPGPError_Last)) {
		result = inErr;
	} else {
		pgpError = PGPFindNode(sErrorMap, inErr, (PGPUserValue *) &tempError);
		if (pgpError == kPGPError_NoErr) {
			result = tempError;
		}
	}
	
	return result;
}

void
InitErrorMap()
{
	PGPAddNode(sErrorMap, 0, (PGPUserValue) kPGPError_NoErr);
	
#if PGP_WIN32
	PGPAddNode(sErrorMap, WSANOTINITIALISED,
		   (PGPUserValue) kPGPError_SocketsNotInitialized);
	PGPAddNode(sErrorMap, WSAENETDOWN,
		   (PGPUserValue) kPGPError_SocketsNetworkDown);
	PGPAddNode(sErrorMap, WSAEFAULT,
		   (PGPUserValue) kPGPError_BadParams);
	PGPAddNode(sErrorMap, WSAEINPROGRESS,
		   (PGPUserValue) kPGPError_SocketsInProgress);
	PGPAddNode(sErrorMap, WSAEINVAL,
		   (PGPUserValue) kPGPError_SocketsNotBound);
	PGPAddNode(sErrorMap, WSAENOBUFS,
		   (PGPUserValue) kPGPError_OutOfMemory);
	PGPAddNode(sErrorMap, WSAENOTSOCK,
		   (PGPUserValue) kPGPError_SocketsNotASocket);
	PGPAddNode(sErrorMap, WSAEOPNOTSUPP,
		   (PGPUserValue) kPGPError_SocketsOperationNotSupported);
	PGPAddNode(sErrorMap, WSAEADDRINUSE,
		   (PGPUserValue) kPGPError_SocketsAddressInUse);
	PGPAddNode(sErrorMap, WSAEAFNOSUPPORT,
		   (PGPUserValue) kPGPError_SocketsAddressFamilyNotSupported);
	PGPAddNode(sErrorMap, WSAEADDRNOTAVAIL,
		   (PGPUserValue) kPGPError_SocketsAddressNotAvailable);
	PGPAddNode(sErrorMap, WSAECONNREFUSED,
		   (PGPUserValue) kPGPError_SocketsNotConnected);
	PGPAddNode(sErrorMap, WSAEDESTADDRREQ,
		   (PGPUserValue) kPGPError_BadParams);
	PGPAddNode(sErrorMap, WSAEISCONN,
		   (PGPUserValue) kPGPError_SocketsAlreadyConnected);
	PGPAddNode(sErrorMap, WSAENETUNREACH,
		   (PGPUserValue) kPGPError_SocketsNetworkDown);
	PGPAddNode(sErrorMap, WSAETIMEDOUT,
		   (PGPUserValue) kPGPError_SocketsTimedOut);
	PGPAddNode(sErrorMap, WSAENOPROTOOPT,
		   (PGPUserValue) kPGPError_BadParams);
	PGPAddNode(sErrorMap, WSAEMSGSIZE,
		   (PGPUserValue) kPGPError_SocketsBufferOverflow);
	PGPAddNode(sErrorMap, WSAECONNABORTED,
		   (PGPUserValue) kPGPError_SocketsNotConnected);
	PGPAddNode(sErrorMap, WSAECONNRESET,
		   (PGPUserValue) kPGPError_SocketsNotConnected);
	PGPAddNode(sErrorMap, WSAENETRESET,
		   (PGPUserValue) kPGPError_SocketsNotConnected);
	PGPAddNode(sErrorMap, WSAEPROTONOSUPPORT,
		   (PGPUserValue) kPGPError_SocketsProtocolNotSupported);
	PGPAddNode(sErrorMap, WSAEPROTOTYPE,
		   (PGPUserValue) kPGPError_SocketsProtocolNotSupported);
	PGPAddNode(sErrorMap, WSAESOCKTNOSUPPORT,
		   (PGPUserValue) kPGPError_BadParams);
	PGPAddNode(sErrorMap, WSASYSNOTREADY,
		   (PGPUserValue) kPGPError_SocketsNetworkDown);
	PGPAddNode(sErrorMap, WSAVERNOTSUPPORTED,
		   (PGPUserValue) kPGPError_BadParams);

	PGPAddNode(sErrorMap, WSAHOST_NOT_FOUND,
		   (PGPUserValue) kPGPError_SocketsHostNotFound);
	PGPAddNode(sErrorMap, WSATRY_AGAIN,
		   (PGPUserValue) kPGPError_SocketsDomainServerError);
	PGPAddNode(sErrorMap, WSANO_RECOVERY,
		   (PGPUserValue) kPGPError_SocketsDomainServerError);
	PGPAddNode(sErrorMap, WSANO_DATA,
		   (PGPUserValue) kPGPError_SocketsDomainServerError);
#endif
	
#if PGP_UNIX
	PGPAddNode(sErrorMap, ENETDOWN,
		   (PGPUserValue) kPGPError_SocketsNetworkDown);
	PGPAddNode(sErrorMap, EFAULT,
		   (PGPUserValue) kPGPError_BadParams);
	PGPAddNode(sErrorMap, EINPROGRESS,
		   (PGPUserValue) kPGPError_SocketsInProgress);
	PGPAddNode(sErrorMap, EINVAL,
		   (PGPUserValue) kPGPError_SocketsNotBound);
	PGPAddNode(sErrorMap, ENOBUFS,
		   (PGPUserValue) kPGPError_OutOfMemory);
	PGPAddNode(sErrorMap, ENOTSOCK,
		   (PGPUserValue) kPGPError_SocketsNotASocket);
	PGPAddNode(sErrorMap, EOPNOTSUPP,
		   (PGPUserValue) kPGPError_SocketsOperationNotSupported);
	PGPAddNode(sErrorMap, EADDRINUSE,
		   (PGPUserValue) kPGPError_SocketsAddressInUse);
	PGPAddNode(sErrorMap, EAFNOSUPPORT,
		   (PGPUserValue) kPGPError_SocketsAddressFamilyNotSupported);
	PGPAddNode(sErrorMap, EADDRNOTAVAIL,
		   (PGPUserValue) kPGPError_SocketsAddressNotAvailable);
	PGPAddNode(sErrorMap, ECONNREFUSED,
		   (PGPUserValue) kPGPError_SocketsNotConnected);
	PGPAddNode(sErrorMap, EDESTADDRREQ,
		   (PGPUserValue) kPGPError_BadParams);
	PGPAddNode(sErrorMap, EISCONN,
		   (PGPUserValue) kPGPError_SocketsAlreadyConnected);
	PGPAddNode(sErrorMap, ENETUNREACH,
		   (PGPUserValue) kPGPError_SocketsNetworkDown);
	PGPAddNode(sErrorMap, ETIMEDOUT,
		   (PGPUserValue) kPGPError_SocketsTimedOut);
	PGPAddNode(sErrorMap, ENOPROTOOPT,
		   (PGPUserValue) kPGPError_BadParams);
	PGPAddNode(sErrorMap, EMSGSIZE,
		   (PGPUserValue) kPGPError_SocketsBufferOverflow);
	PGPAddNode(sErrorMap, ECONNABORTED,
		   (PGPUserValue) kPGPError_SocketsNotConnected);
	PGPAddNode(sErrorMap, ECONNRESET,
		   (PGPUserValue) kPGPError_SocketsNotConnected);
	PGPAddNode(sErrorMap, ENETRESET,
		   (PGPUserValue) kPGPError_SocketsNotConnected);
	PGPAddNode(sErrorMap, EPROTONOSUPPORT,
		   (PGPUserValue) kPGPError_SocketsProtocolNotSupported);
	PGPAddNode(sErrorMap, EPROTOTYPE,
		   (PGPUserValue) kPGPError_SocketsProtocolNotSupported);
	PGPAddNode(sErrorMap, ESOCKTNOSUPPORT,
		   (PGPUserValue) kPGPError_BadParams);
	PGPAddNode(sErrorMap, HOST_NOT_FOUND,
		   (PGPUserValue) kPGPError_SocketsHostNotFound);
	PGPAddNode(sErrorMap, TRY_AGAIN,
		   (PGPUserValue) kPGPError_SocketsDomainServerError);
	PGPAddNode(sErrorMap, NO_RECOVERY,
		   (PGPUserValue) kPGPError_SocketsDomainServerError);
	PGPAddNode(sErrorMap, NO_DATA,
		   (PGPUserValue) kPGPError_SocketsDomainServerError);
#endif
}

#if PGP_WIN32
BOOL WINAPI
SocketsBlockingHook()
{
	PGPEventHandlerProcPtr	eventHandler = NULL;
	PGPUserValue		userValue;
	PGPError			pgpError;
	
	PGPRMWOLockStartReading(&sIdleEventHandlerLock);
	(void) PGPThreadGetSpecific(sIdleEventHandlerIndex, (void**)&eventHandler);
	(void) PGPThreadGetSpecific(sIdleEventHandlerDataIndex, &userValue);
	PGPRMWOLockStopReading(&sIdleEventHandlerLock);
	
	if (eventHandler != NULL) {
		PGPEvent theEvent;
	
		pgpClearMemory(&theEvent, sizeof(theEvent));
		theEvent.type = kPGPEvent_SocketsIdleEvent;
		pgpError = eventHandler(NULL, &theEvent, userValue);
		if (IsPGPError(pgpError)) {
			WSACancelBlockingCall();
		}
		
	}
	
	return FALSE;
}
#endif	/* PGP_WIN32 */

/*
 * This fancy int flipping is needed for maximum compatability with 64-bit
 * machines, which don't always do the right thing in ntohl, or htonl
 * This code correctly flips a 32 bit number on all platforms.
 */
PGPInt32
PGPHostToNetLong(PGPInt32 x)
{
	unsigned int y = x;
	unsigned char *s = (unsigned char *)&y;
	
	return ((int)s[0]) << 24 | ((int)s[1]) << 16
	| ((int)s[2]) << 8 | ((int)s[3]);
}

PGPInt16
PGPHostToNetShort(PGPInt16 x)
{
	return htons(x);
}

PGPInt32
PGPNetToHostLong(PGPInt32 x)
{
	unsigned int y = x;
	unsigned char *s = (unsigned char *)&y;
	
	return ((int)s[0]) << 24 | ((int)s[1]) << 16
		| ((int)s[2]) << 8 | ((int)s[3]);
}

PGPInt16
PGPNetToHostShort(PGPInt16 x)
{
	return ntohs(x);
}