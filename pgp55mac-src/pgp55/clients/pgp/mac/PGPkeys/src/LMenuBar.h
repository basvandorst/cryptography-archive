// ===========================================================================
//	LMenuBar.h				 (C)1993-1996 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_LMenuBar
#define _H_LMenuBar
#pragma once

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#include <PP_Prefix.h>

#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __MENUS__
#include <Menus.h>
#endif

class	LMenu;

const ResIDT	InstallMenu_AtEnd	= 0;


class	LMenuBar {
public:
						LMenuBar(
								ResIDT				inMBARid);
	virtual 			~LMenuBar();
	
	virtual CommandT	MenuCommandSelection(
								const EventRecord	&inMouseEvent,
								Int32				&outMenuChoice) const;
	virtual Boolean		CouldBeKeyCommand(
								const EventRecord	&inKeyEvent) const;
	virtual CommandT	FindKeyCommand(
								const EventRecord	&inKeyEvent,
								Int32				&outMenuChoice) const;
	
	// **************************************************************
	// The following line declares the FindCommand function as virtual
	// which fixes a bug in PowerPlant -- the reason we use this
	// replacement file
	// **************************************************************
	virtual	CommandT	FindCommand(
								ResIDT				inMENUid,
								Int16				inItem) const;
			void		FindMenuItem(
								CommandT			inCommand,
								ResIDT				&outMENUid,
								MenuHandle			&outMenuHandle,
								Int16				&outItem);
			Boolean		FindNextCommand(
								Int16				&ioIndex,
								MenuHandle			&ioMenuHandle,
								LMenu*				&ioMenu,
								CommandT			&outCommand);
			
			void		InstallMenu(
								LMenu				*inMenu,
								ResIDT				inBeforeMENUid);
			void		RemoveMenu(
								LMenu				*inMenu);
			
			LMenu*		FetchMenu(
								ResIDT				inMENUid) const;
			Boolean		FindNextMenu(
								LMenu*				&ioMenu) const;
			
	static LMenuBar*	GetCurrentMenuBar()		{ return sMenuBar; }

protected:
	static	LMenuBar	*sMenuBar;
	
	LMenu				*mMenuListHead;
};


// ---------------------------------------------------------------------------
//	StUnhiliteMenu
// ---------------------------------------------------------------------------
//	Stack-based class to unhilite the currently selected menu title

class	StUnhiliteMenu {
public:
						StUnhiliteMenu() { }
						~StUnhiliteMenu()
						{
							::HiliteMenu(0);
						}
};

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif

#endif
