;detp.asm

	;determinant of matrix det P
	;1990 by Yuji KIDA

comment &
	usage :

	dim Detp%(500):bload "detp",Detp%()
	N=(P-1)\2-2:NN=(N+7)\8*8
	dim A%(N-1,NN-1),W%(N-1,NN-1) ;column size = multiple of 8
	call Detp%(W%(0,0),N,PP,DP)

	original BASIC program is :

  640   *DETP(P):'           assume every elements >=0 and <p
  650      local I%,J%,K%,DP,A
  660   DP=1:'                                     determinant mod p
  670   for J%=0 to N-2
  680      for I%=J% to N-1:'                      search non_zero
  690         if W(I%,J%) then cancel for:goto 720
  700      next
  710      cancel for:DP=0:goto 840
  720      if I%<>J% then neg DP
  730        :swap block W(I%,J%..N-1),W(J%,J%..N-1)
  740      A=W(J%,J%)
  750      DP=DP*A@P
  760      Inv=modinv(W(J%,J%),P)
  770      for I%=J%+1 to N-1
  780         C=W(I%,J%)
  790         if C then C=C*Inv@P
  800           :for K%=J%+1 to N-1:W(I%,K%)=(W(I%,K%)-C*W(J%,K%))@P:next
  810      next
  820   next
  830   DP=DP*W(N-1,N-1)@P
  840   return(DP)
&
	
	INCLUDE	UB.MAC


	jmp	start0

	even
base_segment	dw	?
matrixsize	dw	?
modulus		dw	?
segstep		dw	?
var_J		dw	?
invword		dw	?
multiplier	dw	?
counter		dw	?
detoffset	dw	?
detsegment	dw	?
det		dw	?



modinvword:
; inp : ax (GCD with 'modulus' must be 1)
; out : ax = 1/ax mod 'modulus'
; destroy : nothing

;	cmp	ax,1
;	je	modinvwordret
	push	bx
	push	dx
	push	si
	push	di

	cmp	ax,bx		;bx=modulus
	jb	modinvw10
	xor	dx,dx
	div	bx
	mov	ax,dx
	cmp	ax,1
	je	modinvwout
modinvw10:
	xchg	ax,bx		;##
	xor	si,si		;coef1
	mov	di,1		;coef2
modinvw20:
	xor	dx,dx
	div	bx
	push	dx		;remainder
	push	di		;coef2
	mul	di
	div	cs:[modulus]
	mov	ax,si
	sub	ax,dx
	jae	modinvw30
	add	ax,cs:[modulus]
modinvw30:
	mov	di,ax		;new coef2=coef1-quotient*coef2
	pop	si		;new coef1=old coef2
	mov	ax,bx
	pop	bx
	cmp	bx,1
	jne	modinvw20	;GCD must 1 otherwise endlessloop
	mov	ax,di
modinvwout:
	pop	di
	pop	si
	pop	dx
	pop	bx
modinvwordret:
	ret


getvalue:
	lodsw
	cmp	ax,1
	jne	getvalueilg
	lodsw
	clc
	ret

getvalueilg:
	stc
	ret



detpilg:
	mov	bx,AR1
	mov	word ptr cs:[bx],8001h	;error
	jmp	detpret



start0:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	lds_si	v2
	call	getvalue
	jc	detpilg
	mov	cs:[matrixsize],ax

	lds_si	v3
	call	getvalue
	jc	detpilg
	mov	cs:[modulus],ax

	lds_si	v4
	mov	cs:[detoffset],si
	mov	cs:[detsegment],ds

	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	[det],1		;set initial det

	mov	bx,V1+2
	mov	ax,[bx]		;segment of W()
	mov	ds,ax
	xor	si,si
	mov	ax,[si]		;ax=dimension
	cmp	ax,2
	jne	detpilg
	mov	si,8
	mov	ax,[si]		;size of 2 dim -1
	inc	ax
	xor	dx,dx
	mov	cx,8
	div	cx
	or	dx,dx
	jnz	detpilg
	mov	cs:[segstep],ax

	mov	ax,ds
	add	ax,arrayheadseg
	mov	ds,ax
	mov	es,ax

	mov	cs:[base_segment],ax

	mov	bx,cs:[modulus]
	mov	cx,cs:[matrixsize]
detp10:
	push	cx
	xor	si,si
	mov	cx,cs:[matrixsize]
detp20:
	mov	ax,[si]
	test	ah,80h
	jz	detp30
	and	ah,7fh
	neg	ax
detp30:
	add	ax,7fffh
	xor	dx,dx
	div	bx	;modulus
	mov	[si],dx
	add	si,2
	loop	detp20

	mov	ax,ds
	add	ax,cs:[segstep]
	mov	ds,ax
	mov	es,ax

	pop	cx
	loop	detp10

	;  670   for J%=0 to N-2

	mov	ax,cs:[base_segment]
	mov	ds,ax
	xor	si,si		;ds:si = adr of W(0,J)

	mov	ax,cs:[matrixsize]
	dec	ax
	jnz	detp105
	jmp	detp200
detp105:
	mov	cs:[counter],ax
	mov	cs:[var_J],0

	;main loop
detp110:
	mov	ax,ds
	mov	es,ax

	;  680      for I%=J% to N-1:'                      search non_zero
	;  690         if W(I%,J%) then cancel for:goto 720
	;  700      next
	;  710      cancel for:DP=0:goto 840

	mov	cx,cs:[matrixsize]
	sub	cx,cs:[var_J]
	xor	ax,ax
detp120:
	cmp	es:[si],ax
	jne	detp130
	mov	dx,es
	add	dx,cs:[segstep]
	mov	es,dx
	loop	detp120
	jmp	detpequ0	;det=0

	;  720      if I%<>J% then neg DP

detp130:
	mov	ax,ds
	mov	dx,es
	cmp	ax,dx
	je	detp150

	mov	ax,bx		;neg det
	sub	ax,cs:[det]	;
	mov	cs:[det],ax	;

	;  730        :swap block W(I%,J%..N-1),W(J%,J%..N-1)

	mov	cx,cs:[matrixsize]	;exchange rows
	sub	cx,cs:[var_J]
	mov	di,si
detp140:
	mov	ax,es:[di]
	xchg	[di],ax
	mov	es:[di],ax
	add	di,2
	loop	detp140

	mov	ax,ds
	mov	es,ax

	;  740      A=W(J%,J%)
	;  750      DP=DP*A@P
detp150:
	mov	ax,[si]		;calc new det
	mul	cs:[det]
	div	bx		;##
	mov	cs:[det],dx

	;  770      for I%=J%+1 to N-1

	mov	cx,cs:[matrixsize]
	sub	cx,cs:[var_J]
	dec	cx
	jcxz	detp190

	;  760      Inv=modinv(W(J%,J%),P)

	mov	ax,[si]
	cmp	ax,1
	je	detp155
	call	modinvword
detp155:
	mov	cs:[invword],ax

	;  780         C=W(I%,J%)
	;  790         if C then C=C*Inv@P

detp160:
	mov	ax,es
	add	ax,cs:[segstep]
	mov	es,ax

	;740

	mov	ax,es:[si]
	or	ax,ax
	jz	detp185

	;  800           :for K%=J%+1 to N-1:W(I%,K%)=(W(I%,K%)-C*W(J%,K%))@P
	;				:next
	;  810      next

	push	cx
	mul	cs:[invword]
	div	bx		;##
	mov	ax,dx
	or	ax,ax
	jz	detp170
	neg	ax
	add	ax,bx		;##
detp170:
	mov	cs:[multiplier],ax

	mov	cx,cs:[matrixsize]
	sub	cx,cs:[var_J]
	mov	di,si
detp180:
	mov	ax,[di]
	mul	cs:[multiplier]
	div	bx		;##

	mov	ax,dx
	xor	dx,dx
	add	ax,es:[di]
	adc	dx,0
	div	bx		;##
	mov	es:[di],dx
	add	di,2
	loop	detp180

	pop	cx
detp185:
	loop	detp160

	;  820   next

detp190:	
	inc	cs:[var_J]
	mov	ax,ds
	add	ax,cs:[segstep]
	mov	ds,ax
	add	si,2

	dec	cs:[counter]
	jz	detp200
	jmp	detp110

detpequ0:
	mov	cs:[det],0

	;  830   DP=DP*W(N-1,N-1)@P

detp200:
	mov	ax,[si]		;final element
	mul	cs:[det]
	div	bx
	mov	ax,dx
	lds	si,dword ptr cs:[detoffset]
	or	ax,ax
	jz	detp210
	mov	word ptr [si],1
	add	si,2
detp210:
	mov	[si],ax

	;  840   return(DP)

detpret:
	return


