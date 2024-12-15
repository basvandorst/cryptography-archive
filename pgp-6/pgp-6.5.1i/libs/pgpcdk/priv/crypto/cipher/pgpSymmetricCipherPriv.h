/*____________________________________________________________________________
	pgpSymmetricCipherPriv.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpSymmetricCipherPriv.h,v 1.15 1999/05/15 06:14:48 hal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpSymmetricCipherPriv_h	/* [ */
#define Included_pgpSymmetricCipherPriv_h

#include "pgpBase.h"
#include "pgpOpaqueStructs.h"
#include "pgpSymmetricCipher.h"


PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	a "virtual function table" for a cipher class.
	
	Use of this structure is discouraged. Use formal API where possible.
____________________________________________________________________________*/

struct PGPCipherVTBL
{
	char const *		name;
	PGPCipherAlgorithm	algorithm;
	PGPSize				blocksize;
	PGPSize				keysize;
	PGPSize				context_size;
	PGPSize				context_align;

	void			(*initKey)(void *priv, void const *key);
	void			(*encrypt)(void *priv, void const *in, void *out);
	void			(*decrypt)(void *priv, void const *in, void *out);
	void			(*wash)(void *priv, void const *buf, PGPSize len);
};

PGPBoolean	pgpSymmetricCipherIsValid( const PGPSymmetricCipherContext * ref);


PGPCipherVTBL const  *	pgpCipherGetVTBL (PGPCipherAlgorithm alg);

PGPUInt32	pgpCountSymmetricCiphers( void );

PGPMemoryMgrRef	pgpGetSymmetricCipherMemoryMgr(
							PGPSymmetricCipherContextRef ref);

/* Get the default ciphers for operations */
PGPCipherVTBL const   *	pgpCipherDefaultKey( PGPEnv const *env);
PGPCipherVTBL const  *	pgpCipherDefaultKeyV3( PGPEnv const *env);

/* Static buffer size for keys */
#define PGP_CIPHER_MAXKEYSIZE	32	/* 256 bits */
#define PGP_CIPHER_MAXBLOCKSIZE	16	/* 128 bits */

/* Internal only ciphers, for decrypting PKCS-12 files */
#define kPGPCipherAlgorithm_RC2_40		100
#define kPGPCipherAlgorithm_RC2_128		101

PGPError	pgpNewSymmetricCipherContextInternal(PGPMemoryMgrRef memoryMgr,
					PGPCipherAlgorithm algorithm, PGPSize keySize,
					PGPSymmetricCipherContextRef *outRef );
PGPError 	pgpSymmetricCipherDecryptInternal(PGPSymmetricCipherContextRef ref,
					const void *in, void *out);
PGPError 	pgpSymmetricCipherEncryptInternal(PGPSymmetricCipherContextRef ref,
					const void *in, void *out);
	
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpSymmetricCipherPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
