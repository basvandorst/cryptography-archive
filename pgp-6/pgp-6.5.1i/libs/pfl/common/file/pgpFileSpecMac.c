/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	

	$Id: pgpFileSpecMac.c,v 1.30 1999/05/24 20:24:32 heller Exp $
____________________________________________________________________________*/

#if PGP_MACINTOSH	/* [ */

#include <Aliases.h>
#include <Errors.h>
#include <Folders.h>
#include <Script.h>

#include "pgpPFLConfig.h"

#include <string.h>
#include <stdio.h>
#include <FSp_fopen.h>

#include "pgpDebug.h"
#include "pgpPFLErrors.h"
#include "MacFiles.h"
#include "MacStrings.h"
#include "pgpMem.h"

#include "pgpMemoryMgr.h"

#define PGP_USE_FILE_SPEC_PRIV
#include "PGPFileSpecPriv.h"
#include "PGPFileSpecMacPriv.h"


typedef struct MyData
{
	PGPBoolean		specIsValid;	
	FSSpec			spec;
	
	PFLFileSpecMacMetaInfo	metaInfo;

} MyData;

struct PFLDirectoryIter
{
	PGPMemoryMgrRef	memoryMgr;
	short			vRefNum;
	long			parID;
	long			dirIndex;
};

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

typedef struct
{
	Str31			name;
	PGPUInt32		aliasDataSize;	/* Alias to parent folder of file spec */
	PGPByte			aliasData[1];
	
} ExportedFileSpec;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif


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

	static PGPError
GetSpec(
	PFLConstFileSpecRef	ref,
	FSSpec				*fileSpec,
	CInfoPBRec			*fileInfo)
{
	MyData 		*myData;
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateParam( ref->type == kPFLFileSpecMacType );

	myData = GetMyData( ref );
	if( myData->specIsValid )
	{
		*fileSpec = myData->spec;
		
		if( fileInfo != NULL )
		{
			OSStatus	macErr;
			
			macErr = FSpGetCatInfo( fileSpec, fileInfo );
			if ( macErr == fnfErr )
			{
				err	= kPGPError_FileNotFound;
			}
			else if( macErr != noErr )
			{
				err	= kPGPError_FileOpFailed;
			}
		}	
	}
	else
	{
		err = kPGPError_FileNotFound;
	}
	
	return( err );
}

	PGPError
pgpPlatformGetFileInfo(
	PFLConstFileSpecRef	ref,
	PFLFileInfo *		outInfo )
{
	PGPError	err = kPGPError_NoErr;
	FSSpec		spec;
	CInfoPBRec	cpb;
	
	PGPValidateParam( ref->type == kPFLFileSpecMacType );

	err = GetSpec( ref, &spec, &cpb );
	if( IsntPGPError( err ) )
	{
		time_t			nowStdTime = time( NULL );
		unsigned long	nowMacTime;
			
		if( cpbIsFolder( &cpb ) )
		{
			outInfo->flags 		= kPGPFileInfo_IsDirectory;
			outInfo->dataLength = 0;
		}
		else
		{
			outInfo->flags 		= kPGPFileInfo_IsPlainFile;
			outInfo->dataLength = cpbDataForkSize( &cpb );
		}
		
		GetDateTime( &nowMacTime );
		
		outInfo->modificationTime = (time_t)(cpbModificationDate( &cpb )
							+ ((unsigned long)nowStdTime - nowMacTime));
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
	PGPMemoryMgrRef			memoryMgr	= parentDir->memoryMgr;
	PGPError				err		= kPGPError_NoErr;
	FSSpec					spec;
	PFLDirectoryIterRef		newIter	= NULL;
	CInfoPBRec				cpb;

	*outIter = NULL;
	PGPValidateParam( parentDir->type == kPFLFileSpecMacType );
	PGPValidateMemoryMgr( memoryMgr );

	err = GetSpec( parentDir, &spec, &cpb );
	if( IsntPGPError( err ) )
	{
		newIter = (PFLDirectoryIterRef)PGPNewData( memoryMgr,
											sizeof( *newIter ), 0 );
		if ( IsntNull( newIter ) )
		{
			pgpAssert( cpbIsFolder( &cpb ) );

			newIter->memoryMgr	= memoryMgr;
			newIter->vRefNum	= spec.vRefNum;
			newIter->parID		= cpb.dirInfo.ioDrDirID;
			newIter->dirIndex	= 1;
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}
	}
	
	if ( IsntPGPError( err ) )
		*outIter = newIter;
	else if ( IsntNull( newIter ) )
		PGPFreeData( newIter );

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
		err = PFLNewFileSpecFromFSSpec( iter->memoryMgr, &spec, outRef );
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

	PGPFreeData( iter );
	return kPGPError_NoErr;
}

	static PGPError
sExportProc(
	PFLConstFileSpecRef		ref,
	PGPByte **				dataOut,
	PGPSize *				dataSizeOut )
{
	MyData *			myData;
	PGPError			err				= kPGPError_NoErr;
	AliasHandle			alias			= NULL;
	void				*aliasData		= NULL;
	PGPUInt32			aliasDataSize	= 0;
	PGPByte *			data 			= NULL;
	PGPUInt32			dataSize		= 0;
	
	myData = GetMyData( ref );
	
	if( myData->specIsValid )
	{
		CInfoPBRec	cpb;
		FSSpec		parFolderSpec;
		
		parFolderSpec = myData->spec;
		
		pgpClearMemory( &cpb, sizeof( cpb ) );
		
		cpb.dirInfo.ioVRefNum	= parFolderSpec.vRefNum;
		cpb.dirInfo.ioDrDirID	= parFolderSpec.parID;
		cpb.dirInfo.ioNamePtr	= parFolderSpec.name;
		cpb.dirInfo.ioFDirIndex	= -1;
		
		if( PBGetCatInfoSync( &cpb ) == noErr )
		{
			parFolderSpec.parID	= cpb.dirInfo.ioDrParID;

			if( NewAlias( NULL, &parFolderSpec, &alias ) == noErr )
			{
				HLock( (Handle) alias );
				
				aliasData		= *alias;
				aliasDataSize 	= GetHandleSize( (Handle) alias );
			}
		}
	}
	
	dataSize = sizeof( ExportedFileSpec ) + aliasDataSize;
	
	data = (PGPByte *) PGPNewData( ref->memoryMgr, dataSize, 0 );
	if ( IsntNull( data ) )
	{
		ExportedFileSpec	*exportedData = (ExportedFileSpec *) data;
		
		exportedData->aliasDataSize	= aliasDataSize;
		
		CopyPString( myData->spec.name, exportedData->name );
		
		if( aliasData != NULL )
			pgpCopyMemory( aliasData, &exportedData->aliasData[0], aliasDataSize );
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	if( alias != NULL )
		DisposeHandle( (Handle) alias );
		
	*dataOut		= data;
	*dataSizeOut	= dataSize;
	
	return( err );
}

	static PGPError
sImportProc(
	PFLFileSpecRef		ref,
	PGPByte const *		data,
	PGPSize				dataSize )
{
	const ExportedFileSpec *	exportedData;
	MyData *					newData	= NULL;
	PGPError					err	= kPGPError_NoErr;

	(void) dataSize;
	
	exportedData	= (const ExportedFileSpec *)data;
	newData			= GetMyData( ref );
	
	err	= PGPReallocData( ref->memoryMgr,
			&newData, sizeof( *newData ), 0 );
	if ( IsntPGPError( err ) )
	{
		MyData *		myData	= (MyData *)newData;
		
		ref->data		= (PGPByte *)newData;
		ref->dataSize	= sizeof( *newData );
		
		myData->specIsValid 	= FALSE;
		
		CopyPString( exportedData->name, myData->spec.name );
		
		if( exportedData->aliasDataSize != 0 )
		{
			AliasHandle	alias;
			
			if( PtrToHand( &exportedData->aliasData[0], (Handle *) &alias,
						exportedData->aliasDataSize ) == noErr )
			{
				FSSpec	spec;
				short	aliasCount = 1;
				Boolean	needsUpdate;
				
				if( MatchAlias( NULL, kARMNoUI | kARMSearch,
						alias, &aliasCount, &spec, &needsUpdate, NULL, NULL ) == noErr )
				{
					CInfoPBRec	cpb;
					
					if( FSpGetCatInfo( &spec, &cpb ) == noErr )
					{
						myData->specIsValid = TRUE;
					
						myData->spec.vRefNum 	= spec.vRefNum;
						myData->spec.parID 		= cpb.dirInfo.ioDrDirID;
					}
				}
				
				DisposeHandle( (Handle) alias );
			}
		}
	}
	
	return( err );
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
	pgpAssert( info->fInfo.reserved == 0 );
	
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
	
	if ( myData->specIsValid )
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

	fileCreator	= myData->metaInfo.fInfo.fileCreator;
	fileType	= myData->metaInfo.fInfo.fileType;
	
	if( myData->specIsValid )
	{
		macErr = FSpCreate( &myData->spec, fileCreator, fileType, smSystemScript );
		if ( macErr != noErr )
		{
			err	= kPGPError_FileOpFailed;
		}
	}
	else
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
	
	if ( myData->specIsValid )
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
	
	if ( myData->specIsValid )
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
	
	if( myData->specIsValid )
	{
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
			err = PFLNewFileSpecFromFSSpec( ref->memoryMgr, &spec, outParent );
		}
	}
	else
	{
		err = kPGPError_FileNotFound;
	}
	
	return( err );
}

	static PGPError
sComposeProc(
	PFLConstFileSpecRef	parent,
	const char *		fileName,
	PFLFileSpecRef *	outRef )
{
	PGPError	err		= kPGPError_NoErr;
	FSSpec		spec;
	PGPSize		nameLength;
	CInfoPBRec	cpb;
	
	PGPValidateParam( parent->type == kPFLFileSpecMacType );

	err = GetSpec( parent, &spec, &cpb );
	if( IsntPGPError( err ) )
	{
		nameLength = strlen( fileName );
		if ( nameLength > sizeof( spec.name ) - 1 )
		{
			err = kPGPError_BufferTooSmall;
		}
		else
		{
			pgpCopyMemory( fileName, spec.name + 1, nameLength );
			spec.name[0] 	= nameLength;
			spec.vRefNum 	= cpb.dirInfo.ioVRefNum;
			spec.parID 		= cpb.dirInfo.ioDrDirID;
			
			err = PFLNewFileSpecFromFSSpec( parent->memoryMgr, &spec,
											outRef );
		}
	}
	
	return( err );
}


	PGPError 
PFLNewFileSpecFromFSSpec(
	PGPMemoryMgrRef		memoryMgr,
	FSSpec const *		spec,
	PFLFileSpecRef *	outRef )
{
	PFLFileSpecRef	newFileRef	= NULL;
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateMemoryMgr( memoryMgr );
	PGPValidatePtr( spec );
	PGPValidateParam( spec->vRefNum < 0 && spec->parID != 0 &&
		spec->name[ 0 ] != 0 && spec->name[ 0 ] <= 31 );
	
	err	= pgpNewFileSpec( memoryMgr, kPFLFileSpecMacType,
		sizeof( MyData ), &newFileRef );
	if ( IsntPGPError( err ) )
	{
		MyData *	myData	= GetMyData( newFileRef );
		
		pgpClearMemory( myData, sizeof( *myData ) );
		myData->spec			= *spec;
		
		/* volume info not used if vRefNumIsValid */
		myData->specIsValid	= TRUE;

		if( FSpGetFInfo( spec, (FInfo *) &myData->metaInfo.fInfo ) != noErr )
		{
			myData->metaInfo.fInfo.fileType 		= 'BINA';
			myData->metaInfo.fInfo.fileCreator 		= '????';
			myData->metaInfo.fInfo.finderFlags 		= 0;
			myData->metaInfo.fInfo.location.h 		= -1;
			myData->metaInfo.fInfo.location.v 		= -1;
			myData->metaInfo.fInfo.reserved			= 0;
		}
	}
	
	*outRef	= newFileRef;
	return err;
}

	PGPError
PFLGetFSSpecFromFileSpec(
	PFLConstFileSpecRef		ref,
	FSSpec *				spec)
{
	PGPError	err	= kPGPError_NoErr;
		
	PGPValidatePtr( spec );
	pgpClearMemory( spec, sizeof( *spec ) );
	PFLValidateFileSpec( ref );

	err = GetSpec( ref, spec, NULL );
	
	return err;
}


	PGPError
pgpPlatformGetTempFileSpec(
	PGPMemoryMgrRef			memoryMgr,
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
			err	= PFLNewFileSpecFromFSSpec( memoryMgr, &tempSpec, outRef );
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
	
	PGPValidatePtr( fileOut );
	*fileOut	= NULL;
	PFLValidateFileSpec( spec );
	PGPValidatePtr( openMode );
	PGPValidateParam( spec->type == kPFLFileSpecMacType );
	
	err	= PFLGetFSSpecFromFileSpec( spec, &fsSpec );
	if ( IsntPGPError( err ) )
	{
		stdioFILE	= FSp_fopen( &fsSpec, openMode );
		if ( IsNull( stdioFILE ) )
			err	= kPGPError_FileNotFound;
	}
		
	*fileOut = stdioFILE;
	
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
