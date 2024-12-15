	page	53,132
	include frame.str
	include crypthb.asm
	extrn	getmem:far,freeadj:far,showmsg:far,int2asc:far
	extrn	showtit:far
crypth	segment 'crya'
	assume	cs:crypth,ds:crypthb
	public	help
;on entry, ax = help number wanted
help	proc	far
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	es
	push	ds
	mov	dx,crypthb
	mov	ds,dx		;addressability
	mov	helpno,ax
	mov	flag,0		;reset flags
	mov	count,0 	;reset read buffer byte count
	les	di,dword ptr [bp].freehb	;save memory state
	mov	freeb,di
	mov	freea,es
	les	di,dword ptr [bp].sizeb
	mov	spaceb,di
	mov	spacea,es
	call	freeadj 	;set next paragraph
	mov	cx,25*160	;bytes to save one video buffer
	call	getmem
	jnc	help10
	mov	si,offset msg1
errxt01:
	mov	di,24*160+80	;message area
	call	showmsg 	;not enough memory to save a screen
	stc			;..error msg displayed, to be cleared
	jmp	helpxt		;..quit
help10:
	mov	savescrb,di	;save screen save area ptr
	mov	savescra,es
	push	ds
	lds	si,dword ptr [bp].videob
	mov	cx,25*80	;25 lines, 80 chars,attrs each
	rep movsw		;save the screen
	pop	ds
;find the help file
	mov	ax,helpno
	mov	si,offset hname+3 ;destination for digits in help name
	call	int2asc
	mov	ax,'TP'         ;fix up clobbered name
	push	ds
	pop	es
	mov	di,offset hname+3
	stosw
	mov	al,'0'
	mov	cx,2
help12:
	cmp byte ptr es:[di],' '
	jne	help20
	stosb
	loop	help12
help20: 			;open the help file
	mov	ax,3D00h
	mov	dx,offset hname
	int	21h
	jnc	help30
	mov	si,offset msg2	;couldn't find the help file!!!
	jmp	errxt01 	;error exit, no screen restoring
help30:
	mov	helph,ax	;save file handle
	mov	di,offset helptit	;clear the title area
	call	clear
	mov	[bp].helplb,di		;title line destination
	mov	[bp].helpla,es
	call	movline 	;move title from buffer to title area
;now load the help file
	mov	hlines,0	;number of help lines
	mov	cx,80		;get space for help line
	call	getmem
	jnc	help50
help45:
	mov	ah,3Eh		;close the help file
	mov	bx,helph
	int	21h
	mov	si,offset msg1
	jmp	errxt01
help50:
	mov	[bp].helplb,di	;destination help line
	mov	help1b,di	;1st help line
	mov	[bp].helpla,es
	mov	help1a,es
	call	clear
loadlp:
	test	flag,01h	;if end of file
	jnz	eof		;..show the help
	call	movline
	test	flag,80h	;if i/o error
	jz	help60
	stc			;..quit
	jmp	helpxt
help60:
	inc	hlines		;count the line
	mov	cx,80		;need another line maybe
	call	getmem
	jc	help45
	mov	[bp].helplb,di
	mov	[bp].helpla,es
	call	clear
	jmp	loadlp
eof:
	mov	ah,3Eh		;close the help file
	mov	bx,helph
	int	21h
;now display help
	mov	currh,0 	;current top help line
	mov	ax,hlines	;figure number of screens
	xor	dx,dx
	div	c23		;23 help lines per screen
	cmp	dx,0		;if remainder
	je	help70
	inc	ax		;..count an extra screen
help70:
	mov	hscreens,ax	;save number of screens
	mov	si,offset l25tot	;prepare footer
	call	int2asc
	mov	folge,1 	;current help screen number

;show help screen title
	mov	si,offset helptit-1
	call	showtit
;show footer
showfoot:
	mov	ax,folge	;update screen number
	mov	si,offset l25ith
	call	int2asc
	mov	si,offset line25
	mov	di,24*160
	call	showmsg
;show help
	call	showhlp
forever:
	mov	ah,0
	int	16h
	cmp	al,0		;if not extended
	jne	forever 	;..ignore
	cmp	ah,60		;if f2
	je	quit		;..quit help
	cmp	ah,73		;if page up
	je	pup		;..scroll help up
	cmp	ah,81		;if page down
	je	pdn		;..scrool help down
	jmp	forever 	;unknown key
pup:
	mov	ax,currh	;see if we can scroll
	sub	ax,23		;the attempt
	jnc	pup10		;..good, we can go back 23 lines
	jmp	forever 	;must be at top, ignore
pup10:
	mov	currh,ax	;update current top line
	dec	folge		;count current screen
	jmp	showfoot	;show new screen and footer
pdn:
	mov	ax,currh	;see if we can scroll
	add	ax,23
	cmp	ax,hlines	;if scrolling would take us past text
	jb	pdn10
	jmp	forever 	;..ignore scrolling
pdn10:
	mov	currh,ax	;update current top line
	inc	folge		;increment screen number
	jmp	showfoot	;show new screen and footer
quit:
hlpxt02:		;on exit, restore the screen
	les	di,dword ptr [bp].videob
	push	ds
	lds	si,dword ptr savescrb
	mov	cx,25*80
	rep movsw
	pop	ds
helpxt: 		;on exit, restore the memory
	les	di,dword ptr freeb	;restore memory
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr spaceb
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	pop	ds	;on exit, restore registers
	pop	es
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
help	endp

;try to read 256 bytes of help (may not be that much)
read	proc	near
	mov	count,0
	test	flag,04h	;if pending eof
	jnz	readeof 	;..make it eof now
	mov	bx,helph
	mov	ah,3Fh
	mov	dx,offset helpbuf
	mov	buffptr,dx
	mov	cx,256
	int	21h
	jnc	read10
	or	flag,80h	;flag i/o error
	mov	si,offset msg3
	mov	di,24*160+80
	call	showmsg
	mov	ah,3Eh		;close the help file
	mov	bx,helph
	int	21h
	ret
read10:
	mov	count,ax	;bytes read, could be 0, beware
	cmp	ax,0
	jne	read20
readeof:
	or	flag,01h	;flag end of file
	ret
read20:
	cmp	ax,cx
	je	read30
	or	flag,04h	;flag pending end of file
read30:
	ret
read	endp

;clear line of form length,80 databytes
;es:di-> 80 byte area
clear	proc	near
	push	di
	mov	al,' '
	mov	cx,80
	rep stosb
	pop	di
	ret
clear	endp

;move one line from buffer to line area
;ds:buffptr -> current position in buffer
;count = number of characters in buffer
movline proc	near
	push	es
mvln00:
	mov	cx,count
	jcxz	mvln05
	les	di,dword ptr [bp].helplb
	mov	si,buffptr
mvlnlp:
	cmp byte ptr [si],0Ah	;carriage return
	je	mvln10		;throw this one away, but keep on going
	cmp byte ptr [si],0Dh	;new line
	je	mvln20		;throw this away and stop
	cmp byte ptr [si],1Ah	;end of file
	je	mvln15		;throw this away and stop
	movsb
mvlnret:
	loop	mvlnlp
	mov	[bp].helplb,di	;save where we left off
mvln05:
	call	read
	test	flag,81h	;if i/o error or end of file
	jnz	mvlnxt02	;..quit
	jmp	mvln00
mvlnxt:
	mov	count,cx	;remaining count of input
	mov	[bp].helplb,di	;next output position
	mov	buffptr,si	;next input position
mvlnxt02:
	pop	es
	ret
mvln10: 			;ignore this character
	inc	si
	jmp	mvlnret
mvln15:
	or	flag,01h	;flag end of file found
	jmp	mvlnxt02
mvln20: 			;ignore this character and quit
	inc	si
	dec	cx
	jmp	mvlnxt
movline endp

showhlp proc	near
	mov	cx,hlines	;show help lines
	sub	cx,currh	;from current 1st
	cmp	cx,23		;but no more than 23 at a time
	jna	shwhlp10
	mov	cx,23
shwhlp10:
	mov	ax,currh
	mul	c80		;current top line
	add	ax,help1b	;offset to current top line
	mov	di,160
	mov	es,[bp].videoa	;es:di->screen buffer
	push	ds
	mov	ds,help1a
	mov	si,ax		;ds:si->help lines
	push	cx		;save lines to display
	mov	ah,07		;normal attribute
shwhlp1:
	push	cx
	mov	cx,80
shwhlp2:
	lodsb			;move one help line to video buffer
	stosw
	loop	shwhlp2
	pop	cx
	loop	shwhlp1 	;do next line
	pop	ax		;recover number of displayed lines
	pop	ds		;recover addressability
	mov	cx,23		;if less than 23 lines displayed
	sub	cx,ax
	jz	shwhxt
	mov	al,' '          ;..blank rest of screen
shwhlp3:
	push	cx
	mov	cx,80
	rep stosw
	pop	cx
	loop	shwhlp3
shwhxt:
	ret
showhlp endp
crypth	ends
	end
