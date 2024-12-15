/*____________________________________________________________________________
	pgpEncode.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	This file contains the prototypes for functions which build PGPOptionLists
	and encode/decode files and buffers.

	$Id: pgpEncode.h,v 1.90 1997/10/21 20:29:00 lloyd Exp $
____________________________________________________________________________*/

#ifndef Included_pgpEncode_h	/* [ */
#define Included_pgpEncode_h

#include "pgpPubTypes.h"

#include "pgpSymmetricCipher.h"
#include "pgpCFB.h"
#include "pgpHash.h"

#if PGP_MACINTOSH	/* [ */
#include <Files.h>

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#endif	/* ] */


/* Trust values, used to set validity values */
#define kPGPKeyTrust_Mask       0x7u
#define kPGPKeyTrust_Undefined	0x0u
#define kPGPKeyTrust_Unknown	0x1u
#define kPGPKeyTrust_Never		0x2u
#define kPGPKeyTrust_Marginal	0x5u
#define kPGPKeyTrust_Complete	0x6u
#define kPGPKeyTrust_Ultimate	0x7u

#define kPGPNameTrust_Mask      0x3u
#define kPGPNameTrust_Unknown	0x0u
#define kPGPNameTrust_Untrusted	0x1u
#define kPGPNameTrust_Marginal	0x2u
#define kPGPNameTrust_Complete	0x3u


/* Validity levels, used for thresholds in options */
enum PGPValidity_
{
	kPGPValidity_Unknown	= kPGPNameTrust_Unknown,
	kPGPValidity_Invalid	= kPGPNameTrust_Untrusted,
	kPGPValidity_Marginal	= kPGPNameTrust_Marginal,
	kPGPValidity_Complete	= kPGPNameTrust_Complete,
	kPGPValidity_UNTRUSTED	= kPGPValidity_Invalid,

	PGP_ENUM_FORCE( PGPValidity_ )
} ;
PGPENUM_TYPEDEF( PGPValidity_, PGPValidity );

/* Size of buffer for PGP-MIME separator (null terminated) */
#define kPGPMimeSeparatorSize	81



/* Line endings for PGPOOutputLineEndType */
enum PGPLineEndType_
{
	kPGPLineEnd_Default	= 0,
	kPGPLineEnd_LF		= 1,
	kPGPLineEnd_CR		= 2,
	kPGPLineEnd_CRLF	= (kPGPLineEnd_LF | kPGPLineEnd_CR),
	PGP_ENUM_FORCE( PGPLineEndType_ )
};
PGPENUM_TYPEDEF( PGPLineEndType_, PGPLineEndType );


/*
 * PGPOLocalEncoding options argument
 * Only one of Force or Auto should be used, and the other values are
 * modifiers.
 */
typedef PGPFlags	PGPLocalEncodingFlags;
#define kPGPLocalEncoding_None				0x0	/* nothing on */
#define kPGPLocalEncoding_Force				0x01
#define kPGPLocalEncoding_Auto				0x02
#define kPGPLocalEncoding_NoMacBinCRCOkay	0x04



/*
 * The PGPEvent structure is used to notify clients of the encode API of
 * various events.
 */

/* PGP Event types */

enum PGPEventType_
{
	kPGPEvent_NullEvent			=  0,		/* Nothing happened */
	kPGPEvent_InitialEvent		=  1,		/* Final event */
	kPGPEvent_FinalEvent		=  2,		/* Final event */
	kPGPEvent_ErrorEvent		=  3,		/* An error occurred */
	kPGPEvent_WarningEvent		=  4,		/* Warning event */
	kPGPEvent_EntropyEvent		=  5,		/* More entropy is needed */
	kPGPEvent_PassphraseEvent	=  6,		/* A passphrase is needed */
	kPGPEvent_InsertKeyEvent	=  7,		/* Smart card must be inserted */
	kPGPEvent_AnalyzeEvent		=  8,		/* Initial analysis event,
											   before any output */
	kPGPEvent_RecipientsEvent	=  9,		/* Recipient list report,
											   before any output */
	kPGPEvent_KeyFoundEvent		= 10,		/* Key packet found */
	kPGPEvent_OutputEvent		= 11,		/* Output specification needed */
	kPGPEvent_SignatureEvent	= 12,		/* Signature status report */
	kPGPEvent_BeginLexEvent		= 13,		/* Initial event per lexical unit*/
	kPGPEvent_EndLexEvent		= 14,		/* Final event per lexical unit */
	kPGPEvent_RecursionEvent	= 15,		/* Notification of recursive
											   job creation */
	kPGPEvent_DetachedSignatureEvent = 16,	/* Need input for verification of
											   detached signature */
	kPGPEvent_KeyGenEvent		= 17,		/* Key generation progress */
	kPGPEvent_KeyServerEvent	= 18,		/* Key Server progress */
	kPGPEvent_KeyServerSignEvent= 19,		/* Key Server passphrase */
	PGP_ENUM_FORCE( PGPEventType_ )
};
PGPENUM_TYPEDEF( PGPEventType_, PGPEventType );


/* PGP Analyze event callback codes */

enum PGPAnalyzeType_
{
	kPGPAnalyze_Encrypted			=  0,	/* Encrypted message */
	kPGPAnalyze_Signed				=  1,	/* Signed message */
	kPGPAnalyze_DetachedSignature	=  2,	/* Detached signature */
	kPGPAnalyze_Key					=  3,	/* Key data */
	kPGPAnalyze_Unknown				=  4,	/* Non-pgp message */

	PGP_ENUM_FORCE( PGPAnalyzeType_ )
};
PGPENUM_TYPEDEF( PGPAnalyzeType_, PGPAnalyzeType );




/* Individual event information structs, combined as a union in PGPEvent */

typedef struct PGPEventNullData_
{
	PGPFileOffset			bytesWritten;
	PGPFileOffset			bytesTotal;
} PGPEventNullData;

typedef struct PGPEventErrorData_
{
	PGPError				error;
	void				   *errorArg;
} PGPEventErrorData;

typedef struct PGPEventWarningData_
{
	PGPError				warning;
	void				   *warningArg;
} PGPEventWarningData;

typedef struct PGPEventEntropyData_
{
	PGPUInt32				entropyBitsNeeded;
} PGPEventEntropyData;

typedef struct PGPEventPassphraseData_
{
	PGPBoolean				fConventional;
	PGPKeySetRef			keyset;
} PGPEventPassphraseData;

typedef struct PGPEventRecipientsData_
{
	PGPKeySetRef			recipientSet;
	PGPUInt32				conventionalPassphraseCount;
	PGPUInt32				keyCount;
	PGPKeyID const *		keyIDArray;
} PGPEventRecipientsData;

typedef struct PGPEventKeyFoundData_
{
	PGPKeySetRef			keySet;
} PGPEventKeyFoundData;

typedef struct PGPEventSignatureData_
{
	PGPKeyID				signingKeyID;
	PGPKeyRef				signingKey;
	PGPBoolean				checked;
	PGPBoolean				verified;
	PGPBoolean				keyRevoked;
	PGPBoolean				keyDisabled;
	PGPBoolean				keyExpired;
	PGPBoolean				keyMeetsValidityThreshold;
	PGPValidity				keyValidity;
	PGPTime					creationTime;
} PGPEventSignatureData;

typedef struct PGPEventAnalyzeData_
{
	PGPAnalyzeType			sectionType;
} PGPEventAnalyzeData;

typedef struct PGPEventOutputData_
{
	PGPUInt32				messageType;
	char				   *suggestedName;
	PGPBoolean				forYourEyesOnly;
} PGPEventOutputData;

typedef struct PGPEventBeginLexData_
{
	PGPUInt32				sectionNumber;
	PGPSize					sectionOffset;
} PGPEventBeginLexData;

typedef struct PGPEventEndLexData_
{
	PGPUInt32				sectionNumber;
} PGPEventEndLexData;

typedef struct PGPEventKeyGenData_
{
	PGPUInt32				state;
} PGPEventKeyGenData;

typedef struct PGPEventKeyServerData_
{
	PGPUInt32				state;
	PGPUInt32				soFar;
	PGPUInt32				total;
} PGPEventKeyServerData;

typedef struct PGPEventKeyServerSignData_
{
	PGPUInt32				state;
} PGPEventKeyServerSignData;


/*
 * The following events have no event-specific data defined for them:
 *	kPGPEvent_InsertKeyEvent
 *	kPGPEvent_RecursionEvent
 *	kPGPEvent_DetachedSignatureEvent
 *	kPGPEvent_InitialEvent
 *	kPGPEvent_FinalEvent
 */

/* Union of all event data structures above */
typedef union PGPEventData_
{
	PGPEventNullData			nullData;
	PGPEventErrorData			errorData;
	PGPEventWarningData			warningData;
	PGPEventEntropyData			entropyData;
	PGPEventPassphraseData		passphraseData;
	PGPEventRecipientsData		recipientsData;
	PGPEventKeyFoundData		keyFoundData;
	PGPEventSignatureData		signatureData;
	PGPEventAnalyzeData			analyzeData;
	PGPEventOutputData			outputData;
	PGPEventBeginLexData		beginLexData;
	PGPEventEndLexData			endLexData;
	PGPEventKeyGenData			keyGenData;
	PGPEventKeyServerData		keyServerData;
	PGPEventKeyServerSignData	keyServerSignData;
} PGPEventData;

/* Refs to internal "job" structure */
typedef struct PGPJob *				PGPJobRef;
typedef const struct PGPJob *		PGPConstPGPJobRef;

#define	kInvalidPGPJobRef			((PGPJobRef) NULL)
#define PGPJobRefIsValid( ref )		( (ref) != kInvalidPGPJobRef )

/* PGPEvent structure */

struct PGPEvent_
{
	PGPVersion				 version;		/* Version of event structure */
	struct PGPEvent_		*nextEvent;		/* Allow lists of events */
	PGPJobRef				 job;			/* Associated with what job */
	PGPEventType			 type;			/* Type of event */
	PGPEventData			 data;			/* Event specific data */
};
typedef struct PGPEvent_ PGPEvent;


/* Standard event callback declaration */
typedef PGPError (*PGPEventHandlerProcPtr)(PGPContextRef context,
						PGPEvent *event, PGPUserValue userValue);


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/*
**	Functions to encode and decode. The variable parameters are one or more
**	PGPOptionListRef's which describe the inputs, outputs, and options.
*/

PGPError 				PGPEncode(PGPContextRef context,
								PGPOptionListRef firstOption, ...);
PGPError 				PGPDecode(PGPContextRef context,
								PGPOptionListRef firstOption, ...);

PGPError				PGPNewOptionList( PGPContextRef context,
							PGPOptionListRef *outList );
PGPError 				PGPAppendOptionList(
								PGPContextRef context,
								PGPOptionListRef optionList,
								PGPOptionListRef firstOption, ...);
PGPError 				PGPBuildOptionList( PGPContextRef context,
							PGPOptionListRef *outList,
							PGPOptionListRef firstOption, ...);
PGPError		 		PGPCopyOptionList(PGPConstOptionListRef optionList,
							PGPOptionListRef *outList );
PGPError 				PGPFreeOptionList(PGPOptionListRef optionList);
PGPError 				PGPAddJobOptions(PGPJobRef job,
								PGPOptionListRef firstOption, ...);


/*
**	The following functions are used to create PGPOptionListRef's for
**	specifying the various options to PGPEncode() and PGPEncode(). The
**	functions can be used as inline parameters in a temporary manner or
**	used with PGPBuildOptionList() to create persistent lists.
*/

/*
** Special PGPOptionListRef to mark last option passed to those functions
** which take variable lists of PGPOptionListRef's:
*/

PGPOptionListRef 		PGPOLastOption( PGPContextRef context );

/*
** Special PGPOptionListRef which is always ignored:
*/

PGPOptionListRef 		PGPONullOption( PGPContextRef context);

/* Data input (required): */

PGPOptionListRef 		PGPOInputFile( PGPContextRef context,
							PGPConstFileSpecRef fileRef);
PGPOptionListRef 		PGPOInputBuffer( PGPContextRef context,
							void const *buffer, PGPSize bufferSize);
#if PGP_MACINTOSH
PGPOptionListRef 		PGPOInputFileFSSpec( PGPContextRef context,
							const FSSpec *fileSpec);
#endif
	
/* Data output (optional, generates event if missing): */

PGPOptionListRef 		PGPOOutputFile( PGPContextRef context,
							PGPConstFileSpecRef fileRef);
PGPOptionListRef 		PGPOOutputBuffer( PGPContextRef context,
							void *buffer, PGPSize bufferSize,
							PGPSize *outputDataLength);
#if PGP_MACINTOSH
PGPOptionListRef 		PGPOOutputFileFSSpec( PGPContextRef context,
							const FSSpec *fileSpec);
#endif

/* '*buffer' must be disposed of via PGPFreeData() */
/* maximum memory usage will be no more than maximumBufferSize */
PGPOptionListRef 		PGPOAllocatedOutputBuffer(PGPContextRef context,
							void **buffer,
							PGPSize maximumBufferSize,
							PGPSize *actualBufferSize);
PGPOptionListRef 		PGPOAppendOutput( PGPContextRef context,
							PGPBoolean appendOutput );
PGPOptionListRef 		PGPODiscardOutput( PGPContextRef context,
							PGPBoolean discardOutput );
PGPOptionListRef 		PGPOAskUserForOutput( PGPContextRef context,
							PGPBoolean askUserForOutput );

/* Encrypting and signing */

PGPOptionListRef 		PGPOEncryptToKey( PGPContextRef context,
							PGPKeyRef keyRef);
PGPOptionListRef 		PGPOEncryptToKeySet( PGPContextRef context,
							PGPKeySetRef keySetRef);
PGPOptionListRef 		PGPOEncryptToUserID( PGPContextRef context,
							PGPUserIDRef userIDRef);
PGPOptionListRef 		PGPOSignWithKey( PGPContextRef context,
							PGPKeyRef keyRef,
							PGPOptionListRef firstOption, ...);
PGPOptionListRef 		PGPOConventionalEncrypt( PGPContextRef context,
							PGPOptionListRef firstOption,
								...);

PGPOptionListRef 		PGPOPassphraseBuffer( PGPContextRef context,
							const void *passphrase, PGPSize passphraseLength);
PGPOptionListRef 		PGPOPassphrase( PGPContextRef context,
							const char *passphrase);
PGPOptionListRef 		PGPOAskUserForEntropy( PGPContextRef context,
							PGPBoolean askUserForEntropy );
PGPOptionListRef 		PGPORawPGPInput( PGPContextRef context,
							PGPBoolean rawPGPInput );
PGPOptionListRef 		PGPOCompression( PGPContextRef context,
							PGPBoolean compression );

PGPOptionListRef		PGPOLocalEncoding( PGPContextRef context,
							PGPLocalEncodingFlags localEncode);
PGPOptionListRef		PGPOOutputLineEndType(PGPContextRef context,
							PGPLineEndType lineEnd);
PGPOptionListRef		PGPOPGPMIMEEncoding(PGPContextRef context,
							PGPBoolean mimeEncoding, PGPSize *mimeBodyOffset,
							char mimeSeparator[ kPGPMimeSeparatorSize ]);
PGPOptionListRef		PGPOOmitMIMEVersion( PGPContextRef context,
							PGPBoolean omitVersion);
							
PGPOptionListRef 		PGPODetachedSig( PGPContextRef context,
							PGPOptionListRef firstOption,
								...);

PGPOptionListRef 		PGPOCipherAlgorithm( PGPContextRef context,
							PGPCipherAlgorithm algorithm);
PGPOptionListRef 		PGPOHashAlgorithm( PGPContextRef context,
							PGPHashAlgorithm algorithm);

PGPOptionListRef 		PGPOFailBelowValidity( PGPContextRef context,
							PGPValidity minValidity);
PGPOptionListRef 		PGPOWarnBelowValidity( PGPContextRef context,
							PGPValidity minValidity);


PGPOptionListRef 		PGPOEventHandler( PGPContextRef context,
							PGPEventHandlerProcPtr handler,
								PGPUserValue userValue);
PGPOptionListRef 		PGPOSendNullEvents( PGPContextRef context,
							PGPTimeInterval approxInterval);
	
PGPOptionListRef 		PGPOArmorOutput( PGPContextRef context,
							PGPBoolean armorOutput );
PGPOptionListRef 		PGPODataIsASCII( PGPContextRef context,
							PGPBoolean dataIsASCII );
PGPOptionListRef 		PGPOClearSign( PGPContextRef context,
							PGPBoolean clearSign );
PGPOptionListRef 		PGPOForYourEyesOnly( PGPContextRef context,
							PGPBoolean forYourEyesOnly );
PGPOptionListRef 		PGPOKeySetRef( PGPContextRef context,
							PGPKeySetRef keysetRef);

PGPOptionListRef 		PGPOImportKeysTo( PGPContextRef context,
							PGPKeySetRef keysetRef);
PGPOptionListRef 		PGPOSendEventIfKeyFound( PGPContextRef context,
							PGPBoolean sendEventIfKeyFound );
PGPOptionListRef 		PGPOPassThroughIfUnrecognized( PGPContextRef context,
							PGPBoolean passThroughIfUnrecognized );

PGPOptionListRef		PGPOKeyGenParams( PGPContextRef context,
							PGPPublicKeyAlgorithm pubKeyAlg,
							PGPUInt32 bits);
								
PGPOptionListRef		PGPOKeyGenName( PGPContextRef context,
							const void *name, PGPSize nameLength);
							
PGPOptionListRef		PGPOExpiration( PGPContextRef context,
							PGPUInt32 expirationDays);
							
PGPOptionListRef		PGPOAdditionalRecipientRequestKeySet(
							PGPContextRef context, 
							PGPKeySetRef arKeySetRef, PGPByte arkClass);
								
PGPOptionListRef		PGPOKeyGenMasterKey( PGPContextRef context,
							PGPKeyRef masterKeyRef);
							
PGPOptionListRef		PGPOPreferredAlgorithms(
							PGPContextRef context, 
							PGPCipherAlgorithm const *prefAlg,
							PGPUInt32 numAlgs);
								
PGPOptionListRef		PGPOKeyGenFast( PGPContextRef context,
							PGPBoolean fastGen);
							
PGPOptionListRef		PGPOCommentString( PGPContextRef context,
							char const *comment);

PGPOptionListRef		PGPOVersionString( PGPContextRef context,
							char const *version);
							
PGPOptionListRef		PGPOSigRegularExpression(PGPContextRef context,
							char const *regularExpression);
							
PGPOptionListRef		PGPOExportPrivateKeys( PGPContextRef context,
							PGPBoolean exportPrivate);
							
PGPOptionListRef		PGPOExportable( PGPContextRef context,
							PGPBoolean exportable);
							
PGPOptionListRef		PGPOSigTrust( PGPContextRef context,
							PGPUInt32 trustLevel,
							PGPUInt32 trustValue);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS



#if PGP_MACINTOSH
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif
#endif


#endif /* ] Included_pgpEncode_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
