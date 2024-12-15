/*
 * pgpMakeSig.h -- Make a signature packet from a secret key, a hash, and
 * extra data.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpMakeSig.h,v 1.8 1997/06/25 19:40:58 lloyd Exp $
 */

#ifndef Included_pgpMakeSig_h
#define Included_pgpMakeSig_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

struct PGPSigSpec;
struct PGPHashContext;
struct PGPRandomContext;

/* The maximum size of the signature */
int pgpMakeSigMaxSize(PGPSigSpec const *spec);
	
/*
 * Given a buffer of at least "pgpMakeSigMaxSize" bytes, make a signature
 * into it and return the size of the signature, or 0.
 */
int
pgpMakeSig(PGPByte *buf, PGPSigSpec const *spec,
	PGPRandomContext const *rc, PGPHashContext const *hc);

/* The maximum size of a signature header (one-pass sig) */
int pgpMakeSigHeaderMaxSize (PGPSigSpec const *spec);

/* Create a one-pass signature header block for "spec", putting it in buf */
int pgpMakeSigHeader (PGPByte *buf, PGPSigSpec const *spec, PGPByte nest);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpMakeSig_h */
