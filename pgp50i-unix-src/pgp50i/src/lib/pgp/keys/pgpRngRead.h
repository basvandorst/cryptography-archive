/*
* pgpRngRead.h
*
* Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRngRead.h,v 1.4.2.2 1997/06/07 09:50:43 mhw Exp $
*/

#ifndef PGPRNGREAD_H
#define PGPRNGREAD_H

	#include <stdio.h>		/* For size_t */

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

union RingObject;
#ifndef TYPE_RINGOBJECT
#define TYPE_RINGOBJECT 1
typedef union RingObject RingObject;
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

struct PgpKeySpec;
#ifndef TYPE_PGPKEYSPEC
#define TYPE_PGPKEYSPEC 1
typedef struct PgpKeySpec PgpKeySpec;
#endif

struct PgpSigSpec;
#ifndef TYPE_PGPSIGSPEC
#define TYPE_PGPSIGSPEC 1
typedef struct PgpSigSpec PgpSigSpec;
#endif

struct PgpRandomContext;
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

/* Semi-public (you can use this, but probably don't want to) */
void const PGPExport *
ringFetchObject(struct RingSet const *set, union RingObject *obj,
	size_t *lenp);

/* Public */

struct RingFile PGPExport *ringFileOpen(struct RingPool *pool,
	struct PgpFile *f, int trusted, int *error);
void PGPExport ringFileSetDestructor(struct RingFile *file,
	void (*destructor)(struct RingFile *, struct PgpFile *, void *),
	void *arg);

void PGPExport ringFileHighPri(struct RingFile *file);
void PGPExport ringFileLowPri(struct RingFile *file);

struct RingTrouble {
			struct RingTrouble const *next;
			union RingObject *obj;	/* The pertinent object, if applicable */
			word32 num;	/* An integer parameter, if applicable */
			word32 fpos;	 /* File position related to the error */
			int type;	/* PGPERR_ code from pgpErr.h */
	};

#ifndef TYPE_RINGTROUBLE
#define TYPE_RINGTROUBLE 1
typedef struct RingTrouble RingTrouble;
#endif

struct RingTrouble const PGPExport *ringFileTrouble(
	struct RingFile const *file);
void PGPExport ringFilePurgeTrouble(struct RingFile *file);

int PGPExport ringFileCheckClose(struct RingFile const *file);
int PGPExport ringFileClose(struct RingFile *file);

#define PGP_WRITETRUST_PUB	1
#define PGP_WRITETRUST_SEC	2
int PGPExport ringSetWrite(struct RingSet const *set, struct PgpFile *f,
	struct RingFile **filep, PgpVersion version, int flags);

int ringNamesDiffer(struct RingSet const *set, union RingObject *name1,
	union RingObject *name2);

union RingObject PGPExport *ringCreateName(struct RingSet *dest,
	union RingObject *key, char const *str, size_t len);
union RingObject PGPExport *ringCreateSig(struct RingSet *dest,
	union RingObject *obj, byte *sig, size_t siglen);
union RingObject PGPExport *ringCreateKey(struct RingSet *dest,
	union RingObject *parent, struct PgpPubKey const *key,
	struct PgpKeySpec const *ks, byte pkalg);
union RingObject PGPExport *ringCreateSec(struct RingSet *dest,
	union RingObject *parent, struct PgpSecKey const *sec,
	struct PgpKeySpec const *ks, byte pkalg);
int PGPExport ringRaiseName(struct RingSet *dest, union RingObject *name);

int PGPExport ringSignObject(struct RingSet *set, union RingObject *obj,
		struct PgpSigSpec *spec, struct PgpRandomContext const *rc);

int PGPExport ringCreateKeypair (struct PgpEnv const *env,
		struct PgpSecKey *seckey,
		struct PgpKeySpec *keyspec,
		char const *name, size_t namelen,
		struct PgpRandomContext const *rc,
		struct RingSet *pubset, struct RingSet *secset);
int PGPExport ringCreateSubkeypair (struct PgpEnv const *env,
						struct PgpSecKey *seckey,
						struct PgpSecKey *subseckey,
						struct PgpKeySpec *keyspec,
						struct PgpRandomContext const *rc,
						struct RingSet *pubset, struct RingSet *secset);

#ifdef __cplusplus
}
#endif

#endif /* PGPRNGREAD_H */
