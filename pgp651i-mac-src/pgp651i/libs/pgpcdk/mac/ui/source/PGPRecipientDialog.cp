/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: PGPRecipientDialog.cp,v 1.23 1999/03/10 02:54:42 heller Exp $____________________________________________________________________________*/#include <PP_Messages.h>#include <URegistrar.h>#include "MacStrings.h"#include "pgpMem.h"#include "CPGPRecipientGrafPort.h"#include "pgpDialogs.h"#include "PGPsdkUILibDialogs.h"#include "PGPsdkUILibUtils.h"const ResIDT kPGPRecipientDialogID = 4747;	PGPErrorpgpRecipientDialogPlatform(	PGPContextRef				context,	CPGPRecipientDialogOptions 	*options){	PGPsdkUILibState	state;	PGPError			err;		PGPValidatePtr( options );		err = EnterPGPsdkUILib( context, &state );	if( IsntPGPError( err ) )	{		DialogRef	theDialog;				RegisterClass_( CPGPRecipientGrafPortView );				// Create standard Macintosh window and overlay PowerPlant onto it		theDialog = CPGPRecipientGrafPortView::CreateDialog(							kPGPRecipientDialogID );		if( IsntNull( theDialog ) )		{			CPGPRecipientGrafPortView	*grafPortView;			PGPBoolean					showDialog;						grafPortView = (CPGPRecipientGrafPortView *)										GetWRefCon( theDialog );			pgpAssertAddrValid( grafPortView, VoidAlign );						err = grafPortView->SetOptions( context, options, &showDialog );			if( IsntPGPError( err ) )			{				if( showDialog )				{					err = grafPortView->DoDialog();				}				else				{					grafPortView->HandleMessage( msg_OK );				}			}						delete( grafPortView );			DisposeDialog( theDialog );		}		else		{			err = kPGPError_OutOfMemory;		}	}			ExitPGPsdkUILib( &state );		return( err );}	PGPErrorpgpGetMissingRecipientKeyIDStringPlatform(	PGPContextRef	context,	const PGPKeyID	*keyID,	char			keyIDString[256]){	PGPError	err;		(void) context;		err = PGPGetKeyIDString( keyID, kPGPKeyIDString_Abbreviated, keyIDString );	if( IsntPGPError( err ) )	{		Str255	pString;		Str255	tempStr;				CToPString( keyIDString, tempStr );		GetIndString( pString, kPGPLibDialogsStringListResID,				kUnknownKeyIDStrIndex );		PrintPString( pString, pString, tempStr );		PToCString( pString, keyIDString );	}		return( err );}