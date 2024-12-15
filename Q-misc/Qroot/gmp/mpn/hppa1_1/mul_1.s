; HP-PA-1.1 mpn_mul_1 -- Multiply a limb vector with a limb and store
; the result in a second limb vector.

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
; res_ptr	r26
; s1_ptr	r25
; size		r24
; s2_limb	r23

; This code runs in 9 cycles/limb.  It hardly can become faster
; on the 7000 CPU due to data cache contention after a store.
; For the 7100 CPU we should rewrite this quite much due to superscalar
; instruction issue and shorter pipelines...

	.space $TEXT$
	.subspa $CODE$,quad=0,align=8,access=44,code_only

	.align 4
	.export mpn_mul_1,code,priv_lev=3,argw0=gr,argw1=gr,argw2=gr,argw3=gr,rtnval=gr
mpn_mul_1
	.proc
	.callinfo frame=64,no_calls
	.entry

	ldo	 64(%r30),%r30
	fldws,ma 4(%r25),%fr5
	stw	 %r23,-16(%r30)		; move s2_limb ...
	addib,=	 -1,%r24,just_one_limb
	fldws	 -16(%r30),%fr4		; ... into fr4
	add	 %r0,%r0,%r0		; clear carry
	xmpyu	 %fr4,%fr5,%fr5
	fldws,ma 4(%r25),%fr6
	fstds	 %fr5,-16(%r30)
	xmpyu	 %fr4,%fr6,%fr6
	ldw	 -12(%r30),%r19		; least significant limb in product
	ldw	 -16(%r30),%r28

	fstds	 %fr6,-16(%r30)
	addib,=	 -1,%r24,Lend2
	ldw	 -12(%r30),%r1

; Main loop
Loop	fldws,ma 4(%r25),%fr5
	stws,ma	 %r19,4(%r26)
	xmpyu	 %fr4,%fr5,%fr5
	addc	 %r28,%r1,%r19
	ldw	 -16(%r30),%r28
	fstds	 %fr5,-16(%r30)
	;addl	 %r0,%r0,%r0		; there's an unavoidable bubble here
	addib,<> -1,%r24,Loop
	 ldw	 -12(%r30),%r1

Lend2	stws,ma	 %r19,4(%r26)
	addc	 %r28,%r1,%r19
	ldw	 -16(%r30),%r28
	stws,ma	 %r19,4(%r26)
	addc	 %r0,%r28,%r28
	bv	 0(%r2)
	ldo	 -64(%r30),%r30

just_one_limb
	xmpyu	 %fr4,%fr5,%fr5
	fstws	 %fr5L,-16(%r30)
	ldw	 -16(%r30),%r28
	ldo	 -64(%r30),%r30
	bv	 0(%r2)
	fstws	 %fr5R,0(%r26)

	.exit
	.procend
