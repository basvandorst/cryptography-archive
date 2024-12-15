/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpSKEP.c,v 1.44 1999/03/10 02:58:43 heller Exp $
____________________________________________________________________________*/
#include "pgpSKEP.h"
#include "pgpSharePriv.h"
#include "pgpSockets.h"
#include "pgpTLS.h"
#include "pgpMem.h"
#include "pgpMemoryMgr.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "pgpEndianConversion.h"
#include "pgpClientErrors.h"

#if PGP_WIN32
#include "pgpAsyncDNS.h"
#endif

typedef struct PGPskep		PGPskep;

#define kPGPskepMagic	0x156A3C91

struct PGPskep
{
	PGPUInt32				magic;
	PGPContextRef			context;
	PGPtlsContextRef		tlsContext;
	PGPSocketRef			socketOut;
	PGPSocketRef			socketIn;
	PGPskepEventHandler		eventHandler;
	PGPUserValue			userValue;
	PGPBoolean				cancel;
	PGPBoolean				freeTLSContext;
#if PGP_WIN32
	PGPAsyncHostEntryRef	asyncHostEntryRef;
#endif
};

#define PGPValidateSKEP(ref)	PGPValidateParam(ref != NULL)

#define kPGPskep_Port				14747
#define kPGPskep_ClosingMessage		0x155901C5
#define kPGPskepEndian				kPGPBigEndian

#define kPGPskepMajor	0x01	/* SKEP major version */
#define kPGPskepMinor	0x01	/* SKEP minor version */


static PGPError sSocketSend(PGPskepRef skep, PGPSocketRef socket, 
					void *data, PGPInt32 dataSize, PGPBoolean crossPlatform,
					PGPskepEventProgressData *progressData);

static PGPError sSocketReceive(PGPskepRef skep, PGPSocketRef socket, 
					void *data, PGPInt32 dataSize, PGPBoolean crossPlatform,
					PGPskepEventProgressData *progressData);

static PGPError sProcessResult(PGPskepRef skep, PGPInt32 result,
					PGPskepEventProgressData *progressData);

static PGPError sSocketCallback(PGPContextRef context, 
					struct PGPEvent *event, PGPUserValue userValue);


PGPError PGPNewSKEP(PGPContextRef context, 
				PGPtlsContextRef tlsContext,
				PGPskepRef *skep)
{
	PGPMemoryMgrRef memoryMgr = NULL;
	PGPError		err = kPGPError_NoErr;

	if (IsntNull(skep))
		*skep = NULL;

	PGPValidateParam(PGPContextRefIsValid(context));
	PGPValidatePtr(skep);

	memoryMgr = PGPGetContextMemoryMgr(context);
	
	pgpAssert(PGPMemoryMgrIsValid(memoryMgr));
	if (!PGPMemoryMgrIsValid(memoryMgr))
		return kPGPError_BadParams;

	*skep = (PGPskepRef) PGPNewData(memoryMgr, sizeof(PGPskep), 
							kPGPMemoryMgrFlags_Clear);

	pgpAssert(IsntNull(*skep));
	if (IsNull(*skep))
		return kPGPError_OutOfMemory;

	if (PGPtlsContextRefIsValid(tlsContext))
	{
		(*skep)->tlsContext = tlsContext;
		(*skep)->freeTLSContext = FALSE;
	}
	else
	{
		PGPNewTLSContext(context, &((*skep)->tlsContext));
		(*skep)->freeTLSContext = TRUE;
	}

	(*skep)->magic = kPGPskepMagic;
	(*skep)->context = context;
	(*skep)->socketOut = NULL;
	(*skep)->socketIn = NULL;
	(*skep)->eventHandler = NULL;
	(*skep)->userValue = NULL;
	(*skep)->cancel = FALSE;

	return err;
}


PGPError PGPskepSetEventHandler(PGPskepRef skep, PGPskepEventHandler handler,
				PGPUserValue userValue)
{
	PGPValidateSKEP(skep);
	PGPValidatePtr(handler);
	pgpAssertMsg(skep->magic == kPGPskepMagic, "Invalid SKEP object");

	skep->eventHandler = handler;
	skep->userValue = userValue;
	return kPGPError_NoErr;
}


PGPError PGPskepSendShares(PGPskepRef skep, PGPKeyRef authKey,
				const char *passphrase, PGPShareRef shares,
				const char *destSocketAddress)
{
	PGPSocketRef				socket = NULL;
	PGPtlsSessionRef			tlsSession = NULL;
	PGPEventHandlerProcPtr		oldCallback = NULL;
	PGPUserValue				oldUserValue;
	PGPSocketAddressInternet	sockAddr;
	PGPHostEntry *				hostEntry = NULL;
	PGPskepEvent				skepEvent;
	PGPskepEventProgressData	progressData = {0,0};
	PGPInt32					result;
	PGPKeyRef					remoteKey = NULL;
	PGPKeyID					keyID;
	PGPShareID					shareID;
	PGPUInt32					inetAddress;
	PGPUInt32					threshold;
	PGPUInt32					numShares;
	PGPUInt32					totalShares;
	PGPUInt32					closing;
	PGPSize						shareDataSize;
	PGPByte *					shareData = NULL;
	PGPByte						version;
	PGPBoolean					weConfirmed;
	PGPBoolean					theyConfirmed;
	PGPError					err = kPGPError_NoErr;
	PGPSocketsThreadStorageRef	previousStorage;
#if PGP_WIN32
	char						h_name[256];
#endif

	PGPValidateSKEP(skep);
	PGPValidateParam(PGPKeyRefIsValid(authKey));
	PGPValidatePtr(passphrase);
	PGPValidateParam(PGPShareRefIsValid(shares));
	PGPValidatePtr(destSocketAddress);
	pgpAssertMsg(skep->magic == kPGPskepMagic, "Invalid SKEP object");

	skep->cancel = FALSE;

	PGPSocketsInit();
	PGPSocketsCreateThreadStorage(&previousStorage);
	if (IsntNull(skep->eventHandler))
	{
		PGPGetSocketsIdleEventHandler(&oldCallback, &oldUserValue);
		PGPSetSocketsIdleEventHandler(sSocketCallback, skep);
	}

	inetAddress = PGPDottedToInternetAddress(destSocketAddress);
	if (inetAddress != kPGPSockets_Error)
	{
#if PGP_WIN32
		err = PGPStartAsyncGetHostByAddress(inetAddress,
					&skep->asyncHostEntryRef);
		if (IsntPGPError(err)) {
			PGPWaitForGetHostByAddress(skep->asyncHostEntryRef,
					h_name, 256);
			skep->asyncHostEntryRef = kPGPInvalidAsyncHostEntryRef;
		}
#else
		hostEntry = PGPGetHostByAddress((char *) &inetAddress, 
						sizeof(PGPInternetAddress), 
						kPGPProtocolFamilyInternet);
#endif
	}
	else
	{
#if PGP_WIN32
		err = PGPStartAsyncGetHostByName(destSocketAddress,
					&skep->asyncHostEntryRef);
		if (IsntPGPError(err)) {
			PGPWaitForGetHostByName(skep->asyncHostEntryRef,
					&inetAddress);
			skep->asyncHostEntryRef = kPGPInvalidAsyncHostEntryRef;
			strcpy(h_name, destSocketAddress);
		}
#else
		hostEntry = PGPGetHostByName(destSocketAddress);
#endif
	}
		
	if (IsntPGPError(err) && IsNull(hostEntry))
		if (inetAddress == kPGPSockets_Error)
			err = PGPGetLastSocketsError();

	if (IsntPGPError(err))
	{
		socket = PGPOpenSocket(kPGPAddressFamilyInternet, 
					kPGPSocketTypeStream, kPGPTCPProtocol);
		
		if (IsNull(socket))
		{
			err = PGPGetLastSocketsError();
			pgpAssertNoErr(err);
		}
	}

	if (IsntPGPError(err))
	{
		skep->socketOut = socket;
		sockAddr.sin_family = kPGPAddressFamilyInternet;
		sockAddr.sin_port = PGPHostToNetShort(kPGPskep_Port);

		/* If we were able to get the host entry, use the IP address list */
		/* from that. If not, use the IP address passed in by the caller  */

		if (IsntNull(hostEntry))
			sockAddr.sin_addr = *((PGPInternetAddress*) 
									*hostEntry->h_addr_list);
		else
			sockAddr.sin_addr = *((PGPInternetAddress*) &inetAddress);

		result = PGPConnect(socket, (PGPSocketAddress *) &sockAddr, 
					sizeof(sockAddr));

		if (result == kPGPSockets_Error)
			err = PGPGetLastSocketsError();
		else
		{
			skepEvent.type = kPGPskepEvent_ConnectEvent;
			if (skep->eventHandler != NULL)
				err = skep->eventHandler(skep, &skepEvent, 
						skep->userValue);
		}
	}

	if (IsntPGPError(err))
	{
		err = PGPNewTLSSession(skep->tlsContext, &tlsSession);
		pgpAssertNoErr(err);
	}

	if (IsntPGPError(err))
	{
		err = PGPtlsSetProtocolOptions(tlsSession, kPGPtlsFlags_ClientSide);
		pgpAssertNoErr(err);
	}

	if (IsntPGPError(err))
	{
		err = PGPtlsSetLocalPrivateKey(tlsSession, authKey,
					kInvalidPGPSigRef, kInvalidPGPKeySetRef,
					PGPOPassphrase( skep->context, passphrase ),
					PGPOLastOption( skep->context ) );
		pgpAssertNoErr(err);
	}

	if (IsntPGPError(err))
	{
		err = PGPSocketsEstablishTLSSession(socket, tlsSession);
		if (IsPGPError(err))
			err = PGPGetLastSocketsError();

		if (skep->cancel == TRUE)
			err = kPGPError_UserAbort;

		pgpAssertNoErr(err);
	}

	if (IsntPGPError(err))
	{
		err = PGPtlsGetRemoteAuthenticatedKey(tlsSession, &remoteKey, NULL);

		if (skep->cancel == TRUE)
			err = kPGPError_UserAbort;

		pgpAssertNoErr(err);
	}

	if (IsntPGPError(err))
	{
		/* If we don't have the host name, set it to the IP address */

		if (IsntNull(hostEntry))
#if PGP_WIN32
			skepEvent.data.ad.remoteHostname = h_name;
#else
			skepEvent.data.ad.remoteHostname = hostEntry->h_name;
#endif
		else
			skepEvent.data.ad.remoteHostname = 
				PGPInternetAddressToDottedString(sockAddr.sin_addr);

		skepEvent.type = kPGPskepEvent_AuthenticateEvent;
		skepEvent.data.ad.remoteKey = remoteKey;
		skepEvent.data.ad.remoteIPAddress = 
			PGPInternetAddressToDottedString(sockAddr.sin_addr);
		PGPtlsGetNegotiatedCipherSuite( tlsSession,
			&skepEvent.data.ad.tlsCipher );
		
		if( ( skepEvent.data.ad.tlsCipher == kPGPtls_TLS_PGP_DHE_DSS_WITH_NULL_SHA ) ||
			( skepEvent.data.ad.tlsCipher == kPGPtls_TLS_DHE_DSS_WITH_NULL_SHA ) )
			err = kPGPError_TLSNoCommonCipher;
		else
		{
			if (skep->eventHandler != NULL)
				err = skep->eventHandler(skep, &skepEvent, 
						skep->userValue);

			if (IsntPGPError(err))
				weConfirmed = TRUE;
			else
				weConfirmed = FALSE;

			err = kPGPError_NoErr;
		}
	}

	if (IsntPGPError(err))
	{
		pgpGetShareData(shares, &shareData, &shareDataSize);

		progressData.bytesTotal = sizeof(shareDataSize) + shareDataSize + 
									sizeof(PGPKeyID) + sizeof(PGPShareID) +
									sizeof(threshold) + sizeof(numShares) + 
									sizeof(totalShares) + 4;

		version = kPGPskepMajor;
		err = sSocketSend(skep, socket, &version, 1, FALSE, &progressData);
	}
	else
		remoteKey = NULL;

	if (IsntPGPError(err))
	{
		version = kPGPskepMinor;
		err = sSocketSend(skep, socket, &version, 1, FALSE, &progressData);
	}

	if (IsntPGPError(err))
	{
		err = sSocketSend(skep, socket, &weConfirmed, 1, FALSE, 
				&progressData);
	}

	if (IsntPGPError(err))
	{
		err = sSocketReceive(skep, socket, &theyConfirmed, 1, FALSE, 
				&progressData);

		if (!theyConfirmed)
			err = kPGPClientError_RejectedSKEPAuthentication;

		if (!weConfirmed)
			err = kPGPError_UserAbort;
	}

	if (IsntPGPError(err))
		err = sSocketSend(skep, socket, &shareDataSize, 
				sizeof(shareDataSize), TRUE, &progressData);

	if (IsntPGPError(err))
	{
		PGPGetKeyIDFromShares(shares, &keyID);
		err = sSocketSend(skep, socket, &keyID, sizeof(PGPKeyID), 
				FALSE, &progressData);
	}

	if (IsntPGPError(err))
	{
		PGPGetShareID(shares, &shareID);
		err = sSocketSend(skep, socket, &shareID, sizeof(PGPShareID), 
				FALSE, &progressData);
	}

	if (IsntPGPError(err))
	{
		threshold = PGPGetShareThreshold(shares);
		err = sSocketSend(skep, socket, &threshold, sizeof(threshold), 
				TRUE, &progressData);
	}

	if (IsntPGPError(err))
	{
		numShares = PGPGetNumberOfShares(shares);
		err = sSocketSend(skep, socket, &numShares, sizeof(numShares), 
				TRUE, &progressData);
	}

	if (IsntPGPError(err))
	{
		totalShares = PGPGetTotalNumberOfShares(shares);
		err = sSocketSend(skep, socket, &totalShares, sizeof(totalShares), 
				TRUE, &progressData);
	}

	if (IsntPGPError(err))
	{
		err = sSocketSend(skep, socket, shareData, 
				shareDataSize, FALSE, &progressData);
	}

	if (IsntPGPError(err))
	{
		skepEvent.type = kPGPskepEvent_CloseEvent;
		if (skep->eventHandler != NULL)
			err = skep->eventHandler(skep, &skepEvent, 
					skep->userValue);
	}

	if (IsntPGPError(err))
	{
		progressData.bytesTotal = 0;
		err = sSocketReceive(skep, socket, &closing, sizeof(closing), TRUE,
				&progressData);
	}

	if (IsntPGPError(err))
	{
		if (closing != kPGPskep_ClosingMessage)
		{
			err = kPGPError_CorruptData;
			pgpAssertNoErr(err);
		}
	}

	if (IsntNull(shareData))
		PGPFreeData(shareData);

	if (IsntNull(tlsSession))
	{
		if (IsntNull(remoteKey))
			PGPtlsClose(tlsSession, FALSE);
		else
			PGPtlsClose(tlsSession, TRUE);
		PGPFreeTLSSession(tlsSession);
	}

	if (IsntNull(socket))
	{
		PGPCloseSocket(socket);
		skep->socketOut = NULL;
	}

	if (IsntNull(skep->eventHandler))
		PGPSetSocketsIdleEventHandler(oldCallback, oldUserValue);

	PGPSocketsDisposeThreadStorage(previousStorage);
	PGPSocketsCleanup();

	if (skep->cancel == TRUE)
		err = kPGPError_UserAbort;

	return err;
}


PGPError PGPskepReceiveShares(PGPskepRef skep, PGPKeyRef authKey,
				const char *passphrase)
{
	PGPShareRef					shares;
	PGPSocketRef				socket = NULL;
	PGPSocketRef				socketIn;
	PGPtlsSessionRef			tlsSession;
	PGPEventHandlerProcPtr		oldCallback = NULL;
	PGPUserValue				oldUserValue;
	PGPSocketAddressInternet	sockAddr;
	PGPskepEvent				skepEvent;
	PGPskepEventProgressData	progressData;
	PGPInt32					result;
	PGPKeyRef					remoteKey;
	PGPKeyID					keyID;
	PGPShareID					shareID;
	PGPUInt32					threshold;
	PGPUInt32					numShares;
	PGPUInt32					totalShares;
	PGPUInt32					closing = kPGPskep_ClosingMessage;
	PGPUInt32					shareDataSize;
	PGPByte *					shareData;
	PGPByte						version;
	PGPBoolean					weConfirmed;
	PGPBoolean					theyConfirmed;
	PGPHostEntry *				hostEntry;
	PGPError					err = kPGPError_NoErr;
	PGPSocketsThreadStorageRef	previousStorage;
#if PGP_WIN32
	char						h_name[256];
#endif

	PGPValidateSKEP(skep);
	PGPValidateParam(PGPKeyRefIsValid(authKey));
	PGPValidatePtr(passphrase);
	pgpAssertMsg(skep->magic == kPGPskepMagic, "Invalid SKEP object");

	skep->cancel = FALSE;

	PGPSocketsInit();
	PGPSocketsCreateThreadStorage(&previousStorage);

	if (IsntNull(skep->eventHandler))
	{
		PGPGetSocketsIdleEventHandler(&oldCallback, &oldUserValue);
		PGPSetSocketsIdleEventHandler(sSocketCallback, skep);
	}

	socket = PGPOpenSocket(kPGPAddressFamilyInternet, 
				kPGPSocketTypeStream, kPGPTCPProtocol);

	if (IsNull(socket))
	{
		err = PGPGetLastSocketsError();
		pgpAssertNoErr(err);
	}

	if (IsntPGPError(err))
	{
		skep->socketOut = socket;
		sockAddr.sin_family = kPGPAddressFamilyInternet;
		sockAddr.sin_port = PGPHostToNetShort(kPGPskep_Port);
		sockAddr.sin_addr.s_addr = kPGPInternetAddressAny;

		result = PGPBindSocket(socket, (PGPSocketAddress *) &sockAddr, 
					sizeof(sockAddr));

		if (result == kPGPSockets_Error)
		{
			err = PGPGetLastSocketsError();
			pgpAssertNoErr(err);
		}
	}

	if (IsntPGPError(err))
	{
		result = PGPListen(socket, 4);
		if (result == kPGPSockets_Error)
		{
			err = PGPGetLastSocketsError();
		
			if (skep->cancel == TRUE)
				err = kPGPError_UserAbort;

			pgpAssertNoErr(err);
		}
	}

	while (IsntPGPError(err))
	{
		socketIn = NULL;
		tlsSession = NULL;
		progressData.bytesSoFar = 0;
		progressData.bytesTotal = 0;
		remoteKey = NULL;
		shareData = NULL;
		hostEntry = NULL;
		shares = NULL;
		weConfirmed = TRUE;
		theyConfirmed = TRUE;
		
		skepEvent.type = kPGPskepEvent_ListenEvent;
		if (skep->eventHandler != NULL)
			err = skep->eventHandler(skep, &skepEvent, 
					skep->userValue);
		result = sizeof(sockAddr);
		socketIn = PGPAccept(socket, (PGPSocketAddress *) &sockAddr, 
						&result);

		if (IsNull(socketIn) || ((int) socketIn == kPGPSockets_Error))
		{
			err = PGPGetLastSocketsError();
		
			if (skep->cancel == TRUE)
				err = kPGPError_UserAbort;

			pgpAssertNoErr(err);
		}

		if (IsntPGPError(err))
		{
			skep->socketIn = socketIn;
			skepEvent.type = kPGPskepEvent_ConnectEvent;
			if (skep->eventHandler != NULL)
				err = skep->eventHandler(skep, &skepEvent, 
						skep->userValue);
		}
		
		if (IsntPGPError(err))
		{
	#if PGP_WIN32
			err = PGPStartAsyncGetHostByAddress(sockAddr.sin_addr.s_addr,
						&skep->asyncHostEntryRef);
			if (IsntPGPError(err)) {
				PGPWaitForGetHostByAddress(skep->asyncHostEntryRef,
						h_name, 256);
				skep->asyncHostEntryRef = kPGPInvalidAsyncHostEntryRef;
			}
	#else
			hostEntry = PGPGetHostByAddress((char *) &sockAddr.sin_addr, 
							sizeof(PGPInternetAddress), 
							kPGPProtocolFamilyInternet);
	#endif
		}

		if (IsntPGPError(err))
		{
			err = PGPNewTLSSession(skep->tlsContext, &tlsSession);
			pgpAssertNoErr(err);
		}

		if (IsntPGPError(err))
		{
			err = PGPtlsSetProtocolOptions(tlsSession, 
					kPGPtlsFlags_ServerSide | kPGPtlsFlags_RequestClientCert);
			pgpAssertNoErr(err);
		}

		if (IsntPGPError(err))
		{
			err = PGPtlsSetLocalPrivateKey(tlsSession, authKey,
						kInvalidPGPSigRef, kInvalidPGPKeySetRef,
						PGPOPassphrase( skep->context, passphrase ),
						PGPOLastOption( skep->context ) );
			pgpAssertNoErr(err);
		}

		if (IsntPGPError(err))
		{
			err = PGPSocketsEstablishTLSSession(socketIn, tlsSession);
			if (IsPGPError(err))
				err = PGPGetLastSocketsError();

			if (skep->cancel == TRUE)
				err = kPGPError_UserAbort;

			pgpAssertNoErr(err);
		}

		if (IsntPGPError(err))
		{
			err = PGPtlsGetRemoteAuthenticatedKey(tlsSession, &remoteKey, NULL);

			if (skep->cancel == TRUE)
				err = kPGPError_UserAbort;

			pgpAssertNoErr(err);
		}

		if (IsntPGPError(err))
		{
			skepEvent.type = kPGPskepEvent_AuthenticateEvent;
			skepEvent.data.ad.remoteKey = remoteKey;
			skepEvent.data.ad.remoteIPAddress = 
				PGPInternetAddressToDottedString(sockAddr.sin_addr);
			PGPtlsGetNegotiatedCipherSuite( tlsSession,
				&skepEvent.data.ad.tlsCipher );
			
			/* If the hostname doesn't exist, set it to the ip address */
			if (IsntNull(hostEntry))
			{
	#if PGP_WIN32
				skepEvent.data.ad.remoteHostname = h_name;
	#else
				skepEvent.data.ad.remoteHostname = hostEntry->h_name;
	#endif
			}
			else
			{
				skepEvent.data.ad.remoteHostname = 
					skepEvent.data.ad.remoteIPAddress;
			}

			if (skep->eventHandler != NULL)
				err = skep->eventHandler(skep, &skepEvent, 
						skep->userValue);

			if (IsntPGPError(err))
				weConfirmed = TRUE;
			else
				weConfirmed = FALSE;

			err = kPGPError_NoErr;
		}

		if (IsntPGPError(err))
			err = sSocketReceive(skep, socketIn, &version, 1, FALSE, 
					&progressData);
		else
			remoteKey = NULL;

		if (IsntPGPError(err))
		{
			if (version != kPGPskepMajor)
				err = kPGPClientError_IncorrectSKEPVersion;
		}

		if (IsntPGPError(err))
			err = sSocketReceive(skep, socketIn, &version, 1, FALSE, 
					&progressData);

		if (IsntPGPError(err))
			err = sSocketReceive(skep, socketIn, &theyConfirmed, 1, FALSE, 
					&progressData);

		if (IsntPGPError(err))
		{
			err = sSocketSend(skep, socketIn, &weConfirmed, 1, FALSE, 
					&progressData);

			if (!theyConfirmed)
				err = kPGPClientError_RejectedSKEPAuthentication;

			if (!weConfirmed)
				err = kPGPError_UserAbort;
		}

		if (IsntPGPError(err))
		{
			err = sSocketReceive(skep, socketIn, &shareDataSize, 
					sizeof(shareDataSize), TRUE, &progressData);

			progressData.bytesTotal = sizeof(shareDataSize) + shareDataSize + 
										sizeof(PGPKeyID) + sizeof(PGPShareID) +
										sizeof(threshold) + sizeof(numShares) + 
										sizeof(totalShares) + 4;
		}

		if (IsntPGPError(err))
		{
			shareData = (PGPByte *) 
						PGPNewSecureData(PGPGetContextMemoryMgr(skep->context),
							shareDataSize, kPGPMemoryMgrFlags_Clear);

			if (shareData == NULL)
			{
				err = kPGPError_OutOfMemory;;
				pgpAssertNoErr(err);
			}
		}

		if (IsntPGPError(err))
		{
			err = sSocketReceive(skep, socketIn, &keyID, sizeof(PGPKeyID), 
					FALSE, &progressData);
		}

		if (IsntPGPError(err))
		{
			err = sSocketReceive(skep, socketIn, &shareID, sizeof(PGPShareID), 
					FALSE, &progressData);
		}

		if (IsntPGPError(err))
		{
			err = sSocketReceive(skep, socketIn, &threshold, sizeof(threshold),
					TRUE, &progressData);
		}

		if (IsntPGPError(err))
		{
			err = sSocketReceive(skep, socketIn, &numShares, sizeof(numShares),
					TRUE, &progressData);
		}

		if (IsntPGPError(err))
		{
			err = sSocketReceive(skep, socketIn, &totalShares, 
					sizeof(totalShares), TRUE, &progressData);
		}

		if (IsntPGPError(err))
		{
			err = sSocketReceive(skep, socketIn, shareData, 
					(PGPInt32) shareDataSize, FALSE, &progressData);
		}

		if (IsntPGPError(err))
		{
			skepEvent.type = kPGPskepEvent_CloseEvent;
			if (skep->eventHandler != NULL)
				err = skep->eventHandler(skep, &skepEvent, 
						skep->userValue);
		}

		if (IsntPGPError(err))
		{
			progressData.bytesTotal = 0;
			err = sSocketSend(skep, socketIn, &closing, sizeof(closing), TRUE,
					&progressData);
		}

		if (IsntNull(tlsSession))
		{
			if (IsntNull(remoteKey))
				PGPtlsClose(tlsSession, FALSE);
			else
				PGPtlsClose(tlsSession, TRUE);
			PGPFreeTLSSession(tlsSession);
		}

		if (IsntNull(socketIn))
		{
			PGPCloseSocket(socketIn);
			skep->socketIn = NULL;
		}

		if (IsntPGPError(err) && IsntNull(shareData))
		{
			err = pgpCreateShares(skep->context, keyID, shareID, threshold, 
					numShares, totalShares, shareDataSize, shareData, &shares);
			pgpAssertNoErr(err);
		}

		if (IsntNull(shareData))
			PGPFreeData(shareData);
			
		if (IsntPGPError(err))
		{
			skepEvent.type = kPGPskepEvent_ShareEvent;
			skepEvent.data.sd.shares = shares;

			if (skep->eventHandler != NULL)
				err = skep->eventHandler(skep, &skepEvent, 
						skep->userValue);
		}

		if (!weConfirmed)
			err = kPGPError_NoErr;
	}

	if (IsntNull(socket))
	{
		PGPCloseSocket(socket);
		skep->socketOut = NULL;
	}

	if (IsntNull(skep->eventHandler))
		PGPSetSocketsIdleEventHandler(oldCallback, oldUserValue);

	PGPSocketsDisposeThreadStorage(previousStorage);
	PGPSocketsCleanup();

	if (skep->cancel == TRUE)
		err = kPGPError_NoErr;

	return err;
}


PGPError PGPskepCancel(PGPskepRef skep)
{
	PGPValidateSKEP(skep);
	pgpAssertMsg(skep->magic == kPGPskepMagic, "Invalid SKEP object");

	if (IsntNull(skep->socketIn))
	{
		PGPCloseSocket(skep->socketIn);
		skep->socketIn = NULL;
	}

	if (IsntNull(skep->socketOut))
	{
		PGPCloseSocket(skep->socketOut);
		skep->socketOut = NULL;
	}
#if PGP_WIN32
	if (PGPAsyncHostEntryRefIsValid(skep->asyncHostEntryRef)) {
		PGPCancelAsyncHostEntryRef(skep->asyncHostEntryRef);
		skep->asyncHostEntryRef = kPGPInvalidAsyncHostEntryRef;
	}
#endif

	skep->cancel = TRUE;

	return kPGPError_NoErr;
}


PGPError PGPFreeSKEP(PGPskepRef skep)
{
	PGPValidateSKEP(skep);
	pgpAssertMsg(skep->magic == kPGPskepMagic, "Invalid SKEP object");

	if (IsntNull(skep->socketIn) || IsntNull(skep->socketOut))
		PGPskepCancel(skep);	

	if (skep->freeTLSContext)
		PGPFreeTLSContext(skep->tlsContext);

	PGPFreeData(skep);
	return kPGPError_NoErr;
}

PGPContextRef PGPGetSKEPContext(PGPskepRef skep)
{
	PGPContextRef	context;
	
	if( PGPskepRefIsValid( skep ) )
	{
		if (skep->magic == kPGPskepMagic)
			context = skep->context;
		else
			context = kInvalidPGPContextRef;
	}
	else
	{
		context = kInvalidPGPContextRef;
	}
	
	return context;
}

static PGPError sSocketSend(PGPskepRef skep, PGPSocketRef socket, 
					void *data, PGPInt32 dataSize, PGPBoolean crossPlatform,
					PGPskepEventProgressData *progressData)
{
	PGPByte		tempData16[sizeof(PGPUInt16)];
	PGPByte		tempData32[sizeof(PGPUInt32)];
	PGPUInt16	data16;
	PGPUInt32	data32;
	PGPInt32	result = 0;

	if (crossPlatform)
	{
		if (dataSize == sizeof(PGPUInt16))
		{
			pgpCopyMemory(data, &data16, sizeof(PGPUInt16));
			PGPUInt16ToEndian(data16, kPGPskepEndian, tempData16);
			result = PGPSend(socket, tempData16, dataSize, kPGPSendFlagNone);
		}
		else
		{
			pgpCopyMemory(data, &data32, sizeof(PGPUInt32));
			PGPUInt32ToEndian(data32, kPGPskepEndian, tempData32);
			result = PGPSend(socket, tempData32, dataSize, kPGPSendFlagNone);
		}
	}
	else
		result = PGPSend(socket, data, dataSize, kPGPSendFlagNone);

	return sProcessResult(skep, result, progressData);
}


static PGPError sSocketReceive(PGPskepRef skep, PGPSocketRef socket, 
					void *data, PGPInt32 dataSize, PGPBoolean crossPlatform,
					PGPskepEventProgressData *progressData)
{
	PGPByte		tempData16[sizeof(PGPUInt16)];
	PGPByte		tempData32[sizeof(PGPUInt32)];
	PGPUInt16	data16;
	PGPUInt32	data32;
	PGPInt32	result = 0;

	if (crossPlatform)
	{
		if (dataSize == sizeof(PGPUInt16))
			result = PGPReceive(socket, tempData16, dataSize, 
						kPGPReceiveFlagNone);
		else
			result = PGPReceive(socket, tempData32, dataSize, 
						kPGPReceiveFlagNone);

		if (result > 0)
		{
			if (dataSize == sizeof(PGPUInt16))
			{
				data16 = PGPEndianToUInt16(kPGPskepEndian, tempData16);
				pgpCopyMemory(&data16, data, sizeof(PGPUInt16));
			}
			else
			{
				data32 = PGPEndianToUInt32(kPGPskepEndian, tempData32);
				pgpCopyMemory(&data32, data, sizeof(PGPUInt32));
			}
		}
	}
	else
		result = PGPReceive(socket, data, dataSize, kPGPReceiveFlagNone);

	return sProcessResult(skep, result, progressData);
}


static PGPError sProcessResult(PGPskepRef skep, PGPInt32 result,
					PGPskepEventProgressData *progressData)
{
	PGPskepEvent	skepEvent;
	PGPError		err = kPGPError_NoErr;

	if (result == 0)
		err = kPGPError_UserAbort;
	else if (result == kPGPSockets_Error)
	{
		err = PGPGetLastSocketsError();

		if (skep->cancel == TRUE)
			err = kPGPError_UserAbort;

		pgpAssertNoErr(err);
	}
	else
	{
		progressData->bytesSoFar += result;
		skepEvent.type = kPGPskepEvent_ProgressEvent;
		skepEvent.data.pd.bytesSoFar = progressData->bytesSoFar;
		skepEvent.data.pd.bytesTotal = progressData->bytesTotal;

		if ((skep->eventHandler != NULL) && (progressData->bytesTotal > 0))
			err = skep->eventHandler(skep, &skepEvent, 
					skep->userValue);
	}

	return err;
}


static PGPError sSocketCallback(PGPContextRef context, 
					struct PGPEvent *event, PGPUserValue userValue)
{
	PGPError		err	= kPGPError_NoErr;
	PGPskepRef		skep;
	PGPskepEvent	skepEvent;

	pgpAssert(event != NULL);
	(void) context;
	(void) event;
	skep = (PGPskepRef) userValue;
	
	if (skep->eventHandler != NULL)
	{
		skepEvent.type = kPGPskepEvent_NullEvent;
		err = skep->eventHandler(skep, &skepEvent, skep->userValue);
	}

	return err;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
