/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	This is a PRIVATE header file, for use only within the PGP Library.
	You should not be using these functions in an application.

	$Id: pgpKeyMisc.h,v 1.13.12.1 1998/11/12 03:22:11 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpKeyMisc_h
#define Included_pgpKeyMisc_h

#include "pgpUsuals.h"		/* for PGPByte */
#include "pgpPublicKey.h"	/* for PGPPublicKeyMessageFormat */

PGP_BEGIN_C_DECLARATIONS


/* For future expansion to ignore certain bits of alg */
#define ALGMASK(alg) (alg)

/* Block types from PKCS */
#define PKCS_PAD_ZERO		0
#define PKCS_PAD_SIGNED		1
#define PKCS_PAD_ENCRYPTED	2

/* Commonly used tag values from X509 */
#define X509_TAG_INTEGER		2
#define X509_TAG_BITSTRING		3
#define X509_TAG_SEQUENCE		16
#define X509_TAG_CONSTRUCTED	0x20


unsigned pgpDiscreteLogExponentBits(unsigned modbits);
unsigned pgpDiscreteLogQBits(unsigned modbits);

int pgpPKCSPack(BigNum *bn, PGPByte const *in, unsigned len,
	PGPByte padtype,
	unsigned bytes, PGPRandomContext const *rc);
int pgpPKCSUnpack(PGPByte *buf, unsigned len, BigNum *bn, PGPByte padtype,
	unsigned bytes);

PGPByte pgpBnX509TagLen(PGPByte const **buf, PGPUInt32 *length);
PGPUInt32 pgpBnX509LenLen(PGPUInt32 length);
int pgpBnGetPlain(BigNum *bn, PGPByte const *buf, unsigned size);
int pgpBnGetFormatted(BigNum *bn, PGPByte const *buf, unsigned size,
	unsigned modbytes, PGPPublicKeyMessageFormat format);
int pgpBnGet(BigNum *bn, PGPByte const *buf, unsigned size,
	PGPCFBContext *cfb, unsigned *checksump, int old);
#define pgpBnGetNew(bn,buf,size,cfb,cs) pgpBnGet(bn,buf,size,cfb,cs,0)
#define pgpBnGetOld(bn,buf,size,cfb,cs) pgpBnGet(bn,buf,size,cfb,cs,1)

unsigned pgpChecksumGet(PGPByte const *buf,
	PGPCFBContext *cfb, int old);
#define pgpChecksumGetNew(buf,cfb) pgpChecksumGet(buf,cfb,0)
#define pgpChecksumGetOld(buf,cfb) pgpChecksumGet(buf,cfb,1)

unsigned pgpBnPutPlain(BigNum const *bn, PGPByte *buf);
unsigned pgpBnPutFormatted(BigNum const *bn, PGPByte *buf,
	unsigned modbytes, PGPPublicKeyMessageFormat format);
unsigned pgpBnPut(BigNum const *bn, PGPByte *buf,
	PGPCFBContext *cfb, unsigned *checksump, int old);
#define pgpBnPutNew(bn,buf,cfb,cs) pgpBnPut(bn,buf,cfb,cs,0)
#define pgpBnPutOld(bn,buf,cfb,cs) pgpBnPut(bn,buf,cfb,cs,1)

void	pgpChecksumPut(unsigned csum,
			PGPByte *buf, PGPCFBContext *cfb, int old);
#define pgpChecksumPutNew(csum,buf,cfb) pgpChecksumPut(csum,buf,cfb,0)
#define pgpChecksumPutOld(csum,buf,cfb) pgpChecksumPut(csum,buf,cfb,1)

int pgpBnGenRand(BigNum *bn, PGPRandomContext const *rc,
	     unsigned bits, PGPByte high, PGPByte low, unsigned effective);
int pgpBnParse(PGPByte const *buf, unsigned size, int n, ...);

int		pgpCipherSetup(PGPByte const *buf, unsigned len,
			char const *phrase, size_t plen, PGPBoolean hashedPhrase,
			PGPEnv const *env, PGPCFBContext **cfbp);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpKeyMisc_h */
