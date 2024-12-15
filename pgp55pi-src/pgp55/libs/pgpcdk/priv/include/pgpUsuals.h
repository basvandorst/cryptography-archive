/*
 * pgpUsuals.h - Typedefs and #defines used widely.
 *
 * $Id: pgpUsuals.h,v 1.12 1997/08/19 21:56:27 lloyd Exp $
 */
#ifndef Included_pgpUsuals_h
#define Included_pgpUsuals_h

#include "pgpBase.h"

#include "pgpOpaqueStructs.h"

PGP_BEGIN_C_DECLARATIONS


#if PGP_HAVE64
typedef PGPUInt64	bnword64;
#endif



/* A way to hold the PGP Version number */
typedef int PgpVersion;
#define PGPVERSION_2	2	/* 2.0 through 2.5 */
#define PGPVERSION_2_6	3	/* 2.6.x */
#define PGPVERSION_3    4       /* 3.0 */


/* The PGP Library Cipher IV Length */
#define IVLEN	10

/* Literal Message Types */
#define PGP_LITERAL_TEXT	'\164' /* Ascii 't' */
#define PGP_LITERAL_BINARY	'\142' /* Ascii 'b' */

PGP_END_C_DECLARATIONS

#endif /* Included_pgpUsuals_h */
