/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$Id: pgpUtilities.c,v 1.26 1997/10/09 23:54:19 lloyd Exp $
____________________________________________________________________________*/

#include "pgpErrors.h"
#include "pgpMem.h"

#include "pgpContext.h"
#include "pgpFileSpec.h"
#include "pgpUtilities.h"
#include "pgpUtilitiesPriv.h"
#include "pgpOptionList.h"
#include "pgpMacBinary.h"
#include "pgpMacFileMapping.h"

#if PGP_MACINTOSH
#include "MacEnvirons.h"
#include "MacFiles.h"
#include "MacStrings.h"
#endif

#if PGP_WIN32 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

typedef struct DataHeader
{
#define kHeaderMagic	((PGPUInt16)0xABCD)
	PGPUInt16		magic;
	PGPUInt16		isSecureAlloc : 1;
	PGPUInt16		flags : 15;
	PGPContextRef	context;
	PGPSize			requestSize;
	PGPUserValue	extra;
} DataHeader;

#define HeaderToUserBlock( hdr )	\
	( (void *)( (hdr) + 1) )
#define UserBlockToHeader( user, headerType )	\
	( (headerType *)(((char *)user) - sizeof( headerType )) )


	static PGPBoolean
sDataHeaderIsValid( const DataHeader *header )
{
	return( header->magic == kHeaderMagic &&
		pgpContextIsValid( header->context ) );
}

#if PGP_DEBUG
#define AssertBlockValid( block, headerType) \
	pgpAssert( IsntNull( block ) && \
	sDataHeaderIsValid( (const DataHeader *)\
		UserBlockToHeader( block, headerType) ) )
#else
#define AssertBlockValid( block, headerType ) /* nothing */
#endif



static void		sLockMemory( void *mem, PGPSize numBytes,
					PGPBoolean *didLock );
static void		sUnlockMemory( void *mem, PGPSize numBytes );

	static void
sInitDataHeader(
	PGPContextRef	context,
	PGPSize			requestSize,
	DataHeader *	header )
{
	pgpClearMemory( header, sizeof(*header) );
	header->magic			= kHeaderMagic;
	header->context			= context;
	header->requestSize		= requestSize;
	header->extra			= NULL;
	header->isSecureAlloc	= FALSE;
}


	static DataHeader *
sNewDataHeader(
	PGPContextRef	context,
	PGPSize			requestSize,
	PGPMemoryFlags	flags)
{
	DataHeader *	header	= NULL;
	
	if ( ! pgpContextIsValid( context ) )
	{
		pgpDebugMsg( "invalid context" );
		return( NULL );
	}
	
	header	= (DataHeader *)pgpContextMemAlloc( context,
		sizeof( *header ) + requestSize, flags );
	if ( IsntNull( header ) )
	{
		sInitDataHeader( context, requestSize, header );
	}
	
	return( header );
}


	void *
pgpNewData(
	PGPContextRef	context,
	PGPSize			requestSize,
	PGPMemoryFlags	flags)
{
	DataHeader *	header	= NULL;
	void *			result	= NULL;
	
	header	= sNewDataHeader( context, requestSize, flags );
	if ( IsntNull( header ) )
	{
		result	= HeaderToUserBlock( header );
	}
	
	return( result );
}


/*____________________________________________________________________________
	Same as pgpNewData(), but copies data into the newly allocated block.
____________________________________________________________________________*/
	void *
pgpNewDataCopy(
	PGPContextRef	context,
	PGPSize			requestSize,
	PGPMemoryFlags	flags,
	const void *	dataToCopy )
{
	void *	result	= pgpNewData( context, requestSize, flags );
	if ( IsntNull( result ) )
	{
		pgpCopyMemory( dataToCopy, result, requestSize );
	}
	return( result );
}

	void *
PGPNewData(
	PGPContextRef	context,
	PGPSize			allocationSize)
{
	if ( ! pgpContextIsValid( context ) )
	{
		pgpDebugMsg( "invalid context" );
		return( NULL );
	}

	return( pgpNewData( context, allocationSize, 0 ) );
}


/*____________________________________________________________________________
	This routine should allocate a buffer of the specified size which cannot
	be paged to disk or in anyway stored other than in memory.
	
	The caller must be sure to free it with PGPFreeSecureData()
____________________________________________________________________________*/
	void  *
PGPNewSecureData(
	PGPContextRef	context,
	PGPSize			requestSize,
	PGPBoolean *	didLockOut )
{
	DataHeader *	header	= NULL;
	void *			result	= NULL;
	
	if ( IsntNull( didLockOut ) )
	{
		*didLockOut	= FALSE;
	}	
	if ( ! pgpContextIsValid( context ) )
	{
		pgpDebugMsg( "invalid context" );
		return( NULL );
	}
	
	header	= sNewDataHeader( context, requestSize, 0 );
	if ( IsntNull( header ) )
	{
		PGPBoolean		didLock;
		
		header->isSecureAlloc	= TRUE;
		result	= HeaderToUserBlock( header );
		sLockMemory(  result, requestSize, &didLock );
		if ( IsntNull( didLockOut ) )
			*didLockOut	= didLock;
	}
	
	return( result );
}



	void
PGPFreeData( void * allocation )
{
	DataHeader *	header;
	PGPSize			requestSize;
	
	AssertBlockValid( allocation, DataHeader );
	header	= UserBlockToHeader( allocation, DataHeader);
	if ( IsNull( allocation ) || ! sDataHeaderIsValid( header ) )
		return;
	
	requestSize	= header->requestSize;
	if ( header->isSecureAlloc )
	{
		/* important: clear it first; othersise it could
		be paged after unlocking but before clearing */
		pgpClearMemory( allocation, requestSize );
		sUnlockMemory( allocation, requestSize );
	}
	else
	{
		pgpDebugWhackMemory( allocation, requestSize );
	}
	pgpContextMemFree( header->context, header );
}



#if PGP_MACINTOSH	/* ] [ */

/*____________________________________________________________________________
	MacVersion
____________________________________________________________________________*/
	static void
sLockMemory(
	void *			mem,
	PGPSize			numBytes,
	PGPBoolean *	didLock )
{
	pgpAssert( IsntNull( didLock ) );
	*didLock	= TRUE;
	
	if ( VirtualMemoryIsOn()  )
	{
		HoldMemory( mem, numBytes );
	}
}

/*____________________________________________________________________________
	MacVersion
____________________________________________________________________________*/
	static void
sUnlockMemory(
	void *	mem,
	PGPSize	numBytes )
{
	if ( VirtualMemoryIsOn()  )
	{
		UnholdMemory( mem, numBytes );
	}
}



#elif PGP_WIN32	/* ] [ */


/*____________________________________________________________________________
	Win32 version
____________________________________________________________________________*/

	/* 	Win32 does not support locking pages into memory without
		extraordinary effort (it may be possible by using special-
		purpose device drivers).  

		Here, instead, we do the minimal effort: we call the Win32 API
		functions VirtualLock and VirtualUnlock.  Under Windows95 
		these functions do absolutely nothing.  Under WindowsNT they
		prevent the specified memory range from being paged to disk
		*while this process is executing*.  When all threads of this
		process become pre-empted, the OS is free to page whatever 
		it wants (including "VirtualLock-ed" memory) to disk.

		The interpretation of "didLock" is thus a little shaky here.
		Currently we assign "FALSE" if it absolutely did not page-lock
		the memory (either we're running under Windows95 or VirtualLock 
		failed under NT).  We return TRUE if VirtualLock succeeds under 
		NT, even though it's really not true page-locking.
	*/

	static void
sLockMemory(
	void *			mem,
	PGPSize			numBytes,
	PGPBoolean *	didLock )
{
	OSVERSIONINFO osid;

	pgpAssert( IsntNull( didLock ) );
	*didLock	= FALSE;
 
	osid.dwOSVersionInfoSize = sizeof ( osid );
	GetVersionEx ( &osid );

	switch ( osid.dwPlatformId ) {

		case VER_PLATFORM_WIN32s :			/* Win32s and Win95 versions*/
		case VER_PLATFORM_WIN32_WINDOWS :	/* of VirtualLock don't work*/
 			VirtualLock ( mem, numBytes );	/* but call for good measure*/
			break;

		case VER_PLATFORM_WIN32_NT :
			if ( VirtualLock ( mem, numBytes ) )	
			{									
				*didLock = TRUE;
			}
			break;

		default :
			break;
	}
}

/*____________________________________________________________________________
	Win32 version
____________________________________________________________________________*/
	static void
sUnlockMemory(
	void *	mem,
	PGPSize	numBytes )
{
	VirtualUnlock ( mem, numBytes );	
}



#elif PGP_UNIX	/* ] [ */


/* Code borrowed from Peter Gutmann's cryptlib for secure memory locking */

#if defined( __osf__ )
  #include <sys/mman.h>
#elif defined( sun )
  #include <sys/types.h>
#else
  int mlock( void *address, size_t length );
  int munlock( void *address, size_t length );
#endif /* Unix-variant-specific includes */

/*____________________________________________________________________________
	Unix version
____________________________________________________________________________*/

	static void
sLockMemory(
	void *			mem,
	PGPSize			numBytes,
	PGPBoolean *	didLock )
{
	pgpAssert( IsntNull( didLock ) );

	/* mlock call taken from Peter Gutmann's cryptlib */
	/* Under many Unix variants the SYSV/POSIX mlock() call can be
	   used, but only by the superuser.  OSF/1 has mlock(), but
	   this is defined to the nonexistant memlk() so we need to
	   special-case it out.  Aches and PHUX don't even pretend to
	   have mlock().  Many systems also have plock(), but this is
	   pretty crude since it locks all data, and also has various
	   other shortcomings.  Finally, PHUX has datalock(), which is
	   just a plock() variant */
#if !( defined( __osf__ ) || defined( _AIX ) || defined( __hpux ) || \
defined( _M_XENIX ) )
	*didLock = (mlock( mem, numBytes ) == 0);
#else
	*didLock = FALSE;
#endif /* !( __osf__ || _AIX || __hpux || _M_XENIX ) */

}


/*____________________________________________________________________________
	Unix version
____________________________________________________________________________*/
	static void
sUnlockMemory(
	void *	mem,
	PGPSize	numBytes )
{
#if !( defined( __osf__ ) || defined( _AIX ) || defined( __hpux ) || \
			defined( _M_XENIX ) )
	(void)munlock( mem,  numBytes );
#endif /* !( __osf__ || _AIX || __hpux || _M_XENIX ) */
}


#else	/* ] [ */

#error	unknown platform

#endif	/* ] */









		


	PGPError
PGPCopyFileSpec(
	PGPConstFileSpecRef	fileRef,
	PGPFileSpecRef *	outRef )
{
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PFLValidateFileSpec( (PFLConstFileSpecRef)fileRef );
	
	return( PFLCopyFileSpec( (PFLConstFileSpecRef)fileRef,
		(PFLFileSpecRef*)outRef ) );
}

	PGPError
PGPFreeFileSpec( PGPFileSpecRef		fileRef)
{
	PFLValidateFileSpec( (PFLFileSpecRef)fileRef );
	
	return( PFLFreeFileSpec( (PFLFileSpecRef)fileRef ) );
}




#if PGP_MACINTOSH	/* [ */

	PGPError 
PGPNewFileSpecFromFSSpec(
	PGPContextRef		context,
	FSSpec const *		spec,
	PGPFileSpecRef *	outRef )
{
	return( PFLNewFileSpecFromFSSpec( (PFLContextRef)context,
		spec, (PFLFileSpecRef*)outRef ) );
}


	PGPError
PGPGetFSSpecFromFileSpec(
	PGPConstFileSpecRef	fileRef,
	FSSpec *			spec)
{
	return( PFLGetFSSpecFromFileSpec( (PFLFileSpecRef)fileRef, spec ) );
}

#else /* ] PGP_MACINTOSH [ */


	PGPError 
PGPNewFileSpecFromFullPath(
	PGPContextRef		context,
	char const *		path,
	PGPFileSpecRef *		outRef )
{
	PGPError	err	= kPGPError_NoErr;
	
	err	= PFLNewFileSpecFromFullPath( (PFLContextRef)context,
		path, (PFLFileSpecRef *)outRef );
	return err;
}



	PGPError 
PGPGetFullPathFromFileSpec(
	PGPConstFileSpecRef	fileRefIn,
	char **				fullPathPtr)
{
	PGPError			err;
	PFLContextRef		pflContext	= NULL;
	char *				tempPath	= NULL;
	char *				fullPath	= NULL;
	PFLConstFileSpecRef	fileRef	= (PFLConstFileSpecRef)fileRefIn;
	
	PGPValidatePtr( fullPathPtr );
	*fullPathPtr	= NULL;
	PFLValidateFileSpec( fileRef );
	
	pflContext	= PFLGetFileSpecContext( fileRef );
	
	err	= PFLGetFullPathFromFileSpec( fileRef, &tempPath );
	if ( IsntPGPError( err ) )
	{
		fullPath	= (char *)
			pgpNewData( (PGPContextRef)pflContext, strlen( tempPath ) + 1, 0);
		if ( IsntNull( fullPath ) )
		{
			strcpy( fullPath, tempPath );
		}
		else
		{
			err	= kPGPError_OutOfMemory;
		}
		PFLContextMemFree( pflContext, tempPath );
	}
	
	*fullPathPtr	= fullPath;
	return( err );
}



#endif	/* ] PGP_MACINTOSH */





/*____________________________________________________________________________
	Examine the input file to see if it's a MacBinary file.  If it is
	not a MacBinary file, then nothing is done.  Otherwise, it is
	converted, the original file is deleted and the resulting file is
	designated by 'outPGPSpec'.
	
	creator and type code pointers may be
	null but otherwise contain the mac creator and type.
	
	The output file may have a different name than the original because
	its Mac creator/type codes may be mapped into a file name extension.
	
	Example (assuming it's an MS-Word file):
		MyStuff.doc	=> MyStuff.doc
		MyStuff.bin => MyStuff.doc
		MyStuff		=> MyStuff.doc
____________________________________________________________________________*/
	PGPError
PGPMacBinaryToLocal(
	PGPFileSpecRef		inPGPSpec,
	PGPFileSpecRef *	outPGPSpec,
	PGPUInt32 *			macCreator,
	PGPUInt32 *			macType )
{
	PGPError			err;
	
	if ( IsntNull( macCreator ) )
		*macCreator	= 0;
	if ( IsntNull( macType ) )
		*macType	= 0;
	if ( IsntNull( outPGPSpec ) )
		*outPGPSpec	= NULL;
		
	PGPValidatePtr( outPGPSpec );
	PFLValidateFileSpec( (PFLFileSpecRef)inPGPSpec );
	
	err = pgpMacBinaryToLocal( (PFLFileSpecRef)inPGPSpec,
			(PFLFileSpecRef *)outPGPSpec, macCreator, macType );
	
	return( err );
}













/*____________________________________________________________________________
	Determine where the preference file resides, and create a PFLFileSpecRef
	which locates it.
____________________________________________________________________________*/
#if PGP_MACINTOSH	/* [ */
	PGPError
pgpGetPrefsSpec(
	PFLContextRef		pflContext,
	PFLFileSpecRef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	FSSpec				fsSpec;
	const unsigned char	kCDKPrefsFileName[] = "\pPGPsdkPrefs";
	PFLFileSpecRef		newRef	= NULL;
	
	*outRef		= NULL;
	
	err	= FindPGPPreferencesFolder( -1, &fsSpec.vRefNum, &fsSpec.parID );
	if ( IsntPGPError( err ) )
	{
		CopyPString( kCDKPrefsFileName, fsSpec.name );
		
		err	= PFLNewFileSpecFromFSSpec( pflContext, &fsSpec, &newRef );
		if ( IsntPGPError( err ) )
		{
			PFLFileSpecMacMetaInfo	info;
			#define kCDKPrefsCreator	kPGPMacFileCreator_Keys
			#define kCDKPrefsType		'pref'
			
			pgpClearMemory( &info, sizeof( info ) );
			info.fInfo.fdCreator	= kCDKPrefsCreator;
			info.fInfo.fdType		= kCDKPrefsType;
			PFLSetFileSpecMetaInfo( newRef, &info );
		}
	}
	
	*outRef	= newRef;
	
	return( err );
}

#elif PGP_UNIX		/* ] PGP_MACINTOSH [ */

/*____________________________________________________________________________
	Determine where the preference file resides, and create a PFLFileSpecRef
	which locates it.
____________________________________________________________________________*/
	PGPError
pgpGetPrefsSpec(
	PFLContextRef		pflContext,
	PFLFileSpecRef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	const char			kCDKPrefsFileName[] = "PGPsdkPrefs";
	const char *		pgppath1;
	const char *		pgppath2 = "/";
	char *				prefpath;
	
	*outRef		= NULL;
	
	pgppath1	= getenv( "PGPPATH" );
	if( IsNull( pgppath1 ) ) {
		pgppath1 = getenv( "HOME" );
		if( IsNull( pgppath1 ) )
			pgppath1 = ".";
		else
			pgppath2 = "/.pgp/";
	}

	prefpath = PFLContextMemAlloc( pflContext,
								   strlen(pgppath1)+strlen(pgppath2)+
										strlen(kCDKPrefsFileName)+1,
								   0 );
	if( IsNull( prefpath ) )
		return kPGPError_OutOfMemory;
	
	strcpy( prefpath, pgppath1 );
	strcat( prefpath, pgppath2 );
	strcat( prefpath, kCDKPrefsFileName );

	err = PFLNewFileSpecFromFullPath( pflContext, prefpath, outRef );

	PFLContextMemFree( pflContext, prefpath );

	return err;
}

#elif PGP_WIN32	/* ] PGP_UNIX [ */

/*____________________________________________________________________________
	Determine where the preference file resides, and create a PFLFileSpecRef
	which locates it.
____________________________________________________________________________*/
	PGPError
pgpGetPrefsSpec(
	PFLContextRef		pflContext,
	PFLFileSpecRef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	const char			kCDKPrefsFileName[] = "PGPsdk.prf";
	char				winpath[MAX_PATH];
	char *				prefpath;
	
	*outRef		= NULL;
	
	if( GetWindowsDirectory( winpath, sizeof(winpath) ) == 0)
	{
		strcpy( winpath, "." );
	}

	prefpath = (char *)PFLContextMemAlloc( pflContext, strlen(winpath)+1+
								   strlen(kCDKPrefsFileName)+1, 0);
	if( IsNull( prefpath ) )
		return kPGPError_OutOfMemory;
	
	strcpy( prefpath, winpath );
	strcat( prefpath, "\\" );
	strcat( prefpath, kCDKPrefsFileName );

	err = PFLNewFileSpecFromFullPath( pflContext, prefpath, outRef );

	PFLContextMemFree( pflContext, prefpath );

	return err;
}


#else	/* ] PGP_WIN32 [ */
#error Unsupported operating system
#endif	/* ] OTHER OS */




static PGPUInt32	sInitedCount		= 0;

	PGPError
PGPsdkInit( void )
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( sInitedCount == 0 )
	{
		pgpLeaksBeginSession( "PGPsdk" );
		
		err	=  pgpInitMacBinaryMappings();
		pgpAssertNoErr( err );
	}
	
	if ( IsntPGPError( err ) )
	{
		++sInitedCount;
	}
	
	return( err );
}


	PGPBoolean
pgpsdkIsInited( void )
{
	return( sInitedCount != 0 );
}

	PGPError
pgpsdkCleanupForce( void )
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( sInitedCount != 0 )
	{
		sInitedCount	= 1;
	}
	
	err	= PGPsdkCleanup();
	return( err );
}

	PGPError
PGPsdkCleanup( void )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpAssert( sInitedCount != 0 );
	if ( sInitedCount != 0 )
	{
		--sInitedCount;
		if ( sInitedCount == 0 )
		{
			pgpDisposeMacBinaryMappings();
			
			pgpLeaksEndSession();
		}
	}
	else
	{
		err	= kPGPError_BadParams;
	}
	
	return( err );
}











/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
