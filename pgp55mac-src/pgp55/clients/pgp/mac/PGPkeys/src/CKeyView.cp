/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyView.cp,v 1.6 1997/08/31 12:52:20 wprice Exp $
____________________________________________________________________________*/
#include "CKeyView.h"
#include "CKeyTable.h"
#include "CKeyTableLabels.h"
#include "CGAScroller.h"


CKeyView::CKeyView()
{

}

CKeyView::CKeyView(
	LStream	*inStream)
		: LView(inStream)
{
}

CKeyView::~CKeyView()
{

}

	void
CKeyView::FinishCreateSelf()
{
	LView::FinishCreateSelf();
	
	mKeyTable =			(CKeyTable *)		FindPaneByID(kKeyTable);
	pgpAssertAddrValid(mKeyTable,			VoidAlign);
	mKeyTableLabels =	(CKeyTableLabels *)	FindPaneByID(kKeyTableLabels);
	pgpAssertAddrValid(mKeyTableLabels,	VoidAlign);
	mKeyScroller =		(CGAScroller *)		FindPaneByID(kKeyScroller);
	pgpAssertAddrValid(mKeyScroller,	VoidAlign);
	mKeyTableLabels->SetKeyTable(mKeyTable);
	mKeyTable->SetKeyLabels(mKeyTableLabels);
}

	void
CKeyView::SetKeyDBInfo(	PGPKeySetRef	keySet,
						Boolean			writable,
						Boolean			defaultRings )
{
	pgpAssertAddrValid(keySet, PGPKeySetRef);
	mKeyTable->SetKeyDBInfo(keySet, writable, defaultRings);
}

	void
CKeyView::Empty()
{
	mKeyTable->Empty();
}

	void
CKeyView::DrawSelf()
{
	Rect	frame;
	
	LView::DrawSelf();
	FocusDraw();
	CalcLocalFrameRect(frame);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
	::MoveTo(frame.left, frame.bottom-1);
	::LineTo(frame.left, frame.top);
	::LineTo(frame.right, frame.top);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::MoveTo(frame.left+1, frame.bottom-1);
	::LineTo(frame.right - 16, frame.bottom-1);
	::LineTo(frame.right - 16, frame.bottom - 16);
	::LineTo(frame.right-1, frame.bottom - 16);
	::LineTo(frame.right-1, frame.top+1);
}

	void
CKeyView::CloseKeys()
{
	mKeyTable->Reset1();
}

	PGPKeySetRef
CKeyView::GetKeySet()
{
	return mKeyTable->GetKeySet();
}

	void
CKeyView::ImportKeysFromHandle(Handle data)
{
	mKeyTable->ImportKeysFromHandle(data);
}

