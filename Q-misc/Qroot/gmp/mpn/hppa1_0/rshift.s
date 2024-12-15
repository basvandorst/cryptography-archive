; HP-PA  mpn_rshift -- 

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
; res_ptr	gr26
; s_ptr		gr25
; size		gr24
; cnt		gr23

	.space $TEXT$
	.subspa $CODE$,quad=0,align=8,access=44,code_only

	.align 4
	.export mpn_rshift,code,priv_lev=3,argw0=gr,argw1=gr,argw2=gr,argw3=gr,rtnval=gr
mpn_rshift
	.proc
	.callinfo frame=64,no_calls
	.entry

	ldws,ma 4(0,%r25),%r22
	mtsar %r23
	addib,= -1,%r24,L$0004
	vshd %r22,%r0,%r28		; compute carry out limb
	ldws,ma 4(0,%r25),%r29
	addib,= -1,%r24,L$0002
	vshd %r29,%r22,%r20

Loop	ldws,ma 4(0,%r25),%r22
	stws,ma %r20,4(0,%r26)
	addib,= -1,%r24,L$0003
	vshd %r22,%r29,%r20
	ldws,ma 4(0,%r25),%r29
	stws,ma %r20,4(0,%r26)
	addib,<> -1,%r24,Loop
	vshd %r29,%r22,%r20

L$0002	stws,ma %r20,4(0,%r26)
	vshd %r0,%r29,%r20
	bv 0(%r2)
	stw %r20,0(0,%r26)
L$0003	stws,ma %r20,4(0,%r26)
L$0004	vshd %r0,%r22,%r20
	bv 0(%r2)
	stw %r20,0(0,%r26)

	.exit
	.procend
