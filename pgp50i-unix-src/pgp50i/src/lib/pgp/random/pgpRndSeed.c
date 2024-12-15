/*
 * pgpRndSeed.c -- Read and write randseed.bin files, to seed and "store"
 * random number generate state.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU> and Colin Plumb
 *
 * $Id: pgpRndSeed.c,v 1.6.2.4 1997/06/24 18:51:36 hal Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpRnd.h"
#include "pgpRndSeed.h"
#include "pgpCFB.h"
#include "pgpErr.h"
#include "pgpRndPool.h"

/*
 * Save the state of the random number generator to "file" using "cfb"
 * as the washing mechanism.  If cfb is NULL, do not wash the output.
 *
 * Try to write "bytes", or a reasonable minimum size, whichever is the
 * greater.  Returns the number of bytes actually written.
 */
static size_t
pgpRandSeedWriteBytes (FILE *file, struct PgpRandomContext const *rc,
                       struct PgpCfbContext *cfb, size_t bytes)
{
	byte buf[128];          /* Additional bytes written out */
	unsigned l;
	size_t total, written;

	/* Figure out a "reasonable minimum size" */
	written = (pgpRandPoolSize()+7)/8;
	if (written < PGP_SEED_MIN_BYTES)
		written = PGP_SEED_MIN_BYTES;
	else if (written > 512)
		written = 512;

	/* Always try to write at least this "reasonable minimum size" */
	if (bytes < written)
		bytes = written;

	/* Add the current time */
	(void)ranGetEntropy(&pgpRandomPool);
	/* Stir, to hide patterns */
	if (!rc)
		rc = &pgpRandomPool;
	pgpRandomStir(rc);

	total = 0;
	while (total < bytes) {
		l = (unsigned)(bytes-total < sizeof(buf) ? bytes-total
		                                         : sizeof(buf));
		pgpRandomGetBytes(rc, buf, l);
		if (cfb)
			pgpCfbEncrypt (cfb, buf, buf, l);
		written = fwrite (buf, 1, l, file);
		if (!written)
			break;
		total += written;
	}
	memset (buf, 0, sizeof(buf));
	return total;
}

/*
 * Load the RNG state from the file on disk (randseed.bin).
 * Seeds both the RandomPool *and* the passed-in generator.
 * Returns 0 on success, <0 on error.
 *
 * Must read at least 24 bytes (the size of the X9.17 generator's state),
 * and write as much back out again, to be considered successful.
 * Any additional data is gravy.
 */
int
pgpRandSeedRead (FILE *file, struct PgpRandomContext const *rc)
{
	byte buf[128];
	int len;
	size_t total, rewrite;

	if (!file)
		return PGPERR_NO_FILE;

	pgpAssert (file);

	/* Dump the file into the random number generator */
	total = 0;
	for (;;) {
		len = fread ((char *)buf, 1, sizeof(buf), file);
		if (len <= 0)
			break;
		total += len;
		if (rc)
			pgpRandomAddBytes(rc, buf, len);
		pgpRandomAddBytes(&pgpRandomPool, buf, len);
	}

#if 0
/*
 * Disable this test in the interests of speed.  An earlier version as
 * missing the rewind() causing it to double the size of the randseed
 * file each time on some systems.  The caller checks that the file could
 * be opened read/write, which should be safe enough.
 */
	/* Write it back out again */
	rewind (file);
	rewrite = pgpRandSeedWriteBytes (file, rc, NULL, total);
#else
	rewrite = PGP_SEED_MIN_BYTES;
#endif

	/*
	 * If we can't rewrite it, we may have the same seed as
	 * another run of the program, which means it's not entropy
	 * at all.  We try not to depend *totally* on the seed file,
	 * but paranoia is many levels deep.
	 */
	/* If we didn't read enough *or* rewrite enough, complain */
	if (total < PGP_SEED_MIN_BYTES || rewrite < PGP_SEED_MIN_BYTES)
		return PGPERR_RANDSEED_TOOSMALL;
	return 0;	/* No error */
}

/*
 * Save the "state" of the random number generator to "file" using
 * "cfb" as the washing mechanism.  If cfb is NULL, do not wash
 * the output.
 */
void
pgpRandSeedWrite (FILE *file, struct PgpRandomContext const *rc,
		  struct PgpCfbContext *cfb)
{
	if (!file)
		return;

	/* Here, we just try.  If we fail, oh, well. */
	(void)pgpRandSeedWriteBytes (file, rc, cfb, 0);
}
