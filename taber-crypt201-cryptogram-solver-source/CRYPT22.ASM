	page	56,132
	include frame.str
	include iotaprm.str
	include permprm.str
	include xfrmprm.str
	include crypt22b.asm
	extrn	savefree:far,getmem:far
crypt22 segment 'crya'
	assume	cs:crypt22,ds:crypt22b
	public	factor,i_col,i_row,i_diag,permf,i_spiral,i_zcol,i_zrow,i_zdiag
	public	i_revrow,i_revcol,i_tran,i_atran,permg
factor	proc	far
	push	bx
	push	cx
	push	dx
	push	si
	push	ds
	mov	dx,crypt22b
	mov	ds,dx		;addressability
	mov	n,0
	mov	si,offset saveb ;save memory in case of failure
	call	savefree
	mov	divisor,2	;assume even n
	mov	bx,ax		;save dividend
fact08:
	test	bl,01h		;if n is odd
	jz	fact10
	inc	divisor 	;..skip even divisors
fact10:
	xor	dx,dx
	mov	ax,bx		;restore n
	div	divisor 	;trial division
	or	dx,dx		;if no remainder
	jnz	fact20
	inc	n		;count the factor
	mov	cx,4		;need four bytes to save the factors
	call	getmem
	jc	facterr
;quotient, divisor is preferred order (bigger, smaller)
	mov	es:[di],ax	;save the quotient
	mov	dx,divisor
	mov	es:[di+2],dx	;..and the divisor
fact20:
	inc	divisor 	;next divisor
	cmp	ax,divisor	;if quotient <= divisor
	ja	fact08		;..all divisors found
fact30:
	les	di,dword ptr factb	;->factor pairs if any
	mov	ax,n			;number of factor pairs
	clc
factxit:
	pop	ds
	pop	si
	pop	dx
	pop	cx
	pop	bx
	ret
facterr:
;memory failure, bail out
	les	di,dword ptr saveb	;restore all memory
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	les	di,dword ptr factb
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	stc			;memory error
	jmp	factxit
factor	endp

;column order index generator
i_col	proc	far
	push	bp
	sub	sp,2
	mov	bp,sp		;stack frame mapped by iotaprm
	push	es
	push	cx
	push	di
	push	dx
	les	di,[bp].ndxp	;index destination
	cmp	[bp].dir,0	;if inverse
	je	icol10
	mov	ax,[bp].row	;..store indexes backwards
	mul	[bp].col
	shl	ax,1
	sub	ax,2
	add	di,ax
	std
icol10:
	xor	dx,dx		;index
	mov	cx,[bp].row	;for all rows
icolp1:
	push	cx
	mov	cx,[bp].col	;..for all columns
	mov	ax,dx
icolp2:
	stosw
	add	ax,[bp].row
	loop	icolp2
	pop	cx
	inc	dx
	loop	icolp1
	cld
	pop	dx
	pop	di
	pop	cx
	pop	es
	add	sp,2
	pop	bp
	ret
i_col	endp

;row order index generator
i_row	proc	far
	push	bp
	sub	sp,2
	mov	bp,sp		;stack frame mapped by iotaprm
	push	es
	push	cx
	push	di
	push	dx
	mov	ax,[bp].row	;number of indexes
	mul	[bp].col
	mov	cx,ax
	les	di,[bp].ndxp	;index destination
	cmp	[bp].dir,0	;if inverse
	je	irow10
	shl	ax,1		;..store indexes backwards
	sub	ax,2
	add	di,ax
	std
irow10:
	xor	ax,ax		;i = 0
irowlp:
	stosw
	inc	ax
	loop	irowlp		;for i=0, i<n, i++
	cld
	pop	dx
	pop	di
	pop	cx
	pop	es
	add	sp,2
	pop	bp
	ret
i_row	endp

;diagonal order index generator
i_diag	proc	far
	push	bp
	sub	sp,2
	mov	bp,sp		;stack frame mapped by iotaprm
	push	es
	push	cx
	push	di
	push	dx
	push	bx
	push	si
	push	ds
	les	di,[bp].ndxp	;es:di->index destination
	mov	ax,[bp].row
	mul	[bp].col
	push	ax		;save number of indexes for end
	shl	ax,1
	add	di,ax		;es:di->work area
	push	es		;save work area ptr
	push	di		;..offset
	cmp	[bp].dir,0	;if inverse diagonal
	je	idiag10
	sub	ax,2		;..fill indexes backwards
	add	di,ax		;es:di-> end of work area
	std
idiag10:
	mov	cx,[bp].row	;diagonals = rows + columns -1
	add	cx,[bp].col
	dec	cx
	mov	bx,1		;diagonal length
idiagl1:
	xor	si,si		;column coordinate
	push	cx
	mov	cx,bx
idiagl2:
	cmp	cx,[bp].row	;if this row is out of range
	jbe	idiag20
	mov	si,cx		;..generate next row and column
	sub	si,[bp].row
	sub	cx,si
idiag20:
	cmp	si,[bp].col	;or if this col is out of range
	jae	idiag30 	;..generate next diagonal
;row and column in range.  translate to vector index
	mov	ax,cx
	dec	ax
	mul	[bp].col
	add	ax,si
	stosw
	inc	si		;next column
	loop	idiagl2
idiag30:
	pop	cx
	inc	bx		;next diagonal
	loop	idiagl1
	cld
	pop	si
	pop	ds		;ds:si->indexes in work area
	pop	cx		;number of indexes
	les	di,[bp].ndxp	;destination
	xor	ax,ax
idiagl3:
	mov	bx,[si] 	;pick up index
	shl	bx,1
	mov	es:[di][bx],ax
	add	si,2
	inc	ax
	loop	idiagl3
	pop	ds
	pop	si
	pop	bx
	pop	dx
	pop	di
	pop	cx
	pop	es
	add	sp,2
	pop	bp
	ret
i_diag	endp

;generate spiral order indexes
i_spiral proc	far
	push	bp
	sub	sp,2
	mov	bp,sp		;stack frame mapped by iotaprm
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	es
	push	ds
	xor	ax,ax		;assume forward direction
	mov	[bp].bump,1	;forward increment
	cmp	[bp].dir,0	;if going backwards
	je	ispir05
	mov	ax,[bp].row
	mul	[bp].col
	dec	ax		;backwards index generator
	mov	[bp].bump,-1	;decrement
ispir05:
	mov	bx,[bp].col	;number of columns
	mov	si,bx		;column length - 2
	dec	si
	shl	si,1
	mov	dx,[bp].row	;number of rows - 1
	les	di,[bp].ndxp	;es:di->index destination
ispir10:
	mov	cx,bx
	jcxz	ispir20
ispirl1:
	stosw			;next column position, this row
	add	ax,[bp].bump	;next index
	loop	ispirl1 	;..for column times
	dec	dx		;row - 1
	mov	cx,dx
	jcxz	ispir20
ispirl2:
	add	di,si		;next row position, this column
	stosw
	add	ax,[bp].bump	;next index
	loop	ispirl2 	;..for row times
	dec	bx		;column - 1
	sub	di,4		;next index position
	std			;go backwards
	mov	cx,bx
	jcxz	ispir20
ispirl3:
	stosw			;next index
	add	ax,[bp].bump
	loop	ispirl3 	;..for column times
	dec	dx		;row - 1
	mov	cx,dx
	jcxz	ispir20
ispirl4:
	sub	di,si		;previous row, this column
	stosw
	add	ax,[bp].bump
	loop	ispirl4
	add	di,4
	dec	bx		;column - 1
	cld
	jmp	ispir10
ispir20:
	cld
	pop	ds
	pop	es
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	add	sp,2
	pop	bp
	ret
i_spiral endp

;generate zigzag column order indexes
i_zcol	proc	far
	push	bp
	sub	sp,2
	mov	bp,sp		;stack frame mapped by iotaprm
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	es
	les	di,[bp].ndxp	;index destination
	mov	[bp].bump,1	;assume forward direction
	xor	ax,ax		;iota generator
	mov	cx,[bp].col	;outer loop control is number of columns
	mov	si,[bp].row
	cmp	[bp].dir,0
	je	izcol10
	mov	ax,cx
	mul	[bp].row
	dec	ax
	mov	[bp].bump,-1
	neg	si
izcol10:
	xor	dx,dx
izcolp1:
	push	cx
	mov	cx,[bp].row
	push	dx
izcolp2:
	mov	bx,dx
	shl	bx,1
	mov	es:[di][bx],ax
	add	ax,[bp].bump	;next index
	add	dx,[bp].col	;next row
	loop	izcolp2
	pop	dx
	inc	dx		;next column
	add	ax,si		;next iota start
	neg	[bp].bump	;flip increment/decrement
	add	ax,[bp].bump	;account for overshoot
	pop	cx
	loop	izcolp1
	pop	es
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	add	sp,2
	pop	bp
	ret
i_zcol	endp

;generate zigzag row order indexes
i_zrow	proc	far
	push	bp
	sub	sp,2
	mov	bp,sp		;stack frame mapped by iotaprm
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	es
	les	di,[bp].ndxp	;index destination
	mov	[bp].bump,1	;assume forward direction
	xor	ax,ax		;iota generator
	mov	cx,[bp].row	;outer loop control is number of rows
	mov	si,[bp].col
	cmp	[bp].dir,0
	je	izrow10
	mov	ax,cx
	mul	[bp].col
	dec	ax
	mov	[bp].bump,-1
	neg	si
izrow10:
	xor	dx,dx
izrowl1:
	push	cx
	mov	cx,[bp].col
izrowl2:
	mov	bx,dx
	shl	bx,1
	mov	es:[di][bx],ax
	add	ax,[bp].bump		;next index
	inc	dx			;next position
	loop	izrowl2 		;for all columns this row
	add	ax,si			;next starting index
	neg	[bp].bump
	add	ax,[bp].bump		;account for overshoot
	pop	cx
	loop	izrowl1 		;for all rows
	pop	es
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	add	sp,2
	pop	bp
	ret
i_zrow	endp

;generate zigzag diagonal order indexes
i_zdiag proc	far
	push	bp
	sub	sp,2
	mov	bp,sp		;stack frame mapped by iotaprm
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	es
	push	ds
	les	di,[bp].ndxp	;es:di->index destination
	mov	ax,[bp].row
	mul	[bp].col
	push	ax		;save number of indexes for end
	shl	ax,1
	add	di,ax		;es:di->work area
	push	es		;save work area ptr
	push	di		;..offset
	mov	cx,[bp].row	;diagonals = rows + columns -1
	add	cx,[bp].col
	dec	cx
	xor	bx,bx		;row coordinate
	xor	si,si		;column coordinate
	mov	[bp].bump,1	;diagonal length
izdiaglp:
	push	cx
	mov	cx,[bp].bump
izdiagl2:
	test	[bp].bump,01h	;if diagonal length is odd
	jnz	izdiag05	;..check row to row, col to col
	xchg	bx,si		;..else check row to col, col to row
izdiag05:
	cmp	bx,[bp].row	;if row is out of bounds
	jnb	izdiag22
izdiag10:
	cmp	si,[bp].col	;if column is out of bounds
	jnb	izdiag22
izdiag20:
	mov	ax,bx		;compute vector index of row and column
	mul	[bp].col	;row * column_length + column
	add	ax,si
	stosw
izdiag22:
	test	[bp].bump,01h	;if diagonal length is even
	jnz	izdiag25
	xchg	bx,si		;..restore 'normal' row,column
izdiag25:
	dec	bx		;previous row
	inc	si		;next column
	loop	izdiagl2	;for all rows and columns in this diagonal
	pop	cx
	mov	bx,[bp].bump	;next row start coordinate
	inc	[bp].bump	;next diagonal length
	xor	si,si
	loop	izdiaglp
;indexes generated in zigzag diagonal order in work area
	pop	si
	pop	ds		;ds:si->source
	pop	cx		;number of indexes
	xor	ax,ax		;assume normal order index generator
	mov	[bp].bump,1	;increment
	cmp	[bp].dir,0
	je	izdiag30
	mov	ax,cx		;reversed order, count backwards
	dec	ax
	mov	[bp].bump,-1	;decrement
izdiag30:
	les	di,[bp].ndxp
izdiagl3:
	mov	bx,[si] 	;get offset
	shl	bx,1
	mov	es:[bx][di],ax	;put index into dest+offset
	add	si,2		;next offset
	add	ax,[bp].bump	;next index
	loop	izdiagl3	;for all indexes
	pop	ds
	pop	es
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	add	sp,2
	pop	bp
	ret
i_zdiag endp

;permute source into destination by indexes, parms mapped by permprm
permf	proc	far
	push	bp		;save old frame ptr
	mov	bp,sp		;this frame mapped by permprm
	push	bx
	push	cx
	push	si
	push	di
	push	es
	push	ds
	mov	cx,[bp].elcnt	;number of elements to permute
	les	di,[bp].destp	;..destination
	lds	si,[bp].ndx2p	;source indexes
floop:
	mov	bx,[si] 	;get ith index
	push	ds
	push	si
	lds	si,[bp].sourcep ;->source
	mov	al,[si][bx]	;get the ith byte
	stosb			;..and put it in destination
	pop	si
	pop	ds
	add	si,2		;next index
	loop	floop
	pop	ds
	pop	es
	pop	di
	pop	si
	pop	cx
	pop	bx
	pop	bp
	ret
permf	endp

;inversely permute source into destination by indexes, parms mapped by permprm
permg	proc	far
	push	bp
	mov	bp,sp		;parameter addressability
	push	bx
	push	cx
	push	di
	push	si
	push	ds
	push	es
	mov	cx,[bp].elcnt	;number of bytes to permute
	les	di,[bp].ndx2p	;source of indexes
	lds	si,[bp].sourcep ;source of bytes to permute
gloop:
	mov	bx,es:[di]	;ith output index
	lodsb			;ith source byte
	push	es
	push	di
	les	di,[bp].destp	;destination of byte
	mov	es:[di][bx],al
	pop	di
	pop	es
	add	di,2		;next index
	loop	gloop		;do for all bytes
	pop	es
	pop	ds
	pop	si
	pop	di
	pop	cx
	pop	bx
	pop	bp
	ret
permg	endp

;reverse bytes along row axis (reverses columns)
i_revrow proc	far
	push	bp
	mov	bp,sp		;parm addressability
	push	cx
	push	dx
	push	si
	push	di
	push	es
	push	ds
	lds	si,[bp].xfrmtxt ;point to start of row
	les	di,[bp].xfrmtxt
	mov	dx,[bp].xfrmcol ;number of columns
	add	di,dx
	dec	di		;point to end of row
	mov	cx,[bp].xfrmrow 	;for each row
revrwl1:
	push	cx
	push	si
	push	di
	mov	cx,dx
	shr	cx,1		;for half the columns
revrwl2:
	mov	al,[si] 	;get start of row letter
	xchg	al,es:[di]	;swap with end of row
	xchg	al,[si]
	dec	di
	inc	si
	loop	revrwl2 	;do for half the columns
	pop	di
	pop	si
	pop	cx
	add	si,dx		;next row
	add	di,dx
	loop	revrwl1 	;do for all rows
	pop	ds
	pop	es
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bp
	ret
i_revrow endp

;reverse bytes along column axis (reverses rows)
i_revcol proc	far
	push	bp
	mov	bp,sp		;parm addressability
	push	cx
	push	dx
	push	di
	push	si
	push	ds
	push	es
	lds	si,[bp].xfrmtxt 	;start of ciphertext
	les	di,[bp].xfrmtxt
	mov	ax,[bp].xfrmrow
	mul	[bp].xfrmcol
	add	di,ax
	mov	cx,[bp].xfrmcol 	;for all columns
	sub	di,cx			;es:di->last row of ciphertext
	mov	dx,[bp].xfrmrow 	;dx = rows/2
	shr	dx,1
revclp1:
	push	si
	push	di
	push	cx
	mov	cx,dx		;for half the rows
revclp2:
	mov	al,[si] 	;swap start of row
	xchg	al,es:[di]	;..with end of row
	xchg	al,[si]
	add	si,[bp].xfrmcol ;next row
	sub	di,[bp].xfrmcol ;previous row
	loop	revclp2
	pop	cx
	pop	di
	pop	si
	inc	si		;next column
	inc	di		;next column
	loop	revclp1
	pop	es
	pop	ds
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bp
	ret
i_revcol endp

;transpose column, row axes
i_tran	proc	far
	push	bp
	mov	bp,sp		;parm addressability
	push	cx
	push	dx
	push	di
	push	si
	push	ds
	push	es
	les	di,[bp].xfrmwrk ;work area temp destination
	lds	si,[bp].xfrmtxt ;ciphertext source
	mov	cx,[bp].xfrmcol
tranlp1:
	push	cx
	push	si
	mov	cx,[bp].xfrmrow
tranlp2:
	mov	al,[si]
	stosb
	add	si,[bp].xfrmcol ;next row
	loop	tranlp2 	;do for all rows
	pop	si
	pop	cx
	inc	si		;next column
	loop	tranlp1 	;do for all columns
	les	di,[bp].xfrmtxt ;ciphertext is destination
	les	si,[bp].xfrmwrk ;work area is source
	mov	ax,[bp].xfrmrow
	mul	[bp].xfrmcol	;bytes to move
	mov	cx,ax
	rep movsb
	pop	es
	pop	ds
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bp
	ret
i_tran	endp

;anti-transpose column, row axes
i_atran proc	far
	push	bp
	mov	bp,sp		;parm addressability
	push	cx
	push	dx
	push	di
	push	si
	push	ds
	push	es
	les	di,[bp].xfrmwrk ;temp work area
	lds	si,[bp].xfrmtxt ;ciphertext source
	mov	ax,[bp].xfrmrow
	mul	[bp].xfrmcol
	add	si,ax
	dec	si		;->last byte of ciphertext
	mov	dx,ax		;save length for later
	mov	cx,[bp].xfrmcol
atranl1:
	push	cx
	push	si
	mov	cx,[bp].xfrmrow
atranl2:
	mov	al,[si]
	stosb
	sub	si,[bp].xfrmcol ;previous row
	loop	atranl2 	;do for all rows
	pop	si
	pop	cx
	dec	si		;previous column
	loop	atranl1 	;do for all columns
	les	di,[bp].xfrmtxt
	lds	si,[bp].xfrmwrk
	mov	cx,dx
	rep movsb
	pop	es
	pop	ds
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bp
	ret
i_atran endp

crypt22 ends
	end
