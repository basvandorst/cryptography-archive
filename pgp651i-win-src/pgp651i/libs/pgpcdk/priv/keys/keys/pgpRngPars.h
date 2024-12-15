/*
 * $Id: pgpRngPars.h,v 1.25 1999/02/20 02:51:42 hal Exp $
 */

#ifndef Included_pgpRngPars_h
#define Included_pgpRngPars_h

#include <stddef.h>	/* For size_t */

#include "pgpContext.h"
#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

struct PGPKeySpec;

int ringKeyParse(PGPContextRef context, PGPByte const *buf, size_t len,
	PGPByte *pkalg, PGPByte keyID[8], PGPByte fp20n[20], PGPUInt16 *keybits,
	PGPUInt32 *tstamp, PGPUInt16 *validity, PGPByte *v3, int secretf);
size_t ringKeyParsePublicPrefix(PGPByte const *buf, size_t len);
PGPByte const *ringKeyParseNumericData(PGPByte const *buf, size_t len,
	unsigned *lenp);
PGPByte const *ringKeyParseModulus(PGPByte const *buf, size_t len,
	unsigned *lenp);
PGPByte const *ringKeyParseExponent(PGPByte const *buf, size_t len,
	unsigned *lenp);

int ringKeyParseFingerprint16(PGPContextRef context, PGPByte const *kbuf,
	size_t klen, PGPByte *fingerprint);
int ringSigParse(PGPByte const *buf, size_t len, PGPByte *pkalg,
	PGPByte keyID[8], PGPUInt32 *tstamp, PGPUInt32 *validity, PGPByte *type,
	PGPByte *hashalg, size_t *extralen, PGPByte *version,
	PGPBoolean *exportable, PGPBoolean *revocable, PGPByte *trustLevel,
	PGPByte *trustValue, PGPBoolean *hasRegExp, PGPBoolean *isX509,
	PGPBoolean *primaryUID);
PGPByte const *ringSigParseExtra(PGPByte const *buf, size_t len,
	unsigned *lenp);
PGPByte const *ringSigParseInteger(PGPByte const *buf, size_t len,
	unsigned *lenp);
PGPByte const * ringSigFindSubpacket(PGPByte *buf, int subpacktype,
	unsigned nth, PGPSize *plen, int *pcritical, int *phashed,
	PGPUInt32 *pcreation, unsigned *pmatches);
PGPByte const *ringSigFindNAISubSubpacket(PGPByte *buf, int subpacktype,
	unsigned nth, PGPSize *plen, int *pcritical, int *phashed,
	PGPUInt32 *pcreation, unsigned *pmatches);
PGPError ringSigSubpacketsSpec(PGPSigSpec *spec, PGPByte const *buf);
PGPByte const *ringAttrSubpacket(PGPByte *buf, PGPSize len, unsigned nth,
	PGPUInt32 *subpacktype, PGPSize *plen);
PGPUInt32 ringAttrCountSubpackets(PGPByte const *buf, PGPSize len);

size_t ringKeyBufferLength(PGPKeySpec const *ks, PGPByte pkalg);
int ringKeyToBuffer(PGPByte *buf, PGPKeySpec const *ks, PGPByte pkalg);
size_t ringSecBufferLength(PGPKeySpec const *ks, PGPByte pkalg);
int ringSecToBuffer(PGPByte *buf, PGPKeySpec const *ks, PGPByte pkalg);

PGPError ringCRLParse(RingPool *pool, PGPByte const *buf, PGPSize len,
	PGPByte *version, PGPByte *type, PGPUInt32 *tstamp, PGPUInt32 *tstampnext,
	PGPByte const **dpoint, PGPSize *dpointlen);
PGPByte const * ringCRLFindDPoint( PGPByte const *buf, PGPSize buflen,
	PGPSize *len );
PGPByte const * ringCRLFindCRL( PGPByte const *buf, PGPSize buflen,
	PGPSize *len );

PGP_END_C_DECLARATIONS

#endif /* Included_pgpRngPars_h */