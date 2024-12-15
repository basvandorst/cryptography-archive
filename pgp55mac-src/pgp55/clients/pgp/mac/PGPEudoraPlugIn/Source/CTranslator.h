/*____________________________________________________________________________
	CTranslator.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CTranslator.h,v 1.14 1997/10/10 18:57:44 lloyd Exp $
____________________________________________________________________________*/
#pragma once


#include "EudoraTypes.h"
#include "MacStrings.h"
#include "PGPPlugTypes.h"
#include "TranslatorIDs.h"
#include "CComboError.h"
#include "pgpKeys.h"

#define InitPtrToNil( p )	{ if ( IsntNull( p ) )	*(p) = nil; }

			
typedef enum
{
	kUsePGPMIME = 3,	// arbitrary
	kUseOldPGPFormat
} CipherOutputType;
	
struct PGPKeySpec;

#include "PassphraseCache.h"
#include "pgpEncode.h"
#include "SignatureStatusMessage.h"

class CTranslator
{
protected:
	
	class EventHandlerData
	{
	public:
		enum { kMagic = 'EHND' };
		ulong			mMagic;
		CTranslator *	mTranslator;
							
	protected:
		EventHandlerData( CTranslator *translator )
		{
			pgpAssert( IsntNull( translator ) );
			mMagic			= kMagic;
			mTranslator	 	= translator;
		}
	};
	
	class EncryptSignEventHandlerData : public EventHandlerData
	{
	public:
		PGPKeyRef		mLastKey;
		
		EncryptSignEventHandlerData( CTranslator *translator )
			: EventHandlerData( translator )
		{
			mLastKey		= kPGPInvalidRef;
		}
		
	};
	
	class DecryptVerifyEventHandlerData : public EventHandlerData
	{
	public:
		Boolean					mHaveSignatureData;
		SignatureStatusInfo		mSignatureData;
		char 					mPassphrase[ 256 ];
		
		PGPKeySetRef			mRecipientsList;
		PGPUInt32				mNumMissingKeys;
		
		DecryptVerifyEventHandlerData( CTranslator *translator )
			: EventHandlerData( translator )
		{
			mPassphrase[ 0 ]	= '\0';
			mHaveSignatureData	= FALSE;
			mRecipientsList		= kInvalidPGPKeySetRef;
			mNumMissingKeys		= 0;
		}
		~DecryptVerifyEventHandlerData( void )
		{
			pgpClearMemory( mPassphrase, sizeof( mPassphrase ) );
			
			if ( PGPKeySetRefIsValid( mRecipientsList ) )
			{
				PGPFreeKeySet( mRecipientsList );
				mRecipientsList	= kInvalidPGPKeySetRef;
			}
		}
	};
	
	
	PGPContextRef	mContext;
	PGPKeySetRef	mKeySet;
	emsProgress		mEudoraProgress;
	
	PGPError		LoadDefaultKeySet( PGPBoolean	writeable );
	
	CComboError		GetSigningPassphrase( 
						char * passphrase, PGPKeyRef * signingKey);
				
	CComboError		GetDecryptionPassphrase(  
						PGPKeySetRef	privateKeys,
						PGPKeySetRef	allKeys,
						PGPUInt32		numMissing,
						char *			passphrase );

	CComboError		HandleEncryptSign( const FSSpec *in,
						const FSSpec *out, 
						StringHandle** addressesHandle,
						Boolean doSign, PGPOptionListRef options );

	static PGPError (sPGPEventHandler)(PGPContextRef context,
						PGPEvent *event, PGPUserValue userValue);
	
	virtual PGPError	PGPEventHandler(PGPContextRef context,
							PGPEvent *event, EventHandlerData *data );
	
	virtual CComboError	HandlePassphraseEvent(
							PGPEvent *event,
							EventHandlerData *data );
							
	virtual CComboError	HandleDecryptPassphraseEvent(
							PGPEvent *event,
							EventHandlerData *data );
							
	virtual CComboError	HandleRecipientsEvent(
							PGPEvent *event,
							EventHandlerData *data );
							
	virtual CComboError	HandleSignatureEvent(
							PGPEvent *event,
							EventHandlerData *data );
							
	virtual CComboError	HandleProgressEvent(
							PGPEvent *event,
							EventHandlerData *data );
	
	
	
	virtual PluginError	CanTranslateMIMEType( long transContext,
							ConstemsMIMETypeHandle mimeType,
							ConstStr255Param type, ConstStr255Param subType,
							ConstStr255Param protocol );
	
	virtual void		GetErrorString( CComboError err, StringPtr msg);
	virtual void		ReportError( CComboError err );
	
public:
				CTranslator( PGPContextRef	context,
					emsProgress progressHook);
	virtual		~CTranslator( void );
	
	static	PluginError	GetInfo(  TranslatorID id,
							long * type, long * subtype, unsigned long * flags,
							StringHandle* trans_desc, Handle * iconHandlePtr);
	
	virtual PluginError	CanTranslate( long ransContext,
						emsMIMEtype **	inMimeTypeHandle,
						const FSSpec *	inFSp, StringHandle** addressesHandle,
						long * aprox_len, StringHandle* errorMessage,
						long * result_code);
	
	/* return TRUE if successful; report any errors */
	virtual CComboError		PrepareToTranslate( void );
						
	virtual PluginError	TranslateFile( long transContext, 
							ConstemsMIMETypeHandle InMIMEtypeHandle,
							const FSSpec * inFSp,
							StringHandle** addressesHandle,
							emsMIMEtype ***	OutMIMEtypeHandlePtr,
							const FSSpec * outFSp,
							StringHandle* returnedMessage,
							StringHandle* errorMessage, long * resultCode);
};
