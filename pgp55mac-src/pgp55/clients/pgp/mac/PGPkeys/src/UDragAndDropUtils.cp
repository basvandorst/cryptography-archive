// ============================================================================
//	UDragAndDropUtils.cp		(C)1996 Metrowerks Inc. All rights reserved.
// ============================================================================
#include <UMemoryMgr.h>
/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: UDragAndDropUtils.cp,v 1.2.8.1 1997/12/05 22:14:10 mhw Exp $
____________________________________________________________________________*/
#include "UDragAndDropUtils.h"

// ---------------------------------------------------------------------------
//		* DroppedInTrash
// ---------------------------------------------------------------------------

Boolean
UDragAndDropUtils::DroppedInTrash(
	DragReference	inDragRef )
{
	Boolean	isTrash = false;
	
	Try_ {

#ifdef	Debug_Throw
		// It's okay to fail here, so 
		// temporarily turn off debug actions.
		StValueChanger<EDebugAction>
			theDebugAction( gDebugThrow, debugAction_Nothing );
#endif

		// Get the drop location from the drag ref.
		AEDesc	theDropDestination;
		ThrowIfOSErr_( ::GetDropLocation( inDragRef, &theDropDestination ) );

		// Make sure we're dealing with an alias.
		if(theDropDestination.descriptorType != typeAlias)
			return FALSE;
		ThrowIf_( theDropDestination.descriptorType != typeAlias );

		// Get the file spec of the destination to
		// which the user dragged the item.
		FSSpec	theDestinationFSSpec;
		{
			// Lock the descriptor data handle.
			StHandleLocker	theLock( theDropDestination.dataHandle );
			
			// Attempt to resolve the alias.
			Boolean	isChanged;
			ThrowIfOSErr_( ::ResolveAlias( nil,
				(AliasHandle) theDropDestination.dataHandle,
				&theDestinationFSSpec, &isChanged ) );
		}
		
		// Get the file spec for the trash.
		FSSpec	theTrashFSSpec;
		Int16	theTrashVRefNum;
		Int32	theTrashDirID;
		ThrowIfOSErr_( ::FindFolder( kOnSystemDisk, kTrashFolderType,
			kDontCreateFolder, &theTrashVRefNum, &theTrashDirID ) );
		ThrowIfOSErr_( ::FSMakeFSSpec( theTrashVRefNum,
			theTrashDirID, nil, &theTrashFSSpec ) );

		// Compare the two file specs.
		isTrash =
			(theDestinationFSSpec.vRefNum == theTrashFSSpec.vRefNum ) &&
			(theDestinationFSSpec.parID	 ==	theTrashFSSpec.parID ) &&
			(::EqualString( theDestinationFSSpec.name,
				theTrashFSSpec.name, false, true ));
	
	} Catch_( inErr ) {
	
		// Nothing to do here.
	
	}

	return isTrash;
}


// ---------------------------------------------------------------------------
//		* CheckForOptionKey
// ---------------------------------------------------------------------------

Boolean
UDragAndDropUtils::CheckForOptionKey(
	DragReference	inDragRef )
{
	// Get the drag modifiers.
	Int16	theModifiersNow;
	Int16	theModifiersAtMouseDown;
	Int16	theModifiersAtMouseUp;
	::GetDragModifiers( inDragRef, &theModifiersNow,
		&theModifiersAtMouseDown, &theModifiersAtMouseUp );
	
	// Return true if the option key was down at the
	// beginning or the end of the drag.
	return ( (theModifiersAtMouseDown & optionKey) != 0 ) ||
		( (theModifiersAtMouseUp & optionKey) != 0 );
}


// ---------------------------------------------------------------------------
//		* CheckIfViewIsAlsoSender
// ---------------------------------------------------------------------------

Boolean
UDragAndDropUtils::CheckIfViewIsAlsoSender(
	DragReference	inDragRef )
{
	// Get the drag attributes.
	DragAttributes theDragAttributes;
	::GetDragAttributes(inDragRef, &theDragAttributes);
	
	// Return true if the drag is in the 
	// same window as the originator of the drag.
	return (theDragAttributes & dragInsideSenderWindow) != 0;
}
