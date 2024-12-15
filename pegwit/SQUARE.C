

/*
 * Reference implementation of the Square block cipher.
 *
 * Algorithm developed by Joan Daemen <joan.daemen@club.innet.be> and
 * Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 *
 * This public domain implementation by Paulo S.L.M. Barreto
 * <pbarreto@uninet.com.br> based on software written by Vincent Rijmen.
 *
 * Caveat: this code assumes 32-bit words and probably will not work
 * otherwise.
 *
 * Version 1.0.1 (1996.10.25)
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "square.h"
#include "square.tab"	/* substitution boxes and other tables */

#if R != 6
#error "This implementation assumes exactly 6 rounds"
#endif

static word32 offset[7] = { /* (2^54) * sqrt(2) */
	0x0000005aUL,
	0x00000082UL,
	0x00000079UL,
	0x00000099UL,
	0x000000fcUL,
	0x000000efUL,
	0x00000032UL,
};

#ifdef HARDWARE_ROTATIONS
	/* use this if your compiler provides intrinsic
	 * (hardware) rotation functions:
	 */
	#define ROTR(x, s) (_lrotr ((x), (s)))
	#define ROTL(x, s) (_lrotl ((x), (s)))
#else  /* !HARDWARE_ROTATIONS */
	#define ROTR(x, s) (((x) >> (s)) | ((x) << (32 - (s))))
	#define ROTL(x, s) (((x) << (s)) | ((x) >> (32 - (s))))
#endif /* ?HARDWARE_ROTATIONS */

#define MSB(x) (((x) >> 24) & 0xffU)	/* most  significant byte */
#define SSB(x) (((x) >> 16) & 0xffU)	/* second in significance */
#define TSB(x) (((x) >>  8) & 0xffU)	/* third  in significance */
#define LSB(x) (((x)      ) & 0xffU)	/* least significant byte */

/* mul(a, b) evaluates the product of two elements of GF(2^8) */
#define mul(a, b) (((a) && (b)) ? exptab[logtab[a] + logtab[b]] : 0)


static void squareTransform (word32 in[4], word32 out[4])
	/* Transform an encryption round key into a decryption round key */
{
	int i, j;
	byte A[4][4], B[4][4];

	for (i = 0; i < 4; i++) {
		A[i][0] = (byte) (in[i] >> 24);
		A[i][1] = (byte) (in[i] >> 16);
		A[i][2] = (byte) (in[i] >>  8);
		A[i][3] = (byte) (in[i]      );
	}

	/* B = A * iG */      
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			B[i][j] =
				mul (A[i][0], iG[0][j]) ^
				mul (A[i][1], iG[1][j]) ^
				mul (A[i][2], iG[2][j]) ^
				mul (A[i][3], iG[3][j]);
		}
	}

	for (i = 0; i < 4; i++) {
		out[i] =
			(B[i][0] << 24) ^
			(B[i][1] << 16) ^
			(B[i][2] <<  8) ^
			(B[i][3]      );
	}   
} /* squareTransform */


void squareGenerateRoundkeys (word32 key[4],
	word32 roundkeys_e[R+1][4], word32 roundkeys_d[R+1][4])
	/* Generate the basic roundkeys */
{
	int t;

	roundkeys_e[0][0] = key[0] ^ offset[0];
	roundkeys_e[0][1] = key[1];
	roundkeys_e[0][2] = key[2];
	roundkeys_e[0][3] = key[3];
   
	for (t = 1; t < R+1; t++) {
		roundkeys_e[t][0] =
			ROTR (roundkeys_e[t-1][0], 8) ^ roundkeys_e[t-1][1]
			^ offset[t];
		roundkeys_e[t][1] =
			ROTR (roundkeys_e[t-1][1], 8) ^ roundkeys_e[t-1][2];
		roundkeys_e[t][2] =
			ROTR (roundkeys_e[t-1][2], 8) ^ roundkeys_e[t-1][3];
		roundkeys_e[t][3] =
			ROTR (roundkeys_e[t-1][3], 8) ^ roundkeys_e[t][0];
	}  

	/* calculate the decryption round keys:
	 * reverse order and account for interchange with diffusion 
	 */

	/* first and last round: no change */
	for (t = 0; t < 4; t++) {
		roundkeys_d[0][t] = roundkeys_e[R][t];  
		roundkeys_d[R][t] = roundkeys_e[0][t];
	}

	/* intermediate rounds */
	for (t = 1; t < R; t++) {
		squareTransform (roundkeys_e[t], roundkeys_d[R-t]);
	}
} /* squareGenerateRoundkeys */


#define squareRound(ibuf, obuf, T0, T1, T2, T3, roundkeys, r) \
{ \
	obuf[0] = T0 [MSB (ibuf[0])] \
			^ T1 [MSB (ibuf[1])] \
			^ T2 [MSB (ibuf[2])] \
			^ T3 [MSB (ibuf[3])] \
			^ roundkeys[r][0]; \
\
	obuf[1] = T0 [SSB (ibuf[0])] \
			^ T1 [SSB (ibuf[1])] \
			^ T2 [SSB (ibuf[2])] \
			^ T3 [SSB (ibuf[3])] \
			^ roundkeys[r][1]; \
\
	obuf[2] = T0 [TSB (ibuf[0])] \
			^ T1 [TSB (ibuf[1])] \
			^ T2 [TSB (ibuf[2])] \
			^ T3 [TSB (ibuf[3])] \
			^ roundkeys[r][2]; \
\
	obuf[3] = T0 [LSB (ibuf[0])] \
			^ T1 [LSB (ibuf[1])] \
			^ T2 [LSB (ibuf[2])] \
			^ T3 [LSB (ibuf[3])] \
			^ roundkeys[r][3]; \
} /* squareRound */


#define squareFinal(ibuf, obuf, T0, T1, T2, T3, roundkeys, r) \
{ \
	obuf[0] = (T1 [MSB (ibuf[0])] & 0xff000000UL) \
			^ (T2 [MSB (ibuf[1])] & 0x00ff0000UL) \
			^ (T3 [MSB (ibuf[2])] & 0x0000ff00UL) \
			^ (T0 [MSB (ibuf[3])] & 0x000000ffUL) \
			^ roundkeys[r][0]; \
\
	obuf[1] = (T1 [SSB (ibuf[0])] & 0xff000000UL) \
			^ (T2 [SSB (ibuf[1])] & 0x00ff0000UL) \
			^ (T3 [SSB (ibuf[2])] & 0x0000ff00UL) \
			^ (T0 [SSB (ibuf[3])] & 0x000000ffUL) \
			^ roundkeys[r][1]; \
\
	obuf[2] = (T1 [TSB (ibuf[0])] & 0xff000000UL) \
			^ (T2 [TSB (ibuf[1])] & 0x00ff0000UL) \
			^ (T3 [TSB (ibuf[2])] & 0x0000ff00UL) \
			^ (T0 [TSB (ibuf[3])] & 0x000000ffUL) \
			^ roundkeys[r][2]; \
\
	obuf[3] = (T1 [LSB (ibuf[0])] & 0xff000000UL) \
			^ (T2 [LSB (ibuf[1])] & 0x00ff0000UL) \
			^ (T3 [LSB (ibuf[2])] & 0x0000ff00UL) \
			^ (T0 [LSB (ibuf[3])] & 0x000000ffUL) \
			^ roundkeys[r][3]; \
} /* squareFinal */


void squareEncrypt (word32 text[4], word32 roundkeys[R+1][4])
{
	word32 temp[4];

	/* initial key addition */

	text[0] ^= roundkeys[0][0];
	text[1] ^= roundkeys[0][1];
	text[2] ^= roundkeys[0][2];
	text[3] ^= roundkeys[0][3];

	/* R - 1 full rounds (optimized for R = 6) */

	squareRound (text, temp, Te0, Te1, Te2, Te3, roundkeys, 1);
	squareRound (temp, text, Te0, Te1, Te2, Te3, roundkeys, 2);
	squareRound (text, temp, Te0, Te1, Te2, Te3, roundkeys, 3);
	squareRound (temp, text, Te0, Te1, Te2, Te3, roundkeys, 4);
	squareRound (text, temp, Te0, Te1, Te2, Te3, roundkeys, 5);

	/* last round (diffusion becomes only transposition) */
	squareFinal (temp, text, Te0, Te1, Te2, Te3, roundkeys, R);
} /* squareEncrypt */


void squareDecrypt (word32 text[4], word32 roundkeys[R+1][4])
{
	word32 temp[4];

	/* initial key addition */

	text[0] ^= roundkeys[0][0];
	text[1] ^= roundkeys[0][1];
	text[2] ^= roundkeys[0][2];
	text[3] ^= roundkeys[0][3];

	/* R - 1 full rounds (optimized for R = 6) */

	squareRound (text, temp, Td0, Td1, Td2, Td3, roundkeys, 1);
	squareRound (temp, text, Td0, Td1, Td2, Td3, roundkeys, 2);
	squareRound (text, temp, Td0, Td1, Td2, Td3, roundkeys, 3);
	squareRound (temp, text, Td0, Td1, Td2, Td3, roundkeys, 4);
	squareRound (text, temp, Td0, Td1, Td2, Td3, roundkeys, 5);

	/* last round (diffusion becomes only transposition) */
	squareFinal (temp, text, Td0, Td1, Td2, Td3, roundkeys, R);
} /* squareDecrypt */


/*------------------------------------------------------------------*/


//#define  TEST_SQUARE
#ifdef TEST_SQUARE

static void verify (word32 k0, word32 k1, word32 k2, word32 k3,
	word32 t0, word32 t1, word32 t2, word32 t3,
	word32 c0, word32 c1, word32 c2, word32 c3)
{
	word16 t;
	word32 key[4], text[4];
	word32 roundkeys_e[R+1][4], roundkeys_d[R+1][4];

	key [0] = k0; key [1] = k1; key [2] = k2; key [3] = k3;
	text[0] = t0; text[1] = t1; text[2] = t2; text[3] = t3;
	for(t = 0; t < 4; t++) printf("%08lx ",key[t]);
	printf(" key\n");
	squareGenerateRoundkeys(key, roundkeys_e, roundkeys_d);
	for(t = 0; t < 4; t++) printf("%08lx ",text[t]);
	printf(" data\n");
	squareEncrypt(text, roundkeys_e);
	for(t = 0; t < 4; t++) printf("%08lx ",text[t]);
	printf(" encrypt ");
	if (text[0] != c0 || text[1] != c1 || text[2] != c2 || text[3] != c3) {
		printf ("(failure)\n");
	} else {
		printf ("(success)\n");
	}
	squareDecrypt(text, roundkeys_d);   
	for(t = 0; t < 4; t++) printf("%08lx ",text[t]);
	printf(" decrypt ");
	if (text[0] != t0 || text[1] != t1 || text[2] != t2 || text[3] != t3) {
		printf ("(failure)\n");
	} else {
		printf ("(success)\n");
	}
	printf("\n");
} /* verify */


int main (int argc, char *argv[])
{
	printf ("Square cipher (compiled on " __DATE__ " " __TIME__ ").\n");

	if (argc != 2) {
		printf ("Checking correctness...\n");
		verify (0x00000000UL, 0x00000000UL, 0x00000000UL, 0x00000000UL,
				0x00000000UL, 0x00000000UL, 0x00000000UL, 0x00000000UL,
				0x89362ea3UL, 0x457f51e9UL, 0x939e2c4fUL, 0x2a671ec4UL);

		verify (0x00000000UL, 0x00000000UL, 0x00000000UL, 0x00000000UL,
				0x40414243UL, 0x44454647UL, 0x48494a4bUL, 0x4c4d4e4fUL,
				0x6c2d0bdcUL, 0x14e8a1d4UL, 0x24316b07UL, 0x84690a75UL);

		verify (0x40414243UL, 0x44454647UL, 0x48494a4bUL, 0x4c4d4e4fUL,
				0x00000000UL, 0x00000000UL, 0x00000000UL, 0x00000000UL,
				0x6f845d21UL, 0x66d8c786UL, 0x4870b356UL, 0x112652d0UL);

		verify (0x01234567UL, 0x89abcdefUL, 0x01234567UL, 0x89abcdefUL,
				0x20202020UL, 0x20202020UL, 0x20202020UL, 0x20202020UL,
				0xdc81fcb8UL, 0xb714c7bbUL, 0x3c02a7beUL, 0x57eb5237UL);
	} else {
		word32 key[4], text[4];
		word32 roundkeys_e[R+1][4], roundkeys_d[R+1][4];
		long n; clock_t elapsed; double sec;
	
		printf ("Measuring encryption speed...");
		squareGenerateRoundkeys(key, roundkeys_e, roundkeys_d);
		elapsed = -clock ();
		for (n = atol (argv[1]); n > 0; n--) {
			squareEncrypt(text, roundkeys_e);
		}
		elapsed += clock ();
		sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
		printf (" %.2f sec, %.1f K/sec.\n",
			sec, 16.0*atof (argv[1])/1024.0/sec);
	
		printf ("Measuring decryption speed...");
		elapsed = -clock ();
		for (n = atol (argv[1]); n > 0; n--) {
			squareDecrypt(text, roundkeys_d);   
		}
		elapsed += clock ();
		sec = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
		printf (" %.2f sec, %.1f K/sec.\n",
			sec, 16.0*atof (argv[1])/1024.0/sec);
	}
	return 0;
} /* main */

#endif /* TEST_SQUARE */
