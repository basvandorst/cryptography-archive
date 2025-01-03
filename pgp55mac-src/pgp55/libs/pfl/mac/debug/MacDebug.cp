/*____________________________________________________________________________
	MacDebug.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDebug.cp,v 1.14 1997/09/18 01:34:22 lloyd Exp $
____________________________________________________________________________*/

#include "MacFiles.h"
#include "MacStrings.h"
#include "pgpMem.h"

#include "MacDebug.h"

// trick to force inclusion of MacNew.cp by causing link error if missing
#if USE_PGP_OPERATOR_NEW
	#include "MacNew.h"
	static MacNewDummy	sMacNewDummy;
#endif

#if PGP_DEBUG	// [

// avoid recursive calls with DebugTraps
#include "MacDebugTrapsOff.h"


static Boolean	DebugGetErrorString( OSStatus err, char errString[]);



#define kHStatePurgeableMask	0x40
#define kHStateLockedMask		0x80
#define kHStateResourceMask		0x20

#define HStateIsPurgeable(hState)	( ((hState) & kHStatePurgeableMask) != 0 )
#define HStateIsLocked(hState)		( ((hState) & kHStateLockedMask) != 0 )
#define HStateIsResource(hState)	( ((hState) & kHStateResourceMask) != 0 )



#if PRAGMA_MARK_SUPPORTED
#pragma mark --- Utilities ---
#endif

/*____________________________________________________________________________
	Convert a number to a pascal string.
	
	This version doesn't move memory, unlike the system version of NumToString,
	so it can be used safely anywhere.
____________________________________________________________________________*/
	static void
NumToStringBase(
	long		num, 
	StringPtr	outString, 
	UInt16		base,
	Boolean		isSigned)
	{
	pgpAssertMsg( base >= 2 && base <= 16, "NumToStringBase: illegal base");
	pgpAssertMsg( ( ! isSigned ) || base == 10,
		"NumToStringBase: signed only applies to base 10");

	if (num == 0)
		{
		outString[ 0 ] = 1;
		outString[ 1 ] = '0';
		}
	else
		{
		Boolean		isNegative = FALSE;
		UInt32		ulNum;
		UInt8		digitIndex;
		UInt8		tempBuffer[ 64 ];	// not a pascal string
		UInt8		*outPtr;

		// only base 10 is treated as signed. If so, convert using positive
		// number and remember that it's negative for later.
		if (num < 0 &&
			base == 10 &&
			isSigned)
			{
			isNegative = TRUE;
			num		= -num;
			}
		
		ulNum = (UInt32) num;
	
		// add digits in convenient (reverse) order
		outPtr	= &tempBuffer[ 0 ];
		while ( ulNum > 0 )
			{
			UInt32	temp;
			UInt8	digitVal;
			static unsigned char	sDigits[] = "0123456789ABCDEF";
	
			temp		= ulNum/base;
			digitVal	= ulNum - (temp * base);
	
			*outPtr++	= sDigits[ digitVal ];
	
			ulNum = temp;
			}
	
		if ( isNegative )
			{
			*outPtr++ = '-';
			}
	
		// reverse the digits so they're in the correct order
		outString[0] = outPtr - &tempBuffer[ 0 ];
	
		for (digitIndex = 1; digitIndex <= outString[0]; ++digitIndex)
			{
			outString[ digitIndex ] = *--outPtr;
			}
		}
	}


/*____________________________________________________________________________
	Convert a number to a string (same as NumToString).
	
	Unlike the system version, does not move memory.
____________________________________________________________________________*/
	static void
DebugNumToPString(
	long		num,
	StringPtr	string)
	{
	NumToStringBase( num, string, 10, TRUE);
	}


#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#endif


/*____________________________________________________________________________
	Build an assert message using several strings.
	
	Output is a pascal string, since it will usually be used to call
	DebugStr().
____________________________________________________________________________*/
	static void
BuildBadAddressMsg(
	StringPtr			result,
	const char *		prefix,
	const char *		varName,
	const char *		suffix)
	{
	pgpFormatPStr255( result, TRUE,
		"%s: '%s' [%s]", prefix, varName, suffix );
	}


	static void
MemoryDummy(short	dummy)
	{
	++dummy;
	}




/*____________________________________________________________________________
	Assert (and possibly crash) if the handle is invalid
	This routine expressly does not return a value because part of its
	checks may result in a crash if a handle is "extra" bogus.
____________________________________________________________________________*/
	void
_AssertHandleIsValid(
	const void *		theHandle,	// void * avoids need to cast
	const char *		varName,	// a null terminated C string
	const char *		msg,
	const char *		fileName,
	const UInt32			lineNumber)
	{
	OSErr	err;
	Str255	result;
	
	result[0]	= 0;
	
	if ( IsNull( msg ) )
		{
		msg	= "AssertHandleIsValid()";
		}
	
	char	compositeMsg[ 512 ];
	pgpFormatStr( compositeMsg, sizeof(compositeMsg), TRUE,
		"AssertHandleIsValid: %s: %s\rin: %s line #%lu",
		msg, varName, fileName, (UInt32)lineNumber);
	pgpAssertAddrValidMsg( theHandle, long, compositeMsg);

	
	/*_____________________________________________________________________
	make a memory reference to force a crash here if the handle is invalid
	It is better to force a crash here, 
	rather than crashing in some obscure place in the ROM later
	
	call a dummy routine to hopefully prevent compiler from simply
	optimizing out our dereference.
	_______________________________________________________________________*/
	MemoryDummy( *(short *)theHandle );
		
	(void)HandleZone( (Handle) theHandle );
	err	= MemError();
	if ( IsErr( err ) )
		{
		char	errString[ 256 ];
		
		DebugGetErrorString( err, errString );
		
		pgpFormatPStr255( result, TRUE,
		"error '%s' from HandleZone for '%s'\r%s", errString, varName, msg);
		pgpDebugPStr( result );
		}
	
	if ( IsntNull( *(Handle)theHandle )  )
		// check to see if we can get its size (if it has a master pointer)
		{
		(void)GetHandleSize( (Handle) theHandle );
		err	= MemError();
		if ( IsErr( err ) )
			{
			char	errString[ 256 ];
		
			DebugGetErrorString( err, errString );
		
			pgpFormatPStr255( result, TRUE,
				"error '%s' (%ld) from GetHandleSize for '%s'\r[%s]",
				errString, (long)err, varName, msg);
			
			pgpDebugPStr( result );
			}
		}
	}
	
	
/*____________________________________________________________________________
	A resource should be a valid Handle and be a resource
____________________________________________________________________________*/
	void
_AssertResourceIsValid(
	const void *		theHandle,	// void * avoids need to cast
	const char *		varName,	// a null terminated C string
	const char *		msg,
	const char *		fileName,
	const UInt32			lineNumber)
	{
	_AssertHandleIsValid( theHandle, varName, msg, fileName, lineNumber);

	if ( ! HStateIsResource( HGetState( (Handle)theHandle ) ) )
		{
		pgpDebugFmtMsg( ( pgpaFmtPrefix,
			"AssertResourceIsValid: "
			"not a resource '%s'\r[%s]\rin: %s line #%lu",
		varName, msg, fileName, (UInt32)lineNumber) );
		}
	}
	


	

#if PRAGMA_MARK_SUPPORTED
#pragma mark -
#endif




/*____________________________________________________________________________
	A table consists of an array of entries.
____________________________________________________________________________*/
typedef struct OSErrStringTable
	{
	UInt16								numEntries;
	const DebugOSErrStringTableEntry	*entries;
	} OSErrStringTable;


/*____________________________________________________________________________
	Add an error string table to the list of tables. If the table is already
	in our list, do nothing.
____________________________________________________________________________*/
#define kMaxErrStringTables		20
static OSErrStringTable		sOSErrStringTables[ kMaxErrStringTables ] = {0,};
static UInt16				sNumOSErrStringTables	= 0;
	void
DebugAddOSErrStringTable(
	const DebugOSErrStringTableEntry	*entries,
	UInt16 numEntries)
	{
	Boolean	alreadyInList	= FALSE;
	UInt16	tableIndex;

	// see if we already have the table in our list
	for( tableIndex = 0; tableIndex < sNumOSErrStringTables; ++tableIndex)
		{
		if ( sOSErrStringTables[tableIndex].entries == entries )
			{
			alreadyInList	= TRUE;
			break;
			}
		}

	if ( ! alreadyInList )
		{
		if ( sNumOSErrStringTables < kMaxErrStringTables )
			{
			OSErrStringTable *table	=
				&sOSErrStringTables[ sNumOSErrStringTables ];
			
			table->entries		= entries;
			table->numEntries	= numEntries;
			++sNumOSErrStringTables;
			}
		else
			{
			pgpDebugPStr("\pDebugAddOSErrStringTable(): "
			"no more tables available");
			}
		}
	}



/*____________________________________________________________________________
	search an error table for an error code.

	return TRUE if found, FALSE otherwise.
	If found, return the index in 'foundIndex'
____________________________________________________________________________*/
	static Boolean
SearchErrorTable(
	OSErr					err,
	const OSErrStringTable	*table,
	UInt16 *				foundIndex)
	{
	UInt16		pairIndex;
	Boolean		foundIt	= FALSE;

	*foundIndex	= 0;

	for( pairIndex = 0; pairIndex < table->numEntries; ++pairIndex)
		{
		if ( table->entries[ pairIndex ].err == err )
			{
			foundIt	= TRUE;
			*foundIndex	= pairIndex;
			break;
			}
	}
	return( foundIt );
	}


// include standard error table
#include "MacDebugErrStrings.h"

	static void
AddDefaultErrorStringTables()
	{
	static Boolean sAddedDefaultTables	= FALSE;

	// add the default table to the list if it has not yet been added
	if ( ! sAddedDefaultTables )
		{
		sAddedDefaultTables	= TRUE;

		DebugAddOSErrStringTable( sSystemErrorsTable,
			sizeof( sSystemErrorsTable ) / sizeof( sSystemErrorsTable[0] ));
		}
	}



/*____________________________________________________________________________
	Get a pascal string corresponding to an error code.
	
	If a string was found, return TRUE,
	otherwise return false and use a numeric code.
____________________________________________________________________________*/
	static Boolean
DebugGetErrorString(
	OSStatus	err,
	char		errStringOut[])
	{
	UInt16		tableIndex;
	Boolean		haveString	= FALSE;

	AddDefaultErrorStringTables();

	errStringOut[0]	= 0;
	
	// search all error tables one-by-one until we find a match
	for( tableIndex = 0; tableIndex < sNumOSErrStringTables; ++tableIndex)
		{
		const OSErrStringTable	*table;
		UInt16		entryIndex;

		table	= &sOSErrStringTables[tableIndex];

		if ( SearchErrorTable( err, table, &entryIndex) )
			{
			haveString	= TRUE;
			CopyCString( table->entries[entryIndex].cString, errStringOut);
			break;
			}
		}
	
	if ( errStringOut[ 0 ] == '\0' )
		{
		Str255	errStringTemp;
		
		// just return the numeric code
		DebugNumToPString( err, errStringTemp);
		PToCString( errStringTemp, errStringOut );
		}
	
	return( haveString );
	}




/*____________________________________________________________________________
	drop into the debugger with an assert if 'err' is not 'noErr'
____________________________________________________________________________*/
	void
_AssertNoErr(
	OSStatus			err,
	const char *		optionalMsg,
	const char *		srcFileName,
	const UInt32			lineNumber)
	{
#define IsCancelErr( err )		( (err) == userCanceledErr )

	// don't report cancel errors 
	if ( IsErr( err ) && ! IsCancelErr( err ) )
		{
		Str255	msg;
		char	errString[ 256 ];
		
		if ( IsNull( optionalMsg ) )
			optionalMsg	= "";
	
		if ( ! DebugGetErrorString( err, errString) )
			{
			CopyCString( "unknown error", errString );
			}
		
		pgpFormatPStr255( msg, TRUE,
			"AssertNoErr: %ld (%s) [%s]\rin: %s line #%lu",
			(long)err, errString, optionalMsg,
			srcFileName, (UInt32)lineNumber);
		pgpDebugPStr( msg );
		}
	}

	
	
/*____________________________________________________________________________
	If refNum is not valid, assert and display it
____________________________________________________________________________*/
	void
_AssertFileRefNumIsValid(
	short				refNum,
	const char *		varName,
	const char *		optionalMsg,
	const char *		srcFileName,
	const UInt32			lineNumber)
	{
	#define kFCBSize  94
	
	if ( refNum <= 0 || ((refNum - 2) % kFCBSize) != 0)
		{
		Str255	msg;
		
		if ( IsNull( optionalMsg ) )
			optionalMsg	= "";
		
		pgpFormatPStr255( msg, TRUE,
			"AssertFileRefNumIsValid: "
			"invalid refNum '%s' = %ld [%s]\rin: %s line #%lu",
			varName, (long)refNum, optionalMsg, srcFileName,
			(UInt32)lineNumber);
		pgpDebugPStr( msg );
		}
	}


	static Boolean
IsValidSpec( const FSSpec *spec )
	{
	Boolean	isValid	= TRUE;
	
	if (	spec->vRefNum >= 0 ||
			spec->parID <= 0 ||
			StrLength( spec->name ) == 0 ||
			StrLength( spec->name ) > kMaxHFSFileNameLength 
		)
		{
		isValid	= FALSE;
		}
	
	return( isValid );
	}


	
/*____________________________________________________________________________
	If spec is not valid, assert and display the spec
____________________________________________________________________________*/
	void
_AssertSpecIsValid(
	const FSSpec *		spec,
	const char *		varName,
	const char *		optionalMsg,
	const char *		srcFileName,
	const UInt32			lineNumber)
	{
	pgpAssertAddrValid( spec, FSSpec);
	
	if ( ! IsValidSpec( spec ) )
		{
		Str255	msg;
		FSSpec	tempSpec;
		
		// limit name length to kMaxHFSFileNameLength chars to
		// avoid overflowing available space
		tempSpec	= *spec;
		if ( StrLength( tempSpec.name ) > kMaxHFSFileNameLength )
			tempSpec.name[ 0 ]	= kMaxHFSFileNameLength;
		
		if ( IsNull( optionalMsg ) )
			optionalMsg	= "";
		
		pgpFormatPStr255( msg, TRUE,
			"AssertSpecIsValid: "
			"invalid spec '%s'\r in: %s line #%lu\r[%s]\r vRefNum: "
			"%ld\rparID: %ld\r name: %S\r",
			varName,
			srcFileName, (UInt32)lineNumber,
			optionalMsg, (long)spec->vRefNum, spec->parID, tempSpec.name);
			
		pgpDebugPStr( msg );
		}
	}


	static Boolean
IsValidAEDesc( const AEDesc *desc )
	{
	Boolean	isValid	= IsntNull( desc );
	
	return( isValid );
	}
	
					
	void
_AssertAEDescIsValid(
	const AEDesc *	desc,
	const char *	varName,
	const char *	optionalMsg,
	const char *	fileName,
	const UInt32		lineNumber)
	{
	pgpAssertAddrValid( desc, AEDesc);
	
	if ( ! IsValidAEDesc( desc ) )
		{
		Str255	msg;
		
		if ( IsNull( optionalMsg ) )
			optionalMsg	= "";
		
		pgpFormatPStr255( msg, TRUE,
			"AssertAEDescIsValid: "
			"invalid AEDesc '%s' from %s line #%lu\r[%s]",
			varName,
			fileName, (UInt32)lineNumber,
			optionalMsg );
			
		pgpDebugPStr( msg );
		}
	
	}
	
	
	
	void
_AssertUPPIsValid(
	const UniversalProcPtr	upp,
	const char *			varName,
	const char *			optionalMsg,
	const char *			fileName,
	const UInt32				lineNumber)
	{
	pgpAssertAddrValid( upp, short);
	
#if GENERATINGCFM
	const RoutineDescriptor *descriptor	= (const RoutineDescriptor *)upp;
	
	if ( descriptor->goMixedModeTrap != _MixedModeMagic )
		{
		Str255	msg;
		
		pgpFormatPStr255( msg, TRUE,
			"AssertUPPIsValid: invalid upp '%s' from %s line #%lu\r[%s]",
			varName,
			fileName, (UInt32)lineNumber,
			optionalMsg );
			
		pgpDebugPStr( msg );
		}
#else
	// avoid compiler complaints about unused args
	varName		= varName;
	optionalMsg	= optionalMsg;
	fileName	= fileName;
	UInt32 dummy	= lineNumber;

#endif
	
	}
					

#include "pgpMem.h"
	void
MacDebug_FillWithGarbage(
	void	*ptr,
	UInt32	byteCount)
	{
	//ptr	= ptr;
	//byteCount	= byteCount;
	pgpFillMemory( ptr, byteCount, 0xDD );
	}





/*____________________________________________________________________________
	Allocate a Handle which takes most of the remaining heap space or at least
	a large chunk of it.
____________________________________________________________________________*/
	static Handle
AllocLargeHandle( )
	{
	Handle	theHandle	= nil;
	UInt32	attemptSize = 128UL * 1024UL;
	
	while ( attemptSize >= 4 )
		{
		theHandle	= NewHandle( attemptSize );
		if ( IsntNull( theHandle ) )
			break;
		
		attemptSize	/= 2;
		}
		
	return( theHandle );
	}
	
/*____________________________________________________________________________
	Allocate all free space in the current heap, whack the bytes,
	then free all allocated space.
____________________________________________________________________________*/
	void
MacDebug_WhackAllFreeSpace()
	{
	typedef struct ListItem
	{
		ListItem **	next;
	} ListItem, **ListItemHandle;
	ListItemHandle	nextHandle;
	ListItemHandle	listHead	= nil;
	
	// allocate all free space and link it into a singly linked list
	while ( ( nextHandle = (ListItemHandle)AllocLargeHandle() ) != nil )
		{
		// safety check:
		// could a Handle ever be less than the size of our link?
		if ( GetHandleSize( (Handle)nextHandle ) < sizeof( **nextHandle ) )
			{
			DisposeHandle( (Handle)nextHandle );
			break;
			}
			
			(**nextHandle).next	= listHead;
			listHead	= nextHandle;
		}
	
	// we've now allocated all the space in the heap
	// whack all the bytes and free the space
	while ( IsntNull( listHead ) )
		{
		nextHandle	= (**listHead).next;
		
		MacDebug_FillWithGarbage( *listHead,
			GetHandleSize( (Handle)listHead ) );
		DisposeHandle( (Handle)listHead );
		
		listHead	= nextHandle;
		}
	}


#endif	// ] PGP_DEBUG












