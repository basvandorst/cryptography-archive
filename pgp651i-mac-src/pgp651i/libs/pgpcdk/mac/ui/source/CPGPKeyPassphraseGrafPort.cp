/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CPGPKeyPassphraseGrafPort.cp,v 1.11 1999/03/10 02:36:52 heller Exp $____________________________________________________________________________*/#include <LStaticText.h>#include <PP_Messages.h>#include "MacStrings.h"#include "pgpErrors.h"#include "pgpKeys.h"#include "pgpMem.h"#include "CPassphraseEdit.h"#include "CPGPKeyPassphraseGrafPort.h"#include "PGPsdkUILibDialogs.h"#include "PGPsdkUILibUtils.h"CPGPKeyPassphraseGrafPortView::	CPGPKeyPassphraseGrafPortView(LStream *inStream)	: CPGPPassphraseGrafPortView(inStream){	mOptions = NULL;}CPGPKeyPassphraseGrafPortView::~CPGPKeyPassphraseGrafPortView(){}	PGPErrorCPGPKeyPassphraseGrafPortView::SetOptions(	PGPContextRef					context,	CPGPKeyPassphraseDialogOptions 	*options){	PGPError	err = kPGPError_NoErr;		err = CPGPPassphraseGrafPortView::SetOptions( context, options );	if( IsntPGPError( err ) )	{		mOptions = options;				if( PGPKeyRefIsValid( mOptions->mDefaultKey ) )		{			LStaticText	*keyCaption;						keyCaption = (LStaticText *) FindPaneByID( kKeyIDTextPaneID );			if( IsntNull( keyCaption ) )			{				Str255		buf;				PGPSize		len;								err = PGPGetPrimaryUserIDNameBuffer( mOptions->mDefaultKey,					sizeof( buf ) - 1, (char *) &buf[1], &len );				if ( IsntPGPError( err ) )				{					buf[0] = len;									keyCaption->SetDescriptor( buf );				}			}		}	}		return( err );}	PGPKeyRefCPGPKeyPassphraseGrafPortView::GetPassphraseKey(void){	return( mOptions->mDefaultKey );}	BooleanCPGPKeyPassphraseGrafPortView::VerifyPassphrase(void){	Boolean		valid = FALSE;		if( mOptions->mVerifyPassphrase &&		IsntNull( *mOptions->mPassphrasePtr ) )	{		PGPKeyRef	passphraseKey;				passphraseKey = GetPassphraseKey();		pgpAssert( PGPKeyRefIsValid( passphraseKey ) );		if ( PGPPassphraseIsValid( passphraseKey,				PGPOPassphrase( mContext, *mOptions->mPassphrasePtr ),				PGPOLastOption( mContext ) ) )		{			valid = TRUE;		}		else		{			SysBeep( 1 );						PGPUIWarningAlert( kPGPUIGenericOKAlert,						kPGPLibDialogsStringListResID,						kIncorrectPassphraseStrIndex );		}	}	else	{		valid = TRUE;	}	return( valid );}	MessageTCPGPKeyPassphraseGrafPortView::HandleMessage(MessageT theMessage){	theMessage = CPGPPassphraseGrafPortView::HandleMessage( theMessage );	if( theMessage == msg_OK )	{		if( ! VerifyPassphrase() )		{			ClearPassphrase();			/* Force dialog to stick around */			theMessage = msg_Nothing;		}	}		return( theMessage );}CPGPKeySetPassphraseGrafPortView::	CPGPKeySetPassphraseGrafPortView(LStream *inStream)	: CPGPKeyPassphraseGrafPortView(inStream){	mOptions = NULL;}CPGPKeySetPassphraseGrafPortView::~CPGPKeySetPassphraseGrafPortView(){}	PGPErrorCPGPKeySetPassphraseGrafPortView::SetOptions(	PGPContextRef						context,	CPGPKeySetPassphraseDialogOptions 	*options){	PGPError	err = kPGPError_NoErr;		err = CPGPKeyPassphraseGrafPortView::SetOptions( context, options );	if( IsntPGPError( err ) )	{		mOptions = options;	}		return( err );}	BooleanCPGPKeySetPassphraseGrafPortView::VerifyPassphrase(void){	Boolean		valid = FALSE;		if( mOptions->mVerifyPassphrase &&		IsntNull( *mOptions->mPassphrasePtr ) )	{		PGPKeyRef	passphraseKey;		passphraseKey = GetPassphraseKey();		pgpAssert( PGPKeyRefIsValid( passphraseKey ) );			if ( PGPPassphraseIsValid( passphraseKey,				PGPOPassphrase( mContext, *mOptions->mPassphrasePtr ),				PGPOLastOption( mContext ) ) )		{			valid = TRUE;		}		else		{			Boolean	badPassphrase = TRUE;						if( mOptions->mFindMatchingKey )			{				passphraseKey = FindKeyForPassphrase( mOptions->mKeySet,									*mOptions->mPassphrasePtr );				if ( PGPKeyRefIsValid( passphraseKey ) )				{					badPassphrase 	= FALSE;					valid			= TRUE;				}			}						if( badPassphrase )			{				SysBeep( 1 );								PGPUIWarningAlert( kPGPUIGenericOKAlert,							kPGPLibDialogsStringListResID,							kIncorrectPassphraseStrIndex );			}		}		if( valid )		{			*mOptions->mPassphraseKeyPtr = passphraseKey;		}	}	else	{		valid = TRUE;	}	return( valid );}