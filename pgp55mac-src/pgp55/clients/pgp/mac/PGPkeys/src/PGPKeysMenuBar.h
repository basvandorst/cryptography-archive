/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPKeysMenuBar.h,v 1.4 1997/10/11 12:39:49 wprice Exp $
____________________________________________________________________________*/
#include "LMenuBar.h"

class	PGPKeysMenuBar:	public	LMenuBar
{
public:
						PGPKeysMenuBar(
								ResIDT				inMBARid);
								
	virtual CommandT	MenuCommandSelection(
								const EventRecord	&inMouseEvent,
								Int32				&outMenuChoice) const;

			CommandT	ModifyMenuCommand( 
								CommandT menuCmd, 
								const EventRecord& inMacEvent ) const;
	virtual	CommandT	FindCommand(
								ResIDT				inMENUid,
								Int16				inItem) const;
};
