/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	

	$Id: pgpFileSpecMac.c,v 1.19 1997/09/30 20:00:14 heller Exp $
____________________________________________________________________________*/

#if PGP_MACINTOSH	/* [ */

#include "pgpPFLConfig.h"

#include <string.h>
#include <stdio.h>
#include <FSp_fopen.h>

#include "pgpDebug.h"
#include "pgpPFLErrors.h"
#include "MacFiles.h"
#include "MacStrings.h"
#include "pgpMem.h"

#include "pflContext.h"

#define PGP_USE_FILE_SPEC_PRIV
#include "PGPFileSpecPriv.h"
#include "PGPFileSpecMacPriv.h"


typedef struct MyData
{
	/* in memory state can refer to a real volume or one
	that is not mounted.  If the volume is not available
	then the spec contains the parID and name, but vRefNum
	is set to kInvalidVRefNum.  The 'nm' struct contains
	info needed to get the vRefNum later.
	*/
	PGPBoolean		vRefNumIsValid;	
	struct
	{		
		long			volumeCreationDate;
		unsigned char	volumeName[ 32 ];
	} volumeInfo;
	
	FSSpec		spec;
	PFLFileSpecMacMetaInfo	metaInfo;
} MyData;
#define kInvalidVRefNum		1	/* boot volume */


struct PFLDirectoryIter
{
	PFLContextRef	context;
	short			vRefNum;
	long			parID;
	long			dirIndex;
};


static PGPError	ResolveVolume( MyData *myData );


#define GetMyData( ref )	( (MyData *)(ref)->data )

static PGPError	sExportProc( PFLConstFileSpecRef ref,
					PGPByte **data, PGPSize *dataSize );
static PGPError	sImportProc( PFLFileSpecRef ref,
					PGPByte const *data, PGPSize dataSize );
static PGPError	sGetNameProc( PFLConstFileSpecRef ref, char name[ 256 ] );
static PGPError	sSetNameProc( PFLFileSpecRef ref, char const *name );
static PGPError	sSetMetaInfoProc( PFLFileSpecRef ref, void const *info );
static PGPError	sExistsProc( PFLFileSpecRef ref, PGPBoolean *exists);
static PGPError	sCreateProc( PFLFileSpecRef ref );
static PGPError	sDeleteProc( PFLConstFileSpecRef ref );
static PGPError	sRenameProc( PFLFileSpecRef ref, const char *newName);
static PGPError	sGetMaxNameLengthProc( PFLConstFileSpecRef ref,
					PGPSize * maxNameLength);
static PGPError	sParentDirProc( PFLConstFileSpecRef fileFromDir,
					PFLFileSpecRef *outParent );
static PGPError	sComposeProc( PFLConstFileSpecRef parent, char const *name,
					PFLFileSpecRef *outRef );


static const PGPFileSpecVTBL	sVTBL =
{
	sExportProc,
	sImportProc,
	sGetNameProc,
	sSetNameProc,
	
	sSetMetaInfoProc,
	sExistsProc,
	sCreateProc,
	sDeleteProc,
	sRenameProc,
	sGetMaxNameLengthProc,
	sParentDirProc,
	sComposeProc
};
	
	
	PGPFileSpecVTBL const *
GetFileSpecVTBLMac()
{
	return( &sVTBL );
}


#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif
/* note: since this is mac-specific,
we can get away with writing it out as a whole struct */
typedef struct
{
	FSSpec			spec;
	long			volumeCreationDate;
	unsigned char	volumeName[ 64 ];
} ExportedFileSpec;
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

	PGPError
pgpPlatformGetFileInfo(
	PFLConstFileSpecRef	ref,
	PFLFileInfo *		outInfo )
{
	PGPError		err		= kPGPError_NoErr;
	MyData *		myData;
	FSSpec			spec;
	HFileInfo		pb;

	PGPValidateParam( ref->type == kPFLFileSpecMacType );

	myData	= GetMyData( ref );
	spec	= myData->spec;
	(void)ResolveVolume( myData );
	
	pb.ioNamePtr = spec.name;
	pb.ioVRefNum = spec.vRefNum;
	pb.ioDirID = spec.parID;
	pb.ioFDirIndex = 0;
	
	if ( PBGetCatInfoSync( (CInfoPBPtr)&pb ) != noErr )
	{
		if ( pb.ioResult == fnfErr )
			err	= kPGPError_FileNotFound;
		else
			err	= kPGPError_FileOpFailed;
	}
	else
	{
		/* Check directory bit: XXX couldn't find a #define for these masks */
		if ( pb.ioFlAttrib & 0x10 )
		{
			outInfo->flags = kPGPFileInfo_IsDirectory;
			outInfo->dataLength = 0;
		}
		else
		{
			outInfo->flags = kPGPFileInfo_IsPlainFile;
			outInfo->dataLength = pb.ioFlLgLen;
		}
		{
			time_t			nowStdTime = time( NULL );
			unsigned long	nowMacTime;
			
			GetDateTime( &nowMacTime );
			outInfo->modificationTime = (time_t)(pb.ioFlMdDat
							+ ((unsigned long)nowStdTime - nowMacTime));
		}
	}
	
	return err;
}

	PGPError
pgpPlatformLockFILE(
	FILE *				file,
	PFLFileOpenFlags	flags )
{
	/* Nothing to do, normal file locking should be enough */

	(void)file;		/* Avoid warnings */
	(void)flags;

	return kPGPError_NoErr;
}

	PGPError
pgpPlatformNewDirectoryIter(
	PFLConstFileSpecRef		parentDir,
	PFLDirectoryIterRef *	outIter )
{
	PFLContextRef			context	= parentDir->context;
	PGPError				err		= kPGPError_NoErr;
	MyData *				myData;
	FSSpec					spec;
	PFLDirectoryIterRef		newIter	= NULL;

	*outIter = NULL;
	PGPValidateParam( parentDir->type == kPFLFileSpecMacType );
	PGPValidateParam( PFLContextIsValid( context ) );

	myData	= GetMyData( parentDir );
	spec	= myData->spec;
	(void)ResolveVolume( myData );

	newIter = (PFLDirectoryIterRef)PFLContextMemAlloc( context,
										sizeof( *newIter ), 0 );
	if ( IsntNull( newIter ) )
	{
		HFileInfo		pb;
		
		pb.ioNamePtr = spec.name;
		pb.ioVRefNum = spec.vRefNum;
		pb.ioDirID = spec.parID;
		pb.ioFDirIndex = 0;
		if ( PBGetCatInfoSync( (CInfoPBPtr)&pb ) != noErr )
		{
			if ( pb.ioResult == fnfErr )
				err	= kPGPError_FileNotFound;
			else
				err	= kPGPError_FileOpFailed;
		}
		else
		{
			newIter->context	= context;
			newIter->vRefNum	= spec.vRefNum;
			newIter->parID		= pb.ioDirID;
			newIter->dirIndex	= 1;
		}
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}
	
	if ( IsntPGPError( err ) )
		*outIter = newIter;
	else if ( IsntNull( newIter ) )
		PFLContextMemFree( context, newIter );

	return err;
}

	PGPError
pgpPlatformNextFileInDirectory(
	PFLDirectoryIterRef		iter,
	PFLFileSpecRef *		outRef )
{
	PGPError		err = kPGPError_NoErr;
	FSSpec			spec;
	HFileInfo		pb;

	*outRef = NULL;
	
	pb.ioNamePtr = spec.name;
	pb.ioVRefNum = iter->vRefNum;
	pb.ioDirID = iter->parID;
	pb.ioFDirIndex = iter->dirIndex;
	if ( PBGetCatInfoSync( (CInfoPBPtr)&pb ) != noErr )
	{
		if ( pb.ioResult == fnfErr )
			err	= kPGPError_EndOfIteration;
		else
			err	= kPGPError_FileOpFailed;
	}
	else
	{
		spec.vRefNum = iter->vRefNum;
		spec.parID = iter->parID;
		err = PFLNewFileSpecFromFSSpec( iter->context, &spec, outRef );
	}
	
	if ( IsntPGPError( err ) )
		iter->dirIndex++;

	return err;
}

	PGPError
pgpPlatformFreeDirectoryIter(
	PFLDirectoryIterRef		iter )
{
	PGPValidatePtr( iter );

	PFLContextMemFree( iter->context, iter );
	return kPGPError_NoErr;
}

	static PGPError
sExportProc(
	PFLConstFileSpecRef			ref,
	PGPByte **					dataOut,
	PGPSize *					dataSizeOut )
{
	MyData *			myData;
	FSSpec const *		spec;
	ExportedFileSpec	exported;
	PGPByte *			data	= NULL;
	PGPUInt32			dataSize	= sizeof( exported );
	PGPError			err	= kPGPError_NoErr;
	PGPBoolean			haveVolume	= FALSE;

	myData	= GetMyData( ref );
	spec	= &myData->spec;
	
	(void)ResolveVolume( myData );
	
	exported.spec	= *spec;
	
	haveVolume	= myData->vRefNumIsValid;
	if ( haveVolume )
	{
		err	= GetVolumeName( spec->vRefNum, exported.volumeName);
		if ( IsntPGPError( err ) )
		{
			err	= GetVolumeDates( spec->vRefNum,
				&exported.volumeCreationDate, NULL );
		}
	}
	else
	{
		pgpAssert( myData->volumeInfo.volumeCreationDate != 0 );
		pgpAssert( StrLength( myData->volumeInfo.volumeName ) != 0 );
		pgpAssert( StrLength( myData->volumeInfo.volumeName ) <= 27 );
		
		CopyPString( myData->volumeInfo.volumeName, exported.volumeName);
		exported.volumeCreationDate	= myData->volumeInfo.volumeCreationDate;
	}
	
	if ( IsntPGPError( err ) )
	{
		data = (PGPByte *)
			PFLContextMemAlloc( ref->context, dataSize, 0);
			
		if ( IsntNull( data ) )
		{
			pgpCopyMemory( &exported, data, dataSize);
			*dataSizeOut	= dataSize;
		}
		else
		{
			err	= kPGPError_OutOfMemory;
		}
	}
	else
	{
		err	= kPGPError_BadParams;
	}
	
	*dataOut		= data;
	*dataSizeOut	= dataSize;
	return( err);
}



	static PGPError
FindVRefNum(
	long			volumeCreationDate,
	const uchar *	volumeName,
	short *			vRefNumOut )
{
	PGPUInt32		volIndex;
	short			vRefNums[ 100 ];
	unsigned short	numVRefNums;
	PGPBoolean		foundIt	= false;
	OSStatus		macErr;
	PGPError		err	= kPGPError_NoErr;
	
	numVRefNums	= sizeof( vRefNums ) / sizeof( vRefNums[ 0 ] );
	GetAllVRefNums( vRefNums, numVRefNums, &numVRefNums );
	for( volIndex = 0; volIndex < numVRefNums; ++volIndex )
	{
		long	creationDate;
		short	vRefNum	= vRefNums[ volIndex ];
		Str255	vName;
		
		macErr	= GetVolumeName( vRefNum, vName );
		if ( macErr == noErr )
		{
			macErr	= GetVolumeDates( vRefNum, &creationDate, NULL );
		}
		
		if ( macErr == noErr &&
			creationDate == volumeCreationDate )
		{
			foundIt	= TRUE;
			*vRefNumOut	= vRefNum;
				
			if ( PStringsAreEqual( volumeName, vName) )
			{
				/* both match, that's as good as we get */
				/* otherwise, continue search for matching name */
				break;
			}
		}
	}
	
	if ( ! foundIt  )
	{
		err	= kPGPError_FileNotFound;
	}
	return( err );
}


	static PGPError
ResolveVolume( MyData *myData)
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( ! myData->vRefNumIsValid )
	{
		short	vRefNum;
		
		err	= FindVRefNum( myData->volumeInfo.volumeCreationDate,
						myData->volumeInfo.volumeName, &vRefNum);
		if ( IsntPGPError( err ) )
		{
			myData->vRefNumIsValid	= TRUE;
			myData->spec.vRefNum	= vRefNum;
		}
	}
	
	return( err );
}


	static PGPError
sImportProc(
	PFLFileSpecRef		ref,
	PGPByte const *		data,
	PGPSize				dataSize )
{
	const ExportedFileSpec *	exported;
	MyData *					newData	= NULL;
	PGPError					err	= kPGPError_NoErr;

	PGPValidateParam( dataSize == sizeof( ExportedFileSpec ) );
	
	exported	= (const ExportedFileSpec *)data;
	newData		= GetMyData( ref );
	
	err	= PFLContextMemRealloc( ref->context,
			&newData, sizeof( *newData ), 0 );
	if ( IsntPGPError( err ) )
	{
		MyData *		myData	= (MyData *)newData;
		
		ref->data				= (PGPByte *)newData;
		ref->dataSize			= sizeof( *newData );
		
		myData->spec				= exported->spec;
		myData->spec.vRefNum		= kInvalidVRefNum;
		
		/* stroe info for use by ResolveSpec */
		myData->vRefNumIsValid	= FALSE;
		myData->volumeInfo.volumeCreationDate	= exported->volumeCreationDate;
		CopyPString( exported->volumeName, myData->volumeInfo.volumeName );
		
		(void)ResolveVolume( myData );
	}
	
	return( err);
}


	static PGPError
sGetNameProc(
	PFLConstFileSpecRef	ref,
	char				name[ 256 ] )
{
	const MyData *	myData	= GetMyData( ref );
	const FSSpec *	spec;
	
	PGPValidatePtr( myData );
	spec	= &myData->spec;
	PGPValidateParam( StrLength( spec->name ) <= 31 );
	
	pgpCopyMemory( &spec->name[ 1 ], name, StrLength( spec->name ) );
	name[ StrLength( spec->name ) ]	= '\0';
	
	return( kPGPError_NoErr );
}


	static PGPError
sSetNameProc(
	PFLFileSpecRef		ref,
	char const *		name )
{
	PGPError	err		= kPGPError_NoErr;
	MyData *	myData	= GetMyData( ref );
	PGPUInt32	nameLength;
	
	PGPValidatePtr( myData );
	nameLength	= strlen( name );
	PGPValidateParam( nameLength <= sizeof( myData->spec.name ) - 1 );
	
	if ( nameLength > sizeof( myData->spec.name ) - 1 )
	{
		err = kPGPError_BufferTooSmall;
	}
	else
	{
		pgpCopyMemory( name, &myData->spec.name[ 1 ], nameLength );
		myData->spec.name[ 0 ]	= nameLength;
	}
	
	return( err );

}


	static PGPError
sGetMaxNameLengthProc(
	PFLConstFileSpecRef	ref,
	PGPSize *		maxNameLength )
{
	const MyData *	myData	= GetMyData( ref );
	
	PGPValidatePtr( myData );
	
	/* at some point, this will need to be done programmatically by
	examining the file system in use which the file is  on */
	*maxNameLength	= kMaxHFSFileNameLength;
	
	return( kPGPError_NoErr );
}



	static PGPError
sSetMetaInfoProc(
	PFLFileSpecRef	ref,
	void const *	infoIn )
{
	PGPError	err	= kPGPError_NoErr;
	MyData *	myData	= GetMyData( ref );
	PFLFileSpecMacMetaInfo const *	info;
	
	info	= (PFLFileSpecMacMetaInfo const *)infoIn;
	
	myData->metaInfo	= *info;
	
	return( err );
}


	static PGPError
sExistsProc(
	PFLFileSpecRef	ref,
	PGPBoolean *	exists)
{
	PGPError	err	= kPGPError_NoErr;
	MyData *	myData	= GetMyData( ref );
	FInfo		fInfo;
	
	(void)ResolveVolume( myData );
	if ( myData->vRefNumIsValid )
	{
		err		= FSpGetFInfo( &myData->spec, &fInfo );
		*exists	= ( err == noErr );
	}
	else
	{
		*exists	= FALSE;
	}
	
	return( kPGPError_NoErr );
}

	static PGPError
sCreateProc( PFLFileSpecRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	OSStatus		macErr	= noErr;
	MyData *		myData	= GetMyData( ref );
	OSType			fileCreator;
	OSType			fileType;

	(void)ResolveVolume( myData );
	
	fileCreator	= myData->metaInfo.fInfo.fdCreator;
	fileType	= myData->metaInfo.fInfo.fdType;
	
	macErr	= FSpCreate( &myData->spec, fileCreator, fileType, smSystemScript );
	if ( macErr != noErr )
	{
		err	= kPGPError_FileOpFailed;
	}
	
	return( err );
}


	static PGPError
sDeleteProc( PFLConstFileSpecRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	MyData *		myData	= GetMyData( ref );
	
	(void)ResolveVolume( myData );
	if ( myData->vRefNumIsValid )
	{
		err	= FSpDelete( &myData->spec );
		if ( err == fnfErr )
			err	= kPGPError_FileNotFound;
		else if ( IsPGPError( err ) )
			err	= kPGPError_FileOpFailed;
	}
	else
	{
		err	= kPGPError_FileNotFound;
	}
	
	return( err );
}


	static PGPError
sRenameProc(
	PFLFileSpecRef	ref,
	const char *	newName)
{
	PGPError		err	= kPGPError_NoErr;
	MyData *		myData	= GetMyData( ref );
	PGPUInt32		nameLength;
	Str255			pascalName;
	
	(void)ResolveVolume( myData );
	if ( myData->vRefNumIsValid )
	{
		
		nameLength	= strlen( newName );
		PGPValidateParam( nameLength <= 31 );
		
		pgpCopyMemory( newName, &pascalName[ 1 ], nameLength );
		pascalName[ 0 ]	= nameLength;
		
		err	= FSpRename( &myData->spec, pascalName );
		if ( IsntPGPError( err ) )
		{
			err	= sSetNameProc( ref, newName );
		}
		else if ( err == fnfErr )
			err	= kPGPError_FileNotFound;
		else
			err	= kPGPError_FileOpFailed;
	}
	else
	{
		err	= kPGPError_FileNotFound;
	}
	
	return( err );
}

	static PGPError
sParentDirProc(
	PFLConstFileSpecRef	ref,
	PFLFileSpecRef *	outParent )
{
	PGPError		err		= kPGPError_NoErr;
	MyData *		myData;
	FSSpec			spec;
	HFileInfo		pb;

	PGPValidateParam( ref->type == kPFLFileSpecMacType );

	myData	= GetMyData( ref );
	spec	= myData->spec;
	(void)ResolveVolume( myData );
	
	pb.ioNamePtr = spec.name;
	pb.ioVRefNum = spec.vRefNum;
	pb.ioDirID = spec.parID;
	pb.ioFDirIndex = -1;
	
	if ( PBGetCatInfoSync( (CInfoPBPtr)&pb ) != noErr )
	{
		if ( pb.ioResult == fnfErr )
			err	= kPGPError_FileNotFound;
		else
			err	= kPGPError_FileOpFailed;
	}
	else
	{
		spec.vRefNum = pb.ioVRefNum;
		spec.parID = pb.ioFlParID;
		err = PFLNewFileSpecFromFSSpec( ref->context, &spec, outParent );
	}
	
	return( err );
}

	static PGPError
sComposeProc(
	PFLConstFileSpecRef	parent,
	const char *		fileName,
	PFLFileSpecRef *	outRef )
{
	PGPError		err		= kPGPError_NoErr;
	MyData *		myData;
	FSSpec			spec;
	HFileInfo		pb;
	PGPSize			nameLength;

	PGPValidateParam( parent->type == kPFLFileSpecMacType );

	myData	= GetMyData( parent );
	spec	= myData->spec;
	(void)ResolveVolume( myData );
	
	pb.ioNamePtr = spec.name;
	pb.ioVRefNum = spec.vRefNum;
	pb.ioDirID = spec.parID;
	pb.ioFDirIndex = 0;
	
	if ( PBGetCatInfoSync( (CInfoPBPtr)&pb ) != noErr )
	{
		if ( pb.ioResult == fnfErr )
			err	= kPGPError_FileNotFound;
		else
			err	= kPGPError_FileOpFailed;
	}
	else
	{
		nameLength = strlen( fileName );
		if ( nameLength > sizeof( spec.name ) - 1 )
		{
			err = kPGPError_BufferTooSmall;
		}
		else
		{
			pgpCopyMemory( fileName, spec.name + 1, nameLength );
			spec.name[0] = nameLength;
			spec.vRefNum = pb.ioVRefNum;
			spec.parID = pb.ioDirID;
			err = PFLNewFileSpecFromFSSpec( parent->context, &spec,
											outRef );
		}
	}
	
	return( err );
}


	PGPError 
PFLNewFileSpecFromFSSpec(
	PFLContextRef		context,
	FSSpec const *		spec,
	PFLFileSpecRef *	outRef )
{
	PFLFileSpecRef	newFileRef	= NULL;
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PFLValidateContext( context );
	PGPValidatePtr( spec );
	PGPValidateParam( spec->vRefNum < 0 && spec->parID != 0 &&
		spec->name[ 0 ] != 0 && spec->name[ 0 ] <= 31 );
	
	err	= pgpNewFileSpec( context, kPFLFileSpecMacType,
		sizeof( MyData ), &newFileRef );
	if ( IsntPGPError( err ) )
	{
		MyData *	myData	= GetMyData( newFileRef );
		
		pgpClearMemory( myData, sizeof( *myData ) );
		myData->spec			= *spec;
		
		/* volume info not used if vRefNumIsValid */
		myData->vRefNumIsValid	= TRUE;
		pgpClearMemory( &myData->volumeInfo, sizeof( myData->volumeInfo ) );
		
		(void)FSpGetFInfo( spec, &myData->metaInfo.fInfo );
	}
	
	*outRef	= newFileRef;
	return err;
}

	PGPError
PFLGetFSSpecFromFileSpec(
	PFLConstFileSpecRef		ref,
	FSSpec *				spec)
{
	MyData *	myData;
	PGPError	err	= kPGPError_NoErr;
		
	PGPValidatePtr( spec );
	pgpClearMemory( spec, sizeof( *spec ) );
	PFLValidateFileSpec( ref );

	myData	= GetMyData( ref );
	(void)ResolveVolume( myData );
	pgpAssert( ref->type == kPFLFileSpecMacType );
	
	/* name, parID may be useful even if vRefNum wrong */
	*spec	= myData->spec;
	
	if ( ! myData->vRefNumIsValid )
	{
		err		= kPGPError_FileNotFound;
		pgpAssert( spec->vRefNum == kInvalidVRefNum );
	}
	
	return err;
}


	PGPError
pgpPlatformGetTempFileSpec(
	PFLContextRef			context,
	PFLConstFileSpecRef		optionalRef,
	PFLFileSpecRef *		outRef )
{
	PGPError				err	= kPGPError_NoErr;
	OSStatus				macErr	= noErr;
	FSSpec					tempSpec;
	const unsigned char	*	kTempFileBase	= "\pPGPTemp";
	
	if ( IsntNull( optionalRef ) )
	{
		err	= PFLGetFSSpecFromFileSpec( optionalRef, &tempSpec );
	}
	else
	{
		macErr	= FindFolder( kOnSystemDisk, kTemporaryFolderType,
					kCreateFolder, &tempSpec.vRefNum, &tempSpec.parID);
		if ( macErr != noErr )
			err	= kPGPError_FileOpFailed;
	}
	
	if ( IsntPGPError( err ) )
	{
		CopyPString( kTempFileBase, tempSpec.name );
		macErr	= FSpGetUniqueSpec( &tempSpec, &tempSpec );
		if ( macErr == noErr )
			err	= PFLNewFileSpecFromFSSpec( context, &tempSpec, outRef );
		else
			err	= kPGPError_FileOpFailed;
	}
	
	return( err );
}



	PGPError
pgpPlatformOpenFileSpecAsFILE(
	PFLFileSpecRef	spec,
	const char *	openMode,
	FILE **			fileOut )
{
	PGPError		err	= kPGPError_NoErr;
	FILE *			stdioFILE	= NULL;
	FSSpec			fsSpec;
	MyData *		myData	= NULL;
	
	PGPValidatePtr( fileOut );
	*fileOut	= NULL;
	PFLValidateFileSpec( spec );
	PGPValidatePtr( openMode );
	PGPValidateParam( spec->type == kPFLFileSpecMacType );
	
	myData	= GetMyData( spec );
	
	if ( myData->vRefNumIsValid )
	{
		err	= PFLGetFSSpecFromFileSpec( spec, &fsSpec );
		if ( IsntPGPError( err ) )
		{
			stdioFILE	= FSp_fopen( &fsSpec, openMode );
			if ( IsNull( stdioFILE ) )
				err	= kPGPError_FileOpFailed;
		}
	}
	else
	{
		stdioFILE	= NULL;
		err	= kPGPError_FileNotFound;
	}
		
	*fileOut	= stdioFILE;
	
	return( err );
}


#endif	/* ] PGP_MACINTOSH */


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
