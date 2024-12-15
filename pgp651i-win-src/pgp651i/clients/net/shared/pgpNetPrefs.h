/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: pgpNetPrefs.h,v 1.41.4.1 1999/06/08 04:19:32 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_pgpNetPrefs_h	/* [ */
#define Included_pgpNetPrefs_h

#include "pgpPubTypes.h"
#include "pflPrefs.h"
#include "pgpKeys.h"
#include "pgpNetPrefsHostEntry.h"

typedef struct PGPNetPrefIKEProposalEntry
{
	PGPikeAuthMethod			authMethod;
	PGPikeHash					hash;
	PGPikeCipher				cipher;
	PGPikeGroupID				groupID;
} PGPNetPrefIKEProposalEntry;

typedef struct PGPNetPrefIPSECProposalEntry
{
	PGPBoolean					useESP;
	PGPipsecESPTransformID		espCipher;
	PGPipsecAuthAttribute		espHash;
	PGPBoolean					useAH;
	PGPipsecAHTransformID		ahAuthAlg;
	PGPipsecAuthAttribute		ahHash;
	PGPBoolean					useIPCOMP;
	PGPipsecIPCOMPTransformID	ipcomp;
} PGPNetPrefIPSECProposalEntry;



typedef enum _PGPNetPrefsBool
{
	kPGPNetPrefBoolFirst						=	1000,
	kPGPNetPrefAllowUnconfiguredHosts = kPGPNetPrefBoolFirst,
	kPGPNetPrefRequireSecure					=	1001,
	kPGPNetPrefRequireValidKey					=	1002,	/* new for 6.5.1	*/
	kPGPNetPrefEnablePassphraseCache			=	1003,
	kPGPNetPrefEnablePGPnet						=	1004,
	kPGPNetPrefExpertMode						=	1005,
	kPGPNetPrefEnableIKEKByteExpiration			=	1006,
	kPGPNetPrefEnableIKETimeExpiration			=	1007,
	kPGPNetPrefEnableIPSECKByteExpiration		=	1008,
	kPGPNetPrefEnableIPSECTimeExpiration		=	1009,
	kPGPNetPrefFirstExecution					=	1010,
	kPGPNetPrefWarnReSharedSecret				=	1011,
	kPGPNetPrefAllowCipherNone					=	1040,
	kPGPNetPrefAllowCipherCAST					=	1041,
	kPGPNetPrefAllowCipher3DES					=	1042,
	kPGPNetPrefAllowCipherDES56					=	1043,
	kPGPNetPrefAllowHashNone					=	1050,
	kPGPNetPrefAllowHashSHA1					=	1051,
	kPGPNetPrefAllowHashMD5						=	1052,
	kPGPNetPrefAllowCompLZS						=	1061,
	kPGPNetPrefAllowCompDeflate					=	1062,
	kPGPNetPrefAllowDH768						=	1071,
	kPGPNetPrefAllowDH1024						=	1072,
	kPGPNetPrefAllowDH1536						=	1073,
	kPGPNetPrefBoolLast
} PGPNetPrefsBool;

typedef enum _PGPNetPrefsNumber
{
	kPGPNetPrefNumberFirst						=	1200,
	kPGPNetPrefVersion = kPGPNetPrefNumberFirst,
	kPGPNetPrefLocalIdType						=	1201,
	kPGPNetPrefPGPAuthKeyAlgorithm				=	1202,
	kPGPNetPrefX509AuthKeyAlgorithm				=	1203,
	kPGPNetPrefHostListCount					=	1204,
	kPGPNetPrefIKEKByteExpiration				=	1205,
	kPGPNetPrefIKETimeExpiration				=	1206,
	kPGPNetPrefIPSECKByteExpiration				=	1207,
	kPGPNetPrefIPSECTimeExpiration				=	1208,
	kPGPNetPrefIKEProposalListCount				=	1209,
	kPGPNetPrefIPSECProposalListCount			=	1210,
	kPGPnetPrefIPSECGroupID						=	1211,
	kPGPNetPrefNumberLast
} PGPNetPrefsNumber;

typedef enum _PGPNetPrefsString
{
	kPGPNetPrefStringFirst						=	1400,
	kPGPNetPrefStringLast
} PGPNetPrefsString;

typedef enum _PGPNetPrefsByte
{
	kPGPNetPrefByteFirst						=	1600,
	kPGPNetPrefPGPAuthKeyID = kPGPNetPrefByteFirst,
	kPGPNetPrefX509AuthKeyID					=	1601,
	kPGPNetPrefX509AuthCertIASN					=	1602,
	kPGPNetPrefHostList							=	1603,
	kPGPNetPrefAppWinStatusWindowData			=	1604,
	kPGPNetPrefAppWinLogWindowData				=	1605,
	kPGPNetPrefIKEProposalList					=	1606,
	kPGPNetPrefIPSECProposalList				=	1607,
	kPGPNetPrefMainWindowPos					=	1608,
	kPGPNetPrefByteLast
} PGPNetPrefsByte;


static const PGPNetPrefHostEntry sDefaultHosts[] =
{
	{ 
		kPGPnetInsecureHost, 
		0, 
		0, 
		-1, 
		"",
		kPGPike_ID_IPV4_Addr, 
		"",
		0,
		""
	},
};

static const PGPNetPrefIKEProposalEntry sDefaultIKEProposals[] =
{
	{
		kPGPike_AM_PreSharedKey,
		kPGPike_HA_SHA1,
		kPGPike_SC_CAST_CBC,
		kPGPike_GR_MODPFive
	},
	{
		kPGPike_AM_PreSharedKey,
		kPGPike_HA_MD5,
		kPGPike_SC_3DES_CBC,
		kPGPike_GR_MODPTwo
	},
	{
		kPGPike_AM_DSS_Sig,
		kPGPike_HA_SHA1,
		kPGPike_SC_CAST_CBC,
		kPGPike_GR_MODPFive
	},
	{
		kPGPike_AM_DSS_Sig,
		kPGPike_HA_SHA1,
		kPGPike_SC_3DES_CBC,
		kPGPike_GR_MODPTwo
	},
	{
		kPGPike_AM_RSA_Sig,
		kPGPike_HA_SHA1,
		kPGPike_SC_CAST_CBC,
		kPGPike_GR_MODPFive
	},
	{
		kPGPike_AM_RSA_Sig,
		kPGPike_HA_MD5,
		kPGPike_SC_3DES_CBC,
		kPGPike_GR_MODPTwo
	}
};

static const PGPNetPrefIPSECProposalEntry sDefaultIPSECProposals[] =
{
	{	/* ESP, CAST, SHA1 */
		TRUE, kPGPike_ET_CAST, kPGPike_AA_HMAC_SHA,
		FALSE, kPGPike_AH_None, kPGPike_AA_None,
		FALSE, kPGPike_IC_None
	},
	{	/* ESP, 3DES, MD5 */
		TRUE, kPGPike_ET_3DES, kPGPike_AA_HMAC_MD5,
		FALSE, kPGPike_AH_None, kPGPike_AA_None,
		FALSE, kPGPike_IC_None
	},
};

static const PGPPrefDefault netDefaults[] =
{
{kPGPNetPrefAllowUnconfiguredHosts,		kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefRequireSecure,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPNetPrefRequireValidKey,			kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefEnablePassphraseCache,		kPGPPrefType_Boolean, (void *) FALSE},
{kPGPNetPrefEnablePGPnet,				kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefExpertMode,					kPGPPrefType_Boolean, (void *) FALSE},
{kPGPNetPrefEnableIKEKByteExpiration,	kPGPPrefType_Boolean, (void *) FALSE},
{kPGPNetPrefEnableIKETimeExpiration,	kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefEnableIPSECKByteExpiration,	kPGPPrefType_Boolean, (void *) FALSE},
{kPGPNetPrefEnableIPSECTimeExpiration,	kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefFirstExecution,				kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefWarnReSharedSecret,			kPGPPrefType_Boolean, (void *) TRUE},

{kPGPNetPrefAllowCipherNone,			kPGPPrefType_Boolean, (void *) FALSE},
{kPGPNetPrefAllowCipherCAST,			kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefAllowCipher3DES,			kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefAllowCipherDES56,			kPGPPrefType_Boolean, (void *) FALSE},
{kPGPNetPrefAllowHashNone,				kPGPPrefType_Boolean, (void *) FALSE},
{kPGPNetPrefAllowHashSHA1,				kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefAllowHashMD5,				kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefAllowCompLZS,				kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefAllowCompDeflate,			kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefAllowDH768,					kPGPPrefType_Boolean, (void *) FALSE},
{kPGPNetPrefAllowDH1024,				kPGPPrefType_Boolean, (void *) TRUE},
{kPGPNetPrefAllowDH1536,				kPGPPrefType_Boolean, (void *) TRUE},

{kPGPNetPrefIKEKByteExpiration,			kPGPPrefType_Number,  (void *) 5120},
{kPGPNetPrefIKETimeExpiration,			kPGPPrefType_Number,  (void *) 86400},
{kPGPNetPrefIPSECKByteExpiration,		kPGPPrefType_Number,  (void *) 1048576},
{kPGPNetPrefIPSECTimeExpiration,		kPGPPrefType_Number,  (void *) 28800},
{kPGPnetPrefIPSECGroupID,				kPGPPrefType_Number,  (void *) 0},

{kPGPNetPrefPGPAuthKeyAlgorithm,	
		kPGPPrefType_Number, (void *) kPGPPublicKeyAlgorithm_Invalid},
{kPGPNetPrefX509AuthKeyAlgorithm,	
		kPGPPrefType_Number, (void *) kPGPPublicKeyAlgorithm_Invalid},

{kPGPNetPrefHostListCount,	
		kPGPPrefType_Number, (void *) 0},
{kPGPNetPrefIKEProposalListCount,	
		kPGPPrefType_Number, (void *) 0},
{kPGPNetPrefIPSECProposalListCount,	
		kPGPPrefType_Number, (void *) 0},

{kPGPNetPrefHostList, kPGPPrefType_Byte, 
		(void *) sDefaultHosts, sizeof(sDefaultHosts)},
{kPGPNetPrefIKEProposalList, kPGPPrefType_Byte, 
		(void *) sDefaultIKEProposals, sizeof(sDefaultIKEProposals)},
{kPGPNetPrefIPSECProposalList, kPGPPrefType_Byte, 
		(void *) sDefaultIPSECProposals, sizeof(sDefaultIPSECProposals)},
};

static const PGPSize netDefaultsSize = 
						sizeof(netDefaults) / sizeof(PGPPrefDefault);



PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/*	
	The following are used for getting/setting data structures
	stored in the preference file
*/
	PGPError
PGPGetNetHostPrefs(
		PGPPrefRef				prefRef,
		PGPNetPrefHostEntry		**netHostList,
		PGPUInt32				*netHostCount);

	PGPError
PGPSetNetHostPrefs(
		PGPPrefRef				prefRef,
		PGPNetPrefHostEntry		*netHostList,
		PGPUInt32				netHostCount);

	PGPError
PGPGetNetIKEProposalPrefs(		
		PGPPrefRef					prefRef,
		PGPNetPrefIKEProposalEntry	**proposalList,
		PGPUInt32					*proposalCount);

	PGPError
PGPSetNetIKEProposalPrefs(
		PGPPrefRef					prefRef,
		PGPNetPrefIKEProposalEntry	*proposalList,
		PGPUInt32					proposalCount);

	PGPError
PGPGetNetIPSECProposalPrefs(		
		PGPPrefRef						prefRef,
		PGPNetPrefIPSECProposalEntry	**proposalList,
		PGPUInt32						*proposalCount);

	PGPError
PGPSetNetIPSECProposalPrefs(
		PGPPrefRef						prefRef,
		PGPNetPrefIPSECProposalEntry	*proposalList,
		PGPUInt32						proposalCount);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS




#endif /* ] Included_pgpNetPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
