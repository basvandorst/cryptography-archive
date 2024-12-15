/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	$Id: pgpIKEPriv.h,v 1.66.6.1 1999/06/15 04:33:25 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIKEPriv_h	/* [ */
#define Included_pgpIKEPriv_h

#include "pgpIKE.h"
#include "pgpIKEPrimes.h"
#include "pgpMemoryMgr.h"
#include "pgpBigNum.h"

#define CKERR			if( err ) {goto done;}


#define kPGPike_Magic					0x74052696	/* random */
#define kPGPike_Version					0x10	/* Major = 1, Minor = 0 */
#define kPGPike_IPProtocolUDP			17
#define kPGPike_CookieSize				8
#define kPGPike_MessageIDSize			4
#define kPGPike_ISAKMPHeaderSize		28
#define kPGPike_ISAKMPLengthPos			24
#define kPGPike_ISAKMPPayloadHeaderSize	4
#define kPGPike_DefaultPacketAlloc		65535L
#define kPGPike_Key_IKE_Transform		1
#define kPGPike_SecLifeRekeySlack		30
#define kPGPike_SecLifeRespSlack		6
#define kPGPike_KBLifeMinimum			128
#define kPGPike_MODPGroupType			1
#define kPGPike_MaxRetries				4
#define KPGPike_MaxExchanges			192
#define kPGPike_MaxPropTransforms		16
#define kPGPike_MinNonceSize			8
#define kPGPike_MaxNonceSize			256
#define kPGPike_NonceSize				32
#define kPGPike_RoundTripSlack			1200	/* 1.2 seconds */
#define kPGPike_SADiedSlack				10		/* 10 seconds */
#define kPGPike_CommitBitSlack			60000	/* 60 seconds */

#define kPGPike_MaxHashSize				20
#define kPGPike_MD5HashSize				16
#define kPGPike_SHAHashSize				20

#define kPGPike_MaxKeySize				24
#define kPGPike_MaxCipherBlockSize		8
#define	kPGPike_CASTKeySize				16
#define	kPGPike_IDEAKeySize				16
#define	kPGPike_3DESKeySize				24
#define kPGPike_DESKeySize				8

#define kPGPike_ISAKMPEncryptBit		1
#define kPGPike_ISAKMPCommitBit			2
#define kPGPike_ISAKMPAuthOnlyBit		4

#define kPGPike_IPSEC_SIT_IdentityOnly	1
#define kPGPike_IPSEC_SIT_Secrecy		2
#define kPGPike_IPSEC_SIT_Integrity		4


typedef PGPByte	PGPikeMessageID[kPGPike_MessageIDSize];
typedef PGPByte	PGPikeCookie[kPGPike_CookieSize];


enum PGPikeState_
{
	kPGPike_S_None			= 0,
	
	kPGPike_S_MM_WaitSA,		/* Main Mode */
	kPGPike_S_MM_WaitKE,
	kPGPike_S_MM_WaitFinal,
	
	kPGPike_S_QM_WaitSA,		/* Quick Mode */
	kPGPike_S_QM_WaitHash3,
	
	kPGPike_S_ND_Informational,
	
	PGP_ENUM_FORCE( PGPikeState_ )
};
PGPENUM_TYPEDEF( PGPikeState_, PGPikeState );

enum PGPikeExchangeT_
{
	kPGPike_EX_None				= 0,
	kPGPike_EX_Base				= 1,
	kPGPike_EX_Identity			= 2,
	kPGPike_EX_Authentication	= 3,
	kPGPike_EX_Aggressive		= 4,
	kPGPike_EX_Informational	= 5,
	
	/* IPSEC DOI */
	kPGPike_EX_IPSEC_Quick		= 32,
	kPGPike_EX_IPSEC_NewGroup	= 33,

	PGP_ENUM_FORCE( PGPikeExchangeT_ )
};
PGPENUM_TYPEDEF( PGPikeExchangeT_, PGPikeExchangeT );

enum PGPikePayload_
{
	kPGPike_PY_None				= 0,
	kPGPike_PY_SA				= 1,	/* T */
	kPGPike_PY_Proposal			= 2,
	kPGPike_PY_Transform		= 3,
	kPGPike_PY_KeyExchange		= 4,	/* T */
	kPGPike_PY_Identification	= 5,	/* T */
	kPGPike_PY_Certificate		= 6,	/* T */
	kPGPike_PY_CertRequest		= 7,	/* T */
	kPGPike_PY_Hash				= 8,	/* T */
	kPGPike_PY_Signature		= 9,	/* T */
	kPGPike_PY_Nonce			= 10,	/* T */
	kPGPike_PY_Notification		= 11,
	kPGPike_PY_Delete			= 12,
	kPGPike_PY_VendorID			= 13,	/* T */
	
	kPGPike_PY_Skip				= 1024,
	
	PGP_ENUM_FORCE( PGPikePayload_ )
};
PGPENUM_TYPEDEF( PGPikePayload_, PGPikePayload );

enum PGPikeCertEncoding_
{
	kPGPike_CE_None				= 0,
	kPGPike_CE_X509_PKCS7		= 1,
	kPGPike_CE_PGP				= 2,
	kPGPike_CE_DNSSig			= 3,
	kPGPike_CE_X509_Sig			= 4,
	kPGPike_CE_X509_Exchange	= 5,
	kPGPike_CE_KerberosToken	= 6,
	kPGPike_CE_CRL				= 7,
	kPGPike_CE_ARL				= 8,
	kPGPike_CE_SPKI				= 9,
	kPGPike_CE_X509_Attrib		= 10,
	
	PGP_ENUM_FORCE( PGPikeCertEncoding_ )
};
PGPENUM_TYPEDEF( PGPikeCertEncoding_, PGPikeCertEncoding );

enum PGPikeAttributeType_
{
	kPGPike_AT_None					= 0,
	
	kPGPike_AT_EncryptAlg			= 1,	/* B */
	kPGPike_AT_HashAlg				= 2,	/* B */
	kPGPike_AT_AuthMethod			= 3,	/* B */
	kPGPike_AT_GroupDesc			= 4,	/* B */
	kPGPike_AT_GroupType			= 5,	/* B */
	kPGPike_AT_GroupPrime			= 6,	/* V */
	kPGPike_AT_GroupGenOne			= 7,	/* V */
	kPGPike_AT_GroupGenTwo			= 8,	/* V */
	kPGPike_AT_GroupCurveA			= 9,	/* V */
	kPGPike_AT_GroupCurveB			= 10,	/* V */
	kPGPike_AT_LifeType				= 11,	/* B */
	kPGPike_AT_LifeDuration			= 12,	/* V */
	kPGPike_AT_PRF					= 13,	/* B */
	kPGPike_AT_KeyLength			= 14,	/* B */
	kPGPike_AT_FieldSize			= 15,	/* B */
	kPGPike_AT_GroupOrder			= 16,	/* V */
		
	/* IPSEC Attributes */
	kPGPike_AT_IPSEC_LifeType		= 1,	/* B */
	kPGPike_AT_IPSEC_Duration		= 2,	/* V */
	kPGPike_AT_IPSEC_GroupDesc		= 3,	/* B */
	kPGPike_AT_IPSEC_Encapsulation	= 4,	/* B */
	kPGPike_AT_IPSEC_AuthAttr		= 5,	/* B */
	kPGPike_AT_IPSEC_KeyLength		= 6,	/* B */
	kPGPike_AT_IPSEC_KeyRounds		= 7,	/* B */
	kPGPike_AT_IPSEC_CompressSize	= 8,	/* B */
	kPGPike_AT_IPSEC_CompressAlg	= 9,	/* V */
	
	PGP_ENUM_FORCE( PGPikeAttributeType_ )
};
PGPENUM_TYPEDEF( PGPikeAttributeType_, PGPikeAttributeType );

typedef struct PGPikeNDPayload
{
	struct PGPikeNDPayload *	nextND;
	PGPikePayload				payType;
	PGPSize						payLen;
	PGPByte *					pay;
} PGPikeNDPayload;

typedef struct PGPikeDestination
{
	struct PGPikeDestination *	nextD;
	PGPUInt32					ipAddress;
	PGPBoolean					destIsRange;
	PGPUInt32					ipAddrStart;
	PGPUInt32					ipMaskEnd;
} PGPikeDestination;

typedef struct PGPikeGenericTransform
{
	union
	{
		PGPikeTransform		ike;
		PGPipsecTransform	ipsec;
	} u;
} PGPikeGenericTransform;

typedef struct PGPikeProposal
{
	struct PGPikeProposal *	nextProposal;
	
	PGPUInt8				number;
	PGPipsecProtocol		protocol;
	PGPipsecSPI				initSPI;
	PGPipsecSPI				respSPI;
	PGPUInt8				numTransforms;
	PGPikeGenericTransform	t[kPGPike_MaxPropTransforms];
	PGPUInt32				kbLifeTime;
	PGPUInt32				secLifeTime;
} PGPikeProposal;

/* Each in-progress exchange is represented by this
	structure */
typedef struct PGPikeExchange
{
	struct PGPikeExchange *		nextExchange;
	struct PGPikeContextPriv *	ike;
	struct PGPikePartner *		partner;
	
	PGPikeExchangeT				exchangeT;
	PGPBoolean					initiator;
	PGPBoolean					needsCommit;	/* for the commit bit */
	PGPBoolean					complete;		/* for the commit bit */
	PGPikeState					state;
	PGPikeDOI					doi;
	PGPikeMessageID				messageID;
	
	PGPikeProposal *			proposals;
	
	PGPByte						lastCBC[kPGPike_MaxCipherBlockSize];

	/* Key Exchange */
	PGPBigNumRef				dhP;		/* prime			*/
	PGPBigNumRef				dhG;		/* generator		*/
	PGPBigNumRef				dhX;		/* secret X			*/
	PGPBigNumRef				dhYi;		/* initiator g^x	*/
	PGPBigNumRef				dhYr;		/* responder g^x	*/
	PGPByte *					gXY;		/* shared secret	*/
	PGPSize						gXYLen;		/* length of g^xy	*/
	
	/* Nonce */
	PGPByte						initNonce[kPGPike_MaxNonceSize];
	PGPSize						initNonceLen;
	PGPByte						respNonce[kPGPike_MaxNonceSize];
	PGPSize						respNonceLen;
	
	/* Needed for HASH_I and HASH_R */
	PGPByte *					initSABody;
	PGPSize						initSABodySize;
	PGPByte *					idBody;		/* doubles as P2 IDci */
	PGPSize						idBodySize;
	PGPBoolean					checkedIDci;
	PGPByte *					idRBody;	/* P2 IDcr */
	PGPSize						idRBodySize;
	PGPBoolean					checkedIDcr;
		
	/* Retransmission */
	PGPUInt16					retry;
	PGPUInt32					lastTransmit;	/* milliseconds */
	PGPByte *					lastPkt;
	PGPSize						lastPktSize;
	PGPUInt32					lastRcvdCksm;
	PGPUInt16					lastRcvdLength;
	
	PGPikeAlert					outAlert;
	PGPipsecProtocol			outInfoProtocol;
	PGPUInt16					outInfoSPICount;
	PGPipsecSPI					outInfoSPI[kPGPike_MaxTransforms];
	PGPBoolean					outRLSeconds;
	PGPBoolean					outRLKB;
} PGPikeExchange;

/*	Each remote IKE daemon we communicate with is represented
	by one PGPikePartner. */
typedef struct PGPikePartner
{
	struct PGPikePartner *		nextPartner;
	struct PGPikeContextPriv *	ike;
	
	PGPUInt32					magic;
	PGPUInt32					ipAddress;
	PGPUInt32					localIPAddress;
	PGPBoolean					authenticated;
	
	PGPBoolean					ready;		/* has this IKE SA been keyed */
	PGPBoolean					initiator;
	PGPikeCookie				initCookie;
	PGPikeCookie				respCookie;
	PGPUInt32					kbLifeTime;	/* max KB to be sent on this SA */
	PGPUInt32					secLifeTime;/* max seconds this SA will live */
											/* 0 means no limit for both */
	PGPTime						birthTime;
	PGPUInt32					remotePGPVersion;
	PGPUInt32					rttMillisec;/* measured round trip time */
	PGPTime						termSchedule;

	PGPByte *					sharedKey;
	PGPSize						sharedKeySize;
	PGPikeMTCert				pgpAuthKey;
	PGPikeMTCert				x509AuthKey;
	PGPKeySetRef				remoteKeySet;
	PGPKeyRef					remoteKey;
	PGPSigRef					remoteCert;
	
	/* Key Material */
	PGPHashAlgorithm			sdkHashAlg;
	PGPSize						hashSize;
	PGPCipherAlgorithm			sdkCipherAlg;
	PGPSize						cipherSize;
	PGPBoolean					singleDESMode;
	PGPByte						skeyid[kPGPike_MaxHashSize];
	PGPByte						skeyid_d[kPGPike_MaxHashSize];
	PGPByte						skeyid_a[kPGPike_MaxHashSize];
	PGPByte						skeyid_e[kPGPike_MaxHashSize];
	PGPByte						cipherKey[kPGPike_MaxKeySize];
	PGPCBCContextRef			cbc;
	PGPByte						lastP1CBC[kPGPike_MaxCipherBlockSize];
	PGPSize						cipherBlockSize;
	
	PGPipsecDOIOptions			ipsecOpts;
		
	PGPikeExchange *			exchanges;
} PGPikePartner;

/* the non-opaque PGPikeContext */
typedef struct PGPikeContextPriv
{
	PGPContextRef				pgpContext;
	PGPMemoryMgrRef				memMgr;
	
	PGPBoolean					cookieDough;
	PGPByte						cookieSecret[kPGPike_CookieSize];
	PGPikeMessageProcPtr		msgProc;
	void *						userData;
	PGPikeDestination *			pending;
	PGPUInt32					numExchanges;
	
	PGPUInt32					kbLifeTimeIKE;
	PGPUInt32					secLifeTimeIKE;
	PGPUInt32					kbLifeTimeIPSEC;
	PGPUInt32					secLifeTimeIPSEC;
	
	PGPUInt32					numIKEProps;
	PGPikeTransform	*			defaultIKEProps;
	PGPUInt32					numIPSECProps;
	PGPipsecTransform	*		defaultIPSECProps;
	PGPikeAllowedAlgorithms		allowedAlgorithms;
		
	PGPikePartner *				partner;
	PGPikeSA *					sa;
} PGPikeContextPriv;


PGPError pgpIKEIdle(
	PGPikeContextPriv *		ike );
PGPError pgpIKEHandleSARequest(
	PGPikeContextPriv *		ike,
	PGPikeMTSASetup *		saReq );
PGPError pgpIKESARequestFailed(
	PGPikeContextPriv *		ike,
	PGPikeMTSAFailed *		saf );
PGPError pgpIKEAddPending(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPUInt32				ipAddrStart,
	PGPUInt32				ipMaskEnd,
	PGPBoolean				destIsRange );
PGPBoolean pgpIKEFindPending(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPUInt32				ipAddrStart,
	PGPUInt32				ipMaskEnd,
	PGPBoolean				destIsRange );
PGPError pgpIKERemovePending(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPUInt32				ipAddrStart,
	PGPUInt32				ipMaskEnd,
	PGPBoolean				destIsRange );
PGPError pgpIKEStartIdentityExchange(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPikeMTSASetup *		saSetup,
	PGPBoolean				destIsRange,
	PGPUInt32				ipAddrStart,
	PGPUInt32				ipMaskEnd );
PGPError pgpIKEStartQMExchange(
	PGPikePartner *		partner,
	PGPikeExchange **	exchangeP );
PGPError pgpIKEHandleSAEvent(
	PGPikeContextPriv *		ike,
	PGPBoolean				death,
	PGPikeSA *				sa );
PGPError pgpIKEFindSAPartner(
	PGPikeContextPriv *		ike,
	PGPikeSA *				sa,
	PGPBoolean				mustBeReady,
	PGPikePartner **		partnerP );
PGPError pgpIKEFreeSA(
	PGPikeContextPriv *		ike,
	PGPikeSA *				sa );
PGPError pgpIKEKillSA(
	PGPikePartner **		pPartner,
	PGPikeSA *				sa );
PGPError pgpIKEHandlePacket(
	PGPikeContextPriv *		ike,
	PGPikeMTPacket *		packet );
PGPError pgpIKEProcessPayloads(
	PGPikeExchange *		exchange,
	PGPikePayload			firstPayload,
	PGPByte *				mp );
PGPError pgpIKEProcessCertRequest(
	PGPikeExchange **		exchangeP,
	PGPByte *				pp,
	PGPSize					payLen );
PGPError pgpIKEProcessP2ID(
	PGPikeExchange *		exchange,
	PGPByte *				inIDBody,
	PGPSize					inIDBodySize,
	PGPBoolean *			outIsRange,
	PGPUInt32 *				ipStart,
	PGPUInt32 *				ipEnd );
PGPError pgpIKEProcessSig(
	PGPikeExchange **		exchangeP,
	PGPByte *				pp,
	PGPSize					payLen );
PGPError pgpIKEProcessCert(
	PGPikeExchange **		exchangeP,
	PGPByte *				pp,
	PGPSize					payLen );
PGPError pgpIKEProcessKE(
	PGPikeExchange **		exchangeP,
	PGPByte *				pp,
	PGPSize					payLen );
PGPError pgpIKEProcessProposal(
	PGPikeExchange **		exchangeP,
	PGPByte *				mp,
	PGPByte *				ep,
	PGPikeAlert	*			alert,
	PGPikeProposal **		proposal );
PGPError pgpIKESelectProposal(
	PGPikeExchange *		exchange,
	PGPikeProposal *		proposal,
	PGPBoolean *			isValid );
void pgpIKEGetAttribute(
	PGPByte **				mpp,
	PGPikeAttributeType *	attribute,
	PGPUInt32 *				value );
PGPError pgpIKEAbortExchange(
	PGPikeExchange **		exchangeP,
	PGPikeAlert				alert );
PGPError pgpIKEInitialContact(
	PGPikeExchange *		exchange );
PGPError pgpIKEResponderLifetime(
	PGPikeExchange *		exchange,
	PGPByte *				pp,
	PGPByte *				ep );
PGPError pgpIKEProcessInformational(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPikeExchange **		exchangeP,
	PGPikeNDPayload *		nd );
PGPError pgpIKEHeaderSniffer(
	PGPikeContextPriv *		ike,
	PGPikeMTPacket *		packet,
	PGPBoolean *			isValid );
PGPError pgpIKEPayloadLengthCheck(
	PGPikePartner *			partner,
	PGPikePayload			payload,
	PGPByte *				mp,
	PGPByte *				ep,
	PGPBoolean *			isValid );
PGPError pgpIKEDoPacket(
	PGPikeExchange **		exchangeP,
	PGPikePayload			firstPayload,
	... );
PGPError pgpIKEAddIDPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp );
PGPError pgpIKEAddCertPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp );
PGPError pgpIKEAddCertRequestPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp );
PGPError pgpIKEAddSigPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp );
PGPError pgpIKEAddNoncePayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp );
PGPError pgpIKEAddSAPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp );
PGPError pgpIKEAddProposalPayload(
	PGPikeExchange *		exchange,
	PGPikeProposal *		proposal,
	PGPByte **				mpp );
PGPError pgpIKEAddTransformPayload(
	PGPikeProposal *		proposal,
	PGPikeGenericTransform *transform,
	PGPByte **				mpp );
PGPError pgpIKEAddKEPayload(
	PGPikeExchange *		exchange,
	PGPByte **				mpp );
void pgpIKEAddAttribute(
	PGPikeAttributeType		attribute,
	PGPUInt32				value,
	PGPByte **				mpp );
PGPError pgpIKELocalAlert(
	PGPikeContextPriv *		ike,
	PGPUInt32				ipAddress,
	PGPikeAlert				alert,
	PGPikeInternalAlert		value,
	PGPBoolean				remote );
PGPError pgpIKESendPacket(
	PGPikeExchange *		exchange,
	PGPByte *				packet,
	PGPSize					packetSize  );
PGPError pgpIKESendPacket1(
	PGPikePartner *			partner,
	PGPByte *				packet,
	PGPSize					packetSize );
PGPError pgpIKEResendLastPacket(
	PGPikeExchange *		exchange );
PGPError pgpIKEBakeCookie(
	PGPikeContextPriv *		ike,
	PGPUInt32				destAddress,
	PGPUInt32				srcAddress,
	PGPByte *				outCookie );
PGPError pgpIKEGetCert(
	PGPikePartner *			partner,
	PGPikeMessageType		msg );
PGPError pgpIKESAEstablished(
	PGPikeContextPriv *		ike,
	PGPikeSA *				sa );
PGPError pgpIKECreatePartner(
	PGPikeContextPriv *		ike,
	PGPikeMTSASetup *		setup,
	PGPBoolean				initiator,
	PGPikePartner **		partnerP );
PGPError pgpIKECreateExchange(
	PGPikePartner *			partner,
	PGPikeExchangeT			exchangeT,
	PGPikeMessageID *		messageID,
	PGPikeState				state,
	PGPBoolean				initiator,
	PGPikeExchange **		exchangeP );
PGPError pgpIKECreateProposals(
	PGPikeExchange *		exchange );
PGPError pgpIKEFreePartner(
	PGPikePartner *			partner );
PGPError pgpIKEFreeExchange(
	PGPikeExchange *		exchange );
PGPError pgpIKELoadGroup(
	PGPikeExchange *		exchange );
PGPError pgpIKEModP(
	PGPikeExchange *		exchange );
PGPError pgpIKEGoSecure(
	PGPikeExchange *		exchange );
PGPError pgpIKECompletePhase2(
	PGPikeExchange *		exchange,
	PGPikeSA **				sa );
PGPError pgpIKEGetAuthHash(
	PGPikeExchange *		exchange,
	PGPBoolean				initiator,
	PGPByte *				outHash );
PGPError pgpIKEGetP2Keymat(
	PGPikeExchange *		exchange,
	PGPipsecDOIParams *		ipsec );
PGPBoolean pgpIKEIsTransformValid(
	PGPikePartner *					partner,
	PGPipsecProtocol				protocol,
	const PGPikeGenericTransform *	transform );
PGPBoolean pgpIKEIsTransformEqual(
	PGPipsecProtocol				protocol,
	const PGPikeGenericTransform *	transform1,
	const PGPikeGenericTransform *	transform2 );
PGPError pgpIKESetPref(
	PGPikeContextPriv *		ike,
	PGPikeMTPref *			pref );
void pgpIKEDebugLog(
	PGPikeContextPriv *		ike,
	char *					fmt,
	... );
void pgpIKEDebugData(
	PGPikeContextPriv *		ike,
	char *					name,
	PGPByte *				data,
	PGPSize					length );
char * pgpIKEPayloadTypeString(
	PGPikePayload			payload );
char * pgpIKEExchangeTypeString(
	PGPikeExchangeT			exchange );
char * pgpIKEAuthMethodString(
	PGPikeAuthMethod		auth );
char * pgpIKECipherString(
	PGPikeCipher			cipher );
char * pgpIKEIDTypeString(
	PGPipsecIdentity		id );
char * pgpIKEHashTypeString(
	PGPikeHash				hash );
char * pgpIKEGroupTypeString(
	PGPikeGroupID			group );
char * pgpIKEipsecAuthTypeString(
	PGPipsecAuthAttribute	auth );
char * pgpIKEipsecCipherTypeString(
	PGPipsecESPTransformID	cipher );

#endif /* ] Included_pgpIKEPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/