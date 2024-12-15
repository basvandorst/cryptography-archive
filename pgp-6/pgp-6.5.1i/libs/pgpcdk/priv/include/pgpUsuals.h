/*
 * pgpUsuals.h - Typedefs and #defines used widely.
 *
 * $Id: pgpUsuals.h,v 1.17 1999/04/14 05:47:32 hal Exp $
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
#define PGPVERSION_3	3	/* 2.6.x */
#define PGPVERSION_4    4       /* 3.0 */


/* The PGP Library Cipher maximum IV Length for symmetric encrypted blocks */
#define MAXIVLEN		16

/* Literal Message Types */
#define PGP_LITERAL_TEXT	'\164' /* Ascii 't' */
#define PGP_LITERAL_BINARY	'\142' /* Ascii 'b' */
/* Used only internally for now, when looks like PGP msg inside literal */
#define PGP_LITERAL_RECURSE	'\162' /* Ascii 'r' */

PGP_END_C_DECLARATIONS

#endif /* Included_pgpUsuals_h */