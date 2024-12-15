/*____________________________________________________________________________
	pgpEvent.c
	Handle application event callbacks
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpEvent.c,v 1.33 1999/05/07 23:47:45 hal Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"	/* or pgpConfig.h in the CDK */

#include <string.h>

/* Public headers */
#include "pgpPubTypes.h"
#include "pgpContext.h"
#include "pgpEncode.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpMem.h"

/* Private headers */
#include "pgpEventPriv.h"
#include "pgpEncodePriv.h"

/************************** User event callback ****************************/

	PGPError
pgpEventNull(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPFileOffset		soFar,
	PGPFileOffset		total
	)
{	
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job							= &job;
	event.type							= kPGPEvent_NullEvent;
	event.data.nullData.bytesWritten	= soFar;
	event.data.nullData.bytesTotal		= total;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}


	PGPError
pgpEventInitial(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job							= &job;
	event.type							= kPGPEvent_InitialEvent;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}


	PGPError
pgpEventFinal(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job							= &job;
	event.type							= kPGPEvent_FinalEvent;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}


	PGPError
pgpEventError(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPError			error,
	void			   *errorArg
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job							= &job;
	event.type							= kPGPEvent_ErrorEvent;
	event.data.errorData.error			= error;
	event.data.errorData.errorArg		= errorArg;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventWarning(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPError			warning,
	void			   *warningArg
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job							= &job;
	event.type							= kPGPEvent_WarningEvent;
	event.data.warningData.warning		= warning;
	event.data.warningData.warningArg	= warningArg;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventEntropy(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPUInt32			entropyBitsNeeded
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_EntropyEvent;
	event.data.entropyData.entropyBitsNeeded	= entropyBitsNeeded;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventPassphrase(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPBoolean			fConventional,
	PGPKeySetRef		keyset
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_PassphraseEvent;
	event.data.passphraseData.fConventional		= fConventional;
	event.data.passphraseData.keyset			= keyset;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventAnalyze(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPAnalyzeType		type
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_AnalyzeEvent;
	event.data.analyzeData.sectionType			= type;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventRecipients(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPKeySetRef		recipientSet,
	PGPKeyID			*keyIDArray,
	PGPUInt32			passphraseCount,
	PGPUInt32			keyCount
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_RecipientsEvent;
	event.data.recipientsData.recipientSet		= recipientSet;
	event.data.recipientsData.keyIDArray		= keyIDArray;
	event.data.recipientsData.conventionalPassphraseCount
												= passphraseCount;
	event.data.recipientsData.keyCount			= keyCount;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventKeyFound(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPKeySetRef		keySet
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_KeyFoundEvent;
	event.data.keyFoundData.keySet				= keySet;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventOutput(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPUInt32			messageType,
	char const		   *suggestedName,
	PGPBoolean			FYEO
	)
{
	PGPJob		job;
	PGPEvent	event;
	PGPError	err;
	char		*canonicalName = NULL;
	
	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	if( IsntNull( suggestedName ) )
	{
		/* Convert all illegal characters to underscores */
		
		canonicalName = (char *) PGPNewData( PGPGetContextMemoryMgr( context ),
								strlen( suggestedName ) + 1, 0 );
		if( IsntNull( canonicalName ) )
		{
			char	*cur = canonicalName;
			
			strcpy( canonicalName, suggestedName );
			
		#if PGP_MACINTOSH
			/* Macs are unhappy with filenames starting with '.' */
			if( *cur == '.' )
				*cur = '_';
		#endif
		
			while( *cur != 0 )
			{
			#if PGP_MACINTOSH
				if( *cur < ' ' || *cur == ':' )
			#elif PGP_WIN32 || PGP_UNIX
				if ((*cur < ' ')  || (*cur == ':') || (*cur == '/') || 
					(*cur == '?') || (*cur == '*') || (*cur == '"') || 
					(*cur == '>') || (*cur == '\\') || (*cur == '|') || 
					(*cur == 0x5C) )	/* \ */
			#else
				#error Unknown platform
			#endif
				{
					*cur = '_';
				}
				
				++cur;
			}
		}
	}
	
	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_OutputEvent;
	event.data.outputData.messageType			= messageType;
	event.data.outputData.suggestedName			= canonicalName;
	event.data.outputData.forYourEyesOnly		= FYEO;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	
	if( IsntNull( canonicalName ) )
		PGPFreeData( canonicalName );
		
	return err;
}

	PGPError
pgpEventSignature(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPKeyID const *	signingKeyID,
	PGPKeyRef			signingKey,
	PGPBoolean			checked,
	PGPBoolean			verified,
	PGPBoolean			keyDisabled,
	PGPBoolean			keyRevoked,
	PGPBoolean			keyExpired,
	PGPBoolean			keyValidityThreshold,
	PGPValidity			keyValidity,
	PGPTime				creationTime
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_SignatureEvent;
	event.data.signatureData.signingKeyID		= *signingKeyID;
	event.data.signatureData.signingKey			= signingKey;
	event.data.signatureData.checked			= checked;
	event.data.signatureData.verified			= verified;
	event.data.signatureData.keyDisabled		= keyDisabled;
	event.data.signatureData.keyRevoked			= keyRevoked;
	event.data.signatureData.keyExpired			= keyExpired;
	event.data.signatureData.keyMeetsValidityThreshold = keyValidityThreshold;
	event.data.signatureData.keyValidity		= keyValidity;
	event.data.signatureData.creationTime		= creationTime;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}


	PGPError
pgpEventDecryption(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPCipherAlgorithm	cipheralg,
	PGPByte				*key,
	PGPSize				keylen
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_DecryptionEvent;
	event.data.decryptionData.cipherAlgorithm	= cipheralg;
	event.data.decryptionData.sessionKey		= key;
	event.data.decryptionData.sessionKeyLength	= keylen;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}


	PGPError
pgpEventEncryption(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPCipherAlgorithm	cipheralg,
	PGPByte				*key,
	PGPSize				keylen
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_EncryptionEvent;
	event.data.encryptionData.cipherAlgorithm	= cipheralg;
	event.data.encryptionData.sessionKey		= key;
	event.data.encryptionData.sessionKeyLength	= keylen;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}


	PGPError
pgpEventBeginLex(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPUInt32			sectionNumber,
	PGPSize				sectionOffset
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job								= &job;
	event.type								= kPGPEvent_BeginLexEvent;
	event.data.beginLexData.sectionNumber	= sectionNumber;
	event.data.beginLexData.sectionOffset	= sectionOffset;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventEndLex(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPUInt32			sectionNumber
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job								= &job;
	event.type								= kPGPEvent_EndLexEvent;
	event.data.endLexData.sectionNumber		= sectionNumber;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventDetachedSignature(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job							= &job;
	event.type							= kPGPEvent_DetachedSignatureEvent;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventKeyGen(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPUInt32			state
	)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_KeyGenEvent;
	event.data.keyGenData.state					= state;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}

	PGPError
pgpEventKeyServer(
	PGPContextRef		context,
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPKeyServerRef		keyServerRef,
	PGPUInt32			state)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_KeyServerEvent;
	event.data.keyServerData.state				= state;
	event.data.keyServerData.keyServerRef		= keyServerRef;

	err = func( context, &event, userValue );

	return err;
}

	PGPError
pgpEventKeyServerSign(
	PGPContextRef		context,
	PGPOptionListRef   *newOptionList,	/* Output parameter */
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPKeyServerRef		keyServerRef)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	*newOptionList = NULL;
	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_KeyServerSignEvent;
	event.data.keyServerSignData.keyServerRef	= keyServerRef;

	err = func( context, &event, userValue );

	*newOptionList = job.newOptionList;
	return err;
}



	PGPError
pgpEventKeyServerTLS(
	PGPContextRef		context,
	PGPEventHandlerProcPtr func,
	PGPUserValue		userValue,
	PGPKeyServerRef		keyServerRef,
	PGPUInt32			state,
	PGPtlsSessionRef	tlsSession)
{
	PGPJob				job;
	PGPEvent			event;
	PGPError			err;

	if( IsNull( func ) )
		return kPGPError_NoErr;

	job.context			= context;
	job.newOptionList	= NULL;

	pgpClearMemory( &event, sizeof( event ) );
	event.job									= &job;
	event.type									= kPGPEvent_KeyServerTLSEvent;
	event.data.keyServerTLSData.state			= state;
	event.data.keyServerTLSData.keyServerRef	= keyServerRef;
	event.data.keyServerTLSData.tlsSession		= tlsSession;

	err = func( context, &event, userValue );

	return err;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
