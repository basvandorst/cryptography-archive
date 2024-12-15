/*
* spgptext.c -- Simple PGP API helper -- Text conversions
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgptext.c,v 1.5.2.1 1997/06/07 09:51:57 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"
#include "pgpTimeDate.h"

static PGPLineEndType	sPGPLineEndType
#if defined(MSDOS) || defined(WIN32)
		= kPGPLineEndCRLF;
#elif MACINTOSH
		= kPGPLineEndCR;
#else
		= kPGPLineEndLF;
#endif


/* Create an 8 character key ID string */
void
spgpKeyIDText8 (char *out, byte *keyid)
{
	int i;
	byte c;

	for (i=0; i<4; i++) {
		c = keyid[i+4];
		*out++ = "0123456789abcdef" [ c >> 4 ];
		*out++ = "0123456789abcdef" [ c & 0xf ];
	}
	*out++ = '\0';
}

/* Create a date in the form MM/DD/YYYY */
void
spgpDateText10 (char *out, word32 date)
{
	struct tm const *tm;
	PGPTime	dtime;
	int	m,
		d,
		y;

	if (!date) {
		strcpy (out, "--/--/----");
		return;
	}

	dtime = (PGPTime) date;
	tm = pgpLocalTime (&dtime);

	if (tm) {
		m = tm->tm_mon + 1;
		*out++ = '0' + m / 10;
		*out++ = '0' + m % 10;
		*out++ = '/';
		d = tm->tm_mday;
		*out++ = '0' + d / 10;
		*out++ = '0' + d % 10;
		*out++ = '/';
		y = tm->tm_year + 1900;
		*out++ = '0' + y / 1000;
		y %= 1000;
		*out++ = '0' + y / 100;
		y %= 100;
		*out++ = '0' + y / 10;
		*out++ = '0' + y % 10;
		*out++ = '\0';
	}
}


/* Output a 3 character value, an expiration interval */
void
spgpExpText3 (char *out, int exp)
{
	if (exp > 999) {
		strcpy (out, "XXX");
		return;
	}
	*out++ = '0' + exp / 100;
	exp %= 10;
	*out++ = '0' + exp / 10;
	*out++ = '0' + exp % 10;
	*out++ = '\0';
}


	PGPLineEndType
SimplePGPGetLineEndType (void)
{
	return sPGPLineEndType;
}

	PGPLineEndType
SimplePGPSetLineEndType (
	PGPLineEndType	lineEndType)
{
	PGPLineEndType	oldVal = sPGPLineEndType;

	sPGPLineEndType = lineEndType;
	return oldVal;
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
