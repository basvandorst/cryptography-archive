/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpMacBinary.c,v 1.20.10.1 1998/11/12 03:23:20 heller Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"
#include <string.h>

#include "pgpMem.h"
#include "pgpErrors.h"

#include "pgpIO.h"
#include "pgpLineEndIO.h"
#include "pgpIOUtilities.h"
#include "pgpFileUtilities.h"
#include "pgpMacBinary.h"
#include "pgpMacFileMapping.h"
#include "pgpStrings.h"

#include "pgpEndianConversion.h"



#define RawToUInt16( data )	PGPEndianToUInt16( kPGPBigEndian, data )
#define RawToUInt32( data )	PGPEndianToUInt32( kPGPBigEndian, data )

/*____________________________________________________________________________
	Convert the raw bytes of the header into an in-memory structure.  Fields
	in the structure use the approprite endian-ness.
	
	For example, the file type will be big-endian on a Mac and little-endian
	on a PC.
____________________________________________________________________________*/
	static void
sRawDataToHeader(
	PGPByte const *		rawData,
	MacBinaryHeader *	header
	)
{
	pgpClearMemory( header, sizeof( *header ) );
	
	header->oldVersion	= rawData[ kMacBinaryOffset_Version ];
	
	pgpCopyMemory( &rawData[ kMacBinaryOffset_NameLength ], header->name, 64 );
	
	header->info1.fdType	=
		RawToUInt32( &rawData[ kMacBinaryOffset_FileType ] );
		
	header->info1.fdCreator	=
		RawToUInt32( &rawData[ kMacBinaryOffset_FileCreator ] );
		
	header->info1.fdFlags	=
		RawToUInt16( &rawData[ kMacBinaryOffset_FinderFlags ] );
		
	header->info1.fdLocation.v	=
		RawToUInt16( &rawData[ kMacBinaryOffset_VPos ] );
		
	header->info1.fdLocation.h	=
		RawToUInt16( &rawData[ kMacBinaryOffset_HPos ] );
		
	header->info1.fdFldr	=
		RawToUInt16( &rawData[ kMacBinaryOffset_Folder ] );
	
	header->protectedBit	= rawData[ kMacBinaryOffset_ProtectedBit ];
	
	header->zero1		= rawData[ kMacBinaryOffset_Zero1 ];
	
	header->dLength		=
		RawToUInt32( &rawData[ kMacBinaryOffset_DataForkLength ] );
	
	header->rLength		=
		RawToUInt32( &rawData[ kMacBinaryOffset_ResForkLength ] );
	
	header->creationDate		=
		RawToUInt32( &rawData[ kMacBinaryOffset_CreationDate ] );
	
	header->modificationDate		=
		RawToUInt32( &rawData[ kMacBinaryOffset_ModificationDate ] );
	
	header->crc1		= rawData[ kMacBinaryOffset_crc1 ];
	header->crc2		= rawData[ kMacBinaryOffset_crc2 ];
}



	static PGPUInt16
sCalcCRC16Continue(
	PGPUInt16		crc,
	const void *	data,
	PGPInt32		len)
{
	const char *	dp	= (const char *)data;

	while(len--)
	{
		PGPInt16		i;
	
		crc ^= (PGPUInt16)(*dp++) << 8;
		for (i = 0; i < 8; ++i)
		{
			if (crc & 0x8000)
				crc = (crc << 1) ^ 0x1021;
			else
				crc <<= 1;
		}
	}
	return(crc);
}

	static PGPUInt16
sCalcCRC16(
	const void *data, PGPInt32 len)
{
	return sCalcCRC16Continue( 0, data, len);
}


/*____________________________________________________________________________
	Return TRUE if the raw header bytes are valid (CRC matches)
____________________________________________________________________________*/
	static PGPBoolean
sHeaderIsValid(
	const PGPByte *rawBytes )
{
	PGPUInt16	crc;
	PGPBoolean	isValid	= FALSE;
	
	/* CRCs don't include themselves */
	#define kLengthOfDataToCRC		( kPGPMacBinaryHeaderSize - (2+2) )
	crc	= sCalcCRC16( rawBytes, kLengthOfDataToCRC );
	
	isValid	= ( ( crc >> 8) == rawBytes[ kMacBinaryOffset_crc1 ] &&
		(crc & 0xFF) == rawBytes[ kMacBinaryOffset_crc2 ] );
	if ( ! isValid )
	{
		/* we can be more liberal, but our code always sets the CRC */
		/* so we may as well require that the CRC be valid */
	}
	
	return( isValid );
}


/*____________________________________________________________________________
	Read the header into a structure.  The struct
	produced is an in-memory representation; not necessarily the exact
	byte order on disk (primarily for big/little endian reasons, but also
	because different compilers on different platforms may pad structures
	differently).
____________________________________________________________________________*/
	static PGPError
sReadMacBinaryHeader(
	PFLFileSpecRef		spec,
	MacBinaryHeader *	header,
	PGPBoolean *		isValid
	)
{
	PGPError	err	= kPGPError_NoErr;
	PGPIORef	io;
	
	*isValid	= FALSE;
	
	err	= PGPOpenFileSpec( spec,
			kPFLFileOpenFlags_ReadOnly, (PGPFileIORef *)&io );
	if ( IsntPGPError( err ) )
	{
		PGPByte	buffer[ kPGPMacBinaryHeaderSize ];
		
		err	= PGPIORead( io, sizeof( buffer ), buffer, NULL );
		if ( IsntPGPError( err ) )
		{
			*isValid	= sHeaderIsValid( buffer );
			/* convert, even if not valid */
			sRawDataToHeader( buffer, header );
		}
		
		PGPFreeIO( io );
		io	= NULL;
	}
	
	return( err );
}



#if PGP_MACINTOSH	/* PGP_MACINTOSH [ */

#include <Script.h>

#include "pgpFileSpecMac.h"
#include "MacFiles.h"
#include "MacErrors.h"
#include "MacStrings.h"

/*____________________________________________________________________________
	Mac Version
	
	Create a file from the MacBinary original
____________________________________________________________________________*/
	static OSStatus
sCreateFromMacBinary(
	PGPMemoryMgrRef			memoryMgr,
	short					fileRef,
	MacBinaryHeader	const *	header,
	const FSSpec *			tempSpec )
{
	OSStatus	err;
	CInfoPBRec	cpb;
	
	err	= FSpCreate( tempSpec,
					header->info1.fdCreator, header->info1.fdType,
					smSystemScript );
	
	if ( IsntErr( err ) && header->dLength != 0 )
	{
		short	outRef;
		
		err	= FSpOpenDF( tempSpec, fsRdWrPerm, &outRef );
		if ( IsntErr( err ) )
		{
			err	= FSCopyBytes( memoryMgr, fileRef, kPGPMacBinaryHeaderSize,
					outRef, 0, header->dLength );
			
			FSClose( outRef );
		}
	}
	
	if ( IsntErr( err ) && header->rLength != 0 )
	{
		short	outRef;
		
		err	= FSpOpenRF( tempSpec, fsRdWrPerm, &outRef );
		if ( IsntErr( err ) )
		{
			err	= FSCopyBytes( memoryMgr, fileRef,
					kPGPMacBinaryHeaderSize + header->dLength,
					outRef, 0, header->rLength );
			
			FSClose( outRef );
		}
	}
	
	/* set file meta information */
	if ( IsntErr( err ) )
	{
		err	= FSpGetCatInfo( tempSpec, &cpb );
		if ( IsntErr( err ))
		{
			#define kFinderFlagsIgnoreMask	 \
				( kHasBeenInited | kNameLocked | kIsInvisible | kIsOnDesk )
			cpbFInfo( &cpb ).fdFlags	=
				header->info1.fdFlags & ~kFinderFlagsIgnoreMask;
			cpbFInfo( &cpb ).fdLocation	= header->info1.fdLocation;
				
			cpbModificationDate( &cpb )	= header->modificationDate;
			cpbCreationDate( &cpb )		= header->creationDate;
				
			err	= FSpSetCatInfo( tempSpec, &cpb );
		}
	}
	
	/* cleanup if error */
	if ( IsErr( err ) )
	{
		(void)FSpDelete( tempSpec );
	}
	
	return( err );
}

			
/*____________________________________________________________________________
	Mac Version
____________________________________________________________________________*/
	static PGPError
sDeMacBinarize(
	PFLFileSpecRef				inSpec,
	MacBinaryHeader const *		header,
	PFLFileSpecRef *			outSpec )
{
	PGPError	err	= kPGPError_NoErr;
	FSSpec		fsSpec;
	
	*outSpec	= kInvalidPFLFileSpecRef;
	
	err	= PFLGetFSSpecFromFileSpec( inSpec, &fsSpec );
	if ( IsntPGPError( err ) )
	{
		short		fileRef;
		OSStatus	macErr;
		
		macErr	= FSpOpenDF( &fsSpec, fsRdPerm, &fileRef );
		if ( IsntErr( macErr ) )
		{
			FSSpec	tempSpec;
			
			macErr	= FSpGetUniqueSpec( &fsSpec, &tempSpec );
			if ( IsntErr( macErr ) )
			{
				macErr = sCreateFromMacBinary(
								PFLGetFileSpecMemoryMgr( inSpec ), fileRef,
								header, &tempSpec );
			}
			
			FSClose( fileRef );
			fileRef	= 0;
			
			if ( IsntErr( macErr ) )
			{
				const unsigned char	kBinExtension[]	= "\p.bin";
				
				/* delete original and move output into its place */
				(void)FSpDelete( &fsSpec );
				
				if ( PStringHasSuffix( fsSpec.name, kBinExtension, FALSE ) )
				{
					fsSpec.name[ 0 ]	-= StrLength( kBinExtension );
					
					macErr	= FSpGetUniqueSpec( &fsSpec, &fsSpec );
				}
				
				if ( IsntErr( macErr ) )
					macErr	= FSpRename( &tempSpec, fsSpec.name );
			}
		}
		
		err	= MacErrorToPGPError( macErr );
	}
	
	if ( IsntPGPError( err ) )
	{
		err	= PFLNewFileSpecFromFSSpec(
			PFLGetFileSpecMemoryMgr( inSpec ), &fsSpec, outSpec );
	}
	
	return( err );
}


#else	/* ] PGP_MACINTOSH [ */


/*____________________________________________________________________________
	Non-Mac Version
	
	Copy from input to output, converting mac line endings to PC line endings.
____________________________________________________________________________*/
	static PGPError
sCopyAndLineEndConvert(
	PGPIORef				inRef,
	PGPSize					requestCount,
	PGPIORef				outRef)
{
	PGPLineEndIORef		convertIO	= NULL;
	PGPError			err	= kPGPError_NoErr;
	ConvertLineEndType	toType;
	
	/* wrap a PGPLineEndIO around the given IO */
#if PGP_UNIX
	toType	= kLFLineEndType;
#elif PGP_WIN32
	toType	= kCRLFLineEndType;
#else
	#error unknown platform
#endif
	err	= PGPNewLineEndIO( outRef, FALSE, toType, &convertIO );
	
	if ( IsntPGPError( err ) )
	{
		err	= PGPCopyIO( inRef, requestCount, (PGPIORef)convertIO );
		PGPFreeIO( (PGPIORef)convertIO );
		convertIO	= NULL;
	}
	
	
	return( err );
}


/*____________________________________________________________________________
	Non-Mac Version
____________________________________________________________________________*/
	static PGPError
sCreateFromMacBinary(
	PGPIORef				inRef,
	MacBinaryHeader	const *	header,
	PFLFileSpecRef			outSpec )
{
	PGPError	err;
	
	err	= PFLFileSpecCreate( outSpec );
	if ( IsntPGPError( err ) && header->dLength != 0 )
	{
		PGPIORef	outRef;
		
		err	= PGPOpenFileSpec( outSpec, kPFLFileOpenFlags_ReadWrite,
				(PGPFileIORef *)&outRef );
		if ( IsntPGPError( err ) )
		{
			err	= PGPIOSetPos( inRef, kPGPMacBinaryHeaderSize );
			if ( IsntPGPError( err ) )
			{
				#define kFileTypeText	((OSType)0x54455854)	/* 'TEXT' */
				if ( header->info1.fdType != kFileTypeText )
				{
					err	= PGPCopyIO( inRef, header->dLength, outRef );
				}
				else
				{
					err	= sCopyAndLineEndConvert( inRef,
						header->dLength, outRef );
				}
			}
			
			PGPFreeIO( outRef );
		}
	}
	
	return( err );
}


/*____________________________________________________________________________
	Non-Mac Version
	
	Get a unique filename using the baseSpec as a directory specifier and
	first choice for file name.
	
	If baseSpec does not exist, then an equivalent spec will be created with
	that name.
____________________________________________________________________________*/
	static PGPError
sGetUniqueTempSpec(
	PFLFileSpecRef		baseSpec,
	PFLFileSpecRef *	outSpec)
{
	PGPError		err;
	PFLFileSpecRef	tempSpec	= NULL;
	
	err	= PFLCopyFileSpec( baseSpec, &tempSpec );
	if ( IsntPGPError( err ) )
	{
		static const char 	kTempFileBase[]			= "temp";
		static const char 	kTempFileExtension[]	= "tmp";
		char		tempName[ 64 ];
		PGPUInt32	number	= 0;
		
		/* loop until name is unique */
		do
		{
			PGPBoolean	exists;
			
			err	= PFLFileSpecExists( tempSpec, &exists );
			if ( IsPGPError( err ) )
				break;
			if ( ! exists )
				break; /* good, done */
			++number;
			
			/* generate "temp 1.tmp", "temp 2.tmp", etc */
			sprintf( tempName, "%s%d.%s",
				kTempFileBase, (int)number, kTempFileExtension );
			err	= PFLSetFileSpecName( tempSpec, tempName );
			if ( IsPGPError( err ) )
				break;
				
		} while ( TRUE );
		
		
		if ( IsPGPError( err ) )
		{
			PFLFreeFileSpec( tempSpec );
			tempSpec	= kInvalidPFLFileSpecRef;
		}
	}
	
	*outSpec	= tempSpec;
	return( err );
}




/*____________________________________________________________________________
	Non-Mac Version
	
	Remove the existing file name extension (if any) and add the new one.
____________________________________________________________________________*/
	static PGPError
sCreateNewName(
	PFLFileSpecRef	inSpec,
	char const *	extension,
	PGPSize			maxSize,
	char *			newName )
{
	PGPError		err	= kPGPError_NoErr;
	PGPSize			newLength;

	newName[ 0 ]	= '\0';
	
	err	= PFLGetFileSpecNameBuffer( inSpec, maxSize, newName );
	if ( IsntPGPError( err ) )
	{
		char const *	existingExtension;
	
		existingExtension	= PGPGetFileNameExtension( newName );
		if ( IsntNull( existingExtension ) )
		{
			newLength	= strlen( newName ) -
							strlen( existingExtension );
			newLength	-= 1;	/* account for '.' */
			newName[ newLength ]	= '\0';
			existingExtension	= NULL;
		}
	
		/* add the appropriate extension */
		strcat( newName, "." );	
		strcat( newName, extension );
	}
	
	return( err );
}




/*____________________________________________________________________________
	Determine if the existing file name extension is already suitable for
	this type of file.
____________________________________________________________________________*/
	static PGPBoolean
sHasSuitableExtensionForType(
	OSType			creator,
	OSType			type,
	PFLFileSpecRef	spec )
{
	PGPError		err	= kPGPError_NoErr;
	PGPBoolean		isSuitable	= FALSE;
	char 			name[ 512 ];
	
	err	= PFLGetFileSpecNameBuffer( spec, sizeof( name ), name );
	if ( IsntPGPError( err ) )
	{
		char const *extension;
		
		extension	= PGPGetFileNameExtension( name );
		if ( IsntNull( extension ) )
		{
			isSuitable	= pgpIsValidExtensionForMacType( creator, type,
							extension );
		}
	}
	
	return( isSuitable );
}


/*____________________________________________________________________________
	Non-Mac version.
	
	De-MacBinarize the file giving the new file a suitable file name extension
____________________________________________________________________________*/
	static PGPError
sDeMacBinarize(
	PFLFileSpecRef				inSpec,
	MacBinaryHeader const *		header,
	PFLFileSpecRef *			outSpec )
{
	PGPError		err	= kPGPError_NoErr;
	PGPIORef		inRef		= kInvalidPGPIORef;
	PFLFileSpecRef	tempSpec	= kInvalidPFLFileSpecRef;
	
	*outSpec	= kInvalidPFLFileSpecRef;
	
	/* open the MacBinary file */
	err	= PGPOpenFileSpec( inSpec, kPFLFileOpenFlags_ReadOnly,
			(PGPFileIORef *)&inRef );
	if ( IsntPGPError( err ) )
	{
		/* we'll put the output into a temp file */
		err	= sGetUniqueTempSpec( inSpec, &tempSpec );
		if ( IsntPGPError( err ) )
		{
			err	= sCreateFromMacBinary( inRef, header, tempSpec );
		}
		
		/* all done with input */
		PGPFreeIO( inRef );
		inRef	= NULL;
		
		if ( IsntPGPError( err ) )
		{
			char			extension[ 3 + 1 ];
			PGPBoolean		haveExtension;
			PGPMemoryMgrRef	memoryMgr;
			char			newName[ 512 ];
			
			memoryMgr	= PFLGetFileSpecMemoryMgr( inSpec );
			
			/* delete original and move output into its place */
			(void)PFLFileSpecDelete( inSpec );
			
			if ( sHasSuitableExtensionForType( header->info1.fdCreator,
					header->info1.fdType, inSpec ) )
			{
				haveExtension	= FALSE;
			}
			else
			{
				err	= pgpMapMacCreatorTypeToExtension(
						header->info1.fdCreator, 
						header->info1.fdType, extension );
				/* ok if mapping failed */
				haveExtension	= IsntPGPError( err );
				err	= kPGPError_NoErr;
			}
			
			if ( haveExtension )
			{
				err	= sCreateNewName( inSpec,
					extension, sizeof( newName ), newName );
			}
			else
			{
				/* just give it the same name as the original */
				err	= PFLGetFileSpecNameBuffer( inSpec,
					sizeof( newName ), newName );
			}
			
			if ( IsntPGPError( err ) )
			{
				/* this changes the name and renames the file */
				err	= PFLFileSpecRename( tempSpec, newName );
			}
			pgpAssertNoErr( err );
		}
	}
	
	*outSpec	= tempSpec;
	
	return( err );
}


#endif	/* ] PGP_MACINTOSH */


/*____________________________________________________________________________
	Internal routine to convert a MacBinary format file to something most
	appropriate for the local machine.
____________________________________________________________________________*/
	PGPError
pgpMacBinaryToLocal(
	PFLFileSpecRef		inSpec,
	PFLFileSpecRef *	outSpec,
	PGPUInt32 *			macCreator,
	PGPUInt32 *			macType )
{
	PGPError		err	= kPGPError_NoErr;
	PGPBoolean			isValid;
	MacBinaryHeader	header;
	
	(void)inSpec;
	(void)outSpec;
	if ( IsntNull( macCreator ) )
		*macCreator		= 0;
	if ( IsntNull( macType ) )
		*macType		= 0;
	
	err	= sReadMacBinaryHeader( inSpec, &header, &isValid );
	if ( IsntPGPError( err ) )
	{
		if ( isValid )
		{
			/* if there's no data fork, don't de-macbinarize it */
			if ( header.dLength == 0 )
				err	= kPGPError_NoMacBinaryTranslationAvailable;
		}
		else
		{
			err	= kPGPError_NotMacBinary;
		}
	}
		
	if ( IsntPGPError( err ) )
	{
		if ( IsntNull( macCreator ) )
			*macCreator	= header.info1.fdCreator;
		if ( IsntNull( macType ) )
			*macType	= header.info1.fdType;
		
		err	= sDeMacBinarize( inSpec, &header, outSpec );
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
