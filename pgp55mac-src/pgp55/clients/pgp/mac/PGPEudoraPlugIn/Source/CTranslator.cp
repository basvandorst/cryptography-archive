/*____________________________________________________________________________
	CTranslator.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CTranslator.cp,v 1.42.4.2.4.1 1997/12/05 22:13:29 mhw Exp $
____________________________________________________________________________*/
#include <string.h>
#include <Icons.h>
#include "MacStrings.h"
#include "MacMemory.h"
#include "MacIcons.h"
#include "MacFiles.h"
#include "MacEvents.h"
#include "WarningAlert.h"
#include "MacErrors.h"

#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUserInterface.h"

#include "CTranslator.h"
#include "PassphraseCache.h"
#include "PGPUserInterface.h"
#include "TranslatorIDs.h"
#include "MyMIMEUtils.h"
#include "TranslatorStrings.h"
#include "TranslatorUtils.h"
#include "TranslatorPrefs.h"
#include "pgpVersionHeader.h"
#include "MacMenus.h"
#include "CSecureMemory.h"
#include "pgpSharedEncryptDecrypt.h"


static void		AddCommandKeys( void );

typedef struct TranslatorInfo
{
	ConstStringPtr	description;
	SInt32			type;
	SInt32			id;	// aka subtype
	UInt32			flags;
	short			iconSuiteID;	// 0 if none
} TranslatorInfo;

const ResID		kDoesntHaveIconSuite	= 0;

static TranslatorInfo	sTranslatorInfo[] =
{
	{
		"\pPGP Plugin Encrypt",
		EMST_PREPROCESS,
		kEncryptTranslatorID,
		( EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE |
			EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		kEncryptIconSuiteID
	},
	
	{
		"\pPGP Plugin Sign",
		EMST_SIGNATURE,
		kSignTranslatorID,
		( EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE |
			EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		kSignIconSuiteID
	},
	
	
	{
		"\pPGP Plugin Encrypt & Sign",
		EMST_COALESCED,
		kEncryptAndSignTranslatorID,
		( EMSF_Q4_TRANSMISSION | EMSF_WHOLE_MESSAGE |
			EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		kDoesntHaveIconSuite
	},
	
	{
		"\pPGP Plugin Verify",
		EMST_SIGNATURE,
		kVerifyTranslatorID,
		( EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY | EMSF_REQUIRES_MIME |
			EMSF_GENERATES_MIME ),
		kSignIconSuiteID
	},
	
	{
		"\pPGP Plugin Decrypt",
		EMST_PREPROCESS,
		kDecryptTranslatorID,
		( EMSF_ON_ARRIVAL | EMSF_ON_DISPLAY | EMSF_WHOLE_MESSAGE |
			EMSF_REQUIRES_MIME | EMSF_GENERATES_MIME ),
		kEncryptIconSuiteID
	},
	
	{
		"\pPGP Decrypt/Verify",
		EMST_PREPROCESS,
		kManualDecryptVerifyTranslatorID,
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		kDoesntHaveIconSuite
	},
	
	{
		"\pPGP Add Keys",
		EMST_PREPROCESS,
		kManualAddKeyTranslatorID,
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		kDoesntHaveIconSuite
	},
	
	{
		"\pPGP Encrypt/Sign",
		EMST_PREPROCESS,
		kManualEncryptSignTranslatorID,
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		kDoesntHaveIconSuite
	},
	
	{
		"\pPGP Sign",
		EMST_PREPROCESS,
		kManualSignTranslatorID,
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		kDoesntHaveIconSuite
	},
	
	{
		"\pPGP Encrypt",
		EMST_PREPROCESS,
		kManualEncryptTranslatorID,
		( EMSF_ON_REQUEST | EMSF_WHOLE_MESSAGE  ),
		kDoesntHaveIconSuite
	},
};


	

CTranslator::CTranslator(
	PGPContextRef	context,
	emsProgress		progressHook )
{
	mContext		= context;
	
	mKeySet			= kPGPInvalidRef;
	
	/* may be NULL */
	mEudoraProgress	= progressHook;
}


CTranslator::~CTranslator( )
{
	if ( IsntNull( mKeySet ) )
	{
		PGPFreeKeySet( mKeySet );
		mKeySet	= NULL;
	}
}



	PluginError
CTranslator::GetInfo(
	TranslatorID	id,
	long *			typePtr,
	long *			subtypePtr,
	ulong *			flagsPtr,
	StringHandle *	transDesc,
	Handle *		iconHandlePtr)
{
	CComboError				err;
	const TranslatorInfo *	info	= nil;
	
	if ( id == kManualAddKeyTranslatorID )
	{
		AddCommandKeys();
	}
	
	// find info for the specified translator
	for(	ulong index =  kFirstTranslatorID;
			index < kFirstTranslatorID + kPGPNumTranslators;
			++ index )
	{
		info	= &sTranslatorInfo[ index - 1 ];
		if ( info->id == id )
			break;
		info	= nil;
	}
	pgpAssert( IsntNull( info ) );
	
	if ( IsntNull( typePtr ) )
		*typePtr	= info->type;
		
	if ( IsntNull( subtypePtr ) )
		*subtypePtr	= info->id;
		
	if ( IsntNull( flagsPtr ) )
		*flagsPtr	= info->flags;
		
	if ( IsntNull( iconHandlePtr ) )
		*iconHandlePtr	= nil;
	
	// The description
	if( IsntNull( transDesc ) )
	{
		*transDesc = (StringHandle) NewHandle( StrSize( info->description ) );
		MacLeaks_ForgetHandle( *transDesc,kMacLeaks_DisposeHandleDeallocType);
		
		if ( IsntNull( *transDesc ) )
		{
			CopyPString( info->description, **transDesc);
		}
		else
		{
			err.err	= memFullErr;
		}
	}
	
	// The icon
	if( IsntNull( iconHandlePtr ) &&
			info->iconSuiteID != kDoesntHaveIconSuite )
	{
		Handle	theSuiteH = nil;
	
		MacLeaks_Suspend();
		
		err.err = GetIconSuite( &theSuiteH, info->iconSuiteID,
					svAllAvailableData);
		if( err.IsntError() && IsntNull( theSuiteH ) )
		{
			err.err = DuplicateIconSuite(theSuiteH, iconHandlePtr);
			DisposeIconSuite( theSuiteH, true);
		}
		
		MacLeaks_Resume();
	}
	
	return( CComboErrorToEudoraError( err ) );
}



	PGPError
CTranslator::LoadDefaultKeySet( PGPBoolean	writeable )
{
	PGPError				err	= kPGPError_NoErr;
	PGPKeyRingOpenFlags		openFlags	= 0;
	
	if ( writeable )
		openFlags	|= kPGPKeyRingOpenFlags_Mutable;
		
	if ( IsNull( mKeySet ) )
	{
		err	= PGPOpenDefaultKeyRings( mContext, openFlags, &mKeySet );
	}
	
	return( err );
}

  
/*____________________________________________________________________________
	Caller must call PGPFreeKey() on *signingKeyOut
____________________________________________________________________________*/
	CComboError
CTranslator::GetSigningPassphrase(
	char *			passphraseOut,
	PGPKeyRef *		signingKeyOut )
{
	char		msg[ 256 ];
	CComboError	err;
	
	if ( IsNull( signingKeyOut ) )
	{
		err.pgpErr	= kPGPError_BadParams;
		return( err );
	}
	*signingKeyOut	= kPGPInvalidRef;
	
	if ( IsNull( mKeySet ) )
		err.pgpErr	= LoadDefaultKeySet( FALSE );
	if ( err.IsError() )
		return( err );
	
	if ( gSigningPassphraseCache->GetPassphrase( mKeySet,
			passphraseOut, signingKeyOut ) )
	{
		/* already in cache */
		pgpAssert( ! err.IsError() );
	}
	else
	{
		GetIndCString( msg, kTranslatorErrorStringsResID,
			kPleaseEnterSigningPassphraseStrIndex );
			
		PGPGetPassphraseSettings		settings	= 0;
		const PGPGetPassphraseOptions	options		= 
			kPGPGetPassphraseOptionsHideFileOptions;
		
		LoadDefaultKeySet( FALSE );
		
		PGPGetPassphraseSettings	userSettings;
		err.pgpErr	= PGPGetSigningPassphraseDialog(
			mContext,
			mKeySet,
			msg,
			options,
			settings, 
			NULL,
			passphraseOut,
			&userSettings,
			signingKeyOut );
	}
	
	return( err );
}



	CComboError
CTranslator::GetDecryptionPassphrase(
	PGPKeySetRef		privateKeySet,
	PGPKeySetRef		allKeys,
	PGPUInt32			numMissing,
	char *				passphraseOut )
{
	CComboError		err;
	char			msg[ 256 ];
	Boolean			haveCachedPassphrase	= false;
	Boolean			havePassphrase			= false;
	PGPKeyRef		firstKey	= kPGPInvalidRef;
	
	pgpAssertAddrValid( passphraseOut, char );
	
	GetIndCString( msg, kTranslatorErrorStringsResID,
			kPleaseEnterDecryptionPassphraseStrIndex );
	
	pgpAssertAddrValid( gEncryptionPassphraseCache, CPassphraseCache );
	haveCachedPassphrase =
				gEncryptionPassphraseCache->GetPassphrase( passphraseOut );
	
	/* make sure that that the passphrase works for one of the keys */
	havePassphrase	= FALSE;
		
	PGPKeyListRef	keyListRef;
	err.pgpErr = PGPOrderKeySet( privateKeySet, kPGPAnyOrdering, &keyListRef );
	if( err.IsntError() )
	{
		PGPKeyIterRef	iter;
		PGPKeyRef		key			= kPGPInvalidRef;
		
		err.pgpErr = PGPNewKeyIter( keyListRef, &iter );
		if( err.IsntError() )
		{
			while( IsntPGPError( err.pgpErr = PGPKeyIterNext( iter, &key ) ) )
			{
				if ( ! PGPRefIsValid( firstKey ) )
					firstKey	= key;
					
				if ( ! haveCachedPassphrase )
					break;
					
				if ( PGPPassphraseIsValid( key, passphraseOut ) )
				{
					havePassphrase	= TRUE;
					break;
				}
			}
			if ( err.pgpErr == kPGPError_EndOfIteration )
				err.pgpErr	= kPGPError_NoErr;
	
			PGPFreeKeyIter( iter );
		}
		
		PGPFreeKeyList( keyListRef );
	}
	
	if ( ! havePassphrase )
	{
		err.pgpErr	= PGPGetDecryptionPassphraseDialog( mContext, msg,
						allKeys, numMissing,
						firstKey, passphraseOut, &firstKey );
	}
	
	return( err );
}




	PluginError
CTranslator::CanTranslateMIMEType(
	long					transContext,
	ConstemsMIMETypeHandle	mimeType,
	ConstStr255Param		type,
	ConstStr255Param		subType,
	ConstStr255Param		protocol )
{
	PluginError	err	= EMSR_CANT_TRANS;
	
	if ( IsntNull( mimeType ) )
	{
		Boolean		haveValidContext;
		const long	kValidContextMask	= EMSF_ON_DISPLAY | EMSF_ON_ARRIVAL;
		
		haveValidContext	= ( transContext & kValidContextMask ) != 0;
		
		if ( haveValidContext )
		{
			Boolean		haveMatch;
			
			haveMatch	= MatchMIMEType( mimeType, type, subType );
			
			if ( haveMatch && IsntNull( protocol ) )
			{
				Str255			paramValue;
				
				haveMatch	= false;
				if ( GetMIMEParameter( mimeType, "\pprotocol", paramValue ) )
				{
					haveMatch	= PStringsAreEqual( paramValue, protocol );
				}
				
				pgpAssertMsg( haveMatch,
					"CTranslator::CanTranslateMIMEType: missing param" );
			}
			
			if ( haveMatch )
			{
	 		    if( transContext == EMSF_ON_ARRIVAL )
	 		    {
		   	        err = EMSR_NOT_NOW;
		   	    }
		   	    else
		   	    {
		   		    err = EMSR_NOW;
		   		}
			}
		}
	}
	
	return( err );
}
					
					
	PluginError
CTranslator::CanTranslate(
	long			transContext,
	emsMIMEtype **	inMimeTypeHandle,
	const FSSpec *	inFSp,
	StringHandle**	addressesHandle,
	long *			aprox_len,
	StringHandle *	errorMessage,
	long *			result_code)
{
	PluginError	err	= EMSR_NOW;
	Str255			paramValue;
	
	(void)inFSp;
	(void)addressesHandle;
	(void)aprox_len;
	(void)result_code;
	(void)transContext;
	pgpDebugMsgIf( (transContext & EMSF_Q4_COMPLETION ),"EMSF_Q4_COMPLETION");
	
	InitPtrToNil( errorMessage );
	
	if ( GetMIMEParameter( inMimeTypeHandle,
			kPGPAlreadyProcessedParamName, paramValue ) )
	{
		// it has already been translated
		err	= EMSR_CANT_TRANS;
	}

	err = EMSR_NOW;
	
	return( err );
}


	CComboError
CTranslator::PrepareToTranslate( void )
{
	CComboError	err;
	
	err.pgpErr	= LoadDefaultKeySet( FALSE );
	if ( err.IsError() )
	{
		if ( err.pgpErr == kPGPError_FileNotFound ||
			err.pgpErr == kPGPError_CantOpenFile )
		{
			WarningAlert( kWANoteAlertType, kWAOKStyle,
				kTranslatorErrorStringsResID, kKeyringsNotFoundStrIndex);
		}
		else
		{
			ReportError( err );
		}
	}
	
	return( err );
}


	PluginError
CTranslator::TranslateFile(
	long			transContext,
	ConstemsMIMETypeHandle	inMIMEtypeHandle,
	const FSSpec *	inFSp,
	StringHandle**	addressesHandle,
	emsMIMETypeHandle*	outMIMEtypeHandlePtr,
	const FSSpec *	outFSp,
	StringHandle*	returnedMessage,
	StringHandle*	errorMessage,
	long *			resultCode)
{
	PluginError	err = EMSR_INVALID_TRANS;

	(void)transContext;
	(void)inMIMEtypeHandle;
	(void)inFSp;
	(void)addressesHandle;
	(void)outMIMEtypeHandlePtr;
	(void)outFSp;
	(void)returnedMessage;
	(void)resultCode;
	InitPtrToNil( returnedMessage );
	InitPtrToNil( errorMessage );

	return( err );
}

/*____________________________________________________________________________
	Caller should pass any desired options in 'optionsIn'. Examples:
		PGPOOutputLineEndType
		PGPOPGPMIMEEncoding
	
	All relavant options are set in this routine.
____________________________________________________________________________*/
	CComboError
CTranslator::HandleEncryptSign(
	const FSSpec *		inSpec,
	const FSSpec *		outSpec,
	StringHandle**		addressesHandle,
	Boolean				doSign,
	PGPOptionListRef	optionsIn )
{
	CComboError			err;
	PGPKeySetRef		recipientSet	= kPGPInvalidRef;
	Boolean				gotRecipients	= false;
							
	Boolean					doConventionalEncrypt	= FALSE;
	Boolean					reportedError	= false;
	CSecureCString256		conventionalPassphrase;
	PGPKeySetRef			newKeysToAdd	= kPGPInvalidRef;

	DebugCopyToRAMDisk( inSpec, "\pEncryptIn" );

	err.pgpErr	= LoadDefaultKeySet( FALSE );
	if ( err.IsntError() )
	{
		PGPRecipientSettings	settings	= 0;
	
		err	= GetRecipientList( mContext,
					addressesHandle,
					mKeySet,
					&recipientSet,
					&settings,
					&newKeysToAdd );
					
		doConventionalEncrypt	=
			( settings & kPGPRecipientSettingsConvEncrypt ) != 0;
	}
	
	if ( err.IsntError() && doConventionalEncrypt )
	{
		err.pgpErr	= PGPGetConventionalEncryptionPassphraseDialog( mContext,
						NULL, conventionalPassphrase );
	}
	
	/* enter this block if we are doing either kind of encrypt */
	if ( err.IsntError() &&
		( recipientSet != kPGPInvalidRef || doConventionalEncrypt ) )
	{
		PGPKeyRef			signingKey	= kPGPInvalidRef;
		CSecureCString256	signingPassphrase;
		PGPContextRef		c	= mContext;
		
		signingPassphrase.mString[ 0 ]	= '\0';
			
		if ( doSign && strlen( signingPassphrase ) == 0 )
		{
			err		= GetSigningPassphrase( signingPassphrase,
						&signingKey );
		}
		if ( err.IsntError() )
		{
			EncryptSignEventHandlerData	data( this );
			char				comment[ 256 ];
			PGPOptionListRef	optionList	= kPGPInvalidRef;
			
			PrefGetComment( comment );
			
			// all these options are the same, regardless of whether it's
			// PGP/MIME or not
			err.pgpErr	= PGPBuildOptionList( c, &optionList,
				PGPOInputFileFSSpec( c, inSpec ),
				PGPOOutputFileFSSpec( c, outSpec ),
				PGPOLocalEncoding( c, kPGPLocalEncoding_None ),
				PGPOArmorOutput(c, TRUE),
				PGPODataIsASCII( c, TRUE ),
				PGPOEventHandler( c, sPGPEventHandler, &data ),
				PGPOSendNullEvents( c, TRUE ),
				PGPOCommentString( c, comment ),
				PGPOVersionString( c, pgpVersionHeaderString ),
				PGPOLastOption(c)
				);
			if ( err.IsntError() )
			{
				if ( doConventionalEncrypt  )
				{
					err.pgpErr	= PGPAppendOptionList( c, optionList,
							PGPOConventionalEncrypt( c, 
								PGPOPassphrase( c, conventionalPassphrase ),
								PGPOLastOption( c )),
							PGPOCipherAlgorithm( c,
								PrefGetConventionalCipher() ),
							PGPOLastOption( c ) );
				}
				else
				{
					PreferredAlgorithmsStruct	algs;
					
					PrefGetPreferredAlgorithms( &algs );
					if ( algs.numAlgs !=0 )
					{
						err.pgpErr	= PGPAppendOptionList( c, optionList,
								PGPOPreferredAlgorithms( c,
									algs.algs, algs.numAlgs),
								PGPOLastOption( c ) );
					}
					
					err.pgpErr	= PGPAppendOptionList( c, optionList,
							PGPOEncryptToKeySet( c, recipientSet ),
							PGPOLastOption( c ) );
				}
			}
			
			if ( err.IsntError() && doSign )
			{
				err.pgpErr	= PGPAppendOptionList( c, optionList, 
					PGPOSignWithKey( c, signingKey,
						PGPOPassphrase(c, signingPassphrase),
						PGPOLastOption(c) ),
					PGPOLastOption( c ) );
			}
			
			if ( err.IsntError() )
			{
				err.pgpErr	= PGPEncode( c, optionList,
					optionsIn, PGPOLastOption( c ) );
			}
			
			if ( optionList != kPGPInvalidRef )
				PGPFreeOptionList( optionList );
		}
		
		
		DebugCopyToRAMDisk( outSpec, "\pEncryptFinalOut" );
	
		if ( err.IsntError() && doSign )
		{
			gSigningPassphraseCache->Remember( signingPassphrase,
				signingKey );
			
			// remember it for encryption as well (but not vice versa)
			gEncryptionPassphraseCache->Remember( signingPassphrase );
		}
	}

	if ( recipientSet != kPGPInvalidRef )
	{
		PGPFreeKeySet( recipientSet );
		recipientSet	= NULL;
	}
	
	/* allow user to add any new keys we fetched while sending */
	if ( PGPRefIsValid( newKeysToAdd ) )
	{
		if ( err.IsntError() )
		{
			PGPKeySetRef	importSet	= kPGPInvalidRef;
			char			prompt[ sizeof( Str255 ) ];
			
			GetIndCString( prompt, kTranslatorStringsRID,
				kSelectiveImportAfterSendPromptStrIndex );
			
			err.pgpErr	= PGPSelectiveImportDialog( mContext,
						prompt, newKeysToAdd,
						mKeySet, &importSet);
			if ( err.IsntError() )
			{
				PGPSharedAddKeysToDefaultKeyring( importSet );
				PGPFreeKeySet( importSet );
			}
		}
		PGPFreeKeySet( newKeysToAdd );
	}
		
	if ( ! reportedError )
	{
		ReportError( err );
	}

	return( err );
}





	PGPError
CTranslator::PGPEventHandler(
	PGPContextRef		context,
	PGPEvent *			event,
	EventHandlerData *	data)
{
	CComboError	err;
	
	pgpAssert( context == mContext );
	(void)context;
	
	if ( event->type == kPGPEvent_PassphraseEvent )
	{
		err	= HandlePassphraseEvent( event, data );
	}
	else if ( event->type == kPGPEvent_SignatureEvent )
	{
		err	= HandleSignatureEvent( event, data );
	}
	else if ( event->type == kPGPEvent_RecipientsEvent )
	{
		err	= HandleRecipientsEvent( event, data );
	}
	else if ( event->type == kPGPEvent_NullEvent )
	{
		err	= HandleProgressEvent( event, data );
	}
	
	if ( err.HaveNonPGPError() && ! err.HavePGPError() )
	{
		err.pgpErr	= MacErrorToPGPError( err.err );
	}
	
	return( err.pgpErr );
}



	PGPError
CTranslator::sPGPEventHandler(
	PGPContextRef	context,
	PGPEvent *		event,
	PGPUserValue	userValue)
{
	PGPError					err;
	EventHandlerData *	data	= (EventHandlerData *)userValue;
	
	pgpAssert( data->mMagic == EventHandlerData::kMagic );
	err	= data->mTranslator->PGPEventHandler( context, event, data);
	
	return( err );
}




	void
CTranslator::GetErrorString(
	CComboError		err,
	StringPtr		resultString )
{
	if ( err.HavePGPError() )
	{
		/* pgp error */
		switch( err.pgpErr )
		{
			case memFullErr:
			case kPGPError_OutOfMemory:
				GetIndString( resultString,
					kTranslatorErrorStringsResID, kOutOfMemoryStrIndex );
				break;
			
			case kPGPError_AdminPrefsNotFound:
				GetIndString( resultString,
					kTranslatorErrorStringsResID, kAdminPrefsNotFoundStrIndex );
				break;
				
			default:
			{
				Str255	errString;

				PGPGetErrorString( err.pgpErr,
					sizeof( errString ), (char *)errString);
				CToPString( (char *)errString, errString );
					
				GetIndString( resultString, kTranslatorErrorStringsResID,
					kGenericErrorTemplateString );
				
				PrintPString( resultString, resultString, errString );
				break;
			}
		}
	}
	else
	{
		/* regular mac error */
		switch( err.err )
		{
			case memFullErr:
				GetIndString( resultString,
					kTranslatorErrorStringsResID, kOutOfMemoryStrIndex );
				break;
				
			default:
			{
				Str255	errorNumberString;
				
				GetOSErrorString( err.err, errorNumberString);
				NumToString( err.err, errorNumberString );
				
				GetIndString( resultString, kTranslatorErrorStringsResID,
					kGenericErrorTemplateString );
				
				PrintPString( resultString, resultString, errorNumberString );
				break;
			}
		}
	}
}


	void
CTranslator::ReportError( CComboError	err )
{
	if ( err.IsError() && ! err.IsCancelError() )
	{
		Str255	errorMessage;
		
		GetErrorString( err, errorMessage );
		
		WarningAlert( kWANoteAlertType, kWAOKStyle, errorMessage );
	}
}


	CComboError
CTranslator::HandlePassphraseEvent(
	PGPEvent *			event,
	EventHandlerData *	data )
{
	(void)event;
	(void)data;
	pgpDebugMsg( "should never get here" );
	return( CComboError( CComboError::kPGPError, kPGPError_UnknownError ) );
}



	CComboError
CTranslator::HandleDecryptPassphraseEvent(
	PGPEvent *			event,
	EventHandlerData *	dataIn )
{
	CComboError		err;
	DecryptVerifyEventHandlerData *	data	=
		(DecryptVerifyEventHandlerData *)dataIn;
	
	if( event->data.passphraseData.fConventional )
	{
		err.pgpErr = PGPGetConventionalDecryptionPassphraseDialog( mContext,
				NULL, data->mPassphrase );
	}
	else
	{
		PGPKeyRef	whichKey;
		
		err.pgpErr = PGPSharedGetIndKeyFromKeySet(
						event->data.passphraseData.keyset,
						kPGPAnyOrdering, 1, &whichKey );
		/* 
		   If there aren't any keys in the key set, then return
		   the fact that the needed secret key cound not be found
		*/
		if( err.pgpErr == kPGPError_ItemNotFound )
		{
			err.pgpErr = kPGPError_SecretKeyNotFound;
		}
		else
		{
			err	= GetDecryptionPassphrase( event->data.passphraseData.keyset,
						data->mRecipientsList,
						data->mNumMissingKeys, data->mPassphrase );
		}

	}
	
	if ( err.IsntError() )
	{
		err.pgpErr = PGPAddJobOptions(	event->job,
						PGPOPassphrase(	mContext, data->mPassphrase ),
						PGPOLastOption(mContext)
						);
	}

	return( err );
}


	CComboError
CTranslator::HandleRecipientsEvent(
	PGPEvent *			event,
	EventHandlerData *	dataIn )
{
	CComboError		err;
	PGPUInt32		keysInSet	= 0;
	
	DecryptVerifyEventHandlerData *	data	=
		(DecryptVerifyEventHandlerData *)dataIn;
	
	data->mRecipientsList	= event->data.recipientsData.recipientSet;
	PGPIncKeySetRefCount( data->mRecipientsList );
	
	err.pgpErr	= PGPCountKeys( data->mRecipientsList, &keysInSet );
	pgpAssert( IsntPGPError( err.pgpErr ) );
	pgpAssert( event->data.recipientsData.keyCount >= keysInSet );
	data->mNumMissingKeys	= event->data.recipientsData.keyCount - keysInSet;
	
	
	return( err );
}


							
	CComboError
CTranslator::HandleSignatureEvent(
	PGPEvent *			event,
	EventHandlerData *	dataIn )
{
	CComboError			err;
	DecryptVerifyEventHandlerData *	data	=
		(DecryptVerifyEventHandlerData *)dataIn;
	char	theString[ kPGPMaxKeyIDStringSize ];
		
	data->mSignatureData.sigData	= event->data.signatureData;
	data->mHaveSignatureData		= TRUE;
	
	err.pgpErr	= PGPGetKeyIDString(
		&event->data.signatureData.signingKeyID,
		kPGPKeyIDString_Abbreviated, theString);
		
	if ( err.IsntError() )
	{
		CopyCString( theString, data->mSignatureData.keyIDString );
	}
	else
	{
		data->mSignatureData.keyIDString[ 0 ]	= '\0';
	}
	
	return( err );
}


	CComboError
CTranslator::HandleProgressEvent(
	PGPEvent *			event,
	EventHandlerData *	data)
{
	CComboError	err;
	
	(void)data;
	pgpAssert( event->type == kPGPEvent_NullEvent );
	/* progress event */
	if ( IsntNull( mEudoraProgress ) )
	{
		PGPUInt32		percentDone	= 0;
		PGPFileOffset	bytesWritten;
		PGPFileOffset	bytesTotal;
		
		bytesWritten	= event->data.nullData.bytesWritten;
		bytesTotal		= event->data.nullData.bytesTotal;
		if ( bytesTotal != 0 )
		{
			percentDone	= 100 * ((float)bytesWritten / (float)bytesTotal);
		}
		
		if ( callProgress( mEudoraProgress, percentDone ) != 0 )
		{
			err.pgpErr	= kPGPError_UserAbort;
		}
	}
	
	return( err );
}







	static MenuHandle
GetMessagePluginsMenu()
{
	MenuHandle	pluginsMenu	= NULL;
	MenuHandle	editMenu	= NULL;
	Str255		itemText;
	
	/* get edit menu */
	editMenu	= GetIndMenuInMenuBar( 2 );
	
	if ( IsntNull( editMenu ) )
	{
		short		numItems	= CountMItems( editMenu );
		Boolean		foundIt	= FALSE;
		short		cmdChar;
		short		cmdIndex;
		
		/* search for command containing "Message Plug\320ins" */
		for( cmdIndex = 1; cmdIndex <= numItems; ++cmdIndex )
		{
			char	cText[ 256 ];
			
			GetMenuItemText( editMenu, cmdIndex, itemText);
			PToCString( itemText, cText );
			
			if ( FindSubStringNoCase( cText, "Message Plug\320ins" ))
			{
				foundIt	= TRUE;
				break;
			}
		}
		if ( ! foundIt )
		{
			/* not found; assume hard-coded location */
			cmdIndex	= 16;
		}
		
		GetItemCmd( editMenu, cmdIndex, &cmdChar );
		if ( cmdChar == 0x1B )
		{
			/* it's a submenu */
			short	subMenuID;
			
			GetItemMark( editMenu, cmdIndex, &subMenuID );
			if ( subMenuID != noMark )
			{
				pluginsMenu	= GetMenuHandle( subMenuID );
			}
		}
	}
	
	
	return( pluginsMenu );
}


	static void
AddKeyForCommand(
	ConstStringPtr	cmd,
	char			cmdKey )
{
	MenuHandle			pluginsMenu	= NULL;
	
	pluginsMenu	= GetMessagePluginsMenu();
	if ( IsntNull( pluginsMenu ) )
	{
		short		numItems;
		Str255		itemText;
		
		numItems	= CountMItems( pluginsMenu );
		for( short index	= 1; index <= numItems; ++index )
		{
			GetMenuItemText( pluginsMenu, index, itemText );
			if ( PStringsAreEqual( itemText, cmd ) )
			{
				SetItemCmd( pluginsMenu, index, cmdKey);
				break;
			}
		}
	}
}


	static void
AddCommandKeys()
{
	static Boolean		sAddedKeys	= FALSE;
	
	if ( sAddedKeys )
		return;
	sAddedKeys	= TRUE;
	
	/* commands must match those in translator info */
	AddKeyForCommand( "\pPGP Sign", '2' );
	AddKeyForCommand( "\pPGP Encrypt", '3' );
	AddKeyForCommand( "\pPGP Encrypt/Sign", '4' );
	AddKeyForCommand( "\pPGP Decrypt/Verify", '5' );
}














