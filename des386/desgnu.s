/* 386 assembler version in GNU 'as' format */

#define	F(l,r,key)\
	movl r,%eax;\
	rorl $4,%eax;\
	xorl key(%esi),%eax;\
	andl $0xfcfcfcfc,%eax;\
\
	movb %al,%bl;\
	xorl _Spboxa+6*256(%ebx),l;\
	movb %ah,%bl;\
	rorl $16,%eax;\
	xorl _Spboxa+4*256(%ebx),l;\
	movb %al,%bl;\
	xorl _Spboxa+2*256(%ebx),l;\
	movb %ah,%bl;\
	xorl _Spboxa(%ebx),l;\
\
	movl 4+key(%esi),%eax;\
	xorl r,%eax;\
	andl $0xfcfcfcfc,%eax;\
\
	movb %al,%bl;\
	xorl _Spboxa+7*256(%ebx),l;\
	movb %ah,%bl;\
	rorl $16,%eax;\
	xorl _Spboxa+5*256(%ebx),l;\
	movb %al,%bl;\
	xorl _Spboxa+3*256(%ebx),l;\
	movb %ah,%bl;\
	xorl _Spboxa+256(%ebx),l


/* Tell deskey() that it's linked with the assembler version */

.globl _Asmversion
.data
	.align 4
_Asmversion:
	.long 1

.text
	.align 2
.globl _des
_des:
	pushl %ebp
	movl %esp,%ebp
	pushl %esi
	pushl %ebx
	/* 8(%ebp) is key schedule pointer, 12(%ebp) is buffer pointer */

	/* Fetch 8 bytes from user's buffer in block and place in ecx and edx,
	 * in big-endian order. Uses esi.
	 * There's a very nice BSWAP instruction that executes in only
	 * 1 cycle, but it is only available on the 486. :-(
	 */
	movl 12(%ebp),%esi	/* esi = block */
	movl (%esi),%ecx	/* ecx = ((long *)block)[0] */
	xchgb %cl,%ch		/* bswap ecx */
	roll $16,%ecx
	xchgb %cl,%ch
	movl 4(%esi),%edx	/* edx = ((long *)block)[1] */
	xchgb %dl,%dh		/* bswap edx */
	roll $16,%edx
	xchgb %dl,%dh

	/* Hoey's clever initial permutation algorithm, translated to assembler
	 * (see Schneier p 478)	
	 *
	 * The convention here is *different* from the C version. The permuted
	 * values of left and right are rotated left by two additional
	 * bits so we can avoid the two shifts that would otherwise be
	 * required in each round to convert a S-box input to a memory offset
	 * for Spbox[].
	 */
	/* work = ((left >> 4) ^ right) & 0x0f0f0f0f */
	movl %ecx,%eax
	shrl $4,%eax
	xorl %edx,%eax
	andl $0x0f0f0f0f,%eax

	xorl %eax,%edx		/* right ^= work */

	/* left ^= work << 4 */
	shll $4,%eax
	xorl %eax,%ecx

	/* work = ((left >> 16) ^ right) & 0xffff */
	movl %ecx,%eax
	shrl $16,%eax
	xorl %edx,%eax
	andl $0xffff,%eax

	xorl %eax,%edx		/* right ^= work */

	/* left ^= work << 16 */
	shll $16,%eax
	xorl %eax,%ecx

	/* work = ((right >> 2) ^ left) & 0x33333333 */
	movl %edx,%eax
	shrl $2,%eax
	xorl %ecx,%eax
	andl $0x33333333,%eax

	/* left ^= work */
	xorl %eax,%ecx
	shll $2,%eax

	xorl %eax,%edx		/* right ^= (work << 2) */

	/* work = ((right >> 8) ^ left) & 0xff00ff */
	movl %edx,%eax
	shrl $8,%eax
	xorl %ecx,%eax
	andl $0x00ff00ff,%eax

	xorl %eax,%ecx		/* left ^= work */

	/* right ^= (work << 8) */
	shll $8,%eax
	xorl %eax,%edx

	roll $1,%edx		/* right <<<= 1 */

	/* work = (left ^ right) & 0xaaaaaaaa */
	movl %ecx,%eax
	xorl %edx,%eax
	andl $0xaaaaaaaa,%eax

	xorl %eax,%ecx		/* left ^= work */
	xorl %eax,%edx		/* right ^= work */

	roll $3,%ecx		/* left <<<= 3 */
	roll $2,%edx		/* right <<<= 2 */

	/* Set up for the rounds */
	movl 8(%ebp),%esi	/* esi = key schedule */
	movl $0,%ebx		/* Upper 3 bytes must be zero */

	/* Do the rounds */
	F(%ecx,%edx,0)
	F(%edx,%ecx,8)
	F(%ecx,%edx,16)
	F(%edx,%ecx,24)
	F(%ecx,%edx,32)
	F(%edx,%ecx,40)
	F(%ecx,%edx,48)
	F(%edx,%ecx,56)
	F(%ecx,%edx,64)
	F(%edx,%ecx,72)
	F(%ecx,%edx,80)
	F(%edx,%ecx,88)
	F(%ecx,%edx,96)
	F(%edx,%ecx,104)
	F(%ecx,%edx,112)
	F(%edx,%ecx,120)

	/* Inverse permutation */
	rorl $2,%ecx	/* left >>>= 2 */
	rorl $3,%edx	/* right >>>= 3 */

	/* work = (left ^ right) & 0xaaaaaaaa */
	movl %ecx,%eax
	xorl %edx,%eax
	andl $0xaaaaaaaa,%eax

	xorl %eax,%ecx	/* left ^= work */
	xorl %eax,%edx	/* right ^= work */
	rorl $1,%ecx	/* left >>>= 1 */

	/* work = (left >> 8) ^ right) & 0xff00ff */
	movl %ecx,%eax
	shrl $8,%eax
	xorl %edx,%eax
	andl $0x00ff00ff,%eax

	xorl %eax,%edx	/* right ^= work */

	/* left ^= work << 8 */
	shll $8,%eax
	xorl %eax,%ecx

	/* work = ((left >> 2) ^ right) & 0x33333333 */
	movl %ecx,%eax
	shrl $2,%eax
	xorl %edx,%eax
	andl $0x33333333,%eax

	xorl %eax,%edx	/* right ^= work */

	/* left ^= work << 2 */
	shll $2,%eax
	xorl %eax,%ecx

	/* work = ((right >> 16) ^ left) & 0xffff */
	movl %edx,%eax
	shrl $16,%eax
	xorl %ecx,%eax
	andl $0xffff,%eax

	xorl %eax,%ecx	/* left ^= work */

	/* right ^= work << 16 */
	shll $16,%eax
	xorl %eax,%edx

	/* work = ((right >> 4) ^ left) & 0x0f0f0f0f */
	movl %edx,%eax
	shrl $4,%eax
	xorl %ecx,%eax
	andl $0x0f0f0f0f,%eax

	xorl %eax,%ecx	/* left ^= work */

	/* right ^= work << 4 */
	shll $4,%eax
	xorl %eax,%edx

	/* Write ecx and edx into user's buffer block in big-endian order
	 * after final swap
	 */
	movl 12(%ebp),%esi
	xchgb %dl,%dh		/* bswap edx */
	roll $16,%edx
	xchgb %dl,%dh
	movl %edx,(%esi)
	xchgb %cl,%ch		/* bswap ecx */
	roll $16,%ecx
	xchgb %cl,%ch
	mov %ecx,4(%esi)

	popl %ebx
	popl %esi
	leave
	ret
