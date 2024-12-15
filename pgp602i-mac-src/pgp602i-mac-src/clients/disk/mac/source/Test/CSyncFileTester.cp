/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CSyncFileTester.cp,v 1.5.8.1 1998/11/12 03:06:24 heller Exp $____________________________________________________________________________*/#include <Timer.h>#include <stddef.h>#include "pgpMacMemory.h"#include "CSyncFileTester.h"CSyncFileTester::CSyncFileTester( short testFileRefNum )	{	OSErr	err	= noErr;		mCurMark	= 0;	mEOF		= 0;		mTestRefNum	= testFileRefNum;	}CSyncFileTester::~CSyncFileTester( )	{	mTestRefNum	= 0;	}		voidCSyncFileTester::Run(  )	{	const ulong	kTestSize	= 2 * 128UL * 1024UL;		void *	buffer;	ulong	bufferSize	= kTestSize;		buffer	= pgpAllocMac( bufferSize, kMacMemory_PreferTempMem);	if ( IsntNull( buffer ) )		{		OSErr			err;		ParamBlockRec	pb;				// do two reads: the first one async, followed by a second sync		// this tests a difficult case for the driver		ulong			readSize	= bufferSize / 2;				pb.ioParam.ioCompletion		= nil;		pb.ioParam.ioVRefNum		= 0;		pb.ioParam.ioRefNum			= mTestRefNum;		pb.ioParam.ioBuffer			= (Ptr)buffer;		pb.ioParam.ioReqCount		= readSize;		pb.ioParam.ioPosMode		= fsAtMark;		(void)PBReadAsync( &pb );				long	count	= readSize;		err	= FSRead( mTestRefNum, &count, (Ptr)buffer + readSize );		if ( err == eofErr )			{			SetFPos( mTestRefNum, fsFromStart, 0 );			err	= noErr;			}		AssertNoErr( err, "CSyncFileTester::Run" );				// the async case *must* have finished prior to the sync case.		pgpAssert( pb.ioParam.ioResult != 1 );				pgpFreeMac( buffer );		}	}		