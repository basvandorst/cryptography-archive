/*
 *  Solve Vigenere ciphers by frequency analysis
 *  Program written by Fauzan Mirza, 13 Sept 1995
 *    18 Sept 1995 - Bug fixed
 *
 *  Based on the BSD caesar program by Stan King and John Eldridge
 *
 *  For an explanation of how and why this method works refer to:
 *  Abraham Sinkov, "Elementary Cryptanalysis", Random House, 1968.
 *
 *  Usage: vigsolve < CipherTextFile
 *    Vigenere ciphertext is taken from STDIN (may need to pipe it in)
 *
 *  Added Dec 1995, Index of Coincidence, plus two additional methods,
 *  giving keys for Vigenere, Beaufort and Variant Beaufort. Ernest Brandt
 *
 */

#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef __unix__
#include <unistd.h>
#else
#include <dos.h>
#define STDIN_FILENO 0
#endif

#define KEYLENGTH 15
#define BUFFERLEN 4096

/* Letter frequencies (from BSD caesar) */

double stdf[26] = {
	7.97, 1.35, 3.61, 4.78, 12.37, 2.01, 1.46, 4.49, 6.39, 0.04,
	0.42, 3.81, 2.69, 5.92,  6.96, 2.91, 0.08, 6.63, 8.77, 9.68,
	2.62, 0.81, 1.88, 0.23,  2.07, 0.06,
};

void main()
{
	int freq[KEYLENGTH][26];
	int num[BUFFERLEN];
	char buf[BUFFERLEN];
	char key1[KEYLENGTH], key2[KEYLENGTH], key3[KEYLENGTH];
	int buflen, len, try, lets;
	double dot,  winnerdot;
	double dot3, winnerdot3;
	double icsum, ic;
	double lsum, nsum;
	int i, j;

	printf("\nIndex of Coincidence and Periodic Key Find\n\n");

	buflen=read(STDIN_FILENO, buf, BUFFERLEN);

	lets = 0;
	for (i=0; i<buflen; i++)
	{
		if (!isalpha(buf[i]))
		    continue;

		num[lets]=tolower(buf[i])-'a';
		lets++;
	}

	printf("%u Characters read, %u Alphas\n\n", buflen, lets);

	for (i=0; i<26; i++)
		stdf[i] = log(stdf[i]) + log(26.0 / 100.0);

	printf("Len     Index    %-14s   %-14s   %-14s\n",
	       "Vigenere", "Var. Beaufort", "Beaufort");

	printf("---     ------   %-14s   %-14s   %-14s\n",
	       "c = p+k", "c = p-k", "c = k-p");

	printf("                 %-14s   %-14s   %-14s\n",
	       "p = c-k", "p = c+k", "p = k-c");

	printf("                 %-14s   %-14s   %-14s\n",
	       "k = c-p", "k = p-c", "k = p+c");


	for (len=2; len<KEYLENGTH; len++)
	{
		memset (key1, ' ', KEYLENGTH);
		memset (key2, ' ', KEYLENGTH);
		memset (key3, ' ', KEYLENGTH);
		memset (freq, 0, sizeof(freq));

		for (i=0; i<lets; i++)
		    freq[i%len][num[i]]++;

		for (i=0; i<len; i++)
		{
			key1[i] = '.';
			key2[i] = '.';
			key3[i] = '.';
			winnerdot=0;
			winnerdot3=0;

			for (try=0; try<26; try++)
			{
				dot=0;
				dot3=0;

				for (j=0; j<26; j++)   /* Calculate Dot */
				{
				     dot += freq[i][j] * stdf[(j+try) % 26];
				     dot3+= freq[i][j] * stdf[(try-j+26)%26];
				}

				if (dot > winnerdot)   /* find max */
				{
					key1[i] = (26-try) % 26 + 'a';
					key2[i] = (try)    % 26 + 'a';
					winnerdot=dot;
				}

				if (dot3 > winnerdot3)   /* find max */
				{
					key3[i] = (try)    % 26 + 'a';
					winnerdot3=dot3;
				}

			}
		}
		key1[KEYLENGTH-1]=0;  /* Terminate string */
		key2[KEYLENGTH-1]=0;
		key3[KEYLENGTH-1]=0;

		icsum = 0;   /* Compute Index of Coincidence */
		for (i=0; i<len; i++)
		{
			lsum = 0;
			nsum = 0;
			for (j=0; j<26; j++)
			{
				lsum += freq[i][j] * (freq[i][j]-1);
				nsum += freq[i][j];
			}

			icsum += lsum/(nsum*(nsum-1));

		}

		ic = icsum/len;

		printf("%3u   %f   %s   %s   %s\n",
			len, ic, key1, key2, key3);

	}

	printf("\nEnd of Run\n");
	exit(0);
}

