; 29000 mpn_lshift --

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
; s2_ptr	lr4
; size		lr5

; We use the loadm/storem instructions and operate on chunks of 8
; limbs/per iteration, until less than 8 limbs remain.

	.sect .lit,lit
	.text
	.align	4
	.global	_mpn_lshift
	.word	0x60000
_mpn_lshift:
	sll	gr116,lr4,2
	add	lr3,gr116,lr3
	add	lr2,gr116,lr2
	sub	lr3,lr3,4
	load	0,0,gr119,lr3

	subr	gr116,lr5,32
	srl	gr96,gr119,gr116	; return value
	sub	lr4,lr4,1		; actual loop count is SIZE - 1

	srl	gr117,lr4,3		; chuck count = (actual count) / 8
	cpeq	gr118,gr117,0
	jmpt	gr118,Ltail
	 mtsr	fc,lr5

	sub	gr117,gr117,2		; count for jmpfdec

; Main loop working 8 limbs/iteration.
Loop:	sub	lr3,lr3,32
	mtsrim	cr,(8-1)
	loadm	0,0,gr100,lr3

	extract	gr109,gr119,gr107
	extract	gr108,gr107,gr106
	extract	gr107,gr106,gr105
	extract	gr106,gr105,gr104
	extract	gr105,gr104,gr103
	extract	gr104,gr103,gr102
	extract	gr103,gr102,gr101
	extract	gr102,gr101,gr100

	sub	lr2,lr2,32
	mtsrim	cr,(8-1)
	storem	0,0,gr102,lr2
	jmpfdec	gr117,Loop
	 or	gr119,gr100,0

; Code for the last up-to-7 limbs.

	and	lr4,lr4,(8-1)
Ltail:	cpeq	gr118,lr4,0
	jmpt	gr118,Lend
	 sub	lr4,lr4,2		; count for jmpfdec

Loop2:	sub	lr3,lr3,4
	load	0,0,gr116,lr3
	extract	gr117,gr119,gr116
	sub	lr2,lr2,4
	store	0,0,gr117,lr2
	jmpfdec	lr4,Loop2
	 or	gr119,gr116,0

Lend:	extract	gr117,gr119,0
	sub	lr2,lr2,4
	jmpi	lr0
	 store	0,0,gr117,lr2
