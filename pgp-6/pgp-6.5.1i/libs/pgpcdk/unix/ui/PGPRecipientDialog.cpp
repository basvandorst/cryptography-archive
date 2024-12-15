/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: PGPRecipientDialog.cpp,v 1.3.8.1 1999/06/04 01:12:05 heller Exp $
____________________________________________________________________________*/

#include "pgpDialogs.h"
//#include "RDprecmp.h"

#if 0
extern HINSTANCE g_hInst;
extern HHOOK hhookKeyboard;
extern HHOOK hhookMouse;

// BuildTable
//
// This routine takes our PGPRecipientsList and inserts them
// into the correct listview.

	UINT
PopDialog(
	PGPContextRef				context,
	CPGPRecipientDialogOptions 	*options,
	PGPRecipientsList			*mRecipients)
{
	PGPError			err = kPGPError_NoErr;
	PGPRecipientKey		*curKey;
	PGPRecipientUser	*curGroup;
	PGPRecipientUser	*curUser;
	PGPBoolean	haveRecipients = FALSE;
	UINT  RetVal = ADDUSER_OK;
	PGPInt32	minimumValidity;
				
	if( options->mIgnoreMarginalValidity )
	{
		minimumValidity = (PGPInt32) kPGPValidity_Complete;
	}
	else
	{
		minimumValidity = (PGPInt32) kPGPValidity_Marginal;
	}
				
	/*
	** Check to see if the dialog needs to be displayed.
	** Show the dialog there are missing, ambiguous, invalid,
	** or no default recipients
	*/

	curKey = mRecipients->keys;
	while( IsntNull( curKey ) )
	{
		curUser = curKey->users;
		while( IsntNull( curUser ) )
		{
			if( curUser->location == 
					kPGPRecipientUserLocation_RecipientList )
			{
				haveRecipients=TRUE;

				if( curUser->multipleMatch )
					RetVal=ADDUSER_MULTIPLEMATCH;
				
				if(	curUser->validity < minimumValidity )
					RetVal=ADDUSER_KEYSNOTVALID;
			}
			
			curUser = curUser->nextUser;
		}
	
		curKey = curKey->nextKey;
	}
	
	curGroup = mRecipients->groups;
	while( IsntNull( curGroup ) )
	{
		pgpAssert( curGroup->kind == kPGPRecipientUserKind_Group );
		
		if( curGroup->location == 
				kPGPRecipientUserLocation_RecipientList )
		{
			haveRecipients = TRUE;

			if( curUser->multipleMatch )
				RetVal=ADDUSER_MULTIPLEMATCH;
				
			if(	curUser->validity < minimumValidity )
				RetVal=ADDUSER_KEYSNOTVALID;
		}

		curGroup = curGroup->nextUser;
	}
	
	curUser = mRecipients->missingRecipients;

	while( IsntNull( curUser ) )
	{
		pgpAssert( curUser->kind ==
				kPGPRecipientUserKind_MissingRecipient );

		if( curUser->location ==
				kPGPRecipientUserLocation_RecipientList )
		{
			RetVal=ADDUSER_KEYSNOTFOUND;
		}
				
		curUser = curUser->nextUser;
	}

	if(( ! haveRecipients ) || (IsPGPError(err)))
		RetVal = ADDUSER_NORECIPIENTSTOSEARCH; 

	return( RetVal );
}
	
	PGPError
EventHandler(
	PGPRecipientsList 	*recipients,
	PGPRecipientEvent 	*event,
	PGPUserValue 		userValue)
{
	PGPError	err 		= kPGPError_NoErr;
	DWORD		strIndex 	= 0;
	const char	*userName	= NULL;
	PRECGBL prg;
	
	(void) recipients;
	(void) userValue;
	
	prg=(PRECGBL)userValue;

	switch( event->type )
	{
		case kPGPRecipientEvent_MoveUserFailedEvent:
		{
			if( event->user->kind == kPGPRecipientUserKind_Key )
			{
				strIndex = IDS_NOKEYADKERR;
			}
			else
			{
				strIndex = IDS_NOGROUPADKERR;
			}
		
			userName = PGPGetRecipientUserNamePtr( event->user );
			break;
		}
		
		case kPGPRecipientEvent_MoveUserWarningEvent:
		{
			if( event->user->kind == kPGPRecipientUserKind_Key )
			{
				strIndex = IDS_NOKEYADKWARN;
			}
			else
			{
				strIndex = IDS_NOGROUPADKWARN;
			}
		
			userName = PGPGetRecipientUserNamePtr( event->user );
			break;
		}
		
		case kPGPRecipientEvent_MovedARRWarningEvent:
		{
			strIndex = IDS_MOVEDARRWARN;
			userName = PGPGetRecipientUserNamePtr( event->user );
			
			break;
		}
	}
	
	if( strIndex != 0 )
	{
		char MsgTxt[255];
		char DlgTxt[255+kPGPMaxUserIDSize];

		LoadString(g_hInst, strIndex, MsgTxt, sizeof(MsgTxt));

		sprintf(DlgTxt,MsgTxt,userName);

		MessageBox(prg->hwndRecDlg, DlgTxt, "Recipient Warning", 
			MB_OK|MB_ICONWARNING);
	}
	
	return( err );
}
#endif

	PGPError
pgpRecipientDialogPlatform(
	PGPContextRef				context,
	CPGPRecipientDialogOptions 	*options)
{
	PGPError			err = -1;
//	PRECGBL				prg;
//	PGPUInt32				RetVal;
//	PGPBoolean			bPopDialog=TRUE;

	(void) context;
	(void) options;
	
#if 0
	prg=(PRECGBL)malloc(sizeof(RECGBL));

	if(prg)
	{
		memset(prg,0x00,sizeof(RECGBL));

		prg->context=context;
		prg->tlsContext=options->mTLSContext;
		prg->ksEntries=options->mServerList;
		prg->numKSEntries=options->mServerCount;
		prg->mWindowTitle=options->mWindowTitle;
		prg->mSearchBeforeDisplay=options->mSearchBeforeDisplay;
		prg->mDialogOptions=options->mDialogOptions;
		prg->mNumDefaultRecipients=options->mNumDefaultRecipients;
		prg->mDefaultRecipients=
				(struct PGPRecipientSpec *)options->mDefaultRecipients;
		prg->mDisplayMarginalValidity=options->mDisplayMarginalValidity;
		prg->mIgnoreMarginalValidity=options->mIgnoreMarginalValidity;
		prg->mGroupSet=options->mGroupSet;
		prg->mClientKeySet=options->mClientKeySet;
		prg->mRecipientKeysPtr=options->mRecipientKeysPtr;
		prg->mHwndParent=options->mHwndParent;
		prg->mNewKeys=options->mNewKeys;

		prg->g_hInst=g_hInst;

		err = PGPBuildRecipientsList( options->mHwndParent,
			context, options->mClientKeySet,
			options->mGroupSet, options->mNumDefaultRecipients,
			options->mDefaultRecipients, options->mServerCount,
			options->mServerList, options->mTLSContext,
			options->mSearchBeforeDisplay, options->mEnforcement,
			EventHandler, prg, &(prg->mRecipients) );

		RetVal=	PopDialog(context,options,&(prg->mRecipients));

		if(RetVal==ADDUSER_OK)
		{
			bPopDialog=FALSE;
		}

		if((bPopDialog)||(options->mAlwaysDisplay))
		{
			prg->AddUserRetVal=RetVal;
					
			InitRandomKeyHook(&hhookMouse,&hhookKeyboard);
			RetVal=newPGPRecipientDialog(prg);
			UninitRandomKeyHook(hhookMouse,hhookKeyboard);
		}
		else
		{
			RetVal=TRUE;
		}

		if(RetVal)
		{
			PGPGetRecipientKeys(&(prg->mRecipients),
				options->mRecipientKeysPtr, options->mNewKeys);
		}

		PGPDisposeRecipientsList(&(prg->mRecipients));

		free(prg);
	}

	if(RetVal)
		err=kPGPError_NoErr;
	else
		err=kPGPError_UserAbort;
#endif
	return( err );
}

    PGPError
pgpCheckNetworklibAvailability(void)
{
    // We're simply static linking for now.
    return( kPGPError_NoErr );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
