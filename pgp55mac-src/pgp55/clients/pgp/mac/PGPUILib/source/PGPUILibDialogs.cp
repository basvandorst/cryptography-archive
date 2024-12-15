/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: PGPUILibDialogs.cp,v 1.16 1997/10/30 20:53:05 heller Exp $
____________________________________________________________________________*/

#include <Gestalt.h>
#include <LowMem.h>

#include <LActiveScroller.h>
#include <LCaption.h>
#include <LGABox.h>
#include <LGACaption.h>
#include <LGACheckbox.h>
#include <LGAIconSuite.h>
#include <LGAIconSuiteControl.h>
#include <LGAPrimaryBox.h>
#include <LGASecondaryBox.h>
#include <LGAPushButton.h>
#include <LGAWindowHeader.h>
#include <LTabGroup.h>
#include <PP_Messages.h>
#include <UDrawingState.h>
#include <UGAColorRamp.h>
#include <UEnvironment.h>
#include <URegistrar.h>

#include "MacDebugLeaks.h"
#include "pgpMem.h"
#include "MacStrings.h"

#include "CPassphraseEdit.h"
#include "CPGPGetPassphraseGrafPort.h"
#include "CPGPGetConvPassphraseGrafPrt.h"
#include "CPrivateKeysPopup.h"
#include "CProgressBar.h"
#include "CGAFix.h"
#include "CFocusBorder.h"
#include "CUserIDTable.h"
#include "PGPRecipientGrafPort.h"
#include "PGPUILibDialogs.h"
#include "PGPSelectiveImportDialog.h"
#include "CSharedProgressBar.h"
#include "CServerStatusDialog.h"
#include "PowerPlantLeaks.h"

	static void
RegisterPowerPlantClasses(void)
{
	RegisterClass_(LCaption);
	RegisterClass_(LEditField);
	RegisterClass_(LPane);
	RegisterClass_(LScroller);
	RegisterClass_(LActiveScroller);
	RegisterClass_(LStdControl);
	RegisterClass_(LView);
	RegisterClass_(LTabGroup);
	RegisterClass_(LGrafPortView);
	
	RegisterClass_(LGABox);
	RegisterClass_(LGACaption);
	RegisterClass_(LGACheckbox);
	RegisterClass_(LGAEditField);
	RegisterClass_(LGAIconSuite);
	RegisterClass_(LGAIconSuiteControl);
	RegisterClass_(LGAPopup);
	RegisterClass_(LGAPrimaryBox);
	RegisterClass_(LGAPushButton);
	
	RegisterClass_(CPGPGetPassphraseGrafPort);
	RegisterClass_(CPGPGetConvPassphraseGrafPort);
	RegisterClass_(CPGPLibGrafPortView);
	RegisterClass_(CPGPRecipientGrafPort);
	RegisterClass_(CUserIDTable);
	RegisterClass_(CPassphraseEdit);
	RegisterClass_(CGAPopupFix);
	RegisterClass_(CGACheckboxFix);
	RegisterClass_(CPrivateKeysPopup);
	RegisterClass_(CProgressBar);
	RegisterClass_(CFocusBorder);
	
	RegisterClass_(CSelectiveImportDialog);
	RegisterClass_(CSelectiveImportTable);
	RegisterClass_(LGASecondaryBox);
	RegisterClass_(LGAWindowHeader);
	
	RegisterClass_(CServerStatusDialog);
	RegisterClass_(CSharedProgressBar);

	// LDragAndDrop leaks like a siieve. This is fixed by not using
	// the default RegisterClass behavior for those classes which
	// inherit from LDragAndDrop.
	
	URegistrar::RegisterClass(CUserIDTable::class_ID,
				(ClassCreatorFunc) CUserIDTable::CreateFromStream);
}

	void
InitializePowerPlant(void)
{
	static Boolean	firstTime = TRUE;
	
	if( firstTime )
	{
		Int32		qdVersion = gestaltOriginalQD;
		
#if PGP_DEBUG
		gDebugThrow = debugAction_SourceDebugger;
		gDebugSignal = debugAction_SourceDebugger;
#endif
		qdVersion = gestaltOriginalQD;
		if( IsErr( Gestalt( gestaltQuickdrawVersion, &qdVersion ) ) )
		{
			qdVersion = gestaltOriginalQD;
		}
		
		UQDGlobals::SetQDGlobals((QDGlobals*)(*((long*)::LMGetCurrentA5()) - 
				(sizeof(QDGlobals) - sizeof(GrafPtr))));
		UEnvironment::SetFeature( env_SupportsColor,
					(qdVersion > gestaltOriginalQD) );
		
		MacLeaks_Suspend();

		#if USE_MAC_DEBUG_LEAKS
		{ CForceInitLPeriodical	temp; }
		#endif

		RegisterPowerPlantClasses();

		MacLeaks_Resume();
		
		firstTime = FALSE;
	}

	LCommander::SetDefaultCommander( NULL );
	LPane::SetDefaultView( NULL );
}

	void
BevelBorder(const Rect &frame1)
{
	Rect 		frame = frame1;
	RGBColor	tempColor;
	RGBColor	saveForeColor;
	
	GetForeColor( &saveForeColor );

	PenNormal();
	
	tempColor = UGAColorRamp::GetBlackColor();
	RGBForeColor( &tempColor );
	OffsetRect( &frame, 1, 1);
	FrameRect( &frame);
	
	tempColor = UGAColorRamp::GetWhiteColor();
	RGBForeColor( &tempColor );
	MoveTo( frame.left+1, frame.top + 1 );
	LineTo( frame.left+1, frame.bottom - 3 );
	LineTo( frame.right-3, frame.bottom - 3 );
	LineTo( frame.right-3, frame.top + 1 );
	LineTo( frame.left+1, frame.top + 1 );
	
	tempColor = UGAColorRamp::GetColor( colorRamp_Gray5 );
	RGBForeColor( &tempColor );
	MoveTo( frame.right-4, frame.top + 2 );
	LineTo( frame.left+2, frame.top + 2 );
	LineTo( frame.left+2, frame.bottom - 4 );
	MoveTo( frame.left+1, frame.bottom - 2 );
	LineTo( frame.right-2, frame.bottom - 2 );
	LineTo( frame.right-2, frame.top + 1 );
	
	RGBForeColor( &saveForeColor );
}


	static MessageT
PGPUIWarningAlertInternal(
	ResIDT				dialogResID,
	ConstStr255Param	string)
{
	MessageT			dismissMessage = msg_Nothing;
	CPGPLibGrafPortView	*warnGrafPortView;
	GrafPtr				savePort;
	
	GetPort( &savePort );
	
	// Create standard Macintosh window and overlay PowerPlant onto it
	warnGrafPortView = CPGPLibGrafPortView::CreateDialog( dialogResID);
	if( IsntNull( warnGrafPortView ) )
	{
		DialogRef		warnDialog;
		ModalFilterUPP	filterUPP;
		short			itemHit;
		LGACaption *	caption	= NULL;
		
		warnDialog = warnGrafPortView->GetDialog();
		
		SetPort( warnDialog );
		
		if ( IsntNull( string ) )
		{
			#define kMessagePaneID	'cMSG'
			
			caption	=
				(LGACaption *) warnGrafPortView->FindPaneByID( kMessagePaneID );
			if ( IsntNull( caption ) )
			{
				caption->SetDescriptor( string );
			}
		}
		
		SysBeep( 1 );
		ShowWindow( warnDialog );
		SelectWindow( warnDialog );
		
		filterUPP = NewModalFilterProc( CPGPLibGrafPortView::ModalFilterProc );
		
		itemHit = 0;
		while( itemHit == 0 )
		{
			ModalDialog( filterUPP, &itemHit );
		}

		dismissMessage = warnGrafPortView->GetDismissMessage();
		
		DisposeRoutineDescriptor( filterUPP );

		delete warnGrafPortView;
		DisposeDialog( warnDialog );
	}
	else
	{
		pgpDebugMsg( "PGPUIWarningAlert(): Failed to get dialog" );
		dismissMessage = msg_Nothing;
	}
	
	SetPort( savePort );

	return( dismissMessage );
}


	MessageT
PGPUIWarningAlert(ResIDT dialogResID)
{
	return( PGPUIWarningAlertInternal( dialogResID, NULL ) );
}



	MessageT
PGPUIWarningAlert(
	ResIDT				dialogResID,
	ConstStr255Param	baseString,
	ConstStr255Param	str1,
	ConstStr255Param	str2,
	ConstStr255Param	str3 )
{
	Str255		msg;
	
	InsertPStrings( baseString, str1, str2, str3, msg );
	
	return( PGPUIWarningAlertInternal( dialogResID, msg ) );
}


	
	MessageT
PGPUIWarningAlert(
	ResIDT				dialogResID,
	ResID				stringListResID,
	short				stringListIndex,
	ConstStr255Param	str1,
	ConstStr255Param	str2 ,
	ConstStr255Param	str3 )
{
	Str255		baseString;
	Str255		msg;
	
	GetIndString( baseString, stringListResID, stringListIndex );
	InsertPStrings( baseString, str1, str2, str3, msg );
	
	return( PGPUIWarningAlertInternal( dialogResID, msg ) );
}
















