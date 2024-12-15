	page	53,132
	include frame.str
	include xposprm.str
	include bif.str
	include bifkey.str
	include crypt4b.asm
	extrn	showmsg:far
crypt4	segment 'crya'
	assume	cs:crypt4,ds:crypt4b
	public	log
log	proc	far
	push	ds
	mov	dx,crypt4b
	mov	ds,dx		;addressability
	mov	function,ax	;save function code
	mov	ax,[bp].gramb	;save current cipher line
	mov	savegrm,ax
	mov	[bp].gramb,0	;and force to 1st line for myself
	mov	dx,offset logfile	;try to open file
	mov	ax,3D02h	;for reading/writing
	int	21h
	jnc	log10
;file doesn't exist, create it
	mov	dx,offset logfile
	mov	ah,3Ch
	xor	cx,cx
	int	21h
	mov	logh,ax
	jnc	log20
logerr:
	mov	si,offset msg1	;log file error message
	mov	di,24*160+80
	call	showmsg
	mov	ax,savegrm	;restore current cipher line
	mov	[bp].gramb,ax
	stc			;bad return
	jmp	logxt
log10:
	mov	logh,ax 	;save file handle
	xor	cx,cx		;position logfile at end
	xor	dx,dx
	mov	bx,ax
	mov	ax,4202h
	int	21h
	jnc	log15
loger2:
	mov	bx,logh 	;positioning failed
	mov	ah,3Eh		;..close the file
	int	21h
	jmp	logerr
log15:				;if eof 1A, zap it.
	sub	ax,1
	sbb	dx,0
	mov	fposb,dx	;save position of last byte
	mov	fposa,ax
	mov	cx,dx
	mov	dx,ax
	mov	ax,4200h	;position file to last byte
	int	21h
	jc	loger2
	mov	dx,offset cline ;read one byte to check it
	mov	cx,1
	mov	ah,3Fh
	int	21h
	jc	loger2
	cmp	cline,1Ah	;if not end-of-file
	jne	log20		;..leave pointer past end-of-file
	mov	cx,fposb	;else reposition to last byte which
	mov	dx,fposa	;..will get zapped
	mov	ax,4200h
	int	21h
	jc	loger2
log20:
	mov	bx,function
	shl	bx,1		;determine what log function
	jmp	logtab[bx]

;log a simple substitution solution
simple:
	call	dotitle 	;log the title
	jc	loger2		;quit if i/o error
	mov	cx,[bp].lines	;for all cipher lines
	mov	ax,crypt4b
	mov	es,ax
smplp1:
	call	getline 	;get cipher line
	call	putline
	jnc	simp20
	jmp	loger2
simp20:
	call	plain		;translate to plaintext
	call	putline
	jnc	simp30
	jmp	loger2
simp30:
	loop	smplp1
;now put the key
	les	di,dword ptr [bp].alfb
	mov	cx,es:[di]
	mov	bx,logh
	push	ds
	lds	dx,dword ptr [bp].alfb
	add	dx,514		;ds:dx-> solved alphabet, K1
	mov	ah,40h
	int	21h
	pop	ds
	jnc	simp40
simp38:
	jmp	loger2
simp40:
	call	next
	jc	simp38
	push	ds
	lds	dx,dword ptr [bp].alfb
	add	dx,2		;ds:dx->cipher alphabet
	mov	ah,40h
	int	21h
	pop	ds
	jc	simp38
	call	next
	jc	simp38
	push	ds
	lds	dx,dword ptr [bp].alfb
	add	dx,770		;ds:dx->inverse alphabet, K2
	mov	ah,40h
	int	21h
	pop	ds
	jc	simp38
	call	next
	jc	simp38
	call	next
	jc	simp38
	jmp	logok

;log a rail fence solution
rail:
	call	dotitle 	;log the title, if any
	jnc	rail10
	jmp	loger2		;quit if i/o error
rail10:
;log the key
	mov	bx,logh 	;file handle
	push	ds
	lds	si,dword ptr [bp].lgptrb
	lds	dx,dword ptr [si]	;point to formatted key
	mov	cx,6	;6 bytes
	mov	ah,40h
	int	21h			;write the key
	pop	ds
	jnc	rail20
	jmp	loger2
rail20:
	call	next			;blank line
	jnc	rail30
	jmp	loger2
rail30:
;log the plaintext
	mov	ax,[bp].bilan		;number of bytes
	mov	es,[bp].rlndxa
	mov	di,[bp].plainptr	;es:di->plaintext
railp:
	call	put80		;write up to 80 bytes
	jnc	rail40
	jmp	loger2
rail40:
	call	next
	jnc	rail50
	jmp	loger2
rail50:
	add	di,cx		;bump by bytes written
	sub	ax,cx		;deduct bytes written
	ja	railp
	call	next
	jnc	rail60
	jmp	loger2
rail60:
	jmp	logok
;log a complete transposition
xpose:
	call	dotitle 	;record cryptogram title
	jnc	xpos10
	jmp	loger2		;..logging error
xpos10:
;log the key
	mov	bx,logh 	;file handle
	push	ds
	lds	si,dword ptr [bp].lgptrb
	lds	si,dword ptr [si].xposkyb ;point to formatted key
	xor	cx,cx
	mov	cl,[si] ;key length
	mov	dx,si
	inc	dx
	mov	ah,40h
	int	21h			;write the key
	pop	ds
	jnc	xpos20
	jmp	loger2
xpos20:
	call	next			;blank line
	jnc	xpos30
	jmp	loger2
xpos30:
;log the plaintext
	les	di,dword ptr [bp].lgptrb
	mov	ax,es:[di].xposcnt	;count of plaintext letters
	les	di,dword ptr es:[di].xposptb	;es:di->plaintext
	jmp	railp

;log a bifid
bifid:
	call	dotitle 	;log the title
	jnc	bifid10
	jmp	loger2		;quit if i/o error
bifid10:
	mov	flag1,0
	mov	bkeyl,0 	;key lines to log
	mov	bifptl,0	;plaintext lines to log
	mov	ax,[bp].bfkeyb	;if there is a key
	or	ax,[bp].bfkeya	;..set number of key lines
	jz	bifid20
	les	di,dword ptr [bp].bfkeyb
	mov	ax,es:[di].bifgrund
	mov	bkeyl,ax
	mov	bkeyw,ax
	mov	ax,es:[di].bifside
	mov	bkeyd,ax
	mov	di,es:[di].bifsect
	mov	bkeypb,di
	mov	bkeypa,es
bifid20:
	les	di,dword ptr [bp].bifprm1b
	mov	ax,es:[di].cnt
	mov	bifptl,ax
	mov	ax,es:[di].cipa
	mov	bifpta,ax
	mov	ax,es:[di].subp
	mov	bifptb,ax
	push	ds
	pop	es
bifid30:
	mov	cx,80
	mov	di,offset cline+1
	mov	al,' '
	rep stosb		;clear output buffer
	mov	cline,cl
	cmp	bkeyl,0
	je	bifid40
	call	movbkey
bifid40:
	cmp	bifptl,0
	je	bifid50
	call	movbpt
	call	putline 	;write [key] plaintext
	jmp	bifid30
;no plaintext, maybe key
bifid50:
	call	putline 	;write key line, no plaintext
	test	flag1,01h	;if this is 2nd twin
	jnz	bifid60 	;..we're done
	or	flag1,01h	;start twin
	push	es
	les	di,dword ptr [bp].bifprm2b
	mov	ax,es:[di].cnt
	mov	bifptl,ax
	mov	ax,es:[di].cipa
	mov	bifpta,ax
	mov	ax,es:[di].subp
	mov	bifptb,ax
	pop	es
	jmp	bifid30
bifid60:
	cmp	bkeyl,0 	;there may be more key lines than
	jne	bifid30 	;plaintext lines
	call	next		;one more blank line, just in case
logok:
	clc
	mov	ax,savegrm	;restore current cipher line
	mov	[bp].gramb,ax
logxt:
	pushf			;save return flag
	mov	ah,3Eh		;close the log file
	mov	bx,logh
	int	21h
	popf
	pop	ds
	ret
log	endp

;log the cryptogram title, if any
dotitle proc	near
	mov	bx,logh 	;file handle
	mov	es,[bp].edseg	;log the title first, if any
	mov	di,[bp].hdrptr	;ds:si->title line
	xor	cx,cx
	mov	cl,es:[di]	   ;length
	jcxz	dotit10
	mov	dx,di
	inc	dx
	push	ds
	mov	ds,[bp].edseg
	mov	ah,40h		;write code
	int	21h
	pop	ds
	jc	dotiter
	call	next		;next line
	jc	dotiter
dotit10:
	clc
dotiter:
	ret
dotitle endp

;write a carriage return, line feed sequence to log file
next	proc	near
	push	ax
	push	cx
	mov	dx,offset newline
	mov	cx,2
	mov	bx,logh
	mov	ah,40h
	int	21h
	pop	cx
	pop	ax
	ret
next	endp

;get current cipher line
getline proc	near
	push	ds
	push	cx
	mov	di,offset cline
	lds	si,dword ptr [bp].gramb ;->cipher line
	mov	cx,81
	rep movsb
	pop	cx
	pop	ds
	add	[bp].gramb,81		;next cipher line
	ret
getline endp

;write characters in cline
putline proc	near
	push	cx
	mov	bx,logh
	xor	cx,cx
	mov	cl,cline	;length of line
	jcxz	putl10		;..empty line
	mov	dx,offset cline+1
	mov	ah,40h
	int	21h
	jc	putlxt
putl10:
	call	next		;carriage return line feed
putlxt:
	pop	cx
	ret
putline endp

;translate characters in cline to plaintext
plain	proc	near
	push	es
	push	cx
	xor	cx,cx
	mov	cl,cline
	jcxz	plainxt
	mov	si,offset cline+1
	mov	di,offset cline+1
plainlp:
	lodsb
	push	es
	push	di
	push	cx
	les	di,dword ptr [bp].alfb
	mov	cx,es:[di]
	add	di,2
	mov	bx,di
	repne scasb
	jne	plain10
	dec	di
	sub	di,bx
	mov	al,es:[di][bx+512]	;get solved letter
plain10:
	pop	cx
	pop	di
	pop	es
	stosb
	loop	plainlp
plainxt:
	pop	cx
	pop	es
	ret
plain	endp

;write up to 80 bytes
;es:di-> ascii plaintext
;ax = total length to write
put80	proc	near
	push	ax
	mov	cx,80		;assume full write
	cmp	ax,cx		;..but no more than 80
	jnb	put80aa
	mov	cx,ax
put80aa:		;number of bytes to write is resolved
	mov	bx,logh
	push	ds
	push	es
	pop	ds
	mov	dx,di
	mov	ah,40h
	int	21h
	pop	ds
	pop	ax
	ret
put80	endp

;move bifid key line into buffer
movbkey proc	near
	push	ds
	mov	di,offset cline+1
	mov	cx,bkeyw	;number of key bytes to move
	lds	si,dword ptr bkeypb
	push	cx
	push	si
	rep movsb
	pop	si
	pop	ax
	pop	ds
	mov	cline,al	;length of data to write
	add	si,bkeyd	;next key line
	mov	bkeypb,si
	dec	bkeyl		;one less to write
	ret
movbkey endp

;move bifid plaintext up to 64 chars into cline
movbpt	proc	near
	push	ds
	mov	cx,64
	cmp	cx,bifptl
	jbe	movbpt10
	mov	cx,bifptl
movbpt10:
	mov	di,offset cline+17
	lds	si,dword ptr bifptb
	push	cx
	push	si
	rep movsb
	pop	si
	pop	ax
	pop	ds
	add	si,ax
	mov	bifptb,si	;next plaintext
	sub	bifptl,ax	;remaining length
	add	ax,16
	mov	cline,al	;adjust buffer length
	ret
movbpt	endp

crypt4	ends
	end
