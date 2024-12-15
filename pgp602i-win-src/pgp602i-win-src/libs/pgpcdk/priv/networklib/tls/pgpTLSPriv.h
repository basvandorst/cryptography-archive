/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	$Id: pgpTLSPriv.h,v 1.28.2.1.2.1 1998/11/12 03:22:51 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpTLSPriv_h	/* [ */
#define Included_pgpTLSPriv_h

#include "pgpTLS.h"
#include "pgpMemoryMgr.h"
#include "pgpBigNum.h"

#define	kPGPtls_MajorVersion				3
#define	kPGPtls_MinorVersion				1
#define	kPGPtls_SSL3MinorVersion			0

#define	kPGPtls_MaximumPacketSize			18432/* 2^14+2048 ::6.2.3 */
#define kPGPtls_MaximumDataSize				16384
#define	kPGPtls_RecordHeaderSize			5
#define	kPGPtls_RandomSeedSize				28
#define	kPGPtls_RandomSize					32
#define	kPGPtls_MasterSecretSize			48
#define	kPGPtls_CipherBlockSize				8
#define kPGPtls_FinishedSize				12
#define	kPGPtls_SSL3MD5PaddingBytes			48
#define	kPGPtls_SSL3SHAPaddingBytes			40
#define	kPGPtls_ipad						0x36
#define	kPGPtls_opad						0x5C
#define kPGPtls_MaxHashSize					20
#define	kPGPtls_SequenceNumberSize			8


/* Record Types */
#define	kPGPtls_RT_ChangeCipherSpec			20
#define	kPGPtls_RT_Alert					21
#define	kPGPtls_RT_Handshake				22
#define	kPGPtls_RT_ApplicationData			23

/* Handshake Types */
#define	kPGPtls_HT_HelloRequest				0
#define	kPGPtls_HT_ClientHello				1
#define	kPGPtls_HT_ServerHello				2
#define	kPGPtls_HT_Certificate				11
#define	kPGPtls_HT_ServerKeyExchange		12
#define	kPGPtls_HT_CertificateRequest		13
#define	kPGPtls_HT_ServerHelloDone			14
#define	kPGPtls_HT_CertificateVerify		15
#define	kPGPtls_HT_ClientKeyExchange		16
#define	kPGPtls_HT_Finished					20

/* Alert Levels */
#define	kPGPtls_AL_FatalAlert				2
#define	kPGPtls_AL_WarningAlert				1

/* Compression Types */
#define	kPGPtls_CT_None						0

/* PGP Key Identifier Types */
#define	kPGPtls_KT_KeyID					0
#define	kPGPtls_KT_Key						1

#define	kPGPtls_HA_MD5Size					16
#define	kPGPtls_HA_SHASize					20
#define	kPGPtls_HA_RIPEMD160Size			20

#define	kPGPtls_SC_CASTKeySize				16
#define	kPGPtls_SC_IDEAKeySize				16
#define	kPGPtls_SC_3DESKeySize				24

#define kPGPtls_CC_RSASign					1
#define kPGPtls_CC_DSASign					2
#define kPGPtls_CC_RSAFixedDH				3
#define kPGPtls_CC_DSAFixedDH				4

/* Internal Events */
#define	kPGPtls_EV_ReceiveHelloRequest		0
#define kPGPtls_EV_ReceiveClientHello		1
#define kPGPtls_EV_ReceiveServerHello		2
#define kPGPtls_EV_ReceiveCertificate		3
#define kPGPtls_EV_EphemeralCertificate		4
#define kPGPtls_EV_StaticCertificate		5
#define kPGPtls_EV_ReceiveServerKeyExchange	6
#define kPGPtls_EV_ReceiveCertRequest		7
#define kPGPtls_EV_ReceiveServerHelloDone	8
#define kPGPtls_EV_ReceiveCertVerify		9
#define kPGPtls_EV_ReceiveClientKeyExchange	10
#define kPGPtls_EV_ReceiveFinished			11
#define kPGPtls_EV_ReceiveFatalAlert		12
#define kPGPtls_EV_ReceiveNoCertificate		13
#define kPGPtls_EV_ReceiveChangeCipherSpec	14


typedef PGPByte PGPtlsCipherSuiteID[2];

typedef struct PGPtlsCipherSuite
{
	PGPtlsCipherSuiteID		id;
	PGPPublicKeyAlgorithm	sigAlg;
	PGPCipherAlgorithm		cipherID;
	PGPHashAlgorithm		hashID;
	PGPBoolean				ephemeral;
	PGPBoolean				usePGPKeys;
	PGPtlsCipherSuiteNum	suite;
} PGPtlsCipherSuite;

/* Cipher Suites (in order of preference) */
const PGPtlsCipherSuite kPGPtls_CipherSuites[] =
	{
		{
			{ 0x01, 0x01 },
			kPGPPublicKeyAlgorithm_DSA,
			kPGPCipherAlgorithm_CAST5,
			kPGPHashAlgorithm_SHA, TRUE, TRUE,
			kPGPtls_TLS_PGP_DHE_DSS_WITH_CAST_CBC_SHA
		},
		{
			{ 0x01, 0x10 },
			kPGPPublicKeyAlgorithm_RSA,
			kPGPCipherAlgorithm_CAST5,
			kPGPHashAlgorithm_SHA, TRUE, TRUE,
			kPGPtls_TLS_PGP_DHE_RSA_WITH_CAST_CBC_SHA
		},
		{
			{ 0x01, 0x20 },
			kPGPPublicKeyAlgorithm_RSA,
			kPGPCipherAlgorithm_CAST5,
			kPGPHashAlgorithm_SHA, FALSE, TRUE,
			kPGPtls_TLS_PGP_RSA_WITH_CAST_CBC_SHA
		},
		{
			{ 0x00, 0x0A },
			kPGPPublicKeyAlgorithm_RSA,
			kPGPCipherAlgorithm_3DES,
			kPGPHashAlgorithm_SHA, FALSE, FALSE,
			kPGPtls_TLS_RSA_WITH_3DES_EDE_CBC_SHA
		},
		{
			{ 0x00, 0x13 },
			kPGPPublicKeyAlgorithm_DSA,
			kPGPCipherAlgorithm_3DES,
			kPGPHashAlgorithm_SHA, TRUE, FALSE,
			kPGPtls_TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA
		},
		{
			{ 0x00, 0x16 },
			kPGPPublicKeyAlgorithm_RSA,
			kPGPCipherAlgorithm_3DES,
			kPGPHashAlgorithm_SHA, TRUE, FALSE,
			kPGPtls_TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA
		},
		{
			{ 0x00, 0x07 },
			kPGPPublicKeyAlgorithm_RSA,
			kPGPCipherAlgorithm_IDEA,
			kPGPHashAlgorithm_SHA, FALSE, FALSE,
			kPGPtls_TLS_RSA_WITH_IDEA_CBC_SHA
		},
		{
			{ 0x01, 0xF0 },
			kPGPPublicKeyAlgorithm_DSA,
			kPGPCipherAlgorithm_None,
			kPGPHashAlgorithm_SHA, TRUE, TRUE,
			kPGPtls_TLS_PGP_DHE_DSS_WITH_NULL_SHA
		}
	};
		

typedef struct PGPtlsKeys
{
	PGPByte				macSecret[kPGPtls_MaxHashSize];
	PGPByte				cipherKey[24];
	PGPByte				iv[kPGPtls_CipherBlockSize];
	PGPByte				seqNum[kPGPtls_SequenceNumberSize];
	PGPHMACContextRef	hmac;
} PGPtlsKeys;

/* the non-opaque PGPtlsContext */
typedef struct PGPtlsContextPriv
{
	PGPContextRef			pgpContext;
	PGPBoolean				useCache;
	
	/* cached session keys stored here */
} PGPtlsContextPriv;

typedef struct PGPtlsSessionPriv
{
	PGPtlsProtocolState		state;
	PGPInt8					intState;
	PGPContextRef			pgpContext;
	PGPtlsContextPriv		*tlsContext;
	PGPMemoryMgrRef			memMgr;
	
	PGPtlsReceiveProcPtr	tlsReceiveProc;
	PGPtlsSendProcPtr		tlsSendProc;
	void *					tlsReceiveUserData;
	void *					tlsSendUserData;
		
	PGPBoolean				isClientSide;
	PGPBoolean				blocking;
	PGPBoolean				closeInitiated;
	PGPBoolean				requestCert;
	PGPByte					minorVersion;
	PGPByte					originalClientVersion;
	PGPUInt16				cipherSuiteIndex;
	PGPtlsCipherSuite		*cipherSuites;
	PGPUInt16				numCipherSuites;
	
	PGPKeyRef				localKey;
	PGPPublicKeyAlgorithm	localKeyAlg;
	char *					localKeyPassphrase;
	PGPByte *				localKeyPasskeyBuffer;
	PGPSize					localKeyPasskeySize;
	PGPBoolean				localKeyUsePasskey;
	PGPKeyRef				remoteKey;
	PGPKeySetRef			remoteKeySet;
	PGPPublicKeyAlgorithm	remoteKeyAlg;
	PGPBoolean				certRequested;
	PGPBoolean				certMatched;
	PGPBoolean				certVerified;
	
	PGPtlsPrime				requestedPrime;
	
	PGPHashContextRef		handshakeSHA;
	PGPHashContextRef		handshakeMD5;
	PGPHashContextRef		lastHandSHA;
	PGPHashContextRef		lastHandMD5;
	PGPHashContextRef		innerMACHash;
	PGPHashContextRef		outerMACHash;
	
	PGPByte					cRandom[kPGPtls_RandomSize];
	PGPByte					sRandom[kPGPtls_RandomSize];
	
	PGPByte					masterSecret[kPGPtls_MasterSecretSize];
	
	PGPCBCContextRef		writeCipher;
	PGPCBCContextRef		readCipher;
	
	PGPtlsKeys *			writeActive;
	PGPtlsKeys *			readActive;
	PGPtlsKeys *			clientPending;
	PGPtlsKeys *			serverPending;
	PGPSize					keySize;
	PGPSize					hashSize;
	
	PGPByte	*				queuedSendData;
	PGPSize					queuedSendSize;
	PGPByte *				rcvdRawData;	/* only used for non-blocking */
	PGPSize					rawDataSize;	/* only used for non-blocking */
	PGPByte *				rcvdAppData;
	PGPSize					appDataSize;
	PGPByte *				rcvdHandData;
	PGPSize					handDataSize;
	
	/* DH Parameters */
	PGPBigNumRef			dhP;
	PGPBigNumRef			dhG;
	PGPBigNumRef			dhYs;
	PGPBigNumRef			dhX;
	PGPBigNumRef			dhYc;
	
	PGPtlsAlert				fatalAlert;
} PGPtlsSessionPriv;


PGPError pgpTLSReceiveRecordLayer( PGPtlsSessionPriv *	session,
						PGPByte *			outType,
						PGPByte **			outBuffer,
						PGPSize *			outLength );
PGPError pgpTLSSendRecordLayer( PGPtlsSessionPriv *	session,
						const PGPByte		inType,
						const PGPByte *		inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSReceiveHandshakePacket( PGPtlsSessionPriv *	session,
						PGPByte *			outType,
						PGPByte **			outBuffer,
						PGPSize *			outLength );
PGPError pgpTLSSendHandshakePacket( PGPtlsSessionPriv *	session,
						const PGPByte		inType,
						const PGPByte *		inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSClientHandshake( PGPtlsSessionPriv *	session );
PGPError pgpTLSServerHandshake( PGPtlsSessionPriv *	session );

PGPError pgpTLSSendClientHello( PGPtlsSessionPriv *	session );
PGPError pgpTLSSendServerHello( PGPtlsSessionPriv *	session );
PGPError pgpTLSSendCertificate( PGPtlsSessionPriv *	session );
PGPError pgpTLSSendCertVerify( PGPtlsSessionPriv *	session );
PGPError pgpTLSSendServerKeyExchange( PGPtlsSessionPriv * session );
PGPError pgpTLSSendCertificateRequest( PGPtlsSessionPriv *	session );
PGPError pgpTLSSendClientKeyExchange( PGPtlsSessionPriv * session );
PGPError pgpTLSSendChangeCipherSpec( PGPtlsSessionPriv *	session );
PGPError pgpTLSSendFinished( PGPtlsSessionPriv *	session );
PGPError pgpTLSSendFinalHandshake(  PGPtlsSessionPriv *	session );
PGPError pgpTLSSendAlert( PGPtlsSessionPriv *	session,
						PGPByte				level,
						PGPtlsAlert			type );

PGPError pgpTLSReceiveServerHello( PGPtlsSessionPriv *	session,
						PGPByte *			inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSReceiveClientHello( PGPtlsSessionPriv *	session,
						PGPByte *			inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSReceiveCertificate( PGPtlsSessionPriv *	session,
						PGPByte *			inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSReceiveServerKeyExchange( PGPtlsSessionPriv *session,
						PGPByte *			inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSReceiveClientKeyExchange( PGPtlsSessionPriv *session,
						PGPByte *			inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSReceiveCertRequest( PGPtlsSessionPriv *	session,
						PGPByte *			inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSReceiveCertVerify( PGPtlsSessionPriv *	session,
						PGPByte *			inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSReceiveServerHelloDone( PGPtlsSessionPriv *	session,
						PGPByte *			inBuffer,
						const PGPSize		inLength );
PGPError pgpTLSReceiveFinished( PGPtlsSessionPriv *	session,
						PGPByte *			inBuffer,
						const PGPSize		inLength );

PGPError ssl3MasterSecret16( PGPtlsSessionPriv *	session,
						PGPByte *			preMaster,
						PGPUInt16			preMasterSize,
						PGPByte				mixer,
						PGPByte *			masterSecret );
PGPError ssl3CalculateKeyBlock( PGPtlsSessionPriv *	session,
						PGPByte *			keyBlock,
						PGPSize				needed );
PGPError tlsCalculateKeyBlock( PGPtlsSessionPriv *	session,
						PGPByte *			keyBlock,
						PGPSize				needed );
PGPError pgpTLSCalculateMasterSecret( PGPtlsSessionPriv *	session,
						PGPByte *			preMaster,
						PGPSize				preMasterSize );
PGPError pgpTLSCalculateKeys( PGPtlsSessionPriv *	session );
PGPError pgpTLSCalculateMAC( PGPtlsSessionPriv *	session,
						const PGPBoolean	inSending,
						const PGPByte		inType,
						const PGPByte *		inBuffer,
						const PGPSize		inLength,
						PGPByte *			outBuffer );
PGPError pgpTLSCalculateCertVerify( PGPtlsSessionPriv *	session,
						PGPByte *			outBuffer,
						PGPSize *			outSize );
PGPError pgpTLSCalculateFinished( PGPtlsSessionPriv *	session,
						PGPBoolean			clientSide,
						PGPByte *			inBuffer,
						PGPSize *			outSize );
PGPError pgpTLSPRF(
						PGPtlsSessionPriv *	session,
						PGPByte *			secret,
						PGPSize				secretLen,
						const char *		label,
						PGPByte *			seedData,
						PGPSize				seedLen,
						PGPByte *			outputData,
						PGPSize				outputLen );
PGPError pgpTLSPRFTest(
						PGPtlsSessionPriv *	session );
PGPError pgpTLSActivateReadKeys( PGPtlsSessionPriv *	session );
PGPError pgpTLSActivateWriteKeys( PGPtlsSessionPriv *	session );
PGPError pgpTLSProcessAlert( PGPtlsSessionPriv *	session,
						PGPByte *			inBuffer,
						PGPSize				inLength );
PGPInt8 pgpTLSNextState( PGPtlsSessionPriv *	session,
						PGPByte				event );
PGPInt8 pgpTLSPacketToEvent( PGPByte		packetType );
PGPError pgpTLSBufferRawData( PGPtlsSessionPriv *	session,
						PGPByte *			rawData,
						PGPSize				rawDataSize );
PGPError pgpTLSExtractRawData( PGPtlsSessionPriv *	session,
						PGPByte *			rawData,
						PGPSize *			rawDataSize );
PGPError pgpTLSBufferHandData(
						PGPtlsSessionPriv *	session,
						PGPByte *			handData,
						PGPSize				handDataSize );
PGPError pgpTLSExtractHandData(
						PGPtlsSessionPriv *	session,
						PGPByte **			handData,
						PGPSize *			handDataSize );
PGPError pgpTLSBufferSendData( PGPtlsSessionPriv *	session,
						PGPByte *			sendData,
						PGPSize				sendDataSize );
PGPError pgpTLSSendQueueIdleInternal( PGPtlsSessionPriv *	session );

PGPBoolean pubAlgSupported( PGPPublicKeyAlgorithm		pubAlg );

#endif /* ] Included_pgpTLSPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
