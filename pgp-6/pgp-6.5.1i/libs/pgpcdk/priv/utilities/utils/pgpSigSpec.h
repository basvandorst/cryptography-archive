/*
 * pgpSigSpec.h -- Signature Specification; specify the attributes of a
 * signature or list of signatures
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpSigSpec.h,v 1.25 1998/08/20 17:32:26 hal Exp $
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
#define PGP_SIGTYPE_KEY_PROPERTY	0x1f
#define PGP_SIGTYPE_KEY_REVOKE	0x20
#define PGP_SIGTYPE_KEY_SUBKEY_REVOKE   0x28
#define PGP_SIGTYPE_KEY_UID_REVOKE	0x30
#define PGP_SIGTYPE_NOTARY		0x40
/*
 * A signature with this bit set is considered "extended" and allows us to
 * parse "extra" information > 5 bytes without fear of being bitten by the
 * PGP 2.X aliasing problem.
 */
#define PGP_SIGTYPEF_EXTENDED		0x80

/* Sig flags for access and modification of SigSpec structures */
/* Must keep low byte free for type byte */
#define kPGPSigFlags_Type			0xff
#define kPGPSigFlags_Present		0x100
#define kPGPSigFlags_Unhashed		0x200
#define kPGPSigFlags_Critical		0x400

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
int  pgpSigSpecSetSeckey (PGPSigSpec *spec, PGPSecKey *seckey);
int  pgpSigSpecSetHashtype (PGPSigSpec *spec, PGPByte hashtype);
int  pgpSigSpecSetSigtype (PGPSigSpec *spec, PGPByte sigtype);
int  pgpSigSpecSetTimestamp (PGPSigSpec *spec,
	PGPUInt32 timestamp);
int  pgpSigSpecSetVersion (PGPSigSpec *spec,
	PgpVersion version);

PGPUInt32 
pgpSigSpecSigExpiration (PGPSigSpec const *spec, PGPUInt32 *sigExpire);
PGPError
pgpSigSpecSetSigExpiration (PGPSigSpec *spec, PGPUInt32 flags,
							PGPUInt32 sigExpire);
PGPError
pgpSigSpecSetExportable (PGPSigSpec *spec, PGPUInt32 flags,
						 PGPBoolean exportable);
PGPUInt32
pgpSigSpecExportable (PGPSigSpec const *spec, PGPBoolean *exportable);
PGPError
pgpSigSpecSetRevocable (PGPSigSpec *spec, PGPUInt32 flags,
						PGPBoolean revocable);
PGPUInt32
pgpSigSpecRevocable (PGPSigSpec const *spec, PGPBoolean *revocable);
PGPError
pgpSigSpecSetPrimaryUserID (PGPSigSpec *spec, PGPUInt32 flags,
						PGPBoolean primaryUserID);
PGPUInt32
pgpSigSpecPrimaryUserID (PGPSigSpec const *spec, PGPBoolean *primaryUserID);
PGPError
pgpSigSpecSetTrustLevel (PGPSigSpec *spec, PGPUInt32 flags,
						 PGPByte trustLevel, PGPByte trustValue);
PGPUInt32
pgpSigSpecTrustLevel (PGPSigSpec const *spec, PGPByte *trustLevel,
					  PGPByte *trustValue);
PGPError
pgpSigSpecSetRegExp (PGPSigSpec *spec, PGPUInt32 flags, char const *regExp);
PGPUInt32
pgpSigSpecRegExp (PGPSigSpec const *spec, char **regExp);

/* Access functions for self-sig subpackets */
PGPUInt32 
pgpSigSpecKeyExpiration (PGPSigSpec const *spec, PGPUInt32 *keyExpire);
PGPError
pgpSigSpecSetKeyExpiration (PGPSigSpec *spec, PGPUInt32 flags,
							PGPUInt32 keyExpire);
PGPUInt32
pgpSigSpecPrefAlgs (PGPSigSpec const *spec, PGPByte **prefalgs,
					PGPSize *preflen);
PGPError
pgpSigSpecSetPrefAlgs (PGPSigSpec *spec, PGPUInt32 flags,
					   PGPByte const *algs, size_t len);
PGPUInt32
pgpSigSpecAdditionalRecipientRequest (PGPSigSpec const *spec,
									  PGPByte **arr, size_t *arrlen,
									  int nth);
PGPError
pgpSigSpecSetAdditionalRecipientRequest  (PGPSigSpec *spec, PGPUInt32 flags,
										  PGPByte const *krinfo, PGPSize len);
PGPUInt32
pgpSigSpecRevocationKey (PGPSigSpec const *spec, PGPByte **rev,
						 PGPSize *revlen, int nth);
PGPError
pgpSigSpecSetRevocationKey  (PGPSigSpec *spec, PGPUInt32 flags,
							 PGPByte const *krinfo, size_t len);
PGPUInt32
pgpSigSpecPacket (PGPSigSpec const *spec, PGPByte **extradata, PGPSize *len,
				  int nth);
PGPError
pgpSigSpecSetPacket (PGPSigSpec *spec, PGPUInt32 flags,
							 PGPByte const *extradata, size_t len);


PGPUInt32
pgpSigSpecRemove (PGPSigSpec *spec, PGPUInt32 type);


PGP_END_C_DECLARATIONS

#endif /* Included_pgpSigSpec_h */
