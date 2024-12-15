/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPopupList.cp,v 1.8.8.1 1997/12/05 22:14:08 mhw Exp $
____________________________________________________________________________*/
#include "CPopupList.h"

#include "MacFonts.h"
#include "MacStrings.h"
#include "OffscreenUtils.h"
#include "pgpMem.h"

#include "CPGPKeys.h"	// only used for "max" macro

const Int16 popColors[] =
	{
		//Enabled
		colorRamp_White,		//lefttop bkg
		colorRamp_Black,		//frame
		colorRamp_Gray5,		//first shadow
		colorRamp_Gray2,		//background
		colorRamp_Gray8,		//second(drkr) shadow
		colorRamp_Black,		//text and dropper
		//Pressed
		colorRamp_Gray10,
		colorRamp_Black,
		colorRamp_Gray8,
		colorRamp_Gray9,
		colorRamp_Gray7,
		colorRamp_White,
		//Disabled
		colorRamp_Gray1,
		colorRamp_Gray7,
		colorRamp_Gray5,
		colorRamp_Gray2,
		colorRamp_Gray9,
		colorRamp_Gray7,
	};

CPopupList::CPopupList(LStream	*inStream)
	: LControl(inStream)
{
	inStream->ReadData(&mTextTraitsID, sizeof(ResIDT));
	mValue		= 0;
	mEnabled	=
		mActive = triState_On;
	mNumItems	= 0;
	mItems		= (PopupListItem *)pgpAlloc(sizeof(PopupListItem));
}

CPopupList::~CPopupList()
{
	pgpFree(mItems);
}

	void
CPopupList::DrawSelf1(Int16 state)
{
	Rect		frame;
	FontInfo	fInfo;
	Int16		iinx,
				maxMaxWidth,
				sizeIndex,
				cbase,
				pdepth;
	RgnHandle	triangleRgn;
	Str255		itemString;
	
	if(FocusDraw())
	{
		CalcLocalFrameRect(frame);
		StColorPenState::Normalize();
		//ApplyForeAndBackColors();
		//::EraseRect(&frame);
		UTextTraits::SetPortTextTraits(mTextTraitsID);
		pdepth = PixelDepth();
		switch(state)
		{
			case 0:
				cbase = 0;
				break;
			case 1:
				cbase = 6;
				break;
			case 2:
				cbase = 12;
				break;
		}
		::GetFontInfo(&fInfo);
		maxMaxWidth = frame.right - frame.left - 6 - 15 - 22;
		for( mMaxWidth = iinx = 0 ; iinx < mNumItems ; iinx++ )
		{
			mMaxWidth = max(mMaxWidth, ::StringWidth(mItems[iinx].item));
		}
		if(mMaxWidth > maxMaxWidth)
			mMaxWidth = maxMaxWidth;
		mListRect.left =	frame.left;
		mListRect.top =		frame.top;
		mListRect.right =	frame.left + mMaxWidth + 6 + 15 + 22;
		mListRect.bottom =	frame.top +
							fInfo.ascent +
							fInfo.descent +
							fInfo.leading +
							( fInfo.ascent <= 10 ? 3 : 4 );
		if(mListRect.right > frame.right)
			mListRect.right = frame.right;
		if( pdepth > 1 )
			SetGrayFore(popColors[cbase+3]);
		else
			::ForeColor(whiteColor);
		::PaintRoundRect(&mListRect, 8, 8);
		SetGrayFore(popColors[cbase+1]);
		::FrameRoundRect(&mListRect, 8, 8);
		if( pdepth > 1 )
		{
			SetGrayFore(popColors[cbase]);
			//left shadow
			::MoveTo(mListRect.left+1,mListRect.top+2);
			::LineTo(mListRect.left+1,mListRect.bottom-3);
			//top shadow
			::MoveTo(mListRect.left+2,mListRect.top+1);
			::LineTo(mListRect.right-22,mListRect.top+1);
			//dropper white shadow
			::MoveTo(mListRect.right-3,mListRect.top+2);
			::LineTo(mListRect.right-20,mListRect.top+2);
			::LineTo(mListRect.right-20,mListRect.bottom-4);
			SetGrayFore(popColors[cbase+2]);
			//bottom shadow
			::MoveTo(mListRect.left+2,mListRect.bottom-2);
			::LineTo(mListRect.right-22,mListRect.bottom-2);
			//right shadow
			::LineTo(mListRect.right-22,mListRect.top+2);
			//dropper dark shadow
			::MoveTo(mListRect.right-19,mListRect.bottom-3);
			::LineTo(mListRect.right-4,mListRect.bottom-3);
			::MoveTo(mListRect.right-3,mListRect.bottom-4);
			::LineTo(mListRect.right-3,mListRect.top+3);
			SetGrayFore(popColors[cbase+4]);
			//dropper darkest shadow
			::MoveTo(mListRect.right-20,mListRect.bottom-2);
			::LineTo(mListRect.right-3,mListRect.bottom-2);
			::LineTo(mListRect.right-3,mListRect.bottom-3);
			::LineTo(mListRect.right-2,mListRect.bottom-3);
			::LineTo(mListRect.right-2,mListRect.top+3);
			SetGrayFore(popColors[cbase+5]);
			SetGrayBack(popColors[cbase+3]);
		}
		//dropper
		sizeIndex = fInfo.ascent / 2;
		
		triangleRgn = ::NewRgn();
		::OpenRgn();
			::MoveTo(mListRect.right-7, mListRect.top+2+sizeIndex);
			sizeIndex -= 1;
			::Line(-2*sizeIndex,0);
			::Line(sizeIndex,sizeIndex);
			::Line(sizeIndex,-sizeIndex);
			::CloseRgn(triangleRgn);
			::PaintRgn(triangleRgn);
		::DisposeRgn(triangleRgn);

		if( mNumItems > mValue )
		{
			::MoveTo(	mListRect.left+9,
						mListRect.top + fInfo.ascent + 2 -
						( fInfo.ascent <= 10 ? 1 : 0 ));
			CopyPString(mItems[mValue].item, itemString);
			if(::StringWidth(itemString) > mMaxWidth)
				::TextFace(condense);
			::TruncString(mMaxWidth, itemString, truncMiddle);
			::DrawString(itemString);
		}
		::TextFace(0);
		SetGrayFore(colorRamp_Black);
		SetGrayBack(colorRamp_White);
	}
}

	void
CPopupList::DrawSelf()
{
	if(IsVisible())
		DrawSelf1((mEnabled == triState_On && mActive == triState_On) ? 0 : 2);
}

	void
CPopupList::DrawItem(	Rect	trect,
						Int16	vertical,
						uchar	*name,
						Boolean	checked,
						Int16	ascent)
{
	Str255 itemString;
	
	if(checked)
	{
		if(ascent<=10)
			CopyPString("\p\xA5", itemString);
		else
			CopyPString("\p\x12", itemString);
		::MoveTo(trect.left+3, vertical);
		::DrawString(itemString);
	}
	if(PStringsAreEqual("\p-", name))
	{
		SetGrayFore(colorRamp_Gray7);
		::MoveTo(trect.left, vertical-2);
		::LineTo(trect.right-1, vertical-2);
	}
	else
	{
		::MoveTo(trect.left+16, vertical);
		CopyPString(name, itemString);
		if(::StringWidth(itemString) > mMaxWidth)
			::TextFace(condense);
		::TruncString(mMaxWidth, itemString, truncMiddle);
		::DrawString(itemString);
	}
	::TextFace(0);
}

	void
CPopupList::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	Rect		frame,
				poppedRect,
				ipoppedRect,
				targetRect,
				niRect;
	CGrafPort	port;
	GrafPtr		wmgrPort,
				savePort;
	RgnHandle	savedClip,
				grayRgn,
				savedGrayRgn;
	Point		oldMouse,
				curMouse,
				anchorPos;
	Int16		tFont,
				tSize,
				tLineHeight,
				pixDepth,
				iinx,
				baseLine,
				oldSelected,
				selected;
	long		rowBytes,
				size;
	FontInfo	fInfo;
	GDHandle	maxDevice,
				dev;
	char		*saveBits;
	OSErr		result;
	
	selected = -1;
	if(FocusDraw() && mNumItems)
	{
		CalcLocalFrameRect(frame);
		if(PtInRect(inMouseDown.whereLocal, &mListRect))
		{
			DrawSelf1(1);
			::GetFontInfo(&fInfo);
			tFont = qd.thePort->txFont;
			tSize = qd.thePort->txSize;
			tLineHeight =	fInfo.ascent + fInfo.descent +
							fInfo.leading + ( fInfo.ascent <= 10 ? 2 : 3 );
			savedClip = NewRgn();
			GetClip(savedClip);
			::GetPort(&savePort);
			anchorPos.h = mListRect.left;
			anchorPos.v = mListRect.bottom-1;
			::LocalToGlobal(&anchorPos);
			curMouse = inMouseDown.whereLocal;
			::LocalToGlobal(&curMouse);
			if(UEnvironment::HasFeature(env_SupportsColor))
				GetCWMgrPort((CGrafPtr *)&wmgrPort);
			else
				GetWMgrPort(&wmgrPort);
			poppedRect.top = poppedRect.left = 0;
			poppedRect.bottom = tLineHeight * mNumItems + 4;
			poppedRect.right = mListRect.right - mListRect.left;
			
			::OffsetRect(&poppedRect, anchorPos.h, anchorPos.v);
			::SetPort(wmgrPort);
			grayRgn = ::GetGrayRgn();
			savedGrayRgn = ::NewRgn();
			::CopyRgn(grayRgn, savedGrayRgn);
			::SetClip(savedGrayRgn);
			ipoppedRect = poppedRect;
			ipoppedRect.right--;	ipoppedRect.bottom--;
			maxDevice = GetMaxDevice(&poppedRect);
			if(!maxDevice)
				pixDepth = 1;
			else
				pixDepth = (*(*maxDevice)->gdPMap)->pixelSize;
			rowBytes = ((pixDepth*
						(poppedRect.right-poppedRect.left)+63)>>5)<<2;
			size = rowBytes*(long)(poppedRect.bottom-poppedRect.top);
			if(IsntNull(saveBits = (char *)pgpAlloc(size)))
			{
				StSystemFontState	fontState(tFont, tSize);
				
				result = PGPCreateOffscrn(	poppedRect,
											maxDevice,
											&port,
											&dev,
											saveBits,
											size);
				pgpAssert(IsntErr(result));
				PGPCopyToOffscrn(	(GrafPtr)&port,
									0,
									0,
									poppedRect,
									nil);
				::SetPort(wmgrPort);
				SetGrayBack(colorRamp_Gray2);
				::EraseRect(&ipoppedRect);
				SetGrayBack(colorRamp_White);
				::FrameRect(&ipoppedRect);
				SetGrayFore(colorRamp_Gray12);
				::MoveTo(ipoppedRect.left+2, ipoppedRect.bottom);
				::LineTo(ipoppedRect.right, ipoppedRect.bottom);
				::LineTo(ipoppedRect.right, ipoppedRect.top+2);
				::InsetRect(&ipoppedRect, 1, 1);
				if(pixDepth > 1)
				{
					SetGrayFore(colorRamp_White);
					::MoveTo(ipoppedRect.left,ipoppedRect.bottom-1);
					::LineTo(ipoppedRect.left,ipoppedRect.top);
					::LineTo(ipoppedRect.right-1,ipoppedRect.top);
					SetGrayFore(colorRamp_Gray6);
					::MoveTo(ipoppedRect.left+1,ipoppedRect.bottom-1);
					::LineTo(ipoppedRect.right-1,ipoppedRect.bottom-1);
					::LineTo(ipoppedRect.right-1,ipoppedRect.top+1);
					SetGrayFore(colorRamp_Black);
				}
				baseLine = ipoppedRect.top + fInfo.ascent +
							( fInfo.ascent <= 10 ? 1 : 2 );
				for( iinx = 0 ; iinx < mNumItems ; iinx++ )
				{
					SetGrayFore(colorRamp_Black);
					DrawItem(	ipoppedRect,
								baseLine,
								mItems[iinx].item,
								iinx == mValue,
								fInfo.ascent);
					baseLine += tLineHeight;
				}
				oldMouse = curMouse;
				while(::StillDown())
				{
					::GetMouse(&curMouse);
					::SetRect(	&targetRect,
									ipoppedRect.left,
									ipoppedRect.top+1,
									ipoppedRect.right,
									ipoppedRect.top + tLineHeight);
					oldSelected = -1;
					for( iinx=0 ; iinx < mNumItems ; iinx++ )
					{
						if(PtInRect(curMouse, &targetRect) &&
							!PStringsAreEqual(mItems[iinx].item, "\p-"))
						{
							if(selected != iinx)
							{
								if(selected >= 0)
									oldSelected = selected;
								niRect = targetRect;
								niRect.left++;	niRect.right--;
								SetGrayBack(colorRamp_Gray9);
								SetGrayFore(colorRamp_White);
								::EraseRect(&niRect);
								baseLine = targetRect.top +
											fInfo.ascent +
											( fInfo.ascent <= 10 ? 1 : 2 );
								DrawItem(	targetRect,
											baseLine,
											mItems[iinx].item,
											iinx == mValue,
											fInfo.ascent);
								
								SetGrayFore(colorRamp_Gray8);
								if(pixDepth>1)
								{
									::MoveTo(	targetRect.left,
												targetRect.top);
									::LineTo(	targetRect.left,
												targetRect.bottom-1);
									if(!iinx)
									{
										::MoveTo(	targetRect.left,
													targetRect.top-1);
										::LineTo(	targetRect.right-1,
													targetRect.top-1);
									}
									SetGrayFore(colorRamp_Gray12);
									::MoveTo(	targetRect.right-1,
												targetRect.top);
									::LineTo(	targetRect.right-1,
												targetRect.bottom-1);
									if(iinx == mNumItems-1)
									{
										::MoveTo(	targetRect.left+1,
													targetRect.bottom);
										::LineTo(	targetRect.right-1,
													targetRect.bottom);
									}
								}
								SetGrayBack(colorRamp_White);
								SetGrayFore(colorRamp_Black);
								selected = iinx;
							}
							goto foundItem;
						}
						::OffsetRect(&targetRect, 0, tLineHeight);
					}
					oldSelected = selected;
					selected = -1;
				foundItem:
					if(oldSelected >= 0)
					{
						niRect = targetRect;
						niRect.left++;	niRect.right--;
						niRect.top = targetRect.top = ipoppedRect.top +
										tLineHeight * oldSelected + 1;
						niRect.bottom = targetRect.bottom =
									targetRect.top + tLineHeight - 1;
						SetGrayBack(colorRamp_Gray2);
						SetGrayFore(colorRamp_Black);
						::EraseRect(&niRect);
						baseLine = targetRect.top +
								fInfo.ascent +
								( fInfo.ascent <= 10 ? 1 : 2 );
						DrawItem(	targetRect,
									baseLine,
									mItems[oldSelected].item,
									oldSelected == mValue,
									fInfo.ascent);
						if(pixDepth > 1)
						{
							SetGrayFore(colorRamp_White);
							::MoveTo(	targetRect.left,
										targetRect.top);
							::LineTo(	targetRect.left,
										targetRect.bottom-1);
							if(!oldSelected)
							{
								::MoveTo(	targetRect.left,
											targetRect.top-1);
								::LineTo(	targetRect.right-1,
											targetRect.top-1);
							}
							SetGrayFore(colorRamp_Gray6);
							::MoveTo(	targetRect.right-1,
										targetRect.top);
							::LineTo(	targetRect.right-1,
										targetRect.bottom-1);
							if(oldSelected == mNumItems-1)
							{
								::MoveTo(	targetRect.left+1,
											targetRect.bottom);
								::LineTo(	targetRect.right-1,
											targetRect.bottom);
							}
						}
						SetGrayBack(colorRamp_White);
						SetGrayFore(colorRamp_Black);
					}
					//::YieldToAnyThread();
				}
				PGPCopyFromOffscrn(	(GrafPtr)&port, wmgrPort,
										0, 0,
										poppedRect,
										srcCopy, nil);
				PGPDisposeOffscrn(&port, dev);
				pgpFree(saveBits);
			}
			DisposeRgn(savedGrayRgn);
			SetClip(savedClip);
			DisposeRgn(savedClip);
			::SetPort(savePort);
			if(selected >= 0)
			{
				mValue = selected;
				BroadcastValueMessage();
			}
			DrawSelf1(0);
		}
	}
}

	Int32
CPopupList::GetValue() const
{
	return mValue;
}

	void
CPopupList::SetValue(
	Int32	inValue)
{
	mValue = inValue;
	DrawSelf();
}

	void
CPopupList::AddItem(ConstStringPtr name)
{
	PGPError	err;
	
	err = pgpRealloc(&mItems, sizeof(PopupListItem) * (mNumItems + 1));
	pgpAssertNoErr(err);
	if(IsntErr(err))
		CopyPStringMax(name, mItems[mNumItems++].item, 255);
}

	void
CPopupList::GetItem(Int16 item, Str255 name)
{
	pgpAssert(item < mNumItems);
	CopyPString(mItems[item].item, name);
}

	void
CPopupList::ClearItems()
{
	PGPError	err;
	
	mNumItems = 0;
	err = pgpRealloc(&mItems, sizeof(PopupListItem));
	pgpAssertNoErr(err);
}

	void
CPopupList::SetGrayFore(Int16 index)
{
	::RGBForeColor(&UGAColorRamp::GetColor(index));
}

	void
CPopupList::SetGrayBack(Int16 index)
{
	::RGBBackColor(&UGAColorRamp::GetColor(index));
}

	void
CPopupList::EnableSelf ()
{
	Draw ( nil );
}

	void
CPopupList::DisableSelf ()
{
	Draw ( nil );
}

	void
CPopupList::ActivateSelf ()
{
	if ( mActive == triState_On )
		Refresh ();
}

	void
CPopupList::DeactivateSelf ()
{
	if ( mActive == triState_Off || mActive == triState_Latent )
		Refresh ();
}

