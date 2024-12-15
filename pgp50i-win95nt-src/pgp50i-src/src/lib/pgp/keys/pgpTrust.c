/*
* pgpTrust.c
*
* Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpTrust.c,v 1.6.2.1 1997/06/07 09:50:44 mhw Exp $
*/

#include <math.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpDebug.h"
#include "pgpRngPriv.h"
#include "pgpTrust.h"
#include "pgpTrstPkt.h"

byte
ringTrustToExtern(word16 trust)
{
#if PGPTRUSTMODEL==0
		(void)trust;
		pgpAssert(0);
		return 0;
#else
		if (trust == PGP_TRUST_INFINITE)
				return (byte) PGP_NEWTRUST_INFINITE;
		trust >>= TRUST_CERTSHIFT;
		if (trust > PGP_NEWTRUST_MAX)
				trust = PGP_NEWTRUST_MAX;
		return (byte) trust;
#endif
}

word16
ringTrustToIntern(byte trust)
{
#if PGPTRUSTMODEL==0
		(void)trust;
		pgpAssert(0);
		return 0;
#else
		if (trust > PGP_NEWTRUST_MAX) {
				if (trust == PGP_NEWTRUST_INFINITE)
						return (word16) PGP_TRUST_INFINITE;
				else
				return 0;	/* Undefined */
		}
		return (word16) trust << TRUST_CERTSHIFT;
#endif
}

int
ringTrustValid(struct RingSet const *set, word16 validity)
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
		int d = t - PGP_TRUST_DECADE - PGP_TRUST_OCTAVE;		/* Tweak this*/
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
