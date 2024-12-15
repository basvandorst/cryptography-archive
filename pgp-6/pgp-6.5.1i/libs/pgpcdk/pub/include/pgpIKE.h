/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	$Id: pgpIKE.h,v 1.47.6.1 1999/06/16 03:31:34 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_PGPike_h	/* [ */
#define Included_PGPike_h

#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#ifdef _MSC_VER
#pragma warning (disable:4200)
#endif

typedef struct PGPikeContext *			PGPikeContextRef;

#define	kInvalidPGPikeContextRef		((PGPikeContextRef) NULL)
#define PGPikeContextRefIsValid( ref )	( (ref) != kInvalidPGPikeContextRef )

#define kPGPike_CommonPort				500
#define kPGPike_AuthMaximumKeySize		32		/* 256 bits */
#define kPGPike_ESPMaximumKeySize		32		/* 256 bits */
#define kPGPike_MaxExplicitIVSize		8
#define kPGPike_MaxTransforms			3
#define kPGPike_DefaultKBLife			1048576	/* 1GB */
#define kPGPike_DefaultSecLife			86400	/* 1 Day */
#define kPGPike_UserDataSize			32

typedef PGPByte	PGPipsecSPI[4];

enum PGPikeDOI_
{
	kPGPike_DOI_IKE				= 0,
	kPGPike_DOI_IPSEC			= 1,

	PGP_ENUM_FORCE( PGPikeDOI_ )
};
PGPENUM_TYPEDEF( PGPikeDOI_, PGPikeDOI );

enum PGPikeAlert_
{
	kPGPike_AL_None						= 0,
	
	/* Error Types */
	kPGPike_AL_InvalidPayload			= 1,
	kPGPike_AL_DOIUnsupported			= 2,
	kPGPike_AL_SituationUnsupported		= 3,
	kPGPike_AL_InvalidCookie			= 4,
	kPGPike_AL_InvalidMajorVersion		= 5,
	kPGPike_AL_InvalidMinorVersion		= 6,
	kPGPike_AL_InvalidExchange			= 7,
	kPGPike_AL_InvalidFlags				= 8,
	kPGPike_AL_InvalidMessageID			= 9,
	kPGPike_AL_InvalidProtocolID		= 10,
	kPGPike_AL_InvalidSPI				= 11,
	kPGPike_AL_InvalidTransform			= 12,
	kPGPike_AL_InvalidAttribute			= 13,
	kPGPike_AL_NoProposalChoice			= 14,
	kPGPike_AL_BadProposal				= 15,
	kPGPike_AL_PayloadMalformed			= 16,
	kPGPike_AL_InvalidKey				= 17,
	kPGPike_AL_InvalidID				= 18,
	kPGPike_AL_InvalidCertEncoding		= 19,
	kPGPike_AL_InvalidCert				= 20,
	kPGPike_AL_UnsupportedCert			= 21,
	kPGPike_AL_InvalidCertAuthority		= 22,
	kPGPike_AL_InvalidHash				= 23,
	kPGPike_AL_AuthenticationFailed		= 24,
	kPGPike_AL_InvalidSignature			= 25,
	kPGPike_AL_AddressNotification		= 26,
	kPGPike_AL_NotifySALifetime			= 27,
	kPGPike_AL_CertUnavailable			= 28,
	kPGPike_AL_UnsupportedExchange		= 29,
	kPGPike_AL_UnequalPayloadLengths	= 30,
	kPGPike_AL_LastErrorType			= 30,
	
	/* Status Types */
	kPGPike_AL_Connected				= 16384,
	kPGPike_AL_ResponderLifetime		= 24576,	/* IPSEC DOI */
	kPGPike_AL_ReplayStatus				= 24577,	/* IPSEC DOI */
	kPGPike_AL_InitialContact			= 24578,	/* IPSEC DOI */

	PGP_ENUM_FORCE( PGPikeAlert_ )
};
PGPENUM_TYPEDEF( PGPikeAlert_, PGPikeAlert );

enum PGPikeInternalAlert_
{
	kPGPike_IA_None						= 0,
	
	kPGPike_IA_ResponseTimeout			= 1,
	kPGPike_IA_NoProposals				= 2,
	kPGPike_IA_NewPhase1SA				= 3,
	kPGPike_IA_NewPhase2SA				= 4,
	kPGPike_IA_DeadPhase1SA				= 5,
	kPGPike_IA_DeadPhase2SA				= 6,
	kPGPike_IA_TooManyExchanges			= 7,

	PGP_ENUM_FORCE( PGPikeInternalAlert_ )
};
PGPENUM_TYPEDEF( PGPikeInternalAlert_, PGPikeInternalAlert );

enum PGPikePref_
{
	kPGPike_PF_None						= 0,
	
	kPGPike_PF_Expiration				= 1,
	kPGPike_PF_AllowedAlgorithms		= 2,
	kPGPike_PF_IKEProposals				= 3,
	kPGPike_PF_IPSECProposals			= 4,

	PGP_ENUM_FORCE( PGPikePref_ )
};
PGPENUM_TYPEDEF( PGPikePref_, PGPikePref );

enum PGPipsecEncapsulation_
{
	kPGPike_PM_None				= 0,
	
	kPGPike_PM_Tunnel			= 1,
	kPGPike_PM_Transport		= 2,

	PGP_ENUM_FORCE( PGPipsecEncapsulation_ )
};
PGPENUM_TYPEDEF( PGPipsecEncapsulation_, PGPipsecEncapsulation );

enum PGPikeLifeType_
{
	kPGPike_LT_None				= 0,
	
	kPGPike_LT_Seconds			= 1,
	kPGPike_LT_Kilobytes		= 2,

	PGP_ENUM_FORCE( PGPikeLifeType_ )
};
PGPENUM_TYPEDEF( PGPikeLifeType_, PGPikeLifeType );

enum PGPipsecIdentity_
{
	kPGPike_ID_None				= 0,
	
	kPGPike_ID_IPV4_Addr,
	kPGPike_ID_FQDN,
	kPGPike_ID_UserFDQN,
	kPGPike_ID_IPV4_Addr_Subnet,
	kPGPike_ID_IPV6_Addr,
	kPGPike_ID_IPV6_Addr_Subnet,
	kPGPike_ID_IPV4_Addr_Range,
	kPGPike_ID_IPV6_Addr_Range,
	kPGPike_ID_DER_ASN1_DN,
	kPGPike_ID_DER_ASN1_GN,
	kPGPike_ID_Key_ID,				/* used for PGP fingerprint */

	PGP_ENUM_FORCE( PGPipsecIdentity_ )
};
PGPENUM_TYPEDEF( PGPipsecIdentity_, PGPipsecIdentity );

/* If it doesn't say supported, it isn't. */
enum PGPipsecAHTransformID_
{
	kPGPike_AH_None				= 0,
	
	kPGPike_AH_MD5				= 2,	/* supported */
	kPGPike_AH_SHA				= 3,	/* supported */
	kPGPike_AH_DES				= 4,

	PGP_ENUM_FORCE( PGPipsecAHTransformID_ )
};
PGPENUM_TYPEDEF( PGPipsecAHTransformID_, PGPipsecAHTransformID );

enum PGPipsecAuthAttribute_
{
	kPGPike_AA_None				= 0,
	
	kPGPike_AA_HMAC_MD5			= 1,	/* supported */
	kPGPike_AA_HMAC_SHA			= 2,	/* supported */
	kPGPike_AA_DES_MAC			= 3,
	kPGPike_AA_KPDK				= 4,

	PGP_ENUM_FORCE( PGPipsecAuthAttribute_ )
};
PGPENUM_TYPEDEF( PGPipsecAuthAttribute_, PGPipsecAuthAttribute );

enum PGPipsecESPTransformID_
{
	kPGPike_ET_DES_IV64			= 1,	/* supported */
	kPGPike_ET_DES				= 2,	/* supported */
	kPGPike_ET_3DES				= 3,	/* supported */
	kPGPike_ET_RC5				= 4,
	kPGPike_ET_IDEA				= 5,
	kPGPike_ET_CAST				= 6,	/* supported */
	kPGPike_ET_Blowfish			= 7,
	kPGPike_ET_3IDEA			= 8,
	kPGPike_ET_DES_IV32			= 9,
	kPGPike_ET_RC4				= 10,
	kPGPike_ET_NULL				= 11,	/* supported */
	
	PGP_ENUM_FORCE( PGPipsecESPTransformID_ )
};
PGPENUM_TYPEDEF( PGPipsecESPTransformID_, PGPipsecESPTransformID );

/* IPCOMP is not supported by this version of PGPike */
enum PGPipsecIPCOMPTransformID_
{
	kPGPike_IC_None			= 0,
	
	kPGPike_IC_OUI			= 1,
	kPGPike_IC_Deflate		= 2,		/* supported */
	kPGPike_IC_LZS			= 3,		/* supported */
	kPGPike_IC_V42bis		= 4,

	PGP_ENUM_FORCE( PGPipsecIPCOMPTransformID_ )
};
PGPENUM_TYPEDEF( PGPipsecIPCOMPTransformID_, PGPipsecIPCOMPTransformID );

enum PGPipsecProtocol_
{
	kPGPike_PR_None			= 0,
	kPGPike_PR_IKE			= 1,
	kPGPike_PR_AH			= 2,
	kPGPike_PR_ESP			= 3,
	kPGPike_PR_IPCOMP		= 4,

	PGP_ENUM_FORCE( PGPipsecProtocol_ )
};
PGPENUM_TYPEDEF( PGPipsecProtocol_, PGPipsecProtocol );

enum PGPikeGroupID_
{
	kPGPike_GR_None					= 0,	/* supported */
	
	kPGPike_GR_MODPOne				= 1,	/* supported */
	kPGPike_GR_MODPTwo				= 2,	/* supported */
	kPGPike_GR_MODPFive				= 5,	/* supported */
	
	PGP_ENUM_FORCE( PGPikeGroupID_ )
};
PGPENUM_TYPEDEF( PGPikeGroupID_, PGPikeGroupID );

enum PGPikeCipher_
{
	kPGPike_SC_None				= 0,
	
	kPGPike_SC_DES_CBC			= 1,	/* supported */
	kPGPike_SC_IDEA_CBC			= 2,
	kPGPike_SC_Blowfish_CBC		= 3,
	kPGPike_SC_RC5_R16_B64_CBC	= 4,
	kPGPike_SC_3DES_CBC			= 5,	/* supported */
	kPGPike_SC_CAST_CBC			= 6,	/* supported */
	
	PGP_ENUM_FORCE( PGPikeCipher_ )
};
PGPENUM_TYPEDEF( PGPikeCipher_, PGPikeCipher );

enum PGPikeHash_
{
	kPGPike_HA_None				= 0,
	
	kPGPike_HA_MD5				= 1,	/* supported */
	kPGPike_HA_SHA1				= 2,	/* supported */
	kPGPike_HA_Tiger			= 3,
	
	PGP_ENUM_FORCE( PGPikeHash_ )
};
PGPENUM_TYPEDEF( PGPikeHash_, PGPikeHash );

enum PGPikeAuthMethod_
{
	kPGPike_AM_None				= 0,
	
	kPGPike_AM_PreSharedKey		= 1,	/* supported */
	kPGPike_AM_DSS_Sig			= 2,	/* supported */
	kPGPike_AM_RSA_Sig			= 3,	/* supported */
	kPGPike_AM_RSA_Encrypt		= 4,
	kPGPike_AM_RSA_Encrypt_R	= 5,
	
	PGP_ENUM_FORCE( PGPikeAuthMethod_ )
};
PGPENUM_TYPEDEF( PGPikeAuthMethod_, PGPikeAuthMethod );

typedef struct PGPipsecESPTransform
{
	PGPipsecESPTransformID		cipher;
	PGPipsecAuthAttribute		authAttr;
	PGPipsecEncapsulation		mode;
} PGPipsecESPTransform;

typedef struct PGPipsecAHTransform
{
	PGPipsecAHTransformID		authAlg;
	PGPipsecAuthAttribute		authAttr;
	PGPipsecEncapsulation		mode;
} PGPipsecAHTransform;

typedef struct PGPipsecIPCOMPTransform
{
	PGPipsecIPCOMPTransformID	compAlg;
} PGPipsecIPCOMPTransform;

typedef struct PGPikeTransform
{
	PGPikeAuthMethod		authMethod;
	PGPikeHash				hash;
	PGPikeCipher			cipher;
	PGPBoolean				modpGroup;
	PGPikeGroupID			groupID;
} PGPikeTransform;

typedef struct PGPipsecTransform
{
	PGPBoolean				useESP;
	PGPipsecESPTransform	esp;
	
	PGPBoolean				useAH;
	PGPipsecAHTransform		ah;
	
	PGPBoolean				useIPCOMP;
	PGPipsecIPCOMPTransform	ipcomp;
	
	PGPikeGroupID			groupID;
} PGPipsecTransform;

typedef struct PGPipsecDOIParams
{
	PGPipsecSPI				inSPI;
	PGPipsecSPI				outSPI;
	PGPipsecProtocol		protocol;

	union
	{
		struct
		{
			PGPipsecAHTransform		t;
			PGPByte					inAuthKey[kPGPike_AuthMaximumKeySize];
			PGPByte					outAuthKey[kPGPike_AuthMaximumKeySize];
		} ah;
		
		struct
		{
			PGPipsecESPTransform	t;
			PGPByte					inESPKey[kPGPike_ESPMaximumKeySize];
			PGPByte					outESPKey[kPGPike_ESPMaximumKeySize];
			PGPByte					inAuthKey[kPGPike_AuthMaximumKeySize];
			PGPByte					outAuthKey[kPGPike_AuthMaximumKeySize];
			PGPByte					explicitIV[kPGPike_MaxExplicitIVSize];
		} esp;
		
		struct
		{
			PGPipsecIPCOMPTransform	t;
		} ipcomp;
	} u;	
} PGPipsecDOIParams;

typedef union PGPikeDOIParams
{
	union
	{
		PGPipsecDOIParams		ipsec;
	} u;
} PGPikeDOIParams;

typedef struct PGPikeSA
{
	struct PGPikeSA *		nextSA;		/* INTERNAL USE ONLY */
	struct PGPikeSA *		prevSA;		/* INTERNAL USE ONLY */
	PGPUInt32				ipAddress;
	PGPBoolean				destIsRange;/* dest is IP range */
	PGPUInt32				ipAddrStart;/* IP address */
	PGPUInt32				ipMaskEnd;	/* mask or end range IP address */
	PGPBoolean				activeIn;	/* use for inbound data */
	PGPBoolean				activeOut;	/* use for outbound data */
	PGPUInt32				kbLifeTime;	/* max KB to be sent on this SA */
	PGPUInt32				secLifeTime;/* max seconds this SA will live */
										/* 0 means no limit for either */
	PGPTime					birthTime;
	
	PGPikeDOI				doi;

	PGPUInt16				numTransforms;
	PGPikeDOIParams			transform[kPGPike_MaxTransforms];
	
	PGPTime					termSchedule;	/* SA will be terminated, private */
	PGPByte					userData[kPGPike_UserDataSize];/* for your use */
} PGPikeSA;

typedef struct PGPipsecDOIOptions
{
	PGPipsecEncapsulation	packetMode;
	
	PGPipsecIdentity		idType;		/* only needed for shared key mode */
	PGPByte *				idData;		/* data will be copied */
	PGPSize					idDataSize;	/* this is the Phase 1 ID */
	
	/*	The following is used only for packetMode = kPGPike_PM_Tunnel.
		You may ignore these in all other cases.  For tunnel mode,
		you must pass the destination IP, subnet, or IP range with
		which you wish to communicate.  The SA will be negotiated with
		the gateway using this, and to communicate with anything else
		behind the gateway will require establishment of another SA.
		
	IP address:destIsRange = FALSE, ipAddrStart = IP, ipMaskEnd = FFFFFFFF
	IP subnet: destIsRange = FALSE, ipAddrStart = IP, ipMaskEnd = mask
	IP range:  destIsRange = TRUE,  ipAddrStart = start, ipMaskEnd = end */
	
	PGPBoolean				destIsRange;
	PGPUInt32				ipAddrStart;
	PGPUInt32				ipMaskEnd;
} PGPipsecDOIOptions;

/*	When sent a kPGPike_MT_SARequest, pgpIKE will expect that
	all fields below will be filled in.  The approved member is
	irrelevant in that case.
	
	When you are called with kPGPike_MT_PolicyCheck, pgpIKE will
	set everything to 0 except the IP address.  You are expected
	to fill in the other fields with local policy.  The PolicyCheck
	is used for remote-initiated SA negotiation.  Since you cannot
	predict which remote network might be trying to communicate
	with us, you should leave the destIsRange/ipAddrStart/ipMaskEnd
	fields set to 0 for this message.
*/
typedef struct PGPikeMTSASetup
{
	PGPBoolean				approved;		/* >  */
	
	PGPUInt32				ipAddress;		/* destination */
	PGPUInt32				localIPAddress;	/* source */
	PGPikeDOI				doi;
	PGPByte *				sharedKey;	/* null if none, data will be copied */
	PGPSize					sharedKeySize; /* w/o NULL-terminate */
	
	union
	{
		PGPipsecDOIOptions	ipsec;
	} u;
} PGPikeMTSASetup;

/*	
	When you are called with a kPGPike_MT_ClientIDCheck, all fields
	will be set appropriately.  If the destIsRange/ipAddrStart/ipMaskEnd
	triad is permitted to be represented by the IP address, set
	the approved field to true.  This message is only sent in the case
	where the remote side is the initiator and you have already been
	called with a kPGPike_MT_PolicyCheck message.
*/
typedef struct PGPikeMTClientIDCheck
{
	PGPBoolean				approved;		/* >  */
	
	PGPUInt32				ipAddress;
	
	PGPBoolean				destIsRange;
	PGPUInt32				ipAddrStart;
	PGPUInt32				ipMaskEnd;
} PGPikeMTClientIDCheck;

typedef struct PGPikeMTSAFailed
{
	PGPUInt32				ipAddress;		/* destination */
	PGPikeDOI				doi;
	
	union
	{
		PGPipsecDOIOptions	ipsec;
	} u;
} PGPikeMTSAFailed;

typedef struct PGPikeMTCert
{
	PGPUInt32				ipAddress;		/*  < */
	PGPKeySetRef			baseKeySet;		/* >  */
	PGPKeyRef				authKey;		/* >  ignored for X.509 */
	PGPSigRef				authCert;		/* >  used only for X.509 */
	PGPKeySetRef			authCertChain;	/* >  used only for X.509 */
	
	PGPBoolean				isPassKey;		/* >  */
	void *					pass;			/* >  null-term if passphrase, copied */
	PGPSize					passLength;		/* >  */
} PGPikeMTCert;

typedef struct PGPikeMTRemoteCert
{
	PGPBoolean				approved;		/* >  */
	PGPUInt32				ipAddress;		/*  < */
	PGPKeyRef				remoteKey;		/*  < */
	
	PGPSigRef				remoteCert;		/*  < if non-NULL, X.509 used */
	PGPKeySetRef			remoteCertChain;/*  < may be NULL */
} PGPikeMTRemoteCert;

typedef struct PGPikeMTPacket
{
	PGPUInt32				ipAddress;	/* source or destination */
	PGPSize					packetSize;
	PGPByte *				packet;		/* msg *sender* must free this */
} PGPikeMTPacket;

typedef struct PGPikeMTAlert
{
	PGPikeAlert				alert;
	PGPUInt32				ipAddress;
	PGPikeInternalAlert		value;	/* used if alert is kPGPike_AL_None */
	PGPBoolean				remoteGenerated;
} PGPikeMTAlert;

typedef struct PGPikeAllowedAlgorithms
{
	PGPBoolean				cast5;
	PGPBoolean				tripleDES;
	PGPBoolean				singleDES;
	PGPBoolean				espNULL;
	
	PGPBoolean				sha1;
	PGPBoolean				md5;
	PGPBoolean				noAuth;
		
	PGPBoolean				lzs;
	PGPBoolean				deflate;
	
	PGPBoolean				modpOne768;
	PGPBoolean				modpTwo1024;
	PGPBoolean				modpFive1536;
} PGPikeAllowedAlgorithms;

typedef struct PGPikeMTPref
{
	PGPikePref					pref;
	
	union
	{
		struct
		{
			PGPUInt32			kbLifeTimeIKE;
			PGPUInt32			secLifeTimeIKE;
			PGPUInt32			kbLifeTimeIPSEC;
			PGPUInt32			secLifeTimeIPSEC;
		} expiration;			/* kPGPike_PF_Expiration */
		
		struct
		{
			PGPUInt32			numTransforms;
			PGPikeTransform *	t;
		} ikeProposals;			/* kPGPike_PF_IKEProposals */

		
		struct
		{
			PGPUInt32			numTransforms;
			PGPipsecTransform *	t;
		} ipsecProposals;		/* kPGPike_PF_IPSECProposals */
		
		PGPikeAllowedAlgorithms	allowedAlgorithms;
			/*kPGPike_PF_AllowedAlgorithms*/
	} u;
} PGPikeMTPref;

enum PGPikeMessageType_
{
	/*
		Message types followed by ">" may be sent to PGPike
		Message typed followed by "<" may be sent by PGPike
	*/
	kPGPike_MT_Idle			= 0,	/* >  call this often		*/
	kPGPike_MT_SARequest,			/* >  PGPikeMTSASetup		*/
	kPGPike_MT_SARequestFailed,		/*  < PGPikeMTSAFailed		*/
	kPGPike_MT_SAEstablished,		/*  < PGPikeSA				*/
	kPGPike_MT_SARekey,				/* >  PGPikeSA				*/
	kPGPike_MT_SADied,				/* >< PGPikeSA				*/
	kPGPike_MT_SAUpdate,			/*  < PGPikeSA				*/
	kPGPike_MT_SAKillAll,			/* >  none					*/
	
	kPGPike_MT_PolicyCheck,			/*  < PGPikeMTSASetup		*/
	kPGPike_MT_ClientIDCheck,		/*  < PGPikeMTClientIDCheck	*/
	kPGPike_MT_LocalPGPCert,		/*  < PGPikeMTCert			*/
	kPGPike_MT_LocalX509Cert,		/*  < PGPikeMTCert			*/
	kPGPike_MT_RemoteCert,			/*  < PGPikeMTRemoteCert	*/
	
	kPGPike_MT_Packet,				/* >< PGPikeMTPacket		*/
	kPGPike_MT_Alert,				/*  < PGPikeMTAlert			*/
	
	kPGPike_MT_Pref,				/* >  PGPikeMTPref			*/
	
	kPGPike_MT_DebugLog,			/*  < char *				*/

	PGP_ENUM_FORCE( PGPikeMessageType_ )
};
PGPENUM_TYPEDEF( PGPikeMessageType_, PGPikeMessageType );

typedef PGPError (* PGPikeMessageProcPtr)(
								PGPikeContextRef	ike,
								void *				inUserData,
								PGPikeMessageType	msg,
								void *				data );

PGPError 	PGPNewIKEContext( PGPContextRef	context,
					PGPikeMessageProcPtr	ikeMessageProc,
					void *					inUserData,
					PGPikeContextRef *		outRef );

PGPError 	PGPFreeIKEContext( PGPikeContextRef ref );

/*	Any error from PGPikeProcessMessage is fatal.  Non-fatal
	errors are sent through the kPGPike_MT_Alert message.  */

PGPError	PGPikeProcessMessage( PGPikeContextRef ref,
							PGPikeMessageType	msg,
							void *				data );


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_PGPike_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/