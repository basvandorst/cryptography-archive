	page	53,132
	include frame.str
	include motndx.str
cryptfb segment 'cryb'
cursor	dw	0
c10	dw	10
c32768	dw	32768
lstptr	dw	0		;indirect list ptr
lsth	dw	0		;list handle
fid	db	'CR     .YPT',0         ;file name
cryptfb ends
cryptf	segment 'crya'
	assume	cs:cryptf
	public	getmem,display,get_line,clear_line,del_line,showtit
	public	showmsg,getcurs,setcurs,voff,von,int2asc,savefree
	public	freeadj,savelist,readlist,droplist,asc2int,freemem
;get cx bytes of storage
;on return, if no carry es:di points to start of storage
;			cx = bytes obtained
;			sizeb,sizea reduced
;			freeh adjusted to next free memory byte
;	    if carry, insufficient storage to satisfy request
getmem	proc	far
	push	ax
	push	dx
	mov	ax,[bp].sizea	;low order bytes of available memory
	mov	dx,[bp].sizeb	;hi order
	sub	ax,cx		;test for enough storage
	sbb	dx,0
	jb	getmemxt	;not enough memory, leaves carry on
	mov	[bp].sizea,ax	;update available memory
	mov	[bp].sizeb,dx
;return hi water ptr to caller
getmem10:
	les	di,dword ptr [bp].freehb
	mov	ax,di
	add	ax,cx		;next hi water
	jc	wrapped 	;exceeded segment boundary
	mov	[bp].freehb,ax	;update hi water offset
getmemxt:
	pop	dx
	pop	ax
	ret
wrapped:
	xor	dx,dx
	mov	ax,cx		;save requested bytes
	mov	cx,4		;divide by 16
getmem20:
	shr	di,1		;adjust offset to paragraphs
	rcr	dl,1		;..and collect remainder of offset
	loop	getmem20	;di = no of paragraphs to bump segment ptr
	mov	cl,4		;adjust new offset
	shr	dx,cl
	add	[bp].freeha,di	;adjusted hi water segment paragraph
	mov	[bp].freehb,dx	;..and offset
	xor	dx,dx
	mov	cx,4
getmem30:
	shr	ax,1		;divide by 16
	rcr	dl,1		;..and collect remainder
	loop	getmem30
	mov	cl,4
	shr	dx,cl		;adjust remainder
	add	[bp].freeha,ax	;new paragraph
	add	[bp].freehb,dx	;new offset
	clc
	les	di,dword ptr [bp].freehb
	jmp	getmemxt
getmem	endp
;save current freespace ptr and size
;ds:si->8 byte save area, oldsizeb, oldesizea, oldfreeb, oldfreea
savefree proc	far
	les	di,dword ptr [bp].sizeb ;preserve old size
	mov	[si],di
	mov	[si+2],es
	les	di,dword ptr [bp].freehb ;preserve old free ptr
	mov	[si+4],di
	mov	[si+6],es
	ret
savefree endp
;adjust freespace ptr (and mem size) to segment boundary
freeadj proc	far
	mov	ax,[bp].freehb	;adjust freespace offset
	xor	dx,dx
	mov	cx,4
adj00:
	shr	ax,1		;divide offset by 16
	rcr	dl,1		;..collecting remainder in dl
	loop	adj00
	add	[bp].freeha,ax	;new segment
	mov	[bp].freehb,0	;new offset
	cmp	dl,0		;if no odd bytes left
	je	adj10		;..we're done
	inc	[bp].freeha	;else round up to next paragraph
	mov	cl,4		;..and deduct the rounding complement
	shr	dl,cl
	mov	ax,16
	sub	ax,dx
	sub	[bp].sizea,ax	;from remaining space
	sbb	[bp].sizeb,0
adj10:
	ret
freeadj endp
;restore memory controls
;ds:si->memory size, memory origin
freemem proc	far
	push	es
	push	di
	les	di,dword ptr [si]
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	les	di,dword ptr [si+4]
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	pop	di
	pop	es
	ret
freemem endp
;display cryptogram
display proc	far
	push	ax
	push	cx
	push	dx
	push	si
	push	ds
;figure number of lines to be displayed
	mov	ax,[bp].current
	mov	cx,[bp].lines	;number of lines
	sub	cx,ax		;..less current screen top
	cmp	cx,23		;..if this is too many for one screen
	jna	disp02
	mov	cx,23		;..display only 23
disp02:
	mul	[bp].c81
	les	di,dword ptr [bp].videob  ;destination
	mov	di,160		;skip the comment line
	lds	si,dword ptr [bp].sbuffb
	mov	si,ax		;source
	push	cx		;save number of lines
	mov	ah,07h		;normal attribute
displp:
	inc	si		;skip the length byte
	push	cx		;save number of lines to move
	mov	cx,80		;move this line
displn:
	lodsb
	stosw
	loop	displn
	pop	cx
	loop	displp		;..and all lines
	pop	ax		;recover number of lines
	mov	cx,23
	sub	cx,ax
	jcxz	disp10
;clean the rest of the screen
	mov	ax,0720h
dispcln:
	push	cx
	mov	cx,80
	rep stosw
	pop	cx
	loop	dispcln
disp10:
	pop	ds
	pop	si
	pop	dx
	pop	cx
	pop	ax
	ret
display endp
get_line proc	far
	push	ax
	push	cx
	push	dx
	push	es
	push	di
	push	si
	cmp	[bp].lines,809	;max size permitted
	jnb	getlinfl
	mov	cx,81		;get space for another line
	call far ptr getmem	;es:di->new space
	jc	getlinsp	;..no space
	inc	[bp].lines	;update the number of lines
	mov	cx,di		;compute number of bytes to move
	sub	cx,[bp].gramb	;beginning of new - beginning of current
	add	di,80		;es:di->last byte of new line
	mov	si,di		;compute source
	sub	si,81		;offset to last line
	push	ds		;save my addressability
	push	es
	pop	ds		;ds:si->last byte of last line
	std			;move bytes backwards
	rep movsb
	pop	ds		;recover addressability
	inc	di		;es:di->1st byte of inserted line
	cld			;move bytes forward
	mov	[bp].gramb,di	;new current cryptogram line
	mov	[bp].grama,es
	mov	ax,di		;compute number of lines to new line
	xor	dx,dx
	div	[bp].c81	;index of newline-1
	sub	ax,22		;..less a screen's worth
	jb	getlin05	;if less than a screen, keep current top
	cmp	ax,[bp].current ;if less than current
	jb	getlin05	;..keep current top
	mov	[bp].current,ax ;else update current top
getlin05:
	mov byte ptr es:[di],00h	;length 0
	inc	di
	mov	al,' '  ;rest of line is blanks
	mov	cx,80
	rep stosb
	clc		;good return flag
getlinxt:
	pop	si
	pop	di
	pop	es
	pop	dx
	pop	cx
	pop	ax
	ret
getlinfl:
	xor	bx,bx		;flag cryptogram too long
getliner:
	stc		;bad return flag
	jmp	getlinxt
getlinsp:
	mov	bx,1		;flag not enough memory
	jmp	getliner
get_line endp
;es:di->line to clear
clear_line proc far
	mov	cx,80		;max line length
	xor	ax,ax
	mov	al,dl		;..less cursor position
	sub	cx,ax		;..is letters to clear
	jcxz	clrlnxt 	;can't clear past end of line
	push	di		;save offset to start of line
	inc	di
	add	di,ax		;es:di->letter(s) to clear
	mov	al,' '          ;blank
	rep stosb
	dec	di		;back up to last character
	std
	mov	cx,80		;recount line length
	rep scasb
	cld
	pop	di
	je	clrln10
	inc	cx
clrln10:
	mov	es:[di],cl
clrlnxt:
	ret
clear_line endp
	assume	ds:cryptfb
del_line proc	far
	push	ds
	mov	ax,cryptfb
	mov	ds,ax
	mov	cursor,dx	;save the cursor for now
	mov	cx,[bp].lines	;figure lines to move down
	dec	cx
	jnz	del10
	xor	dl,dl		;there is only one line
	les	di,dword ptr [bp].gramb
	call far ptr clear_line ;..so just clear it
	pop	ds
	ret
del10:
	mov	[bp].lines,cx	;new number of lines
	mov	ax,[bp].freehb	;back up the free space pointer
	sub	ax,81
	mov	[bp].freehb,ax
	mov	dx,[bp].sizeb	;give back the space
	mov	ax,[bp].sizea
	add	ax,81
	adc	dx,0
	mov	[bp].sizeb,dx
	mov	[bp].sizea,ax
	mov	ax,[bp].gramb	;compute the current line index
	xor	dx,dx
	div	[bp].c81	;current line index
	sub	cx,ax		;number of lines to move
	jnz	del15		;don't try to squish last line
;deleted last line
	mov	ax,[bp].gramb	;..instead back up current line
	sub	ax,81
	mov	[bp].gramb,ax
	mov	dx,cursor	;restore cursor
	cmp	[bp].current,0	;if there is a line above current screen
	je	del12
	dec	[bp].current	;..put it on the screen
delxit:
	pop	ds		;..and leave cursor alone
	ret
del12:
	dec	dh		;else back up the cursor
	jmp	delxit
;squish out the current line
del15:
	push	ds
	les	di,dword ptr [bp].gramb
	lds	si,dword ptr [bp].gramb
	add	si,81
delp:
	push	cx
	mov	cx,81
	rep movsb
	pop	cx
	loop	delp
	pop	ds
	mov	dx,cursor	;restore the cursor
	pop	ds
	ret
del_line endp
	assume	ds:nothing
;ds:si->message of length,text
;di = offset in video buffer
showmsg proc	far
	push	cx
	xor	cx,cx
	mov	cl,[si] 	;length of text
	inc	si
	mov	es,[bp].videoa	;segment of video buffer
	mov	ah,0Fh		;bright normal
shwmsglp:
	lodsb			;move message to status line
	stosw
	loop	shwmsglp
	mov	cx,25*160	;blank rest of status line
	sub	cx,di
	shr	cx,1
	jcxz	shwmsgxt
	mov	al,' '
	rep stosw
shwmsgxt:
	pop	cx
	ret
showmsg endp

;on entry, ds:si->title line consisting of l,text
showtit proc	far
	xor	cx,cx
	mov	cl,[si] 	;length of title
	inc	si
	mov	es,[bp].videoa	;video buffer
	xor	di,di		;es:di->1st line of screen
	mov	ah,0FH		;bright attribute
	push	cx		;save length for blanking tail end
	jcxz	shwtit10	;skip title moving if no title
shwtitlp:
	lodsb			;move title to screen
	stosw
	loop	shwtitlp
shwtit10:
	pop	ax		;recover title length
	mov	cx,80
	sub	cx,ax		;clear rest of line
	jcxz	shwtit20	;..but exit if line is full
	mov	ax,0F20h	;bright blanks
	rep stosw
shwtit20:
	ret
showtit endp

getcurs proc	far
	xor	bh,bh		;get cursor position to
	mov	ah,3		;..to restore later
	push	bp
	int	10h
	pop	bp
	test	[bp].flagb,02h	;if current cursor is off
	jz	getcur00
	dec	dh
	mov	dl,80		;adjust to be off on restore
getcur00:
	ret
getcurs endp
setcurs proc	far
	push	bp		;video calls destroy bp
	push	bx
	cmp	dl,80		;if cursor is off the screen
	jb	setcur10
	or	[bp].flagb,02h	;..shut it off
	mov	cx,4000h
	jmp short setcur15	;..then position it
setcur10:			;cursor on the screen
	and	[bp].flagb,0FDh ;flag cursor is on
	mov	cx,[bp].cursz	;replace character cursor
	test	[bp].flagb,04h
	jz	setcur15
	mov	cx,010Ch	;insert character cursor
setcur15:
	mov	ah,1
	int	10h
setcur20:
	xor	bh,bh
	mov	ah,2
	int	10h
	pop	bx
	pop	bp		;restore the frame ptr
	ret
setcurs endp
;turn video off
voff	proc	far
	cli		;no interrupts
	push	ax
	push	dx
	mov	dx,[bp].vport	;monitor's control port
	mov	ax,[bp].vstatus ;control bits for this monitor
	and	al,0F7h 	;turn off video bit
	out	dx,al
	pop	dx
	pop	ax
	ret
voff	endp
;turn video on
von	proc	far
	push	ax
	push	dx
	mov	dx,[bp].vport
	mov	ax,[bp].vstatus
	out	dx,al
	pop	dx
	pop	ax
	sti		;enable interrupts again
	ret
von	endp

;save a word list
;on entry bx -> list to be saved
	assume	ds:cryptfb
savelist proc	far
	push	ds
	mov	ax,cryptfb
	mov	ds,ax		;data addressability
	mov	lstptr,bx	;save list ptr for later
	call	mkname		;create a name for this file
	mov	dx,offset fid	;create the file
	xor	cx,cx		;file attribute
	mov	ah,3Ch		;create
	int	21h
	jc	slstxt		;file error exit
	mov	lsth,ax 	;save file handle
	mov	si,lstptr	;compute bytes to save
	les	di,dword ptr ss:[si+4] ;word index entry again
	xor	ax,ax
	mov	al,es:[di].motrln	;length of a word
	mul	es:[di].motpcnt 	;..times number of words
	div	c32768		;write in 32k chunks
	push	dx		;save remainder
	mov	bx,lsth 	;file handle
	mov	cx,ax		;number of chunks to write
	lds	dx,dword ptr ss:[si]   ;where the list is
	jcxz	slst10		;if no chunks, skip chunk writing
slstlp:
	push	cx		;save number of chunks
	mov	cx,32768	;number of bytes to write
	mov	ah,40h		;write code
	int	21h
	jc	slster		;file error, leave
	cmp	ax,cx		;if all bytes not written
	jne	slster		;..leave
	pop	cx		;retrieve number of chunks
	mov	ax,ds		;next chunk
	add	ax,2048 	;32k/16 bump paragraphs
	mov	ds,ax
	loop	slstlp
slst10:
	pop	cx		;retrieve remainder
	jcxz	slst12
	mov	ah,40h		;last write
	int	21h
	jc	slstxt		;file error
	cmp	ax,cx
	jne	slst20		;file error
slst12:
	mov	ah,3Eh		;close the file
	int	21h
	jc	slstxt		;file error
	mov	ax,cryptfb	;update the word index
	mov	ds,ax
	mov	di,lstptr
	les	di,dword ptr ss:[di+4]	;word index again
	or	es:[di].motflg,01h	;mark this list as filed
;note that above OR clears carry flag to indicate a good return
slstxt:
	pop	ds
	ret
slster:
	pop	cx		;clear the stack
	pop	cx
slst20:
	stc			;force error return
	jmp	slstxt		;..and leave
savelist endp

;read a list
;bx -> which list to read
readlist proc	far
	push	ds
	mov	ax,cryptfb
	mov	ds,ax
	mov	lstptr,bx	;save which list ptr
	call	mkname		;make a name for this file
	mov	dx,offset fid	;open the file
	mov	ax,3D00h	;..for reading
	int	21h
	jc	rdlstxt
	mov	lsth,ax 	;save the file handle
	mov	si,lstptr	;compute bytes to read
	les	di,dword ptr ss:[si+4] ;word index entry again
	xor	ax,ax
	mov	al,es:[di].motrln	;length of a word
	mul	es:[di].motpcnt 	;..times number of words
	div	c32768		;read in 32k chunks
	push	dx		;save remainder
	mov	bx,lsth 	;file handle
	mov	cx,ax		;number of chunks to read
	lds	dx,dword ptr ss:[si]   ;where the list is
	jcxz	rdlst10 	;if no chunks, skip chunk read
rdlstlp:
	push	cx		;save number of chunks
	mov	cx,32768	;number of bytes to read
	mov	ah,3Fh		;read code
	int	21h
	jc	rdlster 	 ;file error, leave
	cmp	ax,cx		;if all bytes not read
	jne	rdlster 	 ;..leave
	pop	cx		;retrieve number of chunks
	mov	ax,ds		;next chunk
	add	ax,2048 	;32k/16 bump paragraphs
	mov	ds,ax
	loop	rdlstlp
rdlst10:
	pop	cx		;retrieve remainder
	jcxz	rdlst12
	mov	ah,3Fh		;last read
	int	21h
	jc	rdlstxt 	 ;file error
	cmp	ax,cx
	jne	rdlst20 	 ;file error
rdlst12:
	mov	ah,3Eh		;close the file
	int	21h
	jc	rdlstxt 	 ;file error
rdlstxt:
	pop	ds
	ret
rdlster:
	pop	cx		;clear the stack
	pop	cx
rdlst20:
	stc			;force error return
	jmp	rdlstxt 	 ;..and leave
readlist endp

;erase all temporary files if any
droplist proc	far
	push	ds
	mov	ax,cryptfb
	mov	ds,ax
	mov	cx,[bp].motzahl ;count of cipher words indexes
	jcxz	dropxt
	les	di,dword ptr [bp].motb	;cipher word indexes
droplp:
	test	es:[di].motflg,01h	;if there is a plaintext word
	jz	drop10
	mov	[bp].word1b,di		;..tell mkname where index is
	mov	[bp].word1a,es
	lea	bx,[bp].primb
	call	mkname			;..and make filename
	mov	dx,offset fid		;..then erase the file
	mov	ah,41h			;delete file
	int	21h
drop10:
	add	di,type motndx		;next cipher word index
	loop	droplp			;do for all cipher words
	mov	[bp].word1b,0		;restore offset ptr in frame
dropxt:
	pop	ds
	ret
droplist endp

;make a filename from the list number
mkname	proc	near
	push	es
	push	di
	push	cx
	les	di,dword ptr ss:[bx+4]	;cipher word index entry
	mov	ax,es:[di].motno	;list number
	mov	si,offset fid+2 	;destination for ascii
	call far ptr int2asc		;convert to ascii
	mov	cx,4		;replace leading blanks with 0s
mknmlp:
	cmp byte ptr [si],' '
	jne	mknmxt
	mov byte ptr [si],'0'
	inc	si
	loop	mknmlp
mknmxt:
	pop	cx
	pop	di
	pop	es
	ret
mkname	endp
	assume	ds:nothing

;on entry, ax=number to be converted
;ds:si->5 byte area to receive ascii
	assume	es:cryptfb
int2asc proc	far
	push	es
	push	dx
	push	bx
	push	cx
	mov	dx,cryptfb
	mov	es,dx
	add	si,4
	xor	bx,bx
intasclp:
	xor	dx,dx
	div	c10
	add	dl,'0'
	mov	[si],dl
	dec	si
	inc	bx
	cmp	ax,0
	ja	intasclp
	mov	cx,5
	sub	cx,bx
	jcxz	intascxt
	mov	al,' '
intasc10:
	mov	[si],al
	dec	si
	loop	intasc10
intascxt:
	inc	si
	pop	cx
	pop	bx
	pop	dx
	pop	es
	ret
int2asc endp

;on entry ds:si->ascii string of numbers
;	  cx = number of bytes
;on return, ax = binary number
;     no carry if < 65536
;     carry if >= 65536, and ax is invalid
asc2int proc	far
	push	es
	push	dx
	mov	ax,cryptfb
	mov	es,ax
	xor	ax,ax
a2ilp:
	mul	c10
	cmp	dx,0		;check for overflow
	jne	a2i10
	mov	dl,[si]
	sub	dl,'0'
	add	ax,dx
	jc	a2i10		;overflow
	inc	si
	loop	a2ilp
	clc
a2ixt:
	pop	dx
	pop	es
	ret
a2i10:
	stc
	jmp	a2ixt
asc2int endp
	assume	es:nothing
cryptf	ends
	end
