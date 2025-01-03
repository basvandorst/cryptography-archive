typedef unsigned char	byte;
typedef unsigned short	word16;
typedef unsigned long	word32;

#define gamma0	0x025F1CDBUL
#define gamma1	0x04BE39B6UL
#define gamma2	0x12F8E6D8UL
#define gamma3	0x2F8E6D81UL

#define ammag0	0x0DAD4694UL
#define ammag1	0x06D6A34AUL
#define ammag2	0x81B5A8D2UL
#define ammag3	0x281B5A8DUL

#define delta	0x2AAAAAAAUL

#define LO(z)	((z) & 0xFFFFUL)
#define HI(z)	((z) >> 16)


static word32 modmult (word32 x, word32 y)
	/* Returns (x * y) mod (2^32 - 1). */
{
	register word32 z0, z1, q;

	z0 = LO(x)*LO(y);
	z1 = HI(x)*LO(y) + HI(z0);
	q  = LO(z0) + (LO(z1) << 16);

	z0 = LO(q) + HI(z1);
	z1 = LO(x)*HI(y) + HI(q) + HI(z0);
	q  = LO(z0) + (LO(z1) << 16);

	z0 = HI(x)*HI(y) + LO(q) + HI(z1);
	z1 = HI(q) + HI(z0);

	return LO(z0) + (LO(z1) << 16) + HI(z1);
} /* modmult */


static void f (word32 x[4])
{
	word32 t0, t1;

	x[0] = modmult (x[0], gamma0);
	x[1] = modmult (x[1], gamma1);
	x[2] = modmult (x[2], gamma2);
	x[3] = modmult (x[3], gamma3);

	if (x[0] & 1 == 1) {
		x[0] ^ delta;
	}
	if (x[3] & 1 == 0) {
		x[3] ^ delta;
	}

	t0 = x[0] ^ x[2];
	t1 = x[1] ^ x[3];
	x[0] ^= t1;
	x[1] ^= t0;
	x[2] ^= t1;
	x[3] ^= t0;
} /* f */


static void g (word32 x[4])
{
	word32 t0, t1;

	t0 = x[0] ^ x[2];
	t1 = x[1] ^ x[3];
	x[0] ^= t1;
	x[1] ^= t0;
	x[2] ^= t1;
	x[3] ^= t0;

	if (x[0] & 1 == 1) {
		x[0] ^ delta;
	}
	if (x[3] & 1 == 0) {
		x[3] ^ delta;
	}

	x[0] = modmult (x[0], ammag0);
	x[1] = modmult (x[1], ammag1);
	x[2] = modmult (x[2], ammag2);
	x[3] = modmult (x[3], ammag3);
} /* g */


void mmbEncrypt (word32 k[4], word32 x[4])
{
	x[0] ^= k[0]; x[1] ^= k[1]; x[2] ^= k[2]; x[3] ^= k[3];
	f(x);
	x[0] ^= k[1]; x[1] ^= k[2]; x[2] ^= k[3]; x[3] ^= k[0];
	f(x);
	x[0] ^= k[2]; x[1] ^= k[3]; x[2] ^= k[0]; x[3] ^= k[1];
	f(x);
	x[0] ^= k[3]; x[1] ^= k[0]; x[2] ^= k[1]; x[3] ^= k[2];
	f(x);
	x[0] ^= k[0]; x[1] ^= k[1]; x[2] ^= k[2]; x[3] ^= k[3];
	f(x);
	x[0] ^= k[1]; x[1] ^= k[2]; x[2] ^= k[3]; x[3] ^= k[0];
	f(x);
	x[0] ^= k[2]; x[1] ^= k[3]; x[2] ^= k[0]; x[3] ^= k[1];
} /* mmbEncrypt */


void mmbDecrypt (word32 k[4], word32 x[4])
{
	x[0] ^= k[2]; x[1] ^= k[3]; x[2] ^= k[0]; x[3] ^= k[1];
	g(x);
	x[0] ^= k[1]; x[1] ^= k[2]; x[2] ^= k[3]; x[3] ^= k[0];
	g(x);
	x[0] ^= k[0]; x[1] ^= k[1]; x[2] ^= k[2]; x[3] ^= k[3];
	g(x);
	x[0] ^= k[3]; x[1] ^= k[0]; x[2] ^= k[1]; x[3] ^= k[2];
	g(x);
	x[0] ^= k[2]; x[1] ^= k[3]; x[2] ^= k[0]; x[3] ^= k[1];
	g(x);
	x[0] ^= k[1]; x[1] ^= k[2]; x[2] ^= k[3]; x[3] ^= k[0];
	g(x);
	x[0] ^= k[0]; x[1] ^= k[1]; x[2] ^= k[2]; x[3] ^= k[3];
} /* mmbDecrypt */

#ifdef TEST_MMB

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[])
{
	word32 a[4], k[4], z;
	long n;
	clock_t elapsed;
	double secs;

	printf ("MMB cipher test (compiled on " __DATE__ " " __TIME__ ").\n");
	if (argc != 2) {
		printf ("Usage: MMB <number of iterations>\n");
		exit (EXIT_SUCCESS);
	}
	a[0] = k[0] = 3L;
	a[1] = k[1] = 2L;
	a[2] = k[2] = 1L;
	a[3] = k[3] = 0L;
	printf ("key k[*] = %08lx %08lx %08lx %08lx\n", k[3], k[2], k[1], k[0]);
	printf ("old a[*] = %08lx %08lx %08lx %08lx\n", a[3], a[2], a[1], a[0]);
	mmbEncrypt (k, a);
	printf ("new a[*] = %08lx %08lx %08lx %08lx\n", a[3], a[2], a[1], a[0]);
	mmbDecrypt (k, a);
	printf ("end a[*] = %08lx %08lx %08lx %08lx\n", a[3], a[2], a[1], a[0]);

	z = modmult (gamma0, ammag0); printf ("gamma0  *  ammag0 = %08lx\n", z);
	z = modmult (gamma1, ammag1); printf ("gamma1  *  ammag1 = %08lx\n", z);
	z = modmult (gamma2, ammag2); printf ("gamma2  *  ammag2 = %08lx\n", z);
	z = modmult (gamma3, ammag3); printf ("gamma3  *  ammag3 = %08lx\n", z);

	elapsed = -clock ();
	for (n = atol (argv[1]); n > 0; n--) {
		mmbEncrypt (k, a);
	}
	elapsed += clock ();
	secs = (elapsed > 0L) ? (double) elapsed / CLOCKS_PER_SEC : 1.0;
	printf ("elapsed time = %.1lf sec, cipher speed = %.1lf K/sec\n",
		secs, atof (argv[1]) * 16.0 / 1024.0 / secs);
	return 0;
} /* main */

#endif /* ?TEST_MMB */
