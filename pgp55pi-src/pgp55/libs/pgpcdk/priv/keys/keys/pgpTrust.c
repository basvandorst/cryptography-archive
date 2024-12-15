/*
 * $Id: pgpTrust.c,v 1.6 1997/07/14 04:27:13 hal Exp $
 */

#include <math.h>

#include "pgpConfig.h"

#include "pgpDebug.h"
#include "pgpRngPriv.h"
#include "pgpTrust.h"
#include "pgpTrstPkt.h"

PGPByte
ringTrustToExtern(PGPUInt16 trust)
{
#if PGPTRUSTMODEL==0
	(void)trust;
	pgpAssert(0);
	return 0;
#else
	if (trust == PGP_TRUST_INFINITE)
		return (PGPByte) PGP_NEWTRUST_INFINITE;
	trust >>= TRUST_CERTSHIFT;
	if (trust > PGP_NEWTRUST_MAX)
		trust = PGP_NEWTRUST_MAX;
	return (PGPByte) trust;
#endif
}

PGPUInt16
ringTrustToIntern(PGPByte trust)
{
#if PGPTRUSTMODEL==0
	(void)trust;
	pgpAssert(0);
	return 0;
#else
	if (trust > PGP_NEWTRUST_MAX) {
		if (trust == PGP_NEWTRUST_INFINITE)
			return (PGPUInt16) PGP_TRUST_INFINITE;
		else
			return 0;	/* Undefined */
	}
	return (PGPUInt16) trust << TRUST_CERTSHIFT;
#endif
}

/* Convert from old 3-level trust to the new 256-level (extern format) */
PGPByte
ringTrustOldToExtern(RingPool const *pool, PGPByte oldTrust)
{
	PGPByte externTrust;

	switch (oldTrust & kPGPKeyTrust_Mask) {
	  case kPGPKeyTrust_Ultimate:
		externTrust = PGP_NEWTRUST_INFINITE;
		break;
	  case kPGPKeyTrust_Complete:
		externTrust = pool->completeconfidence;
		break;
	  case kPGPKeyTrust_Marginal:
		externTrust = pool->marginalconfidence;
		break;
	  case kPGPKeyTrust_Undefined:
		externTrust = PGP_NEWTRUST_UNDEFINED;
		break;
	  default:
		externTrust = 0;
		break;
	}
	return externTrust;
}

/* Convert from new 256-level (extern format) to old 3-level trust  */
PGPByte
ringTrustExternToOld(RingPool const *pool, PGPByte externTrust)
{
	if (externTrust >= PGP_NEWTRUST_INFINITE)
		return kPGPKeyTrust_Ultimate;
	if (externTrust >= pool->completeconfidence)
		return kPGPKeyTrust_Complete;
	if (externTrust >= pool->marginalconfidence)
		return kPGPKeyTrust_Marginal;
	return kPGPKeyTrust_Unknown;
}

int
ringTrustValid(RingSet const *set, PGPUInt16 validity)
{
#if PGPTRUSTMODEL==0
	(void)set;
	(void)validity;
	pgpAssert(0);
	return 0;
#else
	if (validity >= (set->pool->threshold << TRUST_CERTSHIFT))
		return 1;
	else
		return 0;
#endif
}

static double k1, k2;
static double kmaxt;	/* Maximum value of trust in terms of 1/n error prob */

static void
ringTrustInit(void)
{
	double d = log(10.0);

	k1 = d/PGP_TRUST_DECADE;
	k2 = PGP_TRUST_DECADE/d;
	kmaxt = exp (PGP_TRUST_MAX * k1);
}

#define RINGTRUSTINIT(k) if (!k) ringTrustInit()

unsigned
ringIntToTrust(unsigned long r)
{
	RINGTRUSTINIT(k2);
	return (unsigned) (log ((double) r) * k2 + 0.5);
}

unsigned long
ringTrustToInt(unsigned t)
{
	RINGTRUSTINIT(k1);
	return (unsigned long) (exp (t * k1) + 0.5);
}

#ifdef __GNUC__		/* XXX: Should be something like HAS_LONG_LONG */
unsigned long long
ringTrustToUll(unsigned t)
{
	RINGTRUSTINIT(k1);
	return (unsigned long long) (exp (t * k1) + 0.5);
}
#endif

double
ringTrustToDouble(unsigned t)
{
	RINGTRUSTINIT(k1);
	return exp (t * k1);
}

unsigned
ringDoubleToTrust(double d)
{
	pgpAssert (d >= .9);
	RINGTRUSTINIT(k2);
	if (d >= kmaxt)
		return PGP_NEWTRUST_MAX;
	else
		return (unsigned) (log (d) * k2 + 0.5);
}

#if 0
#include <stdio.h>

static void
prettytrust(unsigned t, int wid)
{
	int d = t - PGP_TRUST_DECADE - PGP_TRUST_OCTAVE;	/* Tweak this*/
	char buf[10];
	int i;
	unsigned long l;

	if (d >= 0) {
		/* Straightforward */
		d -= d % PGP_TRUST_DECADE;
		i = d / PGP_TRUST_DECADE;
		l = ringTrustToInt(t-d);
		if (i)
			wid -= printf("1/%lu%0*u", l, i, 0);
		else
			wid -= printf("1/%lu", l);
	} else {
		/* Do decimal point thing */
		i = 0;
		do {
			i++;
			d += PGP_TRUST_DECADE;
			t += PGP_TRUST_DECADE;
		} while (d < 0);

		sprintf(buf, "%lu", ringTrustToInt(t));
		i = (int)strlen(buf) - i;

		wid -= printf("1/%.*s.%s", i, buf, buf+i);
	}
	while (--wid >= 0)
		putchar(' ');
}

int
main(void)
{
	unsigned t;
	unsigned long long j;

	for (t = 0; t <= 65535; t++) {
		j = ringTrustToUll(t);
		printf("%5u = 1/%-9Lu = ", t, j);
		prettytrust(t, 28);
		putchar('\n');
	}
	return 0;
}
#endif


