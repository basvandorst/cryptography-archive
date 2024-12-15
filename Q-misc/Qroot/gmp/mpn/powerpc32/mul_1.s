# PowerPC-32 mpn_mul_1 -- Multiply a limb vector with a limb and store
# the result in a second limb vector.

# Copyright (C) 1993 Free Software Foundation, Inc.

# This file is part of the GNU MP Library.

# The GNU MP Library is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# The GNU MP Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with the GNU MP Library; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.


# INPUT PARAMETERS
# res_ptr	r3
# s1_ptr	r4
# size		r5
# s2_limb	r6

# This is a fairly straightforward implementation.  The timing of the darning
# PC601 is hard to understand, so I will wait to optimize this until I have
# some hardware to play with.

# The code trivially generalizes to 64 bit limbs for the PC620.

	.toc
	.csect .mpn_mul_1[PR]
	.align 2
	.globl mpn_mul_1
	.globl .mpn_mul_1
	.csect mpn_mul_1[DS]
mpn_mul_1:
	.long .mpn_mul_1[PR], TOC[tc0], 0
	.csect .mpn_mul_1[PR]
.mpn_mul_1:
	mtctr	5
# Trick: find an instruction that sets a register to -1 and carry to 1?
# If there is such an insn we can replace the following two insn with that.
# The straightforward way to set a register to 0 and cy to 0 seems impossible.
	addi	10,0,0		# cy_limb := 0
	addic	0,0,0		# cy := 0
Loop:
	lwzu	0,4(4)
	mullw	8,0,6
	adde	7,8,10
	stwu	7,4(3)
	mulhwu	10,0,6
	bdn	Loop

	addze	3,10
	br
