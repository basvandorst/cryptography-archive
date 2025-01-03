# VAX mpn_sub_n -- Subtract two limb vectors of the same length > 0 and store
# difference in a third limb vector.

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
# res_ptr	(sp + 4)
# s1_ptr	(sp + 8)
# s2_ptr	(sp + 12)
# size		(sp + 16)

.text
	.align 1
.globl _mpn_sub_n
_mpn_sub_n:
	.word	0x0
	movl	16(ap),r0
	movl	12(ap),r1
	movl	8(ap),r2
	movl	4(ap),r3
	subl2	r4,r4

Loop:
	movl	(r2)+,r4
	sbwc	(r1)+,r4
	movl	r4,(r3)+
	jsobgtr	r0,Loop

	adwc	r0,r0
	ret
