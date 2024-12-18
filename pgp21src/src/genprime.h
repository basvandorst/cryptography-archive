/*	genprime.h - C include file for generation of large primes
		used by public-key key generation routines.

	(c) Copyright 1987 by Philip Zimmermann.  All rights reserved.
	The author assumes no liability for damages resulting from the use 
	of this software, even if the damage results from defects in this 
	software.  No warranty is expressed or implied.  

	These functions are for the generation of large prime integers and
	for other functions related to factoring and key generation for 
	many number-theoretic cryptographic algorithms, such as the NIST 
	Digital Signature Standard.

	NOTE:  This assumes previous inclusion of "mpilib.h"
*/

extern word16 primetable[]; /* table of small primes, zero-terminated.*/

boolean primetest(unitptr p);
	/* Returns TRUE iff p is a prime. */

int nextprime(unitptr p);
	/* Find next higher prime starting at p, returning result in p. */

void randombits(unitptr p,short nbits);
	/* Make a random unit array p with nbits of precision. */

int randomprime(unitptr p,short nbits);
	/* Makes a "random" prime p with nbits significant bits of precision. */

void mp_gcd(unitptr result,unitptr a,unitptr n);
	/* Computes greatest common divisor via Euclid's algorithm. */

void mp_inv(unitptr x,unitptr a,unitptr n);
	/* Euclid's algorithm extended to compute multiplicative inverse.
	   Computes x such that a*x mod n = 1, where 0<a<n */

int mp_sqrt(unitptr quotient,unitptr dividend); 
	/* Quotient is returned as the square root of dividend. */


