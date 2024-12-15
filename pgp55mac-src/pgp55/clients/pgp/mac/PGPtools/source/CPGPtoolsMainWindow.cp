/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include "CPGPtools.h"
#include "CPGPtoolsMainWindow.h"
#include "PGPtoolsPreferences.h"

const PaneIDT	kSmallEncryptReceiverPaneID			= 'sEnc';
const PaneIDT	kSmallSignReceiverPaneID			= 'sSig';
const PaneIDT	kSmallEncryptSignReceiverPaneID		= 'sEnS';
const PaneIDT	kSmallDecryptVerifyReceiverPaneID	= 'sDec';
const PaneIDT	kSmallWipeReceiverPaneID			= 'sWip';

const PaneIDT	kLargeEncryptReceiverPaneID			= 'dEnc';
const PaneIDT	kLargeSignReceiverPaneID			= 'dSig';
const PaneIDT	kLargeEncryptSignReceiverPaneID		= 'dEnS';
const PaneIDT	kLargeDecryptVerifyReceiverPaneID	= 'dDec';
const PaneIDT	kLargeWipeReceiverPaneID			= 'dWip';

CPGPtoolsMainWindow::CPGPtoolsMainWindow(LStream *inStream)
	: CPGPtoolsWindow(inStream)
{
	mIsSmallSize = false;
}

CPGPtoolsMainWindow::~CPGPtoolsMainWindow(void)
{
}

	void
CPGPtoolsMainWindow::FinishCreateSelf(void)
{
	inherited::FinishCreateSelf();
	
	if( gPreferences.smallMainWindow )
	{
		DoSetZoom( true );
	}
	
	UpdateButtonsState();
}

	void
CPGPtoolsMainWindow::DoSetZoom(Boolean inZoomToStdState)
{
	Rect	minMaxSize;
	short	newHeight;
	short	newWidth;
	
	#pragma unused (inZoomToStdState)
	
	GetMinMaxSize( minMaxSize );
	
	// Always toggle the zoom state
	
	if( mIsSmallSize )
	{
		newHeight = minMaxSize.bottom;
		newWidth = minMaxSize.right;

		FindPaneByID( 'SBox' )->Hide();

		SizeWindow( mMacWindowP, newWidth, newHeight, false );
		ResizeFrameTo( newWidth, newHeight, true );

		FindPaneByID( 'LBox' )->Show();
	}
	else
	{
		newHeight = minMaxSize.top;
		newWidth = minMaxSize.left;
		
		FindPaneByID( 'LBox' )->Hide();
		
		SizeWindow( mMacWindowP, newWidth, newHeight, false );
		ResizeFrameTo( newWidth, newHeight, true );

		if( ! WindowOnVisibleDevice() )
		{
			Point	newPos;
			
			newPos.v = newPos.h = 100;
			
			DoSetPosition( newPos );
		}
		
		FindPaneByID( 'SBox' )->Show();
	}

	mIsSmallSize = ! mIsSmallSize;
	
	gPreferences.smallMainWindow = mIsSmallSize;
}

	void
CPGPtoolsMainWindow::UpdateButtonsState(void)
{
	if( CPGPtools::AppCanDoOperation( kPGPtoolsEncryptOperation ) )
	{
		FindPaneByID( kSmallEncryptReceiverPaneID )->Enable();
		FindPaneByID( kLargeEncryptReceiverPaneID )->Enable();
	}
	else
	{
		FindPaneByID( kSmallEncryptReceiverPaneID )->Disable();
		FindPaneByID( kLargeEncryptReceiverPaneID )->Disable();
	}

	if( CPGPtools::AppCanDoOperation( kPGPtoolsSignOperation ) )
	{
		FindPaneByID( kSmallSignReceiverPaneID )->Enable();
		FindPaneByID( kLargeSignReceiverPaneID )->Enable();
	}
	else
	{
		FindPaneByID( kSmallSignReceiverPaneID )->Disable();
		FindPaneByID( kLargeSignReceiverPaneID )->Disable();
	}

	if( CPGPtools::AppCanDoOperation( kPGPtoolsEncryptSignOperation ) )
	{
		FindPaneByID( kSmallEncryptSignReceiverPaneID )->Enable();
		FindPaneByID( kLargeEncryptSignReceiverPaneID )->Enable();
	}
	else
	{
		FindPaneByID( kSmallEncryptSignReceiverPaneID )->Disable();
		FindPaneByID( kLargeEncryptSignReceiverPaneID )->Disable();
	}

	if( CPGPtools::AppCanDoOperation( kPGPtoolsDecryptVerifyOperation ) )
	{
		FindPaneByID( kSmallDecryptVerifyReceiverPaneID )->Enable();
		FindPaneByID( kLargeDecryptVerifyReceiverPaneID )->Enable();
	}
	else
	{
		FindPaneByID( kSmallDecryptVerifyReceiverPaneID )->Disable();
		FindPaneByID( kLargeDecryptVerifyReceiverPaneID )->Disable();
	}

	if( CPGPtools::AppCanDoOperation( kPGPtoolsWipeOperation ) )
	{
		FindPaneByID( kSmallWipeReceiverPaneID )->Enable();
		FindPaneByID( kLargeWipeReceiverPaneID )->Enable();
	}
	else
	{
		FindPaneByID( kSmallWipeReceiverPaneID )->Disable();
		FindPaneByID( kLargeWipeReceiverPaneID )->Disable();
	}
}
