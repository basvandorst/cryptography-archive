#ifndef LBNPPC_H
#define LBNPPC_H

/*
 * Assembly-language routines for the Power PC processor.
 * Annoyingly, the Power PC does not have 64/32->32 bit divide,
 * so the C code should be reasonably fast.  But it does have
 * 32x32->64-bit multiplies, and these routines provide access
 * to that.
 */

/*
 * Bignums are stored in arrays of 32-bit words, and the least
 * significant 32-bit word has the lowest address, thus "little-endian".
 * The C code is slightly more efficient this way, so unless the
 * processor cares (the PowerPC, like most RISCs, doesn't), it is
 * best to use BN_LITTLE_ENDIAN.
 * Note that this has NOTHING to do with the order of bytes within a 32-bit
 * word; the math library is insensitive to that.
 */
#define BN_LITTLE_ENDIAN 1

/* Shared transition vector array */
extern unsigned const * const lbnPPC_tv[];

/* A function pointer on the PowerPC is a pointer to a transition vector */
#define lbnMulN1_32 \
((void (*)(unsigned *, unsigned const *, unsigned, unsigned))(lbnPPC_tv+0))
#define lbnMulAdd1_32 \
((unsigned (*)(unsigned *, unsigned const *, unsigned, unsigned))(lbnPPC_tv+1))
#define lbnMulSub1_32 \
((unsigned (*)(unsigned *, unsigned const *, unsigned, unsigned))(lbnPPC_tv+2))

#endif
