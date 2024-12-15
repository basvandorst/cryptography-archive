/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
**
** This is an implementation of StDialogHandler which does not enable
** the About menu command
*/

#include "CPGPStDialogHandler.h"

CPGPStDialogHandler::CPGPStDialogHandler(
	ResIDT		inDialogResID,			// 'PPob' and 'RidL' Resource ID
	LCommander	*inSuper)
		: StDialogHandler( inDialogResID, inSuper )
{
}

CPGPStDialogHandler::~CPGPStDialogHandler(void)
{
}

	void
CPGPStDialogHandler::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean&	outUsesMark,
	Char16&		outMark,
	Str255		outName)
{
	#pragma unused( inCommand, outUsesMark, outMark, outName )
	
	// Don't enable any commands at all. StDialogHandler enables
	// the About command which is generally an error for most movable,
	// modal dialogs. This function purposely does not
	// call the inherited FindCommandStatus, thereby suppressing
	// commands that are handled by SuperCommanders. Only those
	// commands enabled by SubCommanders will be active.
	//
	// This is usually what you want for a moveable modal dialog.
	// Commands such as "New", "Open" and "Quit" that are handled
	// by the Applcation are disabled, but items within the dialog
	// can enable commands. For example, an EditField would enable
	// items in the "Edit" menu.
		
	outEnabled = FALSE;
}
