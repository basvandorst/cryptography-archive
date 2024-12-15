/*____________________________________________________________________________
	pgpClientEncode.c
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpClientEncode.c,v 1.102.6.1 1999/06/11 06:14:33 heller Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpSDKBuildFlags.h"

#include "pgpErrors.h"
#include "pgpFileSpec.h"
#include "pgpMem.h"

#include "pgpContext.h"
#include "pgpEncode.h"
#include "pgpEncodePriv.h"
#include "pgpKeys.h"
#include "pgpOptionList.h"
#include "pgpOpaqueStructs.h"
#include "pgpKDBInt.h"
#include "pgpHashPriv.h"
#include "pgpSymmetricCipherPriv.h"

#if PGP_MACINTOSH
#include "MacFiles.h"
#endif

	static PGPError
pgpEncode(
	PGPContextRef		context,
	PGPOptionListRef 	firstOption,
	va_list				args)
{
	PGPError			err;
	PGPOptionListRef	optionList;
	
	pgpAssert( pgpContextIsValid( context ) );
	
	optionList = pgpBuildOptionListArgs( context, FALSE, firstOption, args );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpEncodeInternal( context, optionList );
	}
	
	PGPFreeOptionList( optionList );

	return( err );
}

	PGPError
PGPEncode(
	PGPContextRef		context,
	PGPOptionListRef 	firstOption,
	...)
{
	PGPError	err;
	va_list		args;
	
	pgpAssert( pgpContextIsValid( context ) );
	
	if( pgpContextIsValid( context ) )
	{
		va_start( args, firstOption );
			err = pgpEncode( context, firstOption, args );
		va_end( args );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		err = kPGPError_BadParams;
	}
	
	return( err );
}

	static PGPError
pgpDecode(
	PGPContextRef		context,
	PGPOptionListRef	firstOption,
	va_list				args)
{
	PGPError			err;
	PGPOptionListRef	optionList;
	
	pgpAssert( pgpContextIsValid( context ) );
	
	optionList = pgpBuildOptionListArgs( context, FALSE, firstOption, args );
	
	err = pgpGetOptionListError( optionList );
	if( IsntPGPError( err ) )
	{
		err = pgpDecodeInternal( context, optionList );
		
	}
	
	PGPFreeOptionList( optionList );
	
	return( err );
}

	PGPError
PGPDecode(
	PGPContextRef 		context,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err;
	va_list		args;
	
	pgpAssert( pgpContextIsValid( context ) );
	
	if( pgpContextIsValid( context ) )
	{
		va_start( args, firstOption );
			err = pgpDecode( context, firstOption, args );
		va_end( args );
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		err = kPGPError_BadParams;
	}
	
	return( err );
}

	PGPError
PGPNewOptionList(
	PGPContextRef		context,
	PGPOptionListRef *	outList )
{
	PGPOptionListRef	optionList;
	PGPError			err	= kPGPError_NoErr;
	
	PGPValidatePtr( outList );
	*outList	= kInvalidPGPOptionListRef;
	PGPValidateContext( context );
	
	/* Option lists built with PGPNewOptionList are always persistent */
	optionList = pgpNewOptionList( context, TRUE );
	if ( pgpOptionListIsReal( optionList ) )
	{
		*outList	= optionList;
	}
	else
	{
		err	= pgpOptionListToError( optionList );
	}
	
	return( err );
}

	PGPError
PGPAddJobOptions(
	PGPJobRef			job,
	PGPOptionListRef	firstOption,
	...)
{
	PGPError	err	= kPGPError_NoErr;
	va_list		args;
		
	PGPValidatePtr( job );
	
	va_start( args, firstOption );
		err = pgpAddJobOptionsArgs( job, firstOption, args );
	va_end( args );
	
	return( err );
}

	PGPError
PGPAppendOptionList(
	PGPOptionListRef	optionList,
	PGPOptionListRef	firstOption,
	...) 
{
	PGPError	err = kPGPError_NoErr;
	
	/* don't validate 'optionList' here */
	{
		va_list	args;
		
		va_start( args, firstOption );
			err = pgpAppendOptionListArgs( optionList, firstOption, args );
		va_end( args );
	}
	
	return( err );
}

	PGPError
PGPBuildOptionList(
	PGPContextRef		context,
	PGPOptionListRef *	outList,
	PGPOptionListRef firstOption,
	...) 
{
	PGPOptionListRef	optionList;
	PGPError			err	= kPGPError_NoErr;

	PGPValidatePtr( outList );
	*outList	= NULL;
	PGPValidateContext( context );
	
	/* don't validate 'firstOption' here */
	{
		va_list	args;
		
		/* Option lists built with pgpBuildOptionListInternal are
		   always persistent */
		va_start( args, firstOption );
			optionList = pgpBuildOptionListArgs( context,
				TRUE, firstOption, args );
		va_end( args );
	}
	
	if ( pgpOptionListIsReal( optionList ) )
	{
		*outList	= optionList;
	}
	else
	{
		err	= pgpOptionListToError( optionList );
	}
	
	return( err );
}

	PGPError
PGPCopyOptionList(
	PGPOptionListRef	optionList,
	PGPOptionListRef *		outList
	) 
{
	PGPOptionListRef	newOptionList;
	PGPError			err	= kPGPError_NoErr;
	
	PGPValidatePtr( outList );
	*outList	= NULL;
	PGPValidateParam( pgpOptionListIsValid( optionList ) );
	
	newOptionList = pgpCopyOptionList( optionList );
	
	if ( pgpOptionListIsReal( newOptionList ) )
	{
		*outList	= newOptionList;
	}
	else
	{
		err	= pgpOptionListToError( newOptionList );
	}
	
	return( err );
}

	PGPError
PGPFreeOptionList(PGPOptionListRef optionList)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateParam( pgpOptionListIsValid( optionList ) );
	
	pgpFreeOptionList( optionList );
	
	return( err );
}



/*
**	This is the handler proc for standard allocated options.
**	The free procedure calls pgpContextMemFree() to release
**	the allocation and the copy procedure calls pgpContextMemAlloc()
**	to create a copy.
*/

	static PGPError
AllocatedOptionHandlerProc(
	PGPContextRef				context,
	PGPOptionHandlerOperation 	operation,
	PGPOptionType				type,
	PGPOptionValue				inputValue,
	PGPSize 					inputValueSize,
	PGPOptionValue 				*outputValue,
	PGPSize						*outputValueSize)
{
	PGPError	err = kPGPError_NoErr;
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			if( type == kPGPOptionType_AdditionalRecipientRequestKeySet ||
				type == kPGPOptionType_RevocationKeySet ) {
				PGPOAdditionalRecipientRequestKeySetDesc	*descriptor;
				descriptor = (PGPOAdditionalRecipientRequestKeySetDesc *)
														inputValue.asPtr;
				PGPFreeKeySet( descriptor->arKeySetRef );
			}
				
			pgpContextMemFree( context, inputValue.asPtr );
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			pgpAssertAddrValid( outputValue, PGPOptionValue );
			pgpAssertAddrValid( outputValueSize, PGPSize );

			outputValue->asPtr = pgpContextMemAlloc( context, inputValueSize,
													 0 );
			if( IsntNull( outputValue->asPtr ) )
			{	
				pgpCopyMemory( inputValue.asPtr, outputValue->asPtr,
							   inputValueSize );
				
				if( type == kPGPOptionType_AdditionalRecipientRequestKeySet||
					type == kPGPOptionType_RevocationKeySet ) {
					PGPOAdditionalRecipientRequestKeySetDesc	*descriptor;
					descriptor = (PGPOAdditionalRecipientRequestKeySetDesc *)
															outputValue->asPtr;
					PGPIncKeySetRefCount( descriptor->arKeySetRef );
				}

				*outputValueSize = inputValueSize;
			}
			else
			{
				err = kPGPError_OutOfMemory;
			}
			
			break;
		}
		
		default:
		{
			err = kPGPError_UnknownRequest;
			break;
		}
	}

	return( err );
}

/*
**	This is the handler proc for options which don't need any
**	routine copying, but for which certain ones may need special
**	treatment.
*/

	static PGPError
SpecialOptionHandlerProc(
	PGPContextRef				context,
	PGPOptionHandlerOperation 	operation,
	PGPOptionType				type,
	PGPOptionValue				inputValue,
	PGPSize 					inputValueSize,
	PGPOptionValue 				*outputValue,
	PGPSize						*outputValueSize)
{
	PGPError	err = kPGPError_NoErr;
	
	(void) context;
	(void) inputValueSize;
	(void) outputValueSize;

	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			switch( type ) {
			case kPGPOptionType_EncryptToKey:
			case kPGPOptionType_SignWithKey:
			case kPGPOptionType_KeyGenMasterKey:
				pgpFreeKey( inputValue.asKeyRef );
				break;
			case kPGPOptionType_EncryptToKeySet:
			case kPGPOptionType_KeySetRef:
			case kPGPOptionType_ImportKeysTo:
				PGPFreeKeySet( inputValue.asKeySetRef );
				break;
			case kPGPOptionType_EncryptToUserID:
				pgpFreeUserID( inputValue.asUserIDRef );
				break;
			default:
				break;
			}
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			switch( type ) {
			case kPGPOptionType_EncryptToKey:
			case kPGPOptionType_SignWithKey:
			case kPGPOptionType_KeyGenMasterKey:
				pgpIncKeyRefCount( outputValue->asKeyRef );
				break;
			case kPGPOptionType_EncryptToKeySet:
			case kPGPOptionType_KeySetRef:
			case kPGPOptionType_ImportKeysTo:
				PGPIncKeySetRefCount( outputValue->asKeySetRef );
				break;
			case kPGPOptionType_EncryptToUserID:
				pgpIncUserIDRefCount( outputValue->asUserIDRef );
				break;
			default:
				break;
			}
			break;
		}
		
		default:
		{
			err = kPGPError_UnknownRequest;
			break;
		}
	}

	return( err );
}


	
	PGPOptionListRef
PGPOLastOption( PGPContextRef context )
{
	pgpValidateOptionContext( context );

	return( kPGPEndOfArgsOptionListRef );
}

	PGPOptionListRef
PGPONullOption( PGPContextRef context )
{
	pgpValidateOptionContext( context );

	return( kPGPNullOptionListRef );
}

	PGPOptionListRef
PGPOInputFile(
	PGPContextRef 	context,
	PGPFileSpecRef fileRef)
{
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pflFileSpecIsValid( (PFLFileSpecRef)fileRef ) );
	
	return( pgpCreateFileRefOptionList( context,
				kPGPOptionType_InputFileRef, fileRef ) );
}

#if PGP_MACINTOSH	/* [ */

	PGPOptionListRef
PGPOInputFileFSSpec(
	PGPContextRef 	context,
	const FSSpec	*fileSpec)
{
	PGPOptionListRef	optionList;
	PGPFileSpecRef		fileRef;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( FSSpecIsValid( fileSpec ) );
	
	if( IsntPGPError( PGPNewFileSpecFromFSSpec( context,
			fileSpec, &fileRef ) ) )
	{
		optionList = pgpCreateFileRefOptionList( context,
			kPGPOptionType_InputFileRef, fileRef );
			
		PGPFreeFileSpec( fileRef );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

#endif	/* ] */

	PGPOptionListRef
PGPOInputBuffer(
	PGPContextRef	context,
	void const *	buffer,
	PGPSize			bufferSize)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( buffer ) );
	/* buffer size may be 0 */
	
	value.asConstPtr	= buffer;
	optionList = pgpCreateStandardValueOptionList( context, 
						kPGPOptionType_InputBuffer,
						&value, bufferSize, NULL );

	return( optionList );
}

	PGPOptionListRef
PGPOOutputFile(
	PGPContextRef	context,
	PGPFileSpecRef fileRef)
{
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pflFileSpecIsValid( (PFLFileSpecRef)fileRef ) );
	
	return( pgpCreateFileRefOptionList( context,
				kPGPOptionType_OutputFileRef, fileRef ) );
}

	PGPOptionListRef
PGPOOutputBuffer(
	PGPContextRef	context,
	void			*buffer,
	PGPSize			bufferSize,
	PGPSize			*outputDataLength)
{
	PGPOptionListRef		optionList;
	PGPOOutputBufferDesc	*descriptor;
	
	pgpValidateOptionParam( IsntNull( outputDataLength ) );
	*outputDataLength	= 0;
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( buffer ) && bufferSize > 0 );

	pgpDebugWhackMemory( buffer, bufferSize );
	
	descriptor = (PGPOOutputBufferDesc *)
		pgpContextMemAlloc( context, sizeof(*descriptor),
		kPGPMemoryMgrFlags_Clear);
	if( IsntNull( descriptor ) )
	{
		PGPOptionValue	value;
		
		descriptor->buffer 				= buffer;
		descriptor->bufferSize 			= bufferSize;
		descriptor->outputDataLength	= outputDataLength;
		
		value.asPtr = descriptor;
		
		optionList = pgpCreateStandardValueOptionList( context, 
							kPGPOptionType_OutputBuffer,
							&value, sizeof( *descriptor ),
							AllocatedOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

#if PGP_MACINTOSH	/* [ */

	PGPOptionListRef
PGPOOutputFileFSSpec(
	PGPContextRef 	context,
	const FSSpec	*fileSpec)
{
	PGPOptionListRef	optionList;
	PGPFileSpecRef		fileRef;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( FSSpecIsValid( fileSpec ) );
	
	if( IsntPGPError( PGPNewFileSpecFromFSSpec( context,
			fileSpec, &fileRef ) ) )
	{
		optionList = pgpCreateFileRefOptionList( context,
			kPGPOptionType_OutputFileRef, fileRef );
			
		PGPFreeFileSpec( fileRef );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

#endif	/* ] */

	PGPOptionListRef
PGPOAllocatedOutputBuffer(
	PGPContextRef	context,
	void			**buffer,
	PGPSize			maximumBufferSize,
	PGPSize			*actualBufferSize)
{
	PGPOptionListRef				optionList;
	PGPOAllocatedOutputBufferDesc	*descriptor;
	
	if ( IsntNull( buffer ) )
		*buffer	= NULL;
	if ( IsntNull( actualBufferSize ) )
		*actualBufferSize	= 0;
	pgpValidateOptionParam( IsntNull( buffer ) );
	pgpValidateOptionParam( IsntNull( actualBufferSize ) );
	pgpValidateOptionParam( maximumBufferSize > 0 );
	pgpValidateOptionContext( context );
	
	descriptor = (PGPOAllocatedOutputBufferDesc *)
			pgpContextMemAlloc( context,
				sizeof(*descriptor), kPGPMemoryMgrFlags_Clear);
	if( IsntNull( descriptor ) )
	{
		PGPOptionValue	value;
		
		descriptor->buffer 				= buffer;
		descriptor->maximumBufferSize 	= maximumBufferSize;
		descriptor->actualBufferSize	= actualBufferSize;
		
		value.asPtr = descriptor;
		
		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_OutputAllocatedBuffer,
							&value, sizeof( *descriptor ),
							AllocatedOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

/* Not yet supported */
	PGPOptionListRef
PGPOAppendOutput(
	PGPContextRef	context,
	PGPBoolean		appendOutput
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)appendOutput;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_AppendOutput,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPODiscardOutput(
	PGPContextRef	context,
	PGPBoolean		discardOutput
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)discardOutput;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_DiscardOutput,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}


#if 0

	PGPOptionListRef
PGPOAskUserForOutput(
	PGPContextRef	context,
	PGPBoolean		askUserForOutput
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)askUserForOutput;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_AskUserForOutput,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}
#endif



	PGPOptionListRef
PGPOEncryptToKey(
	PGPContextRef	context,
	PGPKeyRef		keyRef
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpKeyIsValid( keyRef ) );
	
	value.asKeyRef = keyRef;

	pgpIncKeyRefCount( keyRef );

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_EncryptToKey,
							&value, sizeof( keyRef ),
							SpecialOptionHandlerProc );
	
#if PGP_ENCRYPT_DISABLE
	pgpSetOptionListError( optionList, kPGPError_FeatureNotAvailable );
#endif

	return( optionList );
}

	PGPOptionListRef
PGPOEncryptToKeySet(
	PGPContextRef	context,
	PGPKeySetRef	keySetRef
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpKeySetIsValid( keySetRef ) );
	
	value.asKeySetRef = keySetRef;

	PGPIncKeySetRefCount( keySetRef );

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_EncryptToKeySet,
							&value, sizeof( keySetRef ),
							SpecialOptionHandlerProc );
	
#if PGP_ENCRYPT_DISABLE
	pgpSetOptionListError( optionList, kPGPError_FeatureNotAvailable );
#endif

	return( optionList );
}

	PGPOptionListRef
PGPOEncryptToUserID(
	PGPContextRef	context,
	PGPUserIDRef	userIDRef
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( PGPUserIDRefIsValid( userIDRef ) );
	
	value.asUserIDRef = userIDRef;

	pgpIncUserIDRefCount( userIDRef );

	optionList = pgpCreateStandardValueOptionList( context, 
							kPGPOptionType_EncryptToUserID,
							&value, sizeof( userIDRef ),
							SpecialOptionHandlerProc );
	
#if PGP_ENCRYPT_DISABLE
	pgpSetOptionListError( optionList, kPGPError_FeatureNotAvailable );
#endif

	return( optionList );
}

	PGPOptionListRef
PGPOSignWithKey(
	PGPContextRef		context,
	PGPKeyRef			keyRef,
	PGPOptionListRef	firstOption,
	...)
{
	PGPOptionListRef	optionList;
	
	pgpAssert( pgpContextIsValid( context ) );
	pgpAssert( pgpKeyIsValid( keyRef ) );
	
	if( pgpKeyIsValid( keyRef ) && pgpContextIsValid( context ) )
	{
		PGPOptionListRef	subOptions;
		PGPOptionValue		value;
		va_list				args;
		
		va_start( args, firstOption );
			subOptions = pgpBuildOptionListArgs( context,
				FALSE, firstOption, args );
		va_end( args );

		value.asKeyRef = keyRef;
	
		pgpIncKeyRefCount( keyRef );

		optionList = pgpCreateCustomValueOptionList(
								context,
								kPGPOptionType_SignWithKey,
								kPGPOptionFlag_Default, &value,
								sizeof( keyRef ), subOptions,
								SpecialOptionHandlerProc );
	}
	else
	{
		va_list				args;
		
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		optionList = kPGPBadParamsOptionListRef;
	}
	
#if PGP_SIGN_DISABLE
	pgpSetOptionListError( optionList, kPGPError_FeatureNotAvailable );
#endif

	return( optionList );
}

	PGPOptionListRef
PGPOConventionalEncrypt(
	PGPContextRef	context,
	PGPOptionListRef firstOption,
	...)
{
	PGPOptionListRef	optionList;
	PGPOptionListRef	subOptions;
	va_list				args;
	
	pgpAssert( pgpContextIsValid( context ) );

	if ( pgpContextIsValid( context ) )
	{
		va_start( args, firstOption );
			subOptions = pgpBuildOptionListArgs( context,
						FALSE, firstOption, args );
		va_end( args );

		optionList = pgpCreateCustomValueOptionList(
						context,  
						kPGPOptionType_ConventionalEncrypt,
						kPGPOptionFlag_Default, NULL,
						0, subOptions, NULL);
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		optionList = kPGPBadParamsOptionListRef;
	}
	
#if PGP_ENCRYPT_DISABLE
	pgpSetOptionListError( optionList, kPGPError_FeatureNotAvailable );
#endif

	return( optionList );
}

	PGPOptionListRef
PGPOPassphraseBuffer(
	PGPContextRef	context,
	const void *	passphrase,
	PGPSize			passphraseLength)
{
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( passphrase ) );
	
	return( pgpCreateSensitiveOptionList( context,
				kPGPOptionType_Passphrase, passphrase, passphraseLength ) );
}

	PGPOptionListRef
PGPOPassphrase(
	PGPContextRef	context,
	const char *	passphrase)
{
	PGPSize			passphraseLength;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( passphrase ) );

	/* Don't include trailing null, it is not used in the hash operations */
	passphraseLength = strlen( passphrase );
	
	return( pgpCreateSensitiveOptionList( context,
				kPGPOptionType_Passphrase, passphrase, passphraseLength ) );
}


	PGPOptionListRef
PGPOPasskeyBuffer(
	PGPContextRef	context,
	const void *	passkey,
	PGPSize			passkeyLength)
{
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( passkey ) );
	
	return( pgpCreateSensitiveOptionList( context,
				kPGPOptionType_Passkey, passkey, passkeyLength ) );
}


	PGPOptionListRef
PGPOSessionKey(
	PGPContextRef	context,
	const void *	sessionKey,
	PGPSize			sessionKeyLength)
{
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( sessionKey ) );
	
	return( pgpCreateSensitiveOptionList( context,
				kPGPOptionType_SessionKey, sessionKey, sessionKeyLength ) );
}


	PGPOptionListRef
PGPODetachedSig(
	PGPContextRef	context,
	PGPOptionListRef firstOption,
	...)
{
	PGPOptionListRef	optionList;
	PGPOptionListRef	subOptions;
	va_list				args;
	
	pgpAssert( pgpContextIsValid( context ) );

	if ( pgpContextIsValid( context )  )
	{
		va_start( args, firstOption );
			subOptions = pgpBuildOptionListArgs( context,
				FALSE, firstOption, args );
		va_end( args );

		optionList = pgpCreateCustomValueOptionList( context, 
								kPGPOptionType_DetachedSignature,
								kPGPOptionFlag_Default, NULL,
								0, subOptions, NULL);
	}
	else
	{
		va_start( args, firstOption );
		pgpFreeVarArgOptionList( firstOption, args);
		va_end( args );
		
		optionList = kPGPBadParamsOptionListRef;
	}
	
	return( optionList );
}

#if 0
/* Not supported */
	PGPOptionListRef
PGPOMailHeaders(
	PGPContextRef	context,
	PGPOptionListRef firstOption,
	...)
{
	PGPOptionListRef	optionList;
	PGPOptionListRef	subOptions;
	va_list				args;
	
	pgpAssert( pgpContextIsValid( context ) );

	va_start( args, firstOption );
		subOptions = pgpBuildOptionListArgs( context,
			FALSE, firstOption, args );
	va_end( args );

	optionList = pgpCreateCustomValueOptionList( context, 
							kPGPOptionType_MailHeaders,
							kPGPOptionFlag_Default, NULL,
							0, subOptions, NULL);
	
	return( optionList );
}
#endif

	PGPOptionListRef
PGPOCipherAlgorithm(
	PGPContextRef	context,
	PGPCipherAlgorithm	algorithm
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( algorithm == kPGPCipherAlgorithm_IDEA ||
		algorithm == kPGPCipherAlgorithm_3DES ||
		algorithm == kPGPCipherAlgorithm_CAST5);

	value.asCipherAlgorithm = algorithm;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_CipherAlgorithm,
							&value, sizeof( algorithm ), NULL);
	
	return( optionList );
}

	PGPOptionListRef
PGPOHashAlgorithm(
	PGPContextRef		context,
	PGPHashAlgorithm	algorithm
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
		
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( algorithm == kPGPHashAlgorithm_MD5 ||
		algorithm == kPGPHashAlgorithm_SHA ||
		algorithm == kPGPHashAlgorithm_RIPEMD160 );

	value.asHashAlgorithm = algorithm;
	
	optionList = pgpCreateStandardValueOptionList( context, 
						kPGPOptionType_HashAlgorithm,
						&value, sizeof( algorithm ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOFailBelowValidity(
	PGPContextRef	context,
	PGPValidity		minValidity)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asValidity = minValidity;
	
	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_FailBelowValidity,
						&value, sizeof( minValidity ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOWarnBelowValidity(
	PGPContextRef	context,
	PGPValidity		minValidity)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asValidity = minValidity;
	
	optionList = pgpCreateStandardValueOptionList( context, 
						kPGPOptionType_WarnBelowValidity,
						&value, sizeof( minValidity ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOEventHandler(
	PGPContextRef	context,
	PGPEventHandlerProcPtr	handler,
	PGPUserValue			userValue)
{
	PGPOptionListRef		optionList;
	PGPOEventHandlerDesc	*descriptor;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( handler ) );
	
	descriptor = (PGPOEventHandlerDesc *)
						pgpContextMemAlloc( context, sizeof(*descriptor), 0);
	if( IsntNull( descriptor ) )
	{
		PGPOptionValue	value;
		
		descriptor->handlerProc = handler;
		descriptor->userValue 	= userValue;
		
		value.asPtr = descriptor;
		
		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_EventHandler,
							&value, sizeof( *descriptor ),
							AllocatedOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}

	return( optionList );
}

	PGPOptionListRef
PGPOSendNullEvents(
	PGPContextRef	context,
	PGPTimeInterval approxInterval)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
		
	pgpValidateOptionContext( context );

	value.asInterval = approxInterval;
	
	optionList = pgpCreateStandardValueOptionList( context, 
						kPGPOptionType_SendNullEvents,
						&value, sizeof( approxInterval ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOArmorOutput(
	PGPContextRef	context,
	PGPBoolean		armorOutput
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)armorOutput;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_ArmorOutput,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPODataIsASCII(
	PGPContextRef	context,
	PGPBoolean		dataIsASCII
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)dataIsASCII;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_DataIsASCII,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOClearSign(
	PGPContextRef	context,
	PGPBoolean		clearSign
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)clearSign;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_ClearSign,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOForYourEyesOnly(
	PGPContextRef	context,
	PGPBoolean		forYourEyesOnly
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)forYourEyesOnly;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_ForYourEyesOnly,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOSendEventIfKeyFound(
	PGPContextRef	context,
	PGPBoolean		sendEventIfKeyFound
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)sendEventIfKeyFound;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_SendEventIfKeyFound,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOPassThroughIfUnrecognized(
	PGPContextRef	context,
	PGPBoolean		passThroughIfUnrecognized
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)passThroughIfUnrecognized;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_PassThroughIfUnrecognized,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOPassThroughClearSigned(
	PGPContextRef	context,
	PGPBoolean		passThroughClearSigned
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)passThroughClearSigned;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_PassThroughClearSigned,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOPassThroughKeys(
	PGPContextRef	context,
	PGPBoolean		passThroughKeys
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)passThroughKeys;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_PassThroughKeys,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPORecursivelyDecode(
	PGPContextRef	context,
	PGPBoolean		recurse
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)recurse;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_RecursivelyDecode,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOAskUserForEntropy(
	PGPContextRef	context,
	PGPBoolean		askUserForEntropy
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)askUserForEntropy;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_AskUserForEntropy,
							&value, sizeof( PGPUInt32 ), NULL);

	
	return( optionList );
}

	PGPOptionListRef
PGPORawPGPInput(
	PGPContextRef	context,
	PGPBoolean		rawPGPInput
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)rawPGPInput;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_RawPGPInput,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOCompression(
	PGPContextRef	context,
	PGPBoolean		compression
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)compression;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_Compression,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOKeySetRef(
	PGPContextRef	context,
	PGPKeySetRef 	keySetRef)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpKeySetIsValid( keySetRef ) );
	
	value.asKeySetRef = keySetRef;
	
	PGPIncKeySetRefCount( keySetRef );

	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_KeySetRef,
						&value, sizeof( keySetRef ),
						SpecialOptionHandlerProc );
	
	return( optionList );
}

	PGPOptionListRef
PGPOExportKeySet(
	PGPContextRef	context,
	PGPKeySetRef 	keySetRef)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpKeySetIsValid( keySetRef ) );
	
	value.asKeySetRef = keySetRef;
	
	PGPIncKeySetRefCount( keySetRef );

	optionList = pgpCreateStandardValueOptionList( context,  
						kPGPOptionType_ExportKeySet,
						&value, sizeof( keySetRef ),
						SpecialOptionHandlerProc );
	
	return( optionList );
}

	PGPOptionListRef
PGPOExportKey(
	PGPContextRef	context,
	PGPKeyRef	 	keyRef)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpKeyIsValid( keyRef ) );
	
	value.asKeyRef = keyRef;

	pgpIncKeyRefCount( keyRef );

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_ExportKey,
							&value, sizeof( keyRef ),
							SpecialOptionHandlerProc );
	
	return( optionList );
}

	PGPOptionListRef
PGPOExportUserID(
	PGPContextRef	context,
	PGPUserIDRef 	useridRef)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpUserIDIsValid( useridRef ) );
	
	value.asUserIDRef = useridRef;

	pgpIncUserIDRefCount( useridRef );

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_ExportUserID,
							&value, sizeof( useridRef ),
							SpecialOptionHandlerProc );
	
	return( optionList );
}

	PGPOptionListRef
PGPOExportSig(
	PGPContextRef	context,
	PGPSigRef	 	sigRef)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpSigIsValid( sigRef ) );
	
	value.asSigRef = sigRef;

	pgpIncSigRefCount( sigRef );

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_ExportSig,
							&value, sizeof( sigRef ),
							SpecialOptionHandlerProc );
	
	return( optionList );
}


	PGPOptionListRef
PGPOKeyGenParams(
	PGPContextRef			context,
	PGPPublicKeyAlgorithm	pubKeyAlg,
	PGPUInt32				bits)
{
	PGPOptionListRef		optionList;
	PGPOKeyGenParamsDesc	*descriptor;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pubKeyAlg >= kPGPPublicKeyAlgorithm_First &&
		pubKeyAlg <= kPGPPublicKeyAlgorithm_Last );
	pgpValidateOptionParam( bits >= 512 );

	descriptor = (PGPOKeyGenParamsDesc *)
						pgpContextMemAlloc( context, sizeof(*descriptor), 0);
	if( IsntNull( descriptor ) )
	{
		PGPOptionValue	value;

		descriptor->pkalg	= pubKeyAlg;
		descriptor->bits 	= bits;

		value.asPtr = descriptor;

		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyGenParams,
							&value, sizeof( *descriptor ),
							AllocatedOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

	PGPOptionListRef
PGPOKeyGenName(
	PGPContextRef	context,
	const void	*	name,
	PGPSize			nameLength)
{
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( name ) );
	pgpValidateOptionParam( nameLength >= 1 );
	pgpValidateOptionParam( nameLength < 256 );
	
	return( pgpCreateBufferOptionList( context,
				kPGPOptionType_KeyGenName, name, nameLength ) );
}

	PGPOptionListRef
PGPOExpiration(
	PGPContextRef	context,
	PGPUInt32		expirationDays)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;

	pgpValidateOptionContext( context );

	value.asUInt = expirationDays;

	optionList = pgpCreateStandardValueOptionList( context, 
							kPGPOptionType_Expiration,
							&value, sizeof( expirationDays ), NULL);
	
	return( optionList );
}

	PGPOptionListRef
PGPOCreationDate(
	PGPContextRef	context,
	PGPTime			creationDate)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;

	pgpValidateOptionContext( context );

	value.asTime = creationDate;

	optionList = pgpCreateStandardValueOptionList( context, 
							kPGPOptionType_CreationDate,
							&value, sizeof( creationDate ), NULL);
	
	return( optionList );
}

	PGPOptionListRef
PGPOAdditionalRecipientRequestKeySet(
	PGPContextRef	context,
	PGPKeySetRef	arKeySetRef,
	PGPByte			arkclass
	)
{
	PGPOptionListRef	optionList;
	PGPOAdditionalRecipientRequestKeySetDesc	*descriptor;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpKeySetIsValid( arKeySetRef ) );

	descriptor = (PGPOAdditionalRecipientRequestKeySetDesc *)
						pgpContextMemAlloc( context, sizeof(*descriptor), 0);
	if( IsntNull( descriptor ) )
	{
		PGPOptionValue	value;

		descriptor->arKeySetRef	= arKeySetRef;
		descriptor->arkclass 	= arkclass;

		value.asPtr = descriptor;

		PGPIncKeySetRefCount( arKeySetRef );

		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_AdditionalRecipientRequestKeySet,
							&value, sizeof( *descriptor ),
							AllocatedOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

	PGPOptionListRef
PGPORevocationKeySet(
	PGPContextRef	context,
	PGPKeySetRef	raKeySetRef
	)
{
	PGPOptionListRef	optionList;
	PGPOAdditionalRecipientRequestKeySetDesc	*descriptor;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpKeySetIsValid( raKeySetRef ) );

	/* Use ADK descriptor as we have similar data structure */
	descriptor = (PGPOAdditionalRecipientRequestKeySetDesc *)
						pgpContextMemAlloc( context, sizeof(*descriptor), 0);
	if( IsntNull( descriptor ) )
	{
		PGPOptionValue	value;

		descriptor->arKeySetRef	= raKeySetRef;
		descriptor->arkclass 	= 0x80;	/* hard code value for revocation */

		value.asPtr = descriptor;

		PGPIncKeySetRefCount( raKeySetRef );

		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_RevocationKeySet,
							&value, sizeof( *descriptor ),
							AllocatedOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

	PGPOptionListRef
PGPOKeyGenMasterKey(
	PGPContextRef	context,
	PGPKeyRef		masterKeyRef
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpKeyIsValid( masterKeyRef ) );
	
	value.asKeyRef = masterKeyRef;

	pgpIncKeyRefCount( masterKeyRef );

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyGenMasterKey,
							&value, sizeof( masterKeyRef ),
							SpecialOptionHandlerProc );
	
	return( optionList );
}

	PGPOptionListRef
PGPOPreferredAlgorithms(
	PGPContextRef				context,
	PGPCipherAlgorithm const *	prefAlg,
	PGPUInt32					numAlgs)
{
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( prefAlg ) );
	pgpValidateOptionParam( numAlgs != 0 );
	pgpAssert( numAlgs <= kPGPCipherAlgorithm_Last );
	
	return( pgpCreateBufferOptionList( context,
					kPGPOptionType_PreferredAlgorithms,
					prefAlg, numAlgs * sizeof( prefAlg[ 0 ] )  ) );
}

	PGPOptionListRef
PGPOKeyGenFast(
	PGPContextRef	context,
	PGPBoolean		fastGen
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)fastGen;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyGenFast,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOKeyGenUseExistingEntropy(
	PGPContextRef	context,
	PGPBoolean		useExistingEntropy
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)useExistingEntropy;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyGenUseExistingEntropy,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOExportable(
	PGPContextRef	context,
	PGPBoolean		exportable
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)exportable;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_Exportable,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOExportPrivateKeys(
	PGPContextRef	context,
	PGPBoolean		exportKeys
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)exportKeys;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_ExportPrivateKeys,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOExportPrivateSubkeys(
	PGPContextRef	context,
	PGPBoolean		exportSubkeys
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)exportSubkeys;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_ExportPrivateSubkeys,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOOmitMIMEVersion(
	PGPContextRef	context,
	PGPBoolean		omitVersion
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)omitVersion;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_OmitMIMEVersion,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOX509Encoding(
	PGPContextRef	context,
	PGPBoolean		x509Encoding
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32)x509Encoding;
	
	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_X509Encoding,
							&value, sizeof( PGPUInt32 ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOSigTrust(
	PGPContextRef	context,
	PGPUInt32		trustLevel,
	PGPUInt32		trustValue)
{
	PGPOptionListRef			optionList;
	PGPOCertificateTrustDesc	*descriptor;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( trustValue >= kPGPKeyTrust_Unknown &&
		trustValue <= kPGPKeyTrust_Ultimate );

	descriptor = (PGPOCertificateTrustDesc *)
						pgpContextMemAlloc( context, sizeof(*descriptor), 0);
	if( IsntNull( descriptor ) )
	{
		PGPOptionValue	value;

		descriptor->trustLevel	= trustLevel;
		descriptor->trustValue 	= trustValue;

		value.asPtr = descriptor;

		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_CertificateTrust,
							&value, sizeof( *descriptor ),
							AllocatedOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

	PGPOptionListRef
PGPOCommentString(
	PGPContextRef	context,
	char const		*comment
	)
{
	PGPSize			 commentLength;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( comment ) );
	
	commentLength = strlen( comment ) + 1;

	return( pgpCreateBufferOptionList( context,
								kPGPOptionType_CommentString,
								comment, commentLength ) );
}

	PGPOptionListRef
PGPOVersionString(
	PGPContextRef	context,
	char const		*version
	)
{
	PGPSize			 versionLength;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( version ) );
	
	versionLength = strlen( version ) + 1;

	return( pgpCreateBufferOptionList( context,
								kPGPOptionType_VersionString,
								version, versionLength ) );
}

	PGPOptionListRef
PGPOFileNameString(
	PGPContextRef	context,
	char const		*fileName
	)
{
	PGPSize			 fileNameLength;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( fileName ) );
	
	fileNameLength = strlen( fileName ) + 1;

	return( pgpCreateBufferOptionList( context,
								kPGPOptionType_InputFileName,
								fileName, fileNameLength ) );
}

	PGPOptionListRef
PGPOSigRegularExpression(
	PGPContextRef	context,
	char const		*regexp
	)
{
	PGPSize			 regexpLength;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( regexp ) );
	
	regexpLength = strlen( regexp ) + 1;

	return( pgpCreateBufferOptionList( context,
								kPGPOptionType_CertificateRegularExpression,
								regexp, regexpLength ) );
}

	PGPOptionListRef
PGPOLocalEncoding(
	PGPContextRef				context,
	PGPLocalEncodingFlags		localEncode
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	/* Ensure incompatible options not selected */
	if( (localEncode & kPGPLocalEncoding_Force) &&
		(localEncode & kPGPLocalEncoding_Auto) )
		return kPGPBadParamsOptionListRef;

	value.asUInt = localEncode;
	
	optionList = pgpCreateStandardValueOptionList( context,  
					kPGPOptionType_LocalEncoding,
					&value, sizeof( PGPLocalEncodingFlags ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOOutputLineEndType(
	PGPContextRef		context,
	PGPLineEndType		lineEnd
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( lineEnd == kPGPLineEnd_LF ||
		lineEnd == kPGPLineEnd_CR || lineEnd == kPGPLineEnd_CRLF );

	value.asUInt = (PGPUInt32) lineEnd;
	
	optionList = pgpCreateStandardValueOptionList( context,
					kPGPOptionType_OutputLineEndType,
					&value, sizeof( PGPLocalEncodingFlags ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOExportFormat(
	PGPContextRef		context,
	PGPExportFormat		exportFormat
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32) exportFormat;
	
	optionList = pgpCreateStandardValueOptionList( context,
					kPGPOptionType_ExportFormat,
					&value, sizeof( PGPLocalEncodingFlags ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOInputFormat(
	PGPContextRef		context,
	PGPInputFormat		inputFormat
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32) inputFormat;
	
	optionList = pgpCreateStandardValueOptionList( context,
					kPGPOptionType_InputFormat,
					&value, sizeof( PGPLocalEncodingFlags ), NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOOutputFormat(
	PGPContextRef		context,
	PGPOutputFormat		outputFormat
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asUInt = (PGPUInt32) outputFormat;
	
	optionList = pgpCreateStandardValueOptionList( context,
					kPGPOptionType_OutputFormat,
					&value, sizeof( PGPLocalEncodingFlags ), NULL);

	return( optionList );
}

/*
 * We don't use copy semantics on this one because it may have deep
 * structure that we don't know about.
 */
	PGPOptionListRef
PGPOAttributeValue(
	PGPContextRef		context,
	PGPAttributeValue	*attributeValue,
	PGPUInt32			attributeValueCount
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );

	value.asPtr = attributeValue;
	
	optionList = pgpCreateStandardValueOptionList( context,
					kPGPOptionType_AttributeValue,
					&value, attributeValueCount * sizeof(PGPAttributeValue),
					NULL);

	return( optionList );
}

	PGPOptionListRef
PGPOImportKeysTo(
	PGPContextRef	context,
	PGPKeySetRef	keySetRef
	)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( pgpKeySetIsValid( keySetRef ) );
		
	value.asKeySetRef = keySetRef;

	PGPIncKeySetRefCount( keySetRef );

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_ImportKeysTo,
							&value, sizeof( keySetRef ),
							SpecialOptionHandlerProc );
	
	return( optionList );
}

	PGPOptionListRef
PGPOPGPMIMEEncoding(
	PGPContextRef	context,
	PGPBoolean		mimeEncoding,
	PGPSize *		mimeBodyOffset,
	char *			mimeSeparator
	)
{
	PGPOptionListRef			optionList;
	PGPOPGPMIMEEncodingDesc		*descriptor;

	pgpValidateOptionContext( context );
	if ( mimeEncoding )
	{
		if ( IsntNull( mimeBodyOffset ) )
			*mimeBodyOffset	= 0;
		if ( IsntNull( mimeSeparator ) )
			*mimeSeparator	= '\0';
			
		pgpValidateOptionParam( IsntNull( mimeBodyOffset ) );
		pgpValidateOptionParam( IsntNull( mimeSeparator ) );
		pgpDebugWhackMemory( mimeSeparator, kPGPMimeSeparatorSize );
	}
	else
	{
		/* mime off--ignore other params */
	}

	descriptor = (PGPOPGPMIMEEncodingDesc *)
						pgpContextMemAlloc( context, sizeof(*descriptor), 0 );
	if( IsntNull( descriptor ) )
	{
		PGPOptionValue	value;

		descriptor->mimeEncoding	= (PGPUInt32) mimeEncoding;
		descriptor->mimeBodyOffset 	= mimeEncoding ? mimeBodyOffset : NULL;
		descriptor->mimeSeparator 	= mimeEncoding ? mimeSeparator : NULL;

		value.asPtr = descriptor;

		optionList = pgpCreateStandardValueOptionList( context,
							kPGPOptionType_PGPMIMEEncoding,
							&value, sizeof( *descriptor ),
							AllocatedOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
