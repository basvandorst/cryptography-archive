#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Maurer's Universal test for Random bits     *
// C. Stevens  cstevens@iaccess.za
// References:
* Journal of Cryptology, Vol. 5 1992 pp89-105
* and
* posting by Mike Scott on sci.crypt */

#define Q 		2000L
#define K 		20000L
#define MEAN 	7.1836656

static	FILE			*fpin;
static unsigned char 	getblock();

static	unsigned int	q = Q;
static	unsigned long	k;

int 	ubtest()
{ /* Universal Test for Randomness */
	double 	sum, ftu, deviation;
	int 	i;
	long 	n;
	static long tab[256];

	for (i=0; i < 256; i++)
		tab[i]	= (-1);
	for (n = 0; n <= q; n++)
		tab[getblock()]	= n;
/* check each byte occurred at least once */
	for (i = 0; i < 256; i++)
	{
		if (tab[i] < 0)
		{
			printf("\nEach block has not occurred during initialisation!");
			return 0;
		}
	}

	sum	= 0.0;
	for (n = Q+1; n <= Q+k; n++)
	{ /* scan byte sequence */
		i		= getblock();
		sum	   += log((double)n - tab[i]);
		tab[i] 	= n;
	}
	ftu			= ((sum / (double)k) / log(2.0));
	deviation	= 1.5 * sqrt(3.238 / (double)k);
	printf("\nThreshold t = %lf, DEV = %lf\n", ftu-MEAN, deviation);
	if (ftu > (MEAN+deviation) || ftu < (MEAN-deviation))
		return 0;
	return 1;
}

static unsigned char 	getblock()
{ /* random bit generator. Pack bits into byte */
	if (feof(fpin))
	{
		printf("Out of data");
		exit(-1);
	}
	return (unsigned char)(fgetc(fpin));
}

void	main(int argc, char *argv[])
{
	unsigned long	fsize;

	printf("\n  ---- Maurer's Universal Test  Ver 0.1 ----\n");

	if (argc < 2)
	{
		printf("\nUSAGE: <input filename>\n");
		exit (-1);
	}

	if ((fpin	= fopen(argv[1], "rb")) == NULL)
	{
		printf("Cannot open file: %s\n", argv[1]);
		exit (-1);
	}

	fsize	= filelength(fileno(fpin));
	if (fsize < (Q + K))
	{
		printf("\nFile too short! Require %lu bytes", Q+K);
		fclose(fpin);
		exit (-1);
	}
	k	= fsize - Q - 1;		// adjust so we test whole file

	printf("Results of file: %s, length %lu, L = 8, Q = %lu, K = %lu", argv[1], fsize, Q, k);

	if (ubtest())
		printf("\nWithin threshold limits\n");
	else
		printf("\nFail\n");

	printf("\r");
}

