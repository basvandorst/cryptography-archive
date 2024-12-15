/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include <LBroadcaster.h>
#include <LDragAndDrop.h>
#include <LView.h>

#include "CPGPtoolsTaskList.h"

class CPGPtoolsFileTask;

class CDropReceiver	:	public LView,
						public LDragAndDrop,
						public LBroadcaster
{
public:
	enum { class_ID = 'Fdrp' };
	static CDropReceiver *CreateFromStream(LStream *inStream);
						CDropReceiver(LStream *inStream);
						~CDropReceiver();
	void				FocusDropArea();
	virtual Boolean		ItemIsAcceptable(
										DragReference	inDragRef,
										ItemReference	inItemRef);
	virtual void		ReceiveDragItem(
										DragReference	inDragRef,
										DragAttributes	inDragAttrs,
										ItemReference	inItemRef,
										Rect			&inItemBounds);
	void				HiliteDropArea(	DragReference	inDragRef);

protected:

	CPGPtoolsTaskList		*mTaskList;
	PGPtoolsOperation		mDropOperation;
	TArray<FSSpec>			*mDroppedFileList;
	
	virtual void		DoDragReceive(DragReference inDragRef);
};

