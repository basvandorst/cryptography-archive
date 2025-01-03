! SPARC mpn_mul_1 -- Multiply a limb vector with a limb and store
! the result in a second limb vector.

! Copyright (C) 1992, 1993 Free Software Foundation, Inc.

! This file is part of the GNU MP Library.

! The GNU MP Library is free software; you can redistribute it and/or modify
! it under the terms of the GNU General Public License as published by
! the Free Software Foundation; either version 2, or (at your option)
! any later version.

! The GNU MP Library is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
! GNU General Public License for more details.

! You should have received a copy of the GNU General Public License
! along with the GNU MP Library; see the file COPYING.  If not, write to
! the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.


! INPUT PARAMETERS
! res_ptr	o0
! s1_ptr	o1
! size		o2
! s2_limb	o3

 ADD CODE FOR SMALL MULTIPLIERS!
1:	ld
	st

2:	ld	,a
	addxcc	a,a,x
	st	x,

3_unrolled:
	ld	,a
wrong:	addxcc	a,a,x1		! 2a + cy
	addx	%g0,%g0,x2
	addcc	a,x1,x		! 3a + c
	st	x,

	ld	,a
	addxcc	a,a,y1
	addx	%g0,%g0,y2
	addcc	a,y1,x
	st	x,

4_unrolled:
	ld	,a
	srl	a,2,x1		! 4a
	addxcc	y2,x1,x
	sll	a,30,x2
	st	x,

	ld	,a
	srl	a,2,y1
	addxcc	x2,y1,y
	sll	a,30,y2
	st	x,

5_unrolled:
	ld	,a
	srl	a,2,x1		! 4a
	addxcc	a,x1,x		! 5a + c
	sll	a,30,x2
	addx	%g0,x2,x2
	st	x,

	ld	,a
	srl	a,2,y1
	addxcc	a,y1,x
	sll	a,30,y2
	addx	%g0,y2,y2
	st	x,

8_unrolled:
	ld	,a
	srl	a,3,x1		! 8a
	addxcc	y2,x1,x
	sll	a,29,x2
	st	x,

	ld	,a
	srl	a,3,y1
	addxcc	x2,y1,y
	sll	a,29,y2
	st	x,


.text
	.align 4
	.global _mpn_mul_1
	.proc	016
_mpn_mul_1:
	! Make S1_PTR and RES_PTR point at the end of their blocks
	! and put (- 4 x SIZE) in index/loop counter.
	sll	%o2,2,%o2
	add	%o0,%o2,%o4	! RES_PTR in o4 since o0 is retval
	add	%o1,%o2,%o1

	cmp	%o3,0xfff
	bgu	Large
	 sub	%g0,%o2,%o2

	cmp	%o3,0x1f
	bgu	Medium
	 sll	%o3,3,%g1

	sethi	%hi(Tab),%g2
	or	%g2,%lo(Tab),%g2
	jmp	%g2+%g1
	 nop
M.0:
	st	%g0,[%o4+%o2]
Loop0:	addcc	%o2,4,%o2
	bne	Loop0
	st	%g0,[%o4+%o2]
	retl
	mov	0,%o0

M.1:	add	%o4,-4,%o4
Loop1:	ld	[%o1+%o2],%o5
	addcc	%o2,4,%o2
	bne	Loop1
	st	%o5,[%o4+%o2]
	retl
	mov	0,%o0

M.2:	add	%o4,-4,%o4
Loop2:	ld	[%o1+%o2],%o5
	subcc	%g0,%g2,%g0		! cy <= g2
	addxcc	%o5,%o5,%g1
	addx	%g0,%g0,%g2		! g2 <= cy
	addcc	%o2,4,%o2
	bne	Loop2
	st	%g1,[%o4+%o2]
	retl
	mov	0,%o0

M.5:	add	%o4,-4,%o4
	mov	0,%g4

	ld	[%o1+%o2],%o5
	subcc	%g0,%g4,%g0		! cy <= g4

	srl	%o5,2,%g1		| lo = 4x
	addcc	%g2,%g1,%g1		! lo += cylimb
	addxcc	%o5,%g1,%g1		! XXX
	sll	%o5,30,g2

	addx	%g0,%g4,%g4		! g2 <= cy
	addcc	%o2,4,%o2
	bne	Loop5
	st	%g1,[%o4+%o2]
	retl
	addx	


	subcc	%g0,%g2,%g0		! cy <= g2
	addxcc	
	addxcc	%o5,%o5,%g3		! 2a + cy
	addx	%g0,%g0,%g4
	addcc	%o5,%g3,%g1		! 3a + c
	addx	%g0,%g4,%g4		! g2 <= cy
	addcc	%o2,4,%o2
	bne	Loop5
	st	%g1,[%o4+%o2]
	retl
	addx	%g4,


Loop3:	ld	[%o1+%o2],%o5
	subcc	%g0,%g2,%g0		! cy <= g2
	addxcc	%o5,%o5,%g1
	addx	%g0,%g0,%g2		! g2 <= cy
	addcc	%o2,4,%o2
	bne	Loop3
	st	%g1,[%o4+%o2]
	retl
	mov	0,%o0

Tab:	b	M.0	! 0
	nop
	b	M.1	! 1
	nop
	b	M.2	! 2
	nop
	b	M.3	! 3
	nop
	b	M.4	! 4
	nop
	b	M.5	! 5
	nop
	b	M.6	! 6
	nop
	b	M.7	! 7
	nop
	b	M.8	! 8
	nop
	b	M.9	! 9
	nop
	b	Medium	!10
	nop
	b	Medium	!11
	nop
	b	Medium	!12
	nop
	b	Medium	!13
	nop
	b	Medium	!14
	nop
	b	Medium	!15
	nop
	b	Medium	!16
	nop
	b	Medium	!17
	nop
	b	Medium	!18
	nop
	b	Medium	!19
	nop
	b	Medium	!20
	nop
	b	Medium	!21
	nop
	b	Medium	!22
	nop
	b	Medium	!23
	nop
	b	Medium	!24
	nop
	b	Medium	!25
	nop
	b	Medium	!26
	nop
	b	Medium	!27
	nop
	b	Medium	!28
	nop
	b	Medium	!29
	nop
	b	Medium	!30
	nop
	b	Medium	!31
	nop


Medium:
	ld	[%o1+%o2],%o5
	mov	0,%o0
	b	L0
	 add	%o4,-4,%o4
LoopM:
	st	%g1,[%o4+%o2]
L0:	wr	%g0,%o3,%y
	sra	%o5,31,%g2
	and	%o3,%g2,%g2
	andcc	%g1,0,%g1
	mulscc	%g1,%o5,%g1
 	mulscc	%g1,%o5,%g1
 	mulscc	%g1,%o5,%g1
 	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,0,%g1
	sra	%g1,20,%g4
	sll	%g1,12,%g1
 	rd	%y,%g3
	srl	%g3,20,%g3
	or	%g1,%g3,%g1

	addcc	%g1,%o0,%g1
	addx	%g2,%g4,%o0	! add sign-compensation and cy to hi limb
	addcc	%o2,4,%o2	! loop counter
	bne,a	LoopM
	 ld	[%o1+%o2],%o5

	retl
	st	%g1,[%o4+%o2]


Large:	ld	[%o1+%o2],%o5
	mov	0,%o0
	sra	%o3,31,%g4	! g4 = mask of ones iff S2_LIMB < 0
	b	L1
	 add	%o4,-4,%o4
Loop:
	st	%g3,[%o4+%o2]
L1:	wr	%g0,%o5,%y
	and	%o5,%g4,%g2	! g2 = S1_LIMB iff S2_LIMB < 0, else 0
	andcc	%g0,%g0,%g1
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
	mulscc	%g1,%g0,%g1
	rd	%y,%g3
	addcc	%g3,%o0,%g3
	addx	%g2,%g1,%o0	! add sign-compensation and cy to hi limb
	addcc	%o2,4,%o2	! loop counter
	bne,a	Loop
	 ld	[%o1+%o2],%o5

	retl
	st	%g3,[%o4+%o2]
