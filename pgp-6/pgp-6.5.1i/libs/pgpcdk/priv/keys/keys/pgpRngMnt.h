/*
 * $Id: pgpRngMnt.h,v 1.15 1998/06/03 20:24:45 hal Exp $
 */

#ifndef Included_pgpRngMnt_h
#define Included_pgpRngMnt_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"

PGPUInt16	ringKeyCalcTrust(RingSet const *set,
					union RingObject *key);
int			ringMnt(RingSet const *set, RingSet *dest, 
				    PGPUInt32 const timenow);
int			ringPoolCheckCount(RingSet const *sigset, RingSet const *keyset,
					PGPBoolean allflag, PGPBoolean revocationonly);
PGPError	ringPoolCheck(RingSet const *sigset, RingSet const *keyset,
					PGPBoolean allflag, PGPBoolean revocationonly,
					PGPError (*func)(void *, RingIterator *, int), void *arg);
int			ringHashObj (PGPHashContext *hc, union RingObject *obj,
					RingSet const *set, int hashnamelength);
int			ringSignObj(PGPByte *sig, RingSet *dest,
					union RingObject *obj, PGPSigSpec *spec,
					PGPRandomContext const *rc);
void		ringMntValidateName (RingSet const *set, union RingObject *name);
void		ringMntValidateKey (RingSet const *set, union RingObject *key);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpRngMnt_h */
