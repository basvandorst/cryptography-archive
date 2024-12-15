/*
* pgpUsuals.h - Typedefs and #defines used widely.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpUsuals.h,v 1.2.2.1 1997/06/07 09:50:20 mhw Exp $
*/
#ifndef PGPUSUALS_H
#define PGPUSUALS_H

#include <limits.h>
#include "pgpTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#if UCHAR_MAX == 0xff
typedef unsigned char byte;
typedef signed char int8;
#else
#error This machine has no 8-bit type
#endif

#if UINT_MAX == 0xffff
typedef uint word16;
typedef int int16;
#elif USHRT_MAX == 0xffff
typedef unsigned short word16;
typedef short int16;
#else
#error This machine has no 16-bit type
#endif

#if UINT_MAX == 0xfffffffful
typedef uint word32;
typedef int int32;
#elif ULONG_MAX == 0xfffffffful
typedef ulong word32;
typedef long int32;
#else
#error This machine has no 32-bit type
#endif

/*
* Find a 64-bit data type, if possible.
* The conditions here are more complicated to avoid using numbers that
* will choke lesser preprocessors (like 0xffffffffffffffff) unless
* we're reasonably certain that they'll be acceptable.
*/
#if ULONG_MAX > 0xfffffffful
#if ULONG_MAX == 0xfffffffffffffffful
typedef ulong bnword64;
#define BNWORD64 bnword64
#define HAVE64 1
#endif
#endif

/*
* I would test the value of unsigned long long, but some *preprocessors*
* choke on constants that long even if the compiler can accept them, so
* it doesn't work reliably. So cross our fingers and hope that it's
* a 64-bit type.
*
* GCC uses ULONG_LONG_MAX. Solaris uses ULLONG_MAX.
* IRIX uses ULONGLONG_MAX. Are there any other names for this?
*/

#ifndef HAVE64
#if defined(ULONG_LONG_MAX) || defined (ULLONG_MAX) || defined(ULONGLONG_MAX)
typedef unsigned long long word64;
typedef long long int64;
#define HAVE64 1
#endif
#endif

#include <string.h>	 /* Prototype for memset */
/*
* Wipe sensitive data.
* Note that this takes a pointer to a structure to be wiped!
*/
#define wipe(x) memset(x, 0, sizeof(*(x)))

/* This is a useful macro to find the minimum of two values */
#ifndef min
#define min(x,y) (((x)<(y)) ? (x) : (y))
#endif

#ifndef max
#define max(x,y) (((x)>(y)) ? (x) : (y))
#endif

/* A way to hold the PGP Version number */
typedef int PgpVersion;
#define PGPVERSION_2	2	 /* 2.0 through 2.5 */
#define PGPVERSION_2_6	3	/* 2.6.x */
#define PGPVERSION_3 4 /* 3.0 */

/* The PGP Library Version string */
extern char const pgpLibVersionString[];

/* The PGP Library Cipher IV Length */
#define IVLEN	10

/* Literal Message Types */
#define PGP_LITERAL_TEXT		'\164' /* Ascii 't' */
#define PGP_LITERAL_BINARY		'\142' /* Ascii 'b' */

#ifdef __cplusplus
}
#endif

#endif /* PGPUSUALS_H */
