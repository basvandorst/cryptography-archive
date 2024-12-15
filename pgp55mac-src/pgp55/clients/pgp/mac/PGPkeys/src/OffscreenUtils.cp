/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: OffscreenUtils.cp,v 1.2.8.1 1997/12/05 22:14:09 mhw Exp $
____________________________________________________________________________*/
#include "OffscreenUtils.h"

	Int16
PixelDepth(void)
{
	Int16 depth;
	GDHandle gd;
	
	if(!UEnvironment::HasFeature(env_SupportsColor))
		return 1;
	gd=GetMaxDevice(&qd.screenBits.bounds);
	depth=(*(*gd)->gdPMap)->pixelSize;
	return depth;
}

	static int
CalcRowBytesOffscrn(GDHandle theDevice, Int16 width)
{
	return (((theDevice ? (*(*theDevice)->gdPMap)->pixelSize : 1)
			* width + 63) >> 5 ) << 2;
}

	OSErr
PGPDisposeOffscrn(CGrafPtr port, GDHandle dev)
{
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	if(dev)
	{
		if(dev==GetGDevice())
			SetGDevice(GetMainDevice());
		(*dev)->gdPMap=nil;
		DisposeGDevice(dev);
		if((*port->portPixMap)->pmTable!=nil)
			DisposeCTable((*port->portPixMap)->pmTable);
		CloseCPort(port);
	}
	else
		ClosePort((GrafPtr)port);
	SetPort(oldPort);
	return 0;
}

	void
PGPCopyFromOffscrn(
	GrafPtr		offscrnPort,
	GrafPtr		macPort,
	Int16		top,
	Int16		left,
	Rect		destRect,
	Int16		mode,
	RgnHandle	maskRgn)
{
	Rect sr;
	
	ForeColor(blackColor);
	BackColor(whiteColor);
	sr.top=top;
	sr.left=left;
	sr.bottom=top+destRect.bottom-destRect.top;
	sr.right=left+destRect.right-destRect.left;
	PGPCopyBits((CGrafPtr)offscrnPort, (CGrafPtr)macPort,
				&sr, &destRect, mode, maskRgn);
}

	void
PGPCopyToOffscrn(
	GrafPtr		offscrnPort,
	Int16		top,
	Int16		left,
	Rect		srcRect,
	RgnHandle	maskRgn)
{
	Rect		dr;
	CGrafPtr	curPort;
	
	ForeColor(blackColor);
	BackColor(whiteColor);
	dr.top=top;
	dr.left=left;
	dr.bottom=top+srcRect.bottom-srcRect.top;
	dr.right=left+srcRect.right-srcRect.left;
	GetPort((GrafPtr *)&curPort);
	PGPCopyBits(curPort, (CGrafPtr)offscrnPort,
					&srcRect, &dr, srcCopy, maskRgn);
}

	void
PGPCopyBits(
	CGrafPtr	srcPort,
	CGrafPtr	destPort,
	Rect		*srcRect,
	Rect		*destRect,
	Int16		mode,
	RgnHandle	maskRgn)
{
	GrafPtr			oldPort;
	char			srcMapState,
					destMapState;
	PixMapHandle	srcPix,
					destPix;
	void			*src,
					*dest;
	
	GetPort(&oldPort);
	SetPort((GrafPtr)destPort);
	if(UEnvironment::HasFeature(env_SupportsColor) &&
		(srcPort->portVersion & 0xC000)==0xC000)
	{
		srcPix = srcPort->portPixMap;
		srcMapState = HGetState((Handle)srcPix);
		HLock((Handle)srcPix);
		src = *srcPix;
	}
	else
	{
		srcPix = nil;
		src = &((GrafPtr)srcPort)->portBits;
	}
	if(UEnvironment::HasFeature(env_SupportsColor) &&
		(destPort->portVersion & 0xC000)==0xC000)
	{
		destPix = destPort->portPixMap;
		destMapState = HGetState((Handle)destPix);
		HLock((Handle)destPix);
		dest = *destPix;
	}
	else
	{
		destPix = nil;
		dest = &((GrafPtr)destPort)->portBits;
	}
	CopyBits(	(BitMap*)src,	(BitMap*)dest,
				srcRect,		destRect,
				mode, maskRgn);
	if(srcPix)
		HSetState((Handle)srcPix, srcMapState);
	if(destPix)
		HSetState((Handle)destPix, destMapState);
	SetPort(oldPort);
}

	static OSErr
PGPCreateOffscrnBits(
	Rect	bounds,
	GrafPtr	newPort,
	Ptr		newBits,
	Int32	bitsSize)
{
	GrafPtr	oldPort;
	Int16	rowBytes;
	Int16	height,
			width;
	OSErr	err = 0;
	
	height = bounds.bottom-bounds.top;
	width = bounds.right-bounds.left;
	rowBytes = CalcRowBytesOffscrn(nil, width);
	pgpAssert(bitsSize == rowBytes * (Int32)height);
	GetPort(&oldPort);
	OpenPort(newPort);
	newPort->portRect.top = newPort->portRect.left = 0;
	newPort->portRect.bottom = height;
	newPort->portRect.right = width;
	RectRgn(newPort->visRgn, &newPort->portRect);
	ClipRect(&newPort->portRect);
	newPort->portBits.baseAddr = newBits;
	newPort->portBits.rowBytes = rowBytes;
	newPort->portBits.bounds.top = 0;
	newPort->portBits.bounds.bottom = height;
	newPort->portBits.bounds.left = -(bounds.left & 31);
	newPort->portBits.bounds.right = width;
	SetPort(oldPort);
	return err;
}

	static OSErr
PGPCreateOffscrnPix(
	Rect			bounds,
	GDHandle		scrnDev,
	CGrafPtr		newPort,
	GDHandle		*newDev1,
	Ptr				newBits,
	Int32			bitsSize)
{
	PixMapHandle	scrnPix,
					newPix = nil;
	GDHandle		oldDev,
					newDev = nil;
	CTabHandle		newColors;
	ITabHandle		embryo;
	GrafPtr			oldPort;
	char			savedState;
	Int16			rowBytes,
					depth;
	Int16			height,
					width;
	OSErr			err = 0;
	
	height = bounds.bottom-bounds.top;
	width = bounds.right-bounds.left;
	GetPort(&oldPort);
	oldDev = GetGDevice();
	scrnPix = (*scrnDev)->gdPMap;
	savedState = HGetState((Handle)(*scrnPix)->pmTable);
	HNoPurge((Handle)(*scrnPix)->pmTable);
	depth = (*scrnPix)->pixelSize;
	if(depth != 2 && depth != 4 && depth != 8 &&
		((depth != 16 && depth != 32) ||
		!UEnvironment::HasFeature(env_SupportsColor)))
	{
		err = 1;
		goto error;
	}
	SetGDevice(scrnDev);
	rowBytes = CalcRowBytesOffscrn(scrnDev, width);
	if(rowBytes >= 0x4000)
	{
		err = 1;
		goto error;
	}
	OpenCPort(newPort);
	newPort->portRect.top = newPort->portRect.left = 0;
	newPort->portRect.bottom = height;
	newPort->portRect.right = width;
	RectRgn(newPort->visRgn, &newPort->portRect);
	ClipRect(&newPort->portRect);
	
	newPix = newPort->portPixMap;
	if(depth <= 8)
	{
		newColors = (*scrnPix)->pmTable;
		err = HandToHand((Handle *)&newColors);
		pgpAssert(IsntErr(err));
	}
	else
	{
		newColors = (CTabHandle)
			NewHandle(sizeof(ColorTable)-sizeof(CSpecArray));	//**********
		pgpAssert(IsntErr(MemError()));
	}
	pgpAssert(bitsSize == rowBytes*(Int32)height);
	pgpAssert(newBits);
	(*newPix)->baseAddr = newBits;
	(*newPix)->rowBytes = rowBytes | 0x8000;
	(*newPix)->bounds.top = 0;
	(*newPix)->bounds.bottom = height;
	(*newPix)->bounds.left = -(bounds.left&(32/depth-1));
	(*newPix)->bounds.right = width;
	(*newPix)->pmVersion = 0;
	(*newPix)->packType = 0;
	(*newPix)->packSize = 0;
	(*newPix)->hRes = (*newPix)->vRes = 0x480000;	// 72 DPI
	(*newPix)->pixelSize = depth;
	(*newPix)->planeBytes = 0;
	(*newPix)->pmReserved = 0;
	if(depth <= 8)
	{
		(*newPix)->pixelType = 0;
		(*newPix)->cmpCount = 1;
		(*newPix)->cmpSize = depth;
		(*newPix)->pmTable = newColors;
	}
	else
	{
		(*newPix)->pixelType = RGBDirect;
		(*newPix)->cmpCount = 3;
		(*newPix)->cmpSize = (*newColors)->ctSeed = (depth == 16) ? 5 : 8;
		(*newColors)->ctSeed *= 3;
		(*newColors)->ctFlags = 0;
		(*newColors)->ctSize = 0;
		(*newPix)->pmTable = newColors;
	}
	
	newDev = (GDHandle)NewHandle(sizeof(GDevice));	//************
	pgpAssert(IsntErr(MemError()));
	embryo = (ITabHandle)NewHandleClear(2);			//************
	pgpAssert(IsntErr(MemError()));
	(*newDev)->gdRefNum = 0;
	(*newDev)->gdID = 0;
	(*newDev)->gdType = (depth <= 8) ? clutType : directType;
	(*newDev)->gdITable = embryo;
	(*newDev)->gdResPref = 4;
	(*newDev)->gdSearchProc = nil;
	(*newDev)->gdCompProc = nil;
	(*newDev)->gdFlags = 0;
	(*newDev)->gdPMap = newPix;
	(*newDev)->gdRefCon = 0;
	(*newDev)->gdNextGD = nil;
	(*newDev)->gdRect.top = (*newDev)->gdRect.left = 0;
	(*newDev)->gdRect.bottom = height;
	(*newDev)->gdRect.right = (*newPix)->bounds.right;
	(*newDev)->gdMode = -1;
	(*newDev)->gdCCBytes = (*newDev)->gdCCDepth = 0;
	(*newDev)->gdCCXData = (*newDev)->gdCCXMask = nil;
	(*newDev)->gdReserved = 0;
	if(depth>1)
		SetDeviceAttribute(newDev, gdDevType, -1);
	SetDeviceAttribute(newDev, noDriver, -1);
	if(depth <= 8)
	{
		MakeITable((*newPix)->pmTable, (*newDev)->gdITable,
					(*newDev)->gdResPref);
		pgpAssert(IsntErr(QDError()));
	}
	
	if(UEnvironment::HasFeature(env_SupportsColor))
	{
		PortChanged((GrafPtr)newPort);
		GDeviceChanged(newDev);
	}
	
error:
	HSetState((Handle)(*scrnPix)->pmTable, savedState);
	SetGDevice(oldDev);
	SetPort(oldPort);
	pgpAssert(IsntErr(err));
	*newDev1 = newDev;
	return err;
}

	OSErr
PGPCreateOffscrn(
	Rect		bounds,
	GDHandle	scrnDev,
	CGrafPtr	newPort,
	GDHandle	*newDev1,
	Ptr			newBits,
	Int32		bitsSize)
{
	OSErr		result;
	
	*newDev1 = NULL;
	if((scrnDev ? (*(*scrnDev)->gdPMap)->pixelSize : 1) == 1)
	{
		result = PGPCreateOffscrnBits(bounds, (GrafPtr)newPort,
										newBits, bitsSize);
		*newDev1 = nil;
	}
	else
		result = PGPCreateOffscrnPix(bounds, scrnDev, newPort,
										newDev1, newBits, bitsSize);
	return result;
}
