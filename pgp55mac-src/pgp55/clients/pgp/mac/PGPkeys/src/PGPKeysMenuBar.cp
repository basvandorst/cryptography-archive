/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPKeysMenuBar.cp,v 1.4 1997/10/11 12:39:49 wprice Exp $
____________________________________________________________________________*/
#include "PGPKeysMenuBar.h"
#include "CPGPKeys.h"

#include "pgpMem.h"

PGPKeysMenuBar::PGPKeysMenuBar(ResIDT inMBARid)
	:LMenuBar(inMBARid)
{
}


	CommandT
PGPKeysMenuBar::MenuCommandSelection(
	const EventRecord	&inMouseEvent,
	Int32				&outMenuChoice) const
{
	outMenuChoice = ::MenuSelect(inMouseEvent.where);
	
	Int32	menuCmd = cmd_Nothing;
	if (HiWord(outMenuChoice) != 0) {
		menuCmd = FindCommand(HiWord(outMenuChoice), LoWord(outMenuChoice));
	}
	menuCmd = ModifyMenuCommand(menuCmd, inMouseEvent);
	
	return menuCmd;
}


	CommandT
PGPKeysMenuBar::ModifyMenuCommand(
	CommandT menuCmd, 
	const EventRecord& inMacEvent) const
{
	if(menuCmd == cmd_Clear)
	{
		if(IsntNull(inMacEvent.modifiers & optionKey))
			menuCmd = cmd_OptionClear;
	}
	else if (menuCmd == cmd_Revoke)
	{
		if(IsntNull(inMacEvent.modifiers & optionKey))
			menuCmd = cmd_OptionRevoke;
	}
	
	return menuCmd;
}

	CommandT
PGPKeysMenuBar::FindCommand(
	ResIDT				inMENUid,
	Int16				inItem) const
{
	// This fixes a bug in PowerPlant.  Since the ID of the
	// Help menu is a negative number, it is not correctly
	// viewed as a synthetic command after translation, so
	// we substitute a value for it here.
	
	if(inMENUid == kHMHelpMenuID)
		inMENUid = kSubstituteHelpMenuID;
	return LMenuBar::FindCommand(inMENUid, inItem);
}
