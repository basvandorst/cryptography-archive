/*
* pgpVrfySig.h -- Signature Verification Code
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpVrfySig.h,v 1.3.2.1 1997/06/07 09:51:02 mhw Exp $
*/
#ifndef PGPVRFYSIG_H
#define PGPVRFYSIG_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpHashContext;
struct PgpPipeline;
struct PgpEnv;
struct PgpUICb;
struct PgpSig;

int pgpSigSetupPipeline (struct PgpPipeline **head, int *numhashes,
   struct PgpHashContext **hashes,
   byte const *hashlist, unsigned hashlen,
   byte const *charmap);

int pgpSigSetupHashes (struct PgpPipeline **tail, struct PgpEnv const *env,
  struct PgpHashContext **hashlist, int *hashnum,
  struct PgpSig const *siglist,
  struct PgpUICb const *ui, void *ui_arg);

int pgpSigVerify (struct PgpSig const *siglist, struct PgpHashContext *hashes,
  int numhashes, struct PgpUICb const *ui, void *arg);

int pgpSepsigVerify (struct PgpSig const *siglist, struct PgpEnv const *env,
  struct PgpUICb const *ui, void *ui_arg);

#ifdef __cplusplus
}
#endif

#endif /* PGPVRFYSIG_H */
