/*
 * $Id: pgpRngRead.h,v 1.19 1997/10/15 21:51:59 hal Exp $
 */

#ifndef Included_pgpRngRead_h
#define Included_pgpRngRead_h

#include <stdio.h>	/* For size_t */

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"

/* Semi-public (you can use this, but probably don't want to) */
void const  *
ringFetchObject(RingSet const *set, union RingObject *obj,
	PGPSize *lenp);

/* Public */

RingFile  *ringFileOpen(RingPool *pool,
	PGPFile *f, int trusted, PGPError *error);
void  ringFileSetDestructor(RingFile *file,
	void (*destructor)(RingFile *, PGPFile *, void *),
	void *arg);

void  ringFileHighPri(RingFile *file);
void  ringFileLowPri(RingFile *file);

struct RingTrouble {
	struct RingTrouble const *next;
	union RingObject *obj;	/* The pertinent object, if applicable */
	PGPUInt32 num;	/* An integer parameter, if applicable */
	PGPUInt32 fpos;	/* File position related to the error */
	int type;	/* kPGPError_ code from pgpErrors.h */
};


RingTrouble const  *ringFileTrouble(
	RingFile const *file);
void  ringFilePurgeTrouble(RingFile *file);

int  ringFileCheckClose(RingFile const *file);
PGPError  ringFileClose(RingFile *file);

#define PGP_WRITETRUST_PUB	1
#define PGP_WRITETRUST_SEC	2
PGPError  ringSetWrite(RingSet const *set, PGPFile *f,
			RingFile **filep, PgpVersion version, int flags);

int ringNamesDiffer(RingSet const *set, union RingObject *name1,
	union RingObject *name2);

union RingObject  *ringCreateName(RingSet *dest,
	union RingObject *key, char const *str, size_t len);
union RingObject  *ringCreateSig(RingSet *dest,
	union RingObject *obj, PGPByte *sig, size_t siglen);
union RingObject  *ringCreateKey(RingSet *dest,
	union RingObject *parent, PGPPubKey const *key,
	PGPKeySpec const *ks, PGPByte pkalg);
union RingObject  *ringCreateSec(RingSet *dest,
	union RingObject *parent, PGPSecKey const *sec,
	PGPKeySpec const *ks, PGPByte pkalg);
int  ringRaiseName(RingSet *dest, union RingObject *name);

PGPError  ringSignObject(RingSet *set, union RingObject *obj,
		   PGPSigSpec *spec, PGPRandomContext const *rc);

union RingObject  *ringCreateKeypair (PGPEnv const *env,
		       PGPSecKey *seckey,
		       PGPKeySpec *keyspec,
		       char const *name, size_t namelen,
		       PGPRandomContext const *rc,
		       RingSet *pubset, RingSet *secset,
			   RingSet const *adkset, PGPByte adkclass,
			   RingSet const *rakset, PGPByte rakclass,
			   PGPByte *prefAlg, size_t prefAlgLength,
			   PGPError *error);
union RingObject  *ringCreateSubkeypair (PGPEnv const *env,
			  PGPSecKey *seckey,
			  PGPSecKey *subseckey,
			  PGPKeySpec *keyspec,
			  PGPRandomContext const *rc,
			  RingSet *pubset, RingSet *secset,
			  PGPError *error);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpRngRead_h */
