/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: MacQuickdraw.h,v 1.5 1997/09/18 01:34:54 lloyd Exp $
____________________________________________________________________________*/

#pragma once

#include <QuickDraw.h>

#include "pgpBase.h"

typedef enum RectAlignment
{
	kAlignAtNone				= 0x00,

	kAlignAtTop					= 0x01,
	kAlignAtVerticalCenter		= 0x02,
	kAlignAtBottom				= 0x04,
	
	kAlignAtLeft				= 0x10,
	kAlignAtHorizontalCenter	= 0x20,
	kAlignAtRight				= 0x40,

	kAlignAtTopLeft				= (kAlignAtTop | kAlignAtLeft),
	kAlignAtTopCenter			= (kAlignAtTop | kAlignAtHorizontalCenter),
	kAlignAtTopRight			= (kAlignAtTop | kAlignAtRight),

	kAlignAtCenterLeft			= (kAlignAtVerticalCenter | kAlignAtLeft),
	kAlignAtAbsoluteCenter		=
		(kAlignAtVerticalCenter | kAlignAtHorizontalCenter),
	kAlignAtCenterRight			=
		(kAlignAtVerticalCenter | kAlignAtRight),

	kAlignAtBottomLeft			= (kAlignAtBottom | kAlignAtLeft),
	kAlignAtBottomCenter		= (kAlignAtBottom | kAlignAtHorizontalCenter),
	kAlignAtBottomRight			= (kAlignAtBottom | kAlignAtRight)

} RectAlignment;

PGP_BEGIN_C_DECLARATIONS

void	AlignRect(const Rect *staticRect,
			Rect *alignRect, RectAlignment howToAlign);

PGP_END_C_DECLARATIONS