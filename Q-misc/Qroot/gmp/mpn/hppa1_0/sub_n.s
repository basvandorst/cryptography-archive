; HP-PA  mpn_sub_n -- Subtract two limb vectors of the same length > 0 and
; store difference in a third limb vector.

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
; s1_ptr	gr25
; s2_ptr	gr24
; size		gr23

; One might want to unroll this as for other processors, but it turns
; out that the data cache contention after a store makes such
; unrolling useless.  We can't come under 5 cycles/limb anyway.

	.space $TEXT$
	.subspa $CODE$,quad=0,align=8,access=44,code_only

	.align 4
	.export mpn_sub_n,code,priv_lev=3,argw0=gr,argw1=gr,argw2=gr,argw3=gr,rtnval=gr
mpn_sub_n
	.proc
	.callinfo frame=0,no_calls
	.entry

	ldws,ma	 4(0,%r25),%r20
	ldws,ma	 4(0,%r24),%r19

	addib,= -1,%r23,Lend		; check for (SIZE == 1)
	 sub	 %r20,%r19,%r28		; subtract first limbs ignoring cy

Loop	ldws,ma	 4(0,%r25),%r20
	ldws,ma	 4(0,%r24),%r19
	stws,ma	 %r28,4(0,%r26)
	addib,<> -1,%r23,Loop
	 subb	 %r20,%r19,%r28

Lend	stws	 %r28,0(0,%r26)
	addc	%r0,%r0,%r28
	bv	0(%r2)
	 subi	1,%r28,%r28

	.exit
	.procend
