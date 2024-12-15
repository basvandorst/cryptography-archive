/*
 * pgpSigSpec.h -- Signature Specification; specify the attributes of a
 * signature or list of signatures
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpSigSpec.h,v 1.8.2.1 1997/06/07 09:51:42 mhw Exp $
 */

#ifndef PGPSIGSPEC_H
#define PGPSIGSPEC_H

/*
 * Signature type values
 */
#define PGP_SIGTYPE_BINARY				0
#define PGP_SIGTYPE_TEXT				1
#define PGP_SIGTYPE_KEY_GENERIC				0x10
#define PGP_SIGTYPE_KEY_PERSONA				0x11
#define PGP_SIGTYPE_KEY_CASUAL				0x12
#define PGP_SIGTYPE_KEY_POSITIVE			0x13
#define PGP_SIGTYPE_KEY_SUBKEY				0x18
#define PGP_SIGTYPE_KEY_COMPROMISE			0x20
#define PGP_SIGTYPE_KEY_SUBKEY_REVOKE 0x28
#define PGP_SIGTYPE_KEY_UID_REVOKE	0x30
#define PGP_SIGTYPE_NOTARY		0x40
/*
 * A signature with this bit set is considered "extended" and allows us to
 * parse "extra" information > 5 bytes without fear of being bitten by the
 * PGP 2.X aliasing problem.
 */
#define PGP_SIGTYPEF_EXTENDED				0x80

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpSigSpec;
#ifndef TYPE_PGPSIGSPEC
#define TYPE_PGPSIGSPEC 1
typedef struct PgpSigSpec PgpSigSpec;
#endif

struct PgpSecKey;
#ifndef TYPE_PGPSECKEY
#define TYPE_PGPSECKEY 1
typedef struct PgpSecKey PgpSecKey;
#endif

/*
 * Create and Destroy a signature specification. Required parameters
 * are passed into this function. Some parameters are assumed from
 * the PgpEnv.
 */
struct PgpSigSpec PGPExport *pgpSigSpecCreate (struct PgpEnv const *env,
	struct PgpSecKey *seckey, byte sigtype);
struct PgpSigSpec PGPExport *pgpSigSpecCopy (struct PgpSigSpec const *spec);
void PGPExport pgpSigSpecDestroy (struct PgpSigSpec *spec);

/* Deal with lists of signature specifications */
int PGPExport pgpSigSpecAdd (struct PgpSigSpec **list,
	struct PgpSigSpec *spec);
struct PgpSigSpec PGPExport *pgpSigSpecNext (struct PgpSigSpec const *list);

/* Access functions for sigspec parameters */
PgpVersion PGPExport pgpSigSpecVersion (struct PgpSigSpec const *spec);
struct PgpSecKey PGPExport *pgpSigSpecSeckey (struct PgpSigSpec const *spec);
struct PgpHash const PGPExport *pgpSigSpecHash (
	struct PgpSigSpec const *spec);
byte PGPExport pgpSigSpecHashtype (struct PgpSigSpec const *spec);
byte const PGPExport *pgpSigSpecExtra (struct PgpSigSpec const *spec,
unsigned *extralen);

/* Modification functions for sigspec parameters */
int PGPExport pgpSigSpecSetHashtype (struct PgpSigSpec *spec, byte hashtype);
int PGPExport pgpSigSpecSetSigtype (struct PgpSigSpec *spec, byte sigtype);
int PGPExport pgpSigSpecSetTimestamp (struct PgpSigSpec *spec,
	word32 timestamp);
int PGPExport pgpSigSpecSetVersion (struct PgpSigSpec *spec,
	PgpVersion version);

/* Access functions for self-sig subpackets */
word32 PGPExport
pgpSigSpecKeyExpiration (struct PgpSigSpec const *spec);
int PGPExport
pgpSigSpecSetKeyExpiration (struct PgpSigSpec *spec, word32 keyExpire);
byte const PGPExport *
pgpSigSpecPrefAlgs (struct PgpSigSpec const *spec, unsigned *preflen);
int PGPExport
pgpSigSpecSetPrefAlgs (struct PgpSigSpec *spec, byte const *algs, size_t len);
byte const PGPExport *
pgpSigSpecRecoveryKey (struct PgpSigSpec const *spec, unsigned *preflen,
	int nth);
int PGPExport
pgpSigSpecSetRecoveryKey (struct PgpSigSpec *spec, byte *krinfo, size_t len,
	int nth);


#ifdef __cplusplus
}
#endif

#endif /* PGPSIGSPEC_H */
