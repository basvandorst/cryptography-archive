/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: UDragAndDropUtils.h,v 1.2 1999/03/10 03:04:37 heller Exp $____________________________________________________________________________*/#pragma once#include <Drag.h>class UDragAndDropUtils {public:	static Boolean	DroppedInTrash( DragReference inDragRef );	static Boolean	CheckForOptionKey( DragReference inDragRef );	static Boolean	CheckIfViewIsAlsoSender( DragReference inDragRef );};