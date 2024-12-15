	.file "test.s"
	.version "01.00"
	
.text
.align 16

.macro DESROUND14 reg1,reg2
	movl	112(%ebp),%eax       /* eax = s[nnn] */
	xorl	%ebx,%ebx /* move down, so it's done when I have temp-used it */
	xorl	\reg1,%eax			/* eax = reg1^s = u = eax */

	andl	$0xfcfcfcfc, %eax 

	movb	%al,%bl
	movb    %ah,%cl

	movl	      des_SPtrans(%ebx),%edx

	xorl	%edx,\reg2
	movl	0x200+des_SPtrans(%ecx),%edx
	xorl	%edx,\reg2

	shrl	$16,%eax
	movb	%ah,%bl

	andl	$0xfc,%eax
	movl	0x600+des_SPtrans(%ebx),%ebx
	xorl	%ebx,\reg2
	movl	0x400+des_SPtrans(%eax),%ebx
	xorl	%ebx,\reg2
	
	.endm
				
.macro DesRound RoundNo,reg1,reg2
	movl	(\RoundNo * 8)(%ebp),%eax       /* eax = s[nnn] */
	xorl	%ebx,%ebx /* move down, so it's done when I have temp-used it */
	movl	(\RoundNo * 8 + 4)(%ebp),%edx   /* edx = s[nnn+1] */
	xorl	\reg1,%eax			/* eax = reg1^s = u = eax */
	xorl	\reg1,%edx                      /* edx = reg1^s = t = edx */

	.if \RoundNo - 2                        /* I really _hate_ to do it this way */
	.else					/* but it's the easiest way... */
	movl	24(%esp),%ebp                   /* tcmp[0] */
	xorl	%ebp,\reg1                      /* l^=tcmp[0] */
	.endif
	
	andl	$0xfcfcfcfc, %eax 
	andl	$0xcfcfcfcf, %edx

	movb	%al,%bl
	movb    %ah,%cl
	rorl	$4,%edx
	movl	      des_SPtrans(%ebx),%ebp
	movb	%dl,%bl
	xorl	%ebp,\reg2
	movl	0x200+des_SPtrans(%ecx),%ebp
	xorl	%ebp,\reg2
	movb	%dh,%cl
	shrl	$16,%eax
	movl	0x100+des_SPtrans(%ebx),%ebp
	xorl	%ebp,\reg2
	movb	%ah,%bl
	shrl	$16,%edx
	movl	0x300+des_SPtrans(%ecx),%ebp
	xorl	%ebp,\reg2
	movl	0x600+des_SPtrans(%ebx),%ebx
	movl	28(%esp),%ebp
	andl	$0xfc,%eax
	xorl	%ebx,\reg2
	movb	%dh,%cl
	movl	0x400+des_SPtrans(%eax),%ebx
	andl	$0xfc,%edx
	xorl	%ebx,\reg2
	movl	0x700+des_SPtrans(%ecx),%ebx
	xorl	%ebx,\reg2
	movl	0x500+des_SPtrans(%edx),%ebx
	xorl	%ebx,\reg2
.endm

.macro DESROUND15 reg1,reg2

	movl	116(%ebp),%edx			/* edx = s[nnn+1] */
	xorl	%ebx,%ebx			/* move down, so it's done when I have temp-used it */

	xorl	\reg1,%edx                      /* edx = reg1^s = t = edx */

	andl	$0xcfcfcfcf, %edx


	rorl	$4,%edx

	movb	%dl,%bl

	movb	%dh,%cl

	movl	0x100+des_SPtrans(%ebx),%ebp
	xorl	%ebp,\reg2

	shrl	$16,%edx
	movl	0x300+des_SPtrans(%ecx),%ebp
	xorl	%ebp,\reg2
	movl	28(%esp),%ebp
	movb	%dh,%cl
	andl	$0xfc,%edx
	movl	0x700+des_SPtrans(%ecx),%ebx
	xorl	%ebx,\reg2
	movl	0x500+des_SPtrans(%edx),%ebx
	xorl	%ebx,\reg2
.endm

.macro DESROUND_EARLY reg1,reg2
	movl	8(%ebp),%eax       /* eax = s[nnn] */
	xorl	%ebx,%ebx /* move down, so it's done when I have temp-used it */
	movl	12(%ebp),%edx   /* edx = s[nnn+1] */
	xorl	\reg1,%eax			/* eax = reg1^s = u = eax */
	xorl	\reg1,%edx                      /* edx = reg1^s = t = edx */

	andl	$0xfcfcfcfc, %eax 
	andl	$0xcfcfcfcf, %edx

	movb	%al,%bl
	movb    %ah,%cl
	rorl	$4,%edx
	movl	      des_SPtrans(%ebx),%ebp
	movb	%dl,%bl
	xorl	%ebp,\reg2			/* Done with S-Box 0 */
	movl	\reg2,%ebp			/* ebp is now l part */
	andl	$0x02080802,%ebp		/* ebp ANDed with the fixed value */
	jnz	GetOut_Zero			/* != 0 -> get out */
	
	movl	0x200+des_SPtrans(%ecx),%ebp
	xorl	%ebp,\reg2			/* Done with S-Box 0 */
	movl	\reg2,%ebp			/* ebp is now l part */
	andl	$0x060C0903,%ebp		/* ebp ANDed with the fixed value */
	jnz	GetOut_Zero			/* != 0 -> get out */
	
	movb	%dh,%cl
	shrl	$16,%eax
	movl	0x100+des_SPtrans(%ebx),%ebp
	xorl	%ebp,\reg2
	movb	%ah,%bl
	shrl	$16,%edx
	movl	0x300+des_SPtrans(%ecx),%ebp
	xorl	%ebp,\reg2
	movl	28(%esp),%ebp
	movb	%dh,%cl
	andl	$0xfc,%eax
	andl	$0xfc,%edx
	movl	0x600+des_SPtrans(%ebx),%ebx
	xorl	%ebx,\reg2
	movl	0x400+des_SPtrans(%eax),%ebx
	xorl	%ebx,\reg2
	movl	0x500+des_SPtrans(%edx),%ebx
	xorl	%ebx,\reg2
	movl	0x700+des_SPtrans(%ecx),%ebx
	xorl	%ebx,\reg2
.endm


.globl crack_key25
.type crack_key25,@function
			
crack_key25:
	
	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
		
	movl	20(%esp),%eax			/* Load pointer to pointers */
	movl	(%eax),%esi			/* Load left and right part */
	movl	4(%eax),%edi			/* Endian dependant - but this is Intel Only :) */

	movl	28(%esp),%ebp                   /* BP now pointer to key schedule */
		
	/* Here we do the stuff */
	xorl	%ecx,%ecx

	/* Something ahead? Ask FLI to be sure	! */
	/* edi left, esi right */
	
	DESROUND14 %edi,%esi
	
	DesRound 13,%esi,%edi 
	DesRound 12,%edi,%esi 
	DesRound 11,%esi,%edi 
	DesRound 10,%edi,%esi 
	DesRound  9,%esi,%edi 
	DesRound  8,%edi,%esi 
	DesRound  7,%esi,%edi 
 	DesRound  6,%edi,%esi 
	DesRound  5,%esi,%edi 
	DesRound  4,%edi,%esi 
	DesRound  3,%esi,%edi 
	DesRound  2,%edi,%esi   /* This one needs some special tweaking! */

	DESROUND_EARLY %esi,%edi
	
	/* Cleanup and return */
	orl	%edi,%edi	/* This is the left part (I hope) */
	jnz	GetOut_Zero

	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	movl	$1,%eax
	ret

GetOut_Zero:	
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	movl	$0,%eax
	ret
	
	
	
.crack_key25_end:
.size crack_key25,.crack_key25_end-crack_key25
	

.globl crack_key15
.type crack_key15,@function
	
/*void crack_key15(des_key_schedule schedule,DES_LONG tin[2],DES_LONG tout[2]) {*/
		
crack_key15:
	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	movl	24(%esp),%eax			/* Pointer to in and out */
	movl	20(%esp),%ebp			/* Pointer to key schedule */
	movl	(%eax),%esi			/* Load left and righ tparts */
	xorl	%ecx, %ecx			/* Instruction scheduling :) */
	movl	4(%eax),%edi
	
	
	DESROUND15 %edi,%esi

	movl	28(%esp),%eax
	movl	%esi,(%eax)
	movl	%edi,4(%eax)
	
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret

crack_key15_end:
	.size crack_key15,.crack_key15_end-crack_key15
	
	.end







