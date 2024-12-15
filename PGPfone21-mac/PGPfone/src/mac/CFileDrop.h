/*____________________________________________________________________________	Copyright (C) 1996-1999 Network Associates, Inc.	All rights reserved.	$Id: CFileDrop.h,v 1.4 1999/03/10 02:33:16 heller Exp $____________________________________________________________________________*/#pragma once#include <LView.h>#include <LBroadcaster.h>#include <LDragAndDrop.h>class CFileDrop	:	public LView,					public LDragAndDrop,					public LBroadcaster{public:	enum { class_ID = 'fdrp' };	static CFileDrop *CreateFileDropStream(LStream *inStream);						CFileDrop(LStream *inStream);						~CFileDrop();	void				FocusDropArea();	virtual Boolean		ItemIsAcceptable(DragReference inDragRef,										ItemReference inItemRef);	virtual void		EnterDropArea(DragReference inDragRef,										Boolean inDragHasLeftSender);	virtual void		LeaveDropArea(DragReference inDragRef);	virtual void		InsideDropArea(DragReference inDragRef);	virtual void		ReceiveDragItem(DragReference inDragRef,										DragAttributes inDragAttrs,										ItemReference inItemRef,										Rect &inItemBounds);	void				HiliteDropArea(DragReference	inDragRef);private:};