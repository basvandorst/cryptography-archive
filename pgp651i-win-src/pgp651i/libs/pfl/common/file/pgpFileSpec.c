/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	

	$Id: pgpFileSpec.c,v 1.33 1999/05/17 07:53:20 heller Exp $
____________________________________________________________________________*/

#include "pgpPFLConfig.h"

#include <string.h>

#include "pgpDebug.h"
#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pgpMemoryMgr.h"
#include "pgpStrings.h"


#define PGP_USE_FILE_SPEC_PRIV
#include "pgpFileSpecPriv.h"

#include "pgpFileSpecVTBL.h"


#if !defined(_PATH_TMP) && PGP_UNIX && HAVE_MKSTEMP
#define _PATH_TMP	"/tmp/"
#endif



#define PFLFileSpecData(fileRef, type)	((type *)( fileRef->data ) )

	PGPBoolean
pflFileSpecIsValid( PFLConstFileSpecRef	ref )
{
	return( IsntNull( ref ) && ref->magic == kPFLFileSpecMagic );
}



	PGPMemoryMgrRef
PFLGetFileSpecMemoryMgr(
	PFLConstFileSpecRef fileRef)
{
	if ( ! pflFileSpecIsValid( fileRef ) )
		return( NULL );
	
	return( fileRef->memoryMgr );
}


	PGPError
pgpNewFileSpec(
	PGPMemoryMgrRef		memoryMgr,
	PFLFileSpecType		type,
	PGPUInt32			dataSize,
	PFLFileSpecRef *	outRef )
{
	PFLFileSpecRef	newFileRef	= NULL;
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateMemoryMgr( memoryMgr );
	
	/*
	 * XXX In the future if PFLFileSpecs contain pointers,
	 *     this will need to be changed.
	 */
	newFileRef = (PFLFileSpecRef) PGPNewData( memoryMgr,
		sizeof( *newFileRef ), kPGPMemoryMgrFlags_Clear );
	
	if ( IsntNull( newFileRef ) )
	{
		newFileRef->magic		= kPFLFileSpecMagic;
		newFileRef->type		= type;
		newFileRef->memoryMgr	= memoryMgr;
		newFileRef->dataSize	= dataSize;
		newFileRef->data		= NULL;
		newFileRef->vtbl		= pgpGetFileSpecVTBLForType( type );
		
		if ( dataSize != 0 )
		{
			PGPByte *	newData	= NULL;
			
			newData = (PGPByte *) PGPNewData( memoryMgr,
				dataSize, kPGPMemoryMgrFlags_Clear );
			if ( IsntNull( newData ) )
			{
				newFileRef->data	= newData;
			}
			else
			{
				PGPFreeData( newFileRef );
				newFileRef	= NULL;
				err	= kPGPError_OutOfMemory;
			}
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	*outRef	= newFileRef;
	
	return err;
}




	PGPError
PFLCopyFileSpec(
	PFLConstFileSpecRef	fileRef,
	PFLFileSpecRef *	outRef )
{
	PFLFileSpecRef	newFileRef	= NULL;
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PFLValidateFileSpec( fileRef );
	
	err	= pgpNewFileSpec( fileRef->memoryMgr, fileRef->type,
			fileRef->dataSize, &newFileRef );
	if ( IsntPGPError( err ) )
	{
		PGPByte *	dataPtr;
		
		/* copy entire struct, being careful to preserve embedded pointer */
		dataPtr				= PFLFileSpecData( newFileRef, PGPByte );
		*newFileRef			= *fileRef;
		newFileRef->data	= dataPtr;
		
		/* copy data itself */
		if ( IsntNull( dataPtr ) )
		{
			pgpCopyMemory( fileRef->data,
				newFileRef->data, fileRef->dataSize );
		}
	}
	
	*outRef	= newFileRef;
	
	pgpAssertErrWithPtr( err, *outRef );
	
	return err;
}


	PGPError
PFLFreeFileSpec( PFLFileSpecRef	fileRef)
{
	PFLValidateFileSpec( fileRef );
	
	if ( IsntNull( fileRef->data ) )
	{
		PGPFreeData( fileRef->data );
	}
	
	PGPFreeData( fileRef );
	
	return( kPGPError_NoErr );
}

/*____________________________________________________________________________
	Caller must free 'outName' with PGPFreeData
____________________________________________________________________________*/
	PGPError
PFLGetFileSpecName(
	PFLConstFileSpecRef	fileRef,
	char **				outName )
{
	char			tempName[ 256 ];
	PGPError		err	= kPGPError_NoErr;
	char *			allocName	= NULL;
	
	PGPValidatePtr( outName );
	*outName	= NULL;
	PFLValidateFileSpec( fileRef );
	
	err	= PFLGetFileSpecNameBuffer( fileRef, sizeof( tempName ), tempName );
	if ( IsntPGPError( err ) )
	{
		PGPUInt32		nameLength;
	
		nameLength	= strlen( tempName );
		
		allocName = (char *)PGPNewData( fileRef->memoryMgr,
			nameLength + 1, 0);
		if ( IsntNull( allocName ) )
		{
			strcpy( allocName, tempName );
		}
		else
		{
			err	= kPGPError_OutOfMemory;
		}
	}
	
	*outName	= allocName;
	return err;
}

	PGPError
PFLGetFileSpecNameBuffer(
	PFLConstFileSpecRef	fileRef,
	PGPSize				maxSize,
	char *				outName )
{
	char			tempName[ 256 ];
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( outName );
	PGPValidateParam( maxSize >= 1 );
	PFLValidateFileSpec( fileRef );
	
	err	= CallGetNameProc( fileRef, tempName );
	if ( IsntPGPError( err ) )
	{
		if ( strlen( tempName ) + 1 <= maxSize )
		{
			strcpy( outName, tempName );
		}
		else
		{
			err	= kPGPError_BufferTooSmall;
		}
	}
	
	return err;
}



	PGPError
PFLSetFileSpecName(
	PFLFileSpecRef		fileRef,
	char const *		newName)
{
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( newName );
	PFLValidateFileSpec( fileRef );
	
	err	= CallSetNameProc( fileRef, newName );
	
	return err;
}

	PGPError
PFLComposeFileSpec(
	PFLConstFileSpecRef	parentDir,
	char const *		fileName,
	PFLFileSpecRef *	outRef )
{
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( fileName );
	PGPValidatePtr( outRef );
	PFLValidateFileSpec( parentDir );

	*outRef = NULL;
	
	err = CallComposeProc( parentDir, fileName, outRef );
	
	return err;
}


/*____________________________________________________________________________
	Get the maximum length namre legal for this file spec.  The maximum
	length may depend not only on the OS, but also on the volume format.
____________________________________________________________________________*/
	PGPError
PFLGetMaxFileSpecNameLength(
	PFLConstFileSpecRef		fileRef,
	PGPSize *				maxLength)
{
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( maxLength );
	*maxLength	= 0;
	PFLValidateFileSpec( fileRef );
	
	err	= CallGetMaxNameLength( fileRef, maxLength );
	
	return err;
}


/*____________________________________________________________________________
	Extend the file name to include the extension, truncating the base name
	if necessary to meet the constraints of maximum name length
____________________________________________________________________________*/
	PGPError
PFLExtendFileSpecName(
	PFLFileSpecRef	fileRef,
	char const *	extension)
{
	PGPError		err	= kPGPError_NoErr;
	PGPSize			maxLength;
	
	PGPValidatePtr( extension );
	PFLValidateFileSpec( fileRef );
	
	err	= PFLGetMaxFileSpecNameLength( fileRef, &maxLength );
	if ( IsntPGPError( err ) )
	{
		PGPUInt32	extensionLength;
		
		extensionLength	= strlen( extension );
		if ( strlen( extension ) > maxLength )
		{
			err	= kPGPError_BadParams;
		}
		else
		{
			char	tempName[ 512 ];
			
			err	= PFLGetFileSpecNameBuffer( fileRef,
					sizeof( tempName ), tempName );
			if ( IsntPGPError( err ) )
			{
				PGPUInt32	nameLength;
				
				nameLength	= strlen( tempName );
				if ( nameLength + extensionLength > maxLength )
				{
					/* truncate base name */
					tempName[ maxLength - extensionLength ]	= '\0';
				}
				strcat( tempName, extension );
				err	= PFLSetFileSpecName( fileRef, tempName );
			}
		}
	}
	
	return err;
}

/*____________________________________________________________________________
	Get a unique file spec.  If the input spec does not exist, then it will
	be returned.
____________________________________________________________________________*/
	PGPError
PFLFileSpecGetUniqueSpec(
	PFLConstFileSpecRef	spec,
	PFLFileSpecRef *	uniqueOut )
{
	PGPError		err	= kPGPError_NoErr;
	PFLFileSpecRef	out	= NULL;
	PGPBoolean		exists;
	
	PGPValidatePtr( uniqueOut );
	*uniqueOut	= NULL;
	PFLValidateFileSpec( spec );
	
	/* important to copy it to preserve meta-data */
	err	= PFLCopyFileSpec( spec, &out );
	if ( IsntPGPError( err ) )
		err	= PFLFileSpecExists( out, &exists );
	
	if ( IsntPGPError( err ) && exists )
	{
		PGPUInt32	number = 1;
		char		origName[ 256 ];
		PGPSize		maxNameLength;
		
		err	= PFLGetMaxFileSpecNameLength( spec, &maxNameLength );
		if ( IsntPGPError( err ) )
		{
			err	= PFLGetFileSpecNameBuffer( spec,
				sizeof( origName ), origName );
		}
		
		if ( IsntPGPError( err ) ) do
		{
			char	tempName[ 256 ];
			PGPSize	maxSize;
			
			/* name can't exceed buffer size and must be <= platform limits */
			maxSize	= pgpMin( maxNameLength + 1, sizeof(tempName ) );
			
			err	= PGPNewNumberFileName( origName, "", number,
					maxSize, tempName );
			if ( IsPGPError( err ) )
				break;
			
			err	= PFLSetFileSpecName( out, tempName );
			if ( IsPGPError( err ) )
				break;
			
			err	= PFLFileSpecExists( out, &exists );
			if ( IsPGPError( err ) || ! exists )
				break;
			++number;
			
		} while ( TRUE );
	}
	
	if ( IsPGPError( err ) && IsntNull( out ) )
	{
		PFLFreeFileSpec( out );
		out	= NULL;
	}
	
	*uniqueOut	= out;
	return( err );
}


	PGPError
PFLExportFileSpec(
	PFLConstFileSpecRef		fileRef,
	PGPByte **				outData,
	PGPSize *				outDataSize )
{
	PGPError	err	= kPGPError_NoErr;
	PGPByte *	data		= NULL;
	PGPSize		dataSize	= 0;
	
	/* make sure we set these to known values first */
	if ( IsntNull( outData ) )
		*outData	= NULL;
	if ( IsntNull( outDataSize ) )
		*outDataSize	= 0;
		
	PGPValidatePtr( outData );
	PGPValidatePtr( outDataSize );
	PFLValidateFileSpec( fileRef );
	
	err	= CallExportProc( fileRef, &data, &dataSize );
	if ( IsntPGPError( err ) )
	{
		PGPMemoryMgrRef	memoryMgr	= fileRef->memoryMgr;
		void *			temp;
		
		/* prepend the object type byte to the data */
		dataSize	+= 1;
		temp	= data;
		err	= PGPReallocData( memoryMgr, &temp, dataSize, 0);
		data	= (PGPByte *)temp;
		if ( IsntPGPError( err ) )
		{
			/* shift data up and insert the type byte */
			pgpCopyMemory( data, data + 1, dataSize - 1 );
			data[ 0 ]	= (PGPByte)fileRef->type;
			pgpAssert( (PFLFileSpecType)data[ 0 ] == fileRef->type );
		}
		else
		{
			PGPFreeData( data );
			data		= NULL;
			dataSize	= 0;
		}
	}
	
	*outData		= data;
	*outDataSize	= dataSize;
	return err;
}


	PGPError
PFLImportFileSpec(
	PGPMemoryMgrRef		context,
	PGPByte const *		buffer,
	PGPSize				size,
	PFLFileSpecRef *	outRef )
{
	PGPError				err	= kPGPError_NoErr;
	PFLFileSpecRef			newRef	= NULL;
	PFLFileSpecType			type;
	PGPByte const *			typeData;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidatePtr( buffer );
	PGPValidateParam( PGPMemoryMgrIsValid( context ) );
	PGPValidateParam( size != 0);

	/* first byte is the type of file spec */
	type		= (PFLFileSpecType)buffer[ 0 ];
	typeData	= buffer + 1;
	PGPValidateParam(
		type == kPFLFileSpecFullPathType ||
		type == kPFLFileSpecMacType );
	
	err	= pgpNewFileSpec( context, type, 0, &newRef );
	if ( IsntPGPError( err ) )
	{
		err	= CallImportProc( newRef, typeData, size-1 );
		if ( IsPGPError( err ) )
		{
			PFLFreeFileSpec( newRef );
			newRef	= NULL;
		}
	}
	
	*outRef	= newRef;
	
	return err;
}



	PGPError
PFLSetFileSpecMetaInfo(
	PFLFileSpecRef		fileRef,
	const void *		info )
{
	PGPError		err	= kPGPError_NoErr;
	
	PFLValidateFileSpec( fileRef );
	
	err	= CallSetMetaInfoProc( fileRef, info);
	
	return( err );
}


	PGPError
PFLFileSpecExists(
	PFLFileSpecRef	fileRef,
	PGPBoolean *	exists )
{
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( exists );
	*exists	= FALSE;
	PFLValidateFileSpec( fileRef );
	
	err	= CallExistsProc( fileRef, exists );
	
	return( err );
}
					

	PGPError
PFLFileSpecCreate( PFLFileSpecRef	fileRef)
{
	PGPError		err	= kPGPError_NoErr;
	
	PFLValidateFileSpec( fileRef );
	
	err	= CallCreateProc( fileRef );
	
	return( err );
}


	PGPError
PFLFileSpecDelete( PFLConstFileSpecRef	fileRef)
{
	PGPError		err	= kPGPError_NoErr;
	
	PFLValidateFileSpec( fileRef );
	
	err	= CallDeleteProc( fileRef, info);
	
	return( err );
}


	PGPError
PFLFileSpecRename(
	PFLFileSpecRef	fileRef,
	const char *	newName )
{
	PGPError		err	= kPGPError_NoErr;
	
	PFLValidateFileSpec( fileRef );
	PGPValidatePtr( newName );
	
	err	= CallRenameProc( fileRef, newName );
	
	return( err );
}

	PGPError
PFLGetParentDirectory(
	PFLConstFileSpecRef		childSpec,
	PFLFileSpecRef *		outParent )
{
	PGPError	err	= kPGPError_NoErr;
	
	PFLValidateFileSpec( childSpec );
	PGPValidatePtr( outParent );
	*outParent	= NULL;

	err	= CallGetParentDirProc( childSpec, outParent );
	
	return( err );
}


	PGPError
PFLGetTempFileSpec(
	PGPMemoryMgrRef			context,
	PFLConstFileSpecRef		optionalRef,	/* may be NULL */
	PFLFileSpecRef *		outRef)
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateParam( IsNull( optionalRef ) ||
		pflFileSpecIsValid( optionalRef ) );
	
	err	= pgpPlatformGetTempFileSpec( context, optionalRef, outRef );
	
	return( err );
}

	PGPError
PFLGetFileInfo(
	PFLConstFileSpecRef	spec,
	PFLFileInfo *		outInfo )
{
	PGPValidatePtr( outInfo );
	PFLValidateFileSpec( spec );
	pgpClearMemory( outInfo, sizeof( *outInfo ) );

	return pgpPlatformGetFileInfo( spec, outInfo );
}

	PGPError
PFLNewDirectoryIter(
	PFLConstFileSpecRef		parentDir,
	PFLDirectoryIterRef *	outIter )
{
	PFLValidateFileSpec( parentDir );
	PGPValidatePtr( outIter );
	*outIter = NULL;

	return pgpPlatformNewDirectoryIter( parentDir, outIter );
}

	PGPError
PFLNextFileInDirectory(
	PFLDirectoryIterRef		iter,
	PFLFileSpecRef *		outRef )
{
	PGPValidatePtr( iter );
	PGPValidatePtr( outRef );
	*outRef = NULL;

	return pgpPlatformNextFileInDirectory( iter, outRef );
}

	PGPError
PFLFreeDirectoryIter(
	PFLDirectoryIterRef		iter )
{
	PGPValidatePtr( iter );

	return pgpPlatformFreeDirectoryIter( iter );
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
