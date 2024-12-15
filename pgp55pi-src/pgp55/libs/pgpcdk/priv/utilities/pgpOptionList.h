/*____________________________________________________________________________
	pgpOptionList.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	This file contains the types and prototypes for functions which manipulate
	PGPOptionList data structures.

	$Id: pgpOptionList.h,v 1.59 1997/10/17 06:15:14 hal Exp $
____________________________________________________________________________*/

#ifndef Included_pgpOptionList_h	/* [ */
#define Included_pgpOptionList_h

#include <stdarg.h>

#include "pgpPubTypes.h"
#include "pgpEncode.h"

enum PGPOptionType_
{
	kPGPOptionType_InvalidOption		= 0,
	
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
	
	/* kPGPOptionType_AskUserForPassphrase	= 19, */

	kPGPOptionType_DetachedSignature	= 20,
	/* kPGPOptionType_MailHeaders			= 21, */

	kPGPOptionType_CipherAlgorithm		= 22,
	kPGPOptionType_HashAlgorithm		= 23,
	kPGPOptionType_FailBelowValidity	= 24,
	kPGPOptionType_WarnBelowValidity	= 25,

	/* kPGPOptionType_PGPEncoding			= 26, */
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
	/* kPGPOptionType_RecursivelyDecode	= 41, */
	kPGPOptionType_PassThroughIfUnrecognized= 42,

	kPGPOptionType_KeySetRef			= 43,
	/* kPGPOptionType_SignatureStatus		= 44, */

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

	kPGPOptionType_Exportable			= 55,
	kPGPOptionType_CertificateTrust		= 56,

	kPGPOptionType_Compression			= 57,
	kPGPOptionType_VersionString		= 58,
	kPGPOptionType_OmitMIMEVersion		= 59,

	kPGPOptionType_CertificateRegularExpression	= 60,

	kPGPOptionType_LastType,
	
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
	kPGPOptionFlag_Frozen = ( 1 << 2 )
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

typedef union
{
	PGPUInt32			asUInt;
	PGPInt32			asInt;
	void *				asPtr;
	const void *		asConstPtr;
	PGPTimeInterval		asInterval;
	PFLFileSpecRef		asFileRef;
	PGPUserIDRef		asUserIDRef;
	PGPKeyRef			asKeyRef;
	PGPKeySetRef		asKeySetRef;
	PGPCipherAlgorithm	asCipherAlgorithm;
	PGPHashAlgorithm	asHashAlgorithm;
	PGPValidity			asValidity;
	
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
	
PGPOptionListRef	pgpCopyOptionList(PGPConstOptionListRef optionList);
void				pgpFreeOptionList(PGPOptionListRef optionList);
PGPOptionListRef	pgpNewOneOptionList(PGPContextRef context,
						const PGPOption *option);
PGPOptionListRef	pgpNewOptionList(PGPContextRef context,
						PGPBoolean createPersistentList);
PGPBoolean			pgpOptionListIsReal(PGPConstOptionListRef optionList);
PGPBoolean			pgpOptionListIsValid(PGPConstOptionListRef optionList);
PGPError			pgpOptionListToError( PGPConstOptionListRef optionList );

PGPError			pgpGetOptionListError(PGPConstOptionListRef optionList);
void				pgpSetOptionListError(PGPOptionListRef optionList,
							PGPError error);
PGPError			pgpGetIndexedOption(PGPContextRef context,
							PGPOptionListRef optionList,
							PGPOptionType optionType, PGPUInt32	searchIndex,
							PGPBoolean markAsRecognized, PGPOption *optionData,
							PGPUInt32 *numFoundOptions);

PGPError			pgpCheckOptionsInSet(PGPContextRef context,
							PGPOptionListRef optionList,
							PGPOptionType const *optionSet,
							PGPUInt32 optionSetSize);
	
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpOptionList_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
