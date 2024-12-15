/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: GetPassphrase.cp,v 1.3 1997/09/12 22:00:51 heller Exp $
____________________________________________________________________________*/
#include "GetPassphrase.h"
#include "CPGPKeys.h"
#include "CRecessedCaption.h"
#include "CPassphraseEdit.h"
#include "CSecureMemory.h"
#include "MacStrings.h"
#include "pgpMem.h"


const ResIDT	kPassphraseDialogID	=	137;
const PaneIDT	kPassphraseFieldID	=	'ePas';
const PaneIDT	kKeyCaptionID		=	'cKey';
const PaneIDT	kIncorrectPassID	=	'cInc';
const PaneIDT	kHideTypingID		=	'xHid';

	Boolean
DoPassphraseDialog(Boolean firstTry, const char *userID, char *passphrase)
{
	StDialogHandler		theHandler(kPassphraseDialogID, CPGPKeys::TheApp());
	LWindow				*theDialog = theHandler.GetDialog();
	CRecessedCaption	*keyCaption;
	CPassphraseEdit		*theField;
	LGACheckbox			*hideCheckbox;
	CSecurePString255	ppassphrase;
	Str255				puserID;
	
	theField	= (CPassphraseEdit *)
		theDialog->FindPaneByID(kPassphraseFieldID);
	keyCaption	= (CRecessedCaption *)
		theDialog->FindPaneByID(kKeyCaptionID);
	hideCheckbox= (LGACheckbox *)
		theDialog->FindPaneByID(kHideTypingID);
	if(!firstTry)
		((LCaption *)theDialog->FindPaneByID(kIncorrectPassID))->Show();

	SignalIf_(theField == nil);
	SignalIf_(keyCaption == nil);
	
	ppassphrase[0] = 0;
	theField->SelectAll();
	CToPString(userID, puserID);
	keyCaption->SetDescriptor(puserID);
	theDialog->SetLatentSub(theField);
	theDialog->Show();
	
	Boolean		entryOK = false;
	
	while (true) {
		MessageT	hitMessage = theHandler.DoDialog();
		
		if(hitMessage == msg_Cancel)
			break;
		else if(hitMessage == kHideTypingID)
			theField->SetHideTyping((hideCheckbox->GetValue() == 1));
		else if(hitMessage == msg_OK)
		{
			theField->GetDescriptor(ppassphrase);
			entryOK = true;
			break;
		}
	}
	if(entryOK)
		PToCString(ppassphrase, passphrase);
	return entryOK;

}

