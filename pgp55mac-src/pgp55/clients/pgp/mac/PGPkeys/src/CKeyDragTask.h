/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyDragTask.h,v 1.3 1997/09/17 19:36:37 wprice Exp $
____________________________________________________________________________*/
#pragma once

class	CDragSource;

class	CKeyDragTask	:	public LDragTask
{
public:
			CKeyDragTask(	CDragSource & inSource,
							const EventRecord& inEventRecord);
			~CKeyDragTask();

	void	AddThisFlavor( UInt32	theRef );
	
	Boolean	CreateText(ItemReference theItemRef, void **textBlock, long *len);
	Boolean	CreateHFS(ItemReference theItemRef, FSSpec *spec);

protected:
	virtual void	AddFlavors(
							DragReference			inDragRef);
	virtual void	MakeDragRegion(
							DragReference			inDragRef,
							RgnHandle				inDragRegion);

	CDragSource&	mSource;
	DragReference	mDragRef;
	DragSendDataUPP	mDragDataProc;
};

