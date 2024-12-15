/* low level arithmetic for PARI */
/* processor: Intel i386 in native mode */
/* assembler syntax: GNU or SUN, moves go from left to right */
/* compiler: GNU gcc or SUN cc */
/* parameter passing convention: on the stack 4(%esp),8(%esp),... */
/* registers: %eax,%edx,%ecx may be modified,
              everything else must be saved and restored */
/* result: passed in %eax */
/* word length: 32 bits */

/* written by Bruno Haible 14.11.1992 */

#if defined(__EMX__) || defined(__DJGCC__) || defined(linux)
/* GNU assembler */
#ifdef __STDC__
/* ANSI C concatenation */
#define C(entrypoint) _##entrypoint
#else
/* traditional C concatenation */
#define C(entrypoint) _/**/entrypoint
#endif
#define repz     repe
#define shcl     %cl,
#else
/* SUN assembler or Consensys assembler */
/* no concatenation needed */
#define C(entrypoint) entrypoint
#define jecxz    orl %ecx,%ecx ; jz
#define shcl
#endif

        .globl C(addll)
        .globl C(subll)
        .globl C(addllx)
        .globl C(subllx)
        .globl C(shiftl)
        .globl C(shiftlr)
        .globl C(bfffo)
        .globl C(mulll)
        .globl C(addmul)
        .globl C(divll)
        .globl C(mulmodll)
        .globl C(overflow)
        .globl C(hiremainder)

.text

	.align 2
C(addll:)
        xorl    %edx,%edx          /* clear %edx    */
        movl    4(%esp),%eax       /* get x         */
        addl    8(%esp),%eax       /* add y         */
        adcl    %edx,%edx          /* %edx := carry */
        movl    %edx,C(overflow)   /* set overflow  */
	ret                        /* return %eax   */
	.align 2,0x90

	.align 2
C(addllx:)
        xorl    %edx,%edx          /* clear %edx      */
        xorl    %eax,%eax          /* clear %eax      */
        subl    C(overflow),%eax   /* set carry       */
        movl    4(%esp),%eax       /* get x           */
        adcl    8(%esp),%eax       /* add y and carry */
        adcl    %edx,%edx          /* %edx := carry   */
        movl    %edx,C(overflow)   /* set overflow    */
	ret                        /* return %eax     */
	.align 2,0x90

	.align 2
C(subll:)
        xorl    %edx,%edx          /* clear %edx    */
        movl    4(%esp),%eax       /* get x         */
        subl    8(%esp),%eax       /* subtract y    */
        adcl    %edx,%edx          /* %edx := carry */
        movl    %edx,C(overflow)   /* set overflow  */
	ret                        /* return %eax   */
	.align 2,0x90

	.align 2
C(subllx:)
        xorl    %edx,%edx          /* clear %edx           */
        xorl    %eax,%eax          /* clear %eax           */
        subl    C(overflow),%eax   /* set carry            */
        movl    4(%esp),%eax       /* get x                */
        sbbl    8(%esp),%eax       /* subtract y and carry */
        adcl    %edx,%edx          /* %edx := carry        */
        movl    %edx,C(overflow)   /* set overflow         */
	ret                        /* return %eax          */
	.align 2,0x90

	.align 2
C(shiftl:)
        movl    4(%esp),%eax        /* get x                          */
        movb    8(%esp),%cl         /* get shift count i              */
        xorl    %edx,%edx           /* clear %edx                     */
        shldl   shcl %eax,%edx      /* shift %edx left by i bits,
                                       feeding in %eax from the right */
        shll    %cl,%eax            /* shift %eax left by i bits      */
        movl    %edx,C(hiremainder) /* set hiremainder                */
	ret                         /* return %eax                    */
	.align 2,0x90

	.align 2
C(shiftlr:)
        movl    4(%esp),%eax        /* get x                         */
        movb    8(%esp),%cl         /* get shift count i             */
        xorl    %edx,%edx           /* clear %edx                    */
        shrdl   shcl %eax,%edx      /* shift %edx right by i bits,
                                       feeding in %eax from the left */
        shrl    %cl,%eax            /* shift %eax right by i bits    */
        movl    %edx,C(hiremainder) /* set hiremainder               */
	ret                         /* return %eax                   */
	.align 2,0x90

#if 0 /* Only in case bfffo() is called with argument 0 */
	.align 2
C(bfffo:)
        movl    4(%esp),%eax        /* get x                         */
        testl   %eax,%eax           /* check if zero                 */
        jz      bfffo1
        bsrl    %eax,%edx           /* %edx := number of leading bit */
        movl    $31,%eax
        subl    %edx,%eax           /* result is 31 - %edx           */
        ret                         /* return %eax                   */
	.align 2,0x90
bfffo1: movl    $32,%eax            /* result is 32                  */
        ret                         /* return %eax                   */
	.align 2,0x90
#else
	.align 2
C(bfffo:)
        movl    4(%esp),%eax        /* get x                         */
        bsrl    %eax,%edx           /* %edx := number of leading bit */
        movl    $31,%eax
        subl    %edx,%eax           /* result is 31 - %edx           */
        ret                         /* return %eax                   */
	.align 2,0x90
#endif

	.align 2
C(mulll:)
        movl    4(%esp),%eax        /* get x                */
        mull    8(%esp)             /* %edx|%eax := x * y   */
        movl    %edx,C(hiremainder) /* store high word      */
        ret                         /* return low word %eax */
	.align 2,0x90

	.align 2
C(addmul:)
        xorl    %ecx,%ecx           /* clear %ecx           */
        movl    4(%esp),%eax        /* get x                */
        mull    8(%esp)             /* %edx|%eax := x * y   */
        addl    C(hiremainder),%eax /* add 0|hiremainder    */
        adcl    %ecx,%edx
        movl    %edx,C(hiremainder) /* store high word      */
        ret                         /* return low word %eax */
	.align 2,0x90

        .align 2
C(divll:)
        movl    4(%esp),%eax        /* get low word x        */
        movl    C(hiremainder),%edx /* get high word         */
        divl    8(%esp)             /* divide %edx|%eax by y */
        movl    %edx,C(hiremainder) /* store remainder       */
        ret                         /* return quotient %eax  */
	.align 2,0x90

	.align 2
C(mulmodll:)
        movl    4(%esp),%eax        /* get x                 */
        mull    8(%esp)             /* %edx|%eax := x * y    */
        divl    12(%esp)            /* divide %edx|%eax by z */
        movl    %edx,%eax           /* return remainder %edx */
        ret
	.align 2,0x90

.comm C(overflow),4
.comm C(hiremainder),4
