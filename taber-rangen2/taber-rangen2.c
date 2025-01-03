From msuinfo!agate!library.ucla.edu!csulb.edu!csus.edu!netcom.com!jktaber Tue Apr 19 17:53:11 1994
Newsgroups: sci.crypt
Path: msuinfo!agate!library.ucla.edu!csulb.edu!csus.edu!netcom.com!jktaber
From: jktaber@netcom.com (John K. Taber)
Subject: Re: Yet Another Secure? RNG
Message-ID: <jktaberCoDrK5.7Gs@netcom.com>
Organization: None
X-Newsreader: TIN [version 1.2 PL1]
References: <jktaberCnHAoC.3yM@netcom.com> <16F8E10277S86.C445585@mizzou1.missouri.edu> <jktaberCnv3Bv.25p@netcom.com>
Date: Sun, 17 Apr 1994 01:45:41 GMT
Lines: 74


I thought I had an easy fix for Kelsey's attack.  You remember his attack
depends on isolating bits of v.  As it turns out, fixing this rotor 
cipher isn't easy.  The trouble is, the change to v does not diffuse fast 
enough through the 17 byte key to prevent bits from being isolated.  
Dodges like permuting bits is no help.  What is wanted is a method that, 
as a byte in v is changed propagates that change rapidly and complexely 
through the key generator.

The best I could think of was to iterate the prng at least 17 times for
every byte returned to guarantee massive changes to v.	This is something
like the 16 rounds of DES.  It is a very simple change, and I hope
effective.

I think this is the best I can do to make this prng "secure".  Does
anybody see any weaknesses?

BTW, it does have nice properties.  v can be set to almost anything, and
the output still appears random.  v's of very slight differences (just one
bit of difference) generate apparently violently unrelated sequences of
random bytes.

There were suggestions to lengthen the rotors.	I disagree.  Lengthening
the rotors makes it that much harder to diffuse changes throughout the
random number generator.  It's not the period that is the problem.  It's
not the shortness of the rotors that is the problem.  The problem is
making the rng complexely dependent on a small change to itself.

Here is the rewritten byte prng.  Thanks to the C professionals for
showing me how to do a rotate in C.


union
{
   unsigned char v[17]; 	/* vector of 17 random bytes		*/
   struct			/* it is unstated how these bytes are	*/
   {				/* filled.
      unsigned char R2[2],	/* redefined as 4 rotors		*/
		    R3[3],
		    R5[5],
		    R7[7];
   } rotor;
} prng;

#define ROTATE (X) (  ((X) << 1)  |  ((X)  >> 7)  )

void f2(int, *long, *char)
   int		    qty;	/* quantity of random bytes to return	*/
   long 	   *ndx;	/* starting iteration			*/
   unsigned char   *dest;	/* ->receiving buffer			*/
{
   unsigned char    a;		/* accumulator				*/
   int		    i;

   for(a = 0; qty > 0; qty--, ++*ndx, dest++)
      {
	 for(i = 0; i < 17; i++)
	    {				/* ensure all v[i] are changed	*/
	       a = prng.rotor.R2[(i + *ndx) % 2] +
		   prng.rotor.R3[(i + *ndx) % 3] +
		   prng.rotor.R5[(i + *ndx) % 5] +
		   prng.rotor.R7[(i + *ndx) % 7];
	       v[(i + *ndx) % 17] = a + ROTATE(v[(i + *ndx) % 17]);
	    }
	 *dest = a;			/* before returning random byte */
      }
} /* end function f2 */


-- 
John K. Taber                                 jktaber@netcom.com
========================================================================
Avoid taking a definite stand on great public issues either in the Senate
or before the people.  Bend your energies towards making friends of key
men in all classes of voters.  -- advice to Cicero from his brother and
campaign manager, Quintus.


