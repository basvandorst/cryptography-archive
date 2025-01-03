/*____________________________________________________________________________
	pgpOptionList.h
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	This file contains the types and prototypes for functions which manipulate
	PGPOptionList data structures.

	$Id: pgpOptionList.h,v 1.12.10.1 1998/11/12 03:23:38 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpOptionList_h	/* [ */
#define Included_pgpOptionList_h

#include <stdarg.h>

#include "pgpPubTypes.h"

#if PGP_MACINTOSH
#include <Files.h>
#endif

/* Standard event callback declaration */
struct PGPEvent;
typedef PGPError (*PGPEventHandlerProcPtr)(PGPContextRef context,
						struct PGPEvent *event, PGPUserValue userValue);

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPError	PGPNewOptionList(PGPContextRef context,
					PGPOptionListRef *outList);
PGPError 	PGPAppendOptionList(PGPOptionListRef optionList,
					PGPOptionListRef firstOption, ...);
PGPError 	PGPBuildOptionList( PGPContextRef context,
					PGPOptionListRef *outList,
					PGPOptionListRef firstOption, ...);
PGPError	PGPCopyOptionList(PGPOptionListRef optionList,
					PGPOptionListRef *outList );
PGPError 	PGPFreeOptionList(PGPOptionListRef optionList);

/*
**	The following functions are used to create PGPOptionListRef's for
**	specifying the various options to several SDK functions. The
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
							PGPFileSpecRef fileRef);
PGPOptionListRef 		PGPOInputBuffer( PGPContextRef context,
							void const *buffer, PGPSize bufferSize);
#if PGP_MACINTOSH
PGPOptionListRef 		PGPOInputFileFSSpec( PGPContextRef context,
							const FSSpec *fileSpec);
#endif
	
/* Data output (optional, generates event if missing): */

PGPOptionListRef 		PGPOOutputFile( PGPContextRef context,
							PGPFileSpecRef fileRef);
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
PGPOptionListRef 		PGPOPasskeyBuffer( PGPContextRef context,
							const void *passkey, PGPSize passkeyLength);
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
PGPOptionListRef		PGPOX509Encoding( PGPContextRef context,
							PGPBoolean x509Encoding);
							
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
							PGPEventHandlerProcPtr eventHandler,
								PGPUserValue eventHandlerData);
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
PGPOptionListRef 		PGPOPassThroughClearSigned( PGPContextRef context,
							PGPBoolean passThroughClearSigned );
PGPOptionListRef 		PGPOPassThroughKeys( PGPContextRef context,
							PGPBoolean passThroughKeys );
PGPOptionListRef 		PGPORecursivelyDecode( PGPContextRef context,
							PGPBoolean recurse );

PGPOptionListRef		PGPOKeyGenParams( PGPContextRef context,
							PGPPublicKeyAlgorithm pubKeyAlg,
							PGPUInt32 bits);
								
PGPOptionListRef		PGPOKeyGenName( PGPContextRef context,
							const void *name, PGPSize nameLength);
							
PGPOptionListRef		PGPOCreationDate( PGPContextRef context,
							PGPTime creationDate);
PGPOptionListRef		PGPOExpiration( PGPContextRef context,
							PGPUInt32 expirationDays);
							
PGPOptionListRef		PGPOAdditionalRecipientRequestKeySet(
							PGPContextRef context, 
							PGPKeySetRef arKeySetRef, PGPByte arkClass);
								
PGPOptionListRef		PGPORevocationKeySet(PGPContextRef	context,
							PGPKeySetRef raKeySetRef);

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
							
PGPOptionListRef		PGPOFileNameString( PGPContextRef context,
							char const *fileName);
							
PGPOptionListRef		PGPOSigRegularExpression(PGPContextRef context,
							char const *regularExpression);
							
PGPOptionListRef		PGPOExportPrivateKeys( PGPContextRef context,
							PGPBoolean exportKeys);
							
PGPOptionListRef		PGPOExportPrivateSubkeys( PGPContextRef context,
							PGPBoolean exportSubkeys);
							
PGPOptionListRef		PGPOExportFormat(PGPContextRef context,
							PGPExportFormat exportFormat);

PGPOptionListRef		PGPOExportable( PGPContextRef context,
							PGPBoolean exportable);
							
PGPOptionListRef		PGPOSigTrust( PGPContextRef context,
							PGPUInt32 trustLevel,
							PGPUInt32 trustValue);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
	
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpOptionList_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
