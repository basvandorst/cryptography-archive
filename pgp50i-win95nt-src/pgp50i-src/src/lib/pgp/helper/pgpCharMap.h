/*
 * pgpCharMap.h -- Mapping tables between different character sets.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpCharMap.h,v 1.2.2.1 1997/06/07 09:50:01 mhw Exp $
 */
#ifndef PGPCHARMAP_H
#define PGPCHARMAP_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpEnv;

/* The identity charmap; maps all characters onto themselves */
extern unsigned char const charMapIdentity[256];

/*
 * Fills in the charmaps associated with the charset and returns 0
 * or returns a PGP error if the set isn't found or a paramter is
 * invalid.
 */
int pgpCharmaps (char const *charset, int setlen,
                 byte const **toLocal, byte const **toLatin1);

/*
 * Converts a string from one charset to another using the input
 * charmap. The input, of size inlen, is converted through the map
 * and put into output.
 */
void pgpCharmapConvert (byte const *input, size_t inlen, byte *output,
                        byte const *charmap);

/* Two macros to convert strings between the local and latin1 charsets */
#define PGP_CONVERT_TO_LOCAL(in,len,out,env) \
    pgpCharmapConvert (in,len,out,\
            pgpenvGetPointer (env, PGPENV_CHARMAPTOLOCAL,NULL))

#define PGP_CONVERT_TO_LATIN1(in,len,out,env) \
    pgpCharmapConvert (in,len,out,\
            pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1,NULL))

#ifdef __cplusplus
}
#endif

#endif /* PGPCHARMAP_H */
