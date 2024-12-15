/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CPBQueue.cp,v 1.3.8.1 1998/11/12 03:05:41 heller Exp $____________________________________________________________________________*/#pragma once#include <stddef.h>#include "CPBQueue.h"#include "A4Stuff.h"#include "SetupA4.h"CPBQueue::CPBQueue()	{	PrepareCallback();	}	/*_________________________________________________________________________	This routine must be called at a time when memory may be moved_________________________________________________________________________*/CPBQueue::~CPBQueue()	{	while( ! mInProgressQueue.IsEmpty() )		{		// wait till all of them are done		}			// items in our queue were all dynamically allocated	// get rid of them	while ( ! mAvailableQueue.IsEmpty() )		{		QElem *	pb;				pb	= mAvailableQueue.PeekHead();		mAvailableQueue.Dequeue( pb );		DisposePtr( (Ptr)AvailableElemToQueuePB( pb ) );		}	}	pascal voidCPBQueue::sCompletionHook( void )	{	CommonPBHeader *	pb	= (CommonPBHeader *)GetA0();		EnterCodeResource();		AssertNoErr( pb->ioResult, "CPBQueue::sCompletionHook" );	QueuePB *		qpb	= PBToQueuePB( pb );		qpb->thisObject->Completion( qpb );		ExitCodeResource();	}/*_________________________________________________________________________	This routine must be called at a time when memory may be moved_________________________________________________________________________*/	CPBQueue::QueuePB *CPBQueue::GetPB(  )	{	QElem *		elem;	QueuePB *	pb	= nil;	OSErr		err	= noErr;		elem	= mAvailableQueue.PeekHead();	if ( IsntNull( elem ) )		{		err	= mAvailableQueue.Dequeue( elem );		AssertNoErr( err, "CPBQueue::GetPB" );				pb	= AvailableElemToQueuePB( elem );		}	else		{		pb	= (QueuePB *)NewPtrSys( sizeof( *pb ) + GetPBSize() );		}			if ( IsntNull( pb ) )		{		CommonPBHeader *	hdr	= nil;				mInProgressQueue.Enqueue( &pb->inProgressElem );			pb->thisObject				= this;				hdr	= (CommonPBHeader *)&pb->pbBytes[ 0 ];		hdr->ioCompletion	= sCompletionHook;		}	else		{		pgpDebugPStr( "\pcan't create pb" );		}		// caller should call ReleasePB when done with it		return( pb );	}	OSErrCPBQueue::ReleasePB( QueuePB *qpb )	{	OSErr	err	= noErr;		mAvailableQueue.Enqueue( &qpb->availableElem );		err	= mInProgressQueue.Dequeue( &qpb->inProgressElem );	AssertNoErr( err, "CPBQueue::GetPB" );		MacDebug_FillWithGarbage( &qpb->pbBytes, GetPBSize() );		return( err );	}				