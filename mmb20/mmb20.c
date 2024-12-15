/*
	The Modular Multiplication based Block cipher (MMB),
	designed by Joan Daemen. This public domain implementation
	by Paulo Barreto <pbarreto@uninet.com.br>.

	To compile with Daemen's strengthened key schedule against
	Biham's	related-key attack, #define STRENGTHENED_KEY_SCHEDULE
	(see eq. 11.38 of J. Daemen's doctoral thesis, available from
	ftp://ftp.esat.kuleuven.ac.be/pub/COSIC/daemen/thesis/)
*/

#define STRENGTHENED_KEY_SCHEDULE

#if defined _MSC_VER && _MSC_VER >= 1000
	#define ASMV /* modular multiplication coded in assembler */
#endif /* ?_MSC_VER */

#if defined _M_IX86 && _M_IX86 >= 500 /* Pentium or better */
	#define BIT64 /* platforms with 64-bit words */
#endif /* _M_IX86 */

#ifndef USUAL_TYPES
#define USUAL_TYPES
	typedef unsigned char  byte;
	typedef unsigned short word16;
	typedef unsigned long  word32;
	#ifdef BIT64
		typedef unsigned __int64 word64;
	#endif /* ?BIT64 */
#endif /* USUAL_TYPES */

/* the substitution box constants: */
#define GAMMA0	0x025F1CDBUL
#define GAMMA1	0x04BE39B6UL
#define GAMMA2	0x12F8E6D8UL
#define GAMMA3	0x2F8E6D81UL

/* the corresponding inverses: */
#define AMMAG0	0x0DAD4694UL
#define AMMAG1	0x06D6A34AUL
#define AMMAG2	0x81B5A8D2UL
#define AMMAG3	0x281B5A8DUL

/* other constants used by the algorithm: */
#define DELTA	0x2AAAAAAAUL
#define OMEGA	0xFFFFFFFFUL
#define DAEMEN	((word32) 0x0DAE) /* interesting, isn't it? :-) */

#define LO(z)	((z) & 0xFFFFUL)
#define HI(z)	((z) >> 16)


#ifdef CORRECT_BUT_SLOW
static word32 MODMULT (word32 x, word32 y)
	/* Returns (x * y) mod (2^32-1) */
{
	word32 t; word16 u[2], v[2], w[4];

	((word32 *)u)[0] = x;
	((word32 *)v)[0] = y;
	w[0] = w[1] = w[2] = w[3] = 0;

	t = (word32) u[0] * (word32) v[0] + (word32) w[0];
	w[0] = (word16) LO(t);
	t = (word32) u[1] * (word32) v[0] + (word32) w[1] + HI(t);
	w[1] = (word16) LO(t);
	w[2] += (word16) HI(t);

	t = (word32) u[0] * (word32) v[1] + (word32) w[1];
	w[1] = (word16) LO(t);
	t = (word32) u[1] * (word32) v[1] + (word32) w[2] + HI(t);
	w[2] = (word16) LO(t);
	w[3] = (word16) HI(t);

	t = (word32) w[0] + (word32) w[2];
	w[0] = (word16) LO(t);
	t = (word32) w[1] + (word32) w[3] + HI(t);
	w[1] = (word16) LO(t);

	return ((word32 *)w)[0] + HI(t);
} /* MODMULT */
#endif /* ?CORRECT_BUT_SLOW */


/* MODMULT evaluates x = (x * c) mod (2^32-1).             */
/* Let x (+) y and x (*) y denote (x + y) mod (2^m - 1)    */
/* and (x * y) mod (2^m - 1)  (i. e. modular addition  and */
/* modular multiplication),  respectively.  The  following */
/* equalities apply (see Knuth vol. 2 sect 4.3.2):         */
/* x (*) y = (x * y) mod (2^m) (+) (x * y) div (2^m)       */
/* x (+) y = (x + y) mod (2^m)  +  (x + y) div (2^m)       */
/* Note that (x + y) div (2^m) is either 0 if x + y >= 2^m */
/* or 1 if x + y < 2^m; we can test for the right value by */
/* checking if x < 2^m - y (this never causes overflow) or */
/* looking at the carry bit after addition (asm version).  */
#ifdef ASMV
	/* See Knuth vol. 2 exercise 3.2.1.1-8 */
	static __inline word32 do_modmult (word32 x, word32 c)
	{
		__asm {
			__asm mov eax, x
			__asm mov ebx, c
			__asm mul ebx
			__asm add eax, edx
			__asm jnc SKIP_INC
			__asm inc eax
SKIP_INC:
		}
		/* if you don't like stupid compiler grumblings as */
		/* "warning C4035: '__MODMULT' : no return value", */
		/* try uncommenting the following line (there's an */
		/* efficiency penalty of about 10%):               */
		/* return x; */
	} /* do_modmult */
	#define MODMULT(x, c) { x = do_modmult (x, c); }
#elif defined BIT64
	#define MODMULT(x, c) \
	{ \
		w = (word64) x * c, \
		w = (w >> 32) + (w & OMEGA), \
		x = (word32) w + (word32) (w > (word64) OMEGA); \
	} /* MODMULT */
#else  /* !(ASMV || BIT64) */
	/* This mumbo-jumbo  is a combination of  the classical pencil- */
	/* -and-paper multiplication and addition algorithms. All loops */
	/* were unrolled for efficiency  (some extra code massaging was */
	/* done as well :-)  After some thought, you will either notice */
	/* the equivalence to the BIT64 version or get a headache :-)   */
	#define MODMULT(x, c) \
	{ \
		v = HI(x) * LO(c)         + HI(u = LO(x) * LO(c)), \
		v = LO(x) * HI(c) + LO(v) + HI(u =                 LO(u) + HI(v)), \
		v =                 LO(v) + HI(u = HI(x) * HI(c) + LO(u) + HI(v)), \
		x = (v << 16) + LO(u) + HI(v); \
	} /* MODMULT */
#endif /* ?(ASMV || BIT64) */

#define F1(x, g0, g1, g2, g3) \
{ \
	MODMULT (x[0], g0); \
	MODMULT (x[1], g1); \
	MODMULT (x[2], g2); \
	MODMULT (x[3], g3); \
} /* F1 */

#define F2(x) \
{ \
	if (x[0] & 1 == 1) { \
		x[0] ^ DELTA; \
	} \
	if (x[3] & 1 == 0) { \
		x[3] ^ DELTA; \
	} \
} /* F2 */

#define F3(x) \
{ \
	u = x[0] ^ x[2]; \
	v = x[1] ^ x[3]; \
	x[0] ^= v; \
	x[1] ^= u; \
	x[2] ^= v; \
	x[3] ^= u; \
} /* F3 */

#define EF(x) \
	F1 (x, GAMMA0, GAMMA1, GAMMA2, GAMMA3); F2 (x); F3 (x); \

#define DF(x) \
	F3 (x); F2 (x); F1 (x, AMMAG0, AMMAG1, AMMAG2, AMMAG3); \

#ifdef STRENGTHENED_KEY_SCHEDULE
	/* apply a bias to k[i0] */
	#define APPLY_KEY(x, k, i0, i1, i2, i3, r) \
		x[0] ^= k[i0] ^ (DAEMEN << (r)); \
		x[1] ^= k[i1]; \
		x[2] ^= k[i2]; \
		x[3] ^= k[i3];
#else  /* !STRENGTHENED_KEY_SCHEDULE */
	#define APPLY_KEY(x, k, i0, i1, i2, i3, r) \
		x[0] ^= k[i0]; \
		x[1] ^= k[i1]; \
		x[2] ^= k[i2]; \
		x[3] ^= k[i3];
#endif /* ?STRENGTHENED_KEY_SCHEDULE */


void mmbEncrypt (word32 k[4], word32 x[4])
{
	word32 u, v;
#if defined BIT64 && !defined ASMV
	word64 w;
#endif /* ?(BIT64 && !ASMV) */

	APPLY_KEY (x, k, 0, 1, 2, 3, 0);
	EF(x);
	APPLY_KEY (x, k, 1, 2, 3, 0, 1);
	EF(x);
	APPLY_KEY (x, k, 2, 3, 0, 1, 2);
	EF(x);
	APPLY_KEY (x, k, 3, 0, 1, 2, 3);
	EF(x);
	APPLY_KEY (x, k, 0, 1, 2, 3, 4);
	EF(x);
	APPLY_KEY (x, k, 1, 2, 3, 0, 5);
	EF(x);
	APPLY_KEY (x, k, 2, 3, 0, 1, 6);
} /* mmbEncrypt */


void mmbDecrypt (word32 k[4], word32 x[4])
{
	word32 u, v;
#if defined BIT64 && !defined ASMV
	word64 w;
#endif /* ?(BIT64 && !ASMV) */

	APPLY_KEY (x, k, 2, 3, 0, 1, 6);
	DF(x);
	APPLY_KEY (x, k, 1, 2, 3, 0, 5);
	DF(x);
	APPLY_KEY (x, k, 0, 1, 2, 3, 4);
	DF(x);
	APPLY_KEY (x, k, 3, 0, 1, 2, 3);
	DF(x);
	APPLY_KEY (x, k, 2, 3, 0, 1, 2);
	DF(x);
	APPLY_KEY (x, k, 1, 2, 3, 0, 1);
	DF(x);
	APPLY_KEY (x, k, 0, 1, 2, 3, 0);
} /* mmbDecrypt */

#ifdef TEST_MMB

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#ifdef OMIT
static void checkModmult (void)
{
	word32 x;
#ifdef BIT64
	word64 w;
#else  /* !BIT64 */
	word32 u, v;
#endif

	x = GAMMA0; MODMULT (x, AMMAG0);
	printf ("GAMMA0 * AMMAG0 = %08lx\n", x);
	x = GAMMA1; MODMULT (x, AMMAG1);
	printf ("GAMMA1 * AMMAG1 = %08lx\n", x);
	x = GAMMA2; MODMULT (x, AMMAG2);
	printf ("GAMMA2 * AMMAG2 = %08lx\n", x);
	x = GAMMA3; MODMULT (x, AMMAG3);
	printf ("GAMMA3 * AMMAG3 = %08lx\n", x);
	x = OMEGA;  MODMULT (x, GAMMA0);
	printf ("OMEGA  * GAMMA0 = %08lx\n", x);
	x = OMEGA;  MODMULT (x, OMEGA);
	printf ("OMEGA  * OMEGA  = %08lx\n", x);
} /* checkModmult */
#endif /* OMIT */


int main (int argc, char *argv[])
{
	word32 a[4], k[4];
	long n;
	clock_t elapsed;
	double secs;

	printf ("MMB cipher test (compiled on " __DATE__ " " __TIME__ ").\n");
#ifdef STRENGTHENED_KEY_SCHEDULE
	printf ("Using Daemen's strengthened key schedule.\n");
#endif /* ?STRENGTHENED_KEY_SCHEDULE */
#ifdef ASMV
	printf ("Modular multiplication coded in assembler.\n");
#endif /* ?ASMV */
	if (argc != 2) {
		printf ("Usage: MMB <number of iterations>\n");
		exit (EXIT_SUCCESS);
	}
	a[0] = k[0] = 3L;
	a[1] = k[1] = 2L;
	a[2] = k[2] = 1L;
	a[3] = k[3] = 0L;
	printf ("key   k[*] = %08lx %08lx %08lx %08lx\n", k[3], k[2], k[1], k[0]);
	printf ("plain a[*] = %08lx %08lx %08lx %08lx\n", a[3], a[2], a[1], a[0]);
	mmbEncrypt (k, a);
	printf ("crypt a[*] = %08lx %08lx %08lx %08lx\n", a[3], a[2], a[1], a[0]);
	mmbDecrypt (k, a);
	printf ("check a[*] = %08lx %08lx %08lx %08lx\n", a[3], a[2], a[1], a[0]);

	printf ("Encrypting..."); fflush (stdout);
	elapsed = -clock ();
	for (n = atol (argv[1]); n > 0; n--) {
		mmbEncrypt (k, a);
	}
	elapsed += clock ();
	secs = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.1lf sec, %.1lf K/sec\n",
		secs, atof (argv[1]) * sizeof (a) / 1024.0 / secs);

	printf ("Decrypting..."); fflush (stdout);
	elapsed = -clock ();
	for (n = atol (argv[1]); n > 0; n--) {
		mmbDecrypt (k, a);
	}
	elapsed += clock ();
	secs = elapsed ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf (" %.1lf sec, %.1lf K/sec\n",
		secs, atof (argv[1]) * sizeof (a) / 1024.0 / secs);

	return 0;
} /* main */

#endif /* ?TEST_MMB */
