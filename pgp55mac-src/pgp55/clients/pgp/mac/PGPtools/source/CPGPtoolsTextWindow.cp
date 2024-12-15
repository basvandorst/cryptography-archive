/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <LTextEdit.h>
#include <PP_Messages.h>

#include "CPGPtoolsTextWindow.h"

const PaneIDT	kScrollPaneID	= 4001;
const PaneIDT	kTextEditPaneID	= 4002;

CPGPtoolsTextWindow::CPGPtoolsTextWindow(LStream *inStream)
	: CPGPtoolsWindow(inStream)
{
}

CPGPtoolsTextWindow::~CPGPtoolsTextWindow(void)
{
}

	void
CPGPtoolsTextWindow::FinishCreateSelf(void)
{
	LTextEdit	*textEdit;
	
	inherited::FinishCreateSelf();

	textEdit = (LTextEdit *) FindPaneByID( kTextEditPaneID );
	pgpAssertAddrValid( textEdit, LTextEdit );
	
	TEAutoView( TRUE, textEdit->GetMacTEH() );
}


	OSStatus
CPGPtoolsTextWindow::SetText(LogicalAddress theText, UInt32 textLength)
{
	LTextEdit	*textEdit;
	
	pgpAssert( textLength <= max_Int16 );
	
	if( textLength > max_Int16 )
		textLength = max_Int16;
		
	textEdit = (LTextEdit *) FindPaneByID( kTextEditPaneID );
	pgpAssertAddrValid( textEdit, LTextEdit );
	
	textEdit->SetTextPtr( (char *) theText, textLength );
	
	return( noErr );
}

	void
CPGPtoolsTextWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
		case cmd_SelectAll:
			outEnabled = TRUE;
			break;
			
		default:
			CPGPtoolsWindow::FindCommandStatus(inCommand, outEnabled,
									outUsesMark, outMark, outName);
			break;
	}
}

	Boolean
CPGPtoolsTextWindow::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;
	
	switch (inCommand)
	{
		case cmd_SelectAll:
		{
			LTextEdit	*textEdit;
			
			textEdit = (LTextEdit *) FindPaneByID( kTextEditPaneID );
			pgpAssertAddrValid( textEdit, LTextEdit );

			SwitchTarget( textEdit );
			textEdit->SelectAll();
			break;
		}
		
		default:
			cmdHandled = CPGPtoolsWindow::ObeyCommand( inCommand, ioParam );
			break;
	}
	
	return cmdHandled;
}
