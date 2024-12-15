#define __ll_B 0x10000
#define __ll_lowpart(t) ((unsigned long int) (t) & 0xFFFF)
#define __ll_highpart(t) ((unsigned long int) (t) >> 16)

/* Define 32 bit multiplication asm macros (useful only with gnu-cc).

   umul_ppmm(high_prod, low_prod, multiplier, multiplicand)
   multiplies two unsigned long integers multiplier and multiplicand,
   and generates a two unsigned word product in high_prod and
   low_prod.

*/

/* The CPUs come in alphabetical order below.

   Please add support for more CPUs here, or improve the current support
   for the CPUs below!
   (E.g. WE32100, HP-PA (xmpyu?), i960, IBM360.)  */

#if defined (__GNUC__) && !defined (NO_ASM)

#if defined (__a29k__) || defined (___AM29K__)
#define umul_ppmm(xh, xl, m0, m1) \
  do {									\
    unsigned long int __m0 = (m0), __m1 = (m1);				\
    __asm__ ("multiplu %0,%1,%2" : "=r" ((unsigned long int)(xl))	\
	     : "r" (__m0), "r" (__m1));					\
    __asm__ ("multmu %0,%1,%2" : "=r" ((unsigned long int)(xh))		\
	     : "r" (__m0), "r" (__m1));					\
  } while (0)
#endif /* __a29k__ */

#if defined (__gmicro__)
#define umul_ppmm(ph, pl, m0, m1) \
  __asm__ ("mulx %3,%0,%1"						\
	: "=g" ((unsigned long int)(ph)), "=r" ((unsigned long int)(pl))\
	: "%0" ((unsigned long int)(m0)), "g" ((unsigned long int)(m1)))
#endif

#if defined (__i386__) || defined (__i486__)
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("mull %3"							\
	: "=a" ((unsigned long int)(w0)), "=d" ((unsigned long int)(w1))\
	: "%0" ((unsigned long int)(u)), "rm" ((unsigned long int)(v)))
#endif /* __i386__ */

#if defined (___IBMR2__) /* IBM RS6000 */
#define umul_ppmm(xh, xl, m0, m1) \
  do {									\
    unsigned long int __m0 = (m0), __m1 = (m1);				\
    __asm__ ("mul %0,%2,%3"						\
	: "=r" ((unsigned long int)(xh)), "=q" ((unsigned long int)(xl))\
	: "r" (__m0), "r" (__m1));					\
    (xh) += ((((signed long int) __m0 >> 31) & __m1)			\
	     + (((signed long int) __m1 >> 31) & __m0));		\
  } while (0)
#endif /* ___IBMR2__ */

#if defined (__mc68000__)
#if defined (__mc68020__) || defined (__NeXT__) || defined(mc68020)
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("mulu%.l %3,%1:%0"						\
	: "=d" ((unsigned long int)(w0)), "=d" ((unsigned long int)(w1))\
	: "%0" ((unsigned long int)(u)), "dmi" ((unsigned long int)(v)))
#else /* not mc68020 */
#define umul_ppmm(xh, xl, a, b) \
  __asm__ ("| Inlined umul_ppmm
	movel	%2,d0
	movel	%3,d1
	movel	d0,d2
	swap	d0
	movel	d1,d3
	swap	d1
	movew	d2,d4
	mulu	d3,d4
	mulu	d1,d2
	mulu	d0,d3
	mulu	d0,d1
	movel	d4,d0
	eorw	d0,d0
	swap	d0
	addl	d0,d2
	addl	d3,d2
	jcc	1f
	addl	#65536,d1
1:	swap	d2
	moveq	#0,d0
	movew	d2,d0
	movew	d4,d2
	movel	d2,%1
	addl	d1,d0
	movel	d0,%0"							\
       : "=g" ((unsigned long int)(xh)), "=g" ((unsigned long int)(xl))	\
       :"g" ((unsigned long int)(a)), "g" ((unsigned long int)(b))	\
       : "d0", "d1", "d2", "d3", "d4")
#endif /* not mc68020 */
#endif /* mc68000 */

#if defined (__mips__)
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("multu %2,%3
	mflo %0
	mfhi %1"							\
	: "=r" ((unsigned long int)(w0)), "=r" ((unsigned long int)(w1))\
	: "r" ((unsigned long int)(u)), "r" ((unsigned long int)(v)))
#define UMUL_TIME 5
#define UDIV_TIME 100
#endif /* __mips__ */

#if defined (__ns32000__)
#define __umulsidi3(u, v) \
  ({long long int __w;							\
      __asm__ ("meid %2,%0" : "=g" (__w)				\
	: "%0" ((unsigned long int)(u)), "g" ((unsigned long int)(v)));	\
      __w; })
#endif /* __ns32000__ */

#if defined (__pyr__)
/* This insn doesn't work on ancient pyramids.  */
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("movw %2,tr11
	uemul %3,tr10
	movw tr10,%0
	movw tr11,%1"							\
	: "=r" ((unsigned long int)(w1)), "=r" ((unsigned long int)(w0))\
	: "r" ((unsigned long int)(u)), "r" ((unsigned long int)(v))	\
	: "tr10", "tr11")
#endif /* __pyr__ */

#if defined (__ibm032__) /* RT/ROMP */
#define umul_ppmm(ph, pl, m0, m1) \
  do {									\
    unsigned long int __m0 = (m0), __m1 = (m1);				\
    __asm__ (								\
       "s	r2,r2
	mts	r10,%2
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	m	r2,%3
	cas	%0,r2,r0
	mfs	r10,%1"							\
       : "=r" ((unsigned long int)(ph)), "=r" ((unsigned long int)(pl))	\
       : "%r" (__m0), "r" (__m1)					\
       : "r2");								\
    (ph) += ((((signed long int) __m0 >> 31) & __m1)			\
	     + (((signed long int) __m1 >> 31) & __m0));		\
  } while (0)
#endif

#if defined (__sparc__)
#if defined (__sparc8__)	/* How do we recog. version 8 SPARC?  */
#define umul_ppmm(w1, w0, u, v) \
  __asm__ ("! Inlined umul_ppmm

        umul %2,%3,%1
        rd %%y,%0"					\
	: "=r" ((unsigned long int)(w1)), "=r" ((unsigned long int)(w0))\
	: "r" ((unsigned long int)(u)), "r" ((unsigned long int)(v)))
#else
/* SPARC without integer multiplication and divide instructions.
   (i.e. at least Sun4/20,40,60,65,75,110,260,280,330,360,380,470,490) */
#define umul_ppmm(w1, w0, u, v) \
  __asm__("! Inlined umul_ppmm
	wr	%%g0,%2,%%y	! SPARC has 0-3 delay insn after a wr
	sra	%3,31,%%g2	! Don't move this insn
	and	%2,%%g2,%%g2	! Don't move this insn
	andcc	%%g0,0,%%g1	! Don't move this insn
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,%3,%%g1
	mulscc	%%g1,0,%%g1
	add	%%g1,%%g2,%0
	rd	%%y,%1" \
	: "=r" ((unsigned long int)(w1)), "=r" ((unsigned long int)(w0))\
	: "%rI" ((unsigned long int)(u)), "r" ((unsigned long int)(v))	\
       : "%g1", "%g2")

#define UMUL_TIME 39		/* 39 instructions */
/* It's quite necessary to add this much assembler for the sparc.
   The default udiv_qrnnd (in C) is more than 10 times slower!  */
#define udiv_qrnnd(q, r, n1, n0, d) \
  __asm__ ("! Inlined udiv_qrnnd
	mov	32,%%g1
	subcc	%1,%2,%%g0
1:	bcs	5f
	 addxcc %0,%0,%0	! shift n1n0 and a q-bit in lsb
	sub	%1,%2,%1	! this kills msb of n
	addx	%1,%1,%1	! so this can't give carry
	subcc	%%g1,1,%%g1
2:	bne	1b
	 subcc	%1,%2,%%g0
	bcs	3f
	 addxcc %0,%0,%0	! shift n1n0 and a q-bit in lsb
	b	3f
	 sub	%1,%2,%1	! this kills msb of n
4:	sub	%1,%2,%1
5:	addxcc	%1,%1,%1
	bcc	2b
	 subcc	%%g1,1,%%g1
! Got carry from n.  Subtract next step to cancel this carry.
	bne	4b
	 addcc	%0,%0,%0	! shift n1n0 and a 0-bit in lsb
	sub	%1,%2,%1
3:	xnor	%0,0,%0
	! End of inline udiv_qrnnd"					\
	: "=r&" ((unsigned long int)(q)), "=r&" ((unsigned long int)(r))\
	: "r" ((unsigned long int)(d)), "1" ((unsigned long int)(n1)),	\
	  "0" ((unsigned long int)(n0)) : "%g1")
#define UDIV_TIME (3+7*32)	/* 7 instructions/iteration. 32 iterations. */
#endif
#endif /* __sparc8__ */

#if defined (__vax__)
#define umul_ppmm(xh, xl, m0, m1) \
  do {									\
    union {long long int ll;struct {unsigned long int l, h;} i;} __xx;	\
    unsigned long int __m0 = (m0), __m1 = (m1);				\
    __asm__ ("emul %1,%2,$0,%0"						\
	 : "=r" (__xx.ll) : "g" (__m0), "g" (__m1));			\
    (xh) = __xx.i.h; (xl) = __xx.i.l;					\
    (xh) += ((((signed long int) __m0 >> 31) & __m1)			\
	     + (((signed long int) __m1 >> 31) & __m0));		\
  } while (0)
#endif /* __vax__ */

#endif /* __GNUC__ */

/* If this machine has no inline assembler, use C macros.  */

#if !defined (umul_ppmm)
#if !defined (NO_KARAT)
/* Lacy implementation of Karatsuba 32x32 bit mult 5/26/93 */
#define umul_ppmm(ph, pl, a, b)						\
    do {								\
        unsigned long __pmidh, __pmidl;					\
        unsigned long __ahi, __bhi, __alo, __blo;			\
	long __carry;							\
									\
	__ahi = ((a) >> 16);						\
	__alo = (a) & 0xFFFF;						\
	__bhi = ((b) >> 16);						\
	__blo = (b) & 0xFFFF;						\
									\
	(ph) = __ahi * __bhi;						\
	(pl) = __alo * __blo;						\
	__ahi -= __alo;							\
	__blo -= __bhi;							\
	__pmidh = __ahi * __blo;					\
	__carry = (__pmidh)?-((__ahi ^ __blo) & 0x10000):0;		\
									\
	__pmidh += (ph);						\
	__carry += (__pmidh < (ph))?0x10000:0;				\
	__pmidh += (pl);						\
	__carry += (__pmidh < (pl))?0x10000:0;				\
	__pmidl = __pmidh << 16;					\
	__pmidh >>= 16;							\
									\
	(pl) += __pmidl;						\
	__carry += ((pl) < __pmidl);					\
									\
	(ph) += __carry + __pmidh;					\
    } while (0)

#else
#define umul_ppmm(w1, w0, u, v)					\
  do {									\
    unsigned long int __x0, __x1, __x2, __x3;				\
    unsigned int __ul, __vl, __uh, __vh;				\
									\
    __ul = __ll_lowpart (u);						\
    __uh = __ll_highpart (u);						\
    __vl = __ll_lowpart (v);						\
    __vh = __ll_highpart (v);						\
									\
    __x0 = (unsigned long int) __ul * __vl;				\
    __x1 = (unsigned long int) __ul * __vh;				\
    __x2 = (unsigned long int) __uh * __vl;				\
    __x3 = (unsigned long int) __uh * __vh;				\
									\
    __x1 += __ll_highpart (__x0);/* this can't give carry */		\
    __x1 += __x2;		/* but this indeed can */		\
    if (__x1 < __x2)		/* did we get it? */			\
      __x3 += __ll_B;		/* yes, add it in the proper pos. */	\
									\
    (w1) = __x3 + __ll_highpart (__x1);					\
    (w0) = __ll_lowpart (__x1) * __ll_B + __ll_lowpart (__x0);		\
  } while (0)
#endif
#endif

#if !defined (__umulsidi3)
#define __umulsidi3(u, v) \
  ({long_long __w;							\
    umul_ppmm (__w.s.high, __w.s.low, u, v);				\
    __w.ll; })
#endif

