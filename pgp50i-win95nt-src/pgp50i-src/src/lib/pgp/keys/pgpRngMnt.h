/*
* pgpRngMnt.h
*
* Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRngMnt.h,v 1.4.2.1 1997/06/07 09:50:37 mhw Exp $
*/

#ifndef PGPRNGMNT_H
#define PGPRNGMNT_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
* Opaque types
*/
struct RingIterator;
#ifndef TYPE_RINGITERATOR
#define TYPE_RINGITERATOR 1
typedef struct RingIterator RingIterator;
#endif

struct RingPool;
#ifndef TYPE_RINGPOOL
#define TYPE_RINGPOOL 1
typedef struct RingPool RingPool;
#endif

struct RingSet;
#ifndef TYPE_RINGSET
#define TYPE_RINGSET 1
typedef struct RingSet RingSet;
#endif

union RingObject;
#ifndef TYPE_RINGOBJECT
#define TYPE_RINGOBJECT 1
typedef union RingObject RingObject;
#endif

struct PgpHashContext;
#ifndef TYPE_PGPHASHCONTEXT
#define TYPE_PGPHASHCONTEXT 1
typedef struct PgpHashContext PgpHashContext;
#endif

struct PgpRandomContext;
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

struct PgpSigSpec;
#ifndef TYPE_PGPSIGSPEC
#define TYPE_PGPSIGSPEC 1
typedef struct PgpSigSpec PgpSigSpec;
#endif

word16 PGPExport ringKeyCalcTrust(struct RingSet const *set,
	union RingObject *key);
int PGPExport ringMnt(struct RingSet const *set, struct RingSet *dest,
	word32 const timenow);
int PGPExport ringPoolCheckCount(struct RingSet const *sigset,
		struct RingSet const *keyset,	int allflag);
int PGPExport ringPoolCheck(struct RingSet const *sigset,
	struct RingSet const *keyset, int allflag,
	void (*func)(void *, struct RingIterator *, int), void *arg);
int PGPExport ringHashObj (struct PgpHashContext *hc, union RingObject *obj,
		struct RingSet const *set, int hashnamelength);
int PGPExport ringSignObj(byte *sig, struct RingSet *dest,
	union RingObject *obj, struct PgpSigSpec *spec,
	struct PgpRandomContext const *rc);

void PGPExport
ringMntValidateName (struct RingSet const *set, union RingObject *name);
void PGPExport
ringMntValidateKey (struct RingSet const *set, union RingObject *key);

#ifdef __cplusplus
}
#endif

#endif /* PGPRNGMNT_H */
