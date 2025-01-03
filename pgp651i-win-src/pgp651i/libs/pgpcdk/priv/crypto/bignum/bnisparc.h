/*
 * bnisparc.h - 32-bit bignum primitives for the 68020 (or 683xx) processors.
 *
 * These primitives use little-endian word order.
 * (The order of bytes within words is irrelevant.)
 *
 * $Id: bnisparc.h,v 1.1 1997/08/12 08:30:38 colin Exp $
 */
#define BN_LITTLE_ENDIAN 1

typedef unsigned long bnword32;
#define BNWORD32 bnword32

#ifdef __cplusplus
extern "C" {
#endif

void bniMulN1_32(bnword32 *out, bnword32 const *in, unsigned len, bnword32 k);
bnword32
bniMulAdd1_32(bnword32 *out, bnword32 const *in, unsigned len, bnword32 k);
bnword32
bniMulSub1_32(bnword32 *out, bnword32 const *in, unsigned len, bnword32 k);
/*
bnword32 bniDiv21_32(bnword32 *q, bnword32 nh, bnword32 nl, bnword32 d);
unsigned bniModQ_32(bnword32 const *n, unsigned len, unsigned d);
*/

#ifdef __cplusplus
}
#endif

/* #define the values to exclude the C versions */
#define bniMulN1_32 bniMulN1_32
#define bniMulAdd1_32 bniMulAdd1_32
#define bniMulSub1_32 bniMulSub1_32
/*
#define bniDiv21_32 bniDiv21_32
#define bniModQ_32 bniModQ_32
*/
