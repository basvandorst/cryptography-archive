/*____________________________________________________________________________
	pgpHMAC.c
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpHMAC.c,v 1.2 1999/03/10 02:51:37 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpMem.h"

#include "pgpHMAC.h"
#include "pgpHash.h"

#define kPGPHMAC_ipad			0x36
#define kPGPHMAC_opad			0x5C
#define kPGPHMAC_MaxHashBlock	64


struct PGPHMACContext
{
#define kHMACContextMagic		0xEBBADBBA
	PGPUInt32			magic;
	PGPMemoryMgrRef		memoryMgr;
	PGPHashContextRef	hash;
	PGPByte *			secret;
	PGPSize				secretLen;
};

	static PGPBoolean
pgpHMACContextIsValid( PGPHMACContextRef ref)
{
	return( IsntNull( ref ) &&
			IsntNull( ref->hash ) &&
			ref->magic == kHMACContextMagic  );
}


#define pgpValidateHMAC( ref )		\
	PGPValidateParam( pgpHMACContextIsValid( ref ) )
	
	PGPError 
PGPNewHMACContext(
	PGPMemoryMgrRef		memoryMgr,
	PGPHashAlgorithm	algorithm,
	PGPByte *			secret,
	PGPSize				secretLen,
	PGPHMACContextRef *	outRef )
{
	PGPError			err	= kPGPError_NoErr;
	PGPHMACContextRef	ref = NULL;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateMemoryMgr( memoryMgr );
	
	ref	= (PGPHMACContextRef) PGPNewData( memoryMgr, sizeof( *ref ),
											kPGPMemoryMgrFlags_Clear );
	if ( IsntNull( ref ) )
	{
		ref->magic = kHMACContextMagic;
		ref->memoryMgr	= memoryMgr;
		err = PGPNewHashContext( memoryMgr, algorithm, &ref->hash );
		if ( IsPGPError( err ) )
			goto done;
		
		if( secretLen > kPGPHMAC_MaxHashBlock )
		{
			/* revert it down to the hash size */
			err = PGPGetHashSize( ref->hash, &ref->secretLen );
			if( IsPGPError( err ) )
				goto done;
			ref->secret	= (PGPByte *) PGPNewSecureData( memoryMgr,
													ref->secretLen, 0 );
			if ( IsNull( ref->secret ) )
			{
				err	= kPGPError_OutOfMemory;
				goto done;
			}
			err = PGPContinueHash( ref->hash, secret, secretLen );
			if( IsPGPError( err ) )
				goto done;
			err = PGPFinalizeHash( ref->hash, ref->secret );
			if( IsPGPError( err ) )
				goto done;
		}
		else
		{
			ref->secretLen = secretLen;
			ref->secret	=  (PGPByte *) PGPNewSecureData( memoryMgr,
															secretLen, 0 );
			if ( IsNull( ref->secret ) )
			{
				err	= kPGPError_OutOfMemory;
				goto done;
			}
			pgpCopyMemory( secret, ref->secret, secretLen );
		}
		err = PGPResetHMAC( ref );
	}
	else
		err	= kPGPError_OutOfMemory;
done:
	if( IsPGPError( err ) )
	{
		if( PGPHMACContextRefIsValid( ref ) )
		{
			if( PGPHashContextRefIsValid( ref->hash ) )
				PGPFreeHashContext( ref->hash );
			if( IsntNull( ref->secret ) )
				PGPFreeData( ref->secret );
			PGPFreeData( ref );
			ref = NULL;
		}
	}
	*outRef	= ref;
	return( err );
}

	PGPError 
PGPFreeHMACContext( PGPHMACContextRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	
	pgpValidateHMAC( ref );
	
	PGPFreeData( ref->secret );
	PGPFreeHashContext( ref->hash );
	pgpClearMemory( ref, sizeof( *ref ) );
	PGPFreeData( ref );
	
	return( err );
}

	PGPError 
PGPResetHMAC( PGPHMACContextRef ref )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateHMAC( ref );
	
	err = PGPResetHash( ref->hash );
	if( IsntPGPError( err ) )
	{
		PGPByte		bstr[kPGPHMAC_MaxHashBlock];
		PGPUInt32	bindex;
		
		pgpCopyMemory( ref->secret, bstr, ref->secretLen );
		for( bindex = ref->secretLen; bindex < kPGPHMAC_MaxHashBlock; bindex++ )
			bstr[bindex] = '\0';
		for( bindex = 0; bindex < kPGPHMAC_MaxHashBlock; bindex++ )
			bstr[bindex] ^= kPGPHMAC_ipad;
		err = PGPContinueHash( ref->hash, bstr, kPGPHMAC_MaxHashBlock );
	}
	
	return( err );
}

	PGPError 
PGPContinueHMAC(
	PGPHMACContextRef	ref,
	const void *		in,
	PGPSize				numBytes )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateHMAC( ref );
	PGPValidatePtr( in );

	if ( numBytes != 0 )
		err = PGPContinueHash( ref->hash, in, numBytes );
	
	return( err );
}

	PGPError 
PGPFinalizeHMAC(
	PGPHMACContextRef	ref,
	void *				hmacOut )
{
	PGPError			err	= kPGPError_NoErr;
	PGPSize				hashSize;
	PGPByte				bstr[kPGPHMAC_MaxHashBlock];
	PGPUInt32			bindex;
	
	pgpValidateHMAC( ref );
	PGPValidatePtr( hmacOut );
	
	(void)PGPGetHashSize( ref->hash, &hashSize);
	err = PGPFinalizeHash( ref->hash, hmacOut );
	if( IsPGPError( err ) )
		goto done;
	err = PGPResetHash( ref->hash );
	if( IsPGPError( err ) )
		goto done;
	pgpCopyMemory( ref->secret, bstr, ref->secretLen );
	for( bindex = ref->secretLen; bindex < kPGPHMAC_MaxHashBlock; bindex++ )
		bstr[bindex] = '\0';
	for( bindex = 0; bindex < kPGPHMAC_MaxHashBlock; bindex++ )
		bstr[bindex] ^= kPGPHMAC_opad;
	err = PGPContinueHash( ref->hash, bstr, kPGPHMAC_MaxHashBlock );
	if( IsPGPError( err ) )
		goto done;
	err = PGPContinueHash( ref->hash, hmacOut, hashSize );
	if( IsPGPError( err ) )
		goto done;
	err = PGPFinalizeHash( ref->hash, hmacOut );
	if( IsPGPError( err ) )
		goto done;
done:
	return( err );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
