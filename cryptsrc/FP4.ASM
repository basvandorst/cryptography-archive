	page	53,132
	.8087
;converts ascii numeric strings to short or long reals.  see frame2.str
;for parameters.
;strings may be in numerous forms summarized as follows:
;[+|-][i..i][.][f..f][e|E[+|-]p..p]
	include frame2.str
	include fp4b.asm
	extrn	two2x:far
fp4	segment 'crya'
	assume	cs:fp4,ds:fp4b
	public	asc2fp
asc2fp	proc	far
	push	ds
	push	es
	push	di
	push	si
	push	cx
	push	bx
	push	dx
	mov	ax,fp4b
	mov	ds,ax
	mov	bcdp, offset bcd
	mov	digcnt,0
	mov	flag,0
	fldz			;assume +0, prepares 0 bcd
	les	di,dword ptr [bp].strngb	;->ascii string
	mov	cx,[bp].strngl			;length of string
;determine sign
	cmp byte ptr es:[di],'+'
	jne	a2f10
	jmp short a2f20 	;positive number, skip sign
a2f10:
	cmp byte ptr es:[di],'-'
	jne	a2f20		;positive number, no sign
	fchs			;negative number, mark it
	inc	di
	dec	cx
a2f20:
	fbstp	bcd		;set the template
	mov	strngp,di	;start of string sans leading sign
	mov	strcnt,cx	;..and its length
	jcxz	nan		;invalid number check
	mov	al,'.'
	repne scasb
	jne	a2f30		;no decimal, assume integer
	or	flag,01h	;flag decimal
	dec	di
	mov	decp,di 	;->decimal point
a2f30:
	mov	di,strngp
	mov	cx,strcnt
	mov	al,'e'          ;look for exponent e
	repne scasb
	je	a2f40		;exponent found
	mov	di,strngp
	mov	cx,strcnt
	mov	al,'E'          ;look for exponent E
	repne scasb
	jne	a2f50
a2f40:
	or	flag,02h	;flag exponent found
	dec	di
	mov	exp,di		;-> e or E
;now what do I have? dd, dde, dd.ff, dd.ffe, .ff, .ffe
a2f50:
	xor	bx,bx
	mov	bl,flag
	shl	bx,1
	jmp	case[bx]

;invalid character, no number
nan:
	mov	ax,1		;not a number
	fld	indef		;return an indefinite
nan10:
	call	put
	stc			;bad return
a2fxit:
	pop	dx
	pop	bx
	pop	cx
	pop	si
	pop	di
	pop	es
	pop	ds
	ret

;had better be a simple integer
;string of form iii
xx:
	mov	di,strngp
	mov	intp,di
	call	trimi		;trim leading 0s
	mov	cx,intcnt
	mov	di,intp
	call	formbcd
	jnc	xx10
xx03:				;invalid number detected
	cmp	ax,1
	je	nan		;garbage string
xx05:
	fld	infin		;too big
	jmp	nan10
xx10:
	fbld	bcd
	call	put
	xor	ax,ax
	jmp	a2fxit

;string of form iiie[+|-]ppp
xxe:
	mov	di,strngp
	mov	intp,di 	;->beginning of integer
	mov	cx,exp
	sub	cx,di
	mov	intcnt,cx	;->length of integer
	add	di,strcnt
	sub	di,exp
	dec	di
	mov	expcnt,di	;length of exponent
	inc	exp		;->start of exponent, skipping E
	call	exponent	;develop the exponent
	jnc	xxe20
badexp:
	cmp	ax,1
	jne	xxe10
	jmp	nan		;garbage in exponent
xxe10:
	jmp	xx05		;exponent too big
xxe20:
	mov	di,intp
	call	trimi		;trim leading zeroes
	mov	di,intp
	mov	cx,intcnt
	call	formbcd 	;convert to bcd
	jnc	xxe30
	jmp	xx03		;bad number
xxe30:
	mov	ax,p10
	cmp	intcnt,0	;if no integers (eg all 0s)
	je	xxe35		;..no effect on magnitude
	add	ax,intcnt
	dec	ax
xxe35:
	call	testexp
	jnc	xxe40
	mov	ax,3		;exponent out of range
	jmp	xx05
xxe40:
	call	ten2x
	xor	ax,ax
	jmp	a2fxit

;string of form iii.fff
xxd:
	mov	di,strngp
	mov	intp,di 	;->start of integer portion
	mov	cx,decp
	sub	cx,di
	mov	intcnt,cx	;count of integers digits
	inc	decp		;->start of fraction
	add	di,strcnt
	sub	di,decp
	mov	decnt,di	;count of fraction digits
	call	trimf		;trim trailing zeroes from fraction
	call	trimi		;trim leading zeroes from integer
	mov	di,decp
	mov	cx,decnt
	call	formbcd
	jnc	xxd10
	jmp	xx03		;number invalid or too big
xxd10:
	mov	di,intp
	mov	cx,intcnt
	call	formbcd
	jnc	xxd20
	jmp	xx03		;number invalid or too big
xxd20:
	mov	ax,decnt	;set up exponent
	neg	ax
	mov	p10,ax
	or	flag,80h	;make exponent sign negative
	jmp	xxe40

;string of form iii.fffE[+|-]ppp
xxde:
	mov	di,strngp	;figure length of iii, fff, and +-ppp
	mov	si,decp
	mov	bx,exp
	mov	intp,di 	;->start of iii
	mov	ax,si
	sub	ax,di
	mov	intcnt,ax	;length of iii
	inc	si
	mov	decp,si 	;->start of fff
	mov	ax,bx
	sub	ax,si
	mov	decnt,ax	;length of fff
	inc	bx
	mov	exp,bx		;->start of [+|-]ppp
	add	di,strcnt
	sub	di,bx
	mov	expcnt,di	;length of exponent
	call	trimf		;trim trailing 0s of fff
	mov	cx,decnt
	mov	di,decp
	call	formbcd 	;convert to bcd
	jnc	xxde10
	jmp	xx03		;..invalid or too big
xxde10:
	call	trimi		;trim leading 0s of iii
	mov	cx,intcnt
	mov	di,intp
	call	formbcd 	;convert to bcd spliced into fff bcd
	jnc	xxde20
	jmp	xx03		;..invalid or too big
xxde20:
	call	exponent	;develop exponent in p10
	jnc	xxde30
	jmp	badexp		;invalid exponent
xxde30:
	mov	ax,p10		;test exponent for proper range
	cmp	intcnt,0	;if no integer portion (eg all 0s)
	je	xxde35		;..no effect on magnitude
	add	ax,intcnt	;power of ten + number of digits less 1
	dec	ax
xxde35:
	sub	ax,decnt	;..less number of fractional digits
	call	testexp
	jnc	xxde40
	mov	ax,3		;exponent out of range
	jmp	xx05
;consider ii.fffffE2.  p10 would be plus 2, but accounting for
;fractional part .fffff, p10 after adjustment becomes -3.  So, if there is
;a carry in the adjustment, the sign flag for the exponent must be flipped.
xxde40:
	mov	ax,p10		;adjust exponent for fractional part
	sub	ax,decnt	;if adjustment flips exponent sign
	jnc	xxde50
	xor	flag,80h	;..mark flipped sign
xxde50:
	mov	p10,ax
	call	ten2x		;convert to fp
	xor	ax,ax		;good return
	jmp	a2fxit		;leave
asc2fp	endp

ten2x	proc	near
	fldl2t			;log2 of 10
	test	flag,80h	;if negative
	jz	ten2x10
	neg	p10		;..force exp positive
ten2x10:
	fimul	p10		;(lg2of10 * exp)
	call	two2x		;10**exponent
	fbld	bcd		;st = n 	st(1) = 10**exp
	test	flag,80h
	jz	ten2x20
	fdivr			;st(0) = st(0) / st(1) n / 10**exp
	jmp short ten2xit
ten2x20:
	fmul			;number * 10**exponent
ten2xit:
	call	put
	ret
ten2x	endp

;ax = signed exponent
testexp proc	near
	cmp	[bp].form,4
	jne	tstexp10
	cmp	ax,shortxm
	jl	tstexp20
	cmp	ax,shortxp
	jg	tstexp20
expok:
	clc
	ret
tstexp10:
	cmp	ax,longxm
	jl	tstexp20
	cmp	ax,longxp
	jg	tstexp20
	jmp	expok
tstexp20:
	stc
	ret
testexp endp

exponent proc	near
	mov	di,exp
	mov	cx,expcnt
	jcxz	exponer 	;no value!
	cmp byte ptr es:[di],'+'
	je	expon10
	cmp byte ptr es:[di],'-'
	jne	expon20
	or	flag,80h
expon10:
	inc	di
	dec	cx
	jcxz	exponer 	;no value!
expon20:
	xor	ax,ax
	xor	bx,bx
explp:
	mov	bl,es:[di]
	cmp	bl,'0'
	jb	exponer
	cmp	bl,'9'
	ja	exponer
	sub	bl,'0'
	mul	c10
	or	dx,dx
	jnz	exp2big
	add	ax,bx
	inc	di
	loop	explp
	test	flag,80h	;if exponent is negative
	jz	expon30
	neg	ax		;..make it so
expon30:
	mov	p10,ax
	xor	ax,ax		;clears carry flag
	ret
exponer:
	mov	ax,1		;invalid number
	stc
	ret
exp2big:
	mov	ax,3		;exponent out of range
	stc
	ret
exponent endp

;cx = length of string
;es:di -> string
formbcd proc	near
	xor	ax,ax
	mov	al,digcnt
	add	ax,cx
	cmp	ax,16		;max digits
	ja	toobig
	jcxz	fbcdok		;empty string, ok it's 0
	add	di,cx
	mov	si,bcdp 	;bcd template position
fbcdlp:
	dec	di
	call	testdig
	jc	fbcdnan
	or	[si],al
	test	digcnt,01h
	jz	fbcd10
	inc	si
fbcd10:
	inc	digcnt
	loop	fbcdlp
fbcdok:
	mov	bcdp,si 	;update template position
	clc
	ret
toobig:
	mov	ax,2
	stc
	ret
fbcdnan:
	mov	ax,1
	stc
	ret
formbcd endp

testdig proc	near
	mov	al,es:[di]	;pick up character
	cmp	al,'0'
	jb	tstdg10 	;invalid number
	cmp	al,'9'
	ja	tstdg10 	;also invalid
	sub	al,'0'
	test	digcnt,01h
	jz	tstdg05
	push	cx
	mov	cl,4
	shl	al,cl
	pop	cx
tstdg05:
	clc
	ret
tstdg10:
	stc
	ret
testdig endp

trimf	proc	near
	mov	cx,decnt	;fraction length
	jcxz	trimfxt 	;empty fraction! well, ok
	mov	di,decp
	add	di,cx
	dec	di		;->last fraction digit
	std			;scan backwards
	mov	al,'0'
	repe scasb
	cld
	je	trimfxt 	;fraction is all 0s!
	inc	cx		;adjust count for overshoot
trimfxt:
	mov	decnt,cx	;fraction count less trailing zeroes
	ret
trimf	endp

trimi	proc	near
	mov	cx,intcnt
	mov	di,intp
	jcxz	trimixt 	;empty string
	mov	al,'0'
	repe scasb
	je	trimixt 	;nothing but 0s
	dec	di
	inc	cx
trimixt:
	mov	intcnt,cx	;digit count sans leading 0s
	mov	intp,di 	;start of 1st significant digit
	ret
trimi	endp

put	proc	near
	les	di,dword ptr [bp].resltb
	cmp	[bp].form,8
	je	put10
	fstp dword ptr es:[di]
	jmp short put15
put10:
	fstp qword ptr es:[di]
put15:
	ret
put	endp

fp4	ends
	end
