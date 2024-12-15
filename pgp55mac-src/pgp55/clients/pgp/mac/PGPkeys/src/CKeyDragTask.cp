/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyDragTask.cp,v 1.4 1997/09/01 13:00:55 wprice Exp $
____________________________________________________________________________*/
#include "CKeyDragTask.h"
#include "CDragSource.h"

#include "pgpUtilities.h"
#include "pgpMem.h"

	static pascal OSErr 
SendData(
	FlavorType flavor, 
	void *dragSendRefCon, 
	ItemReference theItemRef,
	DragReference theDragRef);

CKeyDragTask::CKeyDragTask(
	CDragSource & inSource,
	const EventRecord & inEventRecord) :
		mSource(inSource),
		LDragTask(inEventRecord)
{
	mDragDataProc = NewDragSendDataProc(SendData);
}

CKeyDragTask::~CKeyDragTask()
{
	DisposeRoutineDescriptor(mDragDataProc);
}

	static pascal OSErr 
SendData(
	FlavorType flavor, 
	void *dragSendRefCon, 
	ItemReference theItemRef,
	DragReference theDragRef)
{
	OSErr	result = cantGetFlavorErr;
	void	*textBlock;
	long	len;

	if(flavor == 'TEXT')
	{
		if(((CKeyDragTask *)dragSendRefCon)->CreateText(theItemRef,
			&textBlock, &len))
		{
			result = SetDragItemFlavorData(theDragRef, 
											theItemRef, 
											'TEXT',
											textBlock, 
											len, 
											0L);
			PGPFreeData((char *)textBlock);
		}
	}
	else if(flavor == flavorTypePromiseHFS)
	{
		FSSpec	spec;
		
		if(((CKeyDragTask *)dragSendRefCon)->CreateHFS(theItemRef, &spec))
		{
			result = SetDragItemFlavorData(theDragRef, 
											theItemRef, 
											flavorTypeHFS,
											&spec, 
											sizeof(FSSpec), 
											0L);
		}
	}
	else
	{
		pgpAssert(0);	// Illegal flavor requested
	}
	return result;
}

	Boolean
CKeyDragTask::CreateText(ItemReference theItemRef, void **textBlock,
						long *len)
{
	return mSource.GetTextBlock(theItemRef, textBlock, len);
}

	Boolean
CKeyDragTask::CreateHFS(ItemReference theItemRef, FSSpec *spec)
{
	return mSource.GetHFS(theItemRef, mDragRef, spec);
}

	void	
CKeyDragTask::AddFlavors(DragReference inDragRef)
{
	mDragRef = inDragRef;
	SetDragSendProc(mDragRef, mDragDataProc, this);
	mSource.AddFlavors(this);
}

	void
CKeyDragTask::AddThisFlavor(UInt32	theRef)
{
	OSErr	theErr;
	
	// I disabled the HFS drag support because Eudora takes the HFS
	// rather than the TEXT and we always want to paste TEXT into
	// Eudora.
	
	//theErr = ::AddDragItemFlavor(mDragRef, theRef, flavorTypePromiseHFS,
	//								nil, 0, 0);
	//pgpAssert(IsntErr(theErr));
	theErr = ::AddDragItemFlavor(mDragRef, theRef, 'TEXT', nil, 0, 0);
	pgpAssert(IsntErr(theErr));
}

 	void	
CKeyDragTask::MakeDragRegion(DragReference	, RgnHandle inDragRegion)
{
	mSource.GetDragHiliteRgn(inDragRegion);
}

