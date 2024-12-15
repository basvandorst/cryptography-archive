/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: CKeyDragTask.h,v 1.6 1999/03/10 02:34:36 heller Exp $____________________________________________________________________________*/#pragma once#include <LDragTask.h>#include "pgpKeys.h"class	CDragSource;const FlavorType	kKeyDragFlavor			= 	'pgKY';class	CKeyDragTask	:	public LDragTask{public:			CKeyDragTask(	CDragSource & inSource,							const EventRecord& inEventRecord);			~CKeyDragTask();	void	AddThisFlavor( UInt32	theRef );		Boolean	CreateKeyRef(ItemReference theItemRef, PGPKeyRef *key);	Boolean	CreateText(ItemReference theItemRef, void **textBlock, long *len);	Boolean	CreateHFS(ItemReference theItemRef, FSSpec *spec);protected:	virtual void	AddFlavors(							DragReference			inDragRef);	virtual void	MakeDragRegion(							DragReference			inDragRef,							RgnHandle				inDragRegion);	CDragSource&	mSource;	DragReference	mDragRef;	DragSendDataUPP	mDragDataProc;};