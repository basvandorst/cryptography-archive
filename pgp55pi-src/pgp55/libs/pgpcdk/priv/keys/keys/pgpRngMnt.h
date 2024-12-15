/*
 * $Id: pgpRngMnt.h,v 1.12 1997/06/25 19:40:44 lloyd Exp $
 */

#ifndef Included_pgpRngMnt_h
#define Included_pgpRngMnt_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"

PGPUInt16  ringKeyCalcTrust(RingSet const *set,
	union RingObject *key);
int  ringMnt(RingSet const *set, RingSet *dest, 
	    PGPUInt32 const timenow);
int  ringPoolCheckCount(RingSet const *sigset, 
		       RingSet const *keyset,	int allflag);
PGPError  ringPoolCheck(RingSet const *sigset,
				RingSet const *keyset, int allflag, 
				void (*func)(void *, RingIterator *, int), void *arg);
int  ringHashObj (PGPHashContext *hc, union RingObject *obj,
		 RingSet const *set, int hashnamelength);
int  ringSignObj(PGPByte *sig, RingSet *dest,
	union RingObject *obj, PGPSigSpec *spec,
	PGPRandomContext const *rc);

void 
ringMntValidateName (RingSet const *set, union RingObject *name);
void 
ringMntValidateKey (RingSet const *set, union RingObject *key);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpRngMnt_h */
