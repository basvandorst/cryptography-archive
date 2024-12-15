	page	53,132
	.8087
;this is an adaptation of Algorithm 299 in the Collected Algorithms of
;the CACM.  This adaptation uses PNORM, borrowed from FNN in CHIVAL
;routine from BASIC-Pack Statistics Programs, Dennie Van Tassel,
;Prentice-Hall (paperback).  Most likely, FNN, hence PNORM does not
;give as much accuracy as Algorithm 209 of the Collected Algorithms.
;Van Tassel comments that his routine gives about 5 decimal places of
;accuracy.  15-16 is possible with an appropriate PNORM, such as 209.
	include cryptgb.asm
	extrn	two2x:far
cryptg	segment 'crya'
	assume	cs:cryptg,ds:cryptgb
	public	chi_prob
;on entry st(0) = chisquared variable x
;	  ax = degrees of freedom
;on exit carry = 0 means st(0) = prob(x)
;		 1 means invalid
chi_prob proc	far
	push	ds
	push	es
	push	bx
	push	ax
	push	cx
	mov	bx,cryptgb
	mov	ds,bx		;addressability
	mov	es,bx
	mov	flag1,0
;special case of df=1
	cmp	ax,1
	jne	chip10
	fsqrt		;prob(chi)=2*-x^.5
	fchs		;-x^.5
	call	pnorm		;normal distribution probability
	fild	c2	;2,p(s)
	fmul		;p(chi)
	clc
chipxt:
	pop	cx
	pop	ax
	pop	bx
	pop	es
	pop	ds
	ret
chip10:
	fild	m1	;-1,x
	fld	st(1)	;x,-1,x
	fscale		;a=x/2,-1,x
;special case of df=2
	cmp	ax,2
	jne	chip20
	fmul		;-a,x
	fstp	st(1)	;-a
	fldl2e		;lg e,-a
	fmul		;exp
	call	two2x	;p(chi)=exp(-x/2)
	clc
	jmp	chipxt
chip20: 		;a,-1,x
	fst	halfx	;a,-1,x 		save a for later
	fmul		;-a,x
	fldl2e
	fmul
	call	two2x	;y=exp(-a),x		compute y, used for even
	fst	yexp	;y,x		save y for later
	fstp	s	;x		assume s=y
;test for x too big (would cause underflow)
	ficom	c1416
	fstsw	status
	fwait
	test byte ptr status+1,41h
	jz	big	;x > 1416
	test byte ptr status+1,01h
	jz	little	;x = 1416
	test byte ptr status+1,40h
	jz	little	;x < 1416
;bad x
	stc		;bad return code
	jmp	chipxt
big:
	or	flag1,01h	;flag big x
little: 		;x    s=y or 2*norm(-sqrt(x)), decide
	mov	cx,ax		;prepare loop control = (df-1)/2
	dec	cx
	shr	cx,1		;cx=stop (x in algorithm)
	test	al,01h
	jz	chip40		;s=y
	or	flag1,02h		;flag odd degrees of freedom
;degrees of freedom is odd, s=2*norm(-sqrt(x))
	fsqrt
	fchs		;-x**.5
	call	pnorm	;pnorm(-x**.5)
	fild	c2	;2,pnorm
	fmul		;s
	fstp	s
	fld	half	;z=.5		loop starting value for odd df
	jmp short chip50
chip40: 		;x
	fstp	st(0)
	fld1		;z=1		loop starting value for even df
chip50: 		;z=1  or  z=.5		even,odd
	test	flag1,01h	;if big x
	jnz	bigx		;..do big x recurrence
;do the little x recurrence
	fldz		;c=0,z
	fld1		;e,c,z		assume df is even, e=1
	test	flag1,02h
	jz	chip70
;compute e for odd degrees of freedom e=(1/sqrt(pi))/sqrt(a)
	fstp	st(0)		;c,z		throw away wrong e
	fld	invsqrpi	;con,c,z
	fld	halfx		;a,con,c,z
	fsqrt			;sqrt(a),con,c,z
	fdiv			;e,c,z
chip70: 			;0 1 2 3 4
	fld	halfx		;a,e,c,z
	fdiv	st,st(3)      ;a/z,e,c,z
	fmul			;e,c,z	      new e=e*a/z
	fadd	st(1),st			;new c=c+e
	fld1			;1,e,c,z
	faddp	st(3),st	;e,c,z	      new z
	loop	chip70
	fstp	st(0)		;c,z
	fstp	st(1)		;c
	fmul	yexp		;y*c
	fadd	s		;prob(chi)
	clc
	jmp	chipxt
bigx:				;z
	fldln2			;ln 2,z 	compute c=ln(a)
	fld	halfx		;a,ln2,z
	fyl2x			;c,z
	fstp	bigc		;z
	fld	s		;s,z
	fldz			;e,s,z		assume e=0 for even df
	test	flag1,02h
	jz	chip80		;if even e=0
	fstp	st(0)		;s,z
	fld	lnsqrpi 	;else e = ln(sqrt(pi))
chip80: 			;e,s,z
;compute e = ln(z)+e			 ln z = (lg2 z)*(ln 2)
	fldln2			;ln2,e,s,z
	fld	st(3)		;z,ln2,e,s,z
	fyl2x			;t,e,s,z	t=ln(z)
	fadd			;e,s,z		  new e = ln(z)+e
;compute s = exp(c*z-a-e)
	fld	bigc		;c,e,s,z	  replicate c
	fmul	st,st(3)	;t,e,s,z	  c*z
	fsub	halfx		;t,e,s,z	  c*z-a
	fsub	st,st(1)	;t,e,s,z	t=c*z-a-e
;e**x = 2**(x*lg2 e)
	fldl2e			;lg e,t,e,s,z
	fmul			;t,e,s,z	(lg e)*(c*z-a-e)
	call	two2x	;exp(c*z-a-e),e,s,z
	faddp	st(2),st	;e,s,z		  new s
;next z
	fld1			;1,e,s,z
	faddp	st(3),st	;e,s,z		z=z+1
	loop	chip80
	fstp	st(0)		;s,z
	fstp	st(1)		;s
	clc
	jmp	chipxt
chi_prob endp

;Taken from FNN in Van Tassle's CHIVAL algorithm.  Computes probability of
;x for a normal distribution.  Supposed accurate to about 5 places.
;on entry, x is in st(0)
pnorm	proc	near
	push	cx
	ftst		;determine sign of x
	fstsw	status
	fabs		;z=absolute value of x
	mov	cx,5
	mov	bx,8
	fld	coef	;coef,z
;begin polynomial approximation
pnormlp:
	fmul	st,st(1)	;z*.5383E-5,z
	fadd	coef[bx]       ;nextcon+p2,z
	add	bx,8
	loop	pnormlp
	fmul		;z*prev
	fld1		;1,z*prev
	fadd		;z*prev+1
	fild	m16	;-16,zetc
	fxch		;zetc,-16
	fyl2x		;-16*lg zetc	lg means log base 2
	call	two2x	;zetc^-16
	fild	m1	;-1,zetc^-16
	fxch		;p3**-16,-1
	fscale		;(p3**-16)/2,-1
	fstp	st(1)	;t
	fld	half	;0.5,t
	fld	st(0)	;0.5,0.5,t
	fsub	st,st(2) ;p=0.5-t,0.5,t
	fstp	st(2)	;.5,p
	test byte ptr status+1,41h	;if x was positive
	jz	pnorm10
	test byte ptr status+1,01h
	jz	pnorm10 		;..pnorm = 0.5+p
	fsubr				;else pnorm = 0.5-p
	pop	cx
	ret
pnorm10:
	fadd
	pop	cx
	ret
pnorm	endp
cryptg	ends
	end
