/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: OffscreenUtils.h,v 1.2 1997/09/10 18:00:37 wprice Exp $
____________________________________________________________________________*/
#pragma once


void PGPCopyFromOffscrn(
	GrafPtr		offscrnPort,
	GrafPtr		macPort,
	Int16		top,
	Int16		left,
	Rect		destRect,
	Int16		mode,
	RgnHandle	maskRgn);
void PGPCopyToOffscrn(
	GrafPtr		offscrnPort,
	Int16		top,
	Int16		left,
	Rect		srcRect,
	RgnHandle	maskRgn);
void PGPCopyBits(
	CGrafPtr	srcPort,
	CGrafPtr	destPort,
	Rect		*srcRect,
	Rect		*destRect,
	Int16		mode,
	RgnHandle	maskRgn);
OSErr PGPCreateOffscrn(
	Rect		bounds,
	GDHandle	scrnDev,
	CGrafPtr	newPort,
	GDHandle	*newDev1,
	Ptr			newBits,
	long		bitsSize);
OSErr PGPDisposeOffscrn(
	CGrafPtr	port,
	GDHandle	dev);
	

Int16 PixelDepth(void);

