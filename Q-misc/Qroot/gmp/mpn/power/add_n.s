# IBM POWER mpn_add_n -- Add two limb vectors of equal, non-zero length.

# Copyright (C) 1992 Free Software Foundation, Inc.

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
# s2_ptr	r5
# size		r6

	.toc
	.extern mpn_add_n[DS]
	.extern .mpn_add_n
.csect [PR]
	.align 2
	.globl mpn_add_n
	.globl .mpn_add_n
	.csect mpn_add_n[DS]
mpn_add_n:
	.long .mpn_add_n, TOC[tc0], 0
	.csect [PR]
.mpn_add_n:
	mtctr	6		# copy size into CTR
	l	8,0(4)		# load least significant s1 limb
	l	0,0(5)		# load least significant s2 limb
	cal	3,-4(3)		# offset res_ptr, it's updated before used
	a	7,0,8		# add least significant limbs, set cy
	bdz	Lend		# If done, skip loop
Loop:	lu	8,4(4)		# load s1 limb and update s1_ptr
	lu	0,4(5)		# load s2 limb and update s2_ptr
	stu	7,4(3)		# store previous limb in load latecny slot
	ae	7,0,8		# add new limbs with cy, set cy
	bdn	Loop		# decrement CTR and loop back
Lend:	st	7,4(3)		# store ultimate result limb
	lil	3,0		# load cy into ...
	aze	3,3		# ... return value register
	br
