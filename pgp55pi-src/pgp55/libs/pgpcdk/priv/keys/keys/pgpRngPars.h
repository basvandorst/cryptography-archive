/*
 * $Id: pgpRngPars.h,v 1.12 1997/10/15 00:52:49 hal Exp $
 */

#ifndef Included_pgpRngPars_h
#define Included_pgpRngPars_h

#include <stddef.h>	/* For size_t */

#include "pgpContext.h"
#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

struct PGPKeySpec;

int ringKeyParse(PGPContextRef context, PGPByte const *buf, size_t len,
	PGPByte *pkalg, PGPByte keyID[8], PGPUInt16 *keybits, PGPUInt32 *tstamp,
	PGPUInt16 *validity, int secretf);
size_t ringKeyParsePublicPrefix(PGPByte const *buf, size_t len);
PGPByte const *ringKeyParseModulus(PGPByte const *buf, size_t len,
	unsigned *lenp);
PGPByte const *ringKeyParseExponent(PGPByte const *buf, size_t len,
	unsigned *lenp);

int ringKeyParseFingerprint16(PGPContextRef context, PGPByte const *kbuf,
	size_t klen, PGPByte *fingerprint);
int ringSigParse(PGPByte const *buf, size_t len, PGPByte *pkalg,
	PGPByte keyID[8], PGPUInt32 *tstamp, PGPUInt16 *validity, PGPByte *type,
	PGPByte *hashalg, size_t *extralen, PGPByte *version,
	PGPBoolean *exportable, PGPBoolean *revokable, PGPByte *trustLevel,
	PGPByte *trustValue, PGPBoolean *hasRegExp);
PGPByte const *ringSigParseExtra(PGPByte const *buf, size_t len,
	unsigned *lenp);
PGPByte const *ringSigParseInteger(PGPByte const *buf, size_t len,
	unsigned *lenp);
PGPByte const * ringSigFindSubpacket(PGPByte const *buf, int subpacktype,
	unsigned nth, PGPSize *plen, int *pcritical, int *phashed,
	PGPUInt32 *pcreation, unsigned *pmatches);

size_t ringKeyBufferLength(PGPKeySpec const *ks, PGPByte pkalg);
int ringKeyToBuffer(PGPByte *buf, PGPKeySpec const *ks, PGPByte pkalg);
size_t ringSecBufferLength(PGPKeySpec const *ks, PGPByte pkalg);
int ringSecToBuffer(PGPByte *buf, PGPKeySpec const *ks, PGPByte pkalg);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpRngPars_h */
