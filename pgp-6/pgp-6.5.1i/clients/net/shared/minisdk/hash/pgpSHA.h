/*
 * pgpSHA.h -- NIST Secure Hhash Algorithm
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpSHA.h,v 1.1 1998/08/18 22:59:31 dgal Exp $
 */

#ifndef Included_pgpSHA_h
#define Included_pgpSHA_h

#include "pgpHashPriv.h"
#include "pgpDebug.h"

#define SHA_BLOCKBYTES	64
#define SHA_BLOCKWORDS	16

#define SHA_HASHBYTES	20
#define SHA_HASHWORDS	5

typedef struct SHAContext {
	PGPUInt32 key[SHA_BLOCKWORDS];
	PGPUInt32 iv[SHA_HASHWORDS];
#if HAVE64
	word64 bytes;
#else
	PGPUInt32 bytesHi, bytesLo;
#endif
	DEBUG_STRUCT_CONSTRUCTOR( SHAContext )
} SHAContext;



PGP_BEGIN_C_DECLARATIONS

extern PGPHashVTBL const HashSHA;
extern PGPByte const SHADERprefix[15];

void	pgpSHATransform(PGPUInt32 *block, PGPUInt32 *key);

PGP_END_C_DECLARATIONS

#endif /* !Included_pgpSHA_h */
