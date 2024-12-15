/*____________________________________________________________________________
	TranslatorUtils.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: TranslatorUtils.cp,v 1.14 1997/09/18 23:30:26 lloyd Exp $
____________________________________________________________________________*/
#include <Timer.h>
#include <stdio.h>
#include <string.h>
#include <FSp_fopen.h>

#include "emsapi-mac.h"
#include "pgpMem.h"
#include "MacStrings.h"
#include "MacMemory.h"
#include "MacFiles.h"
#include "MyMIMEUtils.h"
#include "PluginLibUtils.h"
#include "pgpKeys.h"
#include "TranslatorStrings.h"
#include "PGPSharedEncryptDecrypt.h"

#include "PGPSharedParse.h"
#include "TranslatorUtils.h"


#define MIN(a, b )		( (a) <= (b) ? (a) : (b) )


	long
SignatureDataToEudoraResult( PGPEventSignatureData const *sigData )
{
	long	sigResult	= EMSC_SIGUNKNOWN;
	
	if ( sigData->checked )
	{
		if ( sigData->verified )
			sigResult	= EMSC_SIGOK;
		else
			sigResult	= EMSC_SIGBAD;
	}
	else
	{
		sigResult	= EMSC_SIGBAD;
	}
	
	return( sigResult );
}





	static OSErr
ReadDataForParsing(
	const FSSpec *	spec,
	Handle *		dataHandlePtr )
{
	OSErr			err	= noErr;
	Handle			dataHandle	= nil;
	const ulong		kMaxData	= 1024;

	dataHandle	= pgpNewHandle( kMaxData,
					kMacMemory_PreferTempMem | kMacMemory_UseApplicationHeap);
	if ( IsntNull( dataHandle ) )
	{
		ulong	actualSize;
		
		HLock( dataHandle );
		err	= FSpReadBytes( spec, *dataHandle,
				GetHandleSize( dataHandle ), &actualSize );
		HUnlock( dataHandle );
		
		AssertNoErr( err, "ReadDataForParsing" );
		if ( IsntErr( err ) )
		{
			SetHandleSize( dataHandle, actualSize );
		}
		else
		{
			DisposeHandle( dataHandle );
			dataHandle	= nil;
		}
	}
	else
	{
		err	= memFullErr;
	}
	
	*dataHandlePtr	= dataHandle;
	
	return( err );
}




	OSErr
ParseFileForMIMEType(
	const FSSpec *		spec,
	emsMIMETypeHandle *	typeHandlePtr )
{
	Handle	text	= nil;
	OSErr	err		= noErr;

	err	= ReadDataForParsing( spec, &text );
	
	if ( IsntErr( err ) )
	{
		ParseMIMEType( text, typeHandlePtr);
		DisposeHandle( text );
		
		if ( IsNull( *typeHandlePtr ) )
			err	= paramErr;
	}
	
	return( err );
}


	OSErr
CopyFileToFileRef(
	const FSSpec *	fileToCopy,
	short			destFileRef)
	{
	OSErr	err	= noErr;
	short	srcFileRef;
	
	err	= FSpOpenDF( fileToCopy, fsRdPerm, &srcFileRef );
	if ( IsntErr( err ) )
	{
		long	eof;
		long	curPos;
			
		err	= GetEOF( srcFileRef, &eof );
		if ( IsntErr( err ) )
		{
			err	= GetFPos( destFileRef, &curPos );
		}
		
		if ( IsntErr( err ) )
		{
			err	= FSCopyBytes( srcFileRef, 0, destFileRef, curPos, eof );
		}
	
		FSClose( srcFileRef );
	}
	
	return( err );
	}






#if PGP_DEBUG
	void
DebugCopyToRAMDisk(
	const FSSpec *		inSpec,
	ConstStr255Param	name )
{
	FSSpec	copySpec;
	OSErr	err;
	Str255	tempString;
	
	CopyPString( "\pRAM Disk:", tempString );
	AppendPString( name, tempString );
	
	err	= FSMakeFSSpec( 0, 0, tempString, &copySpec );
	if ( IsntErr( err ) || err == fnfErr )
	{
		(void)FSpDelete( &copySpec );
		err	= FSpCopyFiles( inSpec, &copySpec );
		FSpChangeFileType( &copySpec, 'TEXT' );
	}
}
#endif






/*____________________________________________________________________________
	'addresses' as supplied by Eudora consists of an array of StringHandles
	that come in groups of three.  Each group consists of
		{ "user@host.domain", "user", "full user name" }
		
	The last two entries may be null and the list terminates when a
	first item is null.
____________________________________________________________________________*/
	static OSStatus
CreateRecipientList(
	const StringHandle **	addresses,
	PGPKeySpec **			recipientsOut,
	PGPUInt32 *				numFound )
{
	PGPKeySpec *	recipients		= nil;
	PGPError		err	= noErr;
	const ushort	kClumpSize = 3;
	ulong			curGroupIndex	= kClumpSize;	// skip from:
	
	*recipientsOut	= nil;
	*numFound		= 0;
	
	while ( true )
	{
		StringHandle	userNameHandle;
		char			userName[ sizeof( Str255 ) ];
		char			userNameRevised[ 256 ];
		
		userNameHandle	= (*addresses)[ curGroupIndex ];
		if ( IsNull( userNameHandle ) )
			break;	// all done
		
		PToCString( *userNameHandle, userName );
		if ( userName[ 0 ] == '\0' )
			break;	// all done
		
		// add <> around the user name
		pgpFormatStr( userNameRevised, sizeof( userNameRevised ),
			false, "<%s>", userName );
			
		*numFound		+= 1;
		
		if ( IsNull( recipients ) )
		{
			recipients	= (PGPKeySpec *)pgpAlloc( sizeof( recipients[ 0 ] ) );
			if ( IsNull( recipients ) )
				err	= memFullErr;
		}
		else
		{
			err	= pgpRealloc( &recipients,
					*numFound * sizeof( recipients[ 0 ] ) );
		}
		if ( IsErr( err ) )
			break;
		
		CopyCString( userNameRevised, recipients[ *numFound - 1 ].userIDStr );
		
		curGroupIndex	+= kClumpSize;
	}
	
	if ( IsErr( err ) && IsntNull( recipients ) )
	{
		pgpFree( recipients );
		recipients	= nil;
	}
	
	
	*recipientsOut	= recipients;
	
	return( err );
}


/*____________________________________________________________________________
	Get a list of recipients, interacting with the user if necessary.
	
	'addresses' is as passed from Eudora.  
	Don't prompt the user if we can find all the keys for the addresses.
____________________________________________________________________________*/

	CComboError
GetRecipientList(
	PGPContextRef			context,
	const StringHandle **	addresses,
	PGPKeySetRef			allKeys,
	PGPKeySetRef *			recipientsOut,
	PGPRecipientSettings *	settingsPtr,	// may be pre-set to something)
	PGPKeySetRef *			keysToAdd
	)	
{
	CComboError		err;
	PGPKeySpec *	recipientSpecs	= nil;
	PGPKeySetRef	keysOut	= kPGPInvalidRef;
	Boolean			haveRecipients	= false;
	PGPUInt32		numFound = 0;
	
	pgpAssert( IsntNull( recipientsOut ) );
	pgpAssert( IsntNull( keysToAdd ) );
	*recipientsOut	= kPGPInvalidRef;
	*keysToAdd	= kPGPInvalidRef;
	// signing flag may be set already
	
	if ( IsntNull( addresses ) )
	{
		err.err	= CreateRecipientList( addresses, &recipientSpecs, &numFound );
	}
	
	if ( err.IsntError() )
	{
		PGPBoolean		didShowDialog;
		PGPKeySetRef	newKeys;
		
		const PGPRecipientOptions	kDialogOptions	=
			kPGPRecipientOptionsHideFileOptions;
				
		err.pgpErr	= PGPRecipientDialog( context, allKeys,
					numFound, recipientSpecs,
					kDialogOptions, *settingsPtr, settingsPtr,
					&keysOut, &didShowDialog, &newKeys );
		
		if ( IsntNull( recipientSpecs ) )
		{
			pgpFree( recipientSpecs );
			recipientSpecs	= NULL;
		}
		
		if ( err.IsntError() )
		{
			if( PGPRefIsValid( newKeys ) )
			{
				if( didShowDialog )
				{
					*keysToAdd	= newKeys;
				}
			}
			
			*recipientsOut	= keysOut;
		}
	}
	
	return( err );
}


	long
CComboErrorToEudoraError( CComboError & err )
{
	long	emsrErr	= EMSR_OK;
	
	if ( err.IsntError() )
		return( EMSR_OK );
		
	if ( err.err != noErr )
	{
		switch( err.err )
		{
			default:				emsrErr	= EMSR_UNKNOWN_FAIL;	break;
			case userCanceledErr:	emsrErr	= EMSR_ABORTED;			break;
		}
	}
	else if ( err.pgpErr != noErr )
	{
		switch( err.pgpErr )
		{
			default:					emsrErr	= EMSR_UNKNOWN_FAIL;	break;
			case kPGPError_UserAbort:	emsrErr	= EMSR_ABORTED;			break;
		}
	}
	
	return( emsrErr );
}







	OSErr
FSpReadFileIntoBuffer(
	const FSSpec *		spec,
	void **				bufferOut,
	ulong *				bufferSizeOut )
{
	OSStatus	err	= noErr;
	void *		buffer	= nil;
	ulong		bufferSize = 0;
	
	err	= FSpGetForkSizes( spec, &bufferSize, nil );
	if ( IsntErr( err ) )
	{
		buffer	= pgpAllocMac( bufferSize,
					kMacMemory_PreferTempMem |
					kMacMemory_UseCurrentHeap);
		if ( IsntNull( buffer ) )
		{
			err	= FSpReadBytes( spec, buffer, bufferSize, &bufferSize );
			if ( IsErr( err ) )
			{
				pgpFreeMac( buffer );
				buffer	= nil;
			}
		}
		else
		{
			err	= memFullErr;
		}
	}
	
	*bufferOut		= buffer;
	*bufferSizeOut	= bufferSize;
	AssertNoErr( err, "ParsePGPMimeSignedMessage" );
	
	return( err );
}




	void
MyGetIndCString(
	ushort	whichString,
	char *	theString )
{
	GetIndCString( theString, kTranslatorStringsRID, whichString );
}


	static void
GetIndPString(
	ushort		whichString,
	StringPtr	theString )
{
	GetIndString( theString, kTranslatorStringsRID, whichString );
}


	StringHandle
GetIndStringHandle(
	ushort		whichString)
{
	Str255	theString;
	
	GetIndPString( whichString, theString );
	
	return( NewString( theString ) );
}




/*____________________________________________________________________________
	Eliminate the stuff we dont want from the file. Typically:
		MIME Version: 1.0
		Content-Type: "text/plain"
  ____________________________________________________________________________*/
	OSErr
EliminateMIMEHeader( const FSSpec *spec )
{
	OSErr	err	= noErr;
	void *	buffer;
	ulong	bufferSize;
	
	err	= FSpReadFileIntoBuffer( spec, &buffer, &bufferSize );
	if ( IsntErr( err ) )
	{
		const uchar		kContentType[]	= "\pContent-Type:";
		const char *	start;
		
		start	= FindMatchingBytes( (const char *)buffer, bufferSize,
					(const char *)&kContentType[ 1 ],
					StrLength( kContentType ) );
		if ( IsntNull( start ) )
		{
			const char *	last	=
								((const char *)buffer) + bufferSize - 1;
			const char *	cur;
			short			fileRef;
			ulong			skipBytes;
			
			// skip until the next line
			cur	= start;
			while ( cur <= last )
			{
				if ( *cur++ == '\r' )
				{
					// skip all intervening line ends
					while ( cur <= last && ( *cur == '\r' || *cur == '\n') )
					{
						++cur;
					}
					break;
				}
			}
			
			// figure out how many bytes are left...
			skipBytes	= cur - (const char *)buffer;
			pgpAssert( skipBytes <= bufferSize );
			
			// and overwrite the file with only the desired bytes
			err	= FSpOpenDF( spec, fsRdWrPerm, &fileRef );
			if ( IsntErr( err ) )
			{
				long	count;
				
				SetEOF( fileRef, 0 );
				
				count	= bufferSize - skipBytes;
				err		= FSWrite( fileRef, &count,
							((const char *)buffer) + skipBytes );
				FSClose( fileRef );
			}
		}

		pgpFreeMac( buffer );
	}
	
	return( err );
}




