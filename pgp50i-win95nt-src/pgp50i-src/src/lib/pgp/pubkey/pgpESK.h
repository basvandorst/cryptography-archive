/*
 * pgpESK.h -- Encrypted Session Key handler.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpESK.h,v 1.3.2.1 1997/06/07 09:51:24 mhw Exp $
 */

#ifndef PGPESK_H
#define PGPESK_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Return values from eskType */
#define PGP_ESKTYPE_PASSPHRASE 0
#define PGP_ESKTYPE_PUBKEY 1

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpESK;
#ifndef TYPE_PGPESK
#define TYPE_PGPESK 1
typedef struct PgpESK PgpESK;
#endif

struct PgpSecKey;
#ifndef TYPE_PGPSECKEY
#define TYPE_PGPSECKEY 1
typedef struct PgpSecKey PgpSecKey;
#endif

/* Some access functions. */
int PGPExport pgpEskType (struct PgpESK const *esk);
int PGPExport pgpEskPKAlg (struct PgpESK const *esk);
byte const PGPExport *pgpEskId8 (struct PgpESK const *esk);

/* How big might the output key possibly be? */
size_t PGPExport pgpEskMaxKeySize (struct PgpESK const *esk);
/*
 * The "key" is a passphrase.  Returns the length of the key, which
 * is <algorithm identifier><key bits>
 */
int PGPExport pgpEskConvDecrypt (struct PgpESK const *esk,
	struct PgpEnv const *env, char const *pass, size_t plen, byte *buf);

/* Do public-key decryption */
int PGPExport pgpEskPKdecrypt (struct PgpESK const *esk,
	struct PgpEnv const *env, struct PgpSecKey *sec, byte *buf);

struct PgpESK PGPExport *pgpEskNext (struct PgpESK const *esklist);

int PGPExport pgpEskAdd (struct PgpESK **esklist, int type, byte const *buf,
	size_t len);
void PGPExport pgpEskFreeList (struct PgpESK *esklist);

#ifdef __cplusplus
}
#endif

#endif /* PGPESK_H */
