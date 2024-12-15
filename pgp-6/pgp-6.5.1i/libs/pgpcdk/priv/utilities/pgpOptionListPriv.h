/*____________________________________________________________________________
	pgpOptionListPriv.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	This file contains the private types and prototypes for functions which
	manipulate PGPOptionList data structures.

	$Id: pgpOptionListPriv.h,v 1.38 1999/05/07 23:47:45 hal Exp $
____________________________________________________________________________*/

#ifndef Included_pgpOptionListPriv_h	/* [ */
#define Included_pgpOptionListPriv_h

#include "pflTypes.h"
#include "pgpEncode.h"
#include "pgpGroups.h"
#include "pgpContext.h"
#include "pgpOptionList.h"
#include "pgpUserInterface.h"

#define pgpValidateOptionParam( expr )	\
	if ( ! (expr ) )	\
	{\
		pgpAssert( expr );\
		return( kPGPBadParamsOptionListRef );\
	}

#define pgpValidateOptionContext( context ) \
		pgpValidateOptionParam( pgpContextIsValid( context ) )
	

enum PGPOptionType_
{
	kPGPOptionType_InvalidOption		= 0,
	kPGPOptionType_FirstSDKOption		= kPGPOptionType_InvalidOption,
	
	kPGPOptionType_InputFileRef			= 1,
	kPGPOptionType_InputBuffer			= 2,
	/* kPGPOptionType_InputPipe			= 3, */
	
	kPGPOptionType_OutputFileRef		= 4,
	kPGPOptionType_OutputBuffer			= 5,
	kPGPOptionType_OutputAllocatedBuffer= 6,
	/* kPGPOptionType_OutputPipe			= 7, */
	kPGPOptionType_AppendOutput			= 8,
	kPGPOptionType_DiscardOutput		= 9,
	/* kPGPOptionType_AskUserForOutput		= 10, */
	
	kPGPOptionType_LocalEncoding		= 11,
	kPGPOptionType_RawPGPInput			= 12,

	kPGPOptionType_EncryptToKey			= 13,
	kPGPOptionType_EncryptToKeySet		= 14,
	kPGPOptionType_EncryptToUserID		= 15,
	kPGPOptionType_SignWithKey			= 16,
	kPGPOptionType_ConventionalEncrypt	= 17,
	
	kPGPOptionType_Passphrase			= 18,
	kPGPOptionType_Passkey				= 19,
	
	kPGPOptionType_DetachedSignature	= 20,
	kPGPOptionType_SessionKey			= 21,

	kPGPOptionType_CipherAlgorithm		= 22,
	kPGPOptionType_HashAlgorithm		= 23,
	kPGPOptionType_FailBelowValidity	= 24,
	kPGPOptionType_WarnBelowValidity	= 25,

	kPGPOptionType_X509Encoding			= 26,

	kPGPOptionType_PGPMIMEEncoding		= 27,
	/* kPGPOptionType_NoNewerThanVersion	= 28, */
	/* kPGPOptionType_OnePassEncoding		= 29, */
	kPGPOptionType_OutputLineEndType	= 30,

	kPGPOptionType_EventHandler			= 31,
	kPGPOptionType_SendNullEvents		= 32,
	kPGPOptionType_ImportKeysTo			= 33,
	kPGPOptionType_SendEventIfKeyFound	= 34,
	kPGPOptionType_AskUserForEntropy	= 35,

	kPGPOptionType_ArmorOutput			= 36,
	kPGPOptionType_DataIsASCII			= 37,
	kPGPOptionType_ClearSign			= 38,
	kPGPOptionType_ForYourEyesOnly		= 39,
	
	kPGPOptionType_DecodeOnlyOne		= 40,
	kPGPOptionType_RecursivelyDecode	= 41,
	kPGPOptionType_PassThroughIfUnrecognized= 42,

	kPGPOptionType_KeySetRef			= 43,
	kPGPOptionType_KeyGenUseExistingEntropy	= 44,

	kPGPOptionType_KeyGenParams			= 45,
	kPGPOptionType_KeyGenName			= 46,
	kPGPOptionType_Expiration			= 47,
	kPGPOptionType_AdditionalRecipientRequestKeySet	= 48,
	kPGPOptionType_KeyGenMasterKey		= 49,
	kPGPOptionType_KeyGenFast			= 50,

	kPGPOptionType_RevocationKeySet		= 51,

	kPGPOptionType_PreferredAlgorithms	= 52,

	kPGPOptionType_CommentString		= 53,
	kPGPOptionType_ExportPrivateKeys	= 54,
	kPGPOptionType_ExportPrivateSubkeys	= 55,

	kPGPOptionType_Exportable			= 56,
	kPGPOptionType_CertificateTrust		= 57,

	kPGPOptionType_Compression			= 58,
	kPGPOptionType_VersionString		= 59,
	kPGPOptionType_OmitMIMEVersion		= 60,

	kPGPOptionType_CertificateRegularExpression	= 61,

	kPGPOptionType_PassThroughClearSigned 	= 62,
	kPGPOptionType_PassThroughKeys			= 63,

	kPGPOptionType_CreationDate				= 64,

	kPGPOptionType_InputFileName			= 65,

	kPGPOptionType_ExportFormat				= 66,
	kPGPOptionType_OutputFormat				= 67,
	kPGPOptionType_InputFormat				= 68,

	kPGPOptionType_AttributeValue			= 69,

	kPGPOptionType_ExportKeySet				= 70,
	kPGPOptionType_ExportKey				= 71,
	kPGPOptionType_ExportUserID				= 72,
	kPGPOptionType_ExportSig				= 73,
	
	kPGPOptionType_LastSDKOption,
	
	kPGPOptionType_FirstUIOption			= 1000,
	
	kPGPOptionType_DialogPrompt				= 1000,
	kPGPOptionType_OutputPassphrase			= 1001,
	kPGPOptionType_Checkbox					= 1002,
	kPGPOptionType_PopupList				= 1003,
	kPGPOptionType_ShowPassphraseQuality	= 1004,
	kPGPOptionType_DialogOptions			= 1005,
	kPGPOptionType_MinPassphraseLength		= 1006,
	kPGPOptionType_MinPassphraseQuality		= 1007,
	kPGPOptionType_DefaultKey				= 1008,
	kPGPOptionType_VerifyPassphrase			= 1009,
	kPGPOptionType_FindMatchingKey			= 1010,
	kPGPOptionType_DefaultRecipients		= 1011,
	kPGPOptionType_DisplayMarginalValidity	= 1012,
	kPGPOptionType_IgnoreMarginalValidity	= 1013,
	kPGPOptionType_RecipientGroups			= 1014,
	kPGPOptionType_ParentWindowHandle       = 1015,	/* MS Windows only */
	kPGPOptionType_WindowTitle				= 1016,
	kPGPOptionType_ARREnforcement			= 1017,
	kPGPOptionType_KeyServerUpdateParams	= 1018,
	kPGPOptionType_KeyServerSearchKey		= 1019,
	kPGPOptionType_KeyServerSearchFilter	= 1020,
	kPGPOptionType_KeyServerSearchKeyIDList	= 1021,
	kPGPOptionType_KeyServerSearchKeySet	= 1022,
	kPGPOptionType_TextUI					= 1023,
	kPGPOptionType_RecipientList			= 1024,
	
	kPGPOptionType_LastUIOption,
	
	kPGPOptionType_FirstNetOption			= 2000,
	
	kPGPOptionType_URL						= 2000,
	kPGPOptionType_HostName					= 2001,
	kPGPOptionType_HostAddress				= 2002,
	kPGPOptionType_KeyServerProtocol		= 2003,
	kPGPOptionType_KeyServerKeySpace		= 2004,
	kPGPOptionType_KeyServerAccessType		= 2005,
	kPGPOptionType_KeyServerCAKey			= 2006,
	kPGPOptionType_KeyServerRequestKey		= 2007,
	
	kPGPOptionType_LastNetOption,

	PGP_ENUM_FORCE( PGPOptionType_ )
};
PGPENUM_TYPEDEF( PGPOptionType_, PGPOptionType );

enum
{
	/*
	 * kPGPOptionFlag_Critical means it should be considered an
	 * error if this option isn't recognized.
	 */
	kPGPOptionFlag_Critical	= ( 1 << 0 ),

	/*
	 * kPGPOptionFlag_Recognized is used internally to detect
	 * unrecognized critical options.
	 */
	kPGPOptionFlag_Recognized = ( 1 << 1 ),

	/*
	 * kPGPOptionFlag_Frozen means it cannot be changed,
	 * because it has already been read.
	 */
	kPGPOptionFlag_Frozen = ( 1 << 2 ),
	
	kPGPOptionFlag_Default	= kPGPOptionFlag_Critical
};

typedef PGPFlags	PGPOptionFlags;

typedef struct PGPOOutputBufferDesc
{
	void	*buffer;
	PGPSize	bufferSize;
	PGPSize	*outputDataLength;
	
} PGPOOutputBufferDesc;

typedef struct PGPOAllocatedOutputBufferDesc
{
	void	**buffer;
	PGPSize	maximumBufferSize;
	PGPSize	*actualBufferSize;
	
} PGPOAllocatedOutputBufferDesc;

typedef struct PGPOEventHandlerDesc
{
	PGPEventHandlerProcPtr	handlerProc;
	PGPUserValue			userValue;
	
} PGPOEventHandlerDesc;

typedef struct PGPOKeyGenParamsDesc
{
	PGPUInt32				pkalg;
	PGPUInt32				bits;

} PGPOKeyGenParamsDesc;

typedef struct PGPOAdditionalRecipientRequestKeySetDesc
{
	PGPKeySetRef			arKeySetRef;
	PGPUInt32				arkclass;

} PGPOAdditionalRecipientRequestKeySetDesc;

typedef struct PGPOCertificateTrustDesc
{
	PGPUInt32				trustLevel;
	PGPUInt32				trustValue;

} PGPOCertificateTrustDesc;

typedef struct PGPOPGPMIMEEncodingDesc
{
	PGPUInt32				mimeEncoding;
	PGPSize *				mimeBodyOffset;
	char *					mimeSeparator;
} PGPOPGPMIMEEncodingDesc;

typedef struct PGPOUICheckboxDesc
{
	PGPUInt32		itemID;
	const char *	title;
	const char *	description;
	PGPUInt32 *		valuePtr;

} PGPOUICheckboxDesc;

typedef struct PGPOUIPopupListDesc
{
	PGPUInt32		itemID;
	const char *	title;
	const char *	description;
	PGPUInt32		numListItems;
	const char **	listItems;
	PGPUInt32 *		valuePtr;

} PGPOUIPopupListDesc;

typedef struct PGPOUIKeyServerParamsDesc
{
	struct PGPKeyServerSpec	*serverList;
	PGPUInt32 				serverCount;
	PGPtlsContextRef		tlsContext;
	PGPBoolean 				searchBeforeDisplay;
	PGPKeySetRef 			*foundKeys;

} PGPOUIKeyServerParamsDesc;

typedef struct PGPOUIARRParamsDesc
{
	PGPUInt32	displayDialog	: 1;
	PGPUInt32	enforcement		: 31;
	
} PGPOUIARRParamsDesc;

typedef struct PGPOUIRecipientListDesc
{
	PGPUInt32			*recipientCount;
	PGPRecipientSpec	**recipientList;
	
} PGPOUIRecipientListDesc;

typedef struct PGPONetHostNameDesc
{
	const char 	*hostName;
	PGPUInt16 	port;
	const char	*path;
} PGPONetHostNameDesc;

typedef struct PGPONetHostAddressDesc
{
	PGPUInt32	hostAddress;
	PGPUInt16 	port;

} PGPONetHostAddressDesc;

typedef union
{
	PGPUInt32				asUInt;
	PGPInt32				asInt;
	void *					asPtr;
	const void *			asConstPtr;
	PGPTime					asTime;
	PGPTimeInterval			asInterval;
	PFLFileSpecRef			asFileRef;
	PGPUserIDRef			asUserIDRef;
	PGPSigRef				asSigRef;
	PGPKeyRef				asKeyRef;
	PGPKeySetRef			asKeySetRef;
	PGPCipherAlgorithm		asCipherAlgorithm;
	PGPHashAlgorithm		asHashAlgorithm;
	PGPValidity				asValidity;
	PGPGroupSetRef			asGroupSetRef;
	PGPFilterRef			asFilterRef;
	PGPtlsContextRef		asTLSContextRef;
	PGPOUIARRParamsDesc		asARRParams;
	
} PGPOptionValue;

enum
{
	kPGPOptionHandler_InvalidOperation		= 0,
	kPGPOptionHandler_FreeDataOperation		= 1,
	kPGPOptionHandler_CopyDataOperation		= 2
};

typedef PGPUInt32	PGPOptionHandlerOperation;

typedef PGPError 	(*PGPOptionHandlerProcPtr)(
								PGPContextRef context,
								PGPOptionHandlerOperation operation,
								PGPOptionType type,
								PGPOptionValue inputValue,
								PGPSize inputValueSize,
								PGPOptionValue *outputValue,
								PGPSize *outputValueSize );

struct PGPOption
{
	PGPOptionType			type;
	PGPOptionFlags			flags;
	PGPOptionValue			value;
	PGPSize					valueSize;
	PGPOptionListRef		subOptions;		/* Can be NULL */
	PGPOptionHandlerProcPtr	handlerProc;	/* Can be NULL */
};

typedef struct PGPOption	PGPOption;

#define kPGPEndOfArgsOptionListRef		((PGPOptionListRef) -1L)
#define kPGPOutOfMemoryOptionListRef	((PGPOptionListRef) -2L)
#define kPGPBadParamsOptionListRef		((PGPOptionListRef) -3L)
#define kPGPNullOptionListRef			((PGPOptionListRef) -4L)

PGP_BEGIN_C_DECLARATIONS

PGPError			pgpAddJobOptionsArgs(PGPJobRef job,
							PGPOptionListRef firstOption,
							va_list remainingOptions);
PGPError			pgpAppendOptionListArgs(PGPOptionListRef optionList,
							PGPOptionListRef firstOption,
							va_list remainingOptions);
PGPOptionListRef	pgpBuildOptionListArgs(
							PGPContextRef context,
							PGPBoolean createPersistentList,
							PGPOptionListRef firstOption,
							va_list remainingOptions);
PGPError			pgpFreeVarArgOptionList( PGPOptionListRef firstOption,
							va_list args );
	
PGPError			pgpCopyOption(PGPContextRef context,
						const PGPOption	*sourceOption, PGPOption *destOption);
PGPOptionListRef	pgpCopyOptionList(PGPOptionListRef optionList);
void				pgpFreeOptionList(PGPOptionListRef optionList);
PGPOptionListRef	pgpNewOneOptionList(PGPContextRef context,
						const PGPOption *option);
PGPOptionListRef	pgpNewOptionList(PGPContextRef context,
						PGPBoolean createPersistentList);
PGPBoolean			pgpOptionListIsReal(PGPOptionListRef optionList);
PGPBoolean			pgpOptionListIsValid(PGPOptionListRef optionList);
PGPError			pgpOptionListToError( PGPOptionListRef optionList );

PGPUInt32			pgpGetOptionListCount(PGPOptionListRef optionList);
PGPError			pgpGetOptionListError(PGPOptionListRef optionList);
void				pgpSetOptionListError(PGPOptionListRef optionList,
							PGPError error);
PGPError			pgpGetIndexedOption(PGPOptionListRef optionList,
							PGPUInt32 searchIndex,
							PGPBoolean markAsRecognized,
							PGPOption *optionData);
PGPError			pgpGetIndexedOptionType(PGPOptionListRef optionList,
							PGPOptionType optionType, PGPUInt32	searchIndex,
							PGPBoolean markAsRecognized, PGPOption *optionData,
							PGPUInt32 *numFoundOptions);

PGPError			pgpCheckOptionsInSet(PGPOptionListRef optionList,
							PGPOptionType const *optionSet,
							PGPUInt32 optionSetSize);

/* Parsing Helper Functions */

PGPError	pgpSearchOptionSingle(PGPOptionListRef optionList,
						PGPOptionType optionType, PGPOption *optionData);

PGPError	pgpSearchOption(PGPOptionListRef optionList,
						PGPOptionType optionType, PGPUInt32 searchIndex,
						PGPOption *optionData);

PGPError	pgpFindOptionArgs(PGPOptionListRef optionList,
						PGPOptionType optionType, PGPBoolean fMandatory,
						const char *str, ...);

void		pgpCleanupOptionList(PGPOptionListRef *optionList);

PGPError	pgpOptionInt(PGPOption *op, PGPInt32 *pInt);

PGPError	pgpOptionUInt(PGPOption *op, PGPUInt32 *pUInt);

PGPError	pgpOptionPtr(PGPOption *op, void **pPtr);

PGPError	pgpOptionInterval(PGPOption *op, PGPTimeInterval *pInterval);

PGPError	pgpOptionPtrLength(PGPOption *op, void **pPtr, PGPSize *pLength);

PGPError	pgpOptionPtrPtr(PGPOption *op, void **pPtr, void **pPtr2);

PGPError	pgpOptionPtrLengthPtr(PGPOption *op, void **pPtr, PGPSize *pLength,
						void **pPtr2) ;

char *		pgpAllocCString(PGPContextRef context, char const *string);

/* PGPO Creation Utilities */

PGPOptionListRef	pgpCreateCustomValueOptionList(PGPContextRef context,
							PGPOptionType type, PGPOptionFlags flags,
							const PGPOptionValue *value, PGPSize valueSize,
							PGPOptionListRef subOptions,
							PGPOptionHandlerProcPtr	handlerProc);
	
PGPOptionListRef	pgpCreateStandardValueOptionList(PGPContextRef context,
							PGPOptionType type, const PGPOptionValue *value,
							PGPSize valueSize,
							PGPOptionHandlerProcPtr handlerProc);
	
PGPOptionListRef	pgpCreateFileRefOptionList(PGPContextRef context,
							PGPOptionType type, PGPFileSpecRef fileRef);
	
PGPOptionListRef	pgpCreateSensitiveOptionList(PGPContextRef context,
							PGPOptionType type, void const *sensitiveData,
							PGPSize sensitiveDataSize);
	
PGPOptionListRef	pgpCreateBufferOptionList(PGPContextRef context,
							PGPOptionType type, void const *bufferData,
							PGPSize bufferDataSize);
	
					
PGPError	pgpSensitiveOptionHandlerProc(PGPContextRef context,
						PGPOptionHandlerOperation operation,
						PGPOptionType type, PGPOptionValue inputValue,
						PGPSize inputValueSize, PGPOptionValue *outputValue,
						PGPSize *outputValueSize);

PGPError	pgpBufferOptionHandlerProc(PGPContextRef context,
						PGPOptionHandlerOperation operation,
						PGPOptionType type, PGPOptionValue inputValue,
						PGPSize inputValueSize, PGPOptionValue *outputValue,
						PGPSize *outputValueSize);

PGPError	pgpFileRefOptionHandlerProc(PGPContextRef context,
						PGPOptionHandlerOperation operation,
						PGPOptionType type, PGPOptionValue inputValue,
						PGPSize inputValueSize, PGPOptionValue *outputValue,
						PGPSize *outputValueSize);
	
		
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpOptionListPriv_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
