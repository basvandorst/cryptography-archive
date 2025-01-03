From msuinfo!agate!dog.ee.lbl.gov!ihnp4.ucsd.edu!swrinde!cs.utexas.edu!not-for-mail Wed Apr 27 22:41:23 1994
Path: msuinfo!agate!dog.ee.lbl.gov!ihnp4.ucsd.edu!swrinde!cs.utexas.edu!not-for-mail
From: boucher@csl.sri.com (Peter K. Boucher)
Newsgroups: sci.crypt
Subject: Re: crypto-strengh PRNGs
Date: 25 Apr 1994 13:44:27 -0500
Organization: UTexas Mail-to-News Gateway
Lines: 140
Sender: daemon@cs.utexas.edu
Message-ID: <9404251844.AA02374@redwood.csl.sri.com>
NNTP-Posting-Host: cs.utexas.edu

C445585@mizzou1.missouri.edu (John Kelsey) wrote:
[deletia...]

>   Let X = (A AND B) OR (NOT A AND C).  (This is a bitwise select.)

Funny you should mention it, but I've been testing a version that
(sort of) uses some of the concepts you suggest.  How about this?

============================ BEGIN CODE FRAGMENT ========================

extern void slow_rnd_longs(); /* defined in the reader's imagination :-)    */

/* These must be PRIMES !! */
#define R0_SIZE 349
#define R1_SIZE 953
#define R2_SIZE 601
#define R3_SIZE 751
#define R4_SIZE 557
#define R5_SIZE 887

#define v_SIZE (R0_SIZE+R1_SIZE+R2_SIZE+R3_SIZE+R4_SIZE+R5_SIZE)

#define BSELECT(A,B,M)	(((A)&(M))|((B)&(~(M))))  /* bitwise select         */
#define ROTATE(X,n)	(((X)<<(n))^((X)>>(32-(n))))

static union
{
   unsigned long v[v_SIZE];		/* vector of v_SIZE random longs    */
   struct                               /* initialized by srand128()        */
     {
       unsigned long R0[R0_SIZE],		/* ..redefined as rotors    */
		     R1[R1_SIZE],
		     R2[R2_SIZE],
		     R3[R3_SIZE],
		     R4[R4_SIZE],
		     R5[R5_SIZE];
     } rotor;
} prng; /* end union */

/* pointers for indexing the rotors                                         */
static unsigned long *rotor0, *rotor1, *rotor2, *rotor3, *rotor4, *rotor5;


/* =======================================================================
 *
 * rand32()
 *
 * Returns a ``random'' 32-bit value.
 *
 * =======================================================================  */
unsigned long rand32()
{
			  /* Get the current values from the six rotors.    */
   register unsigned long r0 = *rotor0,
			  r1 = *rotor1,
			  r2 = *rotor2,
			  r3 = *rotor3,
			  r4 = *rotor4,
			  r5 = *rotor5;

        /* Update the state of the six rotors.                              */
        *rotor0 += r5;  /*                 NOTE                             */
	*rotor1 += r3;  /* R5 -> R0 -> R2 -> R4 -> R3 -> R1 -> R5 ...       */
	*rotor2 += r0;  /* where  "->"  means  "modifies the state of".     */
	*rotor3 += r4;  /* The circuit goes thru all 6 with no sub-loops,   */
        *rotor4 += r2;  /* so the equation defining the value of *rotor0    */
        *rotor5 += r1;  /* that went into output[i], call it rotor0_(i), is */
        /* rotor0_(i-R0_SIZE) + rotor5_(i-R0_SIZE).  rotor5_(i-R0_SIZE) is  */
        /* rotor5_(i-(R0_SIZE+R5_SIZE)) + rotor1_(i-(R0_SIZE+R5_SIZE)).     */
        /* rotor1_(i-(R0_SIZE+R5_SIZE)) is                                  */
        /*     rotor1_(i-(R0_SIZE+R5_SIZE+R1_SIZE))                         */
        /*   + rotor3_(i-(R0_SIZE+R5_SIZE+R1_SIZE)).  And so on...          */

	/* Update the pointers that index the six rotors.                   */
	if (++rotor0 >= &(prng.rotor.R0[R0_SIZE])) rotor0 = prng.rotor.R0;
	if (++rotor1 >= &(prng.rotor.R1[R1_SIZE])) rotor1 = prng.rotor.R1;
	if (++rotor2 >= &(prng.rotor.R2[R2_SIZE])) rotor2 = prng.rotor.R2;
	if (++rotor3 >= &(prng.rotor.R3[R3_SIZE])) rotor3 = prng.rotor.R3;
	if (++rotor4 >= &(prng.rotor.R4[R4_SIZE])) rotor4 = prng.rotor.R4;
	if (++rotor5 >= &(prng.rotor.R5[R5_SIZE])) rotor5 = prng.rotor.R5;

        /* Perform a simple permutation of some of the bits.                */
	r0 = ROTATE(r0,8);  /* None of the bits appearing in the output     */
	r1 = ROTATE(r1,16); /* from r0, r1, r2, or r3 will now line up with */
	r2 = ROTATE(r2,24); /* each other.  Also, within a given BSELECT,   */
	r5 = ROTATE(r5,16); /* none of the bits line up.  This prevents     */
        /* Kelsey's attack of guessing the low-order bits of each rotor and */ 
        /* testing the output.  He would have to guess bit(8) of rotor0,    */
        /* bit(16) of rotor1, and bit(0) of rotor4, as well as bit(24) of   */
        /* rotor2, bit(0) of rotor3 and bit(16) of rotor5, if he wanted to  */
        /* predict bit(0) of the output.  Given the size of the rotors, and */
        /* the manner in which they are updated, this is hard, right?       */
                            
    	/* Return a mixed-up set of bits.                                   */
	return( BSELECT(r0,r1,r4) + BSELECT(r2,r3,r5) );
}


/* =======================================================================
 *
 * srand128(seed0,seed1,seed2,seed3)
 * unsigned long seed0,seed1,seed2,seed3;
 *
 * Initializes the state using 128 bits of seed.
 *
 * =======================================================================  */
void
srand128(seed0,seed1,seed2,seed3)
unsigned long seed0,seed1,seed2,seed3;
{
    register int i;

    /* Fill v with "random bits".  The method employed by slow_rnd_longs()  */
    /* crucial, but is left to the imagination the reader (for now).        */
    slow_rnd_longs(seed0, seed1, seed2, seed3, prng.v, v_SIZE);

    /* Set initial, "random" indices into the rotors.                       */
    rotor0 = &(prng.rotor.R0[seed0 % R0_SIZE]);
    rotor1 = &(prng.rotor.R1[seed1 % R1_SIZE]);
    rotor2 = &(prng.rotor.R2[seed2 % R2_SIZE]);
    rotor3 = &(prng.rotor.R3[seed3 % R3_SIZE]);
    rotor4 = &(prng.rotor.R4[(*rotor0 + *rotor2) % R4_SIZE]);
    rotor5 = &(prng.rotor.R5[(*rotor1 + *rotor3) % R5_SIZE]);

    /* Update each value in v several times to obscure the dependency       */
    /* on the initialization performed by slow_rnd_longs().                 */
    for (i=2*v_SIZE; i--;) {
	(void)rand32();
    }
}


============================ END CODE FRAGMENT ========================

-- 
Peter K. Boucher
--
DISCLAIMER:  I am solely responsible for the contents of this message,
	     which should not be misconstrued as being in any way related
	     to the opinions of my employer.

