/*
 * pgpKeyMisc.h -- PKCS packing and unpacking
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpKeyMisc.h,v 1.3.2.1 1997/06/07 09:51:26 mhw Exp $
 */
#ifndef PGPKEYMISC_H
#define PGPKEYMISC_H

#include "pgpUsuals.h"		/* for byte */

#ifdef __cplusplus
extern "C" {
#endif

struct BigNum;
struct PgpEnv;
struct PgpRandomContext;
struct PgpCfbContext;

/* For future expansion to ignore certain bits of alg */
#define ALGMASK(alg) (alg)

unsigned pgpDiscreteLogExponentBits(unsigned modbits);

/* Block types from PKCS */
#define PKCS_PAD_ZERO		0
#define PKCS_PAD_SIGNED		1
#define PKCS_PAD_ENCRYPTED	2

int pgpPKCSPack(struct BigNum *bn, byte const *in, unsigned len, byte padtype,
	unsigned bytes, struct PgpRandomContext const *rc);
int pgpPKCSUnpack(byte *buf, unsigned len, struct BigNum *bn, byte padtype,
	unsigned bytes);

int pgpBnGetPlain(struct BigNum *bn, byte const *buf, unsigned size);
int pgpBnGet(struct BigNum *bn, byte const *buf, unsigned size,
	struct PgpCfbContext *cfb, unsigned *checksump, int old);
#define pgpBnGetNew(bn,buf,size,cfb,cs) pgpBnGet(bn,buf,size,cfb,cs,0)
#define pgpBnGetOld(bn,buf,size,cfb,cs) pgpBnGet(bn,buf,size,cfb,cs,1)

unsigned pgpChecksumGet(byte const *buf, struct PgpCfbContext *cfb, int old);
#define pgpChecksumGetNew(buf,cfb) pgpChecksumGet(buf,cfb,0)
#define pgpChecksumGetOld(buf,cfb) pgpChecksumGet(buf,cfb,1)

unsigned pgpBnPutPlain(struct BigNum const *bn, byte *buf);
unsigned pgpBnPut(struct BigNum const *bn, byte *buf,
	struct PgpCfbContext *cfb, unsigned *checksump, int old);
#define pgpBnPutNew(bn,buf,cfb,cs) pgpBnPut(bn,buf,cfb,cs,0)
#define pgpBnPutOld(bn,buf,cfb,cs) pgpBnPut(bn,buf,cfb,cs,1)

void
pgpChecksumPut(unsigned csum, byte *buf, struct PgpCfbContext *cfb, int old);
#define pgpChecksumPutNew(csum,buf,cfb) pgpChecksumPut(csum,buf,cfb,0)
#define pgpChecksumPutOld(csum,buf,cfb) pgpChecksumPut(csum,buf,cfb,1)

int pgpBnGenRand(struct BigNum *bn, struct PgpRandomContext const *rc,
	     unsigned bits, byte high, byte low, unsigned effective);
int pgpBnParse(byte const *buf, unsigned size, int n, ...);

int
pgpCipherSetup(byte const *buf, unsigned len, char const *phrase, size_t plen,
	struct PgpEnv const *env, struct PgpCfbContext **cfbp);

#ifdef __cplusplus
}
#endif

#endif /* PGPKEYMISC_H */
