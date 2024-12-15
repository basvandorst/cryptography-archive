/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CWhiteList.h,v 1.3 1997/08/20 12:35:45 wprice Exp $
____________________________________________________________________________*/
#pragma once

const MessageT	kWLCellDoubleClickedMessageID	= 'cdWL';
const MessageT	kWLSelectionChangedMessageID	= 'scWL';


class CWhiteList	:	public LTableView,
						public LBroadcaster
{
public:
	enum { class_ID = 'whTV' };
	static CWhiteList*	CreateFromStream(LStream *inStream);
	
							CWhiteList(LStream *inStream);
	void					DrawSelf();
	void					DrawCell(
								const STableCell	&inCell,
								const Rect			&inLocalRect);
	void					ClickCell(
								const STableCell&		inCell,
								const SMouseDownEvent&	inMouseDown);
	void					SelectionChanged();
};

