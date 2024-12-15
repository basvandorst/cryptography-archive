From msuinfo!agate!library.ucla.edu!csulb.edu!csus.edu!netcom.com!jktaber Sat Apr  2 21:49:36 1994
Newsgroups: sci.crypt
Path: msuinfo!agate!library.ucla.edu!csulb.edu!csus.edu!netcom.com!jktaber
From: jktaber@netcom.com (John K. Taber)
Subject: Yet Another Secure? RNG
Message-ID: <jktaberCnHAoC.3yM@netcom.com>
Organization: NETCOM On-line Communication Services (408 241-9760 guest)
Date: Wed, 30 Mar 1994 12:57:48 GMT
Lines: 76

This post appears not to have made it the 1st time.  Please excuse if
it eventually appears twice.  
------------------------------------------------------------------------

This prng is Peter Bosch's original suggestion, with modifications by me
to defeat solving by simultaneous linear equations.  It includes a
suggestion by John Kelsey to insure that all bit positions participate in
changing the state of the prng.


union
{
   unsigned char v[17]			/* vector of 17 random bytes	*/
   struct
     {
       unsigned char R2[2],		/* ..redefined as rotors	*/
		     R3[3],
		     R5[5],
		     R7[7];
     } rotor;
} prng; /* end union */

/* return qty random bytes to dest, updating ndx for next call		*/
void	f(int, *long, *char)
int		qty;			/* number of random bytes to return */
long	       *ndx;			/* iteration count over all calls  */
unsigned char  *dest;			/* where to return random bytes    */
{
   unsigned char a:

   for ( ; 0 < qty; qty--, dest++, ++*ndx)
      { /* a is sum of ith value of each rotor, mod 256 		*/
	a = prng.rotor.R2[(*ndx)%2]
	    + prng.rotor.R3[(*ndx)%3]
	    + prng.rotor.R5[(*ndx)%5]
	    + prng.rotor.R7[(*ndx)%7];
	*dest = a;			/* return the random byte	*/

	/* replace ith element in v.  use rotate so that all bit positions */
	/* eventually participate in changing v's state                    */
	if (a & 0x80) v[(*ndx)%17] = v[(*ndx)%17] + 0x01 | (a<<1);
	else v[(*ndx)%17] = v[(*ndx)%17] + a<<1;
      }
}

The question is, is this a "secure" prng?  John Kelsey said no, I believe
him, but I want to know why.

Some notes.  There is no profound reason for four rotors, nor for adding
them instead of combining them in some other way.  Adding is convenient,
and arguably a tad stronger than XOR.  The essential point is to combine n
values taken r at a time in some systematic fashion that involves all n
values eventually.  Also, the combining function must not be degenerative.
I want r values so that backsolving is impossible.  Each of the n values
is replaced to defeat key recovery by solving n equations in n unknowns.

The vector v is seeded with 17 random values.  To take a cue from the DES
S-boxes, it isn't clear that random is best.  Random values are
convenient, but what "best" means needs to be determined.

It is not clear what the length of v should be.  I used 17 since it is the
sum of the rotor lengths.

Empirically, the randomness of the output appears good.  I've computed
chi-squared, tested for runs up and down, counted 1s and 0s for up to
100,000 bytes.	No cycles within these limits were detected.

When does this prng repeat?  At most I guess that there are 256^17
iterations before this prng repeats.  More likely, there are many smaller
cycles, but I don't have any idea.

What I'm interested in is the kind of thinking needed to see the
weaknesses of such a cipher.
-- 
John K. Taber                        jktaber@netcom.com
=======================================================

