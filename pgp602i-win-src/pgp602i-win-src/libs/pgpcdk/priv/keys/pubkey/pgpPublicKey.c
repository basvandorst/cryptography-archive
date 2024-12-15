/*____________________________________________________________________________
	pgpPublicKey.c
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpPublicKey.c,v 1.13.10.1 1998/11/12 03:22:11 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"
#include <string.h>

#include "pgpPublicKey.h"
#include "pgpHash.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpHashPriv.h"
#include "pgpUtilitiesPriv.h"
#include "pgpKDBInt.h"
#include "pgpPubKey.h"
#include "pgpKeyMisc.h"
#include "pgpOptionListPriv.h"

#define PGPValidatePublicKey( pkref )	\
	PGPValidateParam( pgpPublicKeyIsValid( pkref ) );
#define PGPValidatePrivateKey( pkref )	\
	PGPValidateParam( pgpPrivateKeyIsValid( pkref ) );
#define PGPValidateMessageFormat( messageFormat ) \
	PGPValidateParam( pgpMessageFormatIsValid( messageFormat ) );
	
#define elemsof(x) ((unsigned)(sizeof(x)/sizeof(*x)))

struct PGPPublicKeyContext
{
#define kPublicKeyMagic				0x58210770
	PGPUInt32						magic;
	PGPContextRef					context;
	PGPKeyRef						key;
	PGPPublicKeyMessageFormat		format;
	PGPPubKey						*enckey;
	PGPPubKey						*signkey;
};

struct PGPPrivateKeyContext
{
#define kPrivateKeyMagic			0x58249779
	PGPUInt32						magic;
	PGPContextRef					context;
	PGPKeyRef						key;
	PGPPublicKeyMessageFormat		format;
	PGPSecKey						*enckey;
	PGPSecKey						*signkey;
};


	static PGPBoolean
pgpPublicKeyIsValid( const PGPPublicKeyContext * ref)
{
	PGPBoolean	valid	= FALSE;
	
	valid	= IsntNull( ref ) && ref->magic	 == kPublicKeyMagic;
	
	return( valid );
}

	static PGPBoolean
pgpPrivateKeyIsValid( const PGPPrivateKeyContext * ref)
{
	PGPBoolean	valid	= FALSE;
	
	valid	= IsntNull( ref ) && ref->magic	 == kPrivateKeyMagic;
	
	return( valid );
}

	static PGPBoolean
pgpMessageFormatIsValid( PGPPublicKeyMessageFormat messageFormat )
{
	PGPBoolean	valid	= FALSE;
	
	valid = (messageFormat == kPGPPublicKeyMessageFormat_PGP) ||
			(messageFormat == kPGPPublicKeyMessageFormat_PKCS1) ||
			(messageFormat == kPGPPublicKeyMessageFormat_X509);
	
	return( valid );
}


PGPError		sSetupPubkey( PGPPublicKeyContextRef ref );
PGPError		sSetupPrivkey( PGPPrivateKeyContextRef ref,
							   char const *passphrase,
							   PGPSize passphraseLength,
							   PGPBoolean hashedPhrase );



/*____________________________________________________________________________
	Exported routines, public key operations
____________________________________________________________________________*/

	PGPError
PGPNewPublicKeyContext(
	PGPKeyRef					publicKeyRef,
	PGPPublicKeyMessageFormat	messageFormat,
	PGPPublicKeyContextRef *	outRef )
{
	PGPPublicKeyContextRef		newRef	= NULL;
	PGPContextRef				context;
	PGPError					err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKey( publicKeyRef );
	PGPValidateMessageFormat( messageFormat );
	
	context = PGPGetKeyContext( publicKeyRef );

	newRef	= (PGPPublicKeyContextRef) pgpContextMemAlloc( context,
									sizeof( *newRef ),
									kPGPMemoryMgrFlags_Clear );
			
	if ( IsntNull( newRef ) )
	{
		newRef->magic						= kPublicKeyMagic;
		newRef->context						= context;
		newRef->key							= publicKeyRef;
		newRef->format						= messageFormat;

		err = sSetupPubkey( newRef );

		if( IsPGPError( err ) )
		{
			pgpContextMemFree( context, newRef );
			newRef = NULL;
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
PGPFreePublicKeyContext( PGPPublicKeyContextRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	PGPContextRef	context	= NULL;
	
	PGPValidatePublicKey( ref );

	context	= ref->context;
	
	if( IsntNull( ref->enckey ) )
		pgpPubKeyDestroy( ref->enckey );
	if( IsntNull( ref->signkey ) )
		pgpPubKeyDestroy( ref->signkey );
	
	pgpClearMemory( ref, sizeof( *ref ) );
	pgpContextMemFree( context, ref );
	
	return( err );
}



/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError
PGPPublicKeyEncrypt(
	PGPPublicKeyContextRef	ref,
	const void *			in,
	PGPSize					inSize,
	void *					out,
	PGPSize *				outSize)
{
	PGPRandomContext *			randomContext;
	PGPSize						lOutSize;
	PGPError					err	= kPGPError_NoErr;
	
	if( IsntNull( outSize ) )
	{
		PGPValidatePtr( outSize );
		*outSize = 0;
	}
	PGPValidatePtr( out );
	PGPValidatePublicKey( ref );
	PGPValidatePtr( in );
	PGPValidateParam( inSize != 0 );

	if( IsNull( ref->enckey ) )
		return kPGPError_FeatureNotAvailable;

	randomContext = pgpContextGetX9_17RandomContext( ref->context );

	err = pgpPubKeyEncrypt( ref->enckey, (const PGPByte *) in, inSize,
							(PGPByte *) out, &lOutSize, randomContext,
							ref->format );

	if( IsntPGPError( err )  && IsntNull( outSize ) )
	{
		*outSize = lOutSize;
	}

	return( err );
}


/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError
PGPPublicKeyVerifySignature(
	PGPPublicKeyContextRef		ref,
	PGPHashContextRef			hashContext,
	const void *				signature,
	PGPSize						signatureSize)
{
	PGPError					err	= kPGPError_NoErr;
	PGPHashVTBL const *			hashVTBL;
	PGPSize						hashSize;
	PGPByte						hashData[100];
	PGPInt32					rslt;
	
	PGPValidatePublicKey( ref );
	PGPValidatePtr( hashContext );
	PGPValidatePtr( signature );
	PGPValidateParam( signatureSize != 0 );

	if( IsNull( ref->signkey ) )
		return kPGPError_FeatureNotAvailable;

	/* Hash too big for our array? */
	PGPGetHashSize( hashContext, &hashSize );
	if( hashSize > sizeof(hashData) )
	{
		PGPFreeHashContext( hashContext );
		return kPGPError_BadParams;
	}

	hashVTBL = pgpHashGetVTBL( hashContext );
	PGPFinalizeHash( hashContext, hashData );

	/* Do the verification */
	rslt = pgpPubKeyVerify( ref->signkey, (const PGPByte *) signature,
							signatureSize, hashVTBL, hashData, ref->format );

	PGPFreeHashContext( hashContext );

	if( rslt != 1 )
	{
		err = kPGPError_BadSignature;
	}

	return( err );
}


/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError
PGPPublicKeyVerifyRaw(
	PGPPublicKeyContextRef		ref,
	const void *				signedData,
	PGPSize						signedDataSize,
	const void *				signature,
	PGPSize						signatureSize)
{
	PGPError					err	= kPGPError_NoErr;
	PGPHashVTBL					dummyVTBL;
	PGPInt32					rslt;
	
	PGPValidatePublicKey( ref );
	PGPValidatePtr( signedData );
	PGPValidateParam( signedDataSize != 0 );
	/* Raw mode supported only for PKCS sigs */
	if( ref->format != 	kPGPPublicKeyMessageFormat_PKCS1 ) {
		return kPGPError_BadParams;
	}
	PGPValidatePtr( signature );
	PGPValidateParam( signatureSize != 0 );

	if( IsNull( ref->signkey ) )
		return kPGPError_FeatureNotAvailable;

	/* Set up value for hash size used by signature function */
	pgpClearMemory( &dummyVTBL, sizeof(dummyVTBL) );
	dummyVTBL.hashsize = signedDataSize;

	/* Do the verification */
	rslt = pgpPubKeyVerify( ref->signkey, (const PGPByte *) signature,
							signatureSize, &dummyVTBL,
							(const PGPByte *)signedData, ref->format );

	if( rslt != 1 )
	{
		err = kPGPError_BadSignature;
	}

	return( err );
}


/*____________________________________________________________________________
If we have an encrypt-only or sign only key, we return zeros for the
one(s) which we don't support.
____________________________________________________________________________*/
	PGPError
PGPGetPublicKeyOperationSizes(
	PGPPublicKeyContextRef	ref,
	PGPSize *				maxDecryptedBufferSize,
	PGPSize *				maxEncryptedBufferSize,
	PGPSize *				maxSignatureSize )
{
	PGPValidatePublicKey( ref );
	if( IsntNull( maxDecryptedBufferSize ) )
		PGPValidatePtr( maxDecryptedBufferSize );
	if( IsntNull( maxEncryptedBufferSize ) )
		PGPValidatePtr( maxEncryptedBufferSize );
	if( IsntNull( maxSignatureSize ) )
		PGPValidatePtr( maxSignatureSize );
	
	if( IsntNull( maxEncryptedBufferSize ) )
	{
		if( IsntNull( ref->enckey ) ) {
			*maxEncryptedBufferSize = pgpPubKeyMaxesk( ref->enckey,
													   ref->format );
		} else {
			*maxEncryptedBufferSize = 0;
		}
	}

	if( IsntNull( maxDecryptedBufferSize ) )
	{
		if( IsntNull( ref->enckey ) )
			*maxDecryptedBufferSize = pgpPubKeyMaxdecrypted( ref->enckey,
															 ref->format );
		else
			*maxDecryptedBufferSize = 0;
	}

	if( IsntNull( maxSignatureSize ) )
	{
		if( IsntNull( ref->signkey ) )
			*maxSignatureSize = pgpPubKeyMaxsig( ref->signkey, ref->format );
		else
			*maxSignatureSize = 0;
	}
	return kPGPError_NoErr;
}



/*____________________________________________________________________________
	Exported routines, private key operations
____________________________________________________________________________*/


/* Internal function to create a new private key context */
	static PGPError
sNewPrivateKeyContext (
	PGPKeyRef					privateKeyRef,
	PGPPublicKeyMessageFormat	messageFormat,
	char const *				passphrase,
	PGPSize						passphraseLength,
	PGPBoolean					hashedPhrase,
	PGPPrivateKeyContextRef *	outRef )
	
{
	PGPPrivateKeyContextRef		newRef	= NULL;
	PGPContextRef				context;
	PGPError					err	= kPGPError_NoErr;
	
	context = PGPGetKeyContext( privateKeyRef );

	newRef	= (PGPPrivateKeyContextRef) pgpContextMemAlloc( context,
									sizeof( *newRef ),
									kPGPMemoryMgrFlags_Clear );
			
	if ( IsntNull( newRef ) )
	{
		newRef->magic						= kPrivateKeyMagic;
		newRef->context						= context;
		newRef->key							= privateKeyRef;
		newRef->format						= messageFormat;

		err = sSetupPrivkey( newRef, passphrase, passphraseLength,
							 hashedPhrase );

		if( IsPGPError( err ) )
		{
			pgpContextMemFree( context, newRef );
			newRef = NULL;
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	*outRef	= newRef;
	return( err );
}



static const PGPOptionType privctxOptionSet[] = {
	 kPGPOptionType_Passphrase,
	 kPGPOptionType_Passkey,
};


	static PGPError
pgpNewPrivateKeyContextInternal (
	PGPKeyRef					privateKeyRef,
	PGPPublicKeyMessageFormat	messageFormat,
	PGPPrivateKeyContextRef *	outRef,
	PGPOptionListRef			optionList
	)
{
	char *						passphrase;
	PGPSize						passphraseLength;
	PGPBoolean					hashedPhrase = FALSE;
	PGPError					err = kPGPError_NoErr;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
					privctxOptionSet, elemsof( privctxOptionSet ) ) ) )
		return err;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	err = sNewPrivateKeyContext( privateKeyRef, messageFormat, passphrase,
								 passphraseLength, hashedPhrase, outRef );

error:

	return err;
}

/* Version of NewPrivateKeyContext which takes passphrase options */
	PGPError
PGPNewPrivateKeyContext(
	PGPKeyRef					privateKeyRef,
	PGPPublicKeyMessageFormat	messageFormat,
	PGPPrivateKeyContextRef *	outRef,
	PGPOptionListRef			firstOption,
	...
	)
{
	PGPError	error	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKey( privateKeyRef );
	PGPValidateMessageFormat( messageFormat );

	{
		va_list				args;
		PGPContextRef		context;
		PGPOptionListRef	optionList;

		context = PGPGetKeyContext( privateKeyRef );

		va_start( args, firstOption );
		optionList = pgpBuildOptionListArgs( context,
			FALSE, firstOption, args );

		error = pgpGetOptionListError( optionList );
		if( IsntPGPError( error ) )
		{
			error = pgpNewPrivateKeyContextInternal( privateKeyRef,
										messageFormat, outRef, optionList );
		}
		va_end( args );

		PGPFreeOptionList( optionList );
	}
	
	return( error );
}


#if 0
/* Replaced by PGPOptionList version above */
	PGPError
PGPNewPrivateKeyContext(
	PGPKeyRef					privateKeyRef,
	PGPPublicKeyMessageFormat	messageFormat,
	char const *				passphrase,
	PGPPrivateKeyContextRef *	outRef )
{
	PGPSize		passphraseLength = 0;
	PGPError	err;

	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKey( privateKeyRef );
	PGPValidateMessageFormat( messageFormat );

	if( IsntNull( passphrase ) )
	{
		PGPValidatePtr( passphrase );
		passphraseLength = strlen( passphrase );
	}
	
	err = sNewPrivateKeyContext (privateKeyRef, messageFormat, passphrase,
								 passphraseLength, FALSE, outRef );
	return err;
}
#endif



/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError 
PGPFreePrivateKeyContext( PGPPrivateKeyContextRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	PGPContextRef	context	= NULL;
	
	PGPValidatePrivateKey( ref );

	context	= ref->context;
	
	if( IsntNull( ref->enckey ) )
		pgpSecKeyDestroy( ref->enckey );
	if( IsntNull( ref->signkey ) )
		pgpSecKeyDestroy( ref->signkey );
	
	pgpClearMemory( ref, sizeof( *ref ) );
	pgpContextMemFree( context, ref );
	
	return( err );
}



/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError
PGPPrivateKeyDecrypt(
	PGPPrivateKeyContextRef		ref,
	const void *				in,
	PGPSize						inSize,
	void *						out,
	PGPSize *					outSize)
{
	PGPSize						lOutSize;
	PGPEnv *					env;
	PGPError					err	= kPGPError_NoErr;
	
	if( IsntNull( outSize ) )
	{
		PGPValidatePtr( outSize );
		*outSize = 0;
	}
	PGPValidatePtr( out );
	PGPValidatePrivateKey( ref );
	PGPValidatePtr( in );
	PGPValidateParam( inSize != 0 );


	if( IsNull( ref->enckey ) )
		return kPGPError_FeatureNotAvailable;

	env = pgpContextGetEnvironment( ref->context );

	err = pgpSecKeyDecrypt( ref->enckey, env, (const PGPByte *) in, inSize,
							(PGPByte *) out, &lOutSize, NULL, 0, ref->format );

	if( IsntPGPError( err )  && IsntNull( outSize ) )
	{
		*outSize = lOutSize;
	}

	return( err );
}


/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError
PGPPrivateKeySign(
	PGPPrivateKeyContextRef		ref,
	PGPHashContextRef			hashContext,
	void *						signature,
	PGPSize *					signatureSize)
{
	PGPRandomContext *			randomContext;
	PGPHashVTBL const *			hashVTBL;
	PGPSize						hashSize;
	PGPByte						hashData[100];
	PGPSize						lSignatureSize;
	PGPError					err	= kPGPError_NoErr;
	
	if( IsntNull( signatureSize ) )
	{
		PGPValidatePtr( signatureSize );
		*signatureSize = 0;
	}
	PGPValidatePrivateKey( ref );
	PGPValidatePtr( hashContext );
	PGPValidatePtr( signature );

	if( IsNull( ref->signkey ) )
		return kPGPError_FeatureNotAvailable;

	/* Hash too big for our array? */
	PGPGetHashSize( hashContext, &hashSize );
	if( hashSize > sizeof(hashData) )
	{
		PGPFreeHashContext( hashContext );
		return kPGPError_BadParams;
	}

	hashVTBL = pgpHashGetVTBL( hashContext );
	PGPFinalizeHash( hashContext, hashData );

	randomContext = pgpContextGetX9_17RandomContext( ref->context );

	/* Do the signature */
	err = pgpSecKeySign( ref->signkey, hashVTBL, hashData,
						 (PGPByte *) signature, &lSignatureSize,
						  randomContext, ref->format );

	PGPFreeHashContext( hashContext );

	if( IsntPGPError( err )  && IsntNull( signatureSize ) )
	{
		*signatureSize = lSignatureSize;
	}

	return( err );
}


/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError
PGPPrivateKeySignRaw(
	PGPPrivateKeyContextRef		ref,
	const void *				signedData,
	PGPSize						signedDataSize,
	const void *				signature,
	PGPSize *					signatureSize)
{
	PGPRandomContext *			randomContext;
	PGPSize						lSignatureSize;
	PGPHashVTBL					dummyVTBL;
	PGPError					err	= kPGPError_NoErr;
	
	if( IsntNull( signatureSize ) )
	{
		PGPValidatePtr( signatureSize );
		*signatureSize = 0;
	}
	PGPValidatePrivateKey( ref );
	PGPValidatePtr( signedData );
	PGPValidateParam( signedDataSize != 0 );
	/* Raw mode supported only for PKCS sigs */
	if( ref->format != 	kPGPPublicKeyMessageFormat_PKCS1 ) {
		return kPGPError_BadParams;
	}
	PGPValidatePtr( signature );

	if( IsNull( ref->signkey ) )
		return kPGPError_FeatureNotAvailable;

	randomContext = pgpContextGetX9_17RandomContext( ref->context );

	/* Set up value for hash size used by signature function */
	pgpClearMemory( &dummyVTBL, sizeof(dummyVTBL) );
	dummyVTBL.hashsize = signedDataSize;

	/* Do the signature */
	err = pgpSecKeySign( ref->signkey, &dummyVTBL,
						 (const PGPByte *) signedData,
						 (PGPByte *) signature, &lSignatureSize,
						  randomContext, ref->format );

	if( IsntPGPError( err )  && IsntNull( signatureSize ) )
	{
		*signatureSize = lSignatureSize;
	}

	return( err );
}


/*____________________________________________________________________________
If we have an encrypt-only or sign only key, we return zeros for the
one(s) which we don't support.
____________________________________________________________________________*/
	PGPError
PGPGetPrivateKeyOperationSizes(
	PGPPrivateKeyContextRef	ref,
	PGPSize *				maxDecryptedBufferSize,
	PGPSize *				maxEncryptedBufferSize,
	PGPSize *				maxSignatureSize )
{
	PGPValidatePrivateKey( ref );
	if( IsntNull( maxDecryptedBufferSize ) )
		PGPValidatePtr( maxDecryptedBufferSize );
	if( IsntNull( maxEncryptedBufferSize ) )
		PGPValidatePtr( maxEncryptedBufferSize );
	if( IsntNull( maxSignatureSize ) )
		PGPValidatePtr( maxSignatureSize );
	
	if( IsntNull( maxEncryptedBufferSize ) )
	{
		if( IsntNull( ref->enckey ) ) {
			*maxEncryptedBufferSize = pgpSecKeyMaxesk( ref->enckey,
													   ref->format );
		} else {
			*maxEncryptedBufferSize = 0;
		}
	}

	if( IsntNull( maxDecryptedBufferSize ) )
	{
		if( IsntNull( ref->enckey ) )
			*maxDecryptedBufferSize = pgpSecKeyMaxdecrypted( ref->enckey,
															 ref->format );
		else
			*maxDecryptedBufferSize = 0;
	}

	if( IsntNull( maxSignatureSize ) )
	{
		if( IsntNull( ref->signkey ) )
			*maxSignatureSize = pgpSecKeyMaxsig( ref->signkey, ref->format );
		else
			*maxSignatureSize = 0;
	}
	return kPGPError_NoErr;
}



/*____________________________________________________________________________
Given the size of a prime modulus in bits, this returns an appropriate
size for an exponent in bits, such that the work factor to find a
discrete log modulo the modulus is approximately equal to half the
length of the exponent.  This makes the exponent an appropriate size
for a subgroup in a discrete log signature scheme.  For encryption
schemes, where decryption attacks can be stealthy and undetected, we
use 3/2 times the returned exponent size.
____________________________________________________________________________*/

	PGPError
PGPDiscreteLogExponentBits(
	PGPUInt32				modulusBits,
	PGPUInt32			   *exponentBits )
{
	PGPValidatePtr( exponentBits );

	*exponentBits = pgpDiscreteLogExponentBits( modulusBits );

	return kPGPError_NoErr;
}


#if PRAGMA_MARK_SUPPORTED
#pragma mark --- Internal Routines ---
#endif






/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError
sSetupPubkey( PGPPublicKeyContextRef ref )
{
	RingObject *		ringKey;
	RingSet const *		ringSet;
	PGPError			err = kPGPError_NoErr;

	if( IsPGPError( err = pgpGetKeyRingObject( ref->key, FALSE, &ringKey ) ) )
		goto error;
	if( IsPGPError( err = pgpGetKeyRingSet( ref->key, FALSE, &ringSet ) ) )
		goto error;
	
	ref->enckey = ringKeyPubKey( (RingSet *)ringSet, ringKey,
								 PGP_PKUSE_ENCRYPT );
	ref->signkey = ringKeyPubKey( (RingSet *)ringSet, ringKey,
								  PGP_PKUSE_SIGN );
	
	if( IsNull( ref->enckey )  && IsNull( ref->signkey ) ) {
		err = ringSetError( ringSet ) -> error;
		goto error;
	}

error:

	return err;
}

/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError
sSetupPrivkey( PGPPrivateKeyContextRef ref, char const *passphrase,
	PGPSize passphraseLength, PGPBoolean hashedPhrase )
{
	RingObject *		ringKey;
	RingSet const *		ringSet;
	PGPInt32			rslt;
	PGPEnv *			env;
	PGPError			err = kPGPError_NoErr;

	if( IsPGPError( err = pgpGetKeyRingObject( ref->key, FALSE, &ringKey ) ) )
		goto error;
	if( IsPGPError( err = pgpGetKeyRingSet( ref->key, FALSE, &ringSet ) ) )
		goto error;
	
	ref->enckey = ringSecSecKey( (RingSet *)ringSet, ringKey,
								 PGP_PKUSE_ENCRYPT );
	ref->signkey = ringSecSecKey( (RingSet *)ringSet, ringKey,
								  PGP_PKUSE_SIGN );
	
	if( IsNull( ref->enckey )  && IsNull( ref->signkey ) ) {
		err = ringSetError( ringSet ) -> error;
		goto error;
	}

	env = pgpContextGetEnvironment( ref->context );

	if( IsntNull( ref->enckey ) )
	{
		rslt = pgpSecKeyUnlock( ref->enckey, env, passphrase,
								passphraseLength, hashedPhrase );
		if( rslt != 1 )
		{
			pgpSecKeyDestroy( ref->enckey );
			if( IsntNull( ref->signkey ) )
			{
				pgpSecKeyDestroy( ref->signkey );
			}
			err = kPGPError_BadPassphrase;
			goto error;
		}
	}

	if( IsntNull( ref->signkey ) )
	{
		rslt = pgpSecKeyUnlock( ref->signkey, env, passphrase,
								passphraseLength, FALSE );
		if( rslt != 1 )
		{
			pgpSecKeyDestroy( ref->signkey );
			if( IsntNull( ref->enckey ) )
			{
				pgpSecKeyDestroy( ref->enckey );
			}
			err = kPGPError_BadPassphrase;
			goto error;
		}
	}

error:

	return err;
}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
