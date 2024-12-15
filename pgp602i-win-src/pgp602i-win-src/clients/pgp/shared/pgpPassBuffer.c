/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc. and its affiliates.
	All rights reserved.

	$Id: pgpPassBuffer.c,v 1.1.8.1 1998/11/12 03:11:09 heller Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpMem.h"
#include "pgpOptionList.h"
#include "pgpPassBuffer.h"

#define	kPGPPassBufferMagic			0x50427566L		/* 'PBuf' */
#define	PassBufferIsValid(buffer)	IsntPGPError( VerifyPassBuffer( buffer ) )

typedef struct PGPPassBuffer
{
	PGPUInt32			magic;			/* Always kPGPPassBufferMagic */
	PGPMemoryMgrRef		memoryMgr;
	PGPSize				bufferSize;

	PGPPassBufferType	type;
	
	PGPByte				*data;
	PGPSize				dataSize;

} PGPPassBuffer;

	static PGPError
VerifyPassBuffer(const PGPPassBuffer *buffer)
{
	if( IsNull( buffer ) || buffer->magic != kPGPPassBufferMagic )
		return( kPGPError_BadParams );
		
	return( kPGPError_NoErr );
}

	static PGPError
SetData(
	PGPPassBuffer 		*buffer,
	PGPPassBufferType	dataType,
	const void			*data,
	PGPSize				dataSize)
{
	PGPError	err = kPGPError_NoErr;
	
	buffer->type = dataType;
	
	if( IsNull( buffer->data ) )
	{
		pgpAssert( buffer->bufferSize == 0 );
		pgpAssert( buffer->dataSize == 0 );
		
		buffer->data = (PGPByte *) PGPNewSecureData( buffer->memoryMgr,
									dataSize, 0 );
		if( IsntNull( buffer->data ) )
		{
			buffer->bufferSize = dataSize;
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}
	}
	else
	{
		if( buffer->bufferSize < dataSize )
		{
			err = PGPReallocData( buffer->memoryMgr, &buffer->data,
							dataSize, 0 );
			if( IsntPGPError( err ) )
			{
				buffer->bufferSize = dataSize;
			}
		}
		else
		{
			/* Clear old contents */
			pgpClearMemory( buffer->data, buffer->bufferSize );
		}
	}

	if( IsntPGPError( err ) )
	{
		buffer->dataSize = dataSize;
		pgpCopyMemory( data, buffer->data, dataSize );
	}

	pgpAssertNoErr( err );
	
	return( err );
}

	PGPError
PGPNewPassBuffer(
	PGPMemoryMgrRef		memoryMgr,
	PGPPassBufferRef 	*bufferPtr)
{
	PGPError		err = kPGPError_NoErr;
	PGPPassBuffer	*buffer;
	
	PGPValidatePtr( bufferPtr );
	*bufferPtr = kInvalidPGPPassBufferRef;
	PGPValidateParam( PGPMemoryMgrRefIsValid( memoryMgr ) );
	
	buffer = (PGPPassBuffer *) PGPNewData( memoryMgr, sizeof( **bufferPtr ),
											kPGPMemoryMgrFlags_Clear );
	if( IsntNull( buffer ) )
	{
		buffer->magic 		= kPGPPassBufferMagic;
		buffer->memoryMgr	= memoryMgr;
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}
	
	*bufferPtr = buffer;
	
	return( err );
}

	void
PGPFreePassBuffer(PGPPassBufferRef buffer)
{
	if( PassBufferIsValid( buffer ) )
	{
		if( IsntNull( buffer->data ) )
			PGPFreeData( buffer->data );
			
		buffer->magic = 0;
		
		PGPFreeData( buffer );
	}
}

	PGPError
PGPPassBufferSetPassphrase(
	PGPPassBufferRef 	buffer,
	const char			*passphrase)
{
	PGPValidateParam( PassBufferIsValid( buffer ) );
	PGPValidatePtr( passphrase );

	return( SetData( buffer, kPGPPassBufferType_Passphrase,
					(PGPByte *) passphrase, strlen( passphrase ) + 1 ) );

}

	PGPError
PGPPassBufferSetPassKey(
	PGPPassBufferRef 	buffer,
	const PGPByte		*passKey,
	PGPSize				passKeySize)
{
	PGPValidateParam( PassBufferIsValid( buffer ) );
	PGPValidatePtr( passKey );
	PGPValidateParam( passKeySize != 0 );

	return( SetData( buffer, kPGPPassBufferType_PassKey, passKey,
							passKeySize ) );
}

	PGPError
PGPClearPassBuffer(PGPPassBufferRef buffer)
{
	PGPValidateParam( PassBufferIsValid( buffer ) );

	if( IsntNull( buffer->data ) )
	{
		pgpClearMemory( buffer->data, buffer->dataSize );
		buffer->dataSize = 0;
	}
	
	buffer->type = kPGPPassBufferType_Invalid;
	 
	return( kPGPError_NoErr );
}

	PGPError
PGPCopyPassBuffer(
	PGPPassBufferRef 	buffer,
	PGPPassBufferRef	*bufferCopy)
{
	PGPError		err = kPGPError_NoErr;
	PGPPassBuffer	*newBuffer;
	
	PGPValidatePtr( bufferCopy );
	*bufferCopy = kInvalidPGPPassBufferRef;
	PGPValidateParam( PassBufferIsValid( buffer ) );
	
	err = PGPNewPassBuffer( buffer->memoryMgr, &newBuffer );
	if( IsntPGPError( err ) )
	{
		*newBuffer 		= *buffer;
		newBuffer->data = NULL;
		
		if( IsntNull( buffer->data ) )
		{
			newBuffer->data = (PGPByte *) PGPNewSecureData( buffer->memoryMgr,
												buffer->dataSize, 0 );
			if( IsntNull( newBuffer->data ) )
			{
				pgpCopyMemory( buffer->data, newBuffer->data,
							buffer->bufferSize );
			}
			else
			{
				PGPFreePassBuffer( newBuffer );
				newBuffer = kInvalidPGPPassBufferRef;
			}
		}
	}
	
	*bufferCopy = newBuffer;
	
	return( err );
}

	PGPOptionListRef
PGPOPassBuffer(
	PGPContextRef		context,
	PGPPassBufferRef	buffer)
{
	PGPOptionListRef	optionList = kInvalidPGPOptionListRef;
	
	if( PassBufferIsValid( buffer ) && IsntNull( buffer->data ) )
	{
		if( buffer->type == kPGPPassBufferType_Passphrase )
		{
			optionList = PGPOPassphrase( context, (char *) buffer->data );
		}
		else if( buffer->type == kPGPPassBufferType_PassKey )
		{
			optionList = PGPOPasskeyBuffer( context, buffer->data,
								buffer->dataSize );
		}
	}
	
	if( ! PGPOptionListRefIsValid( optionList ) )
	{
		/*
		** The semantics for PGPO calls is to never return NULL.
		** Since we do not have access to PGPO error generation functions,
		** pass back PGPONullOption() instead.
		*/
		
		pgpDebugMsg( "PGPOPassBuffer(): Failed to create option list" );
		
		optionList = PGPONullOption( context );
	}
	
	return( optionList );
}

	PGPPassBufferType
PGPGetPassBufferType(PGPPassBufferRef buffer)
{
	PGPPassBufferType	type;
	
	if( PassBufferIsValid( buffer ) )
	{
		type = buffer->type;
	}
	else
	{
		type = kPGPPassBufferType_Invalid;
	}
	
	return( type );
}

	PGPError
PGPPassBufferGetPassphrasePtr(
	PGPPassBufferRef 	buffer,
	char				**passphrasePtr)
{
	PGPValidatePtr( passphrasePtr );
	*passphrasePtr = NULL;
	PGPValidateParam( PassBufferIsValid( buffer ) );

	if( buffer->type == kPGPPassBufferType_Passphrase )
	{
		pgpAssert( IsntNull( buffer->data ) );
		
		*passphrasePtr = (char *) buffer->data;
	}
	
	return( kPGPError_NoErr );
}

	PGPError
PGPPassBufferGetPassKeyPtr(
	PGPPassBufferRef 	buffer,
	PGPByte				**passpKeyPtr,
	PGPSize				*passKeySize)
{
	if( IsntNull( passpKeyPtr ) )
		*passpKeyPtr = NULL;
	if( IsntNull( passKeySize ) )
		*passKeySize = 0;

	PGPValidateParam( PassBufferIsValid( buffer ) );
	PGPValidatePtr( passpKeyPtr );
	PGPValidatePtr( passKeySize );

	if( buffer->type == kPGPPassBufferType_PassKey )
	{
		pgpAssert( IsntNull( buffer->data ) );
		
		*passpKeyPtr	= buffer->data;
		*passKeySize	= buffer->dataSize;
	}
	
	return( kPGPError_NoErr );
}