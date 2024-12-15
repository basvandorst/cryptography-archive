/*
 * pgpESK.h -- Encrypted Session Key handler.
 *
 * $Id: pgpESK.h,v 1.11 1997/10/08 01:48:25 lloyd Exp $
 */

#ifndef Included_pgpESK_h
#define Included_pgpESK_h

#include "pgpContext.h"

PGP_BEGIN_C_DECLARATIONS

/* Return values from eskType */
#define PGP_ESKTYPE_PASSPHRASE 0
#define PGP_ESKTYPE_PUBKEY 1

#include "pgpOpaqueStructs.h"

/* Some access functions. */
int  pgpEskType (PGPESK const *esk);
int  pgpEskPKAlg (PGPESK const *esk);


/* caller must free the key ID */
PGPError	pgpGetEskKeyID( PGPESK const *esk, PGPKeyID *outRef);
					
/* How big might the output key possibly be? */
size_t  pgpEskMaxKeySize (PGPESK const *esk);
/*
 * The "key" is a passphrase.  Returns the length of the key, which
 * is <algorithm identifier><key bits>
 */
int  pgpEskConvDecrypt (PGPESK const *esk,
	PGPEnv const *env, char const *pass, size_t plen, PGPByte *buf);

/* Do public-key decryption */
int  pgpEskPKdecrypt (PGPESK const *esk,
	PGPEnv const *env, PGPSecKey *sec, PGPByte *buf);

PGPESK  *pgpEskNext (PGPESK const *esklist);

int  pgpEskAdd ( PGPContextRef	cdkContext,
		PGPESK **esklist, int type, PGPByte const *buf,
		size_t len);
void  pgpEskFreeList (PGPESK *esklist);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpESK_h */
