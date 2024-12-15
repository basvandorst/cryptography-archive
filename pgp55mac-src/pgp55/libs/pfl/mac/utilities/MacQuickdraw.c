/*____________________________________________________________________________
	MacQuickdraw.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacQuickdraw.c,v 1.4 1997/09/18 01:34:53 lloyd Exp $
____________________________________________________________________________*/

#include "MacQuickdraw.h"
#include "pgpDebug.h"

/*____________________________________________________________________________
	Align one rectangle inside another
____________________________________________________________________________*/

	void
AlignRect(const Rect *staticRect, Rect *alignRect, RectAlignment howToAlign)
{
	short	vOffset = 0;
	short	hOffset = 0;

	pgpAssertAddrValid( staticRect, Rect );
	pgpAssertAddrValid( alignRect, Rect );
	
	if( ( howToAlign & kAlignAtTop ) != 0 )
	{
		vOffset = staticRect->top - alignRect->top;
	}
	else if( ( howToAlign & kAlignAtBottom ) != 0 )
	{
		vOffset = staticRect->bottom - alignRect->bottom;
	}
	else if( ( howToAlign & kAlignAtVerticalCenter ) != 0 )
	{
		vOffset = ( ( staticRect->top + staticRect->bottom ) -
						( alignRect->top + alignRect->bottom ) ) / 2;
	}

	if( ( howToAlign & kAlignAtLeft ) != 0 )
	{
		hOffset = staticRect->left - alignRect->left;
	}
	else if( ( howToAlign & kAlignAtRight ) != 0 )
	{
		hOffset = staticRect->right - alignRect->right;
	}
	else if( ( howToAlign & kAlignAtHorizontalCenter ) != 0 )
	{
		hOffset = ( ( staticRect->left + staticRect->right ) -
						( alignRect->left + alignRect->right ) ) / 2;
	}
	
	OffsetRect( alignRect, hOffset, vOffset );
}
