/*
* pgpCiphrMod.h -- secret-key cipher module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpCiphrMod.h,v 1.4.2.1 1997/06/07 09:50:48 mhw Exp $
*/

#ifndef PGPCIPHRMOD_H
#define PGPCIPHRMOD_H

#include "pgpUsuals.h"
#include "pgpEnv.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpCfbContext;
#ifndef TYPE_PGPCFBCONTEXT
#define TYPE_PGPCFBCONTEXT 1
typedef struct PgpCfbContext PgpCfbContext;
#endif

struct PgpFifoDesc;
#ifndef TYPE_PGPFIFODESC
#define TYPE_PGPFIFODESC 1
typedef struct PgpFifoDesc PgpFifoDesc;
#endif

/*
* Create a Cfb Decryption Module using the passed-in PgpCfbContext.
* This module assumes control of the PgpCfbContext and will clear/free
* it upon teardown.
*/
struct PgpPipeline PGPExport **
pgpCipherModDecryptCreate (struct PgpPipeline **head,
			struct PgpCfbContext *cfb, struct PgpEnv const *env);

/*
* Create a Cfb Encryption Module using the passed-in PgpCfbContext. The
* user must pass in an 8-byte iv, which will be encrypted into the
* stream. The Cipher Module assumes control of the PgpCfbContext and
* will clear/free it upon teardown.
*/
struct PgpPipeline PGPExport **
pgpCipherModEncryptCreate (struct PgpPipeline **head, PgpVersion version,
			struct PgpFifoDesc const *fd,
			struct PgpCfbContext *cfb,
			byte const iv[IVLEN-2],
			 		struct PgpEnv const *env);

#ifdef __cplusplus
}
#endif

#endif /* PGPCIPHRMOD_H */
