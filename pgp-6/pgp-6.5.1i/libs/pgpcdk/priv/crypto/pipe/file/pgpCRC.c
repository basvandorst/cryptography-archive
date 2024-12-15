/*
 * pgpCRC.c - CRC computation routines (wow, how thrilling!)
 *
 * Written by Colin Plumb
 *
 * $Id: pgpCRC.c,v 1.5 1997/06/04 16:17:45 lloyd Exp $
 */

#include "pgpConfig.h"

#include "pgpCRC.h"
#include "pgpUsuals.h"

/* PRZ's magic 24-bit polynomial - (x+1) * (irreducible of degree 23) */
#define POLY24 0x1864cfbu

/*
 * Build the CRC-24 table.
 *
 * A byte-at-a-time CRC table is simply a table of the CRCs
 * of the individual bytes.
 *
 * From basic CRC theory, crc(c^y) = crc(x) ^ crc(y).
 * (^ is XOR, which is addition to CRCs.)
 *
 * Through this code, h holds the CRC of i.  (i is a power of 2.)
 * The CRCs of the values from i to 2*i-1 are the CRCs of the
 * values from 0 to i-1, XORed with h.
 *
 * Then when you get to 2*i, shift h up one bit, XOR in the polynomail
 * if necessary to get the right high 8 bits, and continue for 2*i
 * through 4*i-1.
 *
 * This drastically reduces the amount of bit-twiddling necessary
 * to generate a CRC table.
 */
static void
crcInit(PGPUInt32 table[256])
{
	unsigned i, j;
	PGPUInt32 h;	/* CRC of i, where i is a power of 2 */

	table[0] = 0;
	table[1] = h = POLY24;

	for (i = 2; i < 256; i *= 2) {
		if ((h <<= 1) & 0x1000000)	/* h <<= 1 (mod poly) */
			h ^= POLY24;
		for (j = 0; j < i; j++)
			table[i+j] = table[j] ^ h;
	}
}


/*
 * Update a CRC to reflect the addition of some more bytes.
 * It doesn't matter how the bytes are broken up; the CRC is the same.
 * I.e. crcUpdate(0, "abcd", 3) == crcUpdate(crcUpdate(0, "ab", 2), "cd", );
 */
PGPUInt32
crcUpdate(PGPUInt32 crc, PGPByte const *buf, unsigned len)
{
	static PGPUInt32 table[256];	/* Initialized to 0 */

	if (!table[1])
		crcInit(table);

	while (len--)
		crc = crc<<8 ^ table[*buf++ ^ (PGPByte)(crc>>16)];

	return crc & 0xffffff;
}
