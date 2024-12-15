/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CWhiteList.cp,v 1.3 1997/08/20 12:35:44 wprice Exp $
____________________________________________________________________________*/
#include "CWhiteList.h"


CWhiteList*
CWhiteList::CreateFromStream(
	LStream	*inStream)
{
	return (new CWhiteList(inStream));
}


CWhiteList::CWhiteList(
	LStream	*inStream)
		: LTableView(inStream)
{
	mTableGeometry = new LTableMonoGeometry(this, mFrameSize.width, 12);
	mTableSelector = new LTableSingleSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(Str31));
}

	void
CWhiteList::DrawSelf()
{
	Rect frame;

	CalcLocalFrameRect(frame);
	::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
	::EraseRect(&frame);
	LTableView::DrawSelf();
}

	void
CWhiteList::DrawCell(
	const STableCell	&inCell,
	const Rect			&inLocalRect)
{
	Uint32			dataSize = sizeof(Str31);
	Str31			name;
	
	GetCellData(inCell, name, dataSize);
	::ForeColor(blackColor);
	::BackColor(whiteColor);
	::TextSize(9);
	::TextFont(1);
	::TextFace(0);
	::MoveTo(inLocalRect.left + 4, inLocalRect.bottom - 2);
	::DrawString(name);
}

	void
CWhiteList::ClickCell(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
{
	if(GetClickCount() == 2)
		BroadcastMessage(kWLCellDoubleClickedMessageID,
							(void *)inCell.row);
}

	void
CWhiteList::SelectionChanged()
{
	BroadcastMessage(kWLSelectionChangedMessageID, NULL);
}

