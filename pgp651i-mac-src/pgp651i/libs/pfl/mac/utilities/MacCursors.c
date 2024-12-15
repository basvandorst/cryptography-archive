/*____________________________________________________________________________	Copyright (C) 1997-1999 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: MacCursors.c,v 1.10 1999/04/10 11:15:58 jason Exp $____________________________________________________________________________*/#include <LowMem.h>#include "MacCursors.h"#include "MacResources.h"#include "pgpMem.h"enum{	kCursorListResType 			= 'acur',	kDefaultCursorIntervalTicks	= 30,	kDefaultCursorInitalTicks	= kDefaultCursorIntervalTicks};typedef struct AnimatedCursorResource{	UInt16	numFrames;	short	usedFrameCounter;		struct	{		short 	resID;		short	reserved;	} cursorInfo[1];} AnimatedCursorResource;typedef struct AnimatedCursorInfo{	short	resID;	UInt16	numFrames;	UInt16	currentFrame;	UInt32	nextAnimateTicks;	UInt32	intervalTicks;	UInt32	initialTicks;	Boolean	colorCursor;	union	{		Cursor		bwCursor;		CCrsrHandle	colorCursor;	} cursors[1];} AnimatedCursorInfo;	static OSStatusFetchAnimatedCursor(	ResID 				resID,	AnimatedCursorRef 	*cursorRef,	Boolean 			currentResFileOnly){	AnimatedCursorResource	**acurRes;	AnimatedCursorInfo		**acurInfo;	OSStatus				status;		pgpAssertAddrValid( cursorRef, AnimatedCursorRef );		if( IsNull( cursorRef ) )		return( paramErr );			acurInfo 	= nil;	*cursorRef	= nil;	status		= noErr;		if( currentResFileOnly )	{		acurRes = (AnimatedCursorResource **)			Get1Resource( kCursorListResType, resID );	}	else	{		acurRes = (AnimatedCursorResource **)			GetResource( kCursorListResType, resID );	}			if( acurRes != nil )	{		HNoPurge( (Handle) acurRes );				acurInfo = (AnimatedCursorInfo **)				NewHandleClear( sizeof( AnimatedCursorInfo ) +						( ((**acurRes).numFrames - 1) * sizeof( Cursor ) ) );		if( acurInfo != nil )		{			UInt16	frameIndex;			UInt16	numFrames;						(**acurInfo).resID 				= resID;			(**acurInfo).numFrames 			= (**acurRes).numFrames;			(**acurInfo).intervalTicks 		= kDefaultCursorIntervalTicks;			(**acurInfo).initialTicks 		= kDefaultCursorInitalTicks;			(**acurInfo).colorCursor		= true;			numFrames = (**acurRes).numFrames;			for( frameIndex = 0; frameIndex < numFrames; frameIndex++ )			{				short		resID;				resID = (**acurRes).cursorInfo[frameIndex].resID;				if ((**acurInfo).colorCursor)				{					(**acurInfo).cursors[frameIndex].colorCursor = GetCCursor(resID);					if ((**acurInfo).cursors[frameIndex].colorCursor == nil)					{						(**acurInfo).colorCursor = false;					}					else					{						DetachResource((Handle) (**acurInfo).cursors[frameIndex].colorCursor);					}				}				if (! (**acurInfo).colorCursor)				{					CursHandle	theCursor;										if( currentResFileOnly )					{						theCursor = (CursHandle) Get1Resource( 'CURS', resID );					}					else					{						theCursor = (CursHandle) GetResource( 'CURS', resID );					}										if( theCursor != nil )					{						(**acurInfo).cursors[frameIndex].bwCursor = **theCursor;					}					else					{						pgpDebugMsg("FetchAnimatedCursor(): "						"Failed to get a cursor");						status = ForceResError( resNotFound );						break;					}										ReleaseResource( (Handle) theCursor );				}			}		}		else		{			status = memFullErr;		}		ReleaseResource( (Handle) acurRes );	}	else	{		pgpDebugMsg("FetchAnimatedCursor(): Cursor list resource not found");		status = ForceResError( resNotFound );	}		if( status == noErr )	{		*cursorRef = (AnimatedCursorRef) acurInfo;		ResetAnimatedCursor( *cursorRef );	}	else	{		if( IsntNull( acurInfo ) )		{			if ((**acurInfo).colorCursor)			{				UInt16	frameIndex;				UInt16	numFrames;				numFrames = (**acurInfo).numFrames;				for( frameIndex = 0; frameIndex < numFrames; frameIndex++ )				{					if ((**acurInfo).cursors[frameIndex].colorCursor != nil)					{						DisposeHandle((Handle) (**acurInfo).cursors[frameIndex].colorCursor);					}				}			}			DisposeHandle( (Handle) acurInfo );		}	}		return( status );}	OSStatusGetAnimatedCursor(short resID, AnimatedCursorRef *cursorRef){	return( FetchAnimatedCursor( resID, cursorRef, FALSE ) );}	OSStatusGet1AnimatedCursor(short resID, AnimatedCursorRef *cursorRef){	return( FetchAnimatedCursor( resID, cursorRef, TRUE ) );}	voidResetAnimatedCursor(AnimatedCursorRef cursorRef){	AnimatedCursorInfo	**cursorInfo = (AnimatedCursorInfo **) cursorRef;		AssertHandleIsValid( cursorInfo, "ResetAnimatedCursor" );		if( cursorInfo != nil )	{		(**cursorInfo).currentFrame		= 0;		(**cursorInfo).nextAnimateTicks =			LMGetTicks() + (**cursorInfo).initialTicks;	}}	voidSetAnimatedCursorDelay(AnimatedCursorRef cursorRef, UInt32 delayTicks){	AnimatedCursorInfo	**cursorInfo = (AnimatedCursorInfo **) cursorRef;		AssertHandleIsValid( cursorInfo, "SetAnimatedCursorDelay" );		if( cursorInfo != nil )	{		(**cursorInfo).intervalTicks	= delayTicks;		(**cursorInfo).nextAnimateTicks = LMGetTicks() + delayTicks;	}}	voidSetAnimatedCursorInitialDelay(	AnimatedCursorRef	cursorRef,	UInt32				initialTicks){	AnimatedCursorInfo	**cursorInfo = (AnimatedCursorInfo **) cursorRef;		AssertHandleIsValid( cursorInfo, "SetAnimatedCursorDelay" );		if( cursorInfo != nil )	{		(**cursorInfo).initialTicks	= initialTicks;		ResetAnimatedCursor( cursorRef );	}}	BooleanAnimateCursor(AnimatedCursorRef cursorRef){	AnimatedCursorInfo	**cursorInfo 	= (AnimatedCursorInfo **) cursorRef;	Boolean				didAnimate		= FALSE;		AssertHandleIsValid( cursorInfo, "AnimateCursor" );		if( IsntNull( cursorInfo ) )	{		UInt32 curTicks;				curTicks = LMGetTicks();				if( curTicks >= (**cursorInfo).nextAnimateTicks )		{			UInt16	curFrame;			curFrame = (**cursorInfo).currentFrame;			(**cursorInfo).currentFrame += 1;						if( (**cursorInfo).currentFrame >= (**cursorInfo).numFrames )			{				(**cursorInfo).currentFrame = 0;			}					if ((**cursorInfo).colorCursor)			{				SetCCursor( (**cursorInfo).cursors[curFrame].colorCursor );			}			else			{				SetCursor( &(**cursorInfo).cursors[curFrame].bwCursor );			}			didAnimate = TRUE;						(**cursorInfo).nextAnimateTicks =				curTicks + (**cursorInfo).intervalTicks;		}	}		return( didAnimate );}	voidDisposeAnimatedCursor(AnimatedCursorRef cursorRef){	AssertHandleIsValid( cursorRef, "DisposeAnimatedCursor" );		AnimatedCursorInfo	**cursorInfo 	= (AnimatedCursorInfo **) cursorRef;	if ((**cursorInfo).colorCursor)	{		UInt16	frameIndex;		UInt16	numFrames;		numFrames = (**cursorInfo).numFrames;		for( frameIndex = 0; frameIndex < numFrames; frameIndex++ )		{			if ((**cursorInfo).cursors[frameIndex].colorCursor != nil)			{				DisposeHandle((Handle) (**cursorInfo).cursors[frameIndex].colorCursor);			}		}	}	if( IsntNull( cursorRef ) )		DisposeHandle( (Handle) cursorRef );}