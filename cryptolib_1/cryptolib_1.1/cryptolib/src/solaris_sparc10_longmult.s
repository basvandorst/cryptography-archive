	.file	"longmult.c"
.section	".text"
	.align 4
	.global LMULT
	.type	 LMULT,#function
	.proc	017
LMULT:
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	mov 0,%i5
	cmp %i5,%i3
	bge .LL3
	mov 0,%g4
	mov %i0,%i4
.LL5:
	ld [%i2],%o7
	! Inlined umul_ppmm
	wr	%g0,%i1,%y	! SPARC has 0-3 delay insn after a wr
	sra	%o7,31,%g2	! Don't move this insn
	and	%i1,%g2,%g2	! Don't move this insn
	andcc	%g0,0,%g1	! Don't move this insn
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,%o7,%g1
	mulscc	%g1,0,%g1
	add	%g1,%g2,%i0
	rd	%y,%g3
	add %g3,%i5,%g3
	cmp %g3,%i5
	addx %g0,%i0,%i0
	add %i2,4,%i2
	ld [%i4],%g2
	add %g4,1,%g4
	add %g3,%g2,%g2
	st %g2,[%i4]
	cmp %g2,%g3
	addx %g0,%i0,%i5
	cmp %g4,%i3
	bl .LL5
	add %i4,4,%i4
.LL3:
	ret
	restore %g0,%i5,%o0
.LLfe1:
	.size	 LMULT,.LLfe1-LMULT
	.align 4
	.global BUILDDIAG
	.type	 BUILDDIAG,#function
	.proc	020
BUILDDIAG:
	!#PROLOGUE# 0
	!#PROLOGUE# 1
	mov 0,%g3
	sll %o2,2,%o2
.LL7:
	ld [%g3+%o1],%o4
	ld [%g3+%o1],%o3
	! Inlined umul_ppmm
	wr	%g0,%o4,%y	! SPARC has 0-3 delay insn after a wr
	sra	%o3,31,%g2	! Don't move this insn
	and	%o4,%g2,%g2	! Don't move this insn
	andcc	%g0,0,%g1	! Don't move this insn
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,0,%g1
	add	%g1,%g2,%o4
	rd	%y,%o3
	st %o4,[%o0+4]
	st %o3,[%o0]
	add %g3,4,%g3
	cmp %g3,%o2
	bl .LL7
	add %o0,8,%o0
	retl
	nop
.LLfe2:
	.size	 BUILDDIAG,.LLfe2-BUILDDIAG
	.align 4
	.global SQUAREINNERLOOP
	.type	 SQUAREINNERLOOP,#function
	.proc	020
SQUAREINNERLOOP:
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	mov %i1,%o7
	mov %i4,%o0
	mov %i0,%i5
	mov 0,%g4
	mov %i3,%i4
	sll %i4,2,%g2
	add %g2,%i2,%i3
	add %i5,4,%i2
.LL11:
	ld [%i3],%o1
	! Inlined umul_ppmm
	wr	%g0,%o7,%y	! SPARC has 0-3 delay insn after a wr
	sra	%o1,31,%g2	! Don't move this insn
	and	%o7,%g2,%g2	! Don't move this insn
	andcc	%g0,0,%g1	! Don't move this insn
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,%o1,%g1
	mulscc	%g1,0,%g1
	add	%g1,%g2,%i1
	rd	%y,%g3
	add %g3,%g3,%i0
	cmp %i0,%g3
	add %i3,4,%i3
	add %i4,1,%i4
	ld [%i5],%g2
	addx %g0,%g4,%g3
	add %i0,%g2,%g2
	cmp %g2,%i0
	addx %g0,%g3,%g4
	st %g2,[%i5]
	add %i1,%i1,%g3
	cmp %g3,%i1
	add %i5,4,%i5
	ld [%i2],%g2
	addx %g0,0,%i0
	add %g3,%g2,%g2
	cmp %g2,%g3
	addx %g0,%i0,%i0
	add %g2,%g4,%g3
	cmp %g3,%g2
	addx %g0,%i0,%g4
	st %g3,[%i2]
	cmp %i4,%o0
	bl .LL11
	add %i2,4,%i2
	cmp %g4,0
	be .LL15
	add %i5,4,%i5
	sll %o0,1,%i0
	cmp %i4,%i0
	bge .LL15
	nop
	ld [%i5],%g2
.LL17:
	add %i4,1,%i4
	add %g4,%g2,%g3
	cmp %g3,%g2
	addx %g0,0,%g4
	st %g3,[%i5]
	cmp %g4,0
	be .LL15
	add %i5,4,%i5
	cmp %i4,%i0
	bl,a .LL17
	ld [%i5],%g2
.LL15:
	ret
	restore
.LLfe3:
	.size	 SQUAREINNERLOOP,.LLfe3-SQUAREINNERLOOP
	.ident	"GCC: (GNU) 2.5.8"
