	.file	"bn_mulw.c"
	.version	"01.01"
gcc2_compiled.:
.text
	.align 8
.globl _bn_mul_add_word
	.type	 _bn_mul_add_word,@function
_bn_mul_add_word:
	subl $16,%esp
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %ebx
	movl 36(%esp),%ebp
	addl $12,%ebp
	.align 4

	# ax		L(t)
	# dx		H(t)
	# bx		a
	# cx		w
	# di		r
	# si		c
	xorl %esi,%esi		# c=0
	movl 40(%esp),%ebx	# a => exb
	movl 36(%esp),%edi	# r => edi
	movl 48(%esp),%ecx	# w => ecx
.L110:
	movl %ecx,%eax		# w => eax
	mull (%ebx)		# w * *a 
	addl (%edi),%eax	# L(t)+= *r
	adcl 0,%edx		# H(t)+= carry
	addl %esi,%eax		# L(t)+=c
	adcl 0,%edx		# H(t)+=carry
	movl %eax,(%edi)	# *r=L(t)
	movl %edx,%esi		# c=H(t)
	decl 44(%esp)
	je .L111

	movl %ecx,%eax		# w => eax
	mull 4(%ebx)		# w * *a 
	addl 4(%edi),%eax	# L(t)+= *r
	adcl 0,%edx		# H(t)+= carry
	addl %esi,%eax		# L(t)+=c
	adcl 0,%edx		# H(t)+=carry
	movl %eax,4(%edi)	# *r=L(t)
	movl %edx,%esi		# c=H(t)
	decl 44(%esp)
	je .L111

	movl %ecx,%eax		# w => eax
	mull 8(%ebx)		# w * *a 
	addl 8(%edi),%eax	# L(t)+= *r
	adcl 0,%edx		# H(t)+= carry
	addl %esi,%eax		# L(t)+=c
	adcl 0,%edx		# H(t)+=carry
	movl %eax,8(%edi)	# *r=L(t)
	movl %edx,%esi		# c=H(t)
	decl 44(%esp)
	je .L111

	movl %ecx,%eax		# w => eax
	mull 12(%ebx)		# w * *a 
	addl 12(%edi),%eax	# L(t)+= *r
	adcl 0,%edx		# H(t)+= carry
	addl %esi,%eax		# L(t)+=c
	adcl 0,%edx		# H(t)+=carry
	movl %eax,12(%edi)	# *r=L(t)
	movl %edx,%esi		# c=H(t)
	decl 44(%esp)
	je .L111

	addl $16,%ebx		# a+=4 (4 words)
	addl $16,%edi		# r+=4 (4 words)

	jmp .L110
	.align 8
.L111:
	movl %esi,%eax		# retrun(c)
	popl %ebx
	popl %esi
	popl %edi
	popl %ebp
	addl $16,%esp
	ret
.Lfe1:
	.size	 _bn_mul_add_word,.Lfe1-_bn_mul_add_word
	.align 8
.globl _bn_mul_word
	.type	 _bn_mul_word,@function
_bn_mul_word:
	subl $16,%esp
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %ebx
	movl 36(%esp),%ebp
	addl $12,%ebp
	.align 4

	# ax		L(t)
	# dx		H(t)
	# bx		a
	# cx		w
	# di		r
	# si		c
	xorl %esi,%esi		# c=0
	movl 40(%esp),%ebx	# a => exb
	movl 36(%esp),%edi	# r => edi
	movl 48(%esp),%ecx	# w => ecx
.L210:
	movl %ecx,%eax		# w => eax
	mull (%ebx)		# w * *a 
	addl %esi,%eax		# L(t)+=c
	adcl 0,%edx		# H(t)+=carry
	movl %eax,(%edi)	# *r=L(t)
	movl %edx,%esi		# c=H(t)
	decl 44(%esp)
	je .L211

	movl %ecx,%eax		# w => eax
	mull 4(%ebx)		# w * *a 
	addl %esi,%eax		# L(t)+=c
	adcl 0,%edx		# H(t)+=carry
	movl %eax,4(%edi)	# *r=L(t)
	movl %edx,%esi		# c=H(t)
	decl 44(%esp)
	je .L211

	movl %ecx,%eax		# w => eax
	mull 8(%ebx)		# w * *a 
	addl %esi,%eax		# L(t)+=c
	adcl 0,%edx		# H(t)+=carry
	movl %eax,8(%edi)	# *r=L(t)
	movl %edx,%esi		# c=H(t)
	decl 44(%esp)
	je .L211

	movl %ecx,%eax		# w => eax
	mull 12(%ebx)		# w * *a 
	addl %esi,%eax		# L(t)+=c
	adcl 0,%edx		# H(t)+=carry
	movl %eax,12(%edi)	# *r=L(t)
	movl %edx,%esi		# c=H(t)
	decl 44(%esp)
	je .L211

	addl $16,%ebx		# a+=4 (4 words)
	addl $16,%edi		# r+=4 (4 words)

	jmp .L210
	.align 8
.L211:
	movl %esi,%eax		# retrun(c)
	popl %ebx
	popl %esi
	popl %edi
	popl %ebp
	addl $16,%esp
	ret
.Lfe2:
	.size	 _bn_mul_word,.Lfe2-_bn_mul_word

	.align 8
.globl _bn_sqr_words
	.type	 _bn_sqr_words,@function
_bn_sqr_words:
	subl $16,%esp
	pushl %edi
	pushl %esi
	pushl %ebx
	movl 36(%esp),%edi	# a
	movl 40(%esp),%ebx	# n
	movl 32(%esp),%esi	# r
	.align 4
.L28:
	movl (%edi),%eax	# get a
	mull %eax		# a*a
	movl %eax,(%esi)	# put low into return addr
	movl %edx,4(%esi)	# put high into return addr
	decl %ebx		# n--;
	je .L29

	movl 4(%edi),%eax	# get a
	mull %eax		# a*a
	movl %eax,8(%esi)	# put low into return addr
	movl %edx,12(%esi)	# put high into return addr
	decl %ebx		# n--;
	je .L29

	movl 8(%edi),%eax	# get a
	mull %eax		# a*a
	movl %eax,16(%esi)	# put low into return addr
	movl %edx,20(%esi)	# put high into return addr
	decl %ebx		# n--;
	je .L29

	movl 12(%edi),%eax	# get a
	mull %eax		# a*a
	movl %eax,24(%esi)	# put low into return addr
	movl %edx,28(%esi)	# put high into return addr
	decl %ebx		# n--;
	je .L29

	addl $16,%edi
	addl $32,%esi
	jmp .L28
	.align 8
.L29:
	popl %ebx
	popl %esi
	popl %edi
	addl $16,%esp
	ret
.Lfe3:
	.size	 _bn_sqr_words,.Lfe3-_bn_sqr_words
	.ident	"GCC: (GNU) 2.6.3"
