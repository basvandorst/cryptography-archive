	page	53,132
	include frame.str
	include motndx.str
	.8087
	include crypt11b.asm
crypt11 segment 'crya'
	assume	cs:crypt11,ds:crypt11b
	public	fit,chi_squ
;compute probability of fit of each word in list to cipher word
;si -> list ptrs in stack	primia:primib -> list of words buffer
;				wordia:wordib -> cipher word index
;				probia:probib -> probability buffer
fit	proc	far
	push	ds
	mov	ax,crypt11b	;data addressability
	mov	ds,ax
	les	di,dword ptr ss:[si+4]	;get word index
	xor	dx,dx
	mov	dl,es:[di].motrln	;length of a word
	mov	cx,es:[di].motpcnt	;for every word in list
	les	di,dword ptr ss:[si]	;word list ptr
;calculate fit
	finit		;initialize NDP
fitlp3:
	call far ptr chi_squ
;put the chi-squared value
	push	si
	lds	si,dword ptr ss:[si+8]	;get the destination
	fstp qword ptr [si]		 ;..and save probability
	pop	si
	add word ptr ss:[si+8],8	;next destination
fit40:
	add	di,dx
	loop	fitlp3		;do for all words
	mov word ptr ss:[si+8],0
	pop	ds
	ret
fit	endp

;es:di->word in list
;si->word index ptr
;compute chi-squared goodness of fit of this plaintext word to the
;cipher letter counts
chi_squ proc	far
	push	ds
	push	si
	push	cx
	push	di
	push	bx
	lds	si,ss:[si+4]	;->word index
	xor	cx,cx
	mov	cl,[si].motrln
	fldz			;sum	   sum chi-square fit
fitwlp:
	cmp	[si].motacnt,0	;if this is a repeated letter
	je	fitw20		;..skip it
fitw05:
	fild	[si].motacnt	;c[i],sum	cipher letter count
	fild	[bp].bilan	;N,c[i],sum	total letters
	call	getprob 	;get this plaintext letter probability in st
fitw10: 			;p[i],N,c[i],sum
	fmul			;expected,c[i],sum
	fsub	st(1),st	;denom,num,sum
	fxch			;num,denom,sum
	fmul	st,st(0)	;num^2,denom,sum
	fdivr			;num^2/denom,sum
	fadd			;sum
fitw20:
	add	si,2		;next crypt letter count
	inc	di		;next plaintext letter
	loop	fitwlp
	pop	bx
	pop	di
	pop	cx
	pop	si
	pop	ds
	ret
chi_squ endp

;on entry, es:di -> plaintext letter
;on exit, st(0) = probability of plaintext letter
getprob proc	near
	push	es
	push	di
	push	cx
	mov	al,es:[di]	;get plaintext letter
	les	di,dword ptr [bp].alfb
	mov	cx,es:[di]	;alphabet length
	add	di,258		;es:di->defined plaintext alphabet
	mov	bx,di
	repne scasb	;find the plaintext letter
	jne	getper	;..it isn't there, something wrong
	dec	di
	sub	di,bx	;index of plaintext letter
	shl	di,1	;..times 4
	shl	di,1	;..is index of plaintext letter probability
	mov	es,[bp].proba
	add	di,[bp].probb  ;es:di->probability
	fld dword ptr es:[di]
getpxt:
	pop	cx
	pop	di
	pop	es
	ret
getper: 		;plaintext letter undefined
	fldz		;..so it's probability is 0
	jmp	getpxt
getprob endp
crypt11 ends
	end
