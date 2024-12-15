	page	56,132
	.8087
	comment ~
ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ Linkage	Far procedure.						    บ
บ    entry	st(0)		Contains number to be converted 	    บ
บ		ax		Contains number of significant digits to    บ
บ				be developed, from 2 to 16.  If ax is	    บ
บ				invalid, 10 is forced.			    บ
บ		ss:bp -> intersegment address -> result_string in caller's  บ
บ				area, minimum length of 26 bytes.  May be   บ
บ				longer to suit caller's purpose.            บ
บ    exit	st(0)		Popped. 				    บ
บ		ax		Return code				    บ
บ				0 = ok, result_string contains ASCII string บ
บ				1 = unnormal or denormal, result_string set บ
บ				    to '0'                                  บ
บ				2 = NaN, not convertible. result_string set บ
บ				    to blank.				    บ
บ				3 = Not converted.  Either st(0) was beyond บ
บ				    range of FP2ASC, or 8087 stack over-    บ
บ				    flowed.				    บ
บ				4 = st(0) empty.			    บ
บ		result_string	length,ascii_string.  length is one byte    บ
บ				length of following ascii_string.  ascii_   บ
บ				string contains representation of number    บ
บ				or blank if number couldn't be represented. บ
บ Typical invocation							    บ
บ	sub	sp,4	;make stack frame				    บ
บ	mov	bp,sp	;frame pointer					    บ
บ	mov word ptr [bp],offset my_string ;my_string at least 26 bytes     บ
บ	mov	[bp+2],ds ;tell FP2ASC where my_string is		    บ
บ	.								    บ
บ	.								    บ
บ	fld	chi_squared	;number to be converted 		    บ
บ	call	fp2asc							    บ
บ	cmp	ax,0		;did it convert ok?			    บ
บ	jne	errors							    บ
บ	;ascii string is in my_string, first byte of which is length.	    บ
บ Requirements	8087 installed.  Available 8087 stack depth of 4.	    บ
ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
	~
;layout of string in caller's area in which the ASCII representation
;of a floating point number is to be developed.  The string length shown
;is a minimum requirement; it may be longer to suit the caller's needs.
print_string struc
string_length	db	?		;length of the following string
string		db	25 dup(?)	;ASCII representation of number
print_string ends
fp2b	segment 'cryb'
;rounding control
nearest dw	03FFh	;round to nearest
floor	dw	07FFh	;round to - infinity
ceiling dw	0BFFh	;round to + infinity
chop	dw	0FFFh	;round to 0
oldcw	dw	0	;save 8087 oldcw
sw	dw	0	;8087 status word
;FXAM condition code branch table
fptype	dw	unnormplus	;0   +unnormal
	dw	nan		;1   +nan
	dw	unnormmin	;2   -unnormal
	dw	nan		;3   -nan
	dw	itsplus 	;4   +number
	dw	infplus 	;5   +infinity
	dw	itsminus	;6   -number
	dw	infminus	;7   -infinity
	dw	show0		;8   +0
	dw	empty		;9   stack top is empty
	dw	show0		;10  -0
	dw	empty		;11  stack top is empty
	dw	denormplus	;12  +denormalized number
	dw	empty		;13  stack top is empty
	dw	denormmin	;14  -denormalized number
	dw	empty		;15  stack top is empty
ebase2	dw	0	;value of exponent for E in binary
ebase10 dt	0	;value of exponent for E in bcd
bcd_result dt	0	;converted number in bcd
quadpp	dw	10	;print precision
c10	dw	10	;constant 10
;powers of ten table for adjustment
;print precision 2   3	 4   5	 6   7	 8   9
pptable dq	1E2,1E3,1E4,1E5,1E6,1E7,1E8,1E9
;		 10   11   12	13   14   15   16
	dq	1E10,1E11,1E12,1E13,1E14,1E15,1E16
digits	dw	0	;number of digits to be returned
edigits dw	0	;number of exponent digits
sign	db	' '     ;sign of number being converted
retcode dw	0	;return code
unpackd db	16 dup('0') ;work area to convert bcd to ascii
fp2b	ends
	page
fp2	segment 'crya'
	public	fp2asc,two2x
; ax = desired print precision.  st(0) = x, the number to convert
;      print precision should be between 2 and 16, and if out of range,
;      a print precision of 10 is forced.
; on return, [ss:bp] -> print string; x is popped; ax contains condition code.
fp2asc	proc	far
	assume cs:fp2,ds:fp2b
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	ds
	push	es
	pushf
	mov	dx,fp2b 	;segment addressability
	mov	ds,dx
	cld
	mov	retcode,0	;assume good results
	fstcw	oldcw		;preserve caller's control bits
	fldcw	nearest 	;..and set my own
	mov	quadpp,10	;default print precision
	cmp	ax,1		;check desired pp for reason
	jng	f2asc00 	;should be between 2 and
	cmp	ax,16		;..16 or it isn't reasonable
	jg	f2asc00
	mov	quadpp,ax	;set pp to caller's precision
	les	di,[bp] 	;point to caller's return string
;determine if x is representable
f2asc00:
	call	checkst 	;get type of fp value in bx
	shl	bx,1
	jmp	fptype[bx]	;jump to appropriate routine
	page
; x = + or - 0
show0:
	fstp	st(0)		;pop x
show0b:
	mov	ax,3020h	;display blank,0
	mov	es:string_length.[di],2  ;length of string
	inc	di
	stosw
	jmp	xit
;interpret denormals as zero but set return code to flag them
unnormplus:
denormplus:
unnormmin:
denormmin:
	mov	retcode,1
	jmp	show0
;Not A Number, indefinite, infinity
nan:
infplus:
infminus:
	mov	retcode,2
nan02:
	fstp	st(0)		;pop stack
;return a blank
nan04:
	mov	ax,2001h	;length 1, blank
	stosw
	jmp	xit
;st contained a valid number but couldn't be converted due to range
;or stack overflow
out_of_range:
	mov	retcode,3	;stack may have overflowed
	jmp	nan04
;Oh how the caller lied.  There's nothing in the stack top
empty:
	mov	retcode,4
	jmp	nan04
	page
;x is a number that we can convert if it isn't out of range and if there is
;enough 8087 stack.
;x is plus
itsplus:
	mov	sign,' '
	jmp short number
;x is minus
itsminus:
	mov	sign,'-'
number:
	fabs		;since we have the sign, work with positive x
;determine magnitude of number in base 10 to scale it for bcd conversion
;magnitude in base 10 approx [exp(2) x] * [log(10) 2]
	fld	st(0)	;x,x
	fxtract 	;mantissa,exp,x
	fstp	st(0)	;exp,x
	fldlg2		;log10 2,exp,x
	fmul		;mag10-,x
	fldcw	floor	;		take floor of magnitude
	fistp	ebase2	;x
	fldcw	nearest
	inc	ebase2	;		..and bump by 1
	fild	ebase2	;mag,x
	fchs		;-mag,x
	call	ten_to_x ;scale,x
	fmul		;x supposedly scaled to fraction of .fff
;ensure that x is a fraction between 0.1 and 0.9999..
;by trick of taking the floor of approximate magnitude, we have
;guaranteed that scaled x >= 0.1.  Therefore, we need only test for
;x >= 1.0, and adjust if it is.
	fld1		;1,scaled x
	fcomp		;scaled x	condition code set
	fstsw	sw
	fwait
	test	sw,4100h	;if c3 and c0 are off
	jz	scale_ok	;..then 1 > scaled x
	fidiv	c10		;else reduce x to fraction
	inc	ebase2		;..and magnitude was 1 too little
scale_ok:
	mov	bx,quadpp	;now scale fraction to requested
	sub	bx,2		;..number of integer digits.
	mov	cl,3		;index into power of ten table
	shl	bx,cl		;..from 1E2 to 1E16, our allowed
	fmul	pptable[bx]	;..display range.
;note that we round to nearest.  We could use the caller's rounding
;instead, and FRNDINT, if there is a reason to.
	fbstp	bcd_result
;it is possible that the above computations failed because of results
;too large or small for the 8087, or stack overflow.  If so, bcd_result
;will be indefinite.
	fwait
	cmp byte ptr bcd_result+9,0FFh
	jne	bcd_ok		;if invalid, exit via out of range
	jmp	out_of_range
bcd_ok:
	fild	ebase2		;we're ok, make magnitude into bcd for exp10
	fbstp	ebase10
	page
;Convert the bcd to ascii
	mov	si,quadpp	;index to 1st significant bcd byte
	dec	si
	shr	si,1
	mov	di,offset unpackd ;destination for ascii string
	mov	ax,fp2b
	mov	es,ax
	mov	cx,0F04H	;set mask and shift amount
;find bcd nibble to start with
	mov	ah,byte ptr bcd_result[si]	;get bcd byte
	mov	al,ah
	test byte ptr quadpp,01h ;if odd number of digits
	jnz	enter_2nd	;..high nibble is 0
	shr	al,cl		;else high nibble is significant
	jmp short enter_1st
ascii_lp:
	mov	ah,byte ptr bcd_result[si]
	mov	al,ah
	shr	al,cl
enter_1st:
	add	al,'0'          ;make 1st digit an ascii character
	stosb
	mov	al,ah
	and	al,ch
enter_2nd:
	add	al,'0'          ;make 2nd digit an ascii character
	stosb
	dec	si
	jns	ascii_lp
;discount the trailing 0s not to be displayed in some cases
	mov	cx,quadpp	;there is at most pp-1 non-zero digits
	mov	di,cx		;index to last digit
	dec	di
	add	di,offset unpackd
	mov	al,'0'
	std			;scan backwards
	repe  scasb		;..to first non-zero character
	inc	cx		;fix up the count
	cld			;restore direction control
	mov	digits,cx	;sig digits without trailing zeroes
	les	di,[bp] 	;restore ptr to caller's return string
;zero caller's string length
	mov	bx,di		;point to result length byte
	mov	al,0
	stosb			;..and set to 0
;return the sign
	mov	al,sign 	;display the sign
	stosb
	inc byte ptr es:[bx]	;count the sign
	mov	si,offset unpackd ;where they come from
;choose between scientific notation and decimal for display
; if 0 < ebase2 <= quadpp then
;      display as decimal
; if ebase2 + digits <= quadpp then
;      display as decimal
; else
;      display as scientific
	mov	ax,ebase2	;get magnitude
	cmp	ax,0		;if magnitude = 0
	je	f2asc10 	;..display as fraction of pp digits
	jl	f2asc15 	;if magnitude < 0 check for decimal fract
	cmp	ax,quadpp	;if magnitude > pp
	jg	enote		;..display in E notation
;display in decimal notation in form ddd.fff or dddd
	mov	cx,ebase2	;return integer part ddd
	add	es:[bx],cl
	rep movsb
;return fractional part .fff if any
	mov	cx,digits	;get number of fractional digits
	sub	cx,ebase2
	jng	mixedxt 	;there aren't any, it was an integer
;there remain cx fractional digits
	mov	al,'.'          ;display the decimal point
	stosb
	inc byte ptr es:[bx]
	add	es:[bx],cl	;account for fractional digits
	rep movsb
mixedxt:
	jmp	xit		;..then return
; x is a fraction of form 0.fxxx where f is not a zero
f2asc10:
	call	leading_0	;return leading 0.
	mov	cx,digits	;number of digits to return
	add	es:[bx],cl	;adjust string length
	rep movsb		;return the digits
	jmp	xit		;return
;determine if fraction can be displayed as a decimal of form .0--0f--f
f2asc15:
	neg	ax		;if number of 0's
	mov	cx,ax
	add	ax,digits	;..plus number of digits
	cmp	ax,quadpp	;..is > pp then display fraction
	jg	enote		;..in scientific notation.
	call	leading_0	;else display in decimal notation
	add	es:[bx],cl	;number of 0s after decimal point
	mov	al,'0'          ;supply leading 0s
	rep stosb
	mov	cx,digits	;number of significant digits
	add	es:[bx],cl
	rep movsb
	jmp	xit
;display in scientific notation
;determine number of exponent digits
enote:
	mov	ax,ebase2	;note: ebase2 can't be 0, which would
	cmp	ax,0		;always be displayed in decimal form
	jg	enote02 	;get absolute value of magnitude
	neg	ax
enote02:
	mov	edigits,1	;assume one exponent digit
	cmp	ax,10
	jl	enote08
	mov	edigits,2	;assume two exponent digits
	cmp	ax,100
	jl	enote08
	mov	edigits,3	;assume three exponent digits
	cmp	ax,1000
	jl	enote08
	mov	edigits,4	;must be four exponent digits
;format leading 0.
enote08:
	call	leading_0	;supply 0 and decimal point
;move the digits to the return
	mov	cx,digits
	add	es:[bx],cl	;account for digits
	rep movsb
;format E for exponent and the sign of the exponent
	mov	al,'E'          ;put an E into the return
	stosb
	inc byte ptr es:[bx]	;count the E
;format exponent sign
	mov	al,'-'
	cmp byte ptr ebase10+9,0
	je	enote12
	stosb			;put - into the return
	inc byte ptr es:[bx]
;format exponent digits
enote12:
	mov	dx,edigits
	mov	ax,dx
	dec	ax
	shr	ax,1
	mov	si,offset ebase10
	add	si,ax		;-> leading exponent bcd byte
	mov	al,byte ptr [si]
	test	al,0F0h 	;suppress leading zero
	jnz	enote25
	add	al,'0'
	stosb			;put exponent digit into return
	inc byte ptr es:[bx]
	dec	dx
	jmp short enote30
enote25:
	mov	ah,al
	mov	cx,4
	shr	al,cl
	and	ah,0Fh
	add	ax,3030h
	stosb			;put one exponent digit into return
	inc byte ptr es:[bx]
	dec	dx		;and if there are more to go
	jz	xit
	mov	al,ah
	stosb			;put another into the return
	inc byte ptr es:[bx]
	dec	dx
	jz	xit
enote30:
	dec	si
	mov	al,byte ptr [si]
	cmp	dx,0
	jg	enote25
xit:
	fldcw	oldcw	;restore caller's 8087 control word
	mov	ax,retcode
	popf
	pop	es
	pop	ds
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	ret
fp2asc	endp
;return in bx a code for type of number in stack top
checkst proc	near
	fxam		;what kind of a number is this?
	fstsw	sw
	fwait		;let the 8087 finish before examining
	mov	bx,sw
	mov	bl,bh	;we're interested only in 4 condition bits
	and	bx,4007h	;all bits off but these 4
	shr	bh,1
	shr	bh,1
	shr	bh,1
	or	bl,bh	;now have in bl  0 0 0 0 c3 c2 c1 c0
	xor	bh,bh
	ret
checkst endp
; far call interface to two_to_x
two2x	proc	far
	push	ds
	push	ax
	mov	ax,fp2b 	;segment addressability
	mov	ds,ax
	fstcw	oldcw		;preserve caller's control bits
	call	two_to_x	;2**x
	fldcw	oldcw		;restore caller's control bits
	pop	ax
	pop	ds
	ret
two2x	endp
; raise 10 to the x power.  x is in st0
ten_to_x proc	near
	fldl2t			;log2 10,x
	fmul			;X*LOG2(10) = E
	call	two_to_x	;perform 2**(x*log2 10)
	ret
ten_to_x endp
; raise 2 to the x power.  x is in st0
two_to_x proc	near
;break x apart into fraction and integer to use fscale for integer
;and f2xm1 for fraction <= 0.5
	fld1			;1,E
	fchs			;-1,E
	fld	st(1)		;E,-1,E
	fldcw	floor
	frndint 		;int(E)=i,-1,E
	fldcw	nearest
	fxch	st(2)		;	E	;      -1	;	I
	fsub	st(0),st(2)	; E - I = Fract ;      -1	;	I
;note, next step guarantees fraction < 0.5
	fscale			; F*2**-1 = F/2 ;      -1	;	I
	f2xm1			;(2**F/2)-1	;      -1	;	I
	fsubrp	st(1),st(0)	; 2**F/2	;	I	;	?
	fmul	st,st(0)	; 2**F,I
	fscale			;(2**F)*(2**I) = 2**x ; I	;	?
	fstp	st(1)		;2**x		;	?	;	?
	ret
two_to_x endp
;supply leading 0 and decimal point
leading_0 proc	near
	mov	al,'0'          ;supply leading 0.
	mov	ah,'.'
	stosw
	inc byte ptr es:[bx]	;count leading 0
	inc byte ptr es:[bx]	;count decimal point
	ret
leading_0 endp
fp2	ends
	end
