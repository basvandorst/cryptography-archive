From msuinfo!uwm.edu!spool.mu.edu!bloom-beacon.mit.edu!usc!rand.org!mycroft.rand.org!not-for-mail Tue Nov 22 22:04:47 1994
Path: msuinfo!uwm.edu!spool.mu.edu!bloom-beacon.mit.edu!usc!rand.org!mycroft.rand.org!not-for-mail
From: Jim Gillogly <jim@acm.org>
Newsgroups: sci.crypt
Subject: Re: fastest hash function?
Date: 20 Nov 1994 15:28:18 -0800
Organization: Banzai Institute
Lines: 107
Sender: jim@mycroft.rand.org
Message-ID: <3aom2i$nkb@mycroft.rand.org>
References: <3am44e$bpu$1@mhadf.production.compuserve.com>
Reply-To: jim@acm.org
NNTP-Posting-Host: mycroft.rand.org

In article <3am44e$bpu$1@mhadf.production.compuserve.com>,
Bob Jenkins  <74512.261@CompuServe.COM> wrote:
>
>What is the fastest hash function?  I'm not asking as a request
>for information, I'm proposing it as a design exercise.
>
>To be more specific, it doesn't have to be a cryptographic hash 
>function.  But it still must satisfy these criteria:
> * biases of less than 1 part per 256
> * uniformly distributed
> * order dependent (the key "AaAa" is different from "aAaA")
> * total avalanche (changing one bit can affect every other bit)
> * produces a 32-bit hash value
>It should be in C, and keys are usually 16 bytes long.

As a benchmark (without claiming optimality) I suggest Zobrist Hashing.
Al Zobrist described it in a paper in 1970 and used it in his go program;
I used it in a chess program, an assembler, and many other applications.
The cost is N * (XOR + 2-d-table-lookup) for an N-byte key.  The feature
that makes it nice for game programming is that it's incremental, so that
for a chess position you don't need to do something to all 64 squares or
all 32 pieces to get the hash for that position.  If the target
application can make use of this property, Zobrist Hashing might indeed
be optimal.

For example (ob. crypto here), if you're hashing a dictionary in order,
you can get the hash for "aardvarks" with one table lookup and an XOR if
you remember the hash for "aardvark".

It requires setting up a 2-d table of randomish numbers, one for each byte
value (256) and for each position in the key string (typically 16 bytes for
the problem as posed above).  The hash is the XOR of the 32-bit numbers
corresponding to each key value.  For example, "Jim" would hash to

	zob['J'][0] ^ zob['i'][1] ^ zob['m'][2]

The randomish numbers should be pretty independent, and each bit in them
should be randomish -- e.g. not consecutive outputs from a linear
congruential generator.  Unix random() is a reasonable choice.  They may
be a fixed table that's gone through appropriate vetting; having equal
values is not a good thing.  To the extent that the numbers in the table
individually satisfy the criteria (no bias, uniform distribution,
independence) the hash itself will satisfy them.

Here's some C code to explain what I mean.

	Jim Gillogly
	Trewesday, 30 Blotmath S.R. 1994, 23:02

__________________________________snip____________________________________

/* Zobrist hashing - see "A new hashing method with application for game
 * playing", A. L.  Zobrist, U. Wis. Computer Sciences Department TR #88
 * (Apr 1970).
 *
 * Sample implementation Nov 94, J. Gillogly
 */

#include <stdio.h>

#define MAXKEY 20

typedef unsigned long ulong;
typedef unsigned char uchar;

ulong zob[256][MAXKEY]; /* 256 different bytes in each key position */

ulong hash(uchar *s);

main(int argc, char *argv[])
{
	int i, j;

	if (argc < 2)
	{
		fprintf(stderr, "Usage: zob <string>\n");
		return 1;
	}
	if (strlen(argv[1]) > MAXKEY)
	{
		fprintf(stderr, "Max key length: %d\n", MAXKEY);
		return 1;
	}

	srandom(0);     /* Can use the same random numbers each time */
	for (i = 0; i < 256; i++)
		for (j = 0; j < MAXKEY; j++)
			zob[i][j] = random();

	printf("%s hashes to %08x\n", argv[1], hash(argv[1]));
	return 0;
}

ulong hash(uchar *s)
{
	ulong h = 0;
	int i;

	for (i = 0; *s; i++)
		h ^= zob[*s++][i];

	return h;
}
__________________________________snip____________________________________
-- 
	Jim Gillogly
	Trewesday, 30 Blotmath S.R. 1994, 23:28

