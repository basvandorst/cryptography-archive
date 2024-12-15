/*
 * pgpMakeSig.h -- Make a signature packet from a secret key, a hash, and
 *                 extra data.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpMakeSig.h,v 1.2.2.1 1997/06/07 09:51:28 mhw Exp $
 */

#ifndef PGPMAKESIG_H
#define PGPMAKESIG_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpSigSpec;
struct PgpHashContext;
struct PgpRandomContext;

/* The maximum size of the signature */
int pgpMakeSigMaxSize(struct PgpSigSpec const *spec);
	
/*
 * Given a buffer of at least "pgpMakeSigMaxSize" bytes, make a signature
 * into it and return the size of the signature, or 0.
 */
int
pgpMakeSig(byte *buf, struct PgpSigSpec const *spec,
	struct PgpRandomContext const *rc, struct PgpHashContext const *hc);

/* The maximum size of a signature header (one-pass sig) */
int pgpMakeSigHeaderMaxSize (struct PgpSigSpec const *spec);

/* Create a one-pass signature header block for "spec", putting it in buf */
int pgpMakeSigHeader (byte *buf, struct PgpSigSpec const *spec, byte nest);

#ifdef __cplusplus
}
#endif

#endif /* PGPMAKESIG_H */
