/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyTableLabels.h,v 1.4 1997/08/09 13:33:35 wprice Exp $
____________________________________________________________________________*/
#pragma once

class CKeyTable;

class	CKeyTableLabels	:	public LPane
{
public:
	enum { class_ID = 'keyL' };

						CKeyTableLabels(LStream *inStream);
	virtual				~CKeyTableLabels();
	void				DrawSelf();
	void				FinishCreateSelf();
	virtual void		ClickSelf(const SMouseDownEvent	&inMouseDown);

	void				SetKeyTable(CKeyTable *keyTable)
							{	mKeyTable = keyTable;	}
							
	void				NotifyScrolled(Int32 delta);
	
	void				ActivateSelf();
	void				DeactivateSelf();
private:
	Int16				FindColumnHitBy(Point loc);
	void				GetColumnRect(Int16 col, Rect &colRect);
	
	CKeyTable			*mKeyTable;
	Int32				mScrollDelta;
};

