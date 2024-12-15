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

! ADD CODE FOR SMALL MULTIPLIERS!
!1:	ld
!	st
!
!2:	ld	,a
!	addxcc	a,a,x
!	st	x,
!
!3_unrolled:
!	ld	,a
!	addxcc	a,a,x1		! 2a + cy
!	addx	%g0,%g0,x2
!	addcc	a,x1,x		! 3a + c
!	st	x,
!
!	ld	,a
!	addxcc	a,a,y1
!	addx	%g0,%g0,y2
!	addcc	a,y1,x
!	st	x,
!
!4_unrolled:
!	ld	,a
!	srl	a,2,x1		! 4a
!	addxcc	y2,x1,x
!	sll	a,30,x2
!	st	x,
!
!	ld	,a
!	srl	a,2,y1
!	addxcc	x2,y1,y
!	sll	a,30,y2
!	st	x,
!
!5_unrolled:
!	ld	,a
!	srl	a,2,x1		! 4a
!	addxcc	a,x1,x		! 5a + c
!	sll	a,30,x2
!	addx	%g0,x2,x2
!	st	x,
!
!	ld	,a
!	srl	a,2,y1
!	addxcc	a,y1,x
!	sll	a,30,y2
!	addx	%g0,y2,y2
!	st	x,
!
!8_unrolled:
!	ld	,a
!	srl	a,3,x1		! 8a
!	addxcc	y2,x1,x
!	sll	a,29,x2
!	st	x,
!
!	ld	,a
!	srl	a,3,y1
!	addxcc	x2,y1,y
!	sll	a,29,y2
!	st	x,


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
	sub	%g0,%o2,%o2

	cmp	%o3,0xfff
	bgu	Large
	nop

	ld	[%o1+%o2],%o5
	mov	0,%o0
	b	L0
	 add	%o4,-4,%o4
Loop0:
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
	bne,a	Loop0
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
