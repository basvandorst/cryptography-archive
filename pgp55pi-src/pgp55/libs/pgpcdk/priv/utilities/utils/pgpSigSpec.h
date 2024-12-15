/*
 * pgpSigSpec.h -- Signature Specification; specify the attributes of a
 * signature or list of signatures
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpSigSpec.h,v 1.19 1997/10/17 06:15:17 hal Exp $
 */

#ifndef Included_pgpSigSpec_h
#define Included_pgpSigSpec_h

/*
 * Signature type values
 */
#define PGP_SIGTYPE_BINARY		0
#define PGP_SIGTYPE_TEXT		1
#define PGP_SIGTYPE_KEY_GENERIC		0x10
#define PGP_SIGTYPE_KEY_PERSONA		0x11
#define PGP_SIGTYPE_KEY_CASUAL		0x12
#define PGP_SIGTYPE_KEY_POSITIVE	0x13
#define PGP_SIGTYPE_KEY_SUBKEY		0x18
#define PGP_SIGTYPE_KEY_COMPROMISE	0x20
#define PGP_SIGTYPE_KEY_SUBKEY_REVOKE   0x28
#define PGP_SIGTYPE_KEY_UID_REVOKE	0x30
#define PGP_SIGTYPE_NOTARY		0x40
/*
 * A signature with this bit set is considered "extended" and allows us to
 * parse "extra" information > 5 bytes without fear of being bitten by the
 * PGP 2.X aliasing problem.
 */
#define PGP_SIGTYPEF_EXTENDED		0x80

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"

/*
 * Create and Destroy a signature specification.  Required parameters
 * are passed into this function.  Some parameters are assumed from
 * the PGPEnv.
 */
PGPSigSpec  *pgpSigSpecCreate (PGPEnv const *env,
	PGPSecKey *seckey, PGPByte sigtype);
PGPSigSpec  *pgpSigSpecCopy (PGPSigSpec const *spec);
void  pgpSigSpecDestroy (PGPSigSpec *spec);

/* Deal with lists of signature specifications */
int  pgpSigSpecAdd (PGPSigSpec **list,
	PGPSigSpec *spec);
PGPSigSpec  *pgpSigSpecNext (PGPSigSpec const *list);

/* Access functions for sigspec parameters */
PgpVersion  pgpSigSpecVersion (PGPSigSpec const *spec);
PGPSecKey  *pgpSigSpecSeckey (PGPSigSpec const *spec);
PGPHashVTBL const  *pgpSigSpecHash (
	PGPSigSpec const *spec);
PGPByte  pgpSigSpecHashtype (PGPSigSpec const *spec);
PGPByte const  *pgpSigSpecExtra (PGPSigSpec const *spec,
     size_t *extralen);

/* Modification functions for sigspec parameters */
int  pgpSigSpecSetHashtype (PGPSigSpec *spec, PGPByte hashtype);
int  pgpSigSpecSetSigtype (PGPSigSpec *spec, PGPByte sigtype);
int  pgpSigSpecSetTimestamp (PGPSigSpec *spec,
	PGPUInt32 timestamp);
int  pgpSigSpecSetVersion (PGPSigSpec *spec,
	PgpVersion version);

PGPUInt32 
pgpSigSpecSigExpiration (PGPSigSpec const *spec);
int 
pgpSigSpecSetSigExpiration (PGPSigSpec *spec, PGPUInt32 sigExpire);
void
pgpSigSpecSetExportable (PGPSigSpec *spec, PGPBoolean exportable);
PGPBoolean
pgpSigSpecExportable (PGPSigSpec const *spec);
void
pgpSigSpecSetRevokable (PGPSigSpec *spec, PGPBoolean revokable);
PGPBoolean
pgpSigSpecRevokable (PGPSigSpec const *spec);
void
pgpSigSpecSetTrustLevel (PGPSigSpec *spec, PGPByte trustLevel);
PGPByte
pgpSigSpecTrustLevel (PGPSigSpec const *spec);
void
pgpSigSpecSetTrustValue (PGPSigSpec *spec, PGPByte trustValue);
PGPByte
pgpSigSpecTrustValue (PGPSigSpec const *spec);
void
pgpSigSpecSetRegExp (PGPSigSpec *spec, char const *regExp);
const char *
pgpSigSpecRegExp (PGPSigSpec const *spec);

/* Access functions for self-sig subpackets */
PGPUInt32 
pgpSigSpecKeyExpiration (PGPSigSpec const *spec);
int 
pgpSigSpecSetKeyExpiration (PGPSigSpec *spec, PGPUInt32 keyExpire);
PGPByte const  *
pgpSigSpecPrefAlgs (PGPSigSpec const *spec, size_t *preflen);
int 
pgpSigSpecSetPrefAlgs (PGPSigSpec *spec, PGPByte const *algs, size_t len);
PGPByte const  *
pgpSigSpecAdditionalRecipientRequest (PGPSigSpec const *spec, size_t *preflen,
	int nth);
int 
pgpSigSpecSetAdditionalRecipientRequest  (PGPSigSpec *spec, PGPByte *krinfo,
	size_t len, int nth);
PGPByte const *
pgpSigSpecRevocationKey (PGPSigSpec const *spec, size_t *preflen,
	int nth);
int 
pgpSigSpecSetRevocationKey  (PGPSigSpec *spec, PGPByte *krinfo, size_t len,
	int nth);


PGP_END_C_DECLARATIONS

#endif /* Included_pgpSigSpec_h */
