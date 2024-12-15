/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CRestorePortColorsAttachment.cp,v 1.2.10.1 1997/12/05 22:15:04 mhw Exp $
____________________________________________________________________________*/

#include "CRestorePortColorsAttachment.h"



CRestorePortColorsAttachment::CRestorePortColorsAttachment(
	Boolean	inForeColor,
	Boolean inBackColor,
	Boolean	inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost),
		mForeColor(inForeColor), mBackColor(inBackColor)		
{
}



CRestorePortColorsAttachment::CRestorePortColorsAttachment(
	LStream *	inStream)
		: LAttachment(inStream)
{
	inStream->ReadData(&mForeColor, sizeof(Boolean));
	inStream->ReadData(&mBackColor, sizeof(Boolean));
}
					


	void
CRestorePortColorsAttachment::ExecuteSelf(
	MessageT	inMessage,
	void *		ioParam)
{
	RGBColor	foreColor;
	RGBColor	backColor;
	
	::GetForeColor(&foreColor);
	::GetBackColor(&backColor);
	
	dynamic_cast<LPane *>(GetOwnerHost())->ApplyForeAndBackColors();
	if (!mForeColor) {
		::RGBForeColor(&foreColor);
	}
	if (! mBackColor) {
		::RGBBackColor(&backColor);
	}
}
