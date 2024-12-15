/*
* pgpRngPars.h
*
* Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRngPars.h,v 1.5.2.1 1997/06/07 09:50:38 mhw Exp $
*/

#ifndef PGPRNGPARS_H
#define PGPRNGPARS_H

#include <stddef.h>	 /* For size_t */

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpKeySpec;

int ringKeyParse(byte const *buf, size_t len, byte *pkalg, byte keyID[8],
	word16 *keybits, word32 *tstamp, word16 *validity, int secretf);
size_t ringKeyParsePublicPrefix(byte const *buf, size_t len);
byte const *ringKeyParseModulus(byte const *buf, size_t len, unsigned *lenp);
byte const *ringKeyParseExponent(byte const *buf, size_t len, unsigned *lenp);

int ringKeyParseFingerprint16(byte const *kbuf,size_t klen, byte *fingerprint);
int ringSigParse(byte const *buf, size_t len, byte *pkalg, byte keyID[8],
	word32 *tstamp, word16 *validity, byte *type, byte *hashalg,
	size_t *extralen, byte *version);
byte const *ringSigParseExtra(byte const *buf, size_t len, unsigned *lenp);
byte const *ringSigParseInteger(byte const *buf, size_t len, unsigned *lenp);
byte const * ringSigFindSubpacket(byte const *buf, int subpacktype,
	unsigned nth, size_t *plen, int *pcritical, int *phashed,
	word32 *pcreation, unsigned *pmatches);

size_t ringKeyBufferLength(struct PgpKeySpec const *ks, byte pkalg);
int ringKeyToBuffer(byte *buf, struct PgpKeySpec const *ks, byte pkalg);
size_t ringSecBufferLength(struct PgpKeySpec const *ks, byte pkalg);
int ringSecToBuffer(byte *buf, struct PgpKeySpec const *ks, byte pkalg);

#ifdef __cplusplus
}
#endif

#endif /* PGPRNGPARS_H */
