! sparc8 mpn_mul_1 -- Multiply a limb vector with a single limb and
! store the product in a second limb vector.

! Copyright (C) 1992 Free Software Foundation, Inc.

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

! 921104 -- this code is ready apart from that the prologoue need to be
! hacked to handle sizes that are not a multiple of 8.

! Speed is just below 9 cycles/limb on a Viking chip.
! With infinite unrolling it would become 8 cycles/limb.
! With Viking docs, it might be possible to save a cycle or two per limb.

.text
	.align 4
	.global _mpn_mul_1
	.proc 1
_mpn_mul_1:
	addcc	%g0,0,%g1		! clear cy_limb and carry flag
	add	%o0,-32,%o0
Loop:
	ld	[%o1+0],%o4
	add	%o0,32,%o0
	umul	%o4,%o3,%o5
	ld	[%o1+4],%o4
	addcc	%o5,%g1,%g2

	rd	%y,%g1
	umul	%o4,%o3,%o5
	st	%g2,[%o0+0]
	addxcc	%o5,%g1,%g2
	ld	[%o1+8],%o4
	rd	%y,%g1
	umul	%o4,%o3,%o5
	st	%g2,[%o0+4]
	addxcc	%o5,%g1,%g2
	ld	[%o1+12],%o4
	rd	%y,%g1
	umul	%o4,%o3,%o5
	st	%g2,[%o0+8]
	addxcc	%o5,%g1,%g2
	ld	[%o1+16],%o4
	rd	%y,%g1
	umul	%o4,%o3,%o5
	st	%g2,[%o0+12]
	addxcc	%o5,%g1,%g2
	ld	[%o1+20],%o4
	rd	%y,%g1
	umul	%o4,%o3,%o5
	st	%g2,[%o0+16]
	addxcc	%o5,%g1,%g2
	ld	[%o1+24],%o4
	rd	%y,%g1
	umul	%o4,%o3,%o5
	st	%g2,[%o0+20]
	addxcc	%o5,%g1,%g2
	ld	[%o1+28],%o4
	rd	%y,%g1
	umul	%o4,%o3,%o5
	st	%g2,[%o0+24]
	addxcc	%o5,%g1,%g2
	rd	%y,%g1

	addx	%g1,0,%g1
	addcc	%o2,-8,%o2
	st	%g2,[%o0+28]
	bne	Loop
	add	%o1,32,%o1

Lend:
	retl
	 add	%g1,0,%o0
