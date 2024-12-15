/*
 * lbn80386.h - This file defines the interfaces to the 80386
 * assembly primitives.  It is intended to be included in "lbn.h"
 * via the "#include BNINCLUDE" mechanism.
 */
 
#define BN_LITTLE_ENDIAN 1

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl
lbnMulN1_32(unsigned long *out, unsigned long const *in,
            unsigned len, unsigned long k);
#define lbnMulN1_32 lbnMulN1_32
            
unsigned long __cdecl
lbnMulAdd1_32(unsigned long *out, unsigned long const *in,
              unsigned len, unsigned long k);
#define lbnMulAdd1_32 lbnMulAdd1_32
       
unsigned long __cdecl
lbnMulSub1_32(unsigned long *out, unsigned long const *in,
              unsigned len, unsigned long k);
#define lbnMulSub1_32 lbnMulSub1_32

unsigned long __cdecl
lbnDiv21_32(unsigned long *q, unsigned long nh, unsigned long nl,
            unsigned long d);
#define lbnDiv21_32 lbnDiv21_32

unsigned __cdecl
lbnModQ_32(unsigned long const *n, unsigned len, unsigned long d);
#define lbnModQ_32 lbnModQ_32

#ifdef __cplusplus
}
#endif
