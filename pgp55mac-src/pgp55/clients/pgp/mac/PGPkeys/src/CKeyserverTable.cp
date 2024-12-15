/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyserverTable.cp,v 1.3 1997/08/20 12:35:42 wprice Exp $
____________________________________________________________________________*/

#include "CKeyserverTable.h"
#include "CKeyTable.h"
#include "pgpKeyServerPrefs.h"
#include "ResourceConstants.h"

#include "MacStrings.h"


const Int16 kColumnWidth		= 349;
const Int16	kRowHeight			= 18;
const Int16 kServerNameWidth	= 300;

CKeyserverTable::CKeyserverTable(
	LStream	*inStream)
		:	LHierarchyTable(inStream)
{
	mTableGeometry	= new LTableMonoGeometry(this,	kColumnWidth,
													kRowHeight);
	mTableSelector	= new LTableSingleSelector(this);
	mTableStorage	= new LTableArrayStorage(this,
		sizeof(KeyServerTableEntry));
}

void
CKeyserverTable::FinishCreateSelf()
{
	InsertCols(1, 1, NULL, 0, TRUE);
}

CKeyserverTable::~CKeyserverTable()
{
}

	void
CKeyserverTable::DrawCell(
	const STableCell	&inCell,
	const Rect			&inLocalRect)
{
	Rect		sortColumnRect,
				iconRect;
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);
	Rect		frame;
	Str255		str;
	char		cstr[256];

	CalcLocalFrameRect( frame );
	{
		StDeviceLoop devLoop(frame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth))
		{
			if(depth >= 8)
			{
				// Draw Copland-like background for cells
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
				::PaintRect(&inLocalRect);
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
				sortColumnRect = inLocalRect;
				sortColumnRect.right = sortColumnRect.left + 
						kServerNameWidth;
				sortColumnRect.left = inLocalRect.left + kDropFlagSlop;
				::PaintRect(&sortColumnRect);
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
				::MoveTo(inLocalRect.left, inLocalRect.bottom - 1);
				::LineTo(inLocalRect.right - 1, inLocalRect.bottom - 1);
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			else
			{
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
				::EraseRect(&inLocalRect);
			}
			
			DrawDropFlag(inCell, woRow);
			
			STableCell			woCell(woRow, inCell.col);
			Uint32				dataSize = sizeof(KeyServerTableEntry);
			KeyServerTableEntry	serverEntry;
			
			GetCellData(woCell, &serverEntry, dataSize);
			// draw the cell here
			if(serverEntry.isDomain)
			{
				CKeyTable::DrawIcon(kDomainIconID, inLocalRect, 1);
				CopyCString(serverEntry.ks.domain, cstr);
				CToPString(cstr, str);
			}
			else
			{
				CKeyTable::DrawIcon(kKeyServerIconID, inLocalRect, 2);
				CopyCString(serverEntry.ks.serverURL, cstr);
				CToPString(cstr, str);
			}
			::TextFont(geneva);
			::TextSize(9);
			if(!serverEntry.isDomain &&
				IsKeyServerDefault(serverEntry.ks.flags))
				::TextFace(bold);
			else
				::TextFace(0);
			::TruncString(	kServerNameWidth -
							kDropFlagSlop - kLeftIndent -
							(serverEntry.isDomain ? 0 : kLevelIndent) -
							kIconWidth - kLeftBorder,
							str, truncMiddle);
			::MoveTo(inLocalRect.left + kDropFlagSlop + kLeftIndent +
							(serverEntry.isDomain ? 0 : kLevelIndent) +
							kIconWidth + kLeftBorder,
							inLocalRect.bottom - kBottomBorder);
			::DrawString(str);
			iconRect.left	=	inLocalRect.left + kServerNameWidth +
								kLevelIndent;
			iconRect.top	=	inLocalRect.top + 2;
			iconRect.right 	= 	iconRect.left + kIconWidth;
			iconRect.bottom = 	iconRect.top + kIconHeight;
			
			if(!serverEntry.isDomain)
				::PlotIconID(&iconRect, atNone, ttNone,
					IsKeyServerListed(serverEntry.ks.flags) ?
					kValidKeyIconID : kKeyAttrOffIconID);
		}
	}
}

	void
CKeyserverTable::ClickCell(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
{
	if(GetClickCount() == 2)
		BroadcastMessage(kKSTCellDoubleClickedMessageID,
							(void *)inCell.row);
}

	void
CKeyserverTable::HiliteCellActively(
	const STableCell	&inCell,
	Boolean				/*inHilite*/)
{
	Rect	cellFrame;
	
	if(GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		StDeviceLoop	devLoop(cellFrame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth))
		{
			StColorPenState saveColorPen;   // Preserve color & pen state
			StColorPenState::Normalize();
			if(depth >= 8)
			{
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			else
			{
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			cellFrame.right = cellFrame.left + kServerNameWidth;
	        UDrawingUtils::SetHiliteModeOn();
			::InvertRect(&cellFrame);
		}
	}
}

	void
CKeyserverTable::HiliteCellInactively(
	const STableCell	&inCell,
	Boolean			 	/*inHilite*/)
{
	Rect	cellFrame;
	
	if(GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		StDeviceLoop	devLoop(cellFrame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth))
		{
	        StColorPenState saveColorPen;   // Preserve color & pen state
	        StColorPenState::Normalize();
	        if(depth >= 8)
	        {
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			else
			{
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			::PenMode(srcXor);
			cellFrame.right = cellFrame.left + kServerNameWidth;
			UDrawingUtils::SetHiliteModeOn();
			::FrameRect(&cellFrame);
		}
	}
}

	void
CKeyserverTable::DrawSelf()
{
	Rect frame;

	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
	::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
	CalcLocalFrameRect(frame);
	{
		StDeviceLoop	devLoop(frame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth))
		{
			if(depth > 1)
				::PaintRect(&frame);
			else
				::EraseRect(&frame);
		}
	}
	LTableView::DrawSelf();
}

	void
CKeyserverTable::SelectionChanged()
{
	BroadcastMessage(kKSTSelectionChangedMessageID, NULL);
}

