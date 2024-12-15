/* ***************************************************************
 * anal.c --
 *
 * Copyright 1993 Peter K. Boucher
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.
 *
 * Usage:  anal [input_file [output_file]]
 *
 * This program counts the occurances of each character in a file
 * and notifies the user when a the distribution is too ragged or
 * too even.
 *
 * Because the chance of getting byte B after byte A should be 1:256
 * (for all A's and B's), the program also checks that the successors
 * to each byte are randomly distributed.  This means that for each byte
 * value (0 - 255) that occurs in the text, a count is kept of the
 * byte value that followed in the text, and the frequency distribution
 * of these succeeding bytes is also checked.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>

#define BYTESIZE 256
#define BUFSIZE 8192
#ifdef DEBUG
#define PASSED_OFNAME "analocc.pss"
#define PASSED_SFNAME "analsuc.pss"
#define FAILED_OFNAME "analocc.fld"
#define FAILED_SFNAME "analsuc.fld"
#endif

#define	Vmin	(205.33) /*  1% chance it's less */
#define Vlo	(239.39) /* 25% chance it's less */
#define Vhi	(269.88) /* 75% chance it's less */
#define Vmax	(310.57) /* 99% chance it's less */

#define min_nps 5

#define SHOW_RESULT(F,t,s) \
	 fprintf(F, "%s n =%10ld, V=%.2lf\n", \
		 t,    s,       V);

unsigned long cnt[BYTESIZE] = {0}; /* should be all zeros. */
//unsigned long succeed[BYTESIZE][BYTESIZE] = {{0}}; /* should be all zeros. */
typedef unsigned long *longptr;
longptr	*succeed;

//static unsigned char buf[BUFSIZE];
static FILE *ifp, *ofp;

FILE * 	my_fopen(char *file, char *type)
{
	FILE *fp;

	if ((fp = fopen(file, type)) == NULL)
	{
		(void)fprintf(stderr, "Can't open '%s' for '%s'\n", file, type);
		exit(-1);
	}
	return(fp);
}

double	get_V(unsigned long N, unsigned long *Y)
{
#define k (BYTESIZE)
#define p (1.0/k)
	double sum = 0.0;
	double n = N;
	int i;

	if (n == 0.0)
		return sum;

	for (i=0; i<k; i++)
	{
		sum += ((Y[i]*Y[i])/p)/n;
	}
	return( sum - n );
}

unsigned long fill_arrays()
{
	unsigned long size=0L;
	int ch,next,i;

	if ((ch = getc(ifp)) != EOF)
	{ /* prime the pump */
		cnt[ch] = size = 1L;
		while (!feof(ifp))
		{
			if ((i = getc(ifp)) == EOF)
			//if (fread(buf, 1, 1, ifp) == 0)
				break;
			size++;
			next = i;//buf[i];
			cnt[next]++;
			succeed[ch][next]++;
			ch = next;
		}
	}
	fclose(ifp);
	return( size );
}

void	anal_ize_text()
{
	int   	  i;
	double         V;
	unsigned long  size;

	if ((size = fill_arrays()) < (BYTESIZE*min_nps))
	{
		fprintf(stderr, "File too small (%ld) to meaningfully analyze\n",size);
		exit(0);
	}

	V = get_V(size,cnt);
	SHOW_RESULT(ofp, "Occurances: ", size);
	if ((V < Vmin) || (V > Vmax))
	{
#ifdef DEBUG
	   FILE *failed=my_fopen(FAILED_OFNAME, "a");
	   SHOW_RESULT(failed, "", size);
	   fclose(failed);
#endif
	   printf("Character occurances non-random\n");
	}
	else if ((V > Vlo) && (V < Vhi))
	{
#ifdef DEBUG
	   FILE *excell=my_fopen(PASSED_OFNAME, "a");
	   SHOW_RESULT(excell, "", size);
	   fclose(excell);
#endif
	   fprintf(ofp,
"================ Frequency distribution excellent! ====================\n");
	}

	if ((unsigned long)size >= (BYTESIZE*BYTESIZE*min_nps))
	{
		for (i=0,V=0.0; i<BYTESIZE; i++)
		{
			V += get_V(cnt[i],succeed[i]);
		}
		V /= BYTESIZE;
		SHOW_RESULT(ofp, "Successions:", size>>8);
		if ((V < Vmin) || (V > Vmax))
		{
#ifdef DEBUG
			FILE *failed=my_fopen(FAILED_SFNAME, "a");
			SHOW_RESULT(failed, "", size>>8);
			fclose(failed);
#endif
			printf("Character successions non-random\n");
		}
		else if ((V > Vlo) && (V < Vhi))
		{
#ifdef DEBUG
			FILE *excell=my_fopen(PASSED_SFNAME, "a");
			SHOW_RESULT(excell, "", size>>8);
			fclose(excell);
#endif
		fprintf(ofp,
"================= Successor randomness excellent! =====================\n");
		}
	}
}

int	main (int argc, char* argv[])
{
	short i, j;

	ifp = (argc > 1) ? my_fopen(argv[1],"rb") : stdin;
	ofp = (argc > 2) ? my_fopen(argv[2],"wb") : stdout;
	succeed	= new longptr[BYTESIZE];
	for (i = 0; i < BYTESIZE; i++)
		succeed[i]	= new unsigned long[BYTESIZE];

	for (i = 0; i < BYTESIZE; i++)
		for (j = 0; j < BYTESIZE; j++)
			succeed[i][j]	= 0;

	printf("\nPeter Boucher - Chi square test on file: %s\n", argv[1]);
	anal_ize_text();

	for (i = 0; i < BYTESIZE; i++)
		delete	succeed[i];
	delete succeed;

   return(0);
}



