/* Tables and initialization routines for the fast password transform.
 */


/* Big and little indians.
 * 	"The first tribe we met arranged themselves with the smallest
 *	 indian on the left and the biggest on the right.  The second
 *	 tribe used the reverse order.  They put the smallest indian on
 *	 the right.  Of course there was one tribe that arranged the
 *	 smallest half of their tribe on the left, but within each half
 *	 they put the bigger members towards the left."
 *
 *
 * The VAX tribe:
 *
 * Let us name the sixteen bytes that occupy addresses 1000 through 100F
 * (hex) in memory a through h.  That is the byte at address 1001 is
 * called b.  Successive byte fetches starting at 1000 would return the
 * values: a, b, c, ... , h.  Successive word fetches would return:  ba,
 * dc, fe, hg.  Where the least significant bit (LSB) of ba is the LSB of
 * a.  Successive long fetches would return: dcba, hgfe.  Where the most
 * significant bit (MSB) of dcba is the MSB of d.
 *
 * Bits are numbered 0 through 31 with 0 being the LSB.
 *
 *
 * The NBS tribe (related to IBM):
 *
 * Bits are numbered 1 through 64 and are written down with bit 1 leftmost
 * and bit 64 rigt most.  The variants on this are 48, 32, 6, and 4 bits
 * wide but they follow the same numbering.  In the case of 4 bit values
 * (S-box outputs), they are encoded by number from zero through sixteen.
 * This representation comes from the binary expansion of the sixteen
 * decimal values.  BUT the least significant bit of this expansion is
 * the leftmost bit, not the rightmost as in the vax.
 */

/* Bastard representations for speed.
 *
 * 
 * To speed up the des computation the 32 bit values in the NBS spec are
 * represented by 48 bits within the vax.  By doing this, we can avoid
 * the expansion permutation E.
 * 
 * Those 48 bit values are held in two 32 bit words.  To speed up the
 * S-box table lookups, the 48 bits are divided into eight 6 bit groups
 * which directly feed the eight S-boxes.  The eight groups are stored in
 * eight successive bytes in memory.  Within each byte, the 6 bits are
 * the least significant ones.  The high two bits of each byte are zero.
 * 
 * Bit 1 of the 48 bit value according to the NBS spec is stored as the
 * LSB of the first byte in the eight byte representation.  If the eight
 * bytes are fetched as two long words (32 bits on the vax), bit 1 of the
 * NBS spec will be held in the LSB of the first long fetched.
 * 
 * Another representation of 48 bit values used by the code is to split
 * the value into two 32 bit registers and store the 24 bit halves of the
 * original value as the 24 least significant bits of each register.
 * 
 * The final representation is to store one bit per byte in an array of
 * bytes.  This rep has the property that bit N of the NBS spec is stored
 * as the LSB of the Nth byte in the array (i.e., index N-1).
 */


#include	"fdes.h"

/*
 * Final permutation, FP = IP^(-1)  - changed index from 1 to 0
 */
unsb	FP[] = {
	39, 7,47,15,55,23,63,31,
	38, 6,46,14,54,22,62,30,
	37, 5,45,13,53,21,61,29,
	36, 4,44,12,52,20,60,28,
	35, 3,43,11,51,19,59,27,
	34, 2,42,10,50,18,58,26,
	33, 1,41, 9,49,17,57,25,
	32, 0,40, 8,48,16,56,24,
};


/*
 * Permuted-choice 1 from the key bits
 * to yield C and D.
 * Note that bits 8,16... are left out:
 * They are intended for a parity check.
 *
 * Not zero based!
 */
static	unsb	PC1_C[] = {
	57,49,41,33,25,17, 9,
	 1,58,50,42,34,26,18,
	10, 2,59,51,43,35,27,
	19,11, 3,60,52,44,36,
};

static	unsb	PC1_D[] = {
	63,55,47,39,31,23,15,
	 7,62,54,46,38,30,22,
	14, 6,61,53,45,37,29,
	21,13, 5,28,20,12, 4,
};


/*
 * Permuted-choice 2, to pick out the bits from
 * the CD array that generate the key schedule.
 */
static	unsb	PC2_C[] = {
	14,17,11,24, 1, 5,
	 3,28,15, 6,21,10,
	23,19,12, 4,26, 8,
	16, 7,27,20,13, 2,
};

static	unsb	PC2_D[] = {
	41,52,31,37,47,55,
	30,40,51,45,33,48,
	44,49,39,56,34,53,
	46,42,50,36,29,32,
};



/*
 * The E bit-selection table.
 */
unsb	E[] = {
	32, 1, 2, 3, 4, 5,
	 4, 5, 6, 7, 8, 9,
	 8, 9,10,11,12,13,
	12,13,14,15,16,17,
	16,17,18,19,20,21,
	20,21,22,23,24,25,
	24,25,26,27,28,29,
	28,29,30,31,32, 1,
};


/*
 * P is a permutation on the selected combination
 * of the current L and key.
 */
static	unsb	P[] = {
	16, 7,20,21,
	29,12,28,17,
	 1,15,23,26,
	 5,18,31,10,
	 2, 8,24,14,
	32,27, 3, 9,
	19,13,30, 6,
	22,11, 4,25,
};


/*
 * The 8 selection functions.
 */
static	fbpb4R	S[8][64] = {
	14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
	 0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
	 4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
	15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13,

	15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
	 3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
	 0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
	13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9,

	10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
	13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
	13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
	 1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12,

	 7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
	13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
	10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
	 3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14,

	 2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
	14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
	 4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
	11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3,

	12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
	10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
	 9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
	 4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13,

	 4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
	13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
	 1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
	 6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12,

	13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
	 1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
	 7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
	 2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11,
};


/*
 * Sequence of shifts used for the key schedule.
 */
static	unsb	shifts[] = {
	1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 
};


/*
 * The C and D arrays used to calculate the key schedule.
 */
static	obpb1	C[28];
static	obpb1	D[28];


/*
 * Low and high 24 bits of keys are stored alternately.
 */
sbpb24	KS[32];


/*
 * The current block, divided into 2 halves.
 */
obpb1	L[32], R[32];


/*
 * Tables that combine the S and P operations.
 */
sbpb24 S0L[64], S1L[64], S2L[64], S3L[64],
       S4L[64], S5L[64], S6L[64], S7L[64];

sbpb24 S0H[64], S1H[64], S2H[64], S3H[64],
       S4H[64], S5H[64], S6H[64], S7H[64];





/* Convert unsl in twenty-four bit contiguous format
 * to six bits per byte format.  Return result.
 */
sbpb24 tfTOsixbit(tf)
	ebpb24	tf;
{
	sbpb24	res;

	res = 0;
	res |= (tf >> 0) & 077;
	res |= ((tf >> 6) & 077) << 8;
	res |= ((tf >> 12) & 077) << 16;
	res |= ((tf >> 18) & 077) << 24;
	return(res);
}


/* Convert unsl in six bits per byte format
 * to twenty-four bit contiguous format.  Return result.
 */
ebpb24 sixbitTOtf(sb)
	sbpb24	sb;
{
	ebpb24	res;

	res = 0;
	res |= (sb >> 0) & 077;
	res |= ((sb >> 8) & 077) << 6;
	res |= ((sb >> 16) & 077) << 12;
	res |= ((sb >> 24) & 077) << 18;
	return(res);
}


/*
 * Set up the key schedule from the key.
 */
fsetkey(key)
	obpb1	*key;
{
reg	int	i, j, k;
	int	t;

	/*
	 * First, generate C and D by permuting
	 * the key.  The low order bit of each
	 * 8-bit char is not used, so C and D are only 28
	 * bits apiece.
	 */
	for (i = 0 ; i < 28 ; i++)  {
		C[i] = key[PC1_C[i] - 1];
		D[i] = key[PC1_D[i] - 1];
	}
	/*
	 * To generate Ki, rotate C and D according
	 * to schedule and pick up a permutation
	 * using PC2.
	 */
	for (i = 0 ; i < 32 ; i += 2)  {
		/*
		 * rotate.
		 */
		for (k = 0 ; k < shifts[i/2] ; k++)  {
			t = C[0];
			for (j = 0 ; j < 28-1 ; j++)
				C[j] = C[j + 1];
			C[27] = t;
			t = D[0];
			for (j = 0 ; j < 28-1 ; j++)
				D[j] = D[j + 1];
			D[27] = t;
		}
		/*
		 * get Ki. Note C and D are concatenated.
		 */
		KS[i] = KS[i+1] = 0;
		for (j = 0 ; j < 24 ; j++)  {
			KS[i] |= (C[PC2_C[j] - 1] << j);
			KS[i+1] |= (D[PC2_D[j] - 28 - 1] << j);
			}
		KS[i] = tfTOsixbit(KS[i]);
		KS[i+1] = tfTOsixbit(KS[i+1]);
		}
}


/*
 * Lookup an S-box entry.
 */
fbpb4 lookupS(tableno, t6bits)
	unsl	tableno;
	sbpb6R	t6bits;
{
	sbpb6	fixed6bits;
	fbpb4R	r;
	fbpb4	fixedr;

	fixed6bits = (((t6bits >> 0) &01) << 5)+
	             (((t6bits >> 1) &01) << 3)+
                     (((t6bits >> 2) &01) << 2)+
                     (((t6bits >> 3) &01) << 1)+
                     (((t6bits >> 4) &01) << 0)+
                     (((t6bits >> 5) &01) << 4);

	r = S[tableno][fixed6bits];

	fixedr = (((r >> 3)&01) << 0)+
	         (((r >> 2)&01) << 1)+
		 (((r >> 1)&01) << 2)+
		 (((r >> 0)&01) << 3);

	return(fixedr);
}


init_des()
{
	init(0, S0L, S0H);
	init(1, S1L, S1H);
	init(2, S2L, S2H);
	init(3, S3L, S3H);
	init(4, S4L, S4H);
	init(5, S5L, S5H);
	init(6, S6L, S6H);
	init(7, S7L, S7H);
}


init(tableno, lowptr, highptr)
	unsl	tableno;
	sbpb24	*lowptr, *highptr;
{
	static	obpb1	tmp32[32];
	static	obpb1	tmpP32[32];
	static	obpb1	tmpE[32];
	int	j, k, i;

	for (j = 0 ; j < 64 ; j++)  {
		k = lookupS(tableno, j);
		for (i = 0 ; i < 32 ; i++)
			tmp32[i] = 0;
		for (i = 0 ; i < 4 ; i++)
			tmp32[4 * tableno + i] = (k >> i) & 01;
		for (i = 0 ; i < 32 ; i++)
			tmpP32[i] = tmp32[P[i] - 1];
		for (i = 0 ; i < 48 ; i++)
			tmpE[i] = tmpP32[E[i] - 1];
		lowptr[j] = 0;
		highptr[j] = 0;
		for (i = 0 ; i < 24 ; i++)
			lowptr[j] |= tmpE[i] << i;
		for (k = 0, i = 24 ; i < 48 ; i++, k++)
			highptr[j] |= tmpE[i] << k;

		lowptr[j] = tfTOsixbit(lowptr[j]);
		highptr[j] = tfTOsixbit(highptr[j]);
	}
}
