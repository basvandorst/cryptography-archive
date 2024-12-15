/*
* pgpStr2Key.h -- A PGP String to Key Conversion System
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpStr2Key.h,v 1.4.2.1 1997/06/07 09:50:13 mhw Exp $
*/
#ifndef PGPSTR2KEY_H
#define PGPSTR2KEY_H

#include "pgpUsuals.h"	/* For byte */

#ifdef __cplusplus
extern "C" {
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpRandomContext;
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

struct PgpHash;
#ifndef TYPE_PGPHASH
#define TYPE_PGPHASH 1
typedef struct PgpHash PgpHash;
#endif

struct PgpStringToKey {
			void *priv;
			byte const *encoding;	/* For transmission */
			unsigned encodelen;
			void (*destroy) (struct PgpStringToKey *s2k);
			int (*stringToKey) (struct PgpStringToKey const *s2k, char const *str,
				 	size_t slen, byte *key, size_t klen);
	};
#ifndef TYPE_PGPSTRINGTOKEY
#define TYPE_PGPSTRINGTOKEY 1
typedef struct PgpStringToKey PgpStringToKey;
#endif

/* Deallocate the structure */
#define pgpS2Kdestroy(s2k) (s2k)->destroy(s2k)

/* Turn a string into a key */
#define pgpStringToKey(s2k,s,sl,k,kl) (s2k)->stringToKey(s2k,s,sl,k,kl)

/*
* Decodes a byte string into a PgpStringToKey object. Returns either an
* error <0 or the length >=0 of the encoded string2key data found in
* the buffer. If s2k is non-NULL, a structure is allocated and
* returned.
*/
int PGPExport pgpS2Kdecode (struct PgpStringToKey **s2k,
 struct PgpEnv const *env, byte const *buf, size_t len);

/*
* Returns the default PgpStringToKey based on the setting in the
* environment. This is expected to be the usual way to get such
* structures.
*/
struct PgpStringToKey PGPExport *pgpS2Kdefault (struct PgpEnv const *env,
				struct PgpRandomContext const *rc);

/*
* Return old default PgpStringToKey compatible with PGP V2 */
struct PgpStringToKey PGPExport *pgpS2KdefaultV2(struct PgpEnv const *env,
					struct PgpRandomContext const *rc);

int PGPExport pgpS2KisOldVers (struct PgpStringToKey const *s2k);

/*
* Specific PgpStringToKey creation functions. Use these functions to
* specifically request a certain string to key algorithm.
*/
struct PgpStringToKey PGPExport *pgpS2Ksimple (struct PgpEnv const *env,
				struct PgpHash const *h);
struct PgpStringToKey PGPExport *pgpS2Ksalted (struct PgpEnv const *env,
				struct PgpHash const *h,
				byte const *salt8);
struct PgpStringToKey PGPExport *pgpS2Kitersalt (struct PgpEnv const *env,
				struct PgpHash const *h,
				byte const *salt8,
				word32 bytes);

/*
* The PgpStringToKey objects are encoded using the follow table:
*
	* type			number	s2k arguments
	* ----			-----	 -------------
	* simple		\000	 hash specifier
	* salted		\001	 hash specifier + salt8
	* itersalt		\003	hash specifier + salt8 + (compressed) count
	*
	*/

#ifdef __cplusplus
}
#endif

#endif /* PGPSTR2KEY_H */
