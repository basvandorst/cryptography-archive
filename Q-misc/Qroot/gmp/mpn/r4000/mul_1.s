 # MIPS R4000 mpn_mul_1 -- Multiply a limb vector with a single limb and
 # store the product in a second limb vector.

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
 # res_ptr	$4
 # s1_ptr	$5
 # size		$6
 # s2_limb	$7

	.text
	.align	 4
	.globl	 mpn_mul_1
	.ent	mpn_mul_1
mpn_mul_1:

 # warm up phase 0
	ld	$8,0($5)

 # warm up phase 1
	addiu	$5,$5,4
	dmultu	$8,$7

	addiu	$6,$6,-1
	.set    noreorder
	.set    nomacro
	beq	$6,$0,$LC0
	 move	$2,$0			# zero cy2
	.set    macro
	.set    reorder

	addiu	$6,$6,-1
	.set    noreorder
	.set    nomacro
	beq	$6,$0,$LC1
	ld	$8,0($5)		# load new s1 limb as early as possible
	.set    macro
	.set    reorder

Loop:	.set    noreorder
	.set    nomacro
	mflo	$10
	mfhi	$9
	addiu	$5,$5,4
	daddu	$10,$10,$2		# add old carry limb to low product limb
	dmultu	$8,$7
	ld	$8,0($5)		# load new s1 limb as early as possible
	addiu	$6,$6,-1		# decrement loop counter
	sltu	$2,$10,$2		# carry from previous addition -> $2
	sd	$10,0($4)
	addiu	$4,$4,4
	bnel	$6,$0,Loop
	 daddu	$2,$9,$2		# add high product limb and carry from addition
	.set    macro
	.set    reorder

 # cool down phase 1
$LC1:	mflo	$10
	mfhi	$9
	daddu	$10,$10,$2
	sltu	$2,$10,$2
	dmultu	$8,$7
	sd	$10,0($4)
	daddu	$2,$9,$2		# add high product limb and carry from addition

 # cool down phase 0
$LC0:	mflo	$10
	mfhi	$9
	daddu	$10,$10,$2
	sltu	$2,$10,$2
	sd	$10,4($4)		# note: offset 4
	daddu	$2,$9,$2		# add high product limb and carry from addition

	j	$31
	.end	mpn_mul_1
