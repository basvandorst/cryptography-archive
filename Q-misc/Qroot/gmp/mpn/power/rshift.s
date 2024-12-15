# IBM POWER mpn_rshift -- 

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
# s_ptr		r4
# size		r5
# cnt		r6

	.toc
	.extern mpn_rshift[DS]
	.extern .mpn_rshift
.csect [PR]
	.align 2
	.globl mpn_rshift
	.globl .mpn_rshift
	.csect mpn_rshift[DS]
mpn_rshift:
	.long .mpn_rshift, TOC[tc0], 0
	.csect [PR]
.mpn_rshift:
	sfi	8,6,32
	mtctr	5		# put limb count in CTR loop register
	l	0,0(4)		# read least significant limb
	ai	9,3,-4		# adjust res_ptr since it's offset in the stu:s
	sle	3,0,8		# compute carry limb, and init MQ register
	bdz	Lend2		# if just one limb, skip loop
	lu	0,4(4)		# read 2:nd least significant limb
	sleq	7,0,8		# compute least significant limb of result
	bdz	Lend		# if just two limb, skip loop
Loop:	lu	0,4(4)		# load next higher limb
	stu	7,4(9)		# store previous result during read latency
	sleq	7,0,8		# compute result limb
	bdn	Loop		# loop back until CTR is zero
Lend:	stu	7,4(9)		# store 2:nd most significant limb
Lend2:	sre	7,0,6		# compute most significant limb
	st      7,4(9)		# store it"				\
	br
