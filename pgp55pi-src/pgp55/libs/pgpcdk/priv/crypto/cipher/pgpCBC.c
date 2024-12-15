/*____________________________________________________________________________
	pgpCBC.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpCBC.c,v 1.4 1997/10/14 01:48:15 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"
#include <string.h>

#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpCBCPriv.h"
#include "pgpUtilitiesPriv.h"



#define PGPValidateCBC( CBC )	\
	PGPValidateParam( pgpCBCIsValid( CBC ) );
	
/*____________________________________________________________________________
	In CBC, iv[] is the previous block's ciphertext, or the Initial
	Vector before the first block is processed.  As we read plaintext
	bytes during encryption, they are xored into iv[] until it is
	full, then the symmetric cipher is run.  For decryption, iv[] is
	loaded with the previous ciphertext after we finish decrypting the
	current block.
____________________________________________________________________________*/
	
	

struct PGPCBCContext
{
#define kCBCMagic		0xBAAB0957
	PGPUInt32						magic;
	PGPContextRef					context;
	PGPBoolean						CBCInited;
	PGPSymmetricCipherContextRef	symmetricRef;
	PGPByte							iv[ PGP_CBC_MAXBLOCKSIZE ];
};

	static PGPBoolean
pgpCBCIsValid( const PGPCBCContext * ref)
{
	PGPBoolean	valid	= FALSE;
	
	valid	= IsntNull( ref ) && ref->magic	 == kCBCMagic;
	
	return( valid );
}



/*____________________________________________________________________________
	Internal forward references
____________________________________________________________________________*/

static void		pgpCBCInit( PGPCBCContext *	ref,
					void const * key, void const * iv);
					
static PGPError	pgpCBCEncrypt( PGPCBCContext *ref,
					void const * src, PGPSize len, void * dest );
					
static PGPError	pgpCBCDecrypt( PGPCBCContext *ref,
					void const * src, PGPSize len, void * dest );



/*____________________________________________________________________________
	Exported routines
____________________________________________________________________________*/

	PGPError 
PGPNewCBCContext(
	PGPSymmetricCipherContextRef	symmetricRef,
	PGPCBCContextRef *				outRef )
{
	PGPCBCContextRef				newRef	= NULL;
	PGPError						err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidatePtr( symmetricRef );
	
	newRef	= (PGPCBCContextRef)
			pgpContextMemAlloc( pgpGetSymmetricCipherContext( symmetricRef ),
				sizeof( *newRef ), 0 | kPGPMemoryFlags_Clear);
			
	if ( IsntNull( newRef ) )
	{
#if PGP_DEBUG
		/* make original invalid to enforce semantics */
		PGPSymmetricCipherContextRef	tempRef;
		err	= PGPCopySymmetricCipherContext( symmetricRef, &tempRef );
		if ( IsntPGPError( err ) )
		{
			PGPFreeSymmetricCipherContext( symmetricRef );
			symmetricRef	= tempRef;
		}
		err	= kPGPError_NoErr;
#endif

		newRef->magic						= kCBCMagic;
		newRef->CBCInited					= FALSE;
		newRef->symmetricRef				= symmetricRef;
		newRef->context		= pgpGetSymmetricCipherContext( symmetricRef );
		
		/* make sure we clean up */
		if ( IsPGPError( err ) )
		{
			PGPFreeCBCContext( newRef );
			newRef	= NULL;
		}
	}
	else
	{
		/* we own it, so dispose it */
		PGPFreeSymmetricCipherContext( symmetricRef );
		err	= kPGPError_OutOfMemory;
	}
	
	*outRef	= newRef;
	return( err );
}



/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError 
PGPFreeCBCContext( PGPCBCContextRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	PGPContextRef	context	= NULL;
	
	PGPValidateCBC( ref );

	context	= ref->context;
	
	PGPFreeSymmetricCipherContext( ref->symmetricRef );
	
	pgpClearMemory( ref, sizeof( *ref ) );
	pgpContextMemFree( context, ref );
	
	return( err );
}



/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError 
PGPCopyCBCContext(
	PGPCBCContextRef	inRef,
	PGPCBCContextRef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	PGPCBCContextRef	newRef	= NULL;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateCBC( inRef );
	
	newRef	= (PGPCBCContextRef)
		pgpContextMemAlloc( inRef->context,
		sizeof( *newRef ), 0);

	if ( IsntNull( newRef ) )
	{
		*newRef		= *inRef;
		
		/* clear symmetric cipher in case later allocation fails */
		newRef->symmetricRef = NULL;
		
		/* copy symmetric cipher */
		err	= PGPCopySymmetricCipherContext(
				inRef->symmetricRef, &newRef->symmetricRef );
		
		if ( IsPGPError( err ) )
		{
			PGPFreeCBCContext( newRef );
			newRef	= NULL;
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	*outRef	= newRef;
	return( err );
}



/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError 
PGPInitCBC(
	PGPCBCContextRef	ref,
	const void *		key,
	const void *		initializationVector )
{
	PGPError			err	= kPGPError_NoErr;
	
	PGPValidateCBC( ref );
	/* at least one param must be non-nil */
	PGPValidateParam( IsntNull( key ) || IsntNull( initializationVector ) );
		
	pgpCBCInit( ref, key, initializationVector);
	
	return( err );
}





/*____________________________________________________________________________
____________________________________________________________________________*/
	static PGPError
PGPCBCEncryptDecrypt(
	PGPBoolean			encrypt,
	PGPCBCContextRef	ref,
	const void *		in,
	PGPSize				bytesIn,
	void *				out )
{
	PGPError				err	= kPGPError_NoErr;
	
	PGPValidatePtr( out );
	PGPValidateCBC( ref );
	PGPValidatePtr( in );
	PGPValidateParam( bytesIn != 0 );

	if ( ref->CBCInited )
	{
		if ( encrypt )
			err = pgpCBCEncrypt( ref, in, bytesIn, out);
		else
			err = pgpCBCDecrypt( ref, in, bytesIn, out);
	}
	else
	{
		err	= kPGPError_ImproperInitialization;
	}
	
	return( err );
}


/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError 
PGPCBCEncrypt(
	PGPCBCContextRef	ref,
	const void *		in,
	PGPSize				bytesIn,
	void *				out )
{
	return( PGPCBCEncryptDecrypt( TRUE, ref, in, bytesIn, out ) );
}

					
/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError 
PGPCBCDecrypt(
	PGPCBCContextRef	ref,
	const void *		in,
	PGPSize				bytesIn,
	void *				out )
{
	return( PGPCBCEncryptDecrypt( FALSE, ref, in, bytesIn, out ) );
}



/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError 
PGPCBCGetSymmetricCipher(
	PGPCBCContextRef				ref,
	PGPSymmetricCipherContextRef *	outRef )
{
	PGPError						err	= kPGPError_NoErr;
	PGPSymmetricCipherContextRef	symmetricRef	= NULL;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateCBC( ref );

	symmetricRef	= ref->symmetricRef;
	
	*outRef	= symmetricRef;
	return( err );
}





#if PRAGMA_MARK_SUPPORTED
#pragma mark --- Internal Routines ---
#endif








/*____________________________________________________________________________
	Initialize contexts.
	If key is NULL, the current key is not changed.
	if iv is NULL, the IV is set to all zero.
____________________________________________________________________________*/
	static void
pgpCBCInit(
	PGPCBCContext *		ref,
	void const *		key,
	void const *		iv)
{
	PGPSize			blockSize;
	void const *	curIV	= iv;
	
	PGPGetSymmetricCipherSizes( ref->symmetricRef, NULL, &blockSize );
	
	if ( IsntNull( key ) )
	{
		PGPInitSymmetricCipher( ref->symmetricRef, key );
	}

	pgpClearMemory( ref->iv, sizeof( ref->iv )  );
	if ( IsntNull( iv ) )
	{
		pgpCopyMemory( curIV, &ref->iv, blockSize );
	}

	ref->CBCInited		= TRUE;
}



/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPSize
pgpCBCGetKeySize( PGPCBCContextRef ref )
{
	PGPSize	keySize;
	
	pgpAssert( pgpCBCIsValid( ref ) );
	
	PGPGetSymmetricCipherSizes( ref->symmetricRef, &keySize, NULL );
	return( keySize );
}


/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPSize
pgpCBCGetBlockSize( PGPCBCContextRef ref )
{
	PGPSize	blockSize;
	
	pgpAssert( pgpCBCIsValid( ref ) );
	
	PGPGetSymmetricCipherSizes( ref->symmetricRef, NULL, &blockSize );
	return( blockSize );
}




/*____________________________________________________________________________
	Encrypt a buffer of data blocks, using a block cipher in CBC mode.
____________________________________________________________________________*/
	static PGPError
pgpCBCEncrypt(
	PGPCBCContext *		ref,
	void const *		srcParam,
	PGPSize				len,
	void *				destParam )
{
	PGPSize			blockSize;
	PGPSize			bufcnt;
	PGPByte *		bufptr;
	const PGPByte *	src = (const PGPByte *) srcParam;
	PGPByte *		dest = (PGPByte *) destParam;
	
	PGPGetSymmetricCipherSizes( ref->symmetricRef, NULL, &blockSize );
	
	/* Length must be a multiple of blocksize */
	if( len % blockSize != 0 )
	{
		return kPGPError_BadParams;
	}

	while( len != 0 )
	{
		bufptr		= ref->iv;
		bufcnt		= blockSize;

		/* XOR new data into iv buffer */
		while( bufcnt-- )
		{
			*bufptr++ ^= *src++;
		}

		/* Encrypt IV buffer to itself to form ciphertext */
		bufptr = ref->iv;
		PGPSymmetricCipherEncrypt(ref->symmetricRef, bufptr, bufptr);

		/* Copy ciphertext to destination buffer */
		bufcnt = blockSize;
		while( bufcnt-- )
		{
			*dest++ = *bufptr++;
		}

		/* Loop until we have exhausted the data */
		len -= blockSize;
	}

	return kPGPError_NoErr;
}


/*____________________________________________________________________________
	Decrypt a buffer of data blocks, using a block cipher in CBC mode.
____________________________________________________________________________*/
	static PGPError
pgpCBCDecrypt(
	PGPCBCContext *	ref,
	void const *	srcParam,
	PGPSize			len,
	void *			destParam )
{
	PGPSize			blockSize;
	PGPSize			bufcnt;
	PGPByte *		bufptr;
	const PGPByte *	src = (const PGPByte *) srcParam;
	PGPByte *		dest = (PGPByte *) destParam;
	
	PGPGetSymmetricCipherSizes( ref->symmetricRef, NULL, &blockSize );
	
	/* Length must be a multiple of blocksize */
	if( len % blockSize != 0 )
	{
		return kPGPError_BadParams;
	}

	while( len != 0 )
	{
		/* Decrypt incoming ciphertext into output buffer */
		PGPSymmetricCipherDecrypt(ref->symmetricRef, src, dest);

		/* XOR iv into output data to form plaintext */
		bufcnt		= blockSize;
		bufptr		= ref->iv;
		while( bufcnt-- )
		{
			*dest++ ^= *bufptr++;
		}

		/* Copy input ciphertext into iv buffer */
		bufcnt		= blockSize;
		bufptr		= ref->iv;
		while( bufcnt-- )
		{
			*bufptr++ = *src++;
		}

		/* Loop until we have exhausted the data */
		len -= blockSize;
	}

	return kPGPError_NoErr;
}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
