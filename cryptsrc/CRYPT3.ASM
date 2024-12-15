	page	53,132
	include frame.str
	extrn	getcurs:far,showmsg:far,newgram:far
	include crypt3b.asm
crypt3	segment 'crya'
	assume	cs:crypt3,ds:crypt3b
	public	cmd_toggle,cmd_ex
cmd_toggle proc far
	push	ds
	mov	ax,crypt3b
	mov	ds,ax
	mov	cmdptr,di	;save ptr to command line
	mov	cmdseg,es
	test	[bp].modea,01h	;if in command mode
	jnz	esc50		;..leave command mode
	push	dx		;save current command position
	or	[bp].modea,01h	;else go to command mode
	call	getcurs 	;get cursor position
	mov	oldpos,dx	;save for restoring
	pop	dx		;restore command cursor position
	mov	di,offset saveline ;save the status/message line
	push	ds
	push	ds
	pop	es		;es:di-> save area
	lds	si,dword ptr [bp].videob
	mov	si,12*160	;ds:si-> message line
	mov	cx,80
	rep movsw
	push	ds		;make message line a command line
	pop	es
	pop	ds
	lds	si,dword ptr cmdptr
	mov	di,12*160	;es:di -> message line
	mov	cx,80
	inc	si		;ds:si -> command line
	mov	ah,70h		;reverse video
cmdlp:
	lodsb		;display the command
	stosw
	loop	cmdlp
	pop	ds
	ret
esc50:
	call	cmd_ret
	pop	ds
	ret
cmd_toggle endp
cmd_ex	proc	far
	push	ds
	mov	ax,crypt3b
	mov	ds,ax
	mov	cmdptr,di	;save ptr to command line
	mov	cmdseg,es
;parse and execute the command
	xor	cx,cx
	mov	cl,es:[di]
	mov	nxtlen,cx	;remaining length of command line
	inc	di
	mov	nxtptr,di	;start of next token
	call	toka		;find start of command line token
	jnc	cmd10
	jmp	cmdxit		;no token found
cmd10:
	mov	tokptr,di
	call	tokz		;find length of token
	mov	toklen,cx
	call	gocmd
	jc	cmdxit2
cmdxit:
	call	cmd_ret 	;return from command mode
cmdxit2:
	pop	ds
	ret
cmd_ex	endp
cmd_ret proc	near
	and	[bp].modea,0FEh ;leave command mode
	les	di,dword ptr [bp].videob  ;restore former video line
	mov	di,12*160	;es:di->command line on screen
	mov	si,offset saveline ;ds:si->former video line
	mov	cx,80
	rep movsw
	mov	dx,oldpos	;return old cursor position
	ret
cmd_ret endp

gocmd	proc	near
	mov	si,offset cmdtab
	mov	cx,cmdtabn	;find the command
gocmdlp:
	call	gleich		;compare for equal
	je	gocmd10
	add	si,11
	loop	gocmdlp
	mov	si,offset err1
	mov	di,24*160+80	;command not found
	call	showmsg
	stc
	ret
gocmd10:
	mov	si,word ptr [si+9]
	call	si
	ret
gocmd	endp

gleich	proc	near
	push	si
	push	cx
	xor	cx,cx
	mov	cl,[si]
	cmp	cx,toklen
	jne	glexit
	inc	si
	mov	di,tokptr
	repe cmpsb
glexit:
	pop	cx
	pop	si
	ret
gleich	endp

toka	proc	near
	mov	cx,nxtlen	;remaining length of command line
	jcxz	toka20		;..zero length
	mov	di,nxtptr	;remaining ptr to command line
	mov	al,' '          ;seek 1st non-blank if any
	repe scasb
	je	toka20		;all blanks, no token
	dec	di
	mov	nxtptr,di
	inc	cx
	mov	nxtlen,cx
	clc
	ret
toka20:
	stc	;no token left
	ret
toka	endp

tokz	proc	near
	mov	cx,nxtlen
	mov	di,nxtptr
	mov	al,' '          ;seek 1st blank if any
	repne scasb
	jne	tokz10		;not terminated by blank
	dec	di
tokz10:
	mov	nxtptr,di
	mov	cx,di
	sub	cx,tokptr	;length of token
	mov	ax,nxtlen
	sub	ax,cx
	mov	nxtlen,ax
	ret
tokz	endp
get	proc	near
	clc
	mov	bx,0
	ret
get	endp
nom	proc	near
	clc
	mov	bx,1
	ret
nom	endp
start	proc	near
	call	close		;close existing file
	and	[bp].flaga,0FCh ;no file specified
	call	toka	;new file specified?
	jc	strt50	;..nope
	mov	tokptr,di	;start of fileid
	call	tokz		;on return cx has length
	mov	si,tokptr
	push	es
	pop	ds
	mov	es,[bp].pspa
	mov	di,80h
	mov byte ptr es:[di],cl
	inc	di
	rep movsb
	call	newgram
	mov	[bp].grm_handle,0
;reset everything
strt50:
	les	di,dword ptr [bp].abovob       ;original free ptr
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	mov	[bp].sbuffb,di
	mov	[bp].sbuffa,es
	les	di,dword ptr [bp].abovod       ;original free space bytes
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	mov	[bp].lines,0
	mov	[bp].flagb,0
	mov	[bp].modea,0
	mov	[bp].modeb,0
	mov	es,[bp].edseg	;reset header
	mov	di,[bp].hdrptr
	mov	al,0
	stosb
	mov	cx,80
	mov	al,' '
	rep stosb
	les	di,dword ptr [bp].alfb
	add	di,514	;point to solved letters
	mov	cx,512	;blank em all, solved and inverse solved
	rep stosb
	mov	ax,crypt3b	;re-establish addressability
	mov	ds,ax
	mov	bx,2
	clc
	ret
start	endp
close	proc	near
	test	[bp].flaga,01h
	jz	closext
	mov	bx,[bp].grm_handle
	mov	ah,3Eh
	int	21h
closext:
	ret
close	endp
crypt3	ends
	end
