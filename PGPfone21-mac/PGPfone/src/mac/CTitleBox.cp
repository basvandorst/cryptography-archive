// ===========================================================================//	File:					CTitleBox.cp//  Version:				1.1 - Dec 5, 1994////	You may use the source code included in this archive in any application (commercial, //	shareware, freeware, postcardware, etc) when delivered an executable Macintosh //	application. You must attribute use of this source code in your copyright notice. //	You may not sell this source code in any form. This source code may be placed on //	publicly accessable archive sites and source code disks. It may not be placed on //	for profit archive sites and source code disks without the permission of the author, //	Mike Shields.//	//	    This source code is distributed in the hope that it will be useful,//	    but WITHOUT ANY WARRANTY; without even the implied warranty of//	    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.// ===========================================================================//	CTitleBox.h				<- double-click + Command-D to see class declaration////// CTitleBox is my version of the typical "box around some items with a title"; // In MacApp speak, a cluster. This is derived from LView so it can have subpanes. // It does no event handling itself.//#include "CTitleBox.h"#include <TextEdit.h>#include <LStream.h>#include <UEnvironment.h>#include <UTextTraits.h>#include <UDrawingState.h>#include "PGPFoneUtils.h"#include "PGPFMacUtils.h"// ---------------------------------------------------------------------------//	CreateTitleBoxStream// ---------------------------------------------------------------------------//	Create a new TitleBox object from the data in a StreamCTitleBox* CTitleBox::CreateTitleBoxStream(LStream *inStream){	return (new CTitleBox(inStream));}// ---------------------------------------------------------------------------//	CTitleBox// ---------------------------------------------------------------------------//	Default ConstructorCTitleBox::CTitleBox(){	mTitle[0] = 0;	mTxtrID = 0;	mOffsetRect.top = 0;	mOffsetRect.left = 0;	mOffsetRect.bottom = 0;	mOffsetRect.right = 0;}// ---------------------------------------------------------------------------//	CTitleBox(const CTitleBox&)// ---------------------------------------------------------------------------//	Copy ConstructorCTitleBox::CTitleBox(const CTitleBox &inOriginal)		: LView(inOriginal){	Str255	theTitle;				// Copy TitleBox title	Rect	theRect;		SetDescriptor(inOriginal.GetDescriptor(theTitle));									// Copy TextTraits ID	SetTextTraitsID(inOriginal.GetTextTraitsID());	inOriginal.GetOffsetRect(&theRect);	SetOffsetRect(theRect);}// ---------------------------------------------------------------------------//	CTitleBox(SPaneInfo&, Str255, ResIDT)// ---------------------------------------------------------------------------//	Construct from parameters. Use this constructor to create a TitleBox//	from runtime data.CTitleBox::CTitleBox(const SPaneInfo &inPaneInfo, const SViewInfo &inViewInfo, 					Str255 inString, ResIDT inTextTraitsID, Rect &inOffsetRect)		: LView(inPaneInfo, inViewInfo){	pstrcpy(mTitle, inString);	mTxtrID = inTextTraitsID;	BlockMoveData(&inOffsetRect, &mOffsetRect, sizeof(Rect));}	// ---------------------------------------------------------------------------//	CTitleBox(LStream*)// ---------------------------------------------------------------------------//	Construct from data in a StreamCTitleBox::CTitleBox(LStream *inStream)		: LView(inStream){	inStream->ReadPString(mTitle);	inStream->ReadData(&mTxtrID, sizeof(ResIDT));	inStream->ReadData(&mOffsetRect, sizeof(Rect));}// ---------------------------------------------------------------------------//	~CTitleBox// ---------------------------------------------------------------------------//	DestructorCTitleBox::~CTitleBox(){}// ---------------------------------------------------------------------------//	GetDescriptor// ---------------------------------------------------------------------------//	Return title of a TitleBox as a stringStringPtr CTitleBox::GetDescriptor(Str255 /*outDescriptor*/) const{	return NIL;//++++++++++++}// ---------------------------------------------------------------------------//	SetDescriptor// ---------------------------------------------------------------------------//	Set title of a TitleBox from a stringvoid CTitleBox::SetDescriptor(ConstStr255Param inDescriptor){	pstrcpy(mTitle, (uchar *) inDescriptor);}// ---------------------------------------------------------------------------//	GetTextTraitsID// ---------------------------------------------------------------------------//	Get the text traits to use for the titleResIDT CTitleBox::GetTextTraitsID() const{	return mTxtrID;}// ---------------------------------------------------------------------------//	SetTextTraitsID// ---------------------------------------------------------------------------//	Set the text traits to use for the titlevoid CTitleBox::SetTextTraitsID(ResIDT inTxtrID){	mTxtrID = inTxtrID;}// ---------------------------------------------------------------------------//	GetOffsetRect// ---------------------------------------------------------------------------//	Get the offset rect used to draw the boxvoid CTitleBox::GetOffsetRect(Rect *outOffsetRect) const{	BlockMoveData(&mOffsetRect, outOffsetRect, sizeof(Rect));}// ---------------------------------------------------------------------------//	SetOffsetRect// ---------------------------------------------------------------------------//	Set the offset rect used to draw the boxvoid CTitleBox::SetOffsetRect(Rect &inOffsetRect){	BlockMoveData(&inOffsetRect, &mOffsetRect, sizeof(Rect));}// ---------------------------------------------------------------------------//	DrawSelf// ---------------------------------------------------------------------------//	Draw the TitleBoxvoid CTitleBox::DrawSelf(){	Rect			frame;	Rect			textRect;	Int16			justification = UTextTraits::SetPortTextTraits(mTxtrID);	Int16			textWidth;	Int16			textHeight;		FontInfo		theFInfo;								FocusDraw();	CalcLocalFrameRect(frame);	::PenNormal();	if(mTitle[0] > 0)	{		// Get the text width and add in some slop for a border.		textWidth = ::TextWidth((Ptr)mTitle, 1, mTitle[0]) + 4;			// calculate the text height so we can draw the box top in the correct place		::GetFontInfo(&theFInfo);		textHeight = theFInfo.ascent + theFInfo.descent + theFInfo.leading;				textRect.top = frame.top;		textRect.left = frame.left;		textRect.bottom = frame.bottom;		textRect.right = frame.right;				// draw the top of the box throught the center of the text		frame.top += textHeight / 2;			// The text justifiction of the Txtr resource determines where the text is drawn.		switch ( justification )		{			case teFlushDefault:			case teFlushLeft:				textRect.left += 8;				break;						case teCenter:				textRect.left += ((textRect.right - textRect.left - textWidth) / 2);				break;							case teFlushRight:				textRect.left = textRect.right - textWidth - 8;				break;						default:				textRect.left += 8;				break;		}				// adjust the text box so we draw it correctly.		textRect.right = textRect.left + textWidth;		textRect.bottom = textRect.top + textHeight;	}	// Offset where we draw the box. This is also used to adjust for alignment problems	// with the text height.  In otherwords, the top of the drawn box will be adjusted	// from the calculated center of the text.	frame.top += mOffsetRect.top;	frame.left += mOffsetRect.left;	frame.bottom += mOffsetRect.bottom;	frame.right += mOffsetRect.right;	if(UEnvironment::HasFeature(env_SupportsColor))	{		GrayBorder(frame);		ApplyForeAndBackColors();	}	else		::FrameRect(&frame);	if (mTitle[0] > 0)		::TETextBox((Ptr)mTitle+1, mTitle[0], &textRect, teCenter);}