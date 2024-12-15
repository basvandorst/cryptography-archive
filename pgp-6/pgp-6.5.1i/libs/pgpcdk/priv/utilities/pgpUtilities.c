/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpUtilities.c,v 1.51 1999/05/24 07:48:49 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"
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
#include "MacErrors.h"
#include "MacFiles.h"
#include "MacStrings.h"

#elif PGP_WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif



		


	PGPError
PGPCopyFileSpec(
	PGPFileSpecRef	fileRef,
	PGPFileSpecRef *outRef )
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
	return( PFLNewFileSpecFromFSSpec( PGPGetContextMemoryMgr( context ),
		spec, (PFLFileSpecRef*)outRef ) );
}


	PGPError
PGPGetFSSpecFromFileSpec(
	PGPFileSpecRef	fileRef,
	FSSpec *		spec)
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
	
	err	= PFLNewFileSpecFromFullPath( PGPGetContextMemoryMgr( context ),
		path, (PFLFileSpecRef *)outRef );
	return err;
}



	PGPError 
PGPGetFullPathFromFileSpec(
	PGPFileSpecRef	fileRefIn,
	char **			fullPathPtr)
{
	PGPError			err;
	PGPMemoryMgrRef		memoryMgr	= NULL;
	char *				tempPath	= NULL;
	char *				fullPath	= NULL;
	PFLConstFileSpecRef	fileRef	= (PFLConstFileSpecRef)fileRefIn;
	
	PGPValidatePtr( fullPathPtr );
	*fullPathPtr	= NULL;
	PFLValidateFileSpec( fileRef );
	
	memoryMgr	= PFLGetFileSpecMemoryMgr( fileRef );
	
	err	= PFLGetFullPathFromFileSpec( fileRef, &tempPath );
	if ( IsntPGPError( err ) )
	{
		fullPath	= (char *)
			PGPNewData( memoryMgr, strlen( tempPath ) + 1, 0);
		if ( IsntNull( fullPath ) )
		{
			strcpy( fullPath, tempPath );
		}
		else
		{
			err	= kPGPError_OutOfMemory;
		}
		PGPFreeData( tempPath );
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
	PGPMemoryMgrRef		memoryMgr,
	PFLFileSpecRef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	FSSpec				fsSpec;
	const unsigned char	kCDKPrefsFileName[] = "\pPGPsdkPreferences";
	PFLFileSpecRef		newRef	= NULL;
	
	*outRef		= NULL;
	
	err	= MacErrorToPGPError( FindPGPPreferencesFolder( -1, &fsSpec.vRefNum,
					&fsSpec.parID ) );
	if ( IsntPGPError( err ) )
	{
		CopyPString( kCDKPrefsFileName, fsSpec.name );
	
		err	= PFLNewFileSpecFromFSSpec( memoryMgr, &fsSpec, &newRef );
		if ( IsntPGPError( err ) )
		{
			PFLFileSpecMacMetaInfo		info;
			#define kCDKPrefsCreator	kPGPMacFileCreator_Keys
			#define kCDKPrefsType		'pref'
			
			pgpClearMemory( &info, sizeof( info ) );
			info.fInfo.fileCreator	= kCDKPrefsCreator;
			info.fInfo.fileType		= kCDKPrefsType;
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
	PGPMemoryMgrRef		memoryMgr,
	PFLFileSpecRef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	const char			kCDKPrefsFileName[] = "PGPsdkPreferences";
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

	prefpath = PGPNewData( memoryMgr,
								   strlen(pgppath1)+strlen(pgppath2)+
										strlen(kCDKPrefsFileName)+1,
								   0 );
	if( IsNull( prefpath ) )
		return kPGPError_OutOfMemory;
	
	strcpy( prefpath, pgppath1 );
	strcat( prefpath, pgppath2 );
	strcat( prefpath, kCDKPrefsFileName );

	err = PFLNewFileSpecFromFullPath( memoryMgr, prefpath, outRef );

	PGPFreeData( prefpath );

	return err;
}

#elif PGP_WIN32	/* ] PGP_UNIX [ */

/*____________________________________________________________________________
	Determine where the preference file resides, and create a PFLFileSpecRef
	which locates it.
____________________________________________________________________________*/

	static PGPError
sCreatePath (
		char*	pszPath)
{
	DWORD	dw;
	LPSTR	p;

	dw = GetFileAttributes ( pszPath );
	if (( dw != 0xFFFFFFFF ) &&
		( dw & FILE_ATTRIBUTE_DIRECTORY ))
		return TRUE;

	if (dw != 0xFFFFFFFF)
		return FALSE;

	p = strchr ( pszPath, '\\' );
	while ( p )
	{
		*p = '\0';
		CreateDirectory ( pszPath, NULL );
		*p = '\\';
		p++;
		p = strchr ( p, '\\' );
	}
	return TRUE;
}

	 
	PGPError
pgpGetPrefsSpec(
	PGPMemoryMgrRef		memoryMgr,
	PFLFileSpecRef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	const char			kCDKPrefsDefaultPath[] = "Profiles\\Default User\\";
	const char			kCDKPrefsFilePath[] = "Application Data\\PGP\\";
	const char			kCDKPrefsFileName[] = "PGPsdk.dat";
	char				winpath[MAX_PATH];
	char *				prefpath;

	OSVERSIONINFO		osid;

	*outRef		= NULL;

	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);

	switch (osid.dwPlatformId) {
	// Windows NT, use path based on username
	case VER_PLATFORM_WIN32_NT :
		// get the user profile path (e.g. "C:\WINNT\Profiles\username")
		if( GetEnvironmentVariable (
					"USERPROFILE", winpath, sizeof(winpath) ) == 0 )
		{
			if( GetWindowsDirectory( winpath, sizeof(winpath) ) == 0 )
				strcpy( winpath, "." );

			if( winpath[strlen( winpath ) -1] != '\\' ) 
				strcat (winpath, "\\");

			strcat ( winpath, kCDKPrefsDefaultPath );
		}
		if( winpath[strlen( winpath ) -1] != '\\' ) 
			strcat ( winpath, "\\" );

		// add the location of the PGP files
		strcat ( winpath, kCDKPrefsFilePath );

		// create the path if it doesn't exist
		sCreatePath ( winpath );
		break;

	// otherwise, just use the Windows directory
	default :
		if( GetWindowsDirectory( winpath, sizeof(winpath) ) == 0 )
			strcpy( winpath, ".\\" );
		if( winpath[strlen( winpath ) -1] != '\\' ) 
			strcat (winpath, "\\");
		break;
	}

	prefpath = (char *)PGPNewData( memoryMgr, strlen(winpath)+
								   strlen(kCDKPrefsFileName)+1, 0);
	if( IsNull( prefpath ) )
		return kPGPError_OutOfMemory;
	
	strcpy( prefpath, winpath );
	strcat( prefpath, kCDKPrefsFileName );

	err = PFLNewFileSpecFromFullPath( memoryMgr, prefpath, outRef );

	PGPFreeData( prefpath );

	return err;
}


#else	/* ] PGP_WIN32 [ */
#error Unsupported operating system
#endif	/* ] OTHER OS */





/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
