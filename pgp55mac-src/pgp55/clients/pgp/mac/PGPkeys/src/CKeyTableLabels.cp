/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyTableLabels.cp,v 1.8 1997/09/24 06:01:14 wprice Exp $
____________________________________________________________________________*/
#include "CKeyTableLabels.h"
#include "CKeyTable.h"

#define TYPEICONSLOP		16
#define LEFTMARGIN			4

CKeyTableLabels::CKeyTableLabels(LStream *inStream)
		: LPane(inStream)
{
	mKeyTable		= nil;
	mScrollDelta	= 0;
}

CKeyTableLabels::~CKeyTableLabels()
{
}

	void
CKeyTableLabels::FinishCreateSelf()
{	
}

	void
CKeyTableLabels::DrawSelf()
{
	Rect				frame,
						borderClip,
						textClip,
						colFrame;
	short				columnIndex;
	Str255				title;
	KeyTableColumnInfo	columnInfo;
	
	FocusDraw();
	CalcLocalFrameRect(frame);
	if(mActive == triState_On)
	{
		::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray3));
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray12));
	}
	else
	{
		::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray5));
	}
	::EraseRect(&frame);
	::FrameRect(&frame);
	::TextFont(geneva);
	::TextSize(9);
	::TextFace(0);
	
	borderClip = textClip = frame;
	::InsetRect(&textClip, 2, 2);
	::InsetRect(&borderClip, 1, 1);
		
	mKeyTable->GetKeyColumnInfo(&columnInfo);
	for(columnIndex = 0;columnIndex<columnInfo.numActiveColumns;
		columnIndex++)
	{
		GetColumnRect(columnIndex, colFrame);
		{
			StClipRgnState		clip(borderClip);
			
			if(mActive == triState_On)
			{
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
				::MoveTo(colFrame.left+1, colFrame.bottom-1);
				::LineTo(colFrame.left+1, colFrame.top+1);
				::LineTo(colFrame.right-1, colFrame.top+1);
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
				::MoveTo(colFrame.left+2, colFrame.bottom-1);
				::LineTo(colFrame.right-1, colFrame.bottom-1);
				::LineTo(colFrame.right-1, colFrame.top+2);
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray12));
			}
			else
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray5));
			::MoveTo(colFrame.right, colFrame.bottom-1);
			::LineTo(colFrame.right, colFrame.top+1);
		}
		{
			StClipRgnState		clip(textClip);
			
			::GetIndString(title, kColumnTitleStringsID, 1 + 
							columnInfo.columns[columnIndex]);
			::MoveTo(colFrame.left + LEFTMARGIN + (columnIndex == 0 ?
						kDropFlagSlop : 0), frame.top + 12);
			if(mActive == triState_On)
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			else
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
			::DrawString(title);
		}
	}
}

	Int16
CKeyTableLabels::FindColumnHitBy(Point loc)
{
	KeyTableColumnInfo	columnInfo;
	Int16				colIndex,
						pos;
	
	mKeyTable->GetKeyColumnInfo(&columnInfo);
	pos = -mScrollDelta;
	for(colIndex = 0; colIndex < columnInfo.numActiveColumns; colIndex++)
	{
		pos += columnInfo.columnWidths[columnInfo.columns[colIndex]];
		if(loc.h < pos)
			return colIndex;
	}
	return -1;
}

	void
CKeyTableLabels::GetColumnRect(Int16 col, Rect &colRect)
{
	KeyTableColumnInfo	columnInfo;
	Int16				pos,
						colIndex;
	Rect				localRect;
	
	CalcLocalFrameRect(localRect);
	mKeyTable->GetKeyColumnInfo(&columnInfo);
	pos = 0;
	for(colIndex = 0; colIndex < columnInfo.numActiveColumns; colIndex++)
	{
		if(colIndex == col)
		{
			colRect.top		= localRect.top;
			colRect.bottom	= localRect.bottom - 1;
			colRect.left	= localRect.left + pos;
			colRect.right	= colRect.left +
					columnInfo.columnWidths[columnInfo.columns[colIndex]];
			::OffsetRect(&colRect, -mScrollDelta, 0);
			break;
		}
		pos += columnInfo.columnWidths[columnInfo.columns[colIndex]];
	}
}

	void
CKeyTableLabels::ClickSelf(
	const SMouseDownEvent	&inMouseDown)
{
	RgnHandle			columnRgn;
	Rect				columnRect,
						limitRect,
						slopRect,
						ktRect;
	Int16				col,
						newCol;
	UInt32				dragDiff;
	Point				newPoint;
	
	FocusDraw();
	col = FindColumnHitBy(inMouseDown.whereLocal);
	if(col > 0)
	{
		GetColumnRect(col, columnRect);
		mKeyTable->CalcPortFrameRect(ktRect);
		columnRect.top ++;
		columnRect.bottom += ktRect.bottom - ktRect.top + 1;
		columnRect.right++;
		columnRgn = ::NewRgn();
		::RectRgn(columnRgn, &columnRect);
		CalcLocalFrameRect(limitRect);
		limitRect.right -= 16;
		slopRect = limitRect;
		::InsetRect(&slopRect, -32, -256);
		dragDiff = ::DragGrayRgn(columnRgn, inMouseDown.whereLocal,
						&limitRect, &slopRect, hAxisOnly,
						NULL);
		DisposeRgn(columnRgn);
		newPoint = inMouseDown.whereLocal;
		newPoint.h += dragDiff & 0xFFFF;
		newCol = FindColumnHitBy(newPoint);
		if((newCol != col) && (newCol >= 0))
		{
			mKeyTable->MoveColumn(col, newCol);
			Refresh();
		}
	}
}

	void
CKeyTableLabels::NotifyScrolled(Int32 delta)
{
	mScrollDelta += delta;
}

	void
CKeyTableLabels::ActivateSelf()
{
	if(mActive == triState_On)
		Refresh ();
}

	void
CKeyTableLabels::DeactivateSelf()
{
	if(mActive == triState_Off || mActive == triState_Latent)
		Refresh ();
}

