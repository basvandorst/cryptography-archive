/*+********************************************************
 * File name   :  RNDTST.CPP
 * Title       :  Statistical Randomness Tester
 * Project     :  COS497
 * Date created:  16 June, 1994
 * Revision    :  Rev 0.1
 * Author      :  C. C. Stevens
 * History     :  Rev 0.0  16-06-1994   start of coding
 *             :  Rev 0.1   development
 *
 * Description : Randomness Testing
 *             : Tests based on:
 *                1) P Boucher posting for chi-square testing
 *                2) CIPHER SYSTEMS: Beker, H. & Piper, F.
 *                3) Seminumerical Algorithms: Knuth, DE. Vol 2
 *
 * NOTE:       : Link with gamma.obj
 *-*******************************************************/
//#pragma option -v-  kill debug info

/*page*/
/************************INCLUDE FILES********************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <dir.h>
#include <math.h>
#include "gamma.h"

#define	BUFSIZE		8//256
#define BYTESIZE 	256L
#define aRunLimit	128
#define min_nps 5

unsigned long 	cnt[BYTESIZE] = {0}; /* should be all zeros. */
typedef unsigned long *longptr;
longptr			*succeed;

struct	RND
{
	char				fileName[MAXFILE + MAXEXT];
	unsigned long		fileSize;			// total bytes expected
	unsigned int		options;
	unsigned long		numberOfBytes;		// input byte counter
	unsigned char		lastBit;			// last bit seen, ie 0 or 1
	unsigned long		numberOfBits;		//total number of bits
	unsigned long		onesCounter;		//running counter
	unsigned long		zerosCounter;		//running counter
	unsigned long		pattern00Counter;	//running counter
	unsigned long		pattern01Counter;	//running counter
	unsigned long		pattern10Counter;	//running counter
	unsigned long		pattern11Counter;	//running counter
	unsigned long		bSize;
	unsigned long		blocks[aRunLimit];	// runs test - runs of ones result
	unsigned long		gaps[aRunLimit];	// runs test - runs of zeros
	unsigned long		blockCount;			// runs test - runs of ones count
	unsigned long		gapCount;			// runs test - runs of zeros count
};

static	int		readFile(FILE *fpin, char *inBuf);
static	void	convert(FILE *fpin, RND *stats);
static	void	processInBuffer(char *inBuf, int count, RND *stats);
static	void	reportResults(RND *statsCounter);
static	unsigned long fillArrays(FILE *fpin);
static	float	get_V(unsigned long N, unsigned long *Y);
static	float	pokerTest(RND *s, short m);

/*page*/
/*+********************************************************
 *
 *  Name       : main
 *
 *  Function   : Perform tests by getting command line
 *
 *  Parameters : argv - Command line arguments
 *               argv[1]    = input file name
 *
 *  Note       :
 *
 *-*******************************************************/
void	main(int argc, char *argv[])
{
	FILE	*fpin, *fpout;
	int		args, i, j;
	RND		statsCounter;
	char	drive[MAXDRIVE];
	char	dir[MAXDIR];
	char	file[MAXFILE];
	char	ext[MAXEXT];
	char	path[MAXPATH];
	long	length;

//..initialise statistics counter
	memset(&statsCounter, 0, sizeof(RND));
	statsCounter.lastBit	= 0xff;

//..get input and output file names - identify what we are doing
	printf("\n --- `Randomness' Tester. Version 0.3 ---\n");
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
	strcpy(statsCounter.fileName, argv[1]);
	statsCounter.fileSize	= filelength(fileno(fpin));

	succeed	= new longptr[BYTESIZE];
	for (i = 0; i < BYTESIZE; i++)
		succeed[i]	= new unsigned long[BYTESIZE];

	for (i = 0; i < BYTESIZE; i++)
		for (j = 0; j < BYTESIZE; j++)
			succeed[i][j]	= 0;

	convert(fpin, &statsCounter);
	reportResults(&statsCounter);
	fcloseall();

//..release memory used
	for (i = 0; i < BYTESIZE; i++)
		delete	succeed[i];
	delete succeed;

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

//..do Bouchers chi sq data gathering
	stats->bSize = fillArrays(fpin);
	rewind(fpin);

	while (!feof(fpin))
	{
		count	= readFile(fpin, &inBuffer[0]);
		processInBuffer(&inBuffer[0], count, stats);
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
 *  Note       :
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
					if (stats->lastBit == 0)
					{
						stats->pattern01Counter++;
					}
					else
						stats->pattern11Counter++;

//					stats->lastBit	= 0xff;	// uncomment for Knuth
				}
//				else	// uncomment as per Knuth's version of serial test
					stats->lastBit	= 1;

				stats->blockCount++;
				if (stats->gapCount >= aRunLimit)
					stats->gaps[0]	= 1;	// set error (never have a 0 run)
				else if (stats->gapCount)	// don't update [0] position
					stats->gaps[stats->gapCount]++;

				stats->gapCount	= 0;		// reset count as a one occurred
			}
			else
			{
				stats->zerosCounter++;
				if (stats->lastBit != 0xff)
				{
					if (stats->lastBit == 1)
					{
						stats->pattern00Counter++;
					}
					else
						stats->pattern10Counter++;

//					stats->lastBit	= 0xff;
				}
//				else	// uncomment as per Knuth's version of serial test
					stats->lastBit	= 0;

				stats->gapCount++;
				if (stats->blockCount >= aRunLimit)
					stats->blocks[0]	= 1;	// set error (never have a 0 run)
				else if (stats->blockCount)
					stats->blocks[stats->blockCount]++;

				stats->blockCount = 0;
			}

			c = c << 1;
		}
	}
	if (count == 0)
	{
		if (stats->blockCount)
			stats->blocks[stats->blockCount]++;
		if (stats->gapCount)	// don't update [0] position
			stats->gaps[stats->gapCount]++;
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
 *  Note       : gammaq probabilty - small value indicates a
 *               significant difference.
 *
 *-*******************************************************/
static	void	reportResults(RND *s)
{
	float		freqTest, rMean, rVar, z, rChi;		// chi sq test
	float		serialTest, r00, r01, r10, r11, sMean;	// chi sq test
	float		temp, ftf, ratio, poker;
	char		abuffer[255];
	int			i, j;
	float		V;
	unsigned long	totalBlocks, totalGaps, totalRuns, expBlock, actBlock;

	totalBlocks		= 0;
	totalGaps		= 0;
	s->numberOfBits	= s->onesCounter + s->zerosCounter;
	freqTest		= fabs(float(s->zerosCounter) - float(s->onesCounter));

	ratio			= freqTest / (float)(s->numberOfBits);
	freqTest	   	= (freqTest * freqTest) / (float)(s->numberOfBits);
	ftf				= (float)(s->onesCounter) - ((float)(s->numberOfBits) / 2.0);
	ftf				= (ftf * 2.0) / sqrt((float)(s->numberOfBits));

	serialTest		= (float)(s->pattern00Counter) * (float)(s->pattern00Counter);
	serialTest	   += (float)(s->pattern01Counter) * (float)(s->pattern01Counter);
	serialTest	   += (float)(s->pattern10Counter) * (float)(s->pattern10Counter);
	serialTest	   += (float)(s->pattern11Counter) * (float)(s->pattern11Counter);
	serialTest	    = (serialTest * 4.0) / (float)(s->numberOfBits - 1.0);
	temp			= (float)(s->onesCounter) * (float)(s->onesCounter);
	temp		   += (float)(s->zerosCounter) * (float)(s->zerosCounter);
	temp		    = (temp * 2.0) / (float)(s->numberOfBits);
	serialTest		= serialTest - temp + 1.0;
	sMean			= (float)(s->numberOfBits - 1) / 4.0;
	r00				= float(s->pattern00Counter) - sMean;
	r00			   *= r00;
	r01				= float(s->pattern01Counter) - sMean;
	r01			   *= r01;
	r10				= float(s->pattern10Counter) - sMean;
	r10			   *= r10;
	r11				= float(s->pattern10Counter) - sMean;
	r11			   *= r11;
	temp			= ((r00 + r01 + r10 + r11) * 4.0) / (float)(s->numberOfBits);
	rMean			= 1.0 + ((2 * (float)(s->zerosCounter) * (float)(s->onesCounter)) /(float)(s->numberOfBits));
	rVar			= ((rMean - 1.0) * (rMean - 2.0)) / (float)(s->numberOfBits - 1);
	poker			= pokerTest(s, 8);

	printf("\nResults of file : %s at Byte Count: %lu of %lu\n",
			s->fileName, s->numberOfBytes, s->fileSize);

	printf("\n  --- FREQUENCY TEST ----");
	printf("\nNumber Of Bits  : %lu", s->numberOfBits);
	printf("\nNumber Of Ones  : %lu, Ratio : %f", s->onesCounter,
			((float)(s->onesCounter) / (float)(s->numberOfBits)));
	printf("\nNumber Of Zeros : %lu, Ratio : %f", s->zerosCounter,
			((float)(s->zerosCounter) / (float)(s->numberOfBits)));
	printf("\nDifference Ratio: %f  %.2f%%", ratio, ratio * 100.0);
	printf("\nNormal distribtn: %.3f", ftf);
	printf("\nChi sq Freq Test: %.3f with 1 degree of freedom, probability of %.2f", freqTest, gammaq(0.5, (float)(freqTest)/2.0));
	printf("\nChi Square      : ");

	if (s->bSize < (BYTESIZE*min_nps))
		printf("File too small (%ld) to analyze for 255 degress of freedom.", s->bSize);
	else
	{
		V = get_V(s->bSize, cnt);
		printf("%.2f with 255 degrees of freedom, probability of %.2f", V, gammaq(127.5, V/2.0));
	}
	printf("\nSuccessor chi   : ");
//	if ((unsigned long)(s->bSize) >= (BYTESIZE*BYTESIZE*min_nps))
	{
		for (i=0,V=0.0; i<BYTESIZE; i++)
		{
			V += get_V(cnt[i],succeed[i]);
		}
		V /= BYTESIZE;
		printf("%.2f with 255 degrees of freedom, probability of %.2f", V, gammaq(127.5, V/2.0));
	}
	printf("\n");

	printf("\n  --- SERIAL TEST ----");
	printf("\nNumber Of 00    : %lu", s->pattern00Counter);
	printf("\nNumber Of 01    : %lu", s->pattern01Counter);
	printf("\nNumber Of 10    : %lu", s->pattern10Counter);
	printf("\nNumber Of 11    : %lu", s->pattern11Counter);
	printf("\nExpected Mean   : %.3f", sMean);
	printf("\nChi sq Test     : %.3f with 2 degrees of freedom, probability of %.2f", serialTest, gammaq(1.0, (float)(serialTest)/2.0));
	printf("\nChi sq Test     : %.3f with 3 degrees of freedom, probability of %.2f", temp, gammaq(1.5, (float)(temp)/2.0));
	printf("\n");

	printf("\n  --- POKER TEST ----");
	printf("\nChi-square, m=8 : %.3f with 255 degrees of freedom, probability of %.2f", poker, gammaq(127.5, poker/2.0));
	printf("\n");

	printf("\n  --- RUNS TEST ----");
	for (i = 1; i < aRunLimit; i++)
	{
		totalBlocks	+= s->blocks[i];
		totalGaps	+= s->gaps[i];
		if (s->blocks[i] == 0 && s->gaps[i] == 0)
			continue;
		printf("\nRun length %.2d -> Blocks: %.5lu   Gaps: %.5lu", i, s->blocks[i], s->gaps[i]);
	}
	totalRuns	= totalBlocks + totalGaps; // gives for length 1
	z			= fabs((float)(totalRuns) - rMean) / sqrt(rVar);
	printf("\nTotals        -> Blocks: %.5lu + Gaps: %.5lu = %lu runs", totalBlocks, totalGaps, totalRuns);
	printf("\n");

	printf("\nMean            : %.3f", rMean);
	printf("\nVar             : %.3f", rVar);
	printf("\nz-score         : %.3f", z);
	if (s->blocks[0] || s->gaps[0])			// check if run exceeded 63
		printf(" ** ERROR - run length > %d (missing counts) **", aRunLimit);
	printf("\n");
//..test Golomb's postulate R2, ie half the runs will have length 1, etc
	rChi	= 0.0;
	for (j = 1; j < i; j++)
	{
		if (s->blocks[j] < 5 || s->gaps[j] < 5)
			break;			// can get boring
		expBlock	= totalBlocks >> j;
		actBlock	= s->blocks[j];
		temp		= fabs((float)(actBlock) - (float)(expBlock));
		rChi	   += (temp * temp) / (float)(expBlock);
		printf("\nExpected block length %.2d: %.5lu, is: %.5lu | expected gap %.5lu, is %.5lu", j, expBlock, actBlock, totalGaps>>j, s->gaps[j]);
	}
	printf("\nGolomb's R2  Chi-square : %.3f with %d degrees of freedom, probability of %.2f", rChi, j-2, gammaq((float)(j-2)/2.0, rChi/2.0));
	printf("\n");

	printf("\f");
}

 /*
 * Copyright 1993 Peter K. Boucher
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.*/
static	float	get_V(unsigned long N, unsigned long *Y)
{
#define k (BYTESIZE)
#define p (1.0/k)
	float 	sum = 0.0;
	float 	n = N;
	int 	i;

	if (n == 0.0)
		return sum;

	for (i=0; i<k; i++)
	{
		sum += ((Y[i]*Y[i])/p)/n;
	}
	return( sum - n );
}

static	unsigned long fillArrays(FILE *fpin)
{
	unsigned long size=0L;
	int ch,next,i;

	if ((ch = getc(fpin)) != EOF)
	{ /* prime the pump */
		cnt[ch] = size = 1L;
		while (!feof(fpin))
		{
			if ((i = getc(fpin)) == EOF)
				break;
			size++;
			next = i;
			cnt[next]++;
			succeed[ch][next]++;
			ch = next;
		}
	}
	return( size );
}

/*page*/
/*+********************************************************
 *
 *  Name       : pokerTest
 *
 *  Function   : Poker Test
 *
 *  Parameters : s - pointer to accumulator
 *
 *
 *  Note       :
 *
 *-*******************************************************/
static	float	pokerTest(RND *s, short m)
{
	int		i, size;
	float	poker;
	unsigned long F, f, fsq, pokerF;

	F		= 0;
	f		= 0;
	fsq		= 0;
	pokerF	= s->numberOfBits / m;
	size	= pow(2, m);

	for (i = 0; i < size; i++)
	{
		if (f <= pokerF)				//pokerF is actually filesize if m=8
			F	= f;					// F <= (n/m)

		f  += cnt[i];
		fsq+= cnt[i] * cnt[i];
	}
	if (f <= pokerF)
		F	= f;					// F <= (n/m)

	poker	= ((float)(size) / (float)F) * (float)fsq - (float)F;
	return poker;
}
