/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: PGPKeysMenuBar.h,v 1.5 1999/03/10 02:52:41 heller Exp $____________________________________________________________________________*/#include "LMenuBar.h"class	PGPKeysMenuBar:	public	LMenuBar{public:						PGPKeysMenuBar(								ResIDT				inMBARid);									virtual CommandT	MenuCommandSelection(								const EventRecord	&inMouseEvent,								Int32				&outMenuChoice) const;			CommandT	ModifyMenuCommand( 								CommandT menuCmd, 								const EventRecord& inMacEvent ) const;	virtual	CommandT	FindCommand(								ResIDT				inMENUid,								Int16				inItem) const;};