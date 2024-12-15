/*+********************************************************
 * File name   :  BINDERV.CPP
 * Title       :
 * Project     :
 * Date created:  16 June, 1994
 * Revision    :  Rev 0.0
 * Author      :  C. C. Stevens
 * History     :  Rev 0.0  16-06-1994   start of coding
 *             :
 *
 *
 * Description : Randomness Testing
 *             : Based on aricle:
 *               'USING BINARY DERIVATIVES TO TEST AN ENHANCEMENT OF DES'
 *                Carrol, JM & Robbins, LE; Crypto
 *
 *             Results indicate: derivative p(i) close to 0.5 for random str
 *                               average         close to 0.5 for random str
 *                               range           low for random strings
 *
 *-*******************************************************/
#pragma option -v-  //no debug code

/*page*/
/************************INCLUDE FILES********************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <dir.h>
#include <math.h>

#define	BUFSIZE		8//256

struct BINDERIV
{
	unsigned long	nBits;         	// total number of bits
	unsigned long	nOneBits;		// one bits
	float			derivative;		// ones / number of bits
};

struct	RND
{
	char				fileName[MAXFILE + MAXEXT];
	unsigned long		fileSize;			// total bytes expected
	unsigned long		fileBitSize;		// total bits expected
	FILE				*fpBinD1;			// binary derivative fp
	FILE				*fpBinD2;			// binary derivative fp
	FILE				*fpBinD;			// binary derivative fp
	unsigned long		numberOfBytes;		// input byte counter
	unsigned char		binDerivative;		//binary derivative
	unsigned char		binDerivativeCount;	// nr of bits in binDerivative
	unsigned char		lastBit;			// last bit seen, ie 0 or 1
	unsigned long		numberOfBits;		// total number of bits
	unsigned long		onesCounter;		// running counter
	unsigned long		zerosCounter;		// running counter
	short				binP;				// derivative number
	BINDERIV			bins[50];			// all binary dervative counts
	short				numberOfBinD;		// to what derivative do we want
};

static	int		readFile(FILE *fpin, char *inBuf);
static	void	convert(FILE *fpin, RND *stats);
static	void	processInBuffer(char *inBuf, int count, RND *stats);
static	void	reportResults(RND *statsCounter);

/*page*/
/*+********************************************************
 *
 *  Name       : main
 *
 *  Function   : Perform tests by getting command line
 *
 *  Parameters : argv - Command line arguments
 *               argv[1]    = input file name
 *               argv[2]	= number of derivatives to be done
 *                            (1 to 50)
 *  Note       : Nuber of derivatives limited to 50 (do malloc to alloacte
 *               memory for structure - one day)
 *
 *-*******************************************************/
void	main(int argc, char *argv[])
{
	FILE	*fpin;
	RND		statsCounter;
	char	drive[MAXDRIVE];
	char	dir[MAXDIR];
	char	file[MAXFILE];
	char	ext[MAXEXT];
	char	path1[MAXPATH];
	char	path2[MAXPATH];
	short	i;

//..initialise statistics counter
	memset(&statsCounter, 0, sizeof(RND));
	statsCounter.lastBit	= 0xff;

//..get input and output file names - identify what we are doing
	printf("\n --- Binary Derivative Tester. Version 0.0 ---\n");
	if (argc < 2)
	{
		printf("\nUSAGE: <input filename> [number derivatives]\n");
		exit (-1);
	}
	if ((fpin	= fopen(argv[1], "rb")) == NULL)
	{
		printf("Cannot open file: %s\n", argv[1]);
		exit (-1);
	}
	strcpy(statsCounter.fileName, argv[1]);
	statsCounter.fileSize	= filelength(fileno(fpin));
	statsCounter.fileBitSize= statsCounter.fileSize * 8;

	fnsplit(argv[1], drive, dir, file, ext);
	sprintf(ext, ".$$1");
	fnmerge(path1, drive, dir, file, ext);
	if ((statsCounter.fpBinD1 = fopen(path1, "wb+")) == NULL)
	{
		printf("Cannot open file: %s for Bin Deriv\n", path1);
		exit (-1);
	}

	sprintf(ext, ".$$2");
	fnmerge(path2, drive, dir, file, ext);
	if ((statsCounter.fpBinD2 = fopen(path2, "wb+")) == NULL)
	{
		printf("Cannot open file: %s for Bin Deriv\n", path2);
		exit (-1);
	}
	if (argc == 3)
	{
		i = atoi(argv[2]);
		if (i > 0 && i < 50)
			statsCounter.numberOfBinD	= i;
		else
			statsCounter.numberOfBinD	= 8;
	}
	else
		statsCounter.numberOfBinD	= 8;		// default number

	printf("Busy to derivative %d.......", statsCounter.numberOfBinD);

	convert(fpin, &statsCounter);
	reportResults(&statsCounter);
	fcloseall();
	remove(path1);								//delete temporary files
	remove(path2);
	exit(0);
}

/*page*/
/*+********************************************************
 *
 *  Name       : convert
 *
 *  Function   :
 *
 *  Parameters : fpin  - file pointer to open file for read only
 *				 stats - structure pointer for accumulated data
 *
 *
 *  Note       :
 *
 *-*******************************************************/
static	void	convert(FILE *fpin, RND *stats)
{
	char	inBuffer[BUFSIZE];
	char	outBuffer[BUFSIZE];
	int		count;

//..do for all derivatives
	for (stats->binP = 0; stats->binP < stats->numberOfBinD; stats->binP++)
	{
//......swop filename on each alternate derivative
		if (stats->binP == 0)						//first derivative
		{
			stats->fpBinD	= stats->fpBinD1;		//default to start
		}
		else if ((stats->binP % 2) != 0)			// odd derivative number
		{
			fpin			= stats->fpBinD1;
			stats->fpBinD	= stats->fpBinD2;
		}
		else										//even derivative number
		{
			fpin			= stats->fpBinD2;
			stats->fpBinD	= stats->fpBinD1;
		}
//......ensure start at beginning of file
		rewind(fpin);
		rewind(stats->fpBinD);
//......clear all counters
		stats->numberOfBytes	= 0;
		stats->numberOfBits		= 0;
		stats->onesCounter		= 0;
		stats->zerosCounter		= 0;
		stats->lastBit 			= 0xff;
		stats->binDerivative	= 0;
		stats->binDerivativeCount= 0;;
//......expected number of bits
		stats->bins[stats->binP].nBits	= stats->fileBitSize - stats->binP;

		while (!feof(fpin))
		{
			count	= readFile(fpin, &inBuffer[0]);
			if (count)
				processInBuffer(&inBuffer[0], count, stats);
		}
		stats->bins[stats->binP].nOneBits= stats->onesCounter;
	}
}

/*page*/
/*+********************************************************
 *
 *  Name       : readFile
 *
 *  Function   : reads a byte from input file
 *
 *  Parameters : fpin - file pointer
 *               inBuf- pointer to byte storage of input
 *
 *
 *  Note       :
 *
 *-*******************************************************/
static	int		readFile(FILE *fpin, char *inBuf)
{
	int		count = 0;

	count	= fread(inBuf, 1, 1, fpin);
	return	count;
}

/*page*/
/*+********************************************************
 *
 *  Name       : processInBuffer
 *
 *  Function   : process bytes in buffer
 *
 *  Parameters : inBuf  - pointer to input buffer
 *               count  - number of bytes in input buffer
 *               stats  - pointer to accumulator structure
 *
 *  Note       : As bin derivatives drop a bit on each round,
 *               write full byte to disk but filled with lastBit.
 *
 *-*******************************************************/
static	void	processInBuffer(char *inBuf, int count, RND *stats)
{
	int		i, j;
	char	c;

	for (i = 0; i < count; i++)
	{
		stats->numberOfBytes++;

		c	= *(inBuf + i);
		for (j = 0; j < 8; j++)
		{
			if (c & 0x80)
			{
				stats->onesCounter++;
				if (stats->lastBit != 0xff)
				{
					stats->binDerivative	= stats->binDerivative << 1;
					stats->binDerivativeCount++;

					if (stats->lastBit == 0)
						stats->binDerivative |= 1;
				}
				stats->lastBit	= 1;
			}
			else
			{
				stats->zerosCounter++;
				if (stats->lastBit != 0xff)
				{
					stats->binDerivative	= stats->binDerivative << 1;
					stats->binDerivativeCount++;

					if (stats->lastBit == 1)
						stats->binDerivative |= 0x01;
				}
				stats->lastBit	= 0;
			}
			stats->numberOfBits++;

			c = c << 1;

			if (stats->binDerivativeCount == 8)
			{
				fwrite(&stats->binDerivative, 1, 1, stats->fpBinD);
				stats->binDerivative		= 0;
				stats->binDerivativeCount	= 0;
//..............on last byte of file, check when to stop the bin derivative
				if (stats->numberOfBits >= stats->bins[stats->binP].nBits)
					return;
			}
//..........on last byte of file, check when to stop the bin derivative
			else if (stats->numberOfBits >= stats->bins[stats->binP].nBits)
			{
//..............fill rest of buffer with last bit to cancel a derivative
				while (stats->binDerivativeCount != 8)
				{
					stats->binDerivative	= stats->binDerivative << 1;
					stats->binDerivativeCount++;

					if (stats->lastBit)
						stats->binDerivative	|= 1;
				}
				fwrite(&stats->binDerivative, 1, 1, stats->fpBinD);
				stats->binDerivative		= 0;
				stats->binDerivativeCount	= 0;
				return;				//nothing more do even if j != 8
			}
		}
	}
}

/*page*/
/*+********************************************************
 *
 *  Name       : reportResults
 *
 *  Function   : output accumulated results
 *
 *  Parameters : s - pointer to accumulator
 *
 *
 *
 *  Note       :
 *
 *-*******************************************************/
static	void	reportResults(RND *s)
{
	float		minMax, sum, min, max;
	int		p;

	printf("\nResults of file : %s at Byte Count: %lu of %lu\n",
			s->fileName, s->numberOfBytes, s->fileSize);
	printf("\nExpect p(i) and average to be 0.500, Range near 0.00\n");

	printf("\n  --- BINARY DERIVATIVE TEST TO DERIVATIVE %d ----", s->numberOfBinD);
	printf("\n");

	sum	= 0.0;
	min = float(s->numberOfBytes) * 8;			// impossible value
	max = 0.0;
	for (p = 0; p < s->numberOfBinD; p++)
	{
		s->bins[p].derivative 	= (float)(s->bins[p].nOneBits) / (float)(s->bins[p].nBits);
		sum					   += s->bins[p].derivative;
		printf("\np(%d) = %f   Ones : %lu, Total Bits: %lu", p,
		s->bins[p].derivative, s->bins[p].nOneBits, s->bins[p].nBits);
		if (s->bins[p].derivative > max)
			max	= s->bins[p].derivative;
		if (s->bins[p].derivative <= min)
			min	= s->bins[p].derivative;
	}
	printf("\nAverage   : %f", sum / (float)(p));
	printf("\nRange     : %f", (max - min));
	printf("\n");
	printf("\f");
}
