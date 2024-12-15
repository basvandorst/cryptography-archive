/*____________________________________________________________________________
	pgpCBC.c
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpCBC.c,v 1.6 1999/03/10 02:47:06 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"
#include <string.h>

#ifndef USERLAND_TEST
#define USERLAND_TEST	0
#endif

#if USERLAND_TEST

#define DBG_FUNC(x)
#define DBG_ENTER()
#define DBG_PRINT(x)
#define DBG_LEAVE(x)

#else

#if PGP_WIN32

#include <ndis.h>
#include "vpndbg.h"

#elif PGP_MACINTOSH

#define DBG_FUNC(x)
#define DBG_ENTER()
#define DBG_PRINT(x)
#define DBG_LEAVE(x)

#endif

#endif

#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpCBCPriv.h"



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
	PGPMemoryMgrRef					memoryMgr;
	PGPBoolean						CBCInited;
	PGPSymmetricCipherContextRef	symmetricRef;
	PGPByte							iv1[ PGP_CBC_MAXBLOCKSIZE ];
	PGPByte							iv2[ PGP_CBC_MAXBLOCKSIZE ];
	PGPByte *						iv;
};

	static PGPBoolean
pgpCBCIsValid( const PGPCBCContext * ref)
{
	PGPBoolean	valid	= FALSE;
	
	valid	= IsntNull( ref ) && ref->magic	 == kCBCMagic
				&& (ref->iv == ref->iv1  ||  ref->iv == ref->iv2);
	
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
	PGPMemoryMgrRef					memoryMgr	= NULL;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidatePtr( symmetricRef );
	
	memoryMgr	= pgpGetSymmetricCipherMemoryMgr( symmetricRef );
	newRef	= (PGPCBCContextRef)
			PGPNewData( memoryMgr,
				sizeof( *newRef ), 0 | kPGPMemoryMgrFlags_Clear);
			
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

		newRef->magic			= kCBCMagic;
		newRef->CBCInited		= FALSE;
		newRef->symmetricRef	= symmetricRef;
		newRef->iv				= newRef->iv1;
		newRef->memoryMgr		= memoryMgr;
		
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
	
	PGPValidateCBC( ref );
	
	PGPFreeSymmetricCipherContext( ref->symmetricRef );
	
	pgpClearMemory( ref, sizeof( *ref ) );
	PGPFreeData( ref );
	
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
		PGPNewData( inRef->memoryMgr,
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
		else
		{
			if (inRef->iv == inRef->iv1)
				newRef->iv = newRef->iv1;
			if (inRef->iv == inRef->iv2)
				newRef->iv = newRef->iv2;
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
	
	DBG_FUNC("PGPCBCEncryptDecrypt")

	DBG_ENTER();

	PGPValidatePtr( out );
	PGPValidateCBC( ref );
	PGPValidatePtr( in );
	PGPValidateParam( bytesIn != 0 );

	if ( ref->CBCInited )
	{
		if ( encrypt ) {
			err = pgpCBCEncrypt( ref, in, bytesIn, out);
		} else {
			err = pgpCBCDecrypt( ref, in, bytesIn, out);
		}
	}
	else
	{
		err	= kPGPError_ImproperInitialization;
	}
	
	DBG_LEAVE(err);
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

	pgpClearMemory( ref->iv1, sizeof( ref->iv1 )  );
	pgpClearMemory( ref->iv2, sizeof( ref->iv2 )  );
	ref->iv = ref->iv1;

	if ( IsntNull( iv ) )
	{
		pgpCopyMemory( curIV, ref->iv, blockSize );
	}

	/* rely on the symmetric cipher to know whether it has been inited */
	/* also, iv of NULL is OK, since semantics say that means zeroes */
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
		PGPByte *	altiv;

		/* Copy input ciphertext into alternate iv buffer */
		altiv		= (ref->iv == ref->iv1) ? ref->iv2 : ref->iv1;
		bufcnt		= blockSize;
		bufptr		= altiv;
		while( bufcnt-- )
		{
			*bufptr++ = *src++;
		}

		/* Decrypt ciphertext into destination */
		PGPSymmetricCipherDecrypt(ref->symmetricRef, altiv, dest);

		/* XOR iv into output data to form plaintext */
		bufcnt		= blockSize;
		bufptr		= ref->iv;
		while( bufcnt-- )
		{
			*dest++ ^= *bufptr++;
		}

		/* Toggle the IV buffers each iteration */
		ref->iv = altiv;
		
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
