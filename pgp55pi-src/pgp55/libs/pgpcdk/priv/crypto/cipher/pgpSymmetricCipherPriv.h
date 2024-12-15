/*____________________________________________________________________________
	pgpSymmetricCipherPriv.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpSymmetricCipherPriv.h,v 1.5 1997/10/14 01:48:19 heller Exp $
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

PGPContextRef	pgpGetSymmetricCipherContext( PGPSymmetricCipherContextRef ref);

/* Get the default ciphers for operations */
PGPCipherVTBL const   *	pgpCipherDefaultKey( PGPEnv const *env);
PGPCipherVTBL const  *	pgpCipherDefaultKeyV2( PGPEnv const *env);

/* Static buffer size for keys */
#define PGP_CIPHER_MAXKEYSIZE	32	/* 256 bits */


PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpSymmetricCipherPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
