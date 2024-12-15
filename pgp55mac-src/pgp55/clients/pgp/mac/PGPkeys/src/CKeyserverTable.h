/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyserverTable.h,v 1.3 1997/08/20 12:35:43 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpKeyServerPrefs.h"

typedef struct KeyServerTableEntry
{
	Boolean				isDomain;
	PGPKeyServerEntry	ks;
} KeyServerTableEntry;

const MessageT	kKSTCellDoubleClickedMessageID	= 'ksDC';
const MessageT	kKSTSelectionChangedMessageID	= 'ksSC';

class	CKeyserverTable	:	public LHierarchyTable,
							public LBroadcaster
{
public:
	enum { class_ID = 'SPht' };
						CKeyserverTable(LStream *inStream);
	virtual				~CKeyserverTable();
	void				FinishCreateSelf();
	
	void				SelectionChanged();
	virtual void		DrawSelf();
	virtual void		DrawCell(
								const STableCell		&inCell,
								const Rect				&inLocalRect);
	virtual void		ClickCell(
								const STableCell&		inCell,
								const SMouseDownEvent&	inMouseDown);
	virtual void		HiliteCellActively(
								const STableCell	&inCell,
								Boolean				inHilite);
	virtual void		HiliteCellInactively(
								const STableCell	&inCell,
								Boolean				inHilite);
private:
		
	enum	{
				kStringListID		= 1011
			};
};

