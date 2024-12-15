/*
 * pgpKeyRings.h -- Interface to open all the known keyrings.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpKeyRings.h,v 1.3.2.1 1997/06/07 09:49:05 mhw Exp $
 */

#ifndef PGPKEYRINGS_H
#define PGPKEYRINGS_H

#ifdef __cplusplus
extern "C" {
#endif

struct RingPool;
struct RingSet;

void
mainCloseKeyrings (int update, int newversion);

struct RingSet *
mainOpenRingfile (struct PgpEnv const *env, struct RingPool *ringpool,
		  char const *filename, char const *ringtype, int trusted);

int
mainOpenKeyrings (struct PgpEnv const *env, struct RingPool *ringpool,
		  int trusted_only, struct RingSet **ringsetp);

int
mainRingNewSet (char const *filename, int flags, struct RingSet *set);

#ifdef __cplusplus
}
#endif

#endif /* PGPKEYRINGS_H */

