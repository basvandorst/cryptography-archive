/*
* pgpConvKey.h -- Conventional Encryption Keys for PGP
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written By:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpConvKey.h,v 1.3.2.1 1997/06/07 09:50:18 mhw Exp $
*/

#ifndef PGPCONVKEY_H
#define PGPCONVKEY_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpStringToKey;
#ifndef TYPE_PGPSTRINGTOKEY
#define TYPE_PGPSTRINGTOKEY 1
typedef struct PgpStringToKey PgpStringToKey;
#endif

struct PgpConvKey {
		struct PgpConvKey const *next;
			struct PgpStringToKey const *stringToKey;
		char const *pass;
		size_t passlen;
	};
#ifndef TYPE_PGPCONVKEY
#define TYPE_PGPCONVKEY 1
typedef struct PgpConvKey PgpConvKey;
#endif

#ifdef __cplusplus
}
#endif

#endif /* PGPCONVKEY_H */
