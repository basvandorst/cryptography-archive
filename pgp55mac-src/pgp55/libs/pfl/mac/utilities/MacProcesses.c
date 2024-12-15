/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Portions of this code are (C) Copyright 1995-1997 by
	David K. Heller and are provided to PGP without restriction.
	

	$Id: MacProcesses.c,v 1.5.10.1 1997/12/07 04:27:12 mhw Exp $
____________________________________________________________________________*/

#include "pgpMem.h"

#include "MacProcesses.h"

	Boolean
FindProcess(
	OSType 				type,				
	OSType 				creator,		
	ProcessSerialNumber *psn,				
	ProcessInfoRec		*processInfo,		
	FSSpec				*processSpec,		
	StringPtr			processName)		
{
	ProcessSerialNumber	tempPSN;
	ProcessInfoRec		tempProcessInfo;
	Str255				tempProcessName;
	FSSpec				tempProcessSpec;
	Boolean				foundProcess = FALSE;
	
	AssertAddrNullOrValid( psn, ProcessSerialNumber, "FindProcess" );
	AssertAddrNullOrValid( processInfo, ProcessInfoRec, "FindProcess" );
	AssertAddrNullOrValid( processSpec, FSSpec, "FindProcess" );
	AssertAddrNullOrValid( processName, uchar, "FindProcess" );
	
	if( IsNull( psn ) )
		psn = &tempPSN;
	
	if( IsNull( processInfo ) )
		processInfo = &tempProcessInfo;
		
	if( IsNull( processSpec ) )
		processSpec = &tempProcessSpec;
		
	if( IsNull( processName ) )
		processName = &tempProcessName[0];
	
	pgpClearMemory( processInfo, sizeof( *processInfo ) );
	
	psn->lowLongOfPSN 	= kNoProcess;
	psn->highLongOfPSN 	= 0;
	
	processInfo->processInfoLength	= sizeof( *processInfo );
	processInfo->processName		= processName;
	processInfo->processAppSpec		= processSpec;
	
	while( IsntErr( GetNextProcess( psn ) ) )
	{
		if( IsntErr( GetProcessInformation( psn, processInfo ) ) )
		{
			if( processInfo->processType == type &&
				processInfo->processSignature == creator )
			{
				foundProcess = TRUE;
				break;
			}
		}
	}

	return( foundProcess );
}
