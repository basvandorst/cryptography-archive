/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CKeyTable.h,v 1.8.8.1 1997/12/05 22:13:38 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "pgpKeys.h"
#include <LTableView.h>
#include <LBroadcaster.h>
#include <LCommander.h>

class CKeyTable :	public LTableView,
					public LBroadcaster,
					public LCommander {
public:
	enum 				{ class_ID = 'KeyT' };

						CKeyTable(LStream * inStream);
	virtual				~CKeyTable();
	
	virtual void		FindCommandStatus(
								CommandT			inCommand,
								Boolean				&outEnabled,
								Boolean				&outUsesMark,
								Char16				&outMark,
								Str255				outName);
	virtual Boolean		ObeyCommand(
								CommandT			inCommand,
								void				*ioParam = nil);
	virtual Boolean		HandleKeyPress(const EventRecord& inKeyEvent);

	void				SetTableInfo(PGPFilterRef inFilter,
								Boolean inMultiSelector);
	void				SetTableInfo(PGPKeySetRef inKeySet);
	void				GetSelectedKeys(Handle outHandle);
	void				GetSelectedKeys(PGPKeySetRef * outKeys);
	void				GetFirstSelectedKeyID(PGPKeyID * outKeyID,
								SInt32 * outAlgorithm = nil);
	void				GetFirstSelectedUserID(CString & outUserID);
	
protected:
	Boolean				mSelectAllEnabled;
	Boolean				mDisplayList;
	PGPFilterRef		mFilterRef;
	PGPKeyIterRef		mKeyIterRef;
	PGPKeyListRef		mKeyListRef;
		
	virtual void		HiliteCellActively(const STableCell & inCell,
								Boolean inHilite);
	virtual void		HiliteCellInactively(const STableCell & inCell,
								Boolean inHilite);
	
	virtual void		ClickSelf(const SMouseDownEvent & inMouseDown);
	
	virtual void		DrawSelf();
	virtual void		DrawCell(const STableCell & inCell,
								const Rect & inLocalRect);
	
	virtual void		ShowSelf();
	
	void				FreeAllStoredKeys();
};
