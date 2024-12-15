/*____________________________________________________________________________
	pgpSymmetricCipher.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpSymmetricCipher.c,v 1.32 1997/10/14 01:48:18 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include "pgpSymmetricCipherPriv.h"
#include "pgpMem.h"
#include "pgpContext.h"
#include "pgpErrors.h"
#include "pgpUtilitiesPriv.h"

#include "pgpOpaqueStructs.h"
#include "pgpCAST5.h"
#include "pgpIDEA.h"
#include "pgpDES3.h"
#include "pgpEnv.h"

#include "pgpSymmetricCipher.h"
#include "pgpSymmetricCipherPriv.h"



struct PGPSymmetricCipherContext
{
#define kSymmetricContextMagic		0xABBADABA
	PGPUInt32				magic;
	PGPContextRef			context;
	PGPCipherVTBL const *	vtbl;
	void *					cipherData;
	PGPBoolean				keyInited;
} ;



/* Macros to access the member functions */
#define CallInitKey(cc, k) ((cc)->vtbl->initKey((cc)->cipherData, k))
#define CallEncrypt(cc, in, out) \
			((cc)->vtbl->encrypt((cc)->cipherData, in, out))
#define CallDecrypt(cc, in, out) \
			((cc)->vtbl->decrypt((cc)->cipherData, in, out))
#define CallWash(cc, buf, len) \
			((cc)->vtbl->wash((cc)->cipherData, buf, len))


	PGPBoolean
pgpSymmetricCipherIsValid( const PGPSymmetricCipherContext * ref)
{
	PGPBoolean	valid	= FALSE;
	
	valid	= IsntNull( ref ) && ref->magic	 == kSymmetricContextMagic;
	
	return( valid );
}
#define pgpValidateSymmetricCipher( s )		\
	PGPValidateParam( pgpSymmetricCipherIsValid( s ) )

#if PGP_DEBUG
#define AssertSymmetricContextValid( ref )	\
	pgpAssert( pgpSymmetricCipherIsValid( ref ) )
#else
#define AssertSymmetricContextValid( ref )
#endif


	static PGPBoolean
IsValidKeySizeAndAlgorithm(
	PGPCipherAlgorithm	algorithm,
	PGPSize				keySize )
{
	PGPBoolean				valid	= FALSE;
	PGPCipherVTBL const *	vtbl	= NULL;
	
	vtbl	= pgpCipherGetVTBL( algorithm );
	if ( IsntNull( vtbl ) )
	{
		valid	= keySize == vtbl->keysize;
	}
	
	return( valid );
}



	static PGPError
sSymmetricCipherCreate(
	PGPContextRef					context,
	PGPCipherAlgorithm				algorithm,
	PGPSymmetricCipherContextRef *	outRef )
{
	PGPSymmetricCipherContextRef	ref	= NULL;
	PGPError						err	= kPGPError_OutOfMemory;
	
	ref	= (PGPSymmetricCipherContextRef)
		pgpContextMemAlloc( context, sizeof( *ref ),
				0 | kPGPMemoryFlags_Clear );
	
	if ( IsntNull( ref ) )
	{
		PGPCipherVTBL const *	vtbl	= pgpCipherGetVTBL( algorithm );
		void *					cipherData;
		
		ref->vtbl	= vtbl;
			
		cipherData	= pgpContextMemAlloc( context,
						vtbl->context_size,
						0 | kPGPMemoryFlags_Clear);
			
		if ( IsntNull( cipherData ) )
		{
			ref->cipherData	= cipherData;	
			ref->magic		= kSymmetricContextMagic;
			ref->context	= context;
			ref->keyInited	= FALSE;
			err	= kPGPError_NoErr;
		}
		else
		{
			pgpContextMemFree( context, ref );
			ref	= NULL;
			err	= kPGPError_OutOfMemory;
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	*outRef	= ref;
	return( err );
}



	PGPError 
PGPNewSymmetricCipherContext(
	PGPContextRef					context,
	PGPCipherAlgorithm				algorithm,
	PGPSize							keySize,
	PGPSymmetricCipherContextRef *	outRef )
{
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateContext( context );
	PGPValidateParam( IsValidKeySizeAndAlgorithm( algorithm, keySize ) );
	
	err	= sSymmetricCipherCreate( context, algorithm, outRef );
	
	pgpAssertErrWithPtr( err, *outRef );
	return( err );
}




	PGPError 
PGPFreeSymmetricCipherContext( PGPSymmetricCipherContextRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	PGPContextRef	context;
	
	pgpValidateSymmetricCipher( ref );
	
	context = ref->context;
	
	PGPWipeSymmetricCipher( ref );
	pgpContextMemFree( context, ref->cipherData );
	
	pgpClearMemory( ref, sizeof( *ref ) );
	pgpContextMemFree( context, ref );
	
	return( err );
}



	PGPError 
PGPCopySymmetricCipherContext(
	PGPSymmetricCipherContextRef	ref,
	PGPSymmetricCipherContextRef *	outRef )
{
	PGPError						err	= kPGPError_NoErr;
	PGPSymmetricCipherContextRef	newRef	= NULL;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	pgpValidateSymmetricCipher( ref );
	
	newRef	= (PGPSymmetricCipherContextRef)
				pgpContextMemAlloc( ref->context,
				sizeof( *newRef ), 0 );
	if ( IsntNull( newRef ) )
	{
		void *		newData;
		PGPUInt32	dataSize	= ref->vtbl->context_size;
		
		*newRef	= *ref;
		
		newData	= pgpContextMemAlloc( ref->context,
						dataSize, 0 );
		if ( IsntNull( newData ) )
		{
			pgpCopyMemory( ref->cipherData, newData, dataSize );
			newRef->cipherData	= newData;
		}
		else
		{
			pgpContextMemFree( newRef->context, newRef );
			err	= kPGPError_OutOfMemory;
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	*outRef	= newRef;
	pgpAssertErrWithPtr( err, *outRef );
	return( err );
}


	PGPError 
PGPInitSymmetricCipher(
	PGPSymmetricCipherContextRef	ref,
	const void *					key )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateSymmetricCipher( ref );
	PGPValidatePtr( key );
	
	CallInitKey( ref, key );
	ref->keyInited	= TRUE;
	
	return( err );
}



	PGPError 
PGPWipeSymmetricCipher( PGPSymmetricCipherContextRef ref )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateSymmetricCipher( ref );
	
	pgpClearMemory( ref->cipherData, ref->vtbl->context_size);
	ref->keyInited	= FALSE;
	
	return( err );
}



	PGPError 
PGPWashSymmetricCipher(
	PGPSymmetricCipherContextRef	ref,
	void const *					buf,
	PGPSize							len)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateSymmetricCipher( ref );
	PGPValidatePtr( buf );
	
	CallWash( ref, buf, len );
	ref->keyInited = TRUE;
	
	return( err );
}



	PGPError 
PGPSymmetricCipherEncrypt(
	PGPSymmetricCipherContextRef	ref,
	const void *					in,
	void *							out )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateSymmetricCipher( ref );
	PGPValidatePtr( in );
	PGPValidatePtr( out );

	if ( ref->keyInited )
	{
		CallEncrypt( ref, in, out );
	}
	else
	{
		err	= kPGPError_ImproperInitialization;
	}
	
	
	return( kPGPError_NoErr );
}

					
	PGPError 
PGPSymmetricCipherDecrypt(
	PGPSymmetricCipherContextRef	ref,
	const void *					in,
	void *							out )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateSymmetricCipher( ref );
	PGPValidatePtr( in );
	PGPValidatePtr( out );

	if ( ref->keyInited )
	{
		CallDecrypt( ref, in, out );
	}
	else
	{
		err	= kPGPError_ImproperInitialization;
	}
	
	
	return( kPGPError_NoErr );
}

					
	PGPError 
PGPGetSymmetricCipherSizes(
	PGPSymmetricCipherContextRef	ref,
	PGPSize *						keySizePtr,
	PGPSize *						blockSizePtr )
{
	PGPError	err	= kPGPError_NoErr;
	PGPSize		blockSize	= 0;
	PGPSize		keySize		= 0;
	
	if ( IsntNull( keySizePtr ) )
		*keySizePtr	= 0;
	if ( IsntNull( blockSizePtr ) )
		*blockSizePtr	= 0;
		
	pgpValidateSymmetricCipher( ref );
	PGPValidateParam( IsntNull( blockSizePtr ) || IsntNull( keySizePtr ) );

	blockSize	= ref->vtbl->blocksize;
	keySize		= ref->vtbl->keysize;
		
	if ( IsntNull( blockSizePtr ) )
	{
		pgpAssertAddrValid( blockSizePtr, PGPUInt32 );
		*blockSizePtr	= blockSize;
	}
	
	if ( IsntNull( keySizePtr ) )
	{
		pgpAssertAddrValid( keySizePtr, PGPUInt32 );
		*keySizePtr	= keySize;
	}
	
	return( err );
}



	PGPContextRef
pgpGetSymmetricCipherContext( PGPSymmetricCipherContextRef	ref)
{
	AssertSymmetricContextValid( ref );
	
	return( ref->context );
}


static PGPCipherVTBL const * const sCipherList[] =
{
#if PGP_IDEA
	&cipherIDEA,
#endif
	&cipher3DES,
	&cipherCAST5
};
#define kNumCiphers	 ( sizeof( sCipherList ) / sizeof( sCipherList[ 0 ] ) )


	PGPUInt32
pgpCountSymmetricCiphers( void )
{
	return( kNumCiphers );
}


	PGPCipherVTBL const *
pgpCipherGetVTBL (PGPCipherAlgorithm	algorithm)
{
	const PGPCipherVTBL *	vtbl	= NULL;
	PGPUInt32				algIndex;
	
	for( algIndex = 0; algIndex < kNumCiphers; ++algIndex )
	{
		if ( sCipherList[ algIndex ]->algorithm == algorithm )
		{
			vtbl	= sCipherList[ algIndex ];
			break;
		}
	}
	
	return vtbl;
}



/*
 * Choose cipher for key encryption.  Not used for V2 compatible keys.
 * Override pgpenv defaults with our own default.
 */
PGPCipherVTBL const *
pgpCipherDefaultKey(PGPEnv const *env)
{
	PGPInt32	ciphernum;
	PGPInt32	cipherpri;

	ciphernum = pgpenvGetInt (env, PGPENV_CIPHER, &cipherpri, NULL);
	if (cipherpri < PGPENV_PRI_CONFIG)
		ciphernum = kPGPCipherAlgorithm_CAST5;
	return pgpCipherGetVTBL ( (PGPCipherAlgorithm)ciphernum);
}

/* Use this for old pre-PGP3 key encryption; default to backwards compat */
PGPCipherVTBL const *
pgpCipherDefaultKeyV2(PGPEnv const *env)
{
	PGPInt32	ciphernum;
	PGPInt32	cipherpri;

	ciphernum = pgpenvGetInt (env, PGPENV_CIPHER, &cipherpri, NULL);
	if (cipherpri < PGPENV_PRI_CONFIG)
		ciphernum = kPGPCipherAlgorithm_IDEA;
	return pgpCipherGetVTBL ( (PGPCipherAlgorithm) ciphernum);
}






















/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
