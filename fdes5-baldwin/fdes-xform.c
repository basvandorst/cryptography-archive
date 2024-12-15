/* Heart of the fast password transform.
 *
 * Speed up techniques:
 * - Use 48 bit datapath throughout f.
 * - Store 48 bit path in two longs, 24 bits each.
 * - Byte align the four 6-bit groups in the 24 bits per long.
 * - Combine the S and P tables into a single lookup.
 * - Implement the SP lookup by 16 long word lookups.
 * - Access the 16 SP tables via an indirection table to save registers.
 * - Use sed script to use all available registers.  See fdes.sed.doc.
 * - Perform salting by XORing with a computed mask.
 * - XOR table values directly into accumulators.
 * - Indirection table alternates left and right half to save on
 *   pointer increment instructions.
 * - Have loop indices count down to zero for faster code.
 * - Put key schedule in single array alternating low and high longs,
 *   This allows access via an autoinc register variable.
 * - Split computation of k to make register optimizing easier.
 * - Unroll f loop to avoid the need to swap left and right halves.
 */

#include	"fdes.h"


/* Key schedule.
 * Alternating longs with low and high bits of key.
 */
extern sbpb24 KS[/* 32 */];

extern sbpb24 S0L[], S1L[], S2L[], S3L[],
            S4L[], S5L[], S6L[], S7L[];

extern sbpb24 S0H[], S1H[], S2H[], S3H[],
            S4H[], S5H[], S6H[], S7H[];

/* It is slightly faster to indirect through this table than to specify
 * the desired table directly.
 */
static sbpb24 *stt[]  = {
	S3L, S3H,	
	S2L, S2H,
	S1L, S1H,
	S0L, S0H,
	S7L, S7H,
	S6L, S6H,
	S5L, S5H,
	S4L, S4H,
	};


/*
 * The payoff: encrypt a block.
 */
xform(quarters, saltvalue)
	sbpb24 *quarters;
	sbpb24 saltvalue;
{
/* Only get six register vars on Vax, so sed script is used to
 * move other variables into registers.
 * Some vars are declared to be in registers so the compiler will
 * generate easy to transform code.
 */
	sbpb24	Rl, Rh;
	sbpb24	Ll, Lh;
static	sbpb24	Dl,Dh;
reg	sbpb24	k;
	sbpb24	negsalt;
reg	sbpb6	*dp;
reg	int	mi;
reg 	sbpb24	*kp;
reg 	sbpb24	*kend;

	negsalt = ~saltvalue;	/* Vax has bit clear instr not AND. */
	Ll = Lh = Rl = Rh = 0;
 	kend = &KS[32];

    for (mi = 25 ; --mi >= 0 ; )  {
	for (kp = KS ; kp < kend ; )  {
		reg	sbpb24 **stp;

		k = Rl ^ Rh;
		k &= ~negsalt;
		Dl = k ^ Rl ^ *kp++;
		Dh = k ^ Rh ^ *kp++;

		stp = ((sbpb24 **) stt);

		dp = ((sbpb6 *) &Dl);
		/* Compiler bug prevents putting all of these on one line.*/
		Ll ^= (*stp++)[*dp];
		Lh ^= (*stp++)[*dp++];
		Ll ^= (*stp++)[*dp];
		Lh ^= (*stp++)[*dp++];
		Ll ^= (*stp++)[*dp];
		Lh ^= (*stp++)[*dp++];
		Ll ^= (*stp++)[*dp];
		Lh ^= (*stp++)[*dp++];
		Ll ^= (*stp++)[*dp];
		Lh ^= (*stp++)[*dp++];
		Ll ^= (*stp++)[*dp];
		Lh ^= (*stp++)[*dp++];
		Ll ^= (*stp++)[*dp];
		Lh ^= (*stp++)[*dp++];
		Ll ^= (*stp++)[*dp];
		Lh ^= (*stp++)[*dp++];

		k = Ll ^ Lh;
		k &= ~negsalt;
		Dl = k ^ Ll ^ *kp++;
		Dh = k ^ Lh ^ *kp++;

		stp = ((sbpb24 **) stt);

		dp = ((sbpb6 *) &Dl);
		/* Compiler bug prevents putting all of these on one line.*/
		Rl ^= (*stp++)[*dp];
		Rh ^= (*stp++)[*dp++];
		Rl ^= (*stp++)[*dp];
		Rh ^= (*stp++)[*dp++];
		Rl ^= (*stp++)[*dp];
		Rh ^= (*stp++)[*dp++];
		Rl ^= (*stp++)[*dp];
		Rh ^= (*stp++)[*dp++];
		Rl ^= (*stp++)[*dp];
		Rh ^= (*stp++)[*dp++];
		Rl ^= (*stp++)[*dp];
		Rh ^= (*stp++)[*dp++];
		Rl ^= (*stp++)[*dp];
		Rh ^= (*stp++)[*dp++];
		Rl ^= (*stp++)[*dp];
		Rh ^= (*stp++)[*dp++];
		}

	Ll ^= Rl;  Lh ^= Rh;
	Rl ^= Ll;  Rh ^= Lh;
	Ll ^= Rl;  Lh ^= Rh;
	}

    {
    reg   sbpb24   *qp;

    qp = quarters;
    *qp++ = Ll;
    *qp++ = Lh;
    *qp++ = Rl;
    *qp++ = Rh;
    }
}
