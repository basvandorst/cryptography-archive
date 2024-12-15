/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPadmin.cp,v 1.10.8.1 1997/12/05 22:13:39 mhw Exp $
____________________________________________________________________________*/


#include <LGrowZone.h>
#include <LWindow.h>
#include <PP_Messages.h>
#include <PP_Resources.h>
#include <PPobClasses.h>
#include <UDrawingState.h>
#include <UMemoryMgr.h>
#include <URegistrar.h>

#include <LGASeparator.h>
#include <LGAPushButton.h>
#include <LGASecondaryBox.h>
#include <LGACheckbox.h>
#include <LGACaption.h>
#include <LGAEditField.h>
#include <LActiveScroller.h>
#include <LGABox.h>
#include <LGAWindowHeader.h>
#include <LIconPane.h>

#include "MacInternet.h"

#include "pgpUtilities.h"
#include "pgpUserInterface.h"
#include "pgpOpenPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpSDKPrefs.h"
#include "pflPrefTypes.h"
#include "pflContext.h"

#include "CVersionCaption.h"
#include "CPGPStDialogHandler.h"
#include "CPicture.h"
#include "CWarningAlert.h"

#include "CString.h"
#include "StSaveCurResFile.h"
#include "CRestorePortColorsAttachment.h"
#include "CKeyTable.h"
#include "CAdminDialog.h"
#include "PGPadmin.h"


const ResIDT	ppob_Admin		=	128;

const ResIDT	STRx_AdminApp	=	1001;
const SInt16	kURLID			=	1;
const SInt16	kCreditsID		=	2;
const SInt16	kInfoID			=	3;

const ResID		window_About	=	3000;

PGPContextRef	gPGPContext = kPGPInvalidRef;


	void
main(void)
{
	// Set Debugging options
	SetDebugThrow_(debugAction_Alert);
	SetDebugSignal_(debugAction_Alert);

	// Init
	InitializeHeap(3);
	UQDGlobals::InitializeToolbox(&qd);
	new LGrowZone(20000);
	
	// Create our context
	PGPError	pgpErr;
	
	pgpErr = PGPNewContext( kPGPsdkAPIVersion, &gPGPContext);
	if (IsntPGPError(pgpErr)) {
		{
			// Run
			CAdminApp	theApp;
		
//			pgpLeaksBeginSession("PGPadmin");
			theApp.Run();
//			pgpLeaksEndSession();
		}
		PGPFreeContext(gPGPContext);
	}
}



CAdminApp::CAdminApp()
{
	// Register functions to create classes
	RegisterClass_(CAdminDialog);
	RegisterClass_(LGASeparator);
	RegisterClass_(LGAPushButton);
	RegisterClass_(LGASecondaryBox);
	RegisterClass_(CPicture);
	RegisterClass_(LTextEdit);
	RegisterClass_(CRestorePortColorsAttachment);
	RegisterClass_(LView);
	RegisterClass_(LGACheckbox);
	RegisterClass_(LGACaption);
	RegisterClass_(LActiveScroller);
	RegisterClass_(CKeyTable);
	RegisterClass_(LGAEditField);
	RegisterClass_(LGADialogBox);
	RegisterClass_(LGABox);
	RegisterClass_(LCaption);
	RegisterClass_(LGAWindowHeader);
	RegisterClass_(CVersionCaption);
	RegisterClass_(CWarningAlert);
	RegisterClass_(LIconPane);
}



CAdminApp::~CAdminApp()
{
}



void
CAdminApp::StartUp()
{
	PGPsdkLoadDefaultPrefs(gPGPContext);
	ObeyCommand(cmd_New, nil);
}



	Boolean
CAdminApp::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand) {
		case cmd_New:
		{
			LWindow *	theWindow;
			
			theWindow = LWindow::CreateWindow(ppob_Admin, this);	
			theWindow->Show();
		}
		break;


		default:
		{
			cmdHandled = LApplication::ObeyCommand(inCommand, ioParam);
		}
		break;
	}
	
	return cmdHandled;
}



	void
CAdminApp::FindCommandStatus(
	CommandT	inCommand,
	Boolean &	outEnabled,
	Boolean &	outUsesMark,
	Char16 &	outMark,
	Str255		outName)
{

	switch (inCommand) {
		default:
		{
			LApplication::FindCommandStatus(	inCommand,
												outEnabled,
												outUsesMark,
												outMark,
												outName);
		}
		break;
	}
}



	void
CAdminApp::MakeLicenseStr(
	CString &	outLicenseStr)
{
	PFLContextRef	pflContext = kPGPInvalidRef;
	PGPPrefRef		prefRef = kPGPInvalidRef;
	PGPError		pgpErr;
	
	try {
		CString	companyStr;
		CString	licenseStr;

		pgpErr = PFLNewContext(&pflContext);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPOpenClientPrefs(	pflContext,
										&prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPGetPrefStringBuffer(	prefRef,
											kPGPPrefOwnerName,
											outLicenseStr.GetBufferSize(),
											outLicenseStr);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPGetPrefStringBuffer(	prefRef,
											kPGPPrefCompanyName,
											companyStr.GetBufferSize(),
											companyStr);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPGetPrefStringBuffer(	prefRef,
											kPGPPrefLicenseNumber,
											licenseStr.GetBufferSize(),
											licenseStr);
		PGPThrowIfPGPErr_(pgpErr);

		strcat(outLicenseStr, "\r");
		strcat(outLicenseStr, companyStr);
		strcat(outLicenseStr, "\r");
		strcat(outLicenseStr, licenseStr);

		PGPClosePrefFile(prefRef);
		PFLFreeContext(pflContext);
	}
	
	catch (...) {
		if (prefRef != kPGPInvalidRef) {
			PGPClosePrefFile(prefRef);
		}
		if (pflContext != kPGPInvalidRef) {
			PFLFreeContext(pflContext);
		}
	}
}



	void
CAdminApp::ShowAboutBox()
{
	Boolean				didShowDialog = false;
	
	try {
		FSSpec				libraryFileSpec;
		OSStatus			err;
		StSaveCurResFile	savedResfile;
		LFile				libraryFile;

		// Open the libarary resource file at the top of the resource chain so
		// we get our pictures.
		err = PGPGetUILibFSSpec(&libraryFileSpec);
		PGPThrowIfOSErr_(err);
		libraryFile.SetSpecifier(libraryFileSpec);
		libraryFile.OpenResourceFork(fsRdPerm);

		// Handle the dialog
		{
			CPGPStDialogHandler	aboutDialog(window_About, this);
			LWindow				*theDialog;
			MessageT			message;
			Boolean				credits = false;
			CString				licenseStr;
			
			const MessageT		kPGPButtonMessage		= 'bPGP';
			const MessageT		kCreditsButtonMessage	= 'bCre';
			const MessageT		kOKButtonMessage		= 'bOK ';
			
			const PaneIDT		kCreditsButtonPaneID	= 'bCre';
			const PaneIDT		kAboutPicturePaneID		= 'APIC';
			const PaneIDT		kCreditsPicturePaneID	= 'CPIC';
			const PaneIDT		kLicenseCaptionID		= 'cLic';

			theDialog = aboutDialog.GetDialog();
			
			didShowDialog = true;
			
			MakeLicenseStr(licenseStr);
			((LCaption *) theDialog->FindPaneByID(kLicenseCaptionID))->
					SetDescriptor(licenseStr);

			while ((message = aboutDialog.DoDialog()) != kOKButtonMessage) {
				if (message == kPGPButtonMessage) {
					err = OpenURL(CString(STRx_AdminApp, kURLID));
					PGPThrowIfOSErr_(err);
				} else if (message == kCreditsButtonPaneID) {
					SInt16	buttonTitleID;
					
					if (credits) {
						theDialog->FindPaneByID(kAboutPicturePaneID)->
									Show();
						theDialog->FindPaneByID(kCreditsPicturePaneID)->
									Hide();
						buttonTitleID = kCreditsID;
					} else {
						theDialog->FindPaneByID(kCreditsPicturePaneID)->
									Show();
						theDialog->FindPaneByID(kAboutPicturePaneID)->
									Hide();
						buttonTitleID = kInfoID;
					}
					
					((LControl *) theDialog->FindPaneByID(
								kCreditsButtonPaneID))->SetDescriptor(
								CString(STRx_AdminApp, buttonTitleID));

					credits = ! credits;
				}
			}
		}
	}
	
	catch (...) {
	}
	
	if (! didShowDialog) {
		::SysBeep(0);
	}
}
