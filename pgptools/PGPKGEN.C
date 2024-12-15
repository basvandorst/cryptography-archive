/* pgpkgen.c */

/* Most of this code has been chopped out of genprime.c and rsagen.c
 
   (c) Copyright 1987,1993 by Philip Zimmermann.  All rights reserved.
   The author assumes no liability for damages resulting from the use 
   of this software, even if the damage results from defects in this 
   software.  No warranty is expressed or implied.  
*/

#include <stdio.h>
#include <time.h>
#include "usuals.h"
#include "md5.h"
#include "mpilib.h"
#include "pgpmem.h"
#include "fifo.h"
#include "pgptools.h"
#include "pgpkgen.h"

#define swap(p,q)  { unitptr t; t = p;  p = q;  q = t; }
#define iplus1  ( i==2 ? 0 : i+1 )	/* used by Euclid algorithms */
#define iminus1 ( i==0 ? 2 : i-1 )	/* used by Euclid algorithms */

static unsigned stage; /* For output */
static void (*output)(); /* Pointer to output function */

/* Generate a key pair */
void rsa_keygen(unitptr n, unitptr e, unitptr d,
	unitptr p, unitptr q, unitptr u, short keybits, short ebits);

/* Used by rsa_keygen to generate rest of key after primes are found */
static void derive_rsakeys(unitptr n, unitptr e, unitptr d,
	unitptr p, unitptr q, unitptr u, short ebits);

/* Find next higher prime starting at p, returning result in p. */
static void nextprime(unitptr p);

/* Test for primality with Fermat's theorem */
static boolean slowtest(unitptr p);

/* Test with the sieve */
static boolean fastsieve(word16 pdelta, word16 remainders[]);

/* Set up the sieve */
static void buildsieve(unitptr p, word16 remainders[]);

/* primetable is a table of 16-bit prime numbers used for sieving 
   and for other aspects of public-key cryptographic key generation
*/

static word16 primetable[] = {
   2,   3,   5,   7,  11,  13,  17,  19,
  23,  29,  31,  37,  41,  43,  47,  53,
  59,  61,  67,  71,  73,  79,  83,  89,
  97, 101, 103, 107, 109, 113, 127, 131,
 137, 139, 149, 151, 157, 163, 167, 173,
 179, 181, 191, 193, 197, 199, 211, 223,
 227, 229, 233, 239, 241, 251, 257, 263,
 269, 271, 277, 281, 283, 293, 307, 311,
#ifndef EMBEDDED	/* not embedded, use larger table */
 313, 317, 331, 337, 347, 349, 353, 359,
 367, 373, 379, 383, 389, 397, 401, 409,
 419, 421, 431, 433, 439, 443, 449, 457,
 461, 463, 467, 479, 487, 491, 499, 503,
 509, 521, 523, 541, 547, 557, 563, 569,
 571, 577, 587, 593, 599, 601, 607, 613,
 617, 619, 631, 641, 643, 647, 653, 659,
 661, 673, 677, 683, 691, 701, 709, 719,
 727, 733, 739, 743, 751, 757, 761, 769,
 773, 787, 797, 809, 811, 821, 823, 827,
 829, 839, 853, 857, 859, 863, 877, 881,
 883, 887, 907, 911, 919, 929, 937, 941,
 947, 953, 967, 971, 977, 983, 991, 997,
 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049,
 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097,
 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283,
 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321,
 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423,
 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459,
 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571,
 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619,
 1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693,
 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747,
 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811,
 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877,
 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949,
 1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003,
#ifdef BIGSIEVE /* use giant sieve */
 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069,
 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129,
 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203,
 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267,
 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311,
 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377,
 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423,
 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503,
 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579,
 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657,
 2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693,
 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741,
 2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801,
 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861,
 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939,
 2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011,
 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079,
 3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167,
 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221,
 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301,
 3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347,
 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413,
 3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491,
 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541,
 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607,
 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671,
 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727,
 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797,
 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863,
 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923,
 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003,
 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057,
 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129,
 4133, 4139, 4153, 4157, 4159, 4177, 4201, 4211,
 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259,
 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337,
 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409,
 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481,
 4483, 4493, 4507, 4513, 4517, 4519, 4523, 4547,
 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621,
 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673,
 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751,
 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813,
 4817, 4831, 4861, 4871, 4877, 4889, 4903, 4909,
 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967,
 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011,
 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087,
 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167,
 5171, 5179, 5189, 5197, 5209, 5227, 5231, 5233,
 5237, 5261, 5273, 5279, 5281, 5297, 5303, 5309,
 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399,
 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443,
 5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507,
 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573,
 5581, 5591, 5623, 5639, 5641, 5647, 5651, 5653,
 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711,
 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791,
 5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849,
 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897,
 5903, 5923, 5927, 5939, 5953, 5981, 5987, 6007,
 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073,
 6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133,
 6143, 6151, 6163, 6173, 6197, 6199, 6203, 6211,
 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271,
 6277, 6287, 6299, 6301, 6311, 6317, 6323, 6329,
 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379,
 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473,
 6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563,
 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637,
 6653, 6659, 6661, 6673, 6679, 6689, 6691, 6701,
 6703, 6709, 6719, 6733, 6737, 6761, 6763, 6779,
 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833,
 6841, 6857, 6863, 6869, 6871, 6883, 6899, 6907,
 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971,
 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027,
 7039, 7043, 7057, 7069, 7079, 7103, 7109, 7121,
 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207,
 7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253,
 7283, 7297, 7307, 7309, 7321, 7331, 7333, 7349,
 7351, 7369, 7393, 7411, 7417, 7433, 7451, 7457,
 7459, 7477, 7481, 7487, 7489, 7499, 7507, 7517,
 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561,
 7573, 7577, 7583, 7589, 7591, 7603, 7607, 7621,
 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691,
 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757,
 7759, 7789, 7793, 7817, 7823, 7829, 7841, 7853,
 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919,
 7927, 7933, 7937, 7949, 7951, 7963, 7993, 8009,
 8011, 8017, 8039, 8053, 8059, 8069, 8081, 8087,
 8089, 8093, 8101, 8111, 8117, 8123, 8147, 8161,
 8167, 8171, 8179, 8191, 
#endif	/* BIGSIEVE */
#endif	/* not EMBEDDED, use larger table */
 0 } ;	/* null-terminated list, with only one null at end */

/* Write a public key packet out to a fifo */
void pgpk_create_pk(struct fifo *f,struct pgp_pubkey *pk)
{
word16 length;
time_t timestamp=pk->timestamp;
word16 validity=pk->validity;
endian(&timestamp,4);
endian(&validity,2);
length=pk->n.bytes+pk->e.bytes+12;
endian(&length,2);
fifo_put(PGP_PK|1,f);
fifo_aput(&length,2,f);
fifo_put('\x02',f);
fifo_aput(&timestamp,4,f);
fifo_aput(&validity,2,f);
fifo_put('\x01',f);
pgp_create_mpi(f,&pk->n);
pgp_create_mpi(f,&pk->e);
}

/* Write a secret key out to a fifo, encrypting it with ideakey
   or leaving it unencrypted if ideakey is NULL */
void pgpk_create_sk(struct fifo *f,struct pgp_pubkey *pk,
	     struct pgp_seckey *osk,unsigned ideakey[16])
{
struct pgp_seckey *sk=safemalloc(sizeof(struct pgp_seckey));
word16 length;
word16 validity=pk->validity;
word16 iv[4]={0,0,0,0};
word16 checksum=0;
word16 bits;
time_t timestamp=pk->timestamp;
unsigned i;
struct mpi *mpis[4];
struct mpi *cmpi;
byte *cp;

endian(&timestamp,4);
endian(&validity,2);
memcpy(sk,osk,sizeof(struct pgp_seckey));
length=pk->n.bytes+pk->e.bytes+sk->d.bytes+sk->p.bytes+
       sk->q.bytes+sk->u.bytes+23+(ideakey?8:0);
endian(&length,2);
fifo_put(PGP_SK|1,f);
fifo_aput(&length,2,f);
fifo_put('\x02',f);
fifo_aput(&timestamp,4,f);
fifo_aput(&validity,2,f);
fifo_put('\x01',f);
pgp_create_mpi(f,&pk->n);
pgp_create_mpi(f,&pk->e);
if(ideakey) {
  for(i=0;i<8;i++)
    sk->iv[i]=pgp_randombyte();
  initcfb_idea(iv,ideakey,FALSE);
  ideacfb(sk->iv,8);
  }
mpis[0]=&sk->d;
mpis[1]=&sk->p;
mpis[2]=&sk->q;
mpis[3]=&sk->u;
for(i=0;i<4;i++) {
  cmpi=mpis[i];
  cp=(byte *)cmpi->value;
  endian(cp,sizeof(unitarr));
  cp+=(sizeof(unitarr)-cmpi->bytes);
  bits=cmpi->bits;
  endian(&bits,2);
  pgp_checksum(&bits,2,&checksum);
  pgp_checksum(cp,cmpi->bytes,&checksum);
  if(ideakey) 
    ideacfb(cp,cmpi->bytes);
  endian(cmpi->value,sizeof(unitarr));
  }
if(ideakey) {
  close_idea();
  fifo_put('\x01',f);
  fifo_aput(sk->iv,8,f);
  }
else
  fifo_put('\x00',f);
pgp_create_mpi(f,&sk->d);
pgp_create_mpi(f,&sk->p);
pgp_create_mpi(f,&sk->q);
pgp_create_mpi(f,&sk->u);
endian(&checksum,2);
fifo_aput(&checksum,2,f);
free(sk);
}

void pgpk_keygen(struct pgp_pubkey *pk,struct pgp_seckey *sk,
		 unsigned nbits,unsigned ebits,time_t timestamp,
		 word16 validity,void (*xout)() )
{
byte *cp;
output=xout;
rsa_keygen(pk->n.value,pk->e.value,sk->d.value,
	   sk->p.value,sk->q.value,sk->u.value,nbits,ebits);
pgp_fix_mpi(&pk->n);
pgp_fix_mpi(&pk->e);
pgp_fix_mpi(&sk->d);
pgp_fix_mpi(&sk->p);
pgp_fix_mpi(&sk->q);
pgp_fix_mpi(&sk->u);
pk->timestamp=timestamp;
pk->validity=validity;
endian(&pk->n.value,sizeof(unitarr));
cp=(byte *) &pk->n.value;
cp+=sizeof(unitarr)-8;
memcpy(pk->keyid,cp,8);
endian(&pk->n.value,sizeof(unitarr));
}

/* Strong primes not implemented (were turned off anyway) */
void rsa_keygen(unitptr n, unitptr e, unitptr d,
	unitptr p, unitptr q, unitptr u, short keybits, short ebits)
/* Generate RSA key components p, q, n, e, d, and u. 
   This routine sets the global_precision appropriate for n,
   where keybits is desired precision of modulus n.
   The precision of exponent e will be >= ebits.
   It will generate a p that is < q.
*/
{	short pbits,qbits,separation;
	boolean too_close_together; /* TRUE iff p and q are too close */
	int slop;

	/* Don't let keybits get any smaller than 2 units, because	
	   some parts of the math package require at least 2 units 
	   for global_precision.
	   Nor any smaller than the 32 bits of preblocking overhead.
	   Nor any bigger than MAX_BIT_PRECISION - SLOP_BITS.
	   Also, if generating "strong" primes, don't let keybits get
	   any smaller than 64 bits, because of the search latitude.
	*/
	slop = max(SLOP_BITS,1); 
	/* allow at least 1 slop bit for sign bit */
	keybits = min(keybits,(MAX_BIT_PRECISION-slop));
	keybits = max(keybits,UNITSIZE*2);
	keybits = max(keybits,32); /* minimum preblocking overhead */
#ifdef WHOLEWORD_KEY	/* some modmults run faster this way */
	/* Some modmult algorithms run faster if both primes and 
	   the modulus are an exact multiple of UNITSIZE bits long,
	   in other words, they completely fill the most significant 
	   unit.  So we will "round up" keybits to the next multiple 
	   of UNITSIZE*2.
	*/
#define roundup(x,m) (((x)+(m)-1)/(m))*(m)
	keybits = roundup(keybits,UNITSIZE*2);
	if (keybits==MAX_BIT_PRECISION)
		/* allow head room for sign bit */
		keybits -= UNITSIZE*2;
#endif	/* WHOLEWORD_KEY */

	set_precision(bits2units(keybits + slop));

	/* separation is the minimum number bits of difference in the 
	   sizes of p and q. 
	*/
#ifdef MERRITT_KEY	/* using Merritt's modmult algorithm */
	separation = 2;
#else	/* not MERRITT_KEY */
	separation = 0;
#endif	/* not MERRITT_KEY */
	pbits = (keybits-separation)/2;
	qbits = keybits - pbits;

#ifdef MERRITT_KEY
	/* During decrypt, the primes p and q's bit length should 
	   not be an exact multiple of UNITSIZE, because Merritt's 
	   modmult algorithm performs slowest in that case, wasting 
	   an extra unit of precision for overflow.
	   Other modmult algorithms perform differently.
	   Some other modmults actually performs fastest when the 
	   modulus and primes p and q exactly fill the MS unit.
	*/
	{	short qtrim;
		qtrim = (qbits % UNITSIZE)+1; 
		/* how many bits to trim from q */
		if (qtrim <= (separation/2))
			pbits += qtrim; 
			/* allows qbits to be a bit shorter */
	}
	if ((pbits % UNITSIZE)==0)
		/* inefficient to exactly fill a word */ 
		pbits -= 1;
			/* one bit shorter speeds up modmult a lot. */
#endif	/* MERRITT_KEY */
	stage=0;
	if(output)
	  (*output)(stage);
	randomprime(p,pbits);

	/* We now have prime p.  Now generate q such that q>p... */

	qbits = keybits - countbits(p);

#ifdef MERRITT_KEY
	if ((qbits % UNITSIZE)==0)
		/* inefficient to exactly fill a word */ 
		qbits -= 1;
			/* one bit shorter speeds up modmult a lot. */
#endif	/* MERRITT_KEY */

	do
	/* Generate a q until we get one that isn't too close to p. */
	{	
		stage=2;
		if(output)
		  (*output)(stage);
		randomprime(q,qbits);

	 /* Note that at this point we can't be sure that q>p. */
	 /* See if p and q are far enough apart.  Is q-p big enough? */
		mp_move(u,q);	/* use u as scratchpad */
		mp_sub(u,p);	/* compute q-p */
		if (mp_tstminus(u))	/* p is bigger */
		{	mp_neg(u);
			too_close_together = (countbits(u) 
				 < (countbits(p)-7));
		}
		else		/* q is bigger */
			too_close_together = (countbits(u) 
				 < (countbits(q)-7));

	/* Keep trying q's until we get one far enough from p... */
	} while (too_close_together);

	/* In case sizes went awry in making p and q... */
	if (mp_compare(p,q) >= 0)
		/* ensure that p<q for computing u */
	{	mp_move(u,p);
		mp_move(p,q);
		mp_move(q,u);
	}
	if(output)
	  (*output)(4);
	derive_rsakeys(n,e,d,p,q,u,ebits);
}	/* rsa_keygen */

static void derive_rsakeys(unitptr n, unitptr e, unitptr d,
	unitptr p, unitptr q, unitptr u, short ebits)
/* Given primes p and q, derive RSA key components n, e, d, and u. 
   The global_precision must have already been set large enough for n.
   Note that p must be < q.
   Primes p and q must have been previously generated elsewhere.
   The bit precision of e will be >= ebits.  The search for a usable
   exponent e will begin with an ebits-sized number.  The recommended 
   value for ebits is 5, for efficiency's sake.  This could yield 
   an e as small as 17.
*/
{	unit *F=safemalloc(sizeof(unit)*MAX_UNIT_PRECISION);
	unitptr ptemp, qtemp, phi, G; 	/* scratchpads */

	ptemp = d;	/* use d for temporary scratchpad array */
	qtemp = u;	/* use u for temporary scratchpad array */
	phi = n;	/* use n for temporary scratchpad array */
	G = F;		/* use F for both G and F */
	
	if (mp_compare(p,q) >= 0) /* ensure that p<q for computing u */
		swap(p,q);	  /* swap the pointers p and q */

	/* phi(n) is the Euler totient function of n, or the number of
	   positive integers less than n that are relatively prime to n.
	   G is the number of "spare key sets" for a given modulus n. 
	   The smaller G is, the better.  The smallest G can get is 2. 
	*/
	mp_move(ptemp,p); mp_move(qtemp,q);
	mp_dec(ptemp); mp_dec(qtemp);
	mp_mult(phi,ptemp,qtemp);	/*  phi(n) = (p-1)*(q-1)  */
	mp_gcd(G,ptemp,qtemp);		/*  G(n) = gcd(p-1,q-1)  */
	if(output)
	  (*output)(4);

#ifdef DEBUG
	if (countbits(G) > 12)	/* G shouldn't get really big. */
		mp_display("\007G = ",G); /* Worry the user. */
#endif /* DEBUG */
	mp_udiv(ptemp,qtemp,phi,G);	/* F(n) = phi(n)/G(n)  */
	if(output)
	  (*output)(4);
	mp_move(F,qtemp);

	/* We now have phi and F.  Next, compute e...
	   Strictly speaking, we might get slightly faster results by
	   testing all small prime e's greater than 2 until we hit a 
	   good e.  But we can do just about as well by testing all 
	   odd e's greater than 2.
	   We could begin searching for a candidate e anywhere, perhaps
	   using a random 16-bit starting point value for e, or even
	   larger values.  But the most efficient value for e would be 3, 
	   if it satisfied the gcd test with phi.
	   Parameter ebits specifies the number of significant bits e
	   should have to begin search for a workable e.
	   Make e at least 2 bits long, and no longer than one bit 
	   shorter than the length of phi.
	*/
	ebits = min(ebits,countbits(phi)-1);
	if (ebits==0) ebits=5;	/* default is 5 bits long */
	ebits = max(ebits,2);
	mp_init(e,0);
	mp_setbit(e,ebits-1);
	lsunit(e) |= 1;	/* set e candidate's lsb - make it odd */
	mp_dec(e);  mp_dec(e); /* precompensate for preincrements of e */
	do
	{	mp_inc(e); mp_inc(e);
		/* try odd e's until we get it. */
		mp_gcd(ptemp,e,phi); 
	       /* look for e such that gcd(e,phi(n)) = 1 */
	} while (testne(ptemp,1));
	if(output)
	  (*output)(4);
	/* Now we have e.  Next, compute d, then u, then n.
	   d is the multiplicative inverse of e, mod F(n).
	   u is the multiplicative inverse of p, mod q, if p<q.
	   n is the public modulus p*q.
	*/
	mp_inv(d,e,F);	/* compute d such that (e*d) mod F(n) = 1 */
	if(output)
	  (*output)(4);
	mp_inv(u,p,q);		/* (p*u) mod q = 1, assuming p<q */
	if(output)
	  (*output)(4);
	mp_mult(n,p,q);	/*  n = p*q  */
	if(output)
	  (*output)(4);
	mp_burn(F);		/* burn the evidence on the stack */
	free(F);
}	/* derive_rsakeys */

/* Makes a "random" prime p with nbits significant bits of precision.
   Since these primes are used to compute a modulus of a guaranteed 
   length, the top 2 bits of the prime are set to 1, so that the
   product of 2 primes (the modulus) is of a deterministic length.
*/
void randomprime(unitptr p,short nbits)
{
unsigned i;
short bitsleft=nbits;
byte *cp=(byte *) p;
for(i=0;i<sizeof(unitarr);i++)
  cp[i]='\0'; /* Can't use mp_burn; precision is unknown */
cp+=sizeof(unitarr); /* Start at the end, assuming big-endianness */
while(bitsleft>7) {
  *(--cp)=pgp_randombyte();
  bitsleft-=8;
  }
if(bitsleft) { /* Odd bits */
  *(--cp)=pgp_randombyte();
  *cp >>= (8-bitsleft);
  }
endian(p,sizeof(unitarr)); /* swap it */
mp_setbit(p,nbits-1);	/* highest bit is nonrandom */
mp_setbit(p,nbits-2);	/* next highest bit is also nonrandom */
nextprime(p); /* search for next higher prime from starting point p */
}

static void nextprime(unitptr p)
/* Find next higher prime starting at p, returning result in p. 
   Uses fast prime sieving algorithm to search sequentially.
*/
{	word16 pdelta, range;
	short oldprecision;
	short i, suspects;

	/* start search at candidate p */
	mp_inc(p);
	 /* remember, it's the NEXT prime from p, noninclusive. */
	if (significance(p) <= 1) 
	{ /*	p might be smaller than the largest prime in primetable.
		We can't sieve for primes that are already in primetable.
		We will have to directly search the table.
	  */
		for (i=0; primetable[i]; i++)
		 /* scan until null-terminator */
		{	if (primetable[i] >= lsunit(p))
			{	mp_init(p,primetable[i]);
				return;
		/* return next higher prime from primetable */
			}
		} /* We got past the whole primetable without a hit. */
	}  /* p is bigger than any prime in primetable, so let's sieve. */

	if (mp_tstminus(p))	/* error if p<0 */
	{	mp_init(p,2);	/* next prime >0 is 2 */
		return;	/* normal completion status */
	}

#ifndef BLUM
	lsunit(p) |= 1;		/* set candidate's lsb - make it odd */
#else
	lsunit(p) |= 3;		/* Make candidate ==3 mod 4 */
#endif

/* Adjust the global_precision downward to the optimum size for p...*/
	oldprecision = global_precision;  /* save global_precision */
   /* We will need 2-3 extra bits of precision for the falsekeytest. */
	set_precision(bits2units(countbits(p)+4+SLOP_BITS));
	/* Rescale p to global_precision we just defined */
	rescale(p,oldprecision,global_precision);

	{
#ifdef _NOMALLOC /* No malloc and free functions available.  Use stack. */
		word16 remainders[numberof(primetable)];
#else	/* malloc available, we can conserve stack space. */
		word16 *remainders;
		/* Allocate some memory for the table of remainders: */
		/* Whoever originally wrote this forgot to check for a
	    null pointer from malloc...a good reason to use safemalloc */
		remainders = (word16 *) safemalloc(sizeof(primetable));
#endif	/* malloc available */

		/* Build remainders table relative to initial p: */
		buildsieve(p,remainders);
		pdelta = 0;	/* offset from initial random p */
      /* Sieve preparation complete.  Now for some fast fast sieving...*/
		/* slowtest will not be called unless fastsieve is true */

		/* range is how far to search before giving up. */
#ifndef BLUM
		range = 4 * units2bits(global_precision);
#else
		/* Twice as many because step size is twice as large, */
		range = 8 * units2bits(global_precision);
#endif
		suspects = 0;
		/* number of suspected primes and slowtest trials */
		while (TRUE)
		{
			if(output)
			  (*output)(stage);
			/* equivalent to:  if (primetest(p)) break; */
			if (fastsieve(pdelta,remainders))
				/* found suspected prime */
			{	suspects++;	
				/* tally for statistical purposes */
				if (slowtest(p))
					break;
					 /* found a prime */
			}
#ifndef BLUM
			pdelta += 2;	/* try next odd number */
#else
			pdelta += 4;
			mp_inc(p); mp_inc(p);
#endif
			mp_inc(p); mp_inc(p);

     /*			if (pdelta > range)        */
				/* searched too many candidates? */ 
   /*	   break;  */	/* something must be wrong--bail out of search */

		}	/* while (TRUE) */
		for (i=0; primetable[i]; i++)
		 /* scan until null-terminator */
			remainders[i] = 0; 
			/* don't leave remainders exposed in RAM */
#ifndef _NOMALLOC
		free(remainders);		/* free allocated memory */
#endif	/* not _NOMALLOC */
	}
	set_precision(oldprecision);	/* restore precision */
	return;		/* return normal completion status */
}	/* nextprime */

static boolean slowtest(unitptr p)
/* This routine tests p for primality by applying Fermat's theorem:
   For any x, if ((x**(p-1)) mod p) != 1, then p is not prime.
   By trying a few values for x, we can determine if p is "probably" prime.

   Because this test is so slow, it is recommended that p be sieved first
   to weed out numbers that are obviously not prime.

   Contrary to what you may have read in the literature, empirical evidence
   shows this test weeds out a LOT more than 50% of the composite candidates
   for each trial x.  Each test catches nearly all the composites.
*/
{	unit *x=safemalloc(sizeof(unit)*MAX_UNIT_PRECISION);
	unit *is_one=safemalloc(sizeof(unit)*MAX_UNIT_PRECISION);
	unit *pminus1=safemalloc(sizeof(unit)*MAX_UNIT_PRECISION);
	short i;
	byte result;

	mp_move(pminus1,p);
	mp_dec(pminus1);

	for (i=0; i<4; i++)		/* Just do a few tests. */
	{    /*	poll_for_break(); */ 
	/* polls keyboard, allows ctrl-C to abort program */
	/* Commented out. Break, if desired, can be done during output() */
		if(output)
		  (*output)(stage|1);
		mp_init(x,primetable[i]);
			/* Use any old random trial x */
		/* if ((x**(p-1)) mod p) != 1, then p is not prime */
		if (mp_modexp(is_one,x,pminus1,p) < 0)
			/* modexp error? */
			{ result=FALSE; goto done; } /* return(FALSE); */	
			/* error means return not prime status */
		if (testne(is_one,1))	/* then p is not prime */
			{ result=FALSE; goto done; }
			/* return(FALSE); */
			/* return not prime status */
	}

	/* If it gets to this point, it's very likely that p is prime */
	mp_burn(x);		/* burn the evidence on the stack...*/
	mp_burn(is_one);
	mp_burn(pminus1);
	/* return(TRUE); */ result=TRUE;

	done: /* Free memory before exiting */
	free(x);
	free(is_one);
	free(pminus1);
	return(result);
}	/* slowtest -- fermattest */

static void buildsieve(unitptr p, word16 remainders[])
/* Used in conjunction with fastsieve.  Builds a table of remainders 
   relative to the random starting point p, so that fastsieve can 
   sequentially sieve for suspected primes quickly.  Call buildsieve 
   once, then call fastsieve for consecutive prime candidates.
   Note that p must be odd, because the sieve begins at 3. 
*/
{	short i;
	for (i=1; primetable[i]; i++)
	{	remainders[i] = mp_shortmod(p,primetable[i]); 
	}
}	/* buildsieve */

/*
  Fast prime sieving algorithm by Philip Zimmermann, March 1987.
  This is the fastest algorithm I know of for quickly sieving for 
  large (hundreds of bits in length) "random" probable primes, because 
  it uses only single-precision (16-bit) arithmetic.  Because rigorous 
  prime testing algorithms are very slow, it is recommended that 
  potential prime candidates be quickly passed through this fast 
  sieving algorithm first to weed out numbers that are obviously not 
  prime.

  This algorithm is optimized to search sequentially for a large prime 
  from a random starting point.  For generalized nonsequential prime 
  testing, the slower	conventional sieve should be used, as given 
  in primetest(p).

  This algorithm requires a fixed table (called primetable) of the 
  first hundred or so small prime numbers. 
  First we select a random odd starting point (p) for our prime 
  search.  Then we build a table of 16-bit remainders calculated 
  from that initial p.  This table of 16-bit remainders is exactly 
  the same length as the table of small 16-bit primes.  Each 
  remainders table entry contains the remainder of p divided by the 
  corresponding primetable entry.  Then we begin sequentially testing 
  all odd integers, starting from the initial odd random p.  The 
  distance we have searched from the huge random starting point p is 
  a small 16-bit number, pdelta.  If pdelta plus a remainders table 
  entry is evenly divisible by the corresponding primetable entry, 
  then p+pdelta is factorable by that primetable entry, which means 
  p+pdelta is not prime.
*/

static boolean fastsieve(word16 pdelta, word16 remainders[])
/* Fastsieve is used for searching sequentially from a random starting
   point for a suspected prime.  Requires that buildsieve be called 
   first, to build a table of remainders relative to the random starting 
   point p.  

   Returns true iff pdelta passes through the sieve and thus p+pdelta 
   may be a prime.  Note that p must be odd, because the sieve begins 
   at 3.
*/
{	short i;
	for (i=1; primetable[i]; i++)
	{	/* If pdelta plus a remainders table entry is evenly 
		   divisible by the corresponding primetable entry,
		   then p+pdelta is factorable by that primetable entry, 
		   which means p+pdelta is not prime.
		*/
		if (( (pdelta + remainders[i]) % primetable[i] ) == 0)
			return(FALSE);	/* then p+pdelta is not prime */
	}
	/* It passed the sieve.  It is now a suspected prime. */
		return(TRUE);
}	/* fastsieve */

void mp_gcd(unitptr result,unitptr a,unitptr n)
	/* Computes greatest common divisor via Euclid's algorithm. */
{	short i;
/*	unit gcopies[3][MAX_UNIT_PRECISION]; (stack space hog) */
	unit *gcopies[3];
/* #define g(i) (  &(gcopies[i][0])  ) */
for(i=0;i<3;i++)
  gcopies[i]=safemalloc(sizeof(unit)*MAX_UNIT_PRECISION);
#define g(i) gcopies[i]
	mp_move(g(0),n);
	mp_move(g(1),a);
	
	i=1;
	while (testne(g(i),0))
	{	mp_mod( g(iplus1),g(iminus1),g(i) );
		i = iplus1;
	}
	mp_move(result,g(iminus1));
	mp_burn(g(iminus1));	/* burn the evidence on the stack...*/
	mp_burn(g(iplus1));
#undef g
for(i=0;i<3;i++)
  free(gcopies[i]);
}	/* mp_gcd */

void mp_inv(unitptr x,unitptr a,unitptr n)
	/* Euclid's algorithm extended to compute multiplicative inverse.
	   Computes x such that a*x mod n = 1, where 0<a<n */
{
	/*	The variable u is unnecessary for the algorithm, but is 
		included in comments for mathematical clarity. 
	*/
	short i;
/*	unit y[MAX_UNIT_PRECISION], temp[MAX_UNIT_PRECISION];
	unit gcopies[3][MAX_UNIT_PRECISION], vcopies[3][MAX_UNIT_PRECISION];
#define g(i) (  &(gcopies[i][0])  )
#define v(i) (  &(vcopies[i][0])  )
    Major stack space hog! */
unit *y=safemalloc(sizeof(unit)*MAX_UNIT_PRECISION);
unit *temp=safemalloc(sizeof(unit)*MAX_UNIT_PRECISION);
unit *gcopies[3];
unit *vcopies[3];
for(i=0;i<3;i++) {
  gcopies[i]=safemalloc(sizeof(unit)*MAX_UNIT_PRECISION);
  vcopies[i]=safemalloc(sizeof(unit)*MAX_UNIT_PRECISION);
  }
#define g(i) gcopies[i]
#define v(i) vcopies[i]  

/*	unit ucopies[3][MAX_UNIT_PRECISION]; */
/* #define u(i) (  &(ucopies[i][0])  ) */
	mp_move(g(0),n); mp_move(g(1),a);
/*	mp_init(u(0),1); mp_init(u(1),0); */
	mp_init(v(0),0); mp_init(v(1),1);
	i=1;
	while (testne(g(i),0))
	{	/* we know that at this point,  g(i) = u(i)*n + v(i)*a  */	
		mp_udiv( g(iplus1), y, g(iminus1), g(i) );
		mp_mult(temp,y,v(i));
		mp_move(v(iplus1),v(iminus1));
		mp_sub(v(iplus1),temp);
	/*	mp_mult(temp,y,u(i));
		mp_move(u(iplus1),u(iminus1));
		mp_sub(u(iplus1),temp); */
		i = iplus1;
	}
	mp_move(x,v(iminus1));
	if (mp_tstminus(x))
		mp_add(x,n);
	mp_burn(g(iminus1));	/* burn the evidence on the stack...*/
	mp_burn(g(iplus1));
	mp_burn(v(0));
	mp_burn(v(1));
	mp_burn(v(2));
	mp_burn(y);
	mp_burn(temp);
#undef g
#undef v
free(y);
free(temp);
for(i=0;i<3;i++) {
  free(gcopies[i]);
  free(vcopies[i]);
  }
}	/* mp_inv */
