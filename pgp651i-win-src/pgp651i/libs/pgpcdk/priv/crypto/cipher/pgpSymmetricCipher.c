/*____________________________________________________________________________
	pgpSymmetricCipher.c
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpSymmetricCipher.c,v 1.45.6.1 1999/06/11 06:14:29 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include "pgpSymmetricCipherPriv.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpUtilitiesPriv.h"

#include "pgpOpaqueStructs.h"
#include "pgpSDKBuildFlags.h"
#include "pgpCAST5.h"
#include "pgpDES3.h"
#include "pgpRC2.h"
#include "pgpEnv.h"

#ifndef PGP_IDEA
#error you must define PGP_IDEA one way or the other
#endif

#if PGP_IDEA
#include "pgpIDEA.h"
#endif

#include "pgpSymmetricCipher.h"
#include "pgpSymmetricCipherPriv.h"

#if PGP_ENCRYPT_DISABLE && PGP_DECRYPT_DISABLE
	#define PGP_HAVE_SYMMETRIC_CIPHERS	0
#else
	#define PGP_HAVE_SYMMETRIC_CIPHERS	1
#endif

struct PGPSymmetricCipherContext
{
#define kSymmetricContextMagic		0xABBADABA
	PGPUInt32				magic;
	PGPMemoryMgrRef			memoryMgr;
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
	
	vtbl = pgpCipherGetVTBL( algorithm );
	if ( IsntNull( vtbl ) )
	{
		valid	= keySize == vtbl->keysize;
	}
	
	return( valid );
}



	static PGPError
sSymmetricCipherCreate(
	PGPMemoryMgrRef					memoryMgr,
	PGPCipherAlgorithm				algorithm,
	PGPSymmetricCipherContextRef *	outRef )
{
	PGPSymmetricCipherContextRef	ref	= NULL;
	PGPError						err	= kPGPError_OutOfMemory;
	
	ref	= (PGPSymmetricCipherContextRef)
		PGPNewData( memoryMgr, sizeof( *ref ),
				0 | kPGPMemoryMgrFlags_Clear );
	
	if ( IsntNull( ref ) )
	{
		PGPCipherVTBL const *	vtbl	= pgpCipherGetVTBL( algorithm );
		void *					cipherData;
		
		ref->vtbl	= vtbl;
			
		cipherData	= PGPNewData( memoryMgr,
						vtbl->context_size,
						0 | kPGPMemoryMgrFlags_Clear);
			
		if ( IsntNull( cipherData ) )
		{
			ref->cipherData	= cipherData;	
			ref->magic		= kSymmetricContextMagic;
			ref->memoryMgr	= memoryMgr;
			ref->keyInited	= FALSE;
			err	= kPGPError_NoErr;
		}
		else
		{
			PGPFreeData( ref );
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
pgpNewSymmetricCipherContextInternal(
	PGPMemoryMgrRef					memoryMgr,
	PGPCipherAlgorithm				algorithm,
	PGPSize							keySize,
	PGPSymmetricCipherContextRef *	outRef )
{
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateMemoryMgr( memoryMgr );
	PGPValidateParam( IsValidKeySizeAndAlgorithm( algorithm, keySize ) );
	
	err	= sSymmetricCipherCreate( memoryMgr, algorithm, outRef );
	
	pgpAssertErrWithPtr( err, *outRef );
	return( err );
}

	PGPError 
PGPNewSymmetricCipherContext(
	PGPMemoryMgrRef					memoryMgr,
	PGPCipherAlgorithm				algorithm,
	PGPSize							keySize,
	PGPSymmetricCipherContextRef *	outRef)
{
	PGPError	err;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateMemoryMgr( memoryMgr );

#if PGP_HAVE_SYMMETRIC_CIPHERS
	err = pgpNewSymmetricCipherContextInternal( memoryMgr, algorithm,
					keySize, outRef );
#else

	(void) algorithm;
	(void) keySize;
	
	err = kPGPError_FeatureNotAvailable;
#endif

	return( err );
}




	PGPError 
PGPFreeSymmetricCipherContext( PGPSymmetricCipherContextRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	PGPMemoryMgrRef	memoryMgr;
	
	pgpValidateSymmetricCipher( ref );
	
	memoryMgr = ref->memoryMgr;
	
	PGPWipeSymmetricCipher( ref );
	PGPFreeData( ref->cipherData );
	
	pgpClearMemory( ref, sizeof( *ref ) );
	PGPFreeData( ref );
	
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
				PGPNewData( ref->memoryMgr,
				sizeof( *newRef ), 0 );
	if ( IsntNull( newRef ) )
	{
		void *		newData;
		PGPUInt32	dataSize	= ref->vtbl->context_size;
		
		*newRef	= *ref;
		
		newData	= PGPNewData( ref->memoryMgr,
						dataSize, 0 );
		if ( IsntNull( newData ) )
		{
			pgpCopyMemory( ref->cipherData, newData, dataSize );
			newRef->cipherData	= newData;
		}
		else
		{
			PGPFreeData( newRef );
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
pgpSymmetricCipherEncryptInternal(
	PGPSymmetricCipherContextRef	ref,
	const void *					in,
	void *							out )
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( ref->keyInited )
	{
		CallEncrypt( ref, in, out );
	}
	else
	{
		err	= kPGPError_ImproperInitialization;
	}
	
	
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

#if PGP_ENCRYPT_DISABLE
	err = kPGPError_FeatureNotAvailable;
#else
	err = pgpSymmetricCipherEncryptInternal( ref, in, out );
#endif
	
	return( err );
}

					
	PGPError 
pgpSymmetricCipherDecryptInternal(
	PGPSymmetricCipherContextRef	ref,
	const void *					in,
	void *							out )
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( ref->keyInited )
	{
		CallDecrypt( ref, in, out );
	}
	else
	{
		err	= kPGPError_ImproperInitialization;
	}
	
	return( err );
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

#if PGP_DECRYPT_DISABLE
	err = kPGPError_FeatureNotAvailable;
#else
	err = pgpSymmetricCipherDecryptInternal( ref, in, out );
#endif
	
	return( err );
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



	PGPMemoryMgrRef
pgpGetSymmetricCipherMemoryMgr( PGPSymmetricCipherContextRef	ref)
{
	AssertSymmetricContextValid( ref );
	
	return( ref->memoryMgr );
}


static PGPCipherVTBL const * const sCipherList[] =
{
#if PGP_IDEA
	&cipherIDEA,
#endif
	&cipher3DES,
	&cipherCAST5,
	&cipherRC2_40,
	&cipherRC2_128
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
 * Choose cipher for key encryption.  Not used for V2-V3 compatible keys.
 * Override pgpenv defaults with our own default.
 */
PGPCipherVTBL const *
pgpCipherDefaultKey(PGPEnv const *env)
{
	PGPInt32			ciphernum;
	PGPInt32			cipherpri;
	const PGPCipherVTBL *vtbl;
	
	ciphernum = pgpenvGetInt (env, PGPENV_CIPHER, &cipherpri, NULL);
	if (cipherpri < PGPENV_PRI_CONFIG)
		ciphernum = kPGPCipherAlgorithm_CAST5;
	
	vtbl = pgpCipherGetVTBL ( (PGPCipherAlgorithm)ciphernum);
	if( IsNull( vtbl ) )
	{
		pgpDebugMsg( "pgpCipherDefaultKey(): Cipher algorithm not found" );
		vtbl = pgpCipherGetVTBL ( kPGPCipherAlgorithm_CAST5 );
	}
	
	return( vtbl );
}

/* Use this for old pre-PGP5 key encryption; default to backwards compat */
PGPCipherVTBL const *
pgpCipherDefaultKeyV3(PGPEnv const *env)
{
	PGPInt32			ciphernum;
	PGPInt32			cipherpri;
	const PGPCipherVTBL *vtbl;

	ciphernum = pgpenvGetInt (env, PGPENV_CIPHER, &cipherpri, NULL);
	if (cipherpri < PGPENV_PRI_CONFIG)
		ciphernum = kPGPCipherAlgorithm_IDEA;
		
	vtbl = pgpCipherGetVTBL ( (PGPCipherAlgorithm) ciphernum);
	if( IsNull( vtbl ) )
	{
		pgpDebugMsg( "pgpCipherDefaultKeyV3(): Cipher algorithm not found" );
		vtbl = pgpCipherGetVTBL ( kPGPCipherAlgorithm_IDEA );
	}
	
	return( vtbl );
}






















/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
