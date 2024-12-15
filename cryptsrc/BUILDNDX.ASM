	page	53,132
	include index.str
builda	segment public
ndx	index	<>
outbuff label	byte
	org	$+2048
inbuff	label	byte
	org	$+2048
pat_handle dw	?
ndx_handle dw	?
partl	dw	0
stringl dw	0
patternl dw	0
longest dw	0
ndxcnt	dw	0
counter dw	0
inptr	dw	inbuff
outptr	dw	outbuff
flocb	dw	0		;file location, hi bytes
floca	dw	0		;file location, lo bytes
patspec db	'english.dct',0   ;pattern dictionary filespec
ndxspec db	'english.ndx',0   ;index filespce
err1	db	'Pattern file not found',13,10,'$'
err2	db	'Index file error',13,10,'$'
err3	db 17 dup(' ')
	db	'pattern list longer than 65535 bytes',13,10,'$'
filemark db	'PatternIndex'
flag1	db	0	;01 = end of file
			;02 = 1st pattern found
builda	ends

buildb	segment public
	assume	ds:builda,cs:buildb
build	proc	far
	push	ds
	xor	ax,ax
	push	ax
	mov	ax,builda
	mov	ds,ax
	mov	es,ax
	cld
;find the pattern dictionary
	mov	dx,offset patspec     ;try default drive first
	mov	ax,3D00h	;open for read
	int	21h
	jnc	build20
;not there, quit
	mov	dx,offset err1	;no pattern dictionary
	call	prompt
	jmp	exit		;for now
build20:
	mov	pat_handle,ax
;create the index file in the same place
	mov	dx,offset ndxspec
	xor	cx,cx
	mov	ax,3C02h
	int	21h
	jnc	build30
	mov	dx,offset err2
	call	prompt
	jmp	exit00
build30:
	mov	ndx_handle,ax	;write a place holder for length,count
	mov	dx,offset ndx.packwork
	mov	bx,ax
	mov	cx,16
	mov	ah,40h
	int	21h
	jnc	build40
build35:
	mov	dx,offset err2
	call	prompt
	jmp	exit00
build40:
	cmp	ax,cx
	jne	build35
bldndx:
	test	flag1,01h	;while not end of file
	jnz	bldndxz
	call	get_strng	;..build index
	add	ax,partl	;string length = partial length from last
	mov	stringl,ax	;..string, plus length read this string
tokenlp:
	call	get_tok 	;get a token
	jc	bldndx		;..if none, get next string
	cmp byte ptr [si],'A'
	jne	itsaword
;start of a pattern list
	test	flag1,02h	;if first time
	jz	bldndx10	;..there's no completed list
	call	movndx		;else complete index entry
	cmp	counter,128	;if the out buffer is full
	jb	bldndx10
	call	putndx		;..write it out
	jc	exit02		;write error, quit
bldndx10:
	or	flag1,02h
	mov	ax,floca	;plug in file position
	mov	ndx.patloca,ax
	mov	ax,flocb
	mov	ndx.patlocb,ax
	mov	patternl,cx	;pattern token length
	mov	ndx.wcount,0	    ;initialize for list words
	call	pack		;si->pattern
	jmp	tokenlp 	;next token
itsaword:
	inc	ndx.wcount	    ;count words in this list
	jmp	tokenlp
bldndxz:
	call	movndx		;flush last index entry
	call	putndx
	jc	exit02
	mov	di,offset outbuff	;prepare the filemark
	mov	si,offset filemark
	mov	cx,12
	rep movsb
	mov	ax,ndxcnt	;count of index entries to filemark
	stosw
	mov	ax,longest	;longest list to filemark
	stosw
	mov	counter,1	;one pseudo index entry
	mov	ax,4200h	;reset file to beginning
	mov	dx,cx
	mov	bx,ndx_handle
	int	21h
	call	putndx		;write the filemark
exit02:
	mov	bx,ndx_handle
	mov	ah,3Eh
	int	21h
exit00:
	mov	bx,pat_handle
	mov	ah,3Eh
	int	21h
exit:
	ret
build	endp

prompt	proc	near
	mov	ah,9h
	int	21h
	ret
prompt	endp

get_strng proc	near
	mov	cx,2048 	;size of pattern buffer
	sub	cx,partl	;..less wrapped token length
	mov	bx,pat_handle
	mov	dx,offset inbuff
	mov	inptr,dx
	add	dx,partl
	mov	ah,3Fh
	int	21h
	cmp	ax,cx
	je	getpatxt
	or	flag1,01h	;flag end of file
getpatxt:
	ret
get_strng endp

get_tok proc	near
	mov	cx,stringl	;length of remaining string
	jcxz	token07
	mov	di,inptr
	mov	si,di
	mov	al,'A'
token05:		;look for start of token
	cmp	[si],al
	jnb	token10
	inc	si
	add	floca,1 ;position of character in file
	adc	flocb,0
	loop	token05
token07:		;no token starts in the rest of this buffer
	mov	stringl,cx	;remaining string length is 0
	mov	partl,0 	;no partial token length
token08:
	stc		;no token found
	ret
token10:	;start of token found
	mov	di,si
token15:		;look for end of token
	cmp	[di],al
	jb	token20
	inc	di
	add	floca,1 ;position of character in file
	adc	flocb,0
	loop	token15
;partial token
	mov	stringl,cx	;reset string length to 0
	mov	cx,di		;length of partial token
	sub	cx,si
	mov	partl,cx	;save partial length
	sub	floca,cx	;correct file position for rescanning
	sbb	flocb,0
	mov	di,offset inbuff
	rep movsb
	jmp	token08
token20:
	mov	stringl,cx	;remaining string length
	mov	inptr,di      ;next string position
	mov	cx,di		;length of token
	sub	cx,si		;si->start of token
	clc			;di->just past token
	ret
get_tok endp

movndx	proc	near
	push	di
	push	si
	push	cx
	mov	ax,floca  ;figure length of this list
	mov	dx,flocb  ;flocb,floca = end of pattern list + next patlen
	sub	ax,ndx.patloca	;..less start of list
	sbb	dx,ndx.patlocb
	inc	cx
	sub	ax,cx		;..less next pattern length
	sbb	dx,0
	mov	ndx.listlen,ax	;..plug in length
	jz	movndx02	;..if not greater than 1 segment 65535
	mov	ndx.listlen,0	;flag too long by length 0
	mov	dx,offset err3	;tell the world list too long
	call	prompt
	jmp short movndx04
movndx02:
	cmp	ax,longest	;keep track of longest list
	jbe	movndx04
	mov	longest,ax
movndx04:
	mov	di,outptr	;destination
	mov	si,offset ndx.packwork
	mov	cx,16
	rep movsb
	inc	ndxcnt		;count the index
	inc	counter 	;count the index per this buffer
	mov	outptr,di	;update next table location
	pop	cx
	pop	si
	pop	di
	ret
movndx	endp

putndx	proc	near
	push	cx
	mov	ax,counter	;convert count to bytes
	mov	cl,4
	shl	ax,cl
	mov	cx,ax		;bytes to write
	mov	dx,offset outbuff
	mov	bx,ndx_handle
	mov	ah,40h
	int	21h
	jnc	putndx04
putndx02:
	mov	dx,offset err2
	call	prompt
	stc
	jmp short putndxit
putndx04:
	cmp	ax,cx
	jne	putndx02
	mov	counter,0
	mov	outptr,offset outbuff
	clc
putndxit:
	pop	cx
	ret
putndx	endp

;es:si->character string
;patternl = length of string
;sets packwork for index entry
pack	proc	near
	mov	cx,8	     ;zero accumulator
	mov	di,offset ndx.packwork
	xor	ax,ax
	rep stosb
	mov	cx,patternl	;save pattern if error msg needed
	mov	di,offset err3
	mov	bx,si		;save pattern start
	rep movsb
	std			;go backwards
	mov	di,offset ndx.packwork+7	;->last byte destination
	mov	si,bx
	mov	cx,patternl
	add	si,cx
	sub	si,2		;->2nd last byte source
	shr	cx,1		;halve the length
packlp:
	jcxz	pack10		;check for odd number of characters
	lodsw			;pick up two bytes
	sub	al,40h		;..make numerical
	sub	ah,40h
	shl	al,1		;..then pack
	shl	al,1
	shl	al,1
	shl	al,1
	or	al,ah
	stosb			;..and plunk into index
	loop	packlp		;next pair of letters
pack10:
	test byte ptr patternl,01h	;if length is odd
	jz	pack20
	mov	al,[si+1]		;..one more letter left
	sub	al,40h			;..make number
	stosb				;..and stuff it.
pack20:
	cld			;return to forwards
	ret
pack	endp

buildb	ends

stack	segment stack
	db 16 dup ('stack   ')
stack	ends
	end	build
