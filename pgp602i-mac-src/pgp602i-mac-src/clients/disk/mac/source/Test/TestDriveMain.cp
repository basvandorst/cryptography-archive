/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: TestDriveMain.cp,v 1.4.8.1 1998/11/12 03:06:27 heller Exp $____________________________________________________________________________*/#include "MacFiles.h"#include "CTestDrive.h"	static voidInitMac(){	UnloadScrap();		InitGraf( &qd.thePort);	InitFonts();	InitWindows();	InitMenus();	TEInit();	InitDialogs( nil );	InitCursor();	MaxApplZone();}	static voidTestForever( short fileRef )	{	ulong	bufferSize	= 64UL * 1024UL;	void *	buffer	= NewPtrSys( bufferSize );	OSErr	err	= noErr;	ulong	eof;		pgpAssert( IsntNull( buffer ) );		GetEOF( fileRef, (long *)&eof );	pgpAssert( eof >= bufferSize );		while ( TRUE )		{		ulong			count;		EventRecord		theEvent;				if ( WaitNextEvent( everyEvent, &theEvent, 1, nil) )			{			if ( theEvent.what == keyDown &&					( theEvent.message & charCodeMask ) == '.' )				{				pgpDebugPStr( "\ptest halted" );				break;				}			}				ulong	offset;				offset	= ( ( ( (ulong)Random()) << 16) | Random() ) % eof;		if ( offset > eof - bufferSize )			offset	= eof - bufferSize;				err	= FSWriteAtOffset( fileRef, offset, bufferSize, buffer, &count);		AssertNoErr( err, "TestForever" );				count	= bufferSize;		err	= FSRead( fileRef, (long *)&count, buffer );		if ( err == eofErr )			{			SetFPos( fileRef, fsFromStart, 0 );			err	= noErr;			}		}		DisposePtr( (Ptr)buffer );	}	static voidSyncTest( short vRefNum )	{	FSSpec	spec = { vRefNum, fsRtDirID, "\ptest file\r" };	OSErr	err	= noErr;	short	fileRef;	const ulong	kTestFileSize	= 50 * 1024UL * 1024UL;		(void)FSpDelete( &spec );	(void)FSpCreate( &spec, 'fooB', 'FoOb', 0 );	err	= FSpOpenDF( &spec, fsRdWrPerm, &fileRef );	if ( IsntErr( err ) )		{		err	= SetEOF( fileRef, kTestFileSize );		FlushFile( fileRef );		if ( IsntErr( err ) )			{			TestForever( fileRef );			}					FSClose( fileRef );		}	AssertNoErr( err, "SyncTest" );	}				voidmain(void)	{	InitMac();		DebugPatches_PatchDisposeTraps();		pgpLeaksBeginSession( "main" );		short	vRefNum	= 0;	SetD0( 0 );	pgpDebugPStr( "\penter vRefNum in D0" );	vRefNum	= GetD0();	if ( vRefNum != 0 )		{		//CTestDrive	test( vRefNum );		//test.Run();				SyncTest( vRefNum );		}		pgpLeaksEndSession( );	}				