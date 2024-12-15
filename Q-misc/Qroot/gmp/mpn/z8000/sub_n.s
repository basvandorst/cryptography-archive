! Z8000 mpn_sub_n -- Subtract two limb vectors of the same length > 0 and
! store difference in a third limb vector.

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
! s2_ptr	r5
! size		r4

! If we are really crazy, we can use push to write a few result words
! backwards, using push just because it is faster than reg+disp.  We'd
! then add 2x the number of words written to r7...

	unseg
	.text
	even
	global _mpn_sub_n
_mpn_sub_n:
	pop	r0,@r6
	pop	r1,@r5
	sub	r0,r1
	ld	@r7,r0
	dec	r4
	jr	eq,Lend
Loop:	pop	r0,@r6
	pop	r1,@r5
	sbc	r0,r1
	inc	r7,#2
	ld	@r7,r0
	dec	r4
	jr	ne,Loop
Lend:	ld	r2,r4		! use 0 already in r4
	adc	r2,r2
	ret	t
