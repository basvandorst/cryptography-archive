/*
 * lbn8086.h - This file defines the interfaces to the 8086
 * assembly primitives.  It is intended to be included in "lbn.h"
 * via the "#include BNINCLUDE" mechanism.
 */
 
#define BN_LITTLE_ENDIAN 1

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl __far
lbnMulN1_16(unsigned __far *out, unsigned const __far *in,
            unsigned len, unsigned k);
#define lbnMulN1_16 lbnMulN1_16
            
unsigned __cdecl __far
lbnMulAdd1_16(unsigned __far *out, unsigned const __far *in,
              unsigned len, unsigned k);
#define lbnMulAdd1_16 lbnMulAdd1_16
       
unsigned __cdecl __far
lbnMulSub1_16(unsigned __far *out, unsigned const __far *in,
              unsigned len, unsigned k);
#define lbnMulSub1_16 lbnMulSub1_16

unsigned __cdecl __far
lbnDiv21_16(unsigned __far *q, unsigned nh, unsigned nl, unsigned d);
#define lbnDiv21_16 lbnDiv21_16

unsigned __cdecl __far
lbnModQ_16(unsigned const __far *n, unsigned len, unsigned d);
#define lbnModQ_16 lbnModQ_16



void __cdecl __far
lbnMulN1_32(unsigned long __far *out, unsigned long const __far *in,
            unsigned len, unsigned long k);
#define lbnMulN1_32 lbnMulN1_32
            
unsigned long __cdecl __far
lbnMulAdd1_32(unsigned long __far *out, unsigned long const __far *in,
              unsigned len, unsigned long k);
#define lbnMulAdd1_32 lbnMulAdd1_32
       
unsigned long __cdecl __far
lbnMulSub1_32(unsigned long __far *out, unsigned long const __far *in,
              unsigned len, unsigned long k);
#define lbnMulSub1_32 lbnMulSub1_32

unsigned long __cdecl __far
lbnDiv21_32(unsigned long __far *q, unsigned long nh, unsigned long nl,
            unsigned long d);
#define lbnDiv21_32 lbnDiv21_32

unsigned __cdecl __far
lbnModQ_32(unsigned long const __far *n, unsigned len, unsigned long d);
#define lbnModQ_32 lbnModQ_32

int __cdecl __far not386(void);

#ifdef __cplusplus
}
#endif
