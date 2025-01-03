; 29000 mpn_mul_1 -- Multiply a limb vector with a single limb and
; store the product in a second limb vector.

; Copyright (C) 1992 Free Software Foundation, Inc.

; This file is part of the GNU MP Library.

; The GNU MP Library is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2, or (at your option)
; any later version.

; The GNU MP Library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

; You should have received a copy of the GNU General Public License
; along with the GNU MP Library; see the file COPYING.  If not, write to
; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.


; INPUT PARAMETERS
; res_ptr	lr2
; s1_ptr	lr3
; size		lr4
; s2_limb	lr5

	.cputype 29050
	.sect .lit,lit
	.text
	.align	4
	.global	_mpn_mul_1
	.word	0x60000
_mpn_mul_1:
	sub	lr4,lr4,8
	jmpt	lr4,Ltail
	 const	gr120,0			; init cylimb reg

	srl	gr117,lr4,3		; divide by 8
	sub	gr117,gr117,1		; count for jmpfdec

Loop:	mtsrim	cr,(8-1)
	loadm	0,0,gr96,lr3
	add	lr3,lr3,32

	multiplu gr104,gr96,lr5
	multmu	 gr96,gr96,lr5
	multiplu gr105,gr97,lr5
	multmu	 gr97,gr97,lr5
	multiplu gr106,gr98,lr5
	multmu	 gr98,gr98,lr5
	multiplu gr107,gr99,lr5
	multmu	 gr99,gr99,lr5
	multiplu gr108,gr100,lr5
	multmu	 gr100,gr100,lr5
	multiplu gr109,gr101,lr5
	multmu	 gr101,gr101,lr5
	multiplu gr110,gr102,lr5
	multmu	 gr102,gr102,lr5
	multiplu gr111,gr103,lr5
	multmu	 gr103,gr103,lr5

	add	gr104,gr104,gr120
	addc	gr105,gr105,gr96
	addc	gr106,gr106,gr97
	addc	gr107,gr107,gr98
	addc	gr108,gr108,gr99
	addc	gr109,gr109,gr100
	addc	gr110,gr110,gr101
	addc	gr111,gr111,gr102
	addc	gr120,gr103,0

	mtsrim	cr,(8-1)
	storem	0,0,gr104,lr2
	jmpfdec	gr117,Loop
	 add	lr2,lr2,32

Ltail:	and	lr4,lr4,(8-1)
	sub	gr118,lr4,1		; count for CR
	jmpt	gr118,Lend
	 sub	lr4,lr4,2
	sub	lr2,lr2,4		; offset res_ptr by one limb

Loop2:	load	0,0,gr116,lr3
	add	lr3,lr3,4
	multiplu gr117,gr116,lr5
	multmu	gr118,gr116,lr5
	add	lr2,lr2,4
	add	gr117,gr117,gr120
	store	0,0,gr117,lr2
	jmpfdec	lr4,Loop2
	 addc	gr120,gr118,0

Lend:	jmpi	lr0
	 or	gr96,gr120,0		; copy
