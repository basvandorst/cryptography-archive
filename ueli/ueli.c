
/*
From honig@sprynet.com Wed Nov 11 10:37:58 1998
Date: Sat, 26 Sep 1998 16:27:49 -0700
From: David Honig <honig@sprynet.com>
To: cypherpunks@toad.com, coderpunks@toad.com
Subject: Ueli Maurer's Univ. Stat. Test for Rand No -in C
*/

/*
UELI.c

This implements Ueli M Maurer's
"Universal Statistical Test for Random Bit Generators"
using L=16

Accepts a filename on the command line;
writes its results, with other info, to stdout.

Handles input file exhaustion gracefully.

Ref: J. Cryptology v 5 no 2, 1992 pp 89-105
also on the web somewhere.

-David Honig
honig@sprynet.com


Built with Wedit 2.3, lcc-win32
http://www.cs.virginia.edu/~lcc-win32


26 Sept CP Release
Version Notes:

	This version does L=16.  It evolved from an L=8 prototype
	which I ported from the Pascal in the above reference.

	I made the memory usage reasonable
	by replacing Maurer's "block" array
	with the 'streaming' fgetc() call.


Usage:
	UELI filename
	outputs to stdout

*/

#define L 16		// bits per block
#define V (1<<L)	// number of possible blocks
#define Q (10*V)    // at LEAST 10 * V, to assure each block seen
#define K (100*Q)   // at LEAST 100 * Q, as large as possible
#define MAXSAMP (Q + K)

#include <stdio.h>
#include <math.h>


int main( int argc, char **argv )
{
FILE *fptr;
int i;
int b, c;
int table[V];
float sum=0.0;
int run;

// Human Interface
printf("UELI 26 Sep 98\nL=%d %d %d \n", L, V, MAXSAMP);
if (argc <2)
	{printf("Usage: UELI filename\n"); exit(-1); }
else
	printf("Measuring file %s\n", argv[1]);

// FILE IO
fptr=fopen(argv[1],"rb");
if (fptr == NULL) {printf("Can't find %s\n", argv[1]); exit(-1); }

// INIT
for (i=0; i<V; i++) table[i]=0;
for (i=0; i<Q; i++)	{
	b= fgetc(fptr)<<8 | fgetc(fptr);
	table[ b ]=i;
}

printf("Init done\n");

// COMPUTE
run=1;
for (i=Q;  run && i<Q+K; i++)
	{
	// COMPOSE A 16-bit quantity
	b=fgetc(fptr); if (b<0) run=0;
	c=fgetc(fptr); if (c<0) run=0;
	if (run) { b = b<<8 | c;
			sum += log( (double) ( i-table[b] ) ) ;
			table[ b ]=i;
		}
	}

	if (!run) printf("Premature end of file; read %d blocks.\n", i-Q);
	sum = (sum/( (double) (i-Q) ) ) /  log(2.0);    // i should be K if enough samples
	printf("fTU= %f\n\n", sum);
	printf("Expected value for L=16 is 15.167379 \n");

	// Add further interpretation/thresholding of the number of sigmas from expected,
	// and include the fudge factors explained in the paper.




} // end
