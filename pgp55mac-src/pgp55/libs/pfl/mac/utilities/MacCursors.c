/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	Portions of this code are (C) Copyright 1995-1997 by
	David K. Heller and are provided to PGP without restriction.

	$Id: MacCursors.c,v 1.7.10.1 1997/12/07 04:27:09 mhw Exp $
____________________________________________________________________________*/
#include "MacCursors.h"
#include "MacResources.h"
#include "pgpMem.h"

enum
{
	kCursorListResType 			= 'acur',
	kDefaultCursorIntervalTicks	= 30,
	kDefaultCursorInitalTicks	= kDefaultCursorIntervalTicks
};

typedef struct AnimatedCursorResource
{
	UInt16	numFrames;
	short	usedFrameCounter;
	
	struct
	{
		short 	resID;
		short	reserved;
	} cursorInfo[1];

} AnimatedCursorResource;

typedef struct AnimatedCursorInfo
{
	short	resID;
	UInt16	numFrames;
	UInt16	currentFrame;
	UInt32	nextAnimateTicks;
	UInt32	intervalTicks;
	UInt32	initialTicks;
	Cursor	theCursors[1];

} AnimatedCursorInfo;



	static OSStatus
FetchAnimatedCursor(
	ResID 				resID,
	AnimatedCursorRef 	*cursorRef,
	Boolean 			currentResFileOnly)
{
	AnimatedCursorResource	**acurRes;
	AnimatedCursorInfo		**acurInfo;
	OSStatus				status;
	
	pgpAssertAddrValid( cursorRef, AnimatedCursorRef );
	
	if( IsNull( cursorRef ) )
		return( paramErr );
		
	acurInfo 	= nil;
	*cursorRef	= nil;
	status		= noErr;
	
	if( currentResFileOnly )
	{
		acurRes = (AnimatedCursorResource **)
			Get1Resource( kCursorListResType, resID );
	}
	else
	{
		acurRes = (AnimatedCursorResource **)
			GetResource( kCursorListResType, resID );
	}
		
	if( acurRes != nil )
	{
		HNoPurge( (Handle) acurRes );
		
		acurInfo = (AnimatedCursorInfo **)
				NewHandleClear( sizeof( AnimatedCursorInfo ) +
						( ((**acurRes).numFrames - 1) * sizeof( Cursor ) ) );
		if( acurInfo != nil )
		{
			UInt16	frameIndex;
			UInt16	numFrames;
			
			(**acurInfo).resID 				= resID;
			(**acurInfo).numFrames 			= (**acurRes).numFrames;
			(**acurInfo).intervalTicks 		= kDefaultCursorIntervalTicks;
			(**acurInfo).initialTicks 		= kDefaultCursorInitalTicks;

			numFrames = (**acurRes).numFrames;
			for( frameIndex = 0; frameIndex < numFrames; frameIndex++ )
			{
				CursHandle	theCursor;
				short		resID;
				
				resID = (**acurRes).cursorInfo[frameIndex].resID;
				
				if( currentResFileOnly )
				{
					theCursor = (CursHandle) Get1Resource( 'CURS', resID );
				}
				else
				{
					theCursor = (CursHandle) GetResource( 'CURS', resID );
				}
				
				if( theCursor != nil )
				{
					(**acurInfo).theCursors[frameIndex] = **theCursor;
				}
				else
				{
					pgpDebugMsg("FetchAnimatedCursor(): "
					"Failed to get a cursor");
					status = ForceResError( resNotFound );
					break;
				}
				
				ReleaseResource( (Handle) theCursor );
			}
		}
		else
		{
			status = memFullErr;
		}

		ReleaseResource( (Handle) acurRes );
	}
	else
	{
		pgpDebugMsg("FetchAnimatedCursor(): Cursor list resource not found");
		status = ForceResError( resNotFound );
	}
	
	if( status == noErr )
	{
		*cursorRef = (AnimatedCursorRef) acurInfo;
		ResetAnimatedCursor( *cursorRef );
	}
	else
	{
		if( IsntNull( acurInfo ) )
			DisposeHandle( (Handle) acurInfo );
	}
	
	return( status );
}


	OSStatus
GetAnimatedCursor(short resID, AnimatedCursorRef *cursorRef)
{
	return( FetchAnimatedCursor( resID, cursorRef, FALSE ) );
}

	OSStatus
Get1AnimatedCursor(short resID, AnimatedCursorRef *cursorRef)
{
	return( FetchAnimatedCursor( resID, cursorRef, TRUE ) );
}


	void
ResetAnimatedCursor(AnimatedCursorRef cursorRef)
{
	AnimatedCursorInfo	**cursorInfo = (AnimatedCursorInfo **) cursorRef;
	
	AssertHandleIsValid( cursorInfo, "ResetAnimatedCursor" );
	
	if( cursorInfo != nil )
	{
		(**cursorInfo).currentFrame		= 0;
		(**cursorInfo).nextAnimateTicks =
			LMGetTicks() + (**cursorInfo).initialTicks;
	}
}


	void
SetAnimatedCursorDelay(AnimatedCursorRef cursorRef, UInt32 delayTicks)
{
	AnimatedCursorInfo	**cursorInfo = (AnimatedCursorInfo **) cursorRef;
	
	AssertHandleIsValid( cursorInfo, "SetAnimatedCursorDelay" );
	
	if( cursorInfo != nil )
	{
		(**cursorInfo).intervalTicks	= delayTicks;
		(**cursorInfo).nextAnimateTicks = LMGetTicks() + delayTicks;
	}
}


	void
SetAnimatedCursorInitialDelay(
	AnimatedCursorRef	cursorRef,
	UInt32				initialTicks)
{
	AnimatedCursorInfo	**cursorInfo = (AnimatedCursorInfo **) cursorRef;
	
	AssertHandleIsValid( cursorInfo, "SetAnimatedCursorDelay" );
	
	if( cursorInfo != nil )
	{
		(**cursorInfo).initialTicks	= initialTicks;
		ResetAnimatedCursor( cursorRef );
	}
}


	Boolean
AnimateCursor(AnimatedCursorRef cursorRef)
{
	AnimatedCursorInfo	**cursorInfo 	= (AnimatedCursorInfo **) cursorRef;
	Boolean				didAnimate		= FALSE;
	
	AssertHandleIsValid( cursorInfo, "AnimateCursor" );
	
	if( IsntNull( cursorInfo ) )
	{
		UInt32 curTicks;
		
		curTicks = LMGetTicks();
		
		if( curTicks >= (**cursorInfo).nextAnimateTicks )
		{
			UInt16	curFrame;

			curFrame = (**cursorInfo).currentFrame;
			(**cursorInfo).currentFrame += 1;
			
			if( (**cursorInfo).currentFrame >= (**cursorInfo).numFrames )
			{
				(**cursorInfo).currentFrame = 0;
			}
		
			SetCursor( &(**cursorInfo).theCursors[curFrame] );
			didAnimate = TRUE;
			
			(**cursorInfo).nextAnimateTicks =
				curTicks + (**cursorInfo).intervalTicks;
		}
	}
	
	return( didAnimate );
}


	void
DisposeAnimatedCursor(AnimatedCursorRef cursorRef)
{
	AssertHandleIsValid( cursorRef, "DisposeAnimatedCursor" );
	
	if( IsntNull( cursorRef ) )
		DisposeHandle( (Handle) cursorRef );
}
