/*____________________________________________________________________________
	pgpSymmetricCipherPriv.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpSymmetricCipherPriv.h,v 1.2 1999/03/10 02:58:43 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpSymmetricCipherPriv_h	/* [ */
#define Included_pgpSymmetricCipherPriv_h

#include "pgpBase.h"
#include "pgpOpaqueStructs.h"
#include "pgpSymmetricCipher.h"


PGP_BEGIN_C_DECLARATIONS

#define	kPGPNumPossibleCipherAlgorithms		3

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
