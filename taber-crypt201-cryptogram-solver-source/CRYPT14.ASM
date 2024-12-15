	page	53,132
	.8087
; language statistic loader
	include frame.str
	include frame2.str
	include crypt14b.asm
	extrn	asc2int:far,freeadj:far,getmem:far,showmsg:far,asc2fp:far
crypt14 segment 'crya'
	assume	cs:crypt14,ds:crypt14b
	public	getmono
getmono proc	far
	push	ds
	mov	ax,crypt14b
	mov	ds,ax
	mov	bufl,0			;chars in buffer = 0
	mov	bufp,offset buff	;current position = start of buffer
	mov	flag,0
	mov	flagm,0
	les	di,dword ptr [bp].freehb
	mov	freeb,di
	mov	freea,es
	les	di,dword ptr [bp].sizeb
	mov	saveb,di
	mov	savea,es
	call	freeadj
	mov	ax,crypt14b
	mov	es,ax
;open the statistics file
	call	opendat
	jnc	gtmono10
	mov	si,offset err2		;file not found
	call	errmsg
	jmp	gtmonoxt
gtmono10:
	mov	dath,ax 		;save file handle
keylp:
	call	token			;get token
	jnc	gtmono20
;if end of file, check that we have everything
	cmp	ax,-1		;if file error
	jne	gtmonfin	;..already reported
	jmp	gtmonoxt
gtmonfin:
	test	flagm,01h	;If length not specified
	jnz	mnfin10
	mov	si,offset err19 ;..say using default English
finerr:
	call	errmsg
	jmp	gtmonoxt
mnfin10:
	test	flagm,02h	;If cipher alphabet not specified
	jnz	mnfin20
	mov	si,offset err20 ;..say using default English
	jmp	finerr
mnfin20:
	test	flagm,04h	;If plain alphabet not specified
	jnz	mnfin30
	mov	si,offset err21 ;..say using default English
	jmp	finerr
;move what we have
mnfin30:
	mov	cx,alfl 	;alphabet length
	les	di,dword ptr [bp].alfb
	mov	es:[di],cx
	add	di,2
	push	di
	push	ds
	lds	si,dword ptr cipherb	;set the cipher alphabet
	rep movsb
	pop	ds
	pop	di
	add	di,256			;set the plain alphabet
	mov	si,plainb
	mov	cx,alfl
	push	ds
	mov	ds,ciphera
	rep movsb
	pop	ds
;if mono statistics were specified, set them too
	test	flagm,08h
	jnz	mnfin40
	jmp	gtmonoxt
mnfin40:
	mov	cx,alfl
	shl	cx,1
	shl	cx,1
	les	di,dword ptr [bp].probb ;destination
	push	ds
	mov	si,monob		;source
	mov	ds,ciphera
	rep movsb
	pop	ds
	jmp	gtmonoxt

;check for a valid keyword
gtmono20:
	mov	bx,offset monotab	;keyword table
	mov	cx,(monotabz-monotab)/6 ;number of entries in table
chkmlp:
	mov	di,tokp 		;->this token
	mov	si,[bx] 		;->keyword
	mov	ax,[bx]+2		;length of keyword
	cmp	ax,tokl
	jne	gtmono30
	push	cx
	mov	cx,ax
	repe cmpsb
	pop	cx
	je	gtmono40
gtmono30:
	add	bx,6			;next table entry
	loop	chkmlp
	call	next			;skip unwanted tokens
	jc	gtmono32
	jmp	keylp			;get next token
gtmono32:
	jmp	gtmonfin		;..or quit
gtmono40:
	jmp	[bx+4]			;go to keyword routine
gtmonoxt:
	les	di,dword ptr freeb
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr saveb
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	call	closedat
	pop	ds
	ret

;alphabet length
alph:
	test	flagm,01h	;if alphabet length already specified
	jz	alph00
	mov	si,offset err3	;..tell user of invalid file
	call	errmsg
alfbad: 			;..then quit
	mov	ax,2
	stc
	jmp	gtmonoxt
alph00:
	or	flagm,01h	;alphabet length started
	call	token		;get next token, should be length
	jnc	alph10
	cmp	ax,-1		;if file error
	je	alfbad		;..it's already reported
alfbad2:
	mov	si,offset err4	;alphabet length missing
alfbadxt:
	call	errmsg
	jmp	alfbad
alfbad3:
	mov	si,offset err5
	jmp	alfbadxt
alph10:
	mov	cx,tokl 	;validate the length
	mov	di,tokp
alphlp:
	cmp byte ptr [di],'0'
	jb	alfbad2
	cmp byte ptr [di],'9'
	ja	alfbad2
	inc	di
	loop	alphlp
	mov	si,tokp 	;properly all digits
	mov	cx,tokl 	;..convert to binary
	call	asc2int
	jc	alfbad3 	;fixed point overflow
	cmp	ax,256		;no more than 256 chars allowed in alphabet
	ja	alfbad3 	;too big
	or	ax,ax
	jz	alfbad3
	mov	alfl,ax 	;ok length
;figure temp memory needs
	shl	ax,1		;twice that for cipher and plain alphabets
	mov	cx,ax
	shl	ax,1		;four times that for mono statistics
	add	cx,ax
	call	getmem
	jnc	alph20
	mov	si,offset err6	;insufficient memory
	jmp	alfbadxt
alph20:
	mov	cipherb,di	;anchor cipher alphabet holding buffer
	mov	ciphera,es
	add	di,alfl
	mov	plainb,di	;anchor plain alphabet holding buffer
	add	di,alfl
	mov	monob,di	;anchor mono stats holding buffer
	mov	ax,crypt14b
	mov	es,ax
	jmp	keylp

ciph:
	test	flagm,01h		;alphabet length must have been specd
	jnz	ciph10
	mov	si,offset err7
	jmp	alfbadxt		;..else invalid file
ciph10:
	test	flagm,02h		;cipher alphabet must not have been
	jz	ciph20
	mov	si,offset err8		;cipher alphabet duplicated
	jmp	alfbadxt
ciph20:
	or	flagm,02h
	call	token		;find the cipher alphabet
	jnc	ciph30
	mov	si,offset err9
	jmp	alfbadxt	;unexpected end of tokens
ciph30:
	mov	si,tokp
	cmp byte ptr [si],'0'   ;if numeric encoding
	jb	ciph40
	cmp byte ptr [si],'9'
	ja	ciph40
;placeholder for numeric encoding of cipher alphabet
	nop
	jmp	keylp
;alphabet is a character string
ciph40:
	mov	cx,tokl
	cmp	cx,alfl 	;string length must agree with alphabet length
	je	ciph50
	mov	si,offset err10 ;definition length disagrees with alf length
	jmp	alfbadxt
ciph50:
	push	es
	les	di,dword ptr cipherb
	rep movsb		;save the string for later
	pop	es
	jmp	keylp

plain:
	test	flagm,01h
	jnz	plain10
	mov	si,offset err11 ;plain alphabet out of order
	jmp	alfbadxt
plain10:
	test	flagm,04h
	jz	plain20
	mov	si,offset err12 ;plain alphabet respecified
	jmp	alfbadxt
plain20:
	or	flagm,04h	;flag plain alphabet specified
	call	token		;get the alphabet
	jnc	plain30
	mov	si,offset err13
	jmp	alfbadxt	;no alphabet
plain30:
	mov	si,tokp 	;check for numeric encoding of alphabet
	cmp byte ptr [si],'0'
	jb	plain40
	cmp byte ptr [si],'9'
	ja	plain40
;placeholder for numeric encoding of plain alphabet
	nop
	jmp	keylp
plain40:
	mov	cx,tokl 	;verify alphabet length
	cmp	cx,alfl
	je	plain50
	mov	si,offset err14
	jmp	alfbadxt
plain50:
	mov	di,plainb
	push	es
	mov	es,ciphera
	rep movsb		;save plain alphabet for later
	pop	es
	jmp	keylp

mstat:
	test	flagm,01h
	jnz	mstat10
	mov	si,offset err15 ;mono stats out of order
	jmp	alfbadxt
mstat10:
	test	flagm,08h
	jz	mstat20
	mov	si,offset err16 ;respecification of mono stats
	jmp	alfbadxt
mstat20:
	or	flagm,08h	;flag mono specified
	mov	cx,alfl
	mov	di,monob
	finit
mstatlp:
	call	token		;get the mono statistic
	jnc	mstat30
	mov	si,offset err17 ;unexpected end of file
	jmp	alfbadxt
mstat30:
	call	tofp		;convert to floating point
	cmp	ax,0		;if number is invalid
	je	mstat40
	mov	si,offset err18 ;invalid number
	jmp	alfbadxt	;..quit
mstat40:
	add	di,4		;next position for floating point
	loop	mstatlp
	jmp	keylp
getmono endp

;returns no carry	tokp -> start of token
;			tokl = length of token
;	 carry		ax = 0	means no more tokens, end of file
;			ax = -1 file error detected
token	proc	near
	push	cx
	push	di
tok00:
	test	flag,02h		;if end of file
	jz	tok05
	xor	ax,ax			;..no more tokens
	stc
	jmp	tokxt
tok05:
	cmp	bufl,0		;if there are bytes left in the buffer
	jne	tok20		;..look for a token
	call	read		;else replenish the buffer
	jnc	tok20
	mov	ax,-1		;file error
	jmp	tokxt
tok20:
	mov	di,bufp 	;begin where we left off
	mov	cx,bufl 	;remaining bytes in buffer
;skip to first non-blank
	mov	al,' '
skbl:
	jcxz	tok25
	rep scasb
	jne	tok30		;non-blank found
tok25:
	mov	bufl,cx 	;entire buffer was blank
	jmp	tok00		;..so get another buffer
tok30:
;but is this really non-white space?
	cmp byte ptr [di-1],' '
	jb	skbl		;control chars are white space
;genuine character found.  now, is it a comment?
	dec	di
	inc	cx
	mov	bufl,cx
	mov	bufp,di
	cmp byte ptr [di],'*'
	jne	tok40
	call	next		;get next line, skip comment
	jnc	tok00		;..next line found, seek start of token
	jmp short tokxt        ;no more tokens
;non-comment token start found
tok40:
	mov	bufl,cx 	;update buffer controls
	mov	bufp,di
	mov	tokp,di 	;mark start of token
;find end of token
tok45:
	mov	al,' '          ;scan for white space
	mov	di,bufp
	mov	cx,bufl
tok50:
	cmp	al,[di]
	jnb	tok60
	inc	di
	loop	tok50
;end of token may not be in this buffer
	test	flag,02h	;if end of file, this is end of token
	jnz	tok60		;eof = end of token
	call	read		;else get another buffer's worth
	mov	tokp,offset buff ;reset start of token
	jnc	tok45		;..and try again
	mov	ax,-1		;broken file
	jmp short tokxt 	;file error
tok60:
	mov	bufl,cx 	;new length
	mov	bufp,di 	;new scan start
	sub	di,tokp 	;length of token
	mov	tokl,di
	clc
tokxt:
	pop	di
	pop	cx
	ret
token	endp

;run out current line to end of line or end of file
next	proc	near
next00:
	test	flag,02h	;if not end of file
	jz	next05
	xor	ax,ax
	stc
	ret
next05:
	mov	al,0Dh		;seek end of line
	mov	cx,bufl
	jcxz	next10		;no end of line in this buffer's worth
	mov	di,bufp
	repne scasb
	jne	next10		;no end of line in this buffer's worth
	inc	cx		;adjust for end of line found
	dec	di
	mov	bufl,cx
	mov	bufp,di
	clc			;next line found
	ret
next10:
	mov	bufl,0		;nothing left over
	call	read		;get another buffer's worth
	mov	cx,bufl 	;reset scan length
	mov	di,bufp 	;reset scan start
	jnc	next00		;if no error, continue scan
	mov	ax,-1		;flag read error
	ret			;read error return
next	endp

read	proc	near
	mov	cx,bufl 		;shift leftovers to front of buffer
	mov	si,bufp 		;beginning of leftovers
	mov	di,offset buff		;destination of leftovers
	jcxz	read10			;..no leftover bytes
	rep movsb
read10:
	mov	bufp,offset buff	;reset start of buffer
	test	flag,01h		;if no more to read
	jz	read20
	or	flag,02h		;..flag end of file
	ret				;..and quit.
read20:
	mov	dx,di			;fill rest of buffer
	mov	cx,256
	sub	cx,bufl 		;length of rest of buffer
	mov	bx,dath
	mov	ah,3Fh
	int	21h
	jnc	read30
	mov	si,offset err1	;broken file message
	call	errmsg
	stc			;file error
	ret
read30:
	cmp	ax,cx
	je	read40		;not last read
	or	flag,01h	;flag no more to read
read40:
	add	bufl,ax 	;length of string in buffer
	clc
	ret
read	endp

opendat proc	near
	push	ds
	mov	ds,[bp].edseg
	mov	dx,[bp].langspec	;ds:dx->statistics filespec
	mov	ax,3D00h		;open for reading
	int	21h
	pop	ds
	ret
opendat endp

closedat proc	near
	mov	bx,dath
	mov	ah,3Eh
	int	21h
	ret
closedat endp

;tokp->string
;tokl = string length
;convert string to floating point
tofp	proc	near
	push	bp
	sub	sp,12
	mov	bp,sp		;set up frame for conversion
	mov	[bp].form,4	;short real
	mov	[bp].resltb,di	;offset of result
	mov	ax,ciphera
	mov	[bp].reslta,ax	;segment of result
	mov	[bp].strnga,ds	;segment of string
	mov	ax,tokp
	mov	[bp].strngb,ax	;offset of string
	mov	ax,tokl
	mov	[bp].strngl,ax	;length of string
	call	asc2fp		;convert
	add	sp,12
	pop	bp
	ret
tofp	endp

;ds:si->error message to display
errmsg	proc	near
	test	[bp].flagb,20h	;if no screen yet
	jz	errm10
	mov	dx,si		;..display with DOS
	inc	dx
	mov	ah,09h
	int	21h
;to prevent flashing message when screen is finally displayed, wait for
;user to enter a key
	mov	dx,offset msg1	;tell user to press any key
	mov	ah,09h
	int	21h
	mov	ah,0
	int	16h		;wait for input
	ret
errm10:
	push	di		;display error message on screen
	push	es
	mov	di,24*160+80	;where on screen
	call	showmsg
	pop	es
	pop	di
	ret
errmsg	endp

crypt14 ends
	end
