! Z8000 mpn_mul_1 -- Multiply a limb vector with a limb and store
! the result in a second limb vector.

! Copyright (C) 1993 Free Software Foundation, Inc.

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
! res_ptr	r7
! s1_ptr	r6
! size		r5
! s2_limb	r4

	unseg
	.text
	even
	global _mpn_mul_1
_mpn_mul_1:
	sub	r2,r2
	test	r4
	jr	mi,Lneg

Lpos:	pop	r1,@r6
	ld	r9,r1
	mult	rr8,r4
	add	r1,r1		! shift msb of loaded limb into cy
	jr	c,Lp		! branch if loaded limb's msb is set
	add	r8,r4		! hi_limb += sign_comp2
Lp:	add	r9,r2		! lo_limb += cy_limb
	ldk	r2,#0
	adc	r2,r8
	ld	@r7,r9
	inc	r7,#2
	dec	r5
	jr	ne,Lpos
	ret t

Lneg:	pop	r1,@r6
	ld	r9,r1
	mult	rr8,r4
	add	r8,r1		! hi_limb += sign_comp1
	add	r1,r1
	jr	c,Ln
	add	r8,r4		! hi_limb += sign_comp2
Ln:	add	r9,r2		! lo_limb += cy_limb
	ldk	r2,#0
	adc	r2,r8
	ld	@r7,r9
	inc	r7,#2
	dec	r5
	jr	ne,Lneg
	ret t
