/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <UDrawingState.h>

#include "MacFiles.h"

#include "PGPtoolsUtils.h"

#include "CDropReceiver.h"
#include "CPGPtools.h"
#include "CPGPtoolsFileTask.h"
#include "CPGPtoolsTaskList.h"
#include "CPGPtoolsTextTask.h"
#include "CPGPtoolsWipeTask.h"

CDropReceiver *CDropReceiver::CreateFromStream(LStream *inStream)
{
	CDropReceiver	*receiver;
	
	// LDropArea leaks like a sieve. Need to suspend leaks when we create one.
	
	MacLeaks_Suspend();
	receiver = new CDropReceiver(inStream);
	MacLeaks_Resume();
	
	return( receiver );
}

CDropReceiver::CDropReceiver(LStream *inStream)
		:	LView(inStream),
			LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
{
	mTaskList 			= NULL;
	mDroppedFileList	= NULL;
	
	// The operation for this particular drop pane is stored in the
	// user constant field of the PPob.
	mDropOperation = (PGPtoolsOperation) GetUserCon();
}

CDropReceiver::~CDropReceiver()
{
	pgpAssert( IsNull( mTaskList ) );
}

	void
CDropReceiver::FocusDropArea()
{
	OutOfFocus(nil);
	FocusDraw();
}

	void
CDropReceiver::HiliteDropArea(
	DragReference	inDragRef)
{
	Rect		dropRect;
	RgnHandle	dropRgn;
	
	ApplyForeAndBackColors();
	mPane->CalcLocalFrameRect(dropRect);
	::InsetRect(&dropRect, 1, 1);
	dropRgn = ::NewRgn();
	::RectRgn(dropRgn, &dropRect);
	::ShowDragHilite(inDragRef, dropRgn, true);
	::DisposeRgn(dropRgn);
}

// ItemIsAcceptable will be called whenever the Drag Manager wants to know if
// the item the user is currently dragging contains any information that we
// can accept.
//
// In our case, the only thing we'll accept are flavorTypeHFS items.

	Boolean
CDropReceiver::ItemIsAcceptable(
	DragReference inDragRef,
	ItemReference inItemRef)
{
	Boolean		isAcceptable = FALSE;
	ushort		numFlavors;
	OSStatus	status;
					
	status = CountDragItemFlavors( inDragRef, inItemRef, &numFlavors );
	if( IsntErr( status ) )
	{
		ushort flavorIndex;
		
		for( flavorIndex = 1; flavorIndex <= numFlavors; flavorIndex++ )
		{
			FlavorType	flavorType;
			
			status = GetFlavorType( inDragRef, inItemRef, flavorIndex,
						&flavorType );
			if( IsntErr( status ) )
			{
				if( flavorType == 'TEXT' &&
					mDropOperation != kPGPtoolsWipeOperation )
				{
					isAcceptable = TRUE;
				}
				else if( flavorType == flavorTypeHFS )
				{
					HFSFlavor	hfsFlavor;
					Size		dataSize;
					
					dataSize = sizeof( hfsFlavor );
					status = GetFlavorData( inDragRef, inItemRef,
									flavorTypeHFS, &hfsFlavor, &dataSize, 0 );
					if( IsntErr( status ) )
					{
						if( hfsFlavor.fileType != 'disk' )
						{
							isAcceptable = TRUE;
						}
					}
				}
				
				if( isAcceptable )
					break;
			}
		}
	}
	
	AssertNoErr( status, "CDropReceiver::ItemIsAcceptable()" );
	
	return( isAcceptable );
}

	void
CDropReceiver::ReceiveDragItem(
	DragReference	inDragRef,
	DragAttributes	inDragAttrs,
	ItemReference	inItemRef,
	Rect			&inItemBounds)	// In Local coordinates
{
	FlavorFlags			flags;
	OSErr				err = noErr;
	CPGPtoolsTextTask	*textTask = nil;

	#pragma unused( inDragAttrs, inItemBounds )
	
	pgpAssert( IsntNull( mTaskList ) );
	
	// Try for our drag flavors in required order. Check 'TEXT' before
	// flavorTypeHFS because text clipping files come in as both.
	if( mDropOperation != kPGPtoolsWipeOperation &&
		IsntErr( GetFlavorFlags( inDragRef, inItemRef, 'TEXT', &flags) ) )
	{
		Size		dataSize;
		
		err = GetFlavorDataSize( inDragRef, inItemRef, 'TEXT', &dataSize );
		if( IsntErr( err ) )
		{
			PGPByte	*data;
			
			data = (PGPByte *) pgpAlloc( dataSize );
			if( IsntNull( data ) )
			{
				err = GetFlavorData( inDragRef, inItemRef, 'TEXT', data,
								&dataSize, 0L );
				if( IsntErr( err ) )
				{
					textTask = new( CPGPtoolsTextTask )( mDropOperation );
					if( IsntNull( textTask ) )
					{
						err = textTask->SetText( data, dataSize );
						if( IsntErr( err ) )
						{
							mTaskList->AppendTask( textTask );
						}
						else
						{
							delete( textTask );
							textTask = nil;
						}
					}
				}
				
				pgpFree( data );
			}
			else
			{
				err = memFullErr;
			}
		}
	}
	
	if( IsntErr( err ) )
	{
		if( IsntErr( GetFlavorFlags( inDragRef, inItemRef, flavorTypeHFS,
					&flags) ) )
		{
			Size		dataSize;
			HFSFlavor	hfsFlavor;
			
			dataSize = sizeof( hfsFlavor );
			
			err = GetFlavorData( inDragRef, inItemRef, flavorTypeHFS,
						&hfsFlavor, &dataSize, 0 );
			if( IsntErr( err ) )
			{
				Boolean	targetIsFolder;
				Boolean	targetWasAlias;
				
				pgpAssert( dataSize <= sizeof( hfsFlavor ) );
				
				err = ResolveAliasFile( &hfsFlavor.fileSpec, TRUE,
							&targetIsFolder, &targetWasAlias );
				if( IsntErr( err ) )
				{
					if( IsntNull( textTask ) )
					{
						// The dragged text has assocaited file data. Set the
						// file name in the text task object for error
						// reporting and window title purposes.
						
						textTask->SetSourceFileName( hfsFlavor.fileSpec.name );
					}
					else
					{
						// Add the spec to our list of specs to process. We will
						// then process this list to remove child specs of
						// other parent specs in the list
						
						mDroppedFileList->InsertItemsAt( 1, 32000,
								hfsFlavor.fileSpec );
					}
				}
				else if( targetWasAlias )
				{
					// The alias could not be resolved. Skip without failure.
					err = noErr;
				}
			}
		}
	}
	
	AssertNoErr( err, "ReceiveDragItem" );
	
	// Powerplant requires this if we want to return the error correctly.
	ThrowIfOSErr_(err);
}

// Override this method to handle creating and queueing of tasks.
// Drag items are processed individually, however all file objects
// are appended to a single list so we can eliminate duplicates.

	void
CDropReceiver::DoDragReceive(DragReference inDragRef)
{
	pgpAssert( IsNull( mTaskList ) );

	mTaskList = new CPGPtoolsTaskList( mDropOperation);
	if( IsntNull( mTaskList ) )
	{
		// By default, we assume the dragged items are TEXT clippings and
		// the options should not be shown. If a file is encountered,
		// the options will be changed to show the options pane in the
		// recipient dialog.
		
		mTaskList->SetTaskListHasFileTasks( FALSE );

		mDroppedFileList = new TArray<FSSpec>;
		
		LDragAndDrop::DoDragReceive( inDragRef );
	
		// If there were any files dragged, remove child specs, create
		// the tasks here and add them to the task list
		
		if( mDroppedFileList->GetCount() > 0 )
		{
			if( IsntErr( RemoveChildAndDuplicateSpecsFromList(
						mDroppedFileList ) ) &&
				mDroppedFileList->GetCount() > 0 )
			{
				PGPUInt32	specIndex;
				PGPUInt32	numSpecs;
				
				mTaskList->SetTaskListHasFileTasks( TRUE );
				
				numSpecs = mDroppedFileList->GetCount();
				for( specIndex = 1; specIndex <= numSpecs; specIndex++ )
				{
					CPGPtoolsTask	*task;
					FSSpec			theSpec;
					
					mDroppedFileList->FetchItemAt( specIndex, theSpec );
					
					if ( mDropOperation == kPGPtoolsWipeOperation )
					{
						task = new CPGPtoolsWipeTask( &theSpec );
					}
					else
					{
						task = new( CPGPtoolsFileTask )( mDropOperation,
								&theSpec );
					}
					
					if( IsntNull( task ) )
					{
						mTaskList->AppendTask( task );
					}
				}
			}
		}
		
		gApplication->EnqueueTaskList( mTaskList );
		
		delete mDroppedFileList;
		
		mDroppedFileList 	= NULL;
		mTaskList 			= NULL;
	}
}