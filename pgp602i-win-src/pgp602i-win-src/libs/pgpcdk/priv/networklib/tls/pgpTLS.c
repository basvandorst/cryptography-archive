/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	Platform independent state machine based implementation of
	IETF Transport Layer Security 1.0 with support for OpenPGP/TLS.
	
	TLS 1.0 is specified in IETF RFC 2XXX

	$Id: pgpTLS.c,v 1.48.2.3.2.1 1998/11/12 03:22:48 heller Exp $
____________________________________________________________________________*/

#include "pgpTLSPriv.h"
#include "pgpTLSPrimes.h"

#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpMem.h"
#include "pgpEndianConversion.h"
#include "pgpHash.h"
#include "pgpHMAC.h"
#include "pgpPublicKey.h"
#include "pgpSymmetricCipher.h"
#include "pgpCBC.h"
#include "pgpKeys.h"
#include "pgpFeatures.h"

#include "pgpOptionListPriv.h"

#include <string.h>

#define PGPTLS_FORCESSL3	/* define this for all 6.0.X builds */
#define noPGPTLS_DEBUG

#define CKERR			if( err ) goto done	
#define FATALTLS( x )	{									\
			err = x;										\
			session->state = kPGPtls_FatalErrorState;		\
			goto done;	}

#define TLSNUMCLIENTSTATES	10
#define TLSNUMSERVERSTATES	7
#define TLSNUMEVENTS		15

static PGPInt8 kPGPtls_ClientStateTable[TLSNUMCLIENTSTATES][TLSNUMEVENTS] =
{
	/*		HR	CH	SH	CE	EC	SC	SKE	CR	SHD	CV	CKE	FIN	FAT	NC	CC	*/
{	/* 0 */	-1,	-2,	1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2	},
{	/* 1 */	-1, -2,	-2,	2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2	},
{	/* 2 */	-1,	-2,	-2,	-2,	3,	4,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2	},
{	/* 3 */	-1,	-2,	-2,	-2,	-2,	-2,	5,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2	},
{	/* 4 */	-1, -2,	-2,	-2,	-2,	-2,	-2,	6,	7,	-2,	-2,	-2,	-2,	-2,	-2	},
{	/* 5 */	-1, -2,	-2,	-2,	-2,	-2,	-2,	6,	7,	-2,	-2,	-2,	-2,	-2,	-2	},
{	/* 6 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	7,	-2,	-2,	-2,	-2,	-2,	-2	},
{	/* 7 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	8	},
{	/* 8 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	9,	-2,	-2,	-2	},
{	/* 9 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2	}
};

static PGPInt8 kPGPtls_ServerStateTable[TLSNUMSERVERSTATES][TLSNUMEVENTS] =
{
	/*		HR	CH	SH	CE	EC	SC	SKE	CR	SHD	CV	CKE	FIN	FAT	NC	CC	*/
{	/* 0 */	-1,	1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2	},
{	/* 1 */	-1, -2,	-2,	2,	-2,	-2,	-2,	-2,	-2,	-2,	3,	-2,	-2,	-2,	-2	},
{	/* 2 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	3,	-2,	-2,	-2,	-2	},
{	/* 3 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	4,	-2,	-2,	-2,	-2,	5	},
{	/* 4 */	-1, -2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	5	},
{	/* 5 */	-1, -2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	6,	-2,	-2,	-2	},
{	/* 6 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2	}
};

static PGPInt8 kPGPtls_SSL3ServerStateTable[TLSNUMSERVERSTATES][TLSNUMEVENTS] =
{
	/*		HR	CH	SH	CE	EC	SC	SKE	CR	SHD	CV	CKE	FIN	FAT	NC	CC	*/
{	/* 0 */	-1,	1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2	},
{	/* 1 */	-1, -2,	-2,	2,	-2,	-2,	-2,	-2,	-2,	-2,	3,	-2,	-2,	-1,	-2	},
{	/* 2 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	3,	-2,	-2,	-2,	-2	},
{	/* 3 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	4,	-2,	-2,	-2,	-1,	5	},
{	/* 4 */	-1, -2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	5	},
{	/* 5 */	-1, -2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	6,	-2,	-2,	-2	},
{	/* 6 */	-1,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2,	-2	}
};
		
	PGPInt8
pgpTLSNextState(
	PGPtlsSessionPriv *	session,
	PGPByte				event )
{
	PGPInt8	newState;
	
	if( session->isClientSide )
		newState = kPGPtls_ClientStateTable[session->intState][event];
	else if( session->minorVersion == kPGPtls_SSL3MinorVersion )
		newState = kPGPtls_SSL3ServerStateTable[session->intState][event];
	else
		newState = kPGPtls_ServerStateTable[session->intState][event];
	if( newState == -2 )
		session->state = kPGPtls_FatalErrorState;
	else if( newState >= 0 )
		session->intState = newState;
	return newState;
}

	PGPError
PGPNewTLSContext(
	PGPContextRef		context,
	PGPtlsContextRef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	PGPtlsContextPriv *	pContext;

	PGPValidatePtr( context );
	*outRef = NULL;
	
	pContext = (PGPtlsContextPriv *) pgpContextMemAlloc( context,
											sizeof(PGPtlsContextPriv),
											kPGPMemoryMgrFlags_Clear );
	if( IsntNull( pContext ) )
	{
		pContext->pgpContext = context;
		pContext->useCache = TRUE;		
		
		*outRef = ( PGPtlsContextRef ) pContext;
	}
	else
		err = kPGPError_OutOfMemory;
	
	pgpAssertErrWithPtr( err, *outRef );
	return err;
}

	PGPError
PGPFreeTLSContext(
	PGPtlsContextRef	ref )
{
	PGPValidatePtr( ref );

	return pgpContextMemFree( ((PGPtlsContextPriv *) ref)->pgpContext,
								ref );
}

	PGPError
PGPtlsSetCache(
	PGPtlsContextRef	ref,
	PGPBoolean			useCache )
{
	PGPValidatePtr( ref );
	
	((PGPtlsContextPriv *) ref)->useCache = useCache;
	/* not implemented yet */
	
	return kPGPError_NoErr;
}

	PGPError
PGPtlsClearCache(
	PGPtlsContextRef	ref )
{
	(void) ref;
	
	/* not implemented yet */
	return kPGPError_NoErr;
}

	PGPBoolean
pubAlgSupported(
	PGPPublicKeyAlgorithm		pubAlg )
{
	PGPUInt32					numPubAlgs,
								algIndex;
	PGPPublicKeyAlgorithmInfo	info;
	
	PGPCountPublicKeyAlgorithms( &numPubAlgs );
	for( algIndex = 0; algIndex < numPubAlgs; algIndex++ )
	{
		PGPGetIndexedPublicKeyAlgorithmInfo( algIndex, &info);
		if( info.algID == pubAlg )
			return TRUE;
	}
	return FALSE;
}

	PGPError
PGPNewTLSSession(
	PGPtlsContextRef		ref,
	PGPtlsSessionRef *		outRef )
{
	PGPError			err	= kPGPError_NoErr;
	PGPtlsContextPriv *	pContext;
	PGPtlsSessionPriv *	session;

	PGPValidatePtr( ref );
	*outRef = NULL;
	
	pContext = ( PGPtlsContextPriv * ) ref;
	session = ( PGPtlsSessionPriv * ) pgpContextMemAlloc( pContext->pgpContext,
											sizeof( PGPtlsSessionPriv ),
											kPGPMemoryMgrFlags_Clear );
	if( IsntNull( session ) )
	{
		session->pgpContext			= pContext->pgpContext;
		session->tlsContext			= pContext;
		session->memMgr				=
			PGPGetContextMemoryMgr( pContext->pgpContext );
		session->state				= kPGPtls_IdleState;
		session->intState			= 0;
		session->isClientSide		= TRUE;
		session->blocking			= TRUE;
		session->minorVersion		= kPGPtls_MinorVersion;
		
		session->localKeyAlg		= kPGPPublicKeyAlgorithm_Invalid;
		session->requestedPrime		= kPGPtls_DHPrime2048;
		session->certRequested		= FALSE;
		session->certVerified		= FALSE;
		
		session->tlsReceiveProc		= NULL;
		session->tlsSendProc		= NULL;
		session->tlsReceiveUserData	= NULL;
		session->tlsSendUserData	= NULL;

#ifdef PGPTLS_DEBUG
		err = pgpTLSPRFTest( session );
		if( IsPGPError( err ) )
		{
			pgpAssert( 0 );
		}
#endif
		
		err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_SHA,
									&session->handshakeSHA);
		if( IsntPGPError( err ) )
		{
			err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_MD5,
									&session->handshakeMD5);
		}
		if( IsntPGPError( err ) )
		{
			session->cipherSuites = ( PGPtlsCipherSuite * )
				pgpContextMemAlloc( pContext->pgpContext,
									sizeof( kPGPtls_CipherSuites ),
									kPGPMemoryMgrFlags_Clear );
			if( IsntNull( session->cipherSuites ) )
			{
				/* Determine which available cipher suites we can
					actually perform given the underlying SDK
					algorithms. */
				PGPUInt32					suiteIndex,
											numSymAlgs,
											algIndex,
											totalSuites;
				PGPSymmetricCipherInfo		symInfo;
				PGPBoolean					foundPub,
											foundSym;
				
				totalSuites =	sizeof( kPGPtls_CipherSuites ) /
								sizeof( PGPtlsCipherSuite );
				session->numCipherSuites = 0;
				PGPCountSymmetricCiphers( &numSymAlgs );
				for( suiteIndex = 0; suiteIndex < totalSuites;
						suiteIndex++ )
				{
					foundPub = foundSym = FALSE;
					if( pubAlgSupported(
							kPGPtls_CipherSuites[ suiteIndex ].sigAlg ) )
						foundPub = TRUE;
					if( kPGPtls_CipherSuites[ suiteIndex ].cipherID ==
						kPGPCipherAlgorithm_None )
						foundSym = TRUE;
					else for( algIndex = 0; algIndex < numSymAlgs;
								algIndex++ )
					{
						PGPGetIndexedSymmetricCipherInfo(	algIndex,
															&symInfo );
						if( symInfo.algID ==
							kPGPtls_CipherSuites[ suiteIndex ].cipherID )
						{
							foundSym = TRUE;
							break;
						}
					}
					if( foundPub && foundSym )
						session->cipherSuites[ session->numCipherSuites++ ] =
							kPGPtls_CipherSuites[ suiteIndex ];
				}
			}
			else
				err = kPGPError_OutOfMemory;
		}
		
		session->fatalAlert			= kPGPtls_AT_None;
		
		*outRef = ( PGPtlsSessionRef ) session;
	}
	else
		err = kPGPError_OutOfMemory;
	
	if( IsPGPError( err ) && IsntNull( session ) )
	{
		if( PGPHashContextRefIsValid( session->handshakeSHA ) )
		{
			(void)PGPFreeHashContext( session->handshakeSHA );
			session->handshakeSHA = kInvalidPGPHashContextRef;
		}
		if( PGPHashContextRefIsValid( session->handshakeMD5 ) )
		{
			(void)PGPFreeHashContext( session->handshakeMD5 );
			session->handshakeMD5 = kInvalidPGPHashContextRef;
		}
		if( IsntNull( session->cipherSuites ) )
			(void)PGPFreeData( session->cipherSuites );
		
		pgpContextMemFree( pContext->pgpContext, session );
		*outRef = NULL;
	}
	pgpAssertErrWithPtr( err, *outRef );
	return err;
}

	PGPError
PGPFreeTLSSession( PGPtlsSessionRef ref )
{
	PGPError			err	= kPGPError_NoErr;
	PGPtlsSessionPriv *	session;

	PGPValidatePtr( ref );

	session = (PGPtlsSessionPriv *) ref;
	
	if( session->state == kPGPtls_ClosedState )
	{
		/* Cache the session if applicable */
		
		/* ##### */
	}
	
	if( IsntNull( session->localKeyPassphrase ) )
		(void)PGPFreeData( session->localKeyPassphrase );
	if( IsntNull( session->localKeyPasskeyBuffer ) )
		(void)PGPFreeData( session->localKeyPasskeyBuffer );
	if( PGPKeySetRefIsValid( session->remoteKeySet ) )
		(void)PGPFreeKeySet( session->remoteKeySet );
	if( PGPHashContextRefIsValid( session->handshakeSHA ) )
		(void)PGPFreeHashContext( session->handshakeSHA );
	if( PGPHashContextRefIsValid( session->handshakeMD5 ) )
		(void)PGPFreeHashContext( session->handshakeMD5 );
	if( PGPHashContextRefIsValid( session->lastHandSHA ) )
		(void)PGPFreeHashContext( session->lastHandSHA );
	if( PGPHashContextRefIsValid( session->lastHandMD5 ) )
		(void)PGPFreeHashContext( session->lastHandMD5 );
	
	if( PGPHashContextRefIsValid( session->innerMACHash ) )
		(void)PGPFreeHashContext( session->innerMACHash );
	if( PGPHashContextRefIsValid( session->outerMACHash ) )
		(void)PGPFreeHashContext( session->outerMACHash );
	
	if( PGPCBCContextRefIsValid( session->writeCipher ) )
		(void)PGPFreeCBCContext( session->writeCipher );
	if( PGPCBCContextRefIsValid( session->readCipher ) )
		(void)PGPFreeCBCContext( session->readCipher );
	if( IsntNull( session->writeActive ) )
	{
		if( PGPHMACContextRefIsValid( session->writeActive->hmac ) )
			(void)PGPFreeHMACContext( session->writeActive->hmac );
		(void)PGPFreeData( session->writeActive );
	}
	if( IsntNull( session->readActive ) )
	{
		if( PGPHMACContextRefIsValid( session->readActive->hmac ) )
			(void)PGPFreeHMACContext( session->readActive->hmac );
		(void)PGPFreeData( session->readActive );
	}
	if( IsntNull( session->clientPending ) )
		(void)PGPFreeData( session->clientPending );
	if( IsntNull( session->serverPending ) )
		(void)PGPFreeData( session->serverPending );
		
	if( IsntNull( session->queuedSendData ) )
		(void)PGPFreeData( session->queuedSendData );
	if( IsntNull( session->rcvdRawData ) )
		(void)PGPFreeData( session->rcvdRawData );
	if( IsntNull( session->rcvdAppData ) )
		(void)PGPFreeData( session->rcvdAppData );
	if( IsntNull( session->rcvdHandData ) )
		(void)PGPFreeData( session->rcvdHandData );
	
	if( session->dhP != kPGPInvalidBigNumRef )
	{
		err = PGPFreeBigNum( session->dhP );
		pgpAssertNoErr( err );
	}
	if( session->dhG != kPGPInvalidBigNumRef )
	{
		err = PGPFreeBigNum( session->dhG );
		pgpAssertNoErr( err );
	}
	if( session->dhYs != kPGPInvalidBigNumRef )
	{
		err = PGPFreeBigNum( session->dhYs );
		pgpAssertNoErr( err );
	}
	if( session->dhX != kPGPInvalidBigNumRef )
	{
		err = PGPFreeBigNum( session->dhX );
		pgpAssertNoErr( err );
	}
	if( session->dhYc != kPGPInvalidBigNumRef )
	{
		err = PGPFreeBigNum( session->dhYc );
		pgpAssertNoErr( err );
	}
	if( IsntNull( session->cipherSuites ) )
		(void)PGPFreeData( session->cipherSuites );


	err = pgpContextMemFree( session->pgpContext, session );
	return err;
}

	PGPError
pgpTLSBufferRawData(
	PGPtlsSessionPriv *	session,
	PGPByte *			rawData,
	PGPSize				rawDataSize )
{
	PGPError			err = kPGPError_NoErr;
	
	err = pgpContextMemRealloc( session->pgpContext,
							(void **) &session->rcvdRawData,
							session->rawDataSize + rawDataSize, 0 ); CKERR;
	pgpCopyMemory( rawData, session->rcvdRawData + session->rawDataSize,
					rawDataSize );
	session->rawDataSize += rawDataSize;

done:
	return err;
}

	PGPError
pgpTLSExtractRawData(
	PGPtlsSessionPriv *	session,
	PGPByte *			rawData,
	PGPSize *			rawDataSize )
{
	PGPError			err = kPGPError_NoErr;
	PGPSize				maxSize = *rawDataSize;
	
	*rawDataSize = 0;
	if( ( maxSize > 0 ) && ( session->rawDataSize > 0 ) )
	{
		if( maxSize > session->rawDataSize )
			maxSize = session->rawDataSize;
		pgpCopyMemory( session->rcvdRawData, rawData, maxSize );
		*rawDataSize = maxSize;
		pgpCopyMemory( session->rcvdRawData + maxSize,
						session->rcvdRawData, session->rawDataSize - maxSize );
		session->rawDataSize -= maxSize;
		err = pgpContextMemRealloc( session->pgpContext,
							(void **) &session->rcvdRawData,
							session->rawDataSize, 0 ); CKERR;
	}
done:
	return err;
}

	PGPError
pgpTLSBufferHandData(
	PGPtlsSessionPriv *	session,
	PGPByte *			handData,
	PGPSize				handDataSize )
{
	PGPError			err = kPGPError_NoErr;
	
	err = pgpContextMemRealloc( session->pgpContext,
							(void **) &session->rcvdHandData,
							session->handDataSize + handDataSize, 0 ); CKERR;
	pgpCopyMemory( handData, session->rcvdHandData + session->handDataSize,
					handDataSize );
	session->handDataSize += handDataSize;

done:
	return err;
}

	PGPError
pgpTLSExtractHandData(
	PGPtlsSessionPriv *	session,
	PGPByte **			handData,
	PGPSize *			handDataSize )
{
	PGPError			err = kPGPError_NoErr;
	
	if( session->handDataSize > 0 )
	{
		err = pgpContextMemRealloc( session->pgpContext,
							(void **) handData,
							*handDataSize + session->handDataSize, 0 ); CKERR;

		pgpCopyMemory( *handData, *handData + session->handDataSize, *handDataSize );
		pgpCopyMemory( session->rcvdHandData, *handData, session->handDataSize );
		*handDataSize += session->handDataSize;
		session->handDataSize = 0;
	}
done:
	return err;
}

	PGPError
pgpTLSBufferSendData(
	PGPtlsSessionPriv *	session,
	PGPByte *			sendData,
	PGPSize				sendDataSize )
{
	PGPError			err = kPGPError_NoErr;
	
	err = pgpContextMemRealloc( session->pgpContext,
					(void **) &session->queuedSendData,
					session->queuedSendSize + sendDataSize, 0 ); CKERR;
	pgpCopyMemory( sendData, session->queuedSendData + session->queuedSendSize,
					sendDataSize );
	session->queuedSendSize += sendDataSize;

done:
	return err;
}

	PGPError
pgpTLSReceiveRecordLayer(
	PGPtlsSessionPriv *	session,
	PGPByte *			outType,
	PGPByte **			outBuffer,
	PGPSize *			outLength )
{
	PGPError			err	= kPGPError_NoErr;
    PGPError            rerr = kPGPError_NoErr;
	PGPByte				header[kPGPtls_RecordHeaderSize];
	PGPByte *			buffer = NULL;
	PGPSize				rcvd;
	PGPUInt16			length;
	PGPInt32			bytesRead;

	*outType	= 0;
	*outBuffer	= NULL;
	*outLength	= 0;
	
	rcvd = kPGPtls_RecordHeaderSize;
	err = pgpTLSExtractRawData( session, header, &rcvd );	CKERR;
	for(; rcvd < kPGPtls_RecordHeaderSize; rcvd += bytesRead)
	{
		bytesRead = (session->tlsReceiveProc)( session->tlsReceiveUserData,
						header + rcvd,
						(PGPInt32)( kPGPtls_RecordHeaderSize - rcvd ) );
		if( bytesRead < 0 )
		{
			if( bytesRead != kPGPError_TLSWouldBlock )
			{
				FATALTLS( kPGPError_TLSUnexpectedClose );
			}
			else
            {
                rerr = kPGPError_TLSWouldBlock;
				bytesRead = 0;
            }
		}
		if( bytesRead == 0 )
		{
			if( session->blocking )
			{
				FATALTLS( kPGPError_TLSUnexpectedClose );
			}
			else
			{
				if( rcvd )
                {
                    err = pgpTLSBufferRawData( session, header, rcvd );  CKERR;
                    if(rerr == kPGPError_TLSWouldBlock)     /* check to see if receive call returned a blocking error */
                    {
                        err = kPGPError_TLSWouldBlock;
                    }
                }
                else if(rerr == kPGPError_TLSWouldBlock)    /* check for blocking error */
                {
                    err = kPGPError_TLSWouldBlock;
                }
				goto done;
			}
		}
	}
	pgpAssert( rcvd == kPGPtls_RecordHeaderSize );
	
	switch( header[0] )
	{
		case kPGPtls_RT_ChangeCipherSpec:
		case kPGPtls_RT_Alert:
		case kPGPtls_RT_Handshake:
		case kPGPtls_RT_ApplicationData:
			break;
		default:
			/* Unknown Record Type, illegal, abort protocol */
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_UnexpectedMessage );
			FATALTLS( kPGPError_TLSProtocolViolation );
	}
	
	if( header[1] != kPGPtls_MajorVersion )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_ProtocolVersion );
		FATALTLS( kPGPError_TLSVersionUnsupported );
	}
	length = PGPEndianToUInt16( kPGPBigEndian, &header[3] );
	if( length <= 0 || length > kPGPtls_MaximumPacketSize )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_DecodeError );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	buffer = (PGPByte *) pgpContextMemAlloc(session->pgpContext, length, 0 );
	if( IsNull( buffer ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	rcvd = length;
	err = pgpTLSExtractRawData( session, buffer, &rcvd );	CKERR;
	for(; rcvd < length; rcvd += bytesRead)
	{
		bytesRead = (session->tlsReceiveProc)( session->tlsReceiveUserData,
						buffer + rcvd, (PGPInt32)( length - rcvd ) );
		if( bytesRead < 0 )
		{
			if( bytesRead != kPGPError_TLSWouldBlock )
			{
				(void)pgpContextMemFree( session->pgpContext, buffer );
				buffer = NULL;
				FATALTLS( kPGPError_TLSUnexpectedClose );
			}
			else
            {
                bytesRead = 0;
                rerr = kPGPError_TLSWouldBlock;  
            }
		}
		if( bytesRead == 0 )
		{
			if( session->blocking )
			{
				FATALTLS( kPGPError_TLSUnexpectedClose );
			}
			else
			{
				if( rcvd )
				{
					err = pgpTLSBufferRawData( session, header,
												kPGPtls_RecordHeaderSize );CKERR;
					err = pgpTLSBufferRawData( session, buffer, rcvd );	CKERR;
				}
                if(rerr == kPGPError_TLSWouldBlock)
                    err = kPGPError_TLSWouldBlock;
				(void)pgpContextMemFree( session->pgpContext, buffer );
				buffer = NULL;
				goto done;
			}
		}
	}
	pgpAssert( rcvd == length );
	
	if( IsntNull( session->readActive ) )
	{
		PGPByte		macCheck[32];
		
		if( PGPCBCContextRefIsValid( session->readCipher ) )
		{
			err = PGPCBCDecrypt( session->readCipher, buffer, length, buffer );
			CKERR;
			length -= ( buffer[length - 1] + 1 );
		}
		length -= session->hashSize;
		if( length > kPGPtls_MaximumPacketSize )
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_DecodeError );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
		err = pgpTLSCalculateMAC( session, FALSE, header[0], buffer, length,
									macCheck ); CKERR;
		if( !pgpMemoryEqual( macCheck, buffer + length, session->hashSize ) )
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_BadRecordMAC );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
	}
	
	*outType	= header[0];
	*outBuffer	= buffer;
	*outLength	= length;
done:
	if( IsPGPError( err ) && IsntNull( buffer ) )
	{
		(void)pgpContextMemFree( session->pgpContext, buffer );
		*outBuffer = NULL;
	}
	return err;
}

	PGPError
pgpTLSReceiveHandshakePacket(
	PGPtlsSessionPriv *	session,
	PGPByte *			outType,
	PGPByte **			outBuffer,
	PGPSize *			outLength )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				rcvdType;
	PGPByte *			buffer = NULL;
	PGPSize				rcvdLen = 0,
						pktLen = 0;
	
	*outType	= 0;
	*outBuffer	= NULL;
	*outLength	= 0;
	
	if( session->handDataSize > 0 )
	{
		err = pgpTLSExtractHandData( session, &buffer, &rcvdLen );	CKERR;
		rcvdType = kPGPtls_RT_Handshake;
	}
	else
	{
nextPkt:
		err = pgpTLSReceiveRecordLayer( session, &rcvdType, &buffer, &rcvdLen );
	}
	if( IsPGPError( err ) || IsNull( buffer ) )
		goto done;
	if( rcvdType == kPGPtls_RT_Alert )
	{
		err = pgpTLSProcessAlert( session, buffer, rcvdLen ); CKERR;
		(void)pgpContextMemFree( session->pgpContext, buffer );
		buffer = NULL;
		goto nextPkt;
	}
	else if( rcvdType == kPGPtls_RT_ChangeCipherSpec )
	{
		pgpTLSNextState( session, kPGPtls_EV_ReceiveChangeCipherSpec );
		if( ( rcvdLen == 1 ) && ( buffer[0] == 1 ) &&
			( session->readActive == NULL ) &&
			( session->state != kPGPtls_FatalErrorState ) &&
			( session->isClientSide ? IsntNull( session->serverPending ) :
			IsntNull( session->clientPending ) ) )
		{
			err = pgpTLSActivateReadKeys( session ); CKERR;
			(void)pgpContextMemFree( session->pgpContext, buffer );
			buffer = NULL;
			goto nextPkt;
		}
		else
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_UnexpectedMessage );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
	}
	else if( rcvdType != kPGPtls_RT_Handshake )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_UnexpectedMessage );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	err = pgpTLSExtractHandData( session, &buffer, &rcvdLen );	CKERR;
	pktLen = ( buffer[1] << 16 ) | ( buffer[2] << 8 ) | buffer[3];
	if( pktLen + 4 > rcvdLen )
	{
		/* handshake packet is fragmented */
		err = pgpTLSBufferHandData( session, buffer, rcvdLen );	CKERR;
		goto nextPkt;
	}
	if( pktLen + 4 < rcvdLen )
	{
		/* multiple handshake packets were in the record packet */
		err = pgpTLSBufferHandData( session, buffer + pktLen + 4,
									rcvdLen - pktLen - 4 );	CKERR;
	}
	if( buffer[0] != kPGPtls_HT_HelloRequest )
	{
		if( ( buffer[0] == kPGPtls_HT_Finished ) ||
			( buffer[0] == kPGPtls_HT_CertificateVerify ) )
		{
			if( PGPHashContextRefIsValid( session->lastHandSHA ) ||
				PGPHashContextRefIsValid( session->lastHandMD5 ) )
			{
				(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
											kPGPtls_AT_UnexpectedMessage );
				FATALTLS( kPGPError_TLSProtocolViolation );
			}
			err = PGPCopyHashContext( session->handshakeSHA,
										&session->lastHandSHA);	CKERR;
			err = PGPCopyHashContext( session->handshakeMD5,
										&session->lastHandMD5);	CKERR;
		}
		(void)PGPContinueHash( session->handshakeMD5, buffer, pktLen + 4 );
		(void)PGPContinueHash( session->handshakeSHA, buffer, pktLen + 4 );
	}

	*outType = buffer[0];
	*outBuffer = buffer;
	*outLength = pktLen;
	pgpCopyMemory( buffer + 4, buffer, pktLen );
done:
	if( IsPGPError( err ) && IsntNull( buffer ) )
		(void)pgpContextMemFree( session->pgpContext, buffer );
	return err;
}

	PGPError
pgpTLSProcessAlert(
	PGPtlsSessionPriv *	session,
	PGPByte *			inBuffer,
	PGPSize				inLength )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				alertType;
	PGPByte				alertLevel;
	
	if( inLength != 2 )
		FATALTLS( kPGPError_TLSProtocolViolation );
	alertLevel = inBuffer[0];
	alertType = inBuffer[1];
	
	/* Treat all alerts as fatal except close_notify and no_certificate */
	if( alertType == kPGPtls_AT_CloseNotify )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_WarningAlert,
									kPGPtls_AT_CloseNotify );
		session->state = kPGPtls_ClosedState;
	}
	else if( alertType == kPGPtls_AT_NoCertificate )
	{
		pgpTLSNextState( session, kPGPtls_EV_ReceiveNoCertificate );
		if( !session->isClientSide )
			session->certRequested = FALSE;
	}
	else
	{
		session->fatalAlert = (PGPtlsAlert)alertType;
#ifdef PGPTLS_DEBUG
		pgpAssert( 0 );
#endif
		pgpTLSNextState( session, kPGPtls_EV_ReceiveFatalAlert );
	}
	if( session->state == kPGPtls_FatalErrorState )
		err = kPGPError_TLSAlertReceived;
	
done:
	return err;
}

	PGPError
pgpTLSSendRecordLayer(
	PGPtlsSessionPriv *	session,
	const PGPByte		inType,
	const PGPByte *		inBuffer,
	const PGPSize		inLength )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte *			buffer = NULL;
	PGPSize				bufLen = 0,
						payloadLen = 0,
						padLen = 0;
	PGPInt32			serr;
	
	PGPValidatePtr( inBuffer );
	if( inLength > kPGPtls_MaximumPacketSize )
	{
		err = kPGPError_BufferTooSmall;
		goto done;				/* non-fatal */
	}
	pgpAssert(	( inType == kPGPtls_RT_ChangeCipherSpec )	||
				( inType == kPGPtls_RT_Alert )				||
				( inType == kPGPtls_RT_Handshake )			||
				( inType == kPGPtls_RT_ApplicationData ) );

	if( IsntNull( session->writeActive ) )
	{
		if( PGPCBCContextRefIsValid( session->writeCipher ) )
		{
			padLen = (kPGPtls_CipherBlockSize -
				((inLength + session->hashSize + 1) %
					kPGPtls_CipherBlockSize ) );
			if( padLen == kPGPtls_CipherBlockSize )
				padLen = 0;
			bufLen += padLen + 1;
		}
		bufLen += session->hashSize;
	}
	bufLen += kPGPtls_RecordHeaderSize + inLength;
	payloadLen = bufLen - kPGPtls_RecordHeaderSize;
	buffer  = PGPNewData( session->memMgr, bufLen, kPGPMemoryMgrFlags_Clear );
	if( IsNull( buffer ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	buffer[0] = inType;
	buffer[1] = kPGPtls_MajorVersion;
	buffer[2] = session->minorVersion;
	PGPUInt16ToEndian( (PGPUInt16) payloadLen, kPGPBigEndian, &buffer[3] );
	pgpCopyMemory( inBuffer, &buffer[kPGPtls_RecordHeaderSize], inLength );
	if( IsntNull( session->writeActive ) )
	{
		err = pgpTLSCalculateMAC( session, TRUE, inType, inBuffer, inLength,
					buffer + inLength + kPGPtls_RecordHeaderSize ); CKERR;
		if( PGPCBCContextRefIsValid( session->writeCipher ) )
		{
			if( padLen )
				pgpFillMemory( buffer + inLength + session->hashSize +
								kPGPtls_RecordHeaderSize, padLen + 1, padLen );
			err = PGPCBCEncrypt( session->writeCipher,
								buffer + kPGPtls_RecordHeaderSize, payloadLen,
								buffer + kPGPtls_RecordHeaderSize );CKERR;
		}
	}
	
	err = pgpTLSSendQueueIdleInternal( session );
	if( err == kPGPError_TLSWouldBlock )
		serr = err;
	
	if( IsntPGPError( err ) && ( bufLen > 0 ) )
	{
		if( serr != kPGPError_TLSWouldBlock )
			serr = (session->tlsSendProc)( session->tlsSendUserData,
										buffer, (PGPInt32) bufLen );
		if( serr == kPGPError_TLSWouldBlock )
		{
			err = pgpTLSBufferSendData( session, buffer, bufLen ); CKERR;
			err = kPGPError_TLSWouldBlock;
		}
		else if( (PGPSize)serr != bufLen )
			FATALTLS( kPGPError_TLSUnexpectedClose );
	}
done:
	if( IsntNull( buffer ) )
		(void)PGPFreeData( buffer );
	return err;
}

	PGPError
pgpTLSSendHandshakePacket(
	PGPtlsSessionPriv *	session,
	const PGPByte		inType,
	const PGPByte *		inBuffer,
	const PGPSize		inLength )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte *			buffer = NULL;
	PGPByte				prefix[4];
	PGPUInt16			pktLen;
	
	pktLen = inLength + sizeof(prefix);
	prefix[0] = inType;
	prefix[1] = 0;	/* 24 bit length first byte */
	PGPUInt16ToEndian( (PGPUInt16) inLength, kPGPBigEndian, &prefix[2] );
	
	buffer = (PGPByte *) pgpContextMemAlloc( session->pgpContext, pktLen, 0 );
	if( IsNull( buffer ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	pgpCopyMemory( prefix, buffer, sizeof(prefix) );
	if( IsntNull( inBuffer ) )
		pgpCopyMemory( inBuffer, buffer + sizeof(prefix), inLength );
	
	if( inType != kPGPtls_HT_HelloRequest )
	{
		(void)PGPContinueHash(session->handshakeMD5, buffer, pktLen);
		(void)PGPContinueHash(session->handshakeSHA, buffer, pktLen);
	}

	err = pgpTLSSendRecordLayer( session, kPGPtls_RT_Handshake,
									buffer, pktLen ); CKERR;	
done:
	if( IsntNull( buffer ) )
		(void)pgpContextMemFree( session->pgpContext, buffer );
	return err;
}

	PGPError
pgpTLSSendAlert(
	PGPtlsSessionPriv *	session,
	PGPByte				level,
	PGPtlsAlert			type )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				buffer[32];
	PGPSize				pktLen = 0;
	
	buffer[0] = level;
	buffer[1] = type;
	pktLen = 2;
	err = pgpTLSSendRecordLayer( session, kPGPtls_RT_Alert, buffer, pktLen );
	if( level == kPGPtls_AL_FatalAlert )
	{
		session->state		= kPGPtls_FatalErrorState;
		session->fatalAlert	= type;
	}
	return err;
}

	PGPError
pgpTLSSendClientHello(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				buffer[384];
	PGPUInt16			suiteIndex,
						pktLen = 0;
	
	/* client_version */
	buffer[pktLen++] = kPGPtls_MajorVersion;
#ifdef PGPTLS_FORCESSL3
	buffer[pktLen++] = kPGPtls_SSL3MinorVersion;
#else
	buffer[pktLen++] = kPGPtls_MinorVersion;	/* always latest version */
#endif
	/* random */
	*((PGPUInt32 *)&session->cRandom[0]) = PGPGetTime();
	err = PGPContextGetRandomBytes( session->pgpContext,
									&session->cRandom[4],
									kPGPtls_RandomSeedSize ); CKERR;
	pgpCopyMemory( session->cRandom, &buffer[pktLen], kPGPtls_RandomSize );
	pktLen += kPGPtls_RandomSize;
	/* session ID ##### */
	buffer[pktLen++] = 0;
	/* cipher_suites */
	PGPUInt16ToEndian( (PGPUInt16)( session->numCipherSuites * 2 ),
						kPGPBigEndian, &buffer[pktLen] );
	pktLen += sizeof(PGPUInt16);
	for( suiteIndex = 0; suiteIndex < session->numCipherSuites; suiteIndex++ )
	{
		buffer[pktLen++] = session->cipherSuites[suiteIndex].id[0];
		buffer[pktLen++] = session->cipherSuites[suiteIndex].id[1];
	}
	/* compression_methods */
	buffer[pktLen++] = 1;
	buffer[pktLen++] = kPGPtls_CT_None;
	
	pgpAssert( pktLen < sizeof(buffer) );
	
	err = pgpTLSSendHandshakePacket( session, kPGPtls_HT_ClientHello,
										buffer, pktLen );
done:
	return err;
}

	PGPError
pgpTLSSendServerHello(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				buffer[128];
	PGPUInt16			pktLen = 0;
	
	/* server_version */
	buffer[pktLen++] = kPGPtls_MajorVersion;
	buffer[pktLen++] = session->minorVersion;
	/* random */
	*((PGPUInt32 *)&session->sRandom[0]) = PGPGetTime();
	err = PGPContextGetRandomBytes( session->pgpContext,
									&session->sRandom[4],
									kPGPtls_RandomSeedSize ); CKERR;
	pgpCopyMemory( session->sRandom, &buffer[pktLen], kPGPtls_RandomSize );
	pktLen += kPGPtls_RandomSize;
	/* session_id ##### */
	buffer[pktLen++] = 0;
	/* cipher_suite */
	buffer[pktLen++] = session->cipherSuites[session->cipherSuiteIndex].id[0];
	buffer[pktLen++] = session->cipherSuites[session->cipherSuiteIndex].id[1];
	/* compression_method */
	buffer[pktLen++] = kPGPtls_CT_None;
	
	pgpAssert( pktLen < sizeof(buffer) );
	
	err = pgpTLSSendHandshakePacket( session, kPGPtls_HT_ServerHello,
										buffer, pktLen );
done:
	return err;
}

	PGPError
pgpTLSSendCertificate(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte *			buffer = NULL;
	PGPUInt16			pktLen = 0;
	PGPByte *			keyBuffer = NULL;
	PGPSize				keyBufferLen;
	PGPKeySetRef		localKeySet = kInvalidPGPKeySetRef;
	
	if( session->isClientSide && !session->certMatched )
	{
		if( session->minorVersion == kPGPtls_SSL3MinorVersion )
		{
			err = pgpTLSSendAlert( session,	kPGPtls_AL_WarningAlert,
									kPGPtls_AT_NoCertificate ); CKERR;
			goto done;
		}
		else if( !session->cipherSuites[session->cipherSuiteIndex].usePGPKeys )
		{
			buffer = PGPNewData( session->memMgr, 3, 0 );
			if( IsNull( buffer ) )
			{
				err = kPGPError_OutOfMemory;
				goto done;
			}
			buffer[pktLen++] = 0;
			buffer[pktLen++] = 0;
			buffer[pktLen++] = 0;
		}
	}
	else if( session->cipherSuites[session->cipherSuiteIndex].usePGPKeys )
	{
		pgpAssert( PGPKeyRefIsValid( session->localKey ) );
		
		err = PGPNewSingletonKeySet( session->localKey, &localKeySet );CKERR;
		err = PGPExportKeySet( localKeySet,
			PGPOAllocatedOutputBuffer( session->pgpContext,
				(void **) &keyBuffer, MAX_PGPUInt32, &keyBufferLen ),
			PGPOArmorOutput( session->pgpContext, FALSE ),
			PGPOLastOption( session->pgpContext ));	CKERR;
		buffer = PGPNewData( session->memMgr, keyBufferLen + 4, 0 );
		if( IsNull( buffer ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		buffer[0] = ( keyBufferLen >> 16 ) & 0xFF;
		buffer[1] = ( keyBufferLen >> 8 ) & 0xFF;
		buffer[2] = keyBufferLen & 0xFF;
		pgpCopyMemory( keyBuffer, &buffer[3], keyBufferLen );
		pktLen = keyBufferLen + 3;
	}
	else
	{
		/* ##### */
		pgpAssert( 0 );
	}
	err = pgpTLSSendHandshakePacket( session, kPGPtls_HT_Certificate,
										buffer, pktLen );
done:
	if( IsntNull( buffer ) )
		(void)PGPFreeData( buffer );
	if( IsntNull( keyBuffer ) )
		(void)PGPFreeData( keyBuffer );
	if( PGPKeySetRefIsValid( localKeySet ) )
		PGPFreeKeySet( localKeySet );
	return err;
}

	PGPError
pgpTLSSendCertVerify(
	PGPtlsSessionPriv *		session )
{
	PGPError				err	= kPGPError_NoErr;
	PGPByte					buffer[640],
							sigData[640];
	PGPSize					pktLen = 0,
							sigDataLen;
	PGPPrivateKeyContextRef	privKeyCon = kInvalidPGPPrivateKeyContextRef;
	PGPSize					sigSize,
							actSigSize;
	
	err = PGPNewPrivateKeyContext( session->localKey,
		kPGPPublicKeyMessageFormat_PKCS1,
		&privKeyCon,
		session->localKeyUsePasskey ?
		PGPOPasskeyBuffer( session->pgpContext, session->localKeyPasskeyBuffer,
							session->localKeyPasskeySize ) :
		PGPOPassphrase( session->pgpContext, session->localKeyPassphrase ),
		PGPOLastOption( session->pgpContext ) );	CKERR;
	err = PGPGetPrivateKeyOperationSizes( privKeyCon, NULL, NULL, &sigSize );
											CKERR;

	err = pgpTLSCalculateCertVerify( session, sigData, &sigDataLen ); CKERR;
	pgpAssert( sigDataLen < sizeof( sigData ) );
	/* Create the Signature */
	PGPUInt16ToEndian( (PGPUInt16) sigSize, kPGPBigEndian, &buffer[pktLen] );
	pktLen += sizeof(PGPUInt16);
	err = PGPPrivateKeySignRaw( privKeyCon, sigData, sigDataLen,
				buffer + pktLen, &actSigSize ); CKERR;
	pgpAssert( actSigSize == sigSize );
	pktLen += sigSize;
	pgpAssert( pktLen < sizeof(buffer) );
	err = pgpTLSSendHandshakePacket( session, kPGPtls_HT_CertificateVerify,
										buffer, pktLen );
done:
	if( PGPPrivateKeyContextRefIsValid( privKeyCon ) )
		(void)PGPFreePrivateKeyContext( privKeyCon );
	return err;
}

	PGPError
pgpTLSSendCertificateRequest(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				buffer[512];
	PGPUInt16			pktLen = 0;
	
	if( session->cipherSuites[session->cipherSuiteIndex].usePGPKeys )
	{
		/* allowed ClientCertificateTypes */
		buffer[pktLen++] = 2;
		buffer[pktLen++] = kPGPtls_CC_DSASign;
		buffer[pktLen++] = kPGPtls_CC_RSASign;
		/* that's all we need with OpenPGP keys */
	}
	else
	{
		/* ##### */
		pgpAssert( 0 );
		err = kPGPError_FeatureNotAvailable;
	}
	pgpAssert( pktLen < sizeof(buffer) );
	
	if( IsntPGPError( err ) )
	{
		err = pgpTLSSendHandshakePacket(	session,
											kPGPtls_HT_CertificateRequest,
											buffer, pktLen );
		session->certRequested = TRUE;
	}
	
	return err;
}

	PGPError
pgpTLSSendServerKeyExchange(
	PGPtlsSessionPriv *		session )
{
	PGPError				err	= kPGPError_NoErr;
	PGPByte *				buffer = NULL;
	PGPUInt16				pktLen = 0;
	PGPByte *				secretX = NULL;
	PGPPrivateKeyContextRef	privKeyCon = kInvalidPGPPrivateKeyContextRef;
	PGPSize					sigSize,
							actSigSize;
	PGPHashContextRef		skeHashSHA = kInvalidPGPHashContextRef,
							skeHashMD5 = kInvalidPGPHashContextRef;
	DHPrime *				prime;
	PGPByte const			dhGenerator[1] = { 2 };
	PGPUInt32				modBits,
							modBytes,
							expBytes = 0,
							expBits,
							pSize,
							gSize,
							ySSize;
	
	err = PGPNewPrivateKeyContext( session->localKey,
		kPGPPublicKeyMessageFormat_PKCS1,
		&privKeyCon,
		session->localKeyUsePasskey ?
		PGPOPasskeyBuffer( session->pgpContext, session->localKeyPasskeyBuffer,
							session->localKeyPasskeySize ) :
		PGPOPassphrase( session->pgpContext, session->localKeyPassphrase ),
		PGPOLastOption( session->pgpContext ) );	CKERR;
	err = PGPGetPrivateKeyOperationSizes( privKeyCon, NULL, NULL, &sigSize );
	CKERR;
	
	switch( session->requestedPrime )
	{
		case kPGPtls_DHPrime1024:
			prime = &DH_1024bitPrime;
			break;
		case kPGPtls_DHPrime1536:
			prime = &DH_1536bitPrime;
			break;
		default:
		case kPGPtls_DHPrime2048:
			prime = &DH_2048bitPrime;
			break;
		case kPGPtls_DHPrime3072:
			prime = &DH_3072bitPrime;
			break;
		case kPGPtls_DHPrime4096:
			prime = &DH_4096bitPrime;
			break;
	}
	/* Load up the prime modulus */
	err = PGPNewBigNum( session->memMgr, TRUE, &session->dhP ); CKERR;
	err =  PGPBigNumInsertBigEndianBytes(	session->dhP,
											prime->prime,
											0, prime->length ); CKERR;
	/* We always use a generator of 2 for speed */
	err = PGPNewBigNum( session->memMgr, TRUE, &session->dhG ); CKERR;
	err =  PGPBigNumInsertBigEndianBytes(	session->dhG,
											dhGenerator,
											0, sizeof(dhGenerator) ); CKERR;
	modBits	 = PGPBigNumGetSignificantBits( session->dhP );
	modBytes = ( modBits + 7 ) / 8;
	(void)PGPDiscreteLogExponentBits(modBits, &expBytes);
	expBytes = ( expBytes * 3 / 2 + 7 ) / 8;
	expBits  = 8 * expBytes;
	secretX  = PGPNewSecureData( session->memMgr, expBytes, 0 );
	if( IsNull( secretX ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	/* Generate the secret random X value */
	err = PGPContextGetRandomBytes( session->pgpContext, secretX, expBytes );
	CKERR;
	secretX[0] |= 0x80;
	err = PGPNewBigNum( session->memMgr, TRUE, &session->dhX ); CKERR;
	err = PGPBigNumInsertBigEndianBytes(session->dhX, secretX, 0, expBytes );
	CKERR;
	/* Generate our server Y value */
	err = PGPNewBigNum( session->memMgr, TRUE, &session->dhYs ); CKERR;
	err = PGPBigNumExpMod( session->dhG, session->dhX, session->dhP,
							session->dhYs ); CKERR;
	pSize = ( ( PGPBigNumGetSignificantBits( session->dhP ) + 7 ) / 8 );
	gSize = ( ( PGPBigNumGetSignificantBits( session->dhG ) + 7 ) / 8 );
	ySSize = ( ( PGPBigNumGetSignificantBits( session->dhYs ) + 7 ) / 8 );
	buffer  = PGPNewData( session->memMgr,
							pSize + gSize + ySSize + 8 + sigSize, 0 );
	if( IsNull( buffer ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	/* Write out p */
	PGPUInt16ToEndian( (PGPUInt16) pSize, kPGPBigEndian, &buffer[pktLen] );
	pktLen += sizeof(PGPUInt16);
	err = PGPBigNumExtractBigEndianBytes( session->dhP,
									&buffer[pktLen],
									0, pSize ); CKERR;
	pktLen += pSize;
	/* Write out g */
	PGPUInt16ToEndian( (PGPUInt16) gSize, kPGPBigEndian, &buffer[pktLen] );
	pktLen += sizeof(PGPUInt16);
	err = PGPBigNumExtractBigEndianBytes( session->dhG,
									&buffer[pktLen],
									0, gSize ); CKERR;
	pktLen += gSize;
	/* Write out yS */
	PGPUInt16ToEndian( (PGPUInt16) ySSize, kPGPBigEndian, &buffer[pktLen] );
	pktLen += sizeof(PGPUInt16);
	err = PGPBigNumExtractBigEndianBytes( session->dhYs,
									&buffer[pktLen],
									0, ySSize ); CKERR;
	pktLen += ySSize;
	err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_SHA,
						&skeHashSHA ); CKERR;
	(void)PGPContinueHash( skeHashSHA, session->cRandom, kPGPtls_RandomSize);
	(void)PGPContinueHash( skeHashSHA, session->sRandom, kPGPtls_RandomSize);
	(void)PGPContinueHash( skeHashSHA, buffer, pktLen);
	switch( session->cipherSuites[session->cipherSuiteIndex].sigAlg )
	{
		case kPGPPublicKeyAlgorithm_DSA:
		{
			PGPByte		signedData[ kPGPtls_HA_SHASize ];
			
			/* Create the Signature */			
			(void)PGPFinalizeHash( skeHashSHA, signedData );
			PGPUInt16ToEndian( (PGPUInt16) sigSize, kPGPBigEndian,
								&buffer[pktLen] );
			pktLen += sizeof(PGPUInt16);
			err = PGPPrivateKeySignRaw( privKeyCon, signedData,
									kPGPtls_HA_SHASize, buffer + pktLen,
									&actSigSize ); CKERR;
			break;
		}
		case kPGPPublicKeyAlgorithm_RSA:
		{
			PGPByte		signedData[kPGPtls_HA_MD5Size + kPGPtls_HA_SHASize];
			
			/* Do the additional MD5 hash for RSA */
			err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_MD5,
								&skeHashMD5 ); CKERR;
			(void)PGPContinueHash( skeHashMD5, session->cRandom,
									kPGPtls_RandomSize);
			(void)PGPContinueHash( skeHashMD5, session->sRandom,
									kPGPtls_RandomSize);
			(void)PGPContinueHash( skeHashMD5, buffer, pktLen);
			(void)PGPFinalizeHash( skeHashMD5, signedData );
			(void)PGPFinalizeHash( skeHashSHA, signedData +
						kPGPtls_HA_MD5Size );
			
			/* Create the Signature */			
			PGPUInt16ToEndian( (PGPUInt16) sigSize, kPGPBigEndian,
								&buffer[pktLen] );
			pktLen += sizeof(PGPUInt16);
			err = PGPPrivateKeySignRaw( privKeyCon, signedData,
									kPGPtls_HA_MD5Size + kPGPtls_HA_SHASize,
									buffer + pktLen, &actSigSize ); CKERR;
			break;
		}
		
		default:
			break;
	}
	pgpAssert( actSigSize == sigSize );
	pktLen += sigSize;
	
	err = pgpTLSSendHandshakePacket( session, kPGPtls_HT_ServerKeyExchange,
										buffer, pktLen );
done:
	if( IsntNull( secretX ) )
		(void)PGPFreeData( secretX );
	if( IsntNull( buffer ) )
		(void)PGPFreeData( buffer );
	if( PGPPrivateKeyContextRefIsValid( privKeyCon ) )
		(void)PGPFreePrivateKeyContext( privKeyCon );
	if( PGPHashContextRefIsValid( skeHashMD5 ) )
		(void)PGPFreeHashContext( skeHashMD5 );
	if( PGPHashContextRefIsValid( skeHashSHA ) )
		(void)PGPFreeHashContext( skeHashSHA );
	return err;
}

	PGPError
pgpTLSSendClientKeyExchange(
	PGPtlsSessionPriv *		session )
{
	PGPError				err	= kPGPError_NoErr;
	PGPByte					buffer[1024];
	PGPUInt16				pktLen = 0,
							preMasterSize = 0;
	PGPByte *				secretX = NULL;
	PGPByte *				preMaster = NULL;
	PGPBigNumRef			dhSecret = kPGPInvalidBigNumRef;
	PGPPublicKeyContextRef	pubKeyCon = kInvalidPGPPublicKeyContextRef;
	
	if( session->cipherSuites[session->cipherSuiteIndex].ephemeral )
	{
		PGPUInt32		modBits,
						modBytes,
						expBytes = 0,
						expBits;
		
		/* Send ClientDiffieHellmanPublic */
		
		modBits	 = PGPBigNumGetSignificantBits( session->dhP );
		modBytes = ( modBits + 7 ) / 8;
		(void)PGPDiscreteLogExponentBits(modBits, &expBytes);
		expBytes = ( expBytes * 3 / 2 + 7 ) / 8;
		expBits  = 8 * expBytes;
		secretX  = PGPNewSecureData( session->memMgr, expBytes, 0 );
		if( IsNull( secretX ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		err = PGPContextGetRandomBytes( session->pgpContext, secretX,
					expBytes );
		CKERR;
		secretX[0] |= 0x80;
		err = PGPNewBigNum( session->memMgr, TRUE, &session->dhX ); CKERR;
		err = PGPBigNumInsertBigEndianBytes(session->dhX, secretX, 0,
					expBytes );
		CKERR;
		err = PGPNewBigNum( session->memMgr, TRUE, &session->dhYc ); CKERR;
		err = PGPBigNumExpMod( session->dhG, session->dhX, session->dhP,
								session->dhYc ); CKERR;
		/* Setup outgoing packet, sending client Y with 2 byte length */
		PGPUInt16ToEndian( (PGPUInt16) modBytes, kPGPBigEndian, buffer );
		err = PGPBigNumExtractBigEndianBytes( session->dhYc,
										buffer + sizeof(PGPUInt16),
										0, modBytes ); CKERR;
		pktLen = modBytes + sizeof(PGPUInt16);
		
		/* Calculate the shared secret */
		err = PGPNewBigNum( session->memMgr, TRUE, &dhSecret ); CKERR;
		err = PGPBigNumExpMod( session->dhYs, session->dhX, session->dhP,
								dhSecret ); CKERR;
		
		/* Load premastersecret with the shared secret */
		preMasterSize = modBytes;
		preMaster = PGPNewSecureData( session->memMgr, preMasterSize, 0 );
		if( IsNull( preMaster ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		(void)PGPBigNumExtractBigEndianBytes( dhSecret, preMaster,
				0, modBytes );
	}
	else
	{
		PGPSize					encSize;
		
		/* Send EncryptedPreMasterSecret */
		err = PGPNewPublicKeyContext(	session->remoteKey,
										kPGPPublicKeyMessageFormat_PKCS1,
										&pubKeyCon ); CKERR;
		err = PGPGetPublicKeyOperationSizes( pubKeyCon, NULL,
											&encSize, NULL );CKERR;
		preMasterSize = kPGPtls_MasterSecretSize;
		preMaster = PGPNewSecureData( session->memMgr, preMasterSize, 0 );
		if( IsNull( preMaster ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		preMaster[0] = kPGPtls_MajorVersion;
#ifdef PGPTLS_FORCESSL3
		preMaster[1] = kPGPtls_SSL3MinorVersion;
#else
		preMaster[1] = kPGPtls_MinorVersion;	/* always latest version */
#endif
		err = PGPContextGetRandomBytes( session->pgpContext,
									&preMaster[2],
									kPGPtls_MasterSecretSize - 2 ); CKERR;
		if( session->minorVersion > kPGPtls_SSL3MinorVersion )
		{
			PGPUInt16ToEndian( (PGPUInt16) encSize, kPGPBigEndian, buffer );
			pktLen += sizeof(PGPUInt16);
		}
		err = PGPPublicKeyEncrypt( pubKeyCon, preMaster, preMasterSize,
									buffer + pktLen, &encSize ); CKERR;
		pktLen += encSize;
	}
	pgpAssert( pktLen < sizeof(buffer) );
	
	err = pgpTLSSendHandshakePacket( session, kPGPtls_HT_ClientKeyExchange,
										buffer, pktLen );

	/* Calculate the Master Secret */
	err = pgpTLSCalculateMasterSecret( session, preMaster, preMasterSize );
	
done:
	if( IsntNull( secretX ) )
		(void)PGPFreeData( secretX );
	if( IsntNull( preMaster ) )
		(void)PGPFreeData( preMaster );
	if( dhSecret != kPGPInvalidBigNumRef )
		(void)PGPFreeBigNum( dhSecret );
	if( PGPPublicKeyContextRefIsValid( pubKeyCon ) )
		(void)PGPFreePublicKeyContext( pubKeyCon );
	return err;
}

	PGPError
pgpTLSSendFinished(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				buffer[128];
	PGPSize				pktLen = 0;

	err = pgpTLSCalculateFinished( session, session->isClientSide,
									buffer, &pktLen ); CKERR;
	err = pgpTLSSendHandshakePacket( session, kPGPtls_HT_Finished,
										buffer, pktLen );
done:
	return err;
}

	PGPError
pgpTLSSendFinalHandshake( 
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	
	/* Change the cipher spec */
	err = pgpTLSSendChangeCipherSpec( session ); CKERR;
	
	/* Activate the pending client keys */
	err = pgpTLSActivateWriteKeys( session ); CKERR;
	
	/* Signal that our side of the handshake is complete */
	err = pgpTLSSendFinished( session ); CKERR;
	
done:
	return err;
}

	PGPError
pgpTLSSendChangeCipherSpec(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				buffer[16];
	PGPUInt16			pktLen = 0;
	
	buffer[0]	= 1;
	pktLen		= 1;
	
	pgpAssert( pktLen < sizeof(buffer) );
	
	err = pgpTLSSendRecordLayer( session, kPGPtls_RT_ChangeCipherSpec,
									buffer, pktLen );
	return err;
}

	PGPError
pgpTLSReceiveServerHello(
	PGPtlsSessionPriv *	session,
	PGPByte *			inBuffer,
	const PGPSize		inLength )
{
	PGPError			err	= kPGPError_NoErr;
	PGPUInt16			pktInx = 0,
						suiteIndex;
	PGPtlsCipherSuiteID	cipherID;
	Boolean				foundSuite = FALSE;
	
	PGPValidatePtr( inBuffer );
	pgpAssert( inLength >= 38 );
	
	if( inLength < 38 )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_DecodeError );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	/* server_version */
	if( ( inBuffer[pktInx++] != kPGPtls_MajorVersion ) ||
		( inBuffer[pktInx] != kPGPtls_MinorVersion &&
		inBuffer[pktInx] != kPGPtls_SSL3MinorVersion ) )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_ProtocolVersion );
		FATALTLS( kPGPError_TLSVersionUnsupported );
	}
	session->minorVersion = inBuffer[pktInx++];
	/* server_random */
	pgpCopyMemory( &inBuffer[pktInx], session->sRandom,
					kPGPtls_RandomSize );
	pktInx += kPGPtls_RandomSize;
	/* session_id */
	pktInx += inBuffer[pktInx] + 1; /* skip for now ##### */
	/* cipher_suite */
	cipherID[0] = inBuffer[pktInx++];
	cipherID[1] = inBuffer[pktInx++];
	
	for( suiteIndex = 0; suiteIndex < session->numCipherSuites; suiteIndex++ )
	{
		if( ( cipherID[0] == session->cipherSuites[suiteIndex].id[0] ) &&
			( cipherID[1] == session->cipherSuites[suiteIndex].id[1] ) )
		{
			session->cipherSuiteIndex = suiteIndex;
			foundSuite = TRUE;
			break;
		}
	}
	if( !foundSuite )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_InsufficientSecurity );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	/* compression_method */
	if( inBuffer[pktInx++] != kPGPtls_CT_None )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_DecodeError );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	/*	do not verify that we have handled all data in this packet,
		this packet is allowed to contain data we do not understand. */
done:
	return err;
}	

	PGPError
pgpTLSReceiveClientHello(
	PGPtlsSessionPriv *	session,
	PGPByte *			inBuffer,
	const PGPSize		inLength )
{
	PGPError			err	= kPGPError_NoErr;
	PGPUInt16			pktInx = 0,
						suiteIndex,
						numSuites,
						intSuiteIndex;
	PGPtlsCipherSuiteID	cipherID;
	Boolean				foundSuite = FALSE;
	
	PGPValidatePtr( inBuffer );
	pgpAssert( inLength >= 40 );
	
	if( inLength < 40 )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_DecodeError );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	
	/* client_version */
	if( inBuffer[pktInx++] != kPGPtls_MajorVersion )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_ProtocolVersion );
		FATALTLS( kPGPError_TLSVersionUnsupported );
	}
#ifdef PGPTLS_FORCESSL3
	if( inBuffer[pktInx] > kPGPtls_SSL3MinorVersion )
		session->minorVersion = kPGPtls_SSL3MinorVersion;
#else
	if( inBuffer[pktInx] > kPGPtls_MinorVersion )
		session->minorVersion = kPGPtls_MinorVersion;
#endif
	else
		session->minorVersion = inBuffer[pktInx];
	session->originalClientVersion = inBuffer[pktInx];
	pktInx++;
	/* client_random */
	pgpCopyMemory( &inBuffer[pktInx], session->cRandom,
					kPGPtls_RandomSize );
	pktInx += kPGPtls_RandomSize;
	/* session_id */
	pktInx += inBuffer[pktInx] + 1; /* skip for now ##### */
	/* cipher_suites */
	numSuites = PGPEndianToUInt16( kPGPBigEndian, &inBuffer[pktInx] ) / 2;
	pktInx += sizeof(PGPUInt16);
	if( pktInx + numSuites * sizeof(PGPUInt16) >= inLength )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_DecodeError );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	for( suiteIndex = 0; suiteIndex < numSuites; suiteIndex++ )
	{
		cipherID[0] = inBuffer[pktInx++];
		cipherID[1] = inBuffer[pktInx++];
		if( foundSuite )
			continue;
		for( intSuiteIndex = 0; intSuiteIndex < session->numCipherSuites;
				intSuiteIndex++ )
		{
			if( ( cipherID[0] == session->cipherSuites[intSuiteIndex].id[0] ) &&
			 	( cipherID[1] == session->cipherSuites[intSuiteIndex].id[1] ) )
			{
				if( session->localKeyAlg ==
					session->cipherSuites[intSuiteIndex].sigAlg )
				{
					session->cipherSuiteIndex = intSuiteIndex;
					foundSuite = TRUE;
				}
			}
		}
	}
	if( !foundSuite )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_InsufficientSecurity );
		FATALTLS( kPGPError_TLSNoCommonCipher );
	}
	/* compression_methods */
	/* Nobody actually supports this, it's only in the spec for looks */
	pktInx += inBuffer[pktInx];
	
	/*	do not verify that we have handled all data in this packet,
		this packet is allowed to contain data we do not understand. */
	
	err = pgpTLSSendServerHello( session );	CKERR;
	err = pgpTLSSendCertificate( session );	CKERR;
	if( session->cipherSuites[session->cipherSuiteIndex].ephemeral )
	{
		err = pgpTLSSendServerKeyExchange( session );	CKERR;
	}
	if( session->requestCert )
	{
		err = pgpTLSSendCertificateRequest( session );	CKERR;
	}
	err = pgpTLSSendHandshakePacket( session, kPGPtls_HT_ServerHelloDone,
										NULL, 0 ); CKERR;
done:
	return err;
}	

	PGPError
pgpTLSReceiveClientKeyExchange(
	PGPtlsSessionPriv *		session,
	PGPByte *				inBuffer,
	const PGPSize			inLength )
{
	PGPError				err	= kPGPError_NoErr;
	PGPUInt16				pktInx = 0;
	PGPSize					preMasterSize = 0;
	PGPBigNumRef			dhSecret = kPGPInvalidBigNumRef;
	PGPByte *				preMaster = NULL;
	PGPPrivateKeyContextRef	privKeyCon = kInvalidPGPPrivateKeyContextRef;
	
	PGPValidatePtr( inBuffer );
	
	if( session->cipherSuites[session->cipherSuiteIndex].ephemeral )
	{
		PGPUInt32		modBytes,

		varLen = PGPEndianToUInt16( kPGPBigEndian, &inBuffer[pktInx] );
		pktInx += sizeof(PGPUInt16);
		err = PGPNewBigNum( session->memMgr, TRUE, &session->dhYc ); CKERR;
		err = PGPBigNumInsertBigEndianBytes(session->dhYc, &inBuffer[pktInx],
											0, varLen ); CKERR;
		pktInx += varLen;
		
		if( pktInx != inLength )
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_UnexpectedMessage );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}

		/* Calculate the shared secret */
		err = PGPNewBigNum( session->memMgr, TRUE, &dhSecret ); CKERR;
		err = PGPBigNumExpMod( session->dhYc, session->dhX, session->dhP,
								dhSecret ); CKERR;
		/* Load premastersecret with the shared secret */
		modBytes = ( PGPBigNumGetSignificantBits( session->dhYc ) + 7 ) / 8;
		preMasterSize = modBytes;
		preMaster = PGPNewSecureData( session->memMgr, preMasterSize, 0 );
		if( IsNull( preMaster ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		(void)PGPBigNumExtractBigEndianBytes( dhSecret, preMaster,
					0, modBytes );
	}
	else
	{
		PGPSize		decSize,
					encSize;
		
		err = PGPNewPrivateKeyContext( session->localKey,
			kPGPPublicKeyMessageFormat_PKCS1,
			&privKeyCon,
			session->localKeyUsePasskey ?
			PGPOPasskeyBuffer( session->pgpContext,
								session->localKeyPasskeyBuffer,
								session->localKeyPasskeySize ) :
			PGPOPassphrase( session->pgpContext, session->localKeyPassphrase ),
			PGPOLastOption( session->pgpContext ) );	CKERR;
		err = PGPGetPrivateKeyOperationSizes( privKeyCon, &decSize, &encSize,
												NULL ); CKERR;
		if( inLength != encSize )
		{
			if( inLength == encSize + sizeof(PGPUInt16) )
			{
				/* The TLS specification doesn't specify what to do here, whether
					a length should be used or not, but it appears that TLS
					implementations based on mailing list discussions have changed
					the protocol to put a length here.  Since this is thoroughly
					irrelevant to anything, we always read both for now. */
				if( encSize != PGPEndianToUInt16( kPGPBigEndian, &inBuffer[pktInx] ) )
				{
					(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
												kPGPtls_AT_DecryptionFailed );
					FATALTLS( kPGPError_TLSProtocolViolation );
				}
				else
					pktInx += sizeof(PGPUInt16);
			}
			else
			{
				(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
											kPGPtls_AT_DecryptionFailed );
				FATALTLS( kPGPError_TLSProtocolViolation );
			}
		}
		preMasterSize = decSize;
		preMaster = PGPNewSecureData( session->memMgr, decSize, 0 );
		if( IsNull( preMaster ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		err = PGPPrivateKeyDecrypt( privKeyCon, &inBuffer[pktInx], encSize,
									preMaster, &preMasterSize );
		
		/* The following code shifts all error conditions at this point
			into a secret error condition that we do not explicitly
			report here.  This is designed to prevent the Bleichenbacher
			attack against the PKCS-1 padding of SSL when using RSA
			keys.  All errors at this stage set the preMaster secret to
			random data thus causing errors later in the protocol and
			not allowing an attacker to get a specific protocol version
			alert or decryption failed alert here.	*/
		if( IsPGPError( err ) ||
			( preMasterSize != kPGPtls_MasterSecretSize ) ||
			( preMaster[0] != kPGPtls_MajorVersion ) ||
			( preMaster[1] != session->originalClientVersion ) )
		{
			preMasterSize = kPGPtls_MasterSecretSize;
			err = PGPContextGetRandomBytes( session->pgpContext,
									&preMaster, preMasterSize ); CKERR;
		}
	}
	/* Calculate the Master Secret */
	err = pgpTLSCalculateMasterSecret( session, preMaster, preMasterSize );
	
	/* Calculate the Keys */
	err = pgpTLSCalculateKeys( session ); CKERR;

done:
	if( dhSecret != kPGPInvalidBigNumRef )
		(void)PGPFreeBigNum( dhSecret );
	if( IsntNull( preMaster ) )
		(void)PGPFreeData( preMaster );
	if( PGPPrivateKeyContextRefIsValid( privKeyCon ) )
		(void)PGPFreePrivateKeyContext( privKeyCon );
	return err;
}	

	PGPError
pgpTLSReceiveCertVerify(
	PGPtlsSessionPriv *	session,
	PGPByte *			inBuffer,
	const PGPSize		inLength )
{
	PGPError				err	= kPGPError_NoErr;
	PGPByte					sigData[640];
	PGPUInt16				sigLen;
	PGPSize					pktInx = 0,
							sigDataLen;
	PGPPublicKeyContextRef	pubKeyCon = kInvalidPGPPublicKeyContextRef;
	
	if( !PGPKeyRefIsValid( session->remoteKey ) )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_BadCertificate );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	err = PGPNewPublicKeyContext(	session->remoteKey,
									kPGPPublicKeyMessageFormat_PKCS1,
									&pubKeyCon ); CKERR;

	err = pgpTLSCalculateCertVerify( session, sigData, &sigDataLen ); CKERR;
	pgpAssert( sigDataLen < sizeof( sigData ) );
	
	/* Verify the Signature */
	sigLen = PGPEndianToUInt16( kPGPBigEndian, inBuffer );
	pktInx += sizeof(PGPUInt16);
	err = PGPPublicKeyVerifyRaw( pubKeyCon, sigData, sigDataLen,
				inBuffer + pktInx, sigLen );
	pktInx += sigLen;
	if( IsPGPError(err) || ( pktInx != inLength ) )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_BadCertificate );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	session->certVerified = TRUE;
done:
	if( PGPPublicKeyContextRefIsValid( pubKeyCon ) )
		(void)PGPFreePublicKeyContext( pubKeyCon );
	return err;
}	

	PGPError
pgpTLSReceiveCertificate(
	PGPtlsSessionPriv *	session,
	PGPByte *			inBuffer,
	const PGPSize		inLength )
{
	PGPError			err	= kPGPError_NoErr;
	PGPUInt32			certListLen,
						certLen;
	PGPUInt32			numKeys;
	PGPKeyListRef		keyList = kInvalidPGPKeyListRef;
	PGPKeyIterRef		keyIter = kInvalidPGPKeyIterRef;
	PGPBoolean			keyVerify;
	
	PGPValidatePtr( inBuffer );
	
	if( !session->isClientSide )
	{
		if( session->cipherSuites[session->cipherSuiteIndex].usePGPKeys )
		{
			if( inLength == 0)
			{
				session->certRequested = FALSE;
				goto done;
			}
		}
		else if( inLength == 3 )
		{
			certListLen = ( inBuffer[0] << 16 ) | ( inBuffer[1] << 8 )
						| inBuffer[2];
			if( certListLen == 0)
			{
				session->certRequested = FALSE;
				goto done;
			}
		}
	}
	if( session->cipherSuites[session->cipherSuiteIndex].usePGPKeys )
	{
		certLen = ( inBuffer[0] << 16 ) | ( inBuffer[1] << 8 )
					| inBuffer[2];
		if( certLen + 3 != inLength )
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_DecodeError );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
		err = PGPImportKeySet( session->pgpContext, &session->remoteKeySet,
					PGPOInputBuffer( session->pgpContext,
									&inBuffer[3], certLen ),
					PGPOLastOption( session->pgpContext ) );
		if( IsPGPError( err ) )
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_BadCertificate );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
		err = PGPCountKeys( session->remoteKeySet, &numKeys );
		if( numKeys != 1 )
		{
			err = PGPFreeKeySet( session->remoteKeySet );
			session->remoteKeySet = kInvalidPGPKeySetRef;
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_BadCertificate );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
	}
	else
	{
		certListLen = ( inBuffer[0] << 16 ) | ( inBuffer[1] << 8 )
					| inBuffer[2];
		certLen =  ( inBuffer[3] << 16 ) | ( inBuffer[4] << 8 )
					| inBuffer[5];
		if( certLen + 6 > inLength )
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_DecodeError );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
		err = PGPImportKeySet( session->pgpContext, &session->remoteKeySet,
					PGPOInputBuffer( session->pgpContext,
									&inBuffer[6], certLen ),
					PGPOX509Encoding( session->pgpContext, TRUE ),
					PGPOLastOption( session->pgpContext ) );
		if( IsPGPError( err ) )
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_BadCertificate );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
		err = PGPCountKeys( session->remoteKeySet, &numKeys );
		if( numKeys != 1 )
		{
			err = PGPFreeKeySet( session->remoteKeySet );
			session->remoteKeySet = kInvalidPGPKeySetRef;
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_BadCertificate );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
	}
	/* Get just the one key out of it */
	err = PGPOrderKeySet( session->remoteKeySet, kPGPAnyOrdering, 
							&keyList );	CKERR;
	err = PGPNewKeyIter( keyList, &keyIter ); CKERR;
	err = PGPKeyIterNext( keyIter, &session->remoteKey ); CKERR;
	err = PGPGetKeyNumber( session->remoteKey, kPGPKeyPropAlgID,
							( PGPInt32 * ) &session->remoteKeyAlg );CKERR;
	err = PGPGetKeyBoolean( session->remoteKey, kPGPKeyPropCanVerify,
							&keyVerify );	CKERR;
	
	if( !keyVerify || !pubAlgSupported( session->remoteKeyAlg ) ||
		( session->isClientSide && ( session->remoteKeyAlg !=
		session->cipherSuites[session->cipherSuiteIndex].sigAlg ) ) )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_BadCertificate );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	if( session->isClientSide )
	{
		if( session->cipherSuites[session->cipherSuiteIndex].ephemeral )
			pgpTLSNextState( session, kPGPtls_EV_EphemeralCertificate );
		else
			pgpTLSNextState( session, kPGPtls_EV_StaticCertificate );
	}
done:
	if( PGPKeyListRefIsValid( keyList ) )
		PGPFreeKeyList( keyList );
	if( PGPKeyIterRefIsValid( keyIter ) )
		PGPFreeKeyIter( keyIter );
	return err;
}

	PGPError
pgpTLSReceiveServerKeyExchange(
	PGPtlsSessionPriv *		session,
	PGPByte *				inBuffer,
	const PGPSize			inLength )
{
	PGPError				err	= kPGPError_NoErr;
	PGPUInt16				pktInx = 0,
							varLen;
	PGPHashContextRef		skeHashSHA = kInvalidPGPHashContextRef,
							skeHashMD5 = kInvalidPGPHashContextRef;
	PGPSize					sigSize;
	PGPPublicKeyContextRef	pubKeyCon = kInvalidPGPPublicKeyContextRef;
	
	PGPValidatePtr( inBuffer );
	
	err = PGPNewPublicKeyContext(	session->remoteKey,
									kPGPPublicKeyMessageFormat_PKCS1,
									&pubKeyCon ); CKERR;
	err = PGPGetPublicKeyOperationSizes( pubKeyCon, NULL, NULL, &sigSize );
	CKERR;

	/* Parse the p value */
	varLen = PGPEndianToUInt16( kPGPBigEndian, &inBuffer[pktInx] );
	pktInx += sizeof(PGPUInt16);
	err = PGPNewBigNum( session->memMgr, TRUE, &session->dhP ); CKERR;
	err =  PGPBigNumInsertBigEndianBytes(	session->dhP,
											&inBuffer[pktInx],
											0, varLen ); CKERR;
	pktInx += varLen;
	/* Parse the g value */
	varLen = PGPEndianToUInt16( kPGPBigEndian, &inBuffer[pktInx] );
	pktInx += sizeof(PGPUInt16);
	err = PGPNewBigNum( session->memMgr, TRUE, &session->dhG ); CKERR;
	err =  PGPBigNumInsertBigEndianBytes(	session->dhG,
											&inBuffer[pktInx],
											0, varLen ); CKERR;
	pktInx += varLen;
	/* Parse the server y value */
	varLen = PGPEndianToUInt16( kPGPBigEndian, &inBuffer[pktInx] );
	pktInx += sizeof(PGPUInt16);
	err = PGPNewBigNum( session->memMgr, TRUE, &session->dhYs ); CKERR;
	err =  PGPBigNumInsertBigEndianBytes(	session->dhYs,
											&inBuffer[pktInx],
											0, varLen ); CKERR;
	pktInx += varLen;

	varLen = PGPEndianToUInt16( kPGPBigEndian, &inBuffer[pktInx] );
	if( ( varLen != sigSize ) || ( pktInx + 2 + sigSize != inLength ) )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_DecodeError );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_SHA,
						&skeHashSHA ); CKERR;
	(void)PGPContinueHash( skeHashSHA, session->cRandom, kPGPtls_RandomSize);
	(void)PGPContinueHash( skeHashSHA, session->sRandom, kPGPtls_RandomSize);
	(void)PGPContinueHash( skeHashSHA, inBuffer, pktInx);
	
	switch( session->cipherSuites[session->cipherSuiteIndex].sigAlg )
	{
		case kPGPPublicKeyAlgorithm_DSA:
		{
			PGPByte		signedData[ kPGPtls_HA_SHASize ];
			
			pktInx += sizeof(PGPUInt16);
			/* Verify the Signature */			
			(void)PGPFinalizeHash( skeHashSHA, signedData );
			err = PGPPublicKeyVerifyRaw( pubKeyCon, signedData,
									kPGPtls_HA_SHASize, inBuffer + pktInx,
									sigSize );
			break;
		}
		case kPGPPublicKeyAlgorithm_RSA:
		{
			PGPByte		signedData[ kPGPtls_HA_MD5Size + kPGPtls_HA_SHASize ];
			
			/* Hash the client/server randoms and the first part of packet */
			err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_MD5,
								&skeHashMD5 ); CKERR;
			(void)PGPContinueHash( skeHashMD5, session->cRandom,
									kPGPtls_RandomSize);
			(void)PGPContinueHash( skeHashMD5, session->sRandom,
									kPGPtls_RandomSize);
			(void)PGPContinueHash( skeHashMD5, inBuffer, pktInx);
			
			pktInx += sizeof(PGPUInt16);
			/* Verify the Signature */			
			(void)PGPFinalizeHash( skeHashMD5, signedData );
			(void)PGPFinalizeHash( skeHashSHA, signedData +
						kPGPtls_HA_MD5Size );
			
			err = PGPPublicKeyVerifyRaw( pubKeyCon, signedData,
									kPGPtls_HA_MD5Size + kPGPtls_HA_SHASize,
									inBuffer + pktInx, sigSize );
			break;
		}
		
		default:
			break;
	}
	if( IsPGPError(err) )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_BadCertificate );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
done:
	if( PGPHashContextRefIsValid( skeHashMD5 ) )
		(void)PGPFreeHashContext( skeHashMD5 );
	if( PGPHashContextRefIsValid( skeHashSHA ) )
		(void)PGPFreeHashContext( skeHashSHA );
	if( PGPPublicKeyContextRefIsValid( pubKeyCon ) )
		(void)PGPFreePublicKeyContext( pubKeyCon );
		
	return err;
}	

	PGPError
pgpTLSReceiveCertRequest(
	PGPtlsSessionPriv *	session,
	PGPByte *			inBuffer,
	const PGPSize		inLength )
{
	PGPError			err	= kPGPError_NoErr;
	PGPUInt16			pktInx = 0;
	PGPUInt16			numCertTypes;
	
	PGPValidatePtr( inBuffer );
	
	numCertTypes = inBuffer[pktInx++];
	if( (PGPSize) ( pktInx + numCertTypes ) > inLength )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_DecodeError );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	session->certRequested = TRUE;
	if( session->cipherSuites[session->cipherSuiteIndex].usePGPKeys )
	{
		if( PGPKeyRefIsValid( session->localKey ) )
		{
			while( numCertTypes-- )
			{
				switch( inBuffer[pktInx++] )
				{
					case kPGPtls_CC_DSASign:
						if( session->localKeyAlg == kPGPPublicKeyAlgorithm_DSA )
							session->certMatched = TRUE;
						break;
					case kPGPtls_CC_RSASign:
						if( session->localKeyAlg == kPGPPublicKeyAlgorithm_RSA )
							session->certMatched = TRUE;
						break;
				}
			}
		}
	}
	else
	{
		/* ##### */
		pgpAssert( 0 );
	}
	
done:
	return err;
}

	PGPError
pgpTLSReceiveServerHelloDone(
	PGPtlsSessionPriv *	session,
	PGPByte *			inBuffer,
	const PGPSize		inLength )
{
	PGPError	err	= kPGPError_NoErr;
	
	(void) inLength;
	
	PGPValidatePtr( inBuffer );
	
	/* Now we send the ClientCertificate message if requested */
	if( session->certRequested )
	{
		err = pgpTLSSendCertificate( session ); CKERR;
	}
	/* Now we send the ClientKeyExchange message */
	err = pgpTLSSendClientKeyExchange( session ); CKERR;
	
	/* Now we send the CertificateVerify message if applicable */
	if( session->certMatched )
	{
		err = pgpTLSSendCertVerify( session ); CKERR;
	}
	
	/* Calculate the Keys */
	err = pgpTLSCalculateKeys( session ); CKERR;
	
	err = pgpTLSSendFinalHandshake( session ); CKERR;
	
done:
	return err;
}	

	PGPError
pgpTLSReceiveFinished(
	PGPtlsSessionPriv *	session,
	PGPByte *			inBuffer,
	const PGPSize		inLength )
{
	PGPError			err	= kPGPError_NoErr;
	PGPSize				checkSize;
	PGPByte				checkFin[64];
	
	PGPValidatePtr( inBuffer );
	
	if( IsNull( session->readActive ) )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_UnexpectedMessage );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	err = pgpTLSCalculateFinished( session,
									(PGPBoolean) !session->isClientSide,
									checkFin, &checkSize );
	CKERR;
	if( ( inLength != checkSize ) ||
		!pgpMemoryEqual( checkFin, inBuffer, checkSize ) )
	{
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_HandshakeFailure );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}
	if( !session->isClientSide && session->certRequested &&
		( !PGPKeyRefIsValid( session->remoteKey ) ||
		!session->certVerified ) )
	{
		/*	We asked for a certificate but no response was returned,
			or the CertificateVerify message never arrived,
			possible indication of active attack. */
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_HandshakeFailure );
		FATALTLS( kPGPError_TLSProtocolViolation );
	}

done:
	return err;
}	

	PGPError
ssl3MasterSecret16(
	PGPtlsSessionPriv *	session,
	PGPByte *			preMaster,
	PGPUInt16			preMasterSize,
	PGPByte				mixer,
	PGPByte *			masterSecret )
{
	PGPError			err	= kPGPError_NoErr;
	PGPHashContextRef	sha,
						md5;
	PGPByte				shaHash[kPGPtls_HA_SHASize],
						mixerByte,
						mixIters;

	err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_SHA, &sha );
	if( IsPGPError( err ) )
		return err;
	err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_MD5, &md5 );
	if( IsPGPError( err ) )
		return err;

	/* Prefix with n bytes of 0x40 + n */
	mixerByte = 0x40 + mixer;
	for( mixIters = 0; mixIters < mixer; mixIters++ )
		(void)PGPContinueHash( sha, &mixerByte, 1 );

	/* Then premastersecret, client_random, server_random */
	(void)PGPContinueHash( sha, preMaster, preMasterSize );
	(void)PGPContinueHash( sha, session->cRandom, kPGPtls_RandomSize );
	(void)PGPContinueHash( sha, session->sRandom, kPGPtls_RandomSize );
	(void)PGPFinalizeHash( sha, shaHash );
	(void)PGPFreeHashContext( sha );

	/* MD5 gets premastersecret and shahash */
	(void)PGPContinueHash( md5, preMaster, preMasterSize );
	(void)PGPContinueHash( md5, shaHash, kPGPtls_HA_SHASize );

	/* The output of this is 16 bytes of mastersecret */
	(void)PGPFinalizeHash( md5, masterSecret );
	(void)PGPFreeHashContext( md5 );
	
	return err;
}

	PGPError
ssl3CalculateKeyBlock(
	PGPtlsSessionPriv *	session,
	PGPByte *			keyBlock,
	PGPSize				needed )
{
	PGPError			err	= kPGPError_NoErr;
	PGPHashContextRef	sha = kInvalidPGPHashContextRef,
						md5 = kInvalidPGPHashContextRef;
	PGPUInt16			iterations,
						iterIndex,
						mixerIndex;
	PGPByte				mixerByte;
	PGPByte *			preBlockPos;
	PGPByte *			preBlock = NULL;
	PGPByte				shaHash[kPGPtls_HA_SHASize];
	
	err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_SHA, &sha);
	CKERR;
	err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_MD5, &md5);
	CKERR;
	
	iterations = needed / kPGPtls_HA_MD5Size;
	if( needed % kPGPtls_HA_MD5Size )
		iterations++;
	
	preBlock  = PGPNewSecureData( session->memMgr,
									iterations * kPGPtls_HA_MD5Size, 0 );
	if( IsNull( preBlock ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	preBlockPos = preBlock;
	for( iterIndex = 1; iterIndex <= iterations; iterIndex++ )
	{
		mixerByte = 0x40 + iterIndex;
		for( mixerIndex = 0; mixerIndex < iterIndex; mixerIndex++ )
			(void)PGPContinueHash( sha, &mixerByte, 1 );
		(void)PGPContinueHash( sha, session->masterSecret,
								kPGPtls_MasterSecretSize );
		(void)PGPContinueHash( sha, session->sRandom, kPGPtls_RandomSize );
		(void)PGPContinueHash( sha, session->cRandom, kPGPtls_RandomSize );
		(void)PGPFinalizeHash( sha, shaHash );
		(void)PGPResetHash( sha );
		
		(void)PGPContinueHash( md5, session->masterSecret,
				kPGPtls_MasterSecretSize );
		(void)PGPContinueHash( md5, shaHash, kPGPtls_HA_SHASize );
		(void)PGPFinalizeHash( md5, preBlockPos );
		(void)PGPResetHash( md5 );
		preBlockPos += kPGPtls_HA_MD5Size;
	}
	
	/* Finished generating KeyBlock, copy needed portion */
	pgpCopyMemory( preBlock, keyBlock, needed );
	
done:
	if( IsntNull( preBlock ) )
		(void)PGPFreeData( preBlock );
	if( PGPHashContextRefIsValid( sha ) )
		(void)PGPFreeHashContext( sha );
	if( PGPHashContextRefIsValid( md5 ) )
		(void)PGPFreeHashContext( md5 );
	return err;
}

	PGPError
tlsCalculateKeyBlock(
	PGPtlsSessionPriv *	session,
	PGPByte *			keyBlock,
	PGPSize				needed )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				randoms[kPGPtls_RandomSize * 2];
	
	pgpCopyMemory( session->sRandom, randoms, kPGPtls_RandomSize );
	pgpCopyMemory( session->cRandom, randoms + kPGPtls_RandomSize,
					kPGPtls_RandomSize );
	err = pgpTLSPRF( session, session->masterSecret, kPGPtls_MasterSecretSize,
						"key expansion", randoms, kPGPtls_RandomSize * 2,
						keyBlock, needed );
	return err;
}

	PGPError
pgpTLSPRFTest(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				secret[kPGPtls_MasterSecretSize];
	PGPByte				seed[kPGPtls_RandomSize * 2];
	PGPByte				prfOut[104];
	PGPByte				hashOut[kPGPtls_HA_MD5Size];
	PGPHashContextRef	hash;
	PGPByte				testVector[kPGPtls_HA_MD5Size] =
							{	0xCD, 0x7C, 0xA2, 0xCB,
								0x9A, 0x6A, 0x3C, 0x6F,
								0x34, 0x5C, 0x46, 0x65,
								0xA8, 0xB6, 0x81, 0x6B };
	
	pgpFillMemory( secret, kPGPtls_MasterSecretSize, 0xAB );
	pgpFillMemory( seed, kPGPtls_RandomSize * 2, 0xCD );
	err = pgpTLSPRF( session, secret, kPGPtls_MasterSecretSize,
					"PRF Testvector", seed, kPGPtls_RandomSize * 2,
					prfOut, sizeof(prfOut) );	CKERR;
	err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_MD5, &hash );CKERR;
	err = PGPContinueHash( hash, prfOut, sizeof(prfOut) );	CKERR;
	err = PGPFinalizeHash( hash, hashOut );	CKERR;
	err = PGPFreeHashContext( hash );	CKERR;
	
	if( !pgpMemoryEqual( hashOut, testVector, kPGPtls_HA_MD5Size ) )
		err = kPGPError_CorruptData;
	
done:
	return err;
}

	PGPError
pgpTLSPRF(
	PGPtlsSessionPriv *	session,
	PGPByte *			secret,
	PGPSize				secretLen,
	const char *		label,
	PGPByte *			seedData,
	PGPSize				seedLen,
	PGPByte *			outputData,
	PGPSize				outputLen )
{
	PGPError			err	= kPGPError_NoErr;
	PGPHMACContextRef	hmac = kInvalidPGPHMACContextRef,
						inHMAC = kInvalidPGPHMACContextRef;
	PGPByte				secret1[256],
						secret2[256],
						pSeed[256],
						pMD5Output[128],
						pSHAOutput[128];
	PGPSize				halfSecLen,
						labelLen,
						pSeedLen,
						hashOutLen;
	PGPUInt32			hmacIters,
						bIndex;
	PGPByte				inHMACOut[kPGPtls_HA_SHASize];
	PGPByte				inHMACOut2[kPGPtls_HA_SHASize];
	
	/*	This was definitely one of the more complex functions to
		architect in this code, but once one fully understands this
		function, you will get a really good feeling about the security
		provided by TLS.
		N.B.  This function might look simple now, but try reading the
		TLS spec to understand it first.  That ought to thoroughly
		confuse you.	*/
	
	labelLen = strlen( label );
	
	/* To avoid allocating memory we use fixed buffers which should
		easily accomodate all TLS situations. */
	pgpAssert( outputLen < sizeof( pMD5Output ) );
	pgpAssert( secretLen <= sizeof( secret1 ) * 2 );
	pgpAssert( labelLen + seedLen < sizeof( pSeed ) );
	
	/* Divide secret into halves */
	halfSecLen = secretLen / 2;
	if( secretLen % 2 )
		halfSecLen++;
	pgpCopyMemory( secret, secret1, halfSecLen );
	pgpCopyMemory( secret + halfSecLen - ( secretLen % 2 ? 1 : 0 ),
					secret2, halfSecLen );
	
	/* Create seed value for P_Hash functions */
	pgpCopyMemory( label, pSeed, labelLen );
	pgpCopyMemory( seedData, pSeed + labelLen, seedLen );
	pSeedLen = labelLen + seedLen;
	
	/* Do P_MD5 */
	hmacIters = outputLen / kPGPtls_HA_MD5Size;
	if( outputLen % kPGPtls_HA_MD5Size )
		hmacIters++;
	err = PGPNewHMACContext( session->memMgr, kPGPHashAlgorithm_MD5,
							secret1, halfSecLen, &hmac );	CKERR;
	err = PGPNewHMACContext( session->memMgr, kPGPHashAlgorithm_MD5,
							secret1, halfSecLen, &inHMAC );	CKERR;
	err = PGPContinueHMAC( inHMAC, pSeed, pSeedLen );	CKERR;
	err = PGPFinalizeHMAC( inHMAC, inHMACOut );
	err = PGPResetHMAC( inHMAC );	CKERR;
	hashOutLen = 0;
	for( bIndex = 0; bIndex < hmacIters; bIndex++ )
	{
		pgpCopyMemory( inHMACOut, inHMACOut2, kPGPtls_HA_MD5Size );
		err = PGPContinueHMAC( inHMAC, inHMACOut, kPGPtls_HA_MD5Size );	CKERR;
		err = PGPFinalizeHMAC( inHMAC, inHMACOut );
		err = PGPResetHMAC( inHMAC );	CKERR;
		
		/* Feed the HMAC */
		err = PGPContinueHMAC( hmac, inHMACOut2, kPGPtls_HA_MD5Size );	CKERR;
		err = PGPContinueHMAC( hmac, pSeed, pSeedLen );	CKERR;
		err = PGPFinalizeHMAC( hmac, pMD5Output + hashOutLen );
		err = PGPResetHMAC( hmac );	CKERR;
		hashOutLen += kPGPtls_HA_MD5Size;
	}
	err = PGPFreeHMACContext( inHMAC );	CKERR;
	inHMAC = kInvalidPGPHMACContextRef;
	err = PGPFreeHMACContext( hmac );	CKERR;
	hmac = kInvalidPGPHMACContextRef;
	
	/* Do P_SHA */
	hmacIters = outputLen / kPGPtls_HA_SHASize;
	if( outputLen % kPGPtls_HA_SHASize )
		hmacIters++;
	err = PGPNewHMACContext( session->memMgr, kPGPHashAlgorithm_SHA,
							secret2, halfSecLen, &hmac );	CKERR;
	err = PGPNewHMACContext( session->memMgr, kPGPHashAlgorithm_SHA,
							secret2, halfSecLen, &inHMAC );	CKERR;
	err = PGPContinueHMAC( inHMAC, pSeed, pSeedLen );	CKERR;
	err = PGPFinalizeHMAC( inHMAC, inHMACOut );
	err = PGPResetHMAC( inHMAC );	CKERR;
	hashOutLen = 0;
	for( bIndex = 0; bIndex < hmacIters; bIndex++ )
	{
		pgpCopyMemory( inHMACOut, inHMACOut2, kPGPtls_HA_SHASize );
		err = PGPContinueHMAC( inHMAC, inHMACOut, kPGPtls_HA_SHASize );	CKERR;
		err = PGPFinalizeHMAC( inHMAC, inHMACOut );
		err = PGPResetHMAC( inHMAC );	CKERR;
		
		/* Feed the HMAC */
		err = PGPContinueHMAC( hmac, inHMACOut2, kPGPtls_HA_SHASize );	CKERR;
		err = PGPContinueHMAC( hmac, pSeed, pSeedLen );	CKERR;
		err = PGPFinalizeHMAC( hmac, pSHAOutput + hashOutLen );
		err = PGPResetHMAC( hmac );	CKERR;
		hashOutLen += kPGPtls_HA_SHASize;
	}
	err = PGPFreeHMACContext( inHMAC );	CKERR;
	inHMAC = kInvalidPGPHMACContextRef;
	err = PGPFreeHMACContext( hmac );	CKERR;
	hmac = kInvalidPGPHMACContextRef;
	
	/* Create the final output */
	for( bIndex = 0; bIndex < outputLen; bIndex++ )
		outputData[bIndex] = pMD5Output[bIndex] ^ pSHAOutput[bIndex];
	
done:
	if( PGPHMACContextRefIsValid( hmac ) )
		PGPFreeHMACContext( hmac );
	if( PGPHMACContextRefIsValid( inHMAC ) )
		PGPFreeHMACContext( inHMAC );
	return err;
}

	PGPError
pgpTLSCalculateMasterSecret(
	PGPtlsSessionPriv *	session,
	PGPByte *			preMaster,
	PGPSize				preMasterSize )
{
	PGPError			err	= kPGPError_NoErr;
	
	if( session->minorVersion == kPGPtls_SSL3MinorVersion )
	{
		/* The quaint little SSL3 master secret calculation */
		
		ssl3MasterSecret16( session, preMaster, (PGPUInt16) preMasterSize,
							1, session->masterSecret );
		ssl3MasterSecret16( session, preMaster, (PGPUInt16) preMasterSize,
							2, session->masterSecret + kPGPtls_HA_MD5Size );
		ssl3MasterSecret16( session, preMaster, (PGPUInt16) preMasterSize,
							3, session->masterSecret + 2 * kPGPtls_HA_MD5Size );
	}
	else
	{
		PGPByte			randoms[kPGPtls_RandomSize * 2];
		
		/* The TLS mother of all master secret calculations */
		
		pgpCopyMemory( session->cRandom, randoms, kPGPtls_RandomSize );
		pgpCopyMemory( session->sRandom, randoms + kPGPtls_RandomSize,
						kPGPtls_RandomSize );
		
		err = pgpTLSPRF( session, preMaster, preMasterSize,
							"master secret", randoms, kPGPtls_RandomSize * 2,
							session->masterSecret, kPGPtls_MasterSecretSize );
		CKERR;
	}
done:
	return err;
}

	PGPError
pgpTLSCalculateKeys(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPSize				needed = 0,
						hashSize = 0,
						keySize = 0;
	PGPByte *			keyBlock = NULL;
	PGPByte *			keyBlockPos = NULL;

	/* Figure out how much KeyBlock we need */
	switch( session->cipherSuites[session->cipherSuiteIndex].hashID )
	{
		case kPGPHashAlgorithm_SHA:
			hashSize = kPGPtls_HA_SHASize;
			break;
		case kPGPHashAlgorithm_RIPEMD160:
			hashSize = kPGPtls_HA_RIPEMD160Size;
			break;
		case kPGPHashAlgorithm_MD5:
			hashSize = kPGPtls_HA_MD5Size;
			break;
		default:
			break;
	}
	session->hashSize = hashSize;
	needed += hashSize * 2;
	switch( session->cipherSuites[session->cipherSuiteIndex].cipherID )
	{
		case kPGPCipherAlgorithm_CAST5:
			keySize = kPGPtls_SC_CASTKeySize;
			break;
		case kPGPCipherAlgorithm_IDEA:
			keySize = kPGPtls_SC_IDEAKeySize;
			break;
		case kPGPCipherAlgorithm_3DES:
			keySize = kPGPtls_SC_3DESKeySize;
			break;
		default:
			break;
	}
	session->keySize = keySize;
	needed += keySize * 2;
	if( keySize > 0 )
		needed += kPGPtls_CipherBlockSize * 2;	/* 2 IVs for CBC */
	
	/* Calculate KeyBlock */
	keyBlock  = PGPNewSecureData( session->memMgr, needed, 0 );
	if( IsNull( keyBlock ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	if( session->minorVersion == kPGPtls_SSL3MinorVersion )
		err = ssl3CalculateKeyBlock( session, keyBlock, needed );
	else
		err = tlsCalculateKeyBlock( session, keyBlock, needed );
	CKERR;
	
	session->clientPending = PGPNewSecureData( session->memMgr,
												sizeof( PGPtlsKeys ),
												kPGPMemoryMgrFlags_Clear );
	session->serverPending = PGPNewSecureData( session->memMgr,
												sizeof( PGPtlsKeys ),
												kPGPMemoryMgrFlags_Clear );
	if( IsNull( session->clientPending ) || IsNull( session->serverPending ) )
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}
	keyBlockPos = keyBlock;
	pgpCopyMemory( keyBlockPos, session->clientPending->macSecret, hashSize );
	keyBlockPos += hashSize;
	pgpCopyMemory( keyBlockPos, session->serverPending->macSecret, hashSize );
	keyBlockPos += hashSize;
	if( keySize > 0 )
	{
		pgpCopyMemory( keyBlockPos, session->clientPending->cipherKey,
						keySize );
		keyBlockPos += keySize;
		pgpCopyMemory( keyBlockPos, session->serverPending->cipherKey,
						keySize );
		keyBlockPos += keySize;
		pgpCopyMemory( keyBlockPos, session->clientPending->iv,
						kPGPtls_CipherBlockSize );
		keyBlockPos += kPGPtls_CipherBlockSize;
		pgpCopyMemory( keyBlockPos, session->serverPending->iv,
						kPGPtls_CipherBlockSize );
		keyBlockPos += kPGPtls_CipherBlockSize;
	}
	pgpAssert( keyBlockPos == keyBlock + needed );
	
done:
	if( IsntNull( keyBlock ) )
		(void)PGPFreeData( keyBlock );
	return err;
}

	PGPError
pgpTLSCalculateMAC(
	PGPtlsSessionPriv *	session,
	const PGPBoolean	inSending,
	const PGPByte		inType,
	const PGPByte *		inBuffer,
	const PGPSize		inLength,
	PGPByte *			outBuffer )
{
	PGPError			err	= kPGPError_NoErr;
	PGPtlsKeys *		keys;
	PGPUInt16			seqNumIndex;
	PGPByte				stufBuf[5];
	
	if( inSending )
		keys = session->writeActive;
	else
		keys = session->readActive;
	if( session->minorVersion == kPGPtls_SSL3MinorVersion )
	{
		PGPUInt16	padSize,
					padIndex;
		PGPByte		padBuffer[kPGPtls_SSL3MD5PaddingBytes];
		PGPByte		innerHash[32];
		
		if( !PGPHashContextRefIsValid( session->innerMACHash ) )
		{
			err = PGPNewHashContext( session->memMgr,
					session->cipherSuites[session->cipherSuiteIndex].hashID,
					&session->innerMACHash ); CKERR;
			err = PGPNewHashContext( session->memMgr,
					session->cipherSuites[session->cipherSuiteIndex].hashID,
					&session->outerMACHash ); CKERR;
		}
		(void)PGPContinueHash( session->innerMACHash, keys->macSecret,
							session->hashSize );
		if( session->cipherSuites[session->cipherSuiteIndex].hashID ==
			kPGPHashAlgorithm_MD5 )
			padSize = kPGPtls_SSL3MD5PaddingBytes;
		else
			padSize = kPGPtls_SSL3SHAPaddingBytes;
		for( padIndex = 0; padIndex < padSize; padIndex++ )
			padBuffer[padIndex] = kPGPtls_ipad;
		(void)PGPContinueHash( session->innerMACHash, padBuffer, padSize );
		(void)PGPContinueHash( session->innerMACHash, keys->seqNum,
							kPGPtls_SequenceNumberSize );
		stufBuf[0] = inType;
		stufBuf[1] = (inLength >> 8) & 0xFF;
		stufBuf[2] = (inLength >> 0) & 0xFF;
		(void)PGPContinueHash( session->innerMACHash, stufBuf, 3 );
		(void)PGPContinueHash( session->innerMACHash, inBuffer, inLength );
		(void)PGPFinalizeHash( session->innerMACHash, innerHash );
		(void)PGPResetHash( session->innerMACHash );
		
		(void)PGPContinueHash( session->outerMACHash, keys->macSecret,
							session->hashSize );
		for( padIndex = 0; padIndex < padSize; padIndex++ )
			padBuffer[padIndex] = kPGPtls_opad;
		(void)PGPContinueHash( session->outerMACHash, padBuffer, padSize );
		(void)PGPContinueHash( session->outerMACHash, innerHash,
								session->hashSize );
		(void)PGPFinalizeHash( session->outerMACHash, outBuffer );
		(void)PGPResetHash( session->outerMACHash );
	}
	else
	{
		if( !PGPHMACContextRefIsValid( keys->hmac ) )
		{
			err = PGPNewHMACContext( session->memMgr,
					session->cipherSuites[session->cipherSuiteIndex].hashID,
					keys->macSecret, session->hashSize,
					&keys->hmac ); CKERR;
		}
		(void)PGPContinueHMAC( keys->hmac, keys->seqNum,
								kPGPtls_SequenceNumberSize );
		stufBuf[0] = inType;
		stufBuf[1] = kPGPtls_MajorVersion;
		stufBuf[2] = session->minorVersion;
		stufBuf[3] = (inLength >> 8) & 0xFF;
		stufBuf[4] = (inLength >> 0) & 0xFF;
		(void)PGPContinueHMAC( keys->hmac, stufBuf, 5 );
		(void)PGPContinueHMAC( keys->hmac, inBuffer, inLength );
		(void)PGPFinalizeHMAC( keys->hmac, outBuffer );
		
		(void)PGPResetHMAC( keys->hmac );
	}
	for( seqNumIndex = 7; (PGPByte)++keys->seqNum[seqNumIndex] == 0;
			--seqNumIndex )
		;
done:
	return err;
}

	PGPError
pgpTLSCalculateCertVerify(
	PGPtlsSessionPriv *	session,
	PGPByte *			outBuffer,
	PGPSize *			outSize )
{
	PGPError			err	= kPGPError_NoErr;
	PGPHashContextRef	cvHashSHA = kInvalidPGPHashContextRef,
						cvHashMD5 = kInvalidPGPHashContextRef;
	
	*outSize = 0;
	if( !PGPHashContextRefIsValid( session->lastHandSHA ) )
	{
		err = PGPCopyHashContext( session->handshakeSHA,
									&session->lastHandSHA ); CKERR;
		err = PGPCopyHashContext( session->handshakeMD5,
									&session->lastHandMD5 ); CKERR;
	}
	if( session->minorVersion == kPGPtls_SSL3MinorVersion )
	{
		PGPUInt16			padIndex;
		PGPByte				padBuffer[kPGPtls_SSL3MD5PaddingBytes],
							tempHashBuf[32];
		
		err = PGPContinueHash( session->lastHandSHA, session->masterSecret,
								kPGPtls_MasterSecretSize ); CKERR;
		for( padIndex = 0; padIndex < kPGPtls_SSL3SHAPaddingBytes; padIndex++ )
			padBuffer[padIndex] = kPGPtls_ipad;
		err = PGPContinueHash( session->lastHandSHA, padBuffer,
								kPGPtls_SSL3SHAPaddingBytes ); CKERR;
		err = PGPFinalizeHash( session->lastHandSHA, tempHashBuf ); CKERR;
		err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_SHA,
								&cvHashSHA ); CKERR;
		err = PGPContinueHash( cvHashSHA, session->masterSecret,
								kPGPtls_MasterSecretSize ); CKERR;
		for( padIndex = 0; padIndex < kPGPtls_SSL3SHAPaddingBytes; padIndex++ )
			padBuffer[padIndex] = kPGPtls_opad;
		err = PGPContinueHash( cvHashSHA, padBuffer,
								kPGPtls_SSL3SHAPaddingBytes ); CKERR;
		err = PGPContinueHash( cvHashSHA, tempHashBuf,
								kPGPtls_HA_SHASize ); CKERR;
		switch( session->cipherSuites[session->cipherSuiteIndex].sigAlg )
		{
			case kPGPPublicKeyAlgorithm_DSA:
				err = PGPFinalizeHash( cvHashSHA, outBuffer ); CKERR;
				*outSize = kPGPtls_HA_SHASize;
				break;
			case kPGPPublicKeyAlgorithm_RSA:
				err = PGPContinueHash( session->lastHandMD5,
										session->masterSecret,
										kPGPtls_MasterSecretSize ); CKERR;
				for( padIndex = 0; padIndex < kPGPtls_SSL3MD5PaddingBytes;
						padIndex++ )
					padBuffer[padIndex] = kPGPtls_ipad;
				err = PGPContinueHash( session->lastHandMD5, padBuffer,
										kPGPtls_SSL3MD5PaddingBytes ); CKERR;
				err = PGPFinalizeHash( session->lastHandMD5,
										tempHashBuf ); CKERR;
				err = PGPNewHashContext( session->memMgr, kPGPHashAlgorithm_MD5,
									&cvHashMD5 ); CKERR;
				err = PGPContinueHash( cvHashMD5, session->masterSecret,
										kPGPtls_MasterSecretSize ); CKERR;
				for( padIndex = 0; padIndex < kPGPtls_SSL3MD5PaddingBytes;
						padIndex++ )
					padBuffer[padIndex] = kPGPtls_opad;
				err = PGPContinueHash( cvHashMD5, padBuffer,
										kPGPtls_SSL3MD5PaddingBytes ); CKERR;
				err = PGPContinueHash( cvHashMD5, tempHashBuf,
										kPGPtls_HA_MD5Size ); CKERR;
				err = PGPFinalizeHash( cvHashMD5, outBuffer ); CKERR;
				err = PGPFinalizeHash( cvHashSHA, outBuffer +
										kPGPtls_HA_MD5Size ); CKERR;
				*outSize = kPGPtls_HA_SHASize + kPGPtls_HA_MD5Size;
				break;

			default:
				break;
		}
	}
	else
	{
		switch( session->cipherSuites[session->cipherSuiteIndex].sigAlg )
		{
			case kPGPPublicKeyAlgorithm_DSA:
				err = PGPFinalizeHash( session->lastHandSHA, outBuffer ); CKERR;
				*outSize = kPGPtls_HA_SHASize;
				break;
			case kPGPPublicKeyAlgorithm_RSA:
				err = PGPFinalizeHash( session->lastHandMD5, outBuffer ); CKERR;
				err = PGPFinalizeHash( session->lastHandSHA, outBuffer +
											kPGPtls_HA_MD5Size ); CKERR;
				*outSize = kPGPtls_HA_SHASize + kPGPtls_HA_MD5Size;
				break;
			default:
				break;
		}
	}
done:
	if( PGPHashContextRefIsValid( cvHashSHA ) )
		(void)PGPFreeHashContext( cvHashSHA );
	if( PGPHashContextRefIsValid( cvHashMD5 ) )
		(void)PGPFreeHashContext( cvHashMD5 );
	if( PGPHashContextRefIsValid( session->lastHandSHA ) )
		(void)PGPFreeHashContext( session->lastHandSHA );
	session->lastHandSHA = kInvalidPGPHashContextRef;
	if( PGPHashContextRefIsValid( session->lastHandMD5 ) )
		(void)PGPFreeHashContext( session->lastHandMD5 );
	session->lastHandMD5 = kInvalidPGPHashContextRef;
	return err;
}

	PGPError
pgpTLSCalculateFinished(
	PGPtlsSessionPriv *	session,
	PGPBoolean			clientSide,
	PGPByte *			inBuffer,
	PGPSize *			outSize )
{
	PGPError			err	= kPGPError_NoErr;
	PGPSize				pktLen = 0;
	
	*outSize = 0;
	if( !PGPHashContextRefIsValid( session->lastHandSHA ) )
	{
		err = PGPCopyHashContext( session->handshakeSHA,
									&session->lastHandSHA ); CKERR;
		err = PGPCopyHashContext( session->handshakeMD5,
									&session->lastHandMD5 ); CKERR;
	}
	if( session->minorVersion == kPGPtls_SSL3MinorVersion )
	{
		PGPByte				clientSeed[] = {0x43, 0x4c, 0x4e, 0x54};
		PGPByte				serverSeed[] = {0x53, 0x52, 0x56, 0x52};
		PGPByte				padBuffer[64],
							shaHash[kPGPtls_HA_SHASize],
							md5Hash[kPGPtls_HA_MD5Size];
		PGPUInt16			padIndex,
							padSize;
		
		padSize = ( kPGPtls_SSL3MD5PaddingBytes >
						kPGPtls_SSL3SHAPaddingBytes ) ?
							kPGPtls_SSL3MD5PaddingBytes :
							kPGPtls_SSL3SHAPaddingBytes;
		if( clientSide )
		{
			(void)PGPContinueHash( session->lastHandSHA, clientSeed, 4 );
			(void)PGPContinueHash( session->lastHandMD5, clientSeed, 4 );
		}
		else
		{
			(void)PGPContinueHash( session->lastHandSHA, serverSeed, 4 );
			(void)PGPContinueHash( session->lastHandMD5, serverSeed, 4 );
		}
		(void)PGPContinueHash( session->lastHandSHA, session->masterSecret,
							kPGPtls_MasterSecretSize );
		(void)PGPContinueHash( session->lastHandMD5, session->masterSecret,
							kPGPtls_MasterSecretSize );
		for( padIndex = 0; padIndex < padSize; padIndex++ )
			padBuffer[padIndex] = kPGPtls_ipad;
		(void)PGPContinueHash( session->lastHandSHA, padBuffer,
							kPGPtls_SSL3SHAPaddingBytes );
		(void)PGPContinueHash( session->lastHandMD5, padBuffer,
							kPGPtls_SSL3MD5PaddingBytes );
		(void)PGPFinalizeHash( session->lastHandSHA, shaHash );
		(void)PGPFinalizeHash( session->lastHandMD5, md5Hash );
		
		(void)PGPResetHash( session->lastHandSHA );
		(void)PGPResetHash( session->lastHandMD5 );
		
		(void)PGPContinueHash( session->lastHandSHA, session->masterSecret,
							kPGPtls_MasterSecretSize );
		(void)PGPContinueHash( session->lastHandMD5, session->masterSecret,
							kPGPtls_MasterSecretSize );
		for( padIndex = 0; padIndex < padSize; padIndex++ )
			padBuffer[padIndex] = kPGPtls_opad;
		(void)PGPContinueHash( session->lastHandSHA, padBuffer,
							kPGPtls_SSL3SHAPaddingBytes );
		(void)PGPContinueHash( session->lastHandMD5, padBuffer,
							kPGPtls_SSL3MD5PaddingBytes );
		(void)PGPContinueHash( session->lastHandSHA, shaHash,
								kPGPtls_HA_SHASize );
		(void)PGPContinueHash( session->lastHandMD5, md5Hash,
								kPGPtls_HA_MD5Size );
		
		(void)PGPFinalizeHash( session->lastHandMD5, inBuffer );
		pktLen += kPGPtls_HA_MD5Size;
		(void)PGPFinalizeHash( session->lastHandSHA, inBuffer + pktLen );
		pktLen += kPGPtls_HA_SHASize;
	}
	else
	{
		PGPByte		handshakes[kPGPtls_HA_MD5Size + kPGPtls_HA_SHASize];
		
		(void)PGPFinalizeHash( session->lastHandMD5, handshakes );
		(void)PGPFinalizeHash( session->lastHandSHA,
								handshakes + kPGPtls_HA_MD5Size );
		err = pgpTLSPRF( session, session->masterSecret,
						kPGPtls_MasterSecretSize, clientSide ?
						"client finished" : "server finished", handshakes,
						kPGPtls_HA_MD5Size + kPGPtls_HA_SHASize,
						inBuffer, kPGPtls_FinishedSize );	CKERR;
		pktLen = kPGPtls_FinishedSize;
	}
	
	*outSize = pktLen;
done:
	if( PGPHashContextRefIsValid( session->lastHandSHA ) )
		(void)PGPFreeHashContext( session->lastHandSHA );
	session->lastHandSHA = kInvalidPGPHashContextRef;
	if( PGPHashContextRefIsValid( session->lastHandMD5 ) )
		(void)PGPFreeHashContext( session->lastHandMD5 );
	session->lastHandMD5 = kInvalidPGPHashContextRef;
	return err;
}

	PGPError
pgpTLSActivateWriteKeys(
	PGPtlsSessionPriv *				session )
{
	PGPError						err	= kPGPError_NoErr;
	PGPSymmetricCipherContextRef	symmContext;

	if( IsntNull( session->writeActive ) )
	{
		if( PGPHMACContextRefIsValid( session->writeActive->hmac ) )
			(void)PGPFreeHMACContext( session->writeActive->hmac );
		(void)PGPFreeData( session->writeActive );
		session->writeActive = NULL;
	}
	if( PGPCBCContextRefIsValid( session->writeCipher ) )
	{
		(void)PGPFreeCBCContext( session->writeCipher );
		session->writeCipher = kInvalidPGPCBCContextRef;
	}
	
	if( session->isClientSide )
	{
		session->writeActive = session->clientPending;
		session->clientPending = NULL;
	}
	else
	{
		session->writeActive = session->serverPending;
		session->serverPending = NULL;
	}
	if( session->cipherSuites[session->cipherSuiteIndex].cipherID !=
		kPGPCipherAlgorithm_None )
	{
		err = PGPNewSymmetricCipherContext( session->memMgr,
				session->cipherSuites[session->cipherSuiteIndex].cipherID,
				session->keySize, &symmContext ); CKERR;
		err = PGPNewCBCContext( symmContext, &session->writeCipher ); CKERR;
		err = PGPInitCBC( session->writeCipher,
							session->writeActive->cipherKey,
							session->writeActive->iv ); CKERR;
	}
done:
	return err;
}

	PGPError
pgpTLSActivateReadKeys(
	PGPtlsSessionPriv *				session )
{
	PGPError						err	= kPGPError_NoErr;
	PGPSymmetricCipherContextRef	symmContext;

	if( IsntNull( session->readActive ) )
	{
		if( PGPHMACContextRefIsValid( session->readActive->hmac ) )
			(void)PGPFreeHMACContext( session->readActive->hmac );
		(void)PGPFreeData( session->readActive );
		session->readActive = NULL;
	}
	if( PGPCBCContextRefIsValid( session->readCipher ) )
	{
		(void)PGPFreeCBCContext( session->readCipher );
		session->readCipher = kInvalidPGPCBCContextRef;
	}
	
	if( session->isClientSide )
	{
		session->readActive = session->serverPending;
		session->serverPending = NULL;
	}
	else
	{
		session->readActive = session->clientPending;
		session->clientPending = NULL;
	}
	if( session->cipherSuites[session->cipherSuiteIndex].cipherID !=
		kPGPCipherAlgorithm_None )
	{
		err = PGPNewSymmetricCipherContext( session->memMgr,
				session->cipherSuites[session->cipherSuiteIndex].cipherID,
				session->keySize, &symmContext ); CKERR;
		err = PGPNewCBCContext( symmContext, &session->readCipher ); CKERR;
		err = PGPInitCBC( session->readCipher, session->readActive->cipherKey,
							session->readActive->iv ); CKERR;
	}
done:
	return err;
}

	PGPInt8
pgpTLSPacketToEvent(
	PGPByte		packetType )
{
	PGPInt8		event = 0;
	
	switch( packetType )
	{
		case kPGPtls_HT_HelloRequest:
			event = kPGPtls_EV_ReceiveHelloRequest;
			break;
		case kPGPtls_HT_ClientHello:
			event = kPGPtls_EV_ReceiveClientHello;
			break;
		case kPGPtls_HT_ServerHello:
			event = kPGPtls_EV_ReceiveServerHello;
			break;
		case kPGPtls_HT_Certificate:
			event = kPGPtls_EV_ReceiveCertificate;
			break;
		case kPGPtls_HT_ServerKeyExchange:
			event = kPGPtls_EV_ReceiveServerKeyExchange;
			break;
		case kPGPtls_HT_CertificateRequest:
			event = kPGPtls_EV_ReceiveCertRequest;
			break;
		case kPGPtls_HT_ServerHelloDone:
			event = kPGPtls_EV_ReceiveServerHelloDone;
			break;
		case kPGPtls_HT_CertificateVerify:
			event = kPGPtls_EV_ReceiveCertVerify;
			break;
		case kPGPtls_HT_ClientKeyExchange:
			event = kPGPtls_EV_ReceiveClientKeyExchange;
			break;
		case kPGPtls_HT_Finished:
			event = kPGPtls_EV_ReceiveFinished;
			break;
		default:
			pgpAssert( 0 );
	}
	return event;
}

	PGPError
pgpTLSClientHandshake(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				pktType;
	PGPByte *			packet = NULL;
	PGPSize				pktLen;
	PGPInt8				newState;
	PGPBoolean			handshaking = TRUE;
	
	if( ( session->state == kPGPtls_IdleState ) || 
		( session->state == kPGPtls_ReadyState ) )
	{
		session->state = kPGPtls_HandshakeState;
		err = pgpTLSSendClientHello( session );
		if( IsPGPError( err ) )
			goto done;
	}
	while( handshaking &&
			IsntPGPError( err = pgpTLSReceiveHandshakePacket( session,
			&pktType, &packet, &pktLen ) ) && IsntNull( packet ) )
	{
		newState = pgpTLSNextState( session, pgpTLSPacketToEvent( pktType ) );
		if( newState == -2 )
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_UnexpectedMessage );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
		else if( newState == -1 )
		{
			/* do nothing */
		}
		else switch( pktType )
		{
			case kPGPtls_HT_ServerHello:
				err = pgpTLSReceiveServerHello( session, packet, pktLen );
				break;
			case kPGPtls_HT_Certificate:
				err = pgpTLSReceiveCertificate( session, packet, pktLen );
				break;
			case kPGPtls_HT_ServerKeyExchange:
				err = pgpTLSReceiveServerKeyExchange( session, packet, pktLen );
				break;
			case kPGPtls_HT_CertificateRequest:
				err = pgpTLSReceiveCertRequest( session, packet, pktLen );
				break;
			case kPGPtls_HT_ServerHelloDone:
				err = pgpTLSReceiveServerHelloDone( session, packet, pktLen );
				break;
			case kPGPtls_HT_Finished:
				err = pgpTLSReceiveFinished( session, packet, pktLen );
				if( IsntPGPError( err ) )
				{
					session->state = kPGPtls_ReadyState;
					if( PGPHashContextRefIsValid( session->handshakeSHA ) )
					{
						(void)PGPFreeHashContext( session->handshakeSHA );
						session->handshakeSHA = kInvalidPGPHashContextRef;
					}
					if( PGPHashContextRefIsValid( session->handshakeMD5 ) )
					{
						(void)PGPFreeHashContext( session->handshakeMD5 );
						session->handshakeMD5 = kInvalidPGPHashContextRef;
					}
					handshaking = FALSE;
				}
				break;
			default:
				pgpAssert( 0 );
		}
		(void)pgpContextMemFree( session->pgpContext, packet );
		packet = NULL;
		if( IsPGPError( err ) )
			goto done;
		if( !session->blocking )
			break;
	}
done:
	if( IsntNull( packet ) )
		(void)pgpContextMemFree( session->pgpContext, packet );
	return err;
}

	PGPError
pgpTLSServerHandshake(
	PGPtlsSessionPriv *	session )
{
	PGPError			err	= kPGPError_NoErr;
	PGPByte				pktType;
	PGPByte *			packet = NULL;
	PGPSize				pktLen;
	PGPInt8				newState;
	PGPBoolean			handshaking = TRUE;
	
	if( session->state == kPGPtls_ReadyState )
	{
		err = pgpTLSSendHandshakePacket( session, kPGPtls_HT_HelloRequest,
										NULL, 0 );
		if( IsPGPError( err ) )
			goto done;
	}
	session->state = kPGPtls_HandshakeState;
	while( handshaking &&
			IsntPGPError( err = pgpTLSReceiveHandshakePacket( session,
			&pktType, &packet, &pktLen ) ) && IsntNull( packet ) )
	{
		newState = pgpTLSNextState( session, pgpTLSPacketToEvent( pktType ) );
		if( newState == -2 )
		{
			(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
										kPGPtls_AT_UnexpectedMessage );
			FATALTLS( kPGPError_TLSProtocolViolation );
		}
		else if( newState == -1 )
		{
			/* do nothing */
		}
		else switch( pktType )
		{
			case kPGPtls_HT_ClientHello:
				err = pgpTLSReceiveClientHello( session, packet, pktLen );
				break;
			case kPGPtls_HT_Certificate:
				err = pgpTLSReceiveCertificate( session, packet, pktLen );
				break;
			case kPGPtls_HT_ClientKeyExchange:
				err = pgpTLSReceiveClientKeyExchange( session, packet, pktLen );
				break;
			case kPGPtls_HT_CertificateVerify:
				err = pgpTLSReceiveCertVerify( session, packet, pktLen );
				break;
			case kPGPtls_HT_Finished:
				err = pgpTLSReceiveFinished( session, packet, pktLen );
				if( IsntPGPError( err ) )
				{
					err = pgpTLSSendFinalHandshake( session );
					if( IsntPGPError( err ) )
					{
						session->state = kPGPtls_ReadyState;
						if( PGPHashContextRefIsValid( session->handshakeSHA ) )
						{
							(void)PGPFreeHashContext( session->handshakeSHA );
							session->handshakeSHA = kInvalidPGPHashContextRef;
						}
						if( PGPHashContextRefIsValid( session->handshakeMD5 ) )
						{
							(void)PGPFreeHashContext( session->handshakeMD5 );
							session->handshakeMD5 = kInvalidPGPHashContextRef;
						}
						handshaking = FALSE;
					}
				}
				break;
			default:
				pgpAssert( 0 );
		}
		(void)pgpContextMemFree( session->pgpContext, packet );
		packet = NULL;
		if( IsPGPError( err ) )
			goto done;
		if( !session->blocking )
			break;
	}
done:
	if( IsntNull( packet ) )
		(void)pgpContextMemFree( session->pgpContext, packet );
	return err;
}

	PGPError
PGPtlsHandshake(
	PGPtlsSessionRef	ref )
{
	PGPError			err	= kPGPError_NoErr;
	PGPtlsSessionPriv *	session;
	
	PGPValidatePtr( ref );

	session = (PGPtlsSessionPriv *) ref;
	PGPValidatePtr( session->tlsReceiveProc );
	PGPValidatePtr( session->tlsSendProc );

	if( ( session->state != kPGPtls_ReadyState ) &&
		( session->state != kPGPtls_IdleState ) &&
		( ( session->state != kPGPtls_HandshakeState ) ||
		session->blocking ) )
	{
		err = kPGPError_TLSWrongState;
		goto done;
	}
	
	if( session->isClientSide )
		err = pgpTLSClientHandshake( session );
	else
		err = pgpTLSServerHandshake( session );
	if(err == kPGPError_TLSWouldBlock)
		return kPGPError_NoErr;
	if( IsPGPError( err ) && session->state != kPGPtls_FatalErrorState )
		(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
									kPGPtls_AT_InternalError );
done:
	return err;
}

	PGPError
PGPtlsClose(
	PGPtlsSessionRef	ref,
	PGPBoolean			dontCache )
{
	PGPError			err	= kPGPError_NoErr;
	PGPtlsSessionPriv *	session;
	
	PGPValidatePtr( ref );

	session = (PGPtlsSessionPriv *) ref;
	
	if( dontCache )
		session->state = kPGPtls_FatalErrorState;
	else if( session->state == kPGPtls_ReadyState )
	{
	#define kPGPtlsFlags_DummyPacketSize		16
		PGPByte	dummyPacket[kPGPtlsFlags_DummyPacketSize];
		PGPSize	packetSize = kPGPtlsFlags_DummyPacketSize;
		
		if( !session->closeInitiated )
			err = pgpTLSSendAlert( session, kPGPtls_AL_WarningAlert,
										kPGPtls_AT_CloseNotify ); CKERR;
		session->closeInitiated = TRUE;
		while( IsntPGPError( err ) &&
				( session->state != kPGPtls_ClosedState ) )
		{
			session->appDataSize = 0;
			err = PGPtlsReceive( ref, dummyPacket, &packetSize );
			if( !session->blocking )
				goto done;
		}
		/* Throw away any errors here because the other side may not
			have gotten their close through which is technically legal.  */
		err = kPGPError_NoErr;
	}
	if( session->state != kPGPtls_FatalErrorState )
		session->state = kPGPtls_ClosedState;
	
done:
	return err;
}

	PGPError
PGPtlsReceive(
	PGPtlsSessionRef		ref,
	void *					outBuffer,
	PGPSize *				bufferSize )
{
	PGPError				err	= kPGPError_NoErr;
	PGPtlsSessionPriv *		session;
	PGPByte					type;
	PGPByte *				buffer = NULL;
	PGPSize					pktLen,
							useLen,
							maxLen,
							outSize = 0;

	PGPValidatePtr( ref );
	PGPValidatePtr( outBuffer );
	
	session = (PGPtlsSessionPriv *) ref;
	PGPValidatePtr( session->tlsReceiveProc );
	
	if( session->state != kPGPtls_ReadyState )
	{
		err = kPGPError_TLSWrongState;
		goto done;
	}
	maxLen = *bufferSize;
	*bufferSize = 0;

	if( session->appDataSize > 0 )
	{
		if( maxLen < session->appDataSize )
			outSize = maxLen;
		else
			outSize = session->appDataSize;
		pgpCopyMemory( session->rcvdAppData, outBuffer, outSize );
		pgpCopyMemory( session->rcvdAppData + outSize,
						session->rcvdAppData,
						session->appDataSize - outSize );
		session->appDataSize -= outSize;
		err = pgpContextMemRealloc( session->pgpContext,
								(void **) &session->rcvdAppData,
								session->appDataSize, 0 ); CKERR;
		*bufferSize = outSize;
	}
	else
	{
		err = pgpTLSReceiveRecordLayer( session, &type, &buffer, &pktLen );
		CKERR;
		if( IsNull( buffer ) )
			goto done;
		switch( type )
		{
			case kPGPtls_RT_ApplicationData:
				if( pktLen > maxLen - outSize )
					useLen = maxLen - outSize;
				else
					useLen = pktLen;
				pgpCopyMemory( buffer, (PGPByte *)outBuffer + outSize, useLen );
				outSize += useLen;
				if( pktLen > useLen )
				{
					err = pgpContextMemRealloc( session->pgpContext,
									(void **) &session->rcvdAppData,
									session->appDataSize + pktLen - useLen,
									0 ); CKERR;
					pgpCopyMemory( buffer + useLen,
									session->rcvdAppData + session->appDataSize,
									pktLen - useLen );
					session->appDataSize += pktLen - useLen;
				}
				*bufferSize = outSize;
				break;
			case kPGPtls_RT_Alert:
				err = pgpTLSProcessAlert( session, buffer, pktLen ); CKERR;
				break;
			case kPGPtls_RT_Handshake:
				if( ( buffer[0] == kPGPtls_HT_HelloRequest ) &&
					session->isClientSide )
				{
					/* server side has requested renegotiation of the keys	*/
					/* start up the handshake								*/
					if( session->blocking )
						err = pgpTLSClientHandshake( session );
					else
						err = kPGPError_TLSRcvdHandshakeRequest;
					break;
				}
			default:
			case kPGPtls_RT_ChangeCipherSpec:
				(void)pgpTLSSendAlert( session,	kPGPtls_AL_FatalAlert,
											kPGPtls_AT_UnexpectedMessage );
				FATALTLS( kPGPError_TLSProtocolViolation );
		}
	}
done:
	if( IsntNull( buffer ) )
		(void)pgpContextMemFree( session->pgpContext, buffer );
	return err;
}

	PGPError
PGPtlsSend(
	PGPtlsSessionRef		ref,
	const void *			inBuffer,
	PGPSize					inBufferLength )
{
	PGPError				err	= kPGPError_NoErr;
	PGPtlsSessionPriv *		session;
	PGPSize					sentLength = 0;

	PGPValidatePtr( ref );
	PGPValidatePtr( inBuffer );

	session = (PGPtlsSessionPriv *) ref;
	PGPValidatePtr( session->tlsSendProc );
	
	if( session->state != kPGPtls_ReadyState )
	{
		err = kPGPError_TLSWrongState;
		goto done;
	}
	
	while( sentLength < inBufferLength &&
		( ( err == kPGPError_TLSWouldBlock ) || IsntPGPError( err ) ) )
	{
		PGPSize				sizeToSend;
		
		sizeToSend = inBufferLength - sentLength;
		if( sizeToSend > kPGPtls_MaximumDataSize )
			sizeToSend = kPGPtls_MaximumDataSize;
		err = pgpTLSSendRecordLayer( session, kPGPtls_RT_ApplicationData,
									(PGPByte *)inBuffer + sentLength,
									sizeToSend );
		sentLength += sizeToSend;
	}
done:
	return err;
}

	PGPError
pgpTLSSendQueueIdleInternal(
	PGPtlsSessionPriv *	session )
{
	PGPError			err = kPGPError_NoErr;
	PGPInt32			serr;
	
	if( IsntNull( session->queuedSendData ) && ( session->queuedSendSize > 0 ) )
	{
		serr = (session->tlsSendProc)( session->tlsSendUserData,
							session->queuedSendData,
							(PGPInt32) session->queuedSendSize );
		if( serr == kPGPError_TLSWouldBlock )
		{
			err = kPGPError_TLSWouldBlock;
		}
		else if( serr == (PGPInt32) session->queuedSendSize )
		{
			session->queuedSendSize = 0;
			err = pgpContextMemRealloc( session->pgpContext,
					(void **) &session->queuedSendData, 0, 0 ); CKERR;
		}
		else
			FATALTLS( kPGPError_TLSUnexpectedClose );
	}
done:
	return err;
}

	PGPError
PGPtlsSendQueueIdle(
	PGPtlsSessionRef	ref )
{
	PGPtlsSessionPriv *	session;
	PGPError			err = kPGPError_NoErr;

	PGPValidatePtr( ref );
	
	session = (PGPtlsSessionPriv *) ref;
	
	err = pgpTLSSendQueueIdleInternal( session );
		
	return err;
}

	PGPError
PGPtlsSetReceiveCallback(
	PGPtlsSessionRef		ref,
	PGPtlsReceiveProcPtr	tlsReceiveProc,
	void *					inData )
{
	PGPtlsSessionPriv *	session;

	PGPValidatePtr( ref );
	PGPValidatePtr( tlsReceiveProc );
	
	session = (PGPtlsSessionPriv *) ref;
	session->tlsReceiveProc		=	tlsReceiveProc;
	session->tlsReceiveUserData	=	inData;
	
	return kPGPError_NoErr;
}

	PGPError
PGPtlsSetSendCallback(
	PGPtlsSessionRef		ref,
	PGPtlsSendProcPtr		tlsSendProc,
	void *					inData )
{
	PGPtlsSessionPriv *	session;

	PGPValidatePtr( ref );
	PGPValidatePtr( tlsSendProc );
	
	session = (PGPtlsSessionPriv *) ref;
	session->tlsSendProc		=	tlsSendProc;
	session->tlsSendUserData	=	inData;
	
	return kPGPError_NoErr;
}

	PGPError
PGPtlsSetProtocolOptions(
	PGPtlsSessionRef		ref,
	PGPtlsFlags				options )
{
	PGPtlsSessionPriv *	session;

	PGPValidatePtr( ref );
	
	session = (PGPtlsSessionPriv *) ref;
	
	if( options & kPGPtlsFlags_ServerSide )
		session->isClientSide = FALSE;
	if( options & kPGPtlsFlags_ClientSide )
		session->isClientSide = TRUE;
	if( options & kPGPtlsFlags_RequestClientCert )
		session->requestCert = TRUE;
	if( options & kPGPtlsFlags_NonBlockingIO )
		session->blocking = FALSE;

	return kPGPError_NoErr;
}

static const PGPOptionType sTLSOptionSet[] =
{
	kPGPOptionType_Passphrase,
	kPGPOptionType_Passkey
};

	PGPError
PGPtlsSetLocalPrivateKey(
	PGPtlsSessionRef		ref,
	PGPKeyRef				inKey,
	PGPOptionListRef		firstOption,
	... )
{
	PGPError				err	= kPGPError_NoErr;
	PGPtlsSessionPriv *		session;
	PGPBoolean				secret,
							disabled,
							revoked,
							expired,
							canSign;
	PGPInt32				algID;
	void					*passedPhrase = NULL;
	PGPSize					passedLength;
	PGPOptionListRef		optionList = kInvalidPGPOptionListRef;
	va_list					args;

	PGPValidatePtr( ref );
	PGPValidatePtr( inKey );

	session = (PGPtlsSessionPriv *) ref;
	
	va_start( args, firstOption );
	optionList = pgpBuildOptionListArgs(session->pgpContext,
									FALSE, firstOption, args);
	va_end( args );
	
	pgpAssert( pgpOptionListIsValid( optionList ) );
	err = pgpGetOptionListError( optionList );	CKERR;
	err = pgpCheckOptionsInSet( optionList, sTLSOptionSet,
				sizeof(sTLSOptionSet) / sizeof(PGPOptionType) ); CKERR;
	
	err = PGPGetKeyBoolean( inKey, kPGPKeyPropIsSecret, &secret ); CKERR;
	err = PGPGetKeyBoolean( inKey, kPGPKeyPropIsDisabled, &disabled ); CKERR;
	err = PGPGetKeyBoolean( inKey, kPGPKeyPropIsExpired, &expired ); CKERR;
	err = PGPGetKeyBoolean( inKey, kPGPKeyPropIsRevoked, &revoked ); CKERR;
	err = PGPGetKeyBoolean( inKey, kPGPKeyPropCanSign, &canSign ); CKERR;
	
	if( !secret || disabled || expired || revoked || !canSign )
	{
		err = kPGPError_TLSKeyUnusable;
		goto done;
	}
	err = PGPGetKeyNumber( inKey, kPGPKeyPropAlgID, &algID ); CKERR;
	
	session->localKey = inKey;
	session->localKeyAlg = (PGPPublicKeyAlgorithm)algID;
	
	err = pgpFindOptionArgs( optionList, kPGPOptionType_Passphrase, FALSE,
						"%p%l", &passedPhrase, &passedLength ); CKERR;
	if( IsNull( passedPhrase ) )
	{
		err = pgpFindOptionArgs( optionList, kPGPOptionType_Passkey, FALSE,
						"%p%l", &passedPhrase, &passedLength ); CKERR;
		session->localKeyPasskeyBuffer = PGPNewSecureData( session->memMgr,
										passedLength,
										kPGPMemoryMgrFlags_Clear );
		if( IsNull( session->localKeyPasskeyBuffer ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		pgpCopyMemory( passedPhrase, session->localKeyPasskeyBuffer,
						passedLength );
		session->localKeyUsePasskey = TRUE;
	}
	else
	{
		session->localKeyPassphrase = PGPNewSecureData( session->memMgr,
										passedLength + 1,
										kPGPMemoryMgrFlags_Clear );
		if( IsNull( session->localKeyPassphrase ) )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}
		pgpCopyMemory( passedPhrase, session->localKeyPassphrase,
					passedLength );
		session->localKeyPassphrase[passedLength] = '\0';
		session->localKeyUsePasskey = FALSE;
	}
	
done:
	/* do not explicitly free passedPhrase, it is freed with the OptionList */
	if( PGPOptionListRefIsValid( optionList ) )
		PGPFreeOptionList( optionList );
	return err;
}
					
	PGPError
PGPtlsSetPreferredCipherSuite(
	PGPtlsSessionRef		ref,
	PGPtlsCipherSuiteNum	cipher )
{
	PGPError				err	= kPGPError_NoErr;
	PGPtlsSessionPriv *		session;
	PGPUInt16				suiteIndex;
	PGPBoolean				found = FALSE;

	PGPValidatePtr( ref );
	session = (PGPtlsSessionPriv *) ref;
	
	if( session->state == kPGPtls_IdleState )
	{
		for( suiteIndex = 0; suiteIndex < session->numCipherSuites;
				suiteIndex++ )
		{
			if( cipher == session->cipherSuites[ suiteIndex ].suite )
			{
				found = TRUE;
				break;
			}
		}
		if( found )
		{
			PGPtlsCipherSuite	saved;
			
			saved = session->cipherSuites[ suiteIndex ];
			for( ; suiteIndex > 0; suiteIndex-- )
				session->cipherSuites[ suiteIndex ] =
					session->cipherSuites[ suiteIndex - 1 ];
			session->cipherSuites[ 0 ] = saved;
		}
		else
			err = kPGPError_ItemNotFound;
	}
	else
		err = kPGPError_TLSWrongState;
	
	return err;
}

	PGPError
PGPtlsSetRemoteUniqueID(
	PGPtlsSessionRef		ref,
	PGPUInt32				remoteID )
{
	(void) ref;
	(void) remoteID;
	
	/* ##### */
	return kPGPError_NoErr;
}

	PGPError
PGPtlsSetDHPrime(
	PGPtlsSessionRef	ref,
	PGPtlsPrime			prime )
{
	PGPError			err	= kPGPError_NoErr;
	PGPtlsSessionPriv *	session;

	PGPValidatePtr( ref );
	session = (PGPtlsSessionPriv *) ref;
	pgpAssert(	prime == kPGPtls_DHPrime1024 || prime == kPGPtls_DHPrime1536 ||
				prime == kPGPtls_DHPrime2048 || prime == kPGPtls_DHPrime3072 ||
				prime == kPGPtls_DHPrime4096 );
	
	if( session->state == kPGPtls_IdleState )
		session->requestedPrime = prime;
	else
		err = kPGPError_TLSWrongState;
	
	return err;
}

	PGPError
PGPtlsGetRemoteAuthenticatedKey(
	PGPtlsSessionRef	ref,
	PGPKeyRef *			outKey )
{
	PGPError			err	= kPGPError_NoErr;
	PGPtlsSessionPriv *	session;

	PGPValidatePtr( ref );
	session = (PGPtlsSessionPriv *) ref;
	
	*outKey = kInvalidPGPKeyRef;
	if( session->state == kPGPtls_ReadyState )
		*outKey = session->remoteKey;
	else
		err = kPGPError_TLSWrongState;
	
	return err;
}

	PGPError
PGPtlsGetNegotiatedCipherSuite(
	PGPtlsSessionRef		ref,
	PGPtlsCipherSuiteNum *	outCipher )
{
	PGPError				err	= kPGPError_NoErr;
	PGPtlsSessionPriv *		session;

	PGPValidatePtr( ref );
	session = (PGPtlsSessionPriv *) ref;
	
	*outCipher = kPGPtls_TLS_NULL_WITH_NULL_NULL;
	if( session->state == kPGPtls_ReadyState )
		*outCipher = session->cipherSuites[session->cipherSuiteIndex].suite;
	else
		err = kPGPError_TLSWrongState;
	
	return err;
}

	PGPError
PGPtlsGetState(
	PGPtlsSessionRef		ref,
	PGPtlsProtocolState *	outState )
{
	PGPtlsSessionPriv *		session;

	PGPValidatePtr( ref );
	PGPValidatePtr( outState );
	session = (PGPtlsSessionPriv *) ref;
	
	*outState = session->state;
	
	return kPGPError_NoErr;
}

	PGPError
PGPtlsGetAlert(
	PGPtlsSessionRef	ref,
	PGPtlsAlert *		outAlert )
{
	PGPtlsSessionPriv *	session;

	PGPValidatePtr( ref );
	PGPValidatePtr( outAlert );
	session = (PGPtlsSessionPriv *) ref;
	
	*outAlert = session->fatalAlert;
	
	return kPGPError_NoErr;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/