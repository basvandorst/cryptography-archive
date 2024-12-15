/**********************************************************************
 source/rnd.c: random number generator

   Copyright (c) 1993 Carl M. Ellison

   This software may be copied and distributed for non-profit purposes
   provided that this copyright and statement are included in all such
   copies.

   This software implements the subtract-with-borrow generator described by
   George Marsaglia and Arif Zaman, "A New Class of Random Number
   Generators", The Annals of Applied Probability, 1991, Vol 1, #3,
   462-480.

   The routine set_rnd_seed expects to be called with an array of 256
   integers (e.g., a histogram of byte values from a block).  It
   initializes the generator using a dot product between the histogram and
   a preset random vector for each of the 48 seed values.  Rather than
   keep 48 full random vectors of length 256, this uses vectors offset
   by 10 values -- using an array of 800 random numbers.

   Those random numbers were from a hardware random number generator.

   The routine rnd() provides an integer in the range 0..(2^31)-1.

**********************************************************************/

#define B_MASK  (0x7fffffff)  /* to do {mod (2^31)} by AND */


/* seed */
static unsigned long  ran_arr[48] ;
static long r1 ;		/* reader index 1 */
static long r2 ;		/* reader index 2 */
static long carry ;		/* carry value (boolean) */

/* returns a pseudo-random number in the range 0..(2^31)-1 */

unsigned long rnd()
{
  register unsigned long v ;	/* working value */

  if ((++r1)==48) r1 = 0 ;	/* advance r1 - for x_{n-48} */
  if ((++r2)==48) r2 = 0 ;	/* advance r2 - for x_{n-8} */

  v = ran_arr[r1] - ran_arr[r2] - carry ; /* compute x_n */
  carry = (v > B_MASK) ;	/* compute the new carry bit */
  v &= B_MASK ;			/* take the modulus */
  ran_arr[r1] = v ;		/* record x_n */

  return v ;			/* return the new ranno */
}

static long initab[800] = {	/* vector of lth 256, with start point
				   stepped by 10 with each new output value */

0x3462f, 0x009aa, 0x2e383, 0x137a9, 0x07247, 0x35b20, 0x064f0, 0x2ea03,
0x35b39, 0x36d0f, 0x05f58, 0x14502, 0x20488, 0x0f7a0, 0x25b4c, 0x02d91,
0x195da, 0x2f5e5, 0x26817, 0x05c4f, 0x2277a, 0x317b7, 0x1a409, 0x1aff1,
0x29787, 0x1888a, 0x1f9da, 0x18e81, 0x0dfda, 0x3a3d2, 0x3d0e8, 0x2e6fa,
0x2e646, 0x3eab5, 0x0edc2, 0x22ed0, 0x17a30, 0x1da42, 0x19d93, 0x256e4,
0x39d35, 0x01121, 0x19f5d, 0x31d88, 0x3bb76, 0x347c7, 0x3c340, 0x1787c,
0x16b22, 0x34d38, 0x3e785, 0x1f85d, 0x0fed6, 0x2a96f, 0x29a66, 0x17759,
0x3244c, 0x3ee64, 0x26aa5, 0x2bf08, 0x041f2, 0x3789c, 0x32f7a, 0x0cd06,
0x3b933, 0x3881a, 0x260a5, 0x0f582, 0x1fa67, 0x13296, 0x2b2b6, 0x16950,
0x0fe67, 0x1ae59, 0x10aa7, 0x1292e, 0x16960, 0x1e0bb, 0x31057, 0x3a5e0,
0x2e0d5, 0x1b77f, 0x39dfe, 0x3b991, 0x3bd51, 0x1f16b, 0x0555a, 0x0fd11,
0x342a8, 0x2860f, 0x37f4f, 0x1e122, 0x2109e, 0x1c4be, 0x3eb7b, 0x1599f,
0x0b5fb, 0x28ea5, 0x1d3ba, 0x0c51a, 0x0bca9, 0x1d827, 0x3fe84, 0x0c2d4,
0x29c7c, 0x2853f, 0x1af59, 0x39d96, 0x0c19c, 0x2494c, 0x14add, 0x369c0,
0x03dcd, 0x04b0b, 0x09206, 0x2f179, 0x15c9d, 0x05293, 0x39a5e, 0x2acb5,
0x0f4f4, 0x226bb, 0x04d83, 0x2e243, 0x1ee88, 0x0e321, 0x07a5d, 0x00ef5,
0x27fc8, 0x2b2ad, 0x38b77, 0x27928, 0x01aa1, 0x182a6, 0x3ed24, 0x301d7,
0x1ffe2, 0x0ef1a, 0x2cf21, 0x181b7, 0x0a09b, 0x3ec4c, 0x2a1f8, 0x0a98e,
0x308ad, 0x2d701, 0x23b06, 0x005cd, 0x301db, 0x155fd, 0x3bc1f, 0x2d32f,
0x394a0, 0x3c326, 0x05d5a, 0x3e731, 0x21a1c, 0x19cf0, 0x28d67, 0x0ca4a,
0x3717d, 0x27a09, 0x190db, 0x00a99, 0x38993, 0x3f4a1, 0x04b6e, 0x2ae6e,
0x147aa, 0x38a8e, 0x1f87a, 0x02715, 0x3a4ea, 0x12839, 0x26697, 0x0d2bc,
0x096d5, 0x23ddf, 0x00075, 0x39802, 0x17707, 0x39dc4, 0x0cbbc, 0x37c34,
0x3b5ad, 0x22f81, 0x1a195, 0x3c430, 0x17229, 0x001a1, 0x374cf, 0x0beb2,
0x01c64, 0x118b9, 0x35e27, 0x0469f, 0x02fb2, 0x13d66, 0x34829, 0x1930f,
0x1fc50, 0x3a375, 0x2aed1, 0x0a342, 0x32bb3, 0x27bf6, 0x16a9f, 0x3a934,
0x1ffc3, 0x08079, 0x1568c, 0x35ca8, 0x15357, 0x141af, 0x3326f, 0x2debb,
0x3812d, 0x3935a, 0x38eb9, 0x15694, 0x19d1c, 0x1a570, 0x12233, 0x2773f,
0x1bc3e, 0x3e96c, 0x2ab52, 0x14b62, 0x1e89f, 0x00a83, 0x3f04e, 0x18e5c,
0x107b4, 0x39fa8, 0x0cc8b, 0x33a46, 0x3a908, 0x2c6e5, 0x31df2, 0x12ef3,
0x177c6, 0x1159f, 0x00c00, 0x3a342, 0x1a2f7, 0x14170, 0x08fa4, 0x1c480,
0x177b9, 0x048c8, 0x30dc8, 0x16525, 0x3cc7f, 0x38c1d, 0x2187e, 0x19205,
0x2490d, 0x367cf, 0x29260, 0x20f59, 0x2e943, 0x0afe7, 0x2d5ee, 0x329b8,
0x14be3, 0x3d842, 0x26a86, 0x140e9, 0x12de4, 0x25462, 0x13fe6, 0x302b1,
0x20eea, 0x19cc5, 0x26e51, 0x1faf4, 0x1ab5c, 0x2f756, 0x2e1b9, 0x0955e,
0x13b48, 0x209bb, 0x1ba17, 0x2cc4e, 0x17fc3, 0x19887, 0x20fc1, 0x3c09d,
0x3add6, 0x254e3, 0x3e26a, 0x13989, 0x18739, 0x34676, 0x177c9, 0x138bf,
0x201e8, 0x38ef5, 0x16136, 0x1ab86, 0x0176e, 0x1a7bd, 0x36787, 0x14ccb,
0x2bf30, 0x371b3, 0x0a019, 0x0d4b8, 0x2374c, 0x23948, 0x3e691, 0x379d5,
0x299cf, 0x0e46d, 0x26fe6, 0x15bbf, 0x364f1, 0x036c9, 0x39aea, 0x30889,
0x28a8c, 0x311ab, 0x088f3, 0x2f459, 0x307d6, 0x0910f, 0x1ca45, 0x16c28,
0x17a1d, 0x1eac8, 0x17522, 0x11eb5, 0x0da8f, 0x2c8e3, 0x25eb9, 0x0867b,
0x08e2b, 0x0493b, 0x08639, 0x3f797, 0x0603e, 0x29c78, 0x2bf01, 0x36e96,
0x3c3d2, 0x30cb1, 0x1f2c4, 0x33f06, 0x2081d, 0x16f20, 0x206e4, 0x346f1,
0x0ede0, 0x09668, 0x30fbe, 0x12299, 0x3b654, 0x1d221, 0x16b5c, 0x37990,
0x1e26b, 0x0a791, 0x2e3f5, 0x01a9d, 0x2f474, 0x209ed, 0x1fe1f, 0x0a692,
0x28e18, 0x01230, 0x0e5e2, 0x3e427, 0x226f9, 0x023fb, 0x0a367, 0x05d37,
0x39541, 0x0d778, 0x2282c, 0x0a688, 0x256b5, 0x13146, 0x3f5ba, 0x1da29,
0x2bdd2, 0x228e3, 0x16f09, 0x01447, 0x1b70d, 0x2a14b, 0x2d6e3, 0x3179e,
0x35f47, 0x3c648, 0x1f472, 0x3ef05, 0x3286a, 0x32ff1, 0x0e056, 0x0100d,
0x07530, 0x09663, 0x3001d, 0x367ee, 0x3ca83, 0x07ee1, 0x20827, 0x3f9bf,
0x3c577, 0x1d277, 0x2f920, 0x0fac9, 0x0bd05, 0x2582e, 0x20473, 0x2108a,
0x1e0cf, 0x163cc, 0x05def, 0x064ef, 0x2b04e, 0x1fcc0, 0x07367, 0x228d1,
0x368a0, 0x1861c, 0x28523, 0x0b3bb, 0x0cde5, 0x0d8df, 0x21dbf, 0x184ab,
0x1853e, 0x1c0d9, 0x114d8, 0x1afdb, 0x0fec6, 0x1b6d0, 0x0f345, 0x33cff,
0x0f7da, 0x21c40, 0x033b6, 0x21afa, 0x24a32, 0x1e50d, 0x27528, 0x2a116,
0x35638, 0x269bf, 0x10092, 0x23c84, 0x0dc90, 0x292e9, 0x38e0d, 0x18887,
0x0493a, 0x30c3e, 0x23cc5, 0x15593, 0x35c34, 0x2d15c, 0x13812, 0x38d1c,
0x18f0e, 0x07d33, 0x21081, 0x39842, 0x2262d, 0x2bd66, 0x20842, 0x2d3c7,
0x38998, 0x09ced, 0x02049, 0x3346a, 0x2d84b, 0x195ea, 0x092a0, 0x229dd,
0x375b3, 0x1a02e, 0x14746, 0x24366, 0x332ff, 0x10837, 0x12a18, 0x148c3,
0x03bae, 0x3ab4d, 0x04606, 0x1dc7f, 0x135bc, 0x266c4, 0x3448b, 0x37751,
0x3df71, 0x3085a, 0x1ea14, 0x2e1aa, 0x3cfea, 0x21dfd, 0x25d3d, 0x22b5f,
0x060c5, 0x37bdd, 0x2863a, 0x3f829, 0x34b5f, 0x3bfcc, 0x3cec1, 0x1fad9,
0x3c733, 0x34e97, 0x1fb4c, 0x33fa7, 0x0d01f, 0x18837, 0x0e1e8, 0x115f8,
0x2add2, 0x24f6a, 0x25c68, 0x16568, 0x1cc9b, 0x07459, 0x114d6, 0x25d3f,
0x15e49, 0x3eda2, 0x2dcb8, 0x0a25b, 0x0b08f, 0x1f646, 0x2b84c, 0x141d7,
0x3ec72, 0x2c95c, 0x32c9c, 0x39aef, 0x3a57c, 0x35210, 0x19cb1, 0x2bf2c,
0x1cae4, 0x3a0db, 0x02601, 0x3825a, 0x3c399, 0x159ee, 0x1ede5, 0x041ea,
0x0b780, 0x3ee6d, 0x3543e, 0x3e0a3, 0x11a4f, 0x0da04, 0x01fab, 0x0dd66,
0x125fd, 0x08188, 0x30d57, 0x26dc2, 0x101db, 0x35ee3, 0x179d6, 0x3122a,
0x3436e, 0x20327, 0x27f7f, 0x37a29, 0x1226e, 0x2a007, 0x02e5b, 0x37322,
0x06919, 0x20cf4, 0x3dd49, 0x20610, 0x010f8, 0x3b8d3, 0x19968, 0x0640d,
0x08ee7, 0x33fd9, 0x25cee, 0x363c1, 0x215e9, 0x2ef9c, 0x04469, 0x3d2e8,
0x3ff56, 0x16bfd, 0x1f7ce, 0x2d0ae, 0x2f1ee, 0x081ca, 0x1ec02, 0x1d107,
0x09625, 0x2ee32, 0x05964, 0x14b95, 0x2605d, 0x3ff77, 0x02ff9, 0x07476,
0x121bc, 0x39b51, 0x2259e, 0x19542, 0x3053e, 0x29df8, 0x2456e, 0x3a4e7,
0x0e8e4, 0x09e1f, 0x1b5ca, 0x18f04, 0x2b5f9, 0x0d8e5, 0x1e2a4, 0x0f291,
0x13864, 0x2dc16, 0x35dd2, 0x3d200, 0x061aa, 0x10c30, 0x1b975, 0x27da4,
0x1db12, 0x18ec9, 0x38cf3, 0x13cfa, 0x1dde9, 0x270cb, 0x1f6b5, 0x0e6c6,
0x128ab, 0x01384, 0x3df53, 0x08673, 0x25ca8, 0x3e209, 0x37375, 0x2c6fe,
0x0f4cb, 0x27edb, 0x0939c, 0x2fab1, 0x3ec06, 0x07fe7, 0x1d4c6, 0x0884e,
0x32d5b, 0x2bcab, 0x03eb3, 0x1fe65, 0x0fb90, 0x04e47, 0x3f8f8, 0x24b09,
0x3ad3f, 0x004e1, 0x1de0b, 0x2c583, 0x38cac, 0x0219d, 0x185fc, 0x1b222,
0x3bde4, 0x091bf, 0x3fda5, 0x384d3, 0x22dcd, 0x3289f, 0x2e78d, 0x14c9a,
0x25f9b, 0x1ee7b, 0x2aa11, 0x26aa6, 0x11d98, 0x1c704, 0x0f986, 0x0737f,
0x2734e, 0x0dfe8, 0x297f9, 0x1fdd4, 0x3db72, 0x1a7b8, 0x01e1b, 0x14dd7,
0x3a5cb, 0x3c8fa, 0x31ee8, 0x0109f, 0x0674b, 0x0a013, 0x0d96f, 0x1a401,
0x1c58c, 0x14f82, 0x3c114, 0x13895, 0x28e73, 0x3379f, 0x3d4c8, 0x0588d,
0x14fe0, 0x1f00d, 0x10ef0, 0x13756, 0x2da91, 0x2507d, 0x34446, 0x07d74,
0x3e52c, 0x2d4e9, 0x3a5e4, 0x2e904, 0x3ecab, 0x20bc2, 0x0b255, 0x3e9f5,
0x2e3f4, 0x3ba16, 0x2feaa, 0x230a2, 0x25ea0, 0x1091f, 0x30f13, 0x2825f,
0x2e452, 0x0e83a, 0x26f5e, 0x35b74, 0x064cb, 0x2aa85, 0x2496a, 0x28086,
0x1c394, 0x1a87d, 0x30f39, 0x1bec6, 0x26da3, 0x345b5, 0x21f4d, 0x36eef,
0x217ee, 0x032fa, 0x323b6, 0x3af97, 0x00191, 0x144de, 0x36f7e, 0x3982f,
0x3de33, 0x01dd8, 0x3b201, 0x15c6b, 0x17803, 0x32827, 0x27234, 0x0cd7f,
0x0e5df, 0x1d120, 0x27e4d, 0x207c7, 0x07aa2, 0x37fa1, 0x1409e, 0x3b4b1,
0x11935, 0x1ef82, 0x0d8eb, 0x08d15, 0x10621, 0x23ff3, 0x21cf7, 0x3ec13,
0x1a76c, 0x148c1, 0x2786c, 0x02a78, 0x24aaf, 0x2caeb, 0x3b9ed, 0x0d72d,
0x1d058, 0x16660, 0x25f62, 0x02fe5, 0x06ce1, 0x3582d, 0x2dda7, 0x19931

} ;				/* end of initab */

void set_rnd_seed (seedval)
long seedval[256];
{
  int i, j ;
  long  *it_base ;
  long v ;

  for (i=0;i<48;i++)
    {
      v = 0 ;
      it_base = &(initab[10*i]) ; /* step through the array of rannos */
      for (j=0;j<256;j++) v += seedval[j]*(*(it_base++)) ;
      ran_arr[i] = v & B_MASK ;
    } ;				/* end of for i */

  carry = 0 ;			/* start with no carry */
  r1 = 47 ;			/* n-48 = end of the array */
  r2 = r1 - 8 ;			/* n-8 */

  for ( i = 48*32 ; i > 0 ; i-- ) rnd() ; /* munch the array */
				/* i.e. get enough values so that */
				/* each LSB has affected the MSBs */
} /* end of set_rnd_seed */
