/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "WarningAlertTypes.h"

#include "PGPtools.h"
#include "pgpEncode.h"
#include "pgpUserInterface.h"
#include "PGPSharedEncryptDecrypt.h"
#include "CResultsTable.h"

struct PGPKeySpec;
class CPGPtoolsTaskProgressDialog;
class LArray;
class CPGPtoolsPassphraseList;

#include "CPGPtoolsTask.h"

/*____________________________________________________________________________
	The encode/decode params should contain only information need by a
	task to set up and get started.  It should not be used for callback info.
____________________________________________________________________________*/
class CEncodeDecodeParams
{
public:
	CPGPtoolsTaskProgressDialog	*progressDialog;
	PGPKeySetRef				allKeys;
	
	CEncodeDecodeParams( void )
	{
		progressDialog		= NULL;
		allKeys				= kInvalidPGPKeySetRef;
	}
	virtual ~CEncodeDecodeParams( void )
	{
	}
};


class CDecodeParams : public CEncodeDecodeParams
{
public:
	CPGPtoolsPassphraseList		*passphraseList;
	
	CDecodeParams( void ) : CEncodeDecodeParams()
	{
		passphraseList		= NULL;
	}
} ;

class CEncodeParams : public CEncodeDecodeParams
{
public:
	PGPOptionListRef			encodeOptions;
	Boolean						encrypting;
	Boolean						conventionalEncrypt;
	Boolean						signing;
	Boolean						detachedSignature;
	Boolean						textOutput;
	Boolean						wipeOriginal;
	Boolean						encryptingToSelf;
	PGPKeyRef					signingKey;
	PGPLocalEncodingFlags		encodingFlags;
	char						signingPassphrase[256];
	
	CEncodeParams( void )
		: CEncodeDecodeParams()
	{
		encodeOptions			= kInvalidPGPOptionListRef;
		encrypting				= FALSE;
		signing					= FALSE;
		wipeOriginal			= FALSE;
		detachedSignature		= FALSE;
		textOutput				= FALSE;
		signingKey				= kInvalidPGPKeyRef;
		signingPassphrase[ 0 ]	= '\0';
	}
	
	virtual ~CEncodeParams( void )
	{
		if( PGPOptionListRefIsValid( encodeOptions ) )
			PGPFreeOptionList( encodeOptions );
			
		pgpClearMemory( signingPassphrase, sizeof( signingPassphrase ) );
	}
	
} ;




class CPGPtoolsCryptoTask : public CPGPtoolsTask
{
public:
						CPGPtoolsCryptoTask(PGPtoolsOperation operation);
	virtual				~CPGPtoolsCryptoTask(void);

	virtual	CToolsError	DecryptVerify(PGPContextRef context,
								CDecodeParams *state) = 0;
	virtual	CToolsError	EncryptSign(PGPContextRef context,
								CEncodeParams *state) = 0;
	
protected:

	PGPContextRef			mContext;
	UInt32					mBadPassphraseCount;
	PGPAnalyzeType			mDataType;
	PGPtoolsSignatureData	mSignatureData;
	Boolean					mSignatureDataValid;
	PGPKeySetRef			mRecipientsKeySet;
	PGPUInt32				mNumMissingRecipientKeys;
	
	class CEventHandlerData
	{
	public:
		enum { kMagic = 'EHND' };
		OSType					mMagic;
		CPGPtoolsCryptoTask *	mTask;
		CEncodeDecodeParams *	mParams;
		
		CEventHandlerData( CPGPtoolsCryptoTask *task,
				CEncodeDecodeParams *params )
		{
			mMagic	= kMagic;
			mTask	= task;
			mParams	= params;
		}
	} ;
	
	class CDecodeEventHandlerData : public CEventHandlerData
	{
	public:
		Boolean				retryingSigVerification;
		LArrayIterator *	passphraseIterator;
	
		CDecodeEventHandlerData( CPGPtoolsCryptoTask *task,
			CDecodeParams *state)
			: CEventHandlerData( task, state )
			{
			retryingSigVerification	= FALSE;
			passphraseIterator		= NULL;
			}
	} ;
	
	class CEncodeEventHandlerData : public CEventHandlerData
	{
	public:
		CEncodeEventHandlerData( CPGPtoolsCryptoTask *task,
			CEncodeDecodeParams *state )
			: CEventHandlerData( task, state )
		{
		}
	} ;
	
	virtual	PGPError	NullEvent( PGPEvent *event,
							CEventHandlerData *data);
								
	virtual PGPError	GenericEventHandler( PGPEvent *event,
							CEventHandlerData *state);
							
	virtual PGPError	DecodeEventHandler( PGPEvent *event,
							CDecodeEventHandlerData *state);
							
	virtual PGPError	EncodeEventHandler( PGPEvent *event,
								CEncodeEventHandlerData *data);
								
	virtual	PGPError	DecodeAnalyzeEvent(PGPEvent *event,
								CDecodeEventHandlerData *data);
								
	virtual	PGPError	DecodePassphraseEvent(
								PGPEvent *event,
								CDecodeEventHandlerData *data);
								
	virtual	PGPError	DecodeRecipientsEvent(
								PGPEvent *event,
								CDecodeEventHandlerData *data);

	virtual	PGPError	DecodeSignatureEvent(
								PGPEvent *event,
								CDecodeEventHandlerData *data);
								
	
	virtual	void		GetDecryptPassphrasePromptString(StringPtr prompt) = 0;
	
	static OSStatus		sProgressCallbackProc(UInt32 bytesProcessed,
								UInt32 totalBytesToProcess, void *userData);
								
	static PGPError		sEncodeEventHandlerProc(PGPContextRef context,
								PGPEvent *event, PGPUserValue userValue);
	static PGPError		sDecodeEventHandlerProc(PGPContextRef context,
								PGPEvent *event, PGPUserValue userValue);
							
};


