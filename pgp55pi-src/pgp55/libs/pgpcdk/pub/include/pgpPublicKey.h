/*____________________________________________________________________________
	pgpPublicKey.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpPublicKey.h,v 1.4 1997/10/23 00:12:26 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpPublicKey_h	/* [ */
#define Included_pgpPublicKey_h

#include "pgpPubTypes.h"



/*____________________________________________________________________________
	Encryption/Signature Message Formats
____________________________________________________________________________*/

enum PGPPublicKeyMessageFormat_
{
	kPGPPublicKeyMessageFormat_PGP		= 1,
	kPGPPublicKeyMessageFormat_PKCS1	= 2,

	PGP_ENUM_FORCE( PGPPublicKeyMessageFormat_ )
};
PGPENUM_TYPEDEF( PGPPublicKeyMessageFormat_, PGPPublicKeyMessageFormat );



PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



/*____________________________________________________________________________
	Public-key operations
____________________________________________________________________________*/



/*____________________________________________________________________________
	Return a context for public-key operations based on the specified key.
	The specified message format is used for all operations with this
	context.
____________________________________________________________________________*/
PGPError 	PGPNewPublicKeyContext( PGPKeyRef publicKeyRef,
					PGPPublicKeyMessageFormat messageFormat,
					PGPPublicKeyContextRef *outRef );


/*____________________________________________________________________________
	Dispose of a public-key context.
____________________________________________________________________________*/
PGPError 	PGPFreePublicKeyContext( PGPPublicKeyContextRef ref );


/*____________________________________________________________________________
	Determine maximum sizes for inputs and outputs.
____________________________________________________________________________*/
PGPError	PGPGetPublicKeyOperationSizes( PGPPublicKeyContextRef ref,
					PGPSize *maxDecryptedBufferSize,
					PGPSize *maxEncryptedBufferSize,
					PGPSize *maxSignatureSize );


/*____________________________________________________________________________
	Encrypt one block of data, using PKCS-1 padding.  Output buffer must
	be of size maxEncryptedBufferSize from PGPGetPublicKeyEncryptionSize.
	outSize is a return parameter.  For some formatting modes the actual
	output size may be less than the maximum possible.
____________________________________________________________________________*/
PGPError	PGPPublicKeyEncrypt( PGPPublicKeyContextRef ref,
					void const *in, PGPSize inSize, void *out,
					PGPSize *outSize );


/*____________________________________________________________________________
	Verify a signature on a message hash.  Returns kPGPError_NoErr on
	correct verification, else an error code.  The message hash is
	finalized by this call (and should not have been finalized prior
	to the call).
____________________________________________________________________________*/
	
PGPError	PGPPublicKeyVerifySignature( PGPPublicKeyContextRef ref,
					PGPHashContextRef hashContext,
					void const *signature, PGPSize signatureSize );




/*____________________________________________________________________________
	Private-key operations
____________________________________________________________________________*/



/*____________________________________________________________________________
	Return a context for private-key operations based on the specified
	key (which must have a private part).  The specified message
	format is used for all operations with this context.  Unlocks key
	data using passphrase.
____________________________________________________________________________*/
	
	
PGPError 	PGPNewPrivateKeyContext( PGPKeyRef privateKeyRef,
					PGPPublicKeyMessageFormat messageFormat,
					char const *passphrase,
					PGPPrivateKeyContextRef *outRef );


/*____________________________________________________________________________
	Dispose of a private-key context.  All sensitive data is wiped before
	being deleted.
____________________________________________________________________________*/
PGPError 	PGPFreePrivateKeyContext( PGPPrivateKeyContextRef ref );


/*____________________________________________________________________________
	Determine maximum sizes for inputs and outputs.
____________________________________________________________________________*/
PGPError	PGPGetPrivateKeyOperationSizes( PGPPrivateKeyContextRef ref,
					PGPSize *maxDecryptedBufferSize,
					PGPSize *maxEncryptedBufferSize,
					PGPSize *maxSignatureSize);


/*____________________________________________________________________________
	Decrypt one block of data.  Output buffer must be of size at least
	maxDecryptedBufferSize from PGPGetPrivateKeyDecryptionSize.
	outSize is a return parameter.  For some formatting modes the actual
	output size may be less than the maximum possible.
____________________________________________________________________________*/
PGPError	PGPPrivateKeyDecrypt( PGPPrivateKeyContextRef ref,
					void const *in, PGPSize inSize, void *out,
					PGPSize *outSize );


/*____________________________________________________________________________
	Sign a message hash.  Output signature buffer must be of size at
	least maxSignatureSize from PGPGetPrivateKeyDecryptionSize.
	signatureSize is a return parameter.  For some formatting modes
	the actual signature size may be less than the maximum possible.
	The message hash is finalized by this call (and should not have been
	finalized prior to the call).
____________________________________________________________________________*/
	
PGPError	PGPPrivateKeySign( PGPPrivateKeyContextRef ref,
					PGPHashContextRef hashContext,
					void *signature, PGPSize *signatureSize );



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpPublicKey_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
