/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: UDragAndDropUtils.h,v 1.1 1997/06/30 10:33:19 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include <Drag.h>

class UDragAndDropUtils {
public:
	static Boolean	DroppedInTrash( DragReference inDragRef );
	static Boolean	CheckForOptionKey( DragReference inDragRef );
	static Boolean	CheckIfViewIsAlsoSender( DragReference inDragRef );
};
