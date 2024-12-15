/*
 * A pair of Random number generators in which all bits returned are random
 *
 * unsigned short rand16() - 16 bit unsigned random number generator
 * unsigned long  rand32() - 32 bit unsigned random number generator
 *
 * void srand16(seed)
 *    unsigned short seed;
 *
 * void srand32(seed)
 *    unsigned long seed;
 */
/*
 * M = 2 ^ 32 
 * Spectral Test Scores: (v's are squares, roots shown on 2nd line)
 *
 *         A          v2      v3    v4    v5   v6    u2   u3   u4    u5   u6
 * 101619437  4855083850 2857326 70740  6526 1476  3.55 4.71 5.75  4.22 3.87
 *                 69678    1690   266    81   38
 *  53934261  4815268522 2772954 59602  6884 1466  3.52 4.50 4.08  4.82 3.79
 *                 69392    1665   244    83   38
 */
#define A (unsigned long) 101619437
#define C (unsigned long) 907633385

static unsigned long seed = C;

unsigned long srand16(newSeed)
   unsigned long newSeed;
{
   register unsigned long res = seed;

   seed = newSeed;
   return res;
}

unsigned short rand16()
{
   seed = A * seed + C;				/* implicit mod */
   return seed >> 16;
}

/*
 * M = 2 ^ 64				
 * A = 6364136223846793005		Taken from Knuth 2nd Ed
 * C = 3898255708540604107
 *
 * Spectral Test Scores (precision):
 *   v2 = 31.5 bits   
 *   v3 = 21.2 bits
 *   v4 = 16.0 bits
 *   v5 = 6757
 *   v6 = 1359
 */
#define A0  1284865837			/* low order part */
#define A1  1481765933			/* high order part */
#define A00 32557			/* low order part of low order part */
#define A01 19605			/* high order part of low order part */
#define C0  3207827147			/* low order part */
#define C1   907633385			/* high order part */
#define MM  1087140149			/* M - C0 */

static unsigned long seed0 = C0;	/* low order part */
static unsigned long seed1 = C1;	/* high order part */

void srand32(newSeed)
   unsigned long newSeed;
{
   seed0 = newSeed;
   seed1 = 0;
}

unsigned long rand32()
{
   register unsigned long  r1, temp;
   register unsigned short q0, q1;

   /* 
    * r1 = hi(A0 * seed0)
    */
   q0 = (unsigned short) seed0;
   q1 = seed0 >> 16;			

   temp  = A00 * q1    + A01 * q0;
   r1    = A01 * q1    + ((temp >> 16) & 0xffff);
   seed1 = A0  * seed1 + A1  * seed0 + r1 + C1;
   seed0 = A00 * q0    + ((temp & 0xffff) << 16);
   if (seed0 >= MM) {		/* carry */
      seed1++;
   }
   seed0 += C0;

   return seed1;
}
