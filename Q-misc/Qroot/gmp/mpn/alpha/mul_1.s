 # Alpha 21064 mpn_mul_1 -- Multiply a limb vector with a limb and store
 # the result in a second limb vector.

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
 # res_ptr	r16
 # s1_ptr	r17
 # size		r18
 # s2_limb	r19

 # This code runs at 42 cycles/limb on the 21064.

 # To improve performance for long multiplications, we would use
 # 'fetch' for S1 and 'fetch_m' for RES.  It's not obvious how to use
 # these instructions without slowing down the general code: 1. We can
 # only have two prefetches in operation at any time in the Alpha
 # architecture.  2. There will seldom be any special alignment
 # between RES_PTR and S1_PTR.  Maybe we can simply divide the current
 # loop into an inner and outer loop, having the inner loop handle
 # exactly one prefetch block?

	.set	noreorder
	.set	noat
.text
	.align	3
	.globl	mpn_mul_1
	.ent	mpn_mul_1 2
mpn_mul_1:
	.frame	$30,0,$26

	ldq	$2,0($17)	# $2 = s1_limb
	addq	$17,8,$17	# s1_ptr++
	subq	$18,1,$18	# size--
	mulq	$2,$19,$3	# $3 = prod_low
	bic	$31,$31,$4	# clear cy_limb
	umulh	$2,$19,$0	# $0 = prod_high
	beq	$18,Lend1	# jump if size was == 1
	ldq	$2,0($17)	# $2 = s1_limb
	addq	$17,8,$17	# s1_ptr++
	subq	$18,1,$18	# size--
	stq	$3,0($16)
	addq	$16,8,$16	# res_ptr++
	beq	$18,Lend2	# jump if size was == 2

	.align	3
Loop:	mulq	$2,$19,$3	# $3 = prod_low
	addq	$4,$0,$0	# cy_limb = cy_limb + 'cy'
	umulh	$2,$19,$4	# $4 = cy_limb
	ldq	$2,0($17)	# $2 = s1_limb
	addq	$17,8,$17	# s1_ptr++
	addq	$3,$0,$3	# $3 = cy_limb + prod_low
	cmpult	$3,$0,$0	# $0 = carry from (cy_limb + prod_low)
	subq	$18,1,$18	# size--
	stq	$3,0($16)
	addq	$16,8,$16	# res_ptr++
	bne	$18,Loop

Lend2:	mulq	$2,$19,$3	# $3 = prod_low
	addq	$4,$0,$0	# cy_limb = cy_limb + 'cy'
	umulh	$2,$19,$4	# $4 = cy_limb
	addq	$3,$0,$3	# $3 = cy_limb + prod_low
	cmpult	$3,$0,$0	# $0 = carry from (cy_limb + prod_low)
	stq	$3,0($16)
	addq	$4,$0,$0	# cy_limb = prod_high + cy
	ret	$31,($26),1
Lend1:	stq	$3,0($16)
	ret	$31,($26),1

	.end	mpn_mul_1
