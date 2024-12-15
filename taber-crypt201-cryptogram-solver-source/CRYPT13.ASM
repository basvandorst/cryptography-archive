	page	53,132
;DOS command line parser
	include frame.str
	include crypt13b.asm
crypt13 segment 'crya'
	assume	cs:crypt13,ds:crypt13b
	public	param,newgram
; cryptogram name followed by options
; crypt [ [d:][path\]filename[.ext] ]	default no cryptogram name
;	[ -l[anguage] [d:][path\]filename[.ext] ] default ENGLISH.DAT
;			if no extension default is .DAT
; case insensitive.
; format
;  crypt filename into fspec->gramid		for file i/o
;  cryptname	  into ?->gramid2		for panel display
;  language filename into langid->langspec	     for file i/o
param	proc	far
	push	ds
	mov	ax,crypt13b
	mov	ds,ax
;set up defaults - no cryptogram file name, and language data file
;		   english.dat
	mov	es,[bp].edseg
	mov	di,[bp].langspec
	mov	si,offset default1
	xor	cx,cx
	mov	cl,[si]
	inc	si
	rep movsb		;english.dat,0
	mov	di,[bp].langid
	mov	si,offset default1
	mov	cl,[si]
	dec	cl
	inc	si
	mov	es:[di],cl	;length of english.dat less trailing 0
	inc	di
	rep movsb
	mov	si,offset default2+1 ;default index file
	mov	cl,default2
	les	di,ndxspec
	rep movsb
	mov	si,offset default3+1 ;default dictionary file
	mov	cl,default3
	les	di,dctspec
	rep movsb
;parse for cryptogram name and language option
	mov	es,[bp].pspa
	mov	di,80h	;es:di-> command line in psp
	call	getstrng	;fetch command line string
	cmp	stringl,0	;if null string
	jne	param20
param10:  ;ok return, default parameters, no cryptogram name, english data
	clc
paramxt:
	pop	ds
	ret
;parameters
param20:
	mov	ax,crypt13b
	mov	es,ax
paramlp:
	call	getparm 	;isolate this parameter
	jc	param10 	;..no parameter, blank line
	cmp	parm,'-'        ;if option
	jne	param30
	jmp	option		;..handle option
;must be cryptogram file spec
param30:
	call	movea		;move crypt spec to its destination
	call	namen		;isolate the display name, move to edseg
	jmp	paramlp 	;get next parameter
option:
	cmp	parml,1 	;must be more than one byte
	ja	opt00
	stc			;abandon processing, invalid parameter
	jmp	paramxt
opt00:				;normalize option as lower case
	mov	si,offset parm
	mov	cx,parml
optlp1:
	or byte ptr [si],20h	;turn on lower case bit
	inc	si		;..for all option bytes
	loop	optlp1
;search the option table for this parameter
	mov	bx,offset optab 	;start of options
	mov	cx,(optabz-optab)/4	;number of options
	xor	ax,ax
optlp2:
	mov	di,[bx] 	;->name of this option
	mov	si,offset parm	;->user's string
	mov	al,[di] 	;max option length
	cmp	ax,parml	;if user string longer
	jb	opt20		;..try next option
	je	opt10		;if equal length, ok
	mov	ax,parml	;if user string shorter, use it
opt10:
	push	cx
	mov	cx,ax
	inc	di
	repe cmpsb
	pop	cx
	je	found
opt20:
	add	bx,4
	loop	optlp2
	stc			;invalid option, not found
	jmp	paramxt 	;leave
found:
	jmp	[bx+2]		;go to option routine

;language name option
tongue:
	call	getparm 	;get what language
	jnc	opt30
	jmp	paramxt 	;..no language specified
opt30:
	cmp	parml,8 	;no more than 8 chars
	jna	opt40
	stc
	jmp	paramxt
opt40:			;move language file names to destinations
	push	es
	mov	si,offset parm
	mov	es,[bp].edseg
	mov	di,[bp].langspec
	mov	cx,parml
	rep movsb
	mov	si,offset default1+8
	mov	cx,5
	rep movsb
	mov	si,offset parm
	mov	di,[bp].langid
	mov	cx,parml
	mov	es:[di],cl
	inc	di
	rep movsb
	mov	si,offset default1+8
	mov	cx,4
	rep movsb
	mov	si,offset parm
	les	di,ndxspec
	mov	cx,parml
	rep movsb
	mov	si,offset default2+8
	mov	cx,5
	rep movsb
	mov	si,offset parm
	les	di,dctspec
	mov	cx,parml
	rep movsb
	mov	si,offset default3+8
	mov	cx,5
	rep movsb
	pop	es
	jmp	paramlp
param	endp

;start a new cryptogram.  parse the cryptogram name
newgram proc	far
	push	ds
	mov	ax,crypt13b
	mov	ds,ax
;parse for cryptogram name and language option
	mov	es,[bp].pspa
	mov	di,80h	;es:di-> command line in psp
	call	getstrng	;fetch command line string
	cmp	stringl,0	;if null string
	jne	newgrm20
newgrm10:
	clc			;..no name, ok
	pop	ds
	ret
newgrm20:
	mov	ax,crypt13b
	mov	es,ax
	call	getparm
	jc	newgrm10	;no cryptogram name
	call	movea		;move cryptogram name to destination
	call	namen		;move cryptogram display name
	jmp	newgrm10
newgram endp

;name. strip path out of file spec to make display name
namen	proc	near
	mov	part1,0 	;assume no drive
	mov	di,offset parm
	mov	cx,parml
	mov	al,'\'          ;look for a path
	repne scasb
	jne	nopath
;assume drive specified
	mov	cx,di
	sub	cx,offset parm
	dec	cx
	cmp	cx,2		;only 2 chars for a drive
	je	name10
;something wrong, pretend no drive specified
	xor	cx,cx
name10:
	mov	part1,cx	;length of drive spec
	mov	di,offset dname
	mov	si,offset parm
	jcxz	name20		;no drive spec
	rep movsb		;move drive as part of cryptogram name
name20:
	mov	partb,di	;next spot in dname
	mov	di,offset parm
	mov	cx,parml
	add	di,cx		;point to end of filespec
	dec	di
	mov	si,di
	std
	repne scasb
	cld
	inc	di
	mov	cx,si
	sub	cx,di
	jcxz	nopath		;must have been a bare slash
	mov	si,di		;source
	mov	di,partb
	mov	ax,14		;validate length
	cmp	part1,0
	je	name30
	mov	ax,12
name30:
	cmp	cx,ax
	jna	name40
	mov	cx,ax		;filename.ext too long
name40:
	add	part1,cx
	inc	si		;skip over \
	rep movsb
	call	moveb
	ret
nopath:
	push	es
	mov	es,[bp].edseg
	mov	di,[bp].gramname
	mov	cx,parml
	mov	si,offset parm
	cmp	cx,14
	jna	name50
	mov	cx,14		;something wrong, filespec too long
name50:
	mov	es:[di],cl
	inc	di
	rep movsb
	pop	es
	ret
namen	endp

;movea, move cryptogram file spec to editor segment
movea	proc	near
	push	es
	les	di,dword ptr [bp].fspec
	mov	si,offset parm
	mov	cx,parml
	rep movsb
	mov byte ptr es:[di],0	;make into asciiz string
	or	[bp].flaga,01h	;flag cryptogram filespec given
	pop	es
	ret
movea	endp

;moveb, move display form of cryptogram filespec to editor segment
moveb	proc	 near
	push	es
	mov	es,[bp].edseg
	mov	di,[bp].gramname
	mov	si,offset dname
	mov	cx,part1
	mov	es:[di],cl
	inc	di
	rep movsb
	pop	es
	ret
moveb	endp

;isolate a parameter
getparm proc	near
	call	skbl	;skip leading blanks
	jnc	gtprm10
	stc		;no parm, nothing but blanks, or zero length
	ret
gtprm10:
	mov	si,di		;start of parm
	repne scasb		;find end of parm
	jne	gtprm20 	;no blank found
	dec	di		;blank found, adjust ptr
	inc	cx		;..and length
gtprm20:
	mov	stringl,cx	;remaining string length
	mov	hiwater,di	;resume ptr
	mov	cx,di		;length of parm
	sub	cx,si
	mov	parml,cx
	mov	di,offset parm
	rep movsb
	mov byte ptr es:[di],' '
	clc
	ret
getparm endp

;skip leading blanks in string if any
skbl	proc	near
	mov	di,hiwater	;begin where we left off
	mov	cx,stringl	;length to end of string
	mov	al,' '
	jcxz	noparm
	repe scasb
	je	noparm		;found nothing but blanks
	dec	di
	inc	cx
	clc
	ret
noparm:
	stc		;flag end of string
	ret
skbl	endp

;move string to this ds for ease of work
;es:di-> length,string,padbyte
	assume es:crypt13b,ds:nothing
getstrng proc	near
	push	ds
	push	ds		;exchange segments
	push	es
	pop	ds
	pop	es
	mov	si,di		;ds:si->source string
	mov	di,offset string
	mov	hiwater,di
	xor	cx,cx		;retrieve string length
	mov	cl,[si]
	mov	stringl,cx
	jcxz	gets20		;null string
	cmp	cx,80		;move no more than 80 bytes
	jna	gets10
	mov	cx,80
gets10:
	inc	si		;skip the length byte
	rep movsb
gets20:
	mov byte ptr es:[di],' '     ;blank terminate string
	pop	ds
	ret
getstrng endp

crypt13 ends
	end
