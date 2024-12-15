	page	53,132
	.8087
	include frame.str
	include crypt1b.asm
	extrn	getmem:far,display:far,showmsg:far,setcurs:far,cry_char:far
	extrn	rub_out:far,del_char:far,get_line:far,cmd_toggle:far
	extrn	cmd_ex:far,clear_line:far,del_line:far,showtit:far
	extrn	menu:far,help:far,int2asc:far,param:far,getmono:far
crypt1	segment 'crya'
	assume	cs:crypt1,ds:crypt1b
	public	editor,savefile
editor	proc	far
	push	ds
	mov	ax,crypt1b
	mov	ds,ax
;anchor editor data segment
	mov	[bp].edseg,ds
	mov	[bp].fspec,offset gramid
	mov	[bp].gramname,offset gramid2
	mov	[bp].hdrptr,offset titre
	mov	[bp].langspec,offset language
	mov	[bp].langid,offset langname
;anchor the cipher alphabet
	mov	[bp].alfa,ax
	mov	[bp].alfb,offset alphac
;anchor the probabilities
	mov	[bp].proba,ax
	mov	[bp].probb,offset prob
;start the cryptogram buffer
	mov	ax,[bp].freeha
	mov	[bp].sbuffa,ax
;set line length
	mov	[bp].c81,81
;retrieve DOS command line parameters
	or	[bp].flagb,20h	;no screen yet, use DOS for error msgs
	call	param
	call	getmono
	and	[bp].flagb,0DFh ;put messages to screen for now on
edit02:
	mov	posgrm,0100h
	mov	poshdr,0
;if cryptogram file exists, open and get it
	test	[bp].flaga,01h	;if cryptogram file was specified
	jz	edit06
	mov	ax,3D02h	;open as read/write file
	mov	dx,offset gramid
	int	21h
	jc	edit04
	mov	[bp].grm_handle,ax
	call	get_gram	;read file into storage
	jnc	edit10		;..read ok
	jmp	exit
edit04:
	or	[bp].flaga,02h	;new file request
edit06:
;set one insert line (blanks) in screen storage
	mov	cx,81		;get 81 bytes for a blank line
	call	getmem
	jnc	edit07
	jmp	nomem
edit07:
	xor	al,al		;length 0
	stosb			;..to buffer
	mov	al,' '          ;blank
	mov	cx,80		;..80 of them
	rep stosb		;..to buffer
	inc	[bp].lines	;count the lines
edit10:
;display the title line
	mov	si,offset titre
	call	showtit
;display the screen
	mov	[bp].current,0	;set the current line
	les	di,dword ptr [bp].sbuffb
	mov	[bp].gramb,di	;current cryptogram input line ptr
	mov	[bp].grama,es
	call	display 	;display the cryptogram
;clear the message line
	mov	es,[bp].videoa
	mov	di,statline
	mov	ax,0720h
	mov	cx,80
	rep stosw
;display the panel function
	mov	si,offset panelid ;say edit
	mov	di,24*160+32
	call	showmsg
;display the fileid		assumes es-> video buffer
	test	[bp].flaga,01h
	jz	edit19		;no file id
	mov	si,offset gramid2+1
	mov	di,statline
	mov	ah,normal
	xor	cx,cx
	mov	cl,gramid2
edit18:
	lodsb
	stosw
	loop	edit18
edit19:
	test	[bp].flaga,02h	;say if it's a new cryptogram
	jz	edit30
	mov	si,offset msg1	;new cryptogram msg
	mov	di,24*160+42
	call	showmsg
edit30:
	mov	si,offset msg2	;announce title entry
	mov	di,24*160+80
	call	showmsg
;set the cursor for title entry
	mov	dx,poshdr	;title line cursor, normal blinking
	call	setcurs
;flag quit is ok, editable data not touched
	or	flag1,40h	;quit ok
	or	[bp].modea,04h	     ;title entry mode
;do forever on keystroke input
forever:
	call	platz		;display cursor position if possible
	mov	ah,0
	int	16h		;wait for input
	mov	savechar,ax	;save keyboard input
	mov	si,offset panelid
	mov	di,24*160+32	;clear messages and reidentify function
	call	showmsg
	mov	ax,savechar	;restore keyboard input
	mov	bx,offset keytab ;classify the keystroke
	xlat	keytab
	xor	bh,bh
	mov	bl,al
	shl	bl,1		;index into branch table
	jmp	fanout1[bx]
extended:	;examine the scan code
	mov	al,ah
	mov	bx,offset scantab ;classify the scan code
	xlat	scantab
	xor	bh,bh		;index into branch table
	mov	bl,al
	shl	bx,1
	jmp	fanout2[bx]
	page
f2:		;quit
	test	flag1,40h	;if cryptogram altered
	jz	quitno		;..only forced quit is good
sf2:		;forced quit
	mov	ax,0600h	;clear the screen
	mov	bh,07h		;normal attribute
	xor	cx,cx		;upper left corner row 0, col 0
	mov	dx,184Fh	;lower right corner, row 24, col 79
	push	bp		;video calls destroy bp
	int	10h
	pop	bp
	jmp	exit
quitno:
	mov	si,offset err5	;say you can't just quit
	mov	di,24*160+80
	call	showmsg 	;..because you'll lose your changes
	jmp	forever

;file
f3:
	test	[bp].flaga,01h	;if a file is specified
	jnz	f3c		;..open it
	mov	si,offset err10 ;say no file name, can't file
	jmp short f3d
f3c:
	call	open	;if new file, create it
	jnc	f3b
f3a:
	mov	si,offset err8	;file error
f3d:
	mov	di,24*160+80
	call	showmsg
	jmp	forever
f3b:
	call	file	;write out the file
	jnc	sf2	;..if ok, clear screen and exit
	jmp	forever ;..else await more input
;save file
f4:
	call far ptr savefile
	jmp	forever

;toggle command mode
escape: 	;flip flop cursors
	push	ds
	pop	es
	mov	di,offset command
	mov	dx,poscmd
	call	cmd_toggle	;set or remove command line
escret:
	call	setcurs 	;set or restore cursor
	jmp	forever 	;get next keystroke

;toggle title mode
f8:
	test	[bp].modea,01h	;don't toggle to title if
	jz	hdr00		;..in command mode
	jmp	forever
hdr00:
	test	[bp].modea,04h	;if in title mode
	jnz	hdr10		;..go to crypt mode
	and	[bp].modea,0FDh ;else in crypt mode
	or	[bp].modea,04h	;..switch to title mode
	mov	si,offset msg2	;title entry msg
	mov	di,24*160+80	;..and announce it
	call	showmsg
	mov	dx,poshdr
	call	setcurs
	jmp	forever
hdr10:
	and	[bp].modea,0FBh ;off title mode
	or	[bp].modea,02h	;..switch to crypt mode
	mov	si,offset msg3	;..and announce crypt entry mode
	mov	di,24*160+80
	call	showmsg
	mov	dx,posgrm
	call	setcurs
	jmp	forever
	page
;character input
null:		;convert to null
	mov	savechar,0
punct:		;mark or punctuation entered
useasis:	;uninteresting character
numeral:	;numeral entered
euruc:		;european upper case alphabetic
eurlc:		;european lower case alphabetic
blank:		;blank entered
	call	testinp ;does not return if input not allowed
;al = character to put on the screen.
;dx = current cursor position
;es:di->current line or command line
asis00:
	test	[bp].modea,01h	;command mode
	jnz	asis02		;..doesn't mean can't quit
;easy quit not ok if title or crypt is changed
	and	flag1,0BFh	;quit not ok, must use qquit
asis02:
	call	cry_char	;put and display cryptogram character
	jc	charerr 	;explain error if any
charret:
	test	[bp].modea,01h	;if command mode
	jnz	showcmd 	;..display command
	test	[bp].modea,04h	;if title mode
	jnz	showhdr 	;..display new title
charretb:
	call	display 	;display updated cryptogram
	mov	posgrm,dx	;update cursor's location
	call	setcurs
	jmp	forever 	;get next keystroke
charerr:
	mov	si,offset err6	;assume cryptogram too long
	cmp	bx,0
	je	charerr02
	mov	si,offset err7	;out of memory
charerr02:
	mov	di,24*160+80
	call	showmsg
	jmp	forever

enguc:		;english upper case alphabetic
	call	testinp ;does not return if input not allowed
	jmp	asis00
englc:		;english lower case alphabetic
	call	testinp ;does not return if input not allowed
	test	[bp].modea,05h	     ;if in hdr or command mode, leave alone
	jnz	asis00
	test	[bp].modea,02h	     ;if in crypt mode, maybe translate
	jz	asis00
	test	flag1,04h	;if we should fold crypt chars
	jz	asis00
	and	al,0DFh 	;make it upper case
	jmp	asis00
showcmd:			;update command display
	les	di,dword ptr [bp].videob
	mov	di,12*160
	mov	si,offset command+1
	mov	cx,80
	mov	ah,70h		;reverse video
shwcmdlp:
	lodsb
	stosw
	loop	shwcmdlp
	mov	poscmd,dx	;update command cursor location
	call	setcurs
	jmp	forever
showhdr:			;show updated title
	mov	si,offset titre
	call	showtit
	mov	poshdr,dx	;new cursor position
	call	setcurs
	jmp	forever
	page
;cursor control
;move cursor left
curleft:
	test	[bp].modea,01h	;if command mode
	jnz	cmdleft 	;..use command cursor
	test	[bp].modea,04h	;if title mode
	jnz	hdrleft 	;..use title cursor
	cmp	colgrm,0	;if already at 0
	je	curxit		;..ignore cursor movement
	dec	colgrm
curgram:
	mov	dx,posgrm
	call	setcurs
curxit:
	jmp	forever
cmdleft:
	cmp	colcmd,0
	je	curcmdxt
	dec	colcmd
curcmd:
	mov	dx,poscmd
	call	setcurs
curcmdxt:
	jmp	forever
hdrleft:
	cmp	colhdr,0
	je	curcmdxt
	dec	colhdr
curhdr:
	mov	dx,poshdr
	call	setcurs
	jmp	forever
;move cursor right
curite:
	test	[bp].modea,01h
	jnz	cmdrite
	test	[bp].modea,04h
	jnz	hdrrite
	cmp	colgrm,79	;if already in last column
	jnb	curxit		;..ignore cursor movement
	inc	colgrm
	jmp	curgram
cmdrite:
	cmp	colcmd,79
	jnb	curcmdxt
	inc	colcmd
	jmp	curcmd
hdrrite:
	cmp	colhdr,79
	jnb	curcmdxt
	inc	colhdr
	jmp	curhdr
;move cursor up to previous row
curup:
	test	[bp].modea,7Dh	;we're not ready for anything but crypt
	jnz	curxit
	mov	ax,[bp].gramb	;don't move cursor above 1st line
	sub	ax,81
	jb	curxit
	mov	[bp].gramb,ax	;previous line now current line
	cmp	rowgrm,1	;if row is at top of screen
	ja	curup05
	dec	[bp].current	;..scroll the screen
	call	display 	;redisplay from new current line
	jmp	forever 	;..and leave cursor just where it is
curup05:
	dec	rowgrm
	jmp	curgram

;move cursor down to next row
curdn:
	test	[bp].modea,7Dh	;we're not ready for anything but crypt
	jz	crcur
	jmp	curxit
crcur:
	xor	dx,dx
	mov	ax,[bp].gramb	;don't move cursor below bottom line
	div	[bp].c81
	inc	ax
	cmp	ax,[bp].lines
	jb	curdn03
	jmp	curgram 	;redisplay cursor anyhow
curdn03:
	mov	ax,[bp].gramb
	add	ax,81
	mov	[bp].gramb,ax	;new current line
	cmp	rowgrm,23	;if row is at bottom of screen
	jb	curdn05
	inc	[bp].current	;..scroll the screen
	call	display 	;..redisplay from new current line
	jmp	curgram 	;..and redisplay cursor
curdn05:
	inc	rowgrm
	jmp	curgram

;move to start of line
home:
	call	testinp
	xor	dl,dl
home10:
	call	setcurs
	test	[bp].modea,01h	;if command
	jnz	home20		;..update command cursor position
	test	[bp].modea,04h	;if title
	jnz	home30		;..update title cursor position
	mov	posgrm,dx	;else update cryptogram cursor position
	jmp	forever
home20:
	mov	poscmd,dx
	jmp	forever
home30:
	mov	poshdr,dx
	jmp	forever
;move to end of line
endkey:
	call	testinp
	mov	dl,es:[di]
	jmp	home10

;toggle insert/replace mode
insert:
	call	testinp ;no return if input not allowed
	xor	[bp].flagb,04h	;toggle insert character flag
	call	setcurs 	;toggle the cursor
	jmp	forever

rubout: 	;backspace and squeeze out character
	call	testinp
	test	[bp].modea,05h	;command mode doesn't mean altered data
	jnz	rubout10
	and	flag1,0BFh	;quit not ok, must use qquit
rubout10:
	call	rub_out 	;shift right, rubbing out previous char
	jmp	charret 	;..then redisplay gram or command

;add a line
f10:
	test	[bp].modea,05h	;if in command or title mode
	jnz	f10xit		;..ignore add line
	call	get_line	;add new line after current line
	jnc	f10aa
	jmp	charerr 	;getline failed, show message
f10aa:
	and	flag1,0BFh	;quit not ok, must use qquit
	mov	dx,posgrm	;update the cursor
	xor	dl,dl		;..column 1
	cmp	dh,23
	jnb	f10ad
	inc	dh		;next row
f10ad:
	jmp	charretb
f10xit:
	jmp	forever

;carriage return
;if command mode, execute the command
;if cryptogram edit mode, advance to next line
endline:	;carriage return
	test	[bp].modea,01h	;if not command mode
	jz	editcr		;..try edit mode
	mov	di,offset command
	push	ds
	pop	es	;es:di->command line
	call	cmd_ex	;execute command
;will eventually need a jump table return I think
	jnc	endl10	;no command error
	jmp	forever ;command error, do nothing but await more input
endl10:
	mov	colcmd,0 ;reset command cursor
	shl	bx,1	;branch to return point
	jmp	cmdret[bx]

editcr:
	test	[bp].modea,02h	;if not edit mode
	jz	titcr		;..try title mode
	mov	colgrm,0	;column 0
	jmp	crcur		;advance to next line if any
titcr:
	test	[bp].modea,04h	;if not title mode
	jz	subcr		;..give up
	jmp	hdr10		;else switch to crypt entry
subcr:
	jmp	forever

;clear current line
f5:
	call	testinp
	xor	dl,dl
f5aa:
	call	clear_line
	test	[bp].modea,05h
	jnz	f5ab
	and	flag1,0BFh	;quit not ok, must use qquit
f5ab:
	jmp	charret
;delete chars from current position to end of line
f6:
	call	testinp
	jmp	f5aa

;delete current line
del:		;delete line
	test	[bp].modea,05h	;don't allow for command or sub
	jz	del00
	jmp	forever
del00:				;..only for cryptogram edit
	mov	dx,posgrm
	call	del_line
	and	flag1,0BFh	;quit not ok, must use qquit
	jmp	charretb

;delete char at cursor position
delete: 		;delete character at cursor position
	call	testinp 	;get current line and cursor
	call	del_char
	test	[bp].modea,05h
	jnz	delt05
	and	flag1,0BFh	;quit not ok, must use qquit
delt05:
	jmp	charret

top:
	test	[bp].modea,05h	;if not cryptogram edit
	jz	top02
	jmp	forever 	;..don't go to top
top02:
	mov	[bp].current,0
	mov	[bp].gramb,0
	mov	dx,posgrm
	mov	dh,1
top10:
	mov	posgrm,dx
	call	display
	call	setcurs
	jmp	forever

bottom:
	test	[bp].modea,05h	;if not cryptogram edit
	jz	bot02
	jmp	forever 	;..don't go to bottom
bot02:
	mov	ax,[bp].lines	;set current line to last
	dec	ax
	push	ax
	mul	[bp].c81
	mov	[bp].gramb,ax
	pop	ax		;set the screen top
	mov	dx,posgrm	;..and cursor
	cmp	ax,23		;if less than 24 lines
	jnb	bot10		;..leave screen top where it is
	mov	dh,al		;new cursor row at last line on screen
	inc	dh
	jmp	top10
bot10:
	sub	ax,22		;new screen top
	mov	[bp].current,ax
	mov	dh,23		;new cursor row at bottom
	jmp	top10

;scroll up
pagup:
	test	[bp].modea,05h	;if not cryptogram edit
	jz	pagup02
	jmp	forever 	;..don't scroll up for now
pagup02:
	cmp	[bp].current,0	;if already at top of file
	jne	pagup04
	jmp	forever 	;..don't scroll up
pagup04:
	mov	ax,[bp].current
	sub	ax,22		;back up 22 lines
	jnb	pagup10 	;and if there aren't 22 lines
	xor	ax,ax		;set to 1st
pagup10:
	mov	[bp].current,ax ;new screen top
	xor	bx,bx		;compute new current line at cursor row
	mov	bl,rowgrm
	add	ax,bx		;..from screen top
	dec	ax
	mul	[bp].c81
	mov	[bp].gramb,ax
	mov	dx,posgrm
	jmp	top10
;scroll down
pagdn:
	test	[bp].modea,05h	;if not cryptogram edit
	jz	pagdn02
	jmp	forever 	;..don't scroll down for now
pagdn02:
	mov	ax,[bp].current
	add	ax,22+23	;check if full scroll is possible
	cmp	ax,[bp].lines
	jb	pagdn04
	mov	ax,[bp].lines	;..not a full screen left if full scroll
pagdn04:
	sub	ax,23
	jnb	pagdn06
	jmp	forever 	;less than a full screen of lines
pagdn06:
	mov	[bp].current,ax
	xor	bx,bx		;compute new current line
	mov	bl,rowgrm
	add	ax,bx		;screen top + cursor row
	dec	ax
	mul	[bp].c81
	mov	[bp].gramb,ax
	mov	dx,posgrm
	jmp	top10
	page
;display menu
f1:
	test	[bp].modea,01h	;ignore if command mode
	jnz	menu00
	call	menu
	mov	si,offset panelid
	mov	di,24*160+32
	call	showmsg
	call	display
menu00:
	jmp	forever

;help for edit
sf1:
	mov	ax,1		;help screen wanted
	call	help
	jmp	forever
tab:		;tab to next word
backtab:
prewind:
nxtwind:
botpg:
toppg:
a1:
a2:
a3:
a4:
a5:
a6:
a7:
a8:
a9:
a0:
adash:
aequal:
altq:
altw:
alte:
altr:
altt:
alty:
altu:
alti:
alto:
altp:
alta:
alts:
altd:
altf:
altg:
alth:
altj:
altk:
altl:
altz:
altx:
altc:
altv:
altb:
altn:
altm:
f7:
f9:
sf3:
sf4:
sf5:
sf6:
sf7:
sf8:
sf9:
sf10:
cf1:
cf2:
cf3:
cf4:
cf5:
cf6:
cf7:
cf8:
cf9:
cf10:
af1:
af2:
af3:
af4:
af5:
af6:
af7:
af8:
af9:
af10:
	jmp	forever
nomem:
exit:
	pop	ds
	ret
editor	endp

;read the alphabet and set it
;read the key and set it
;read the cryptogram lines into screen storage
get_gram proc	near
	mov	bx,[bp].grm_handle
	mov	dx,offset rdbuf  ;get the alphabet length
	mov	cx,2
	call	read		;leaves proc on error
	mov	ax,word ptr rdbuf
	cmp	ax,26		;old style crypt or new style with title?
	je	get05		;..old style without title
	mov word ptr titre,ax	;length and 1 byte of text of title
	mov	cx,79		;read the rest of the title
	mov	dx,offset titre+2
	call	read
	mov	cx,2		;now get alphabet length
	mov	dx,offset alphac
	call	read
	jmp short get07
get05:
	mov	alphac,ax	;alphabet length
get07:
	mov	cx,alphac
	mov	dx,offset alphac+2
	call	read		;get cipher alphabet
	mov	dx,offset alphap ;get plaintext alphabet
	call	read
	mov	dx,offset alphas
	call	read		;get solved letters
	mov	dx,offset alphai
	call	read		;get inverse solved letters
	mov	dx,offset rdbuf ;file buffer
	mov	cx,2
	call	read		;get number of cryptogram lines
	mov	si,dx
	mov	cx,[si]
getlp:
	call	getl		;get a cryptogram line
	jnc	get25
	mov	dx,offset derr1
get23:
	mov	ax,crypt1b	;recover my addressability
	mov	ds,ax
	mov	ah,9h
	int	21h
	stc
	ret			;return and quit
get25:			;ds:si->length,characters length may be 0
	cmp	[bp].lines,809
	jb	get30
	mov	dx,offset merr1
	jmp	get23
get30:
	push	cx
	mov	cx,81		;get space for line
	call	getmem
	pop	cx
	jnc	get35
;no memory error
	mov	dx,offset merr2
	jmp	get23
get35:
	call	movl		;move line from buffer to screen
	inc	[bp].lines
	loop	getlp		;for all lines
	mov	ax,4200h	;reset file
	xor	cx,cx
	xor	dx,dx
	int	21h
	clc			;good return
	ret
get_gram endp
getl	proc	near
	push	cx
	push	dx
	mov	cx,1		;get line length
	mov	ah,3Fh
	int	21h
	jc	getlxit
	cmp	ax,cx
	je	getl05
	stc
	jmp short getlxit
getl05:
	xor	cx,cx
	mov	si,dx
	mov	cl,[si]
	jcxz	getlxit 	;line of length 0
	inc	dx
	mov	ah,3Fh
	int	21h		;get line
	jc	getlxit
	cmp	ax,cx
	je	getlxit
	stc
getlxit:
	pop	dx
	pop	cx
	ret
getl	endp
;ds:si -> cryptogram line in file buffer
;es:di -> destination in screen buffer
movl	proc	near
	push	cx
	xor	cx,cx
	lodsb			;get length
	stosb			;..and move it
	mov	cl,al
	push	cx
	jcxz	movl05		;empty line
	rep movsb		;get character
movl05:
	pop	ax		;recover line length
	mov	cx,80		;max length of line
	sub	cx,ax		;length to pad with blanks
	jz	movlxit 	;full line
	mov	al,' '          ;pad with blanks
	rep  stosb		;pad with blanks
movlxit:
	pop	cx
	ret
movl	endp
read	proc	near
	mov	ah,3Fh
	int	21h
	jc	reader
	cmp	ax,cx
	jne	reader
	ret
reader:
	mov	dx,offset derr1
	mov	ah,9h
	int	21h
	stc
	pop	ax		;pop one return
	ret			;return to caller's caller
read	endp

savefile proc	far
	push	ds
	mov	ax,crypt1b
	mov	ds,ax
	test	[bp].flaga,01h	     ;if file is specified
	jnz	savef01
	mov	si,offset err10 ;no file name
savefer:
	mov	di,24*160+80
	call	showmsg
	stc
savefer2:
	pop	ds
	ret
savef01:
	call	open		;if new file, create it
	jnc	savef10
	mov	si,offset err8	;file error
	jmp	savefer
savef10:
	call	file	;write out the file
	jc	savefer2
	mov	ah,3Eh		;close the file to flush buffers
	int	21h
	mov	dx,offset gramid
	mov	ax,3D02h	;reopen file read/write
	int	21h
	mov	[bp].grm_handle,ax
	or	flag1,40h	;quit now ok
	clc
	pop	ds		;restore addressability
	ret
savefile endp

;create file if new
open	proc	near
	test	[bp].flaga,02h	     ;if new file
	jnz	open00		;..create it
	clc			;good return
openxit:
	ret
open00:
	xor	cx,cx		;ordinary file
	mov	dx,offset gramid ;file name
	mov	ah,3Ch		;create
	int	21h
	jc	openxit 	;..failed
	and	[bp].flaga,0FDh      ;file created
	mov	[bp].grm_handle,ax   ;file handle saved
	ret
open	endp

file	proc	near
;file cryptogram title
	mov	cx,81
	mov	dx,offset titre
	mov	bx,[bp].grm_handle
	mov	ah,40h
	int	21h
	jc	filerr1 	;i/o error
	cmp	ax,cx
	jne	file02		;disk full
	mov	cx,alphac	;write the alphabets
	add	cx,2		;+ length field
	mov	dx,offset alphac
	mov	ah,40h		;write
	int	21h
	jnc	file00
filerr1:
	mov	si,offset err8
filerr2:
	mov	di,24*160+80
	call	showmsg
	stc			;..file error
	ret			;..failed
file00:
	cmp	ax,cx		;if disk full
	je	file05
file02:
	mov	si,offset err9	;..tell user
	jmp	filerr2
file05:
	sub	cx,2		;plaintext alphabet
	mov	dx,offset alphap
	mov	ah,40h		;write
	int	21h
	jc	filerr1
	cmp	ax,cx
	jne	file02
	mov	dx,offset alphas ;solved letters
	mov	ah,40h
	int	21h
	jc	filerr1
	cmp	ax,cx
	jne	file02
	mov	dx,offset alphai ;inverse solved letters
	mov	ah,40h
	int	21h
	jc	filerr1
	cmp	ax,cx
	jne	file02
;prepare to file the cryptogram
	mov	dx,offset rdbuf ;file buffer
	mov	ax,[bp].lines	;put number of lines
	mov word ptr rdbuf,ax
	mov	cx,2
	mov	ah,40h
	int	21h
	jc	filerr1
	cmp	ax,cx
	jne	file02

;file all the lines
	mov	cx,[bp].lines
	push	ds
	lds	si,dword ptr [bp].sbuffb
file10:
	call	put
	jnc	file15
	pop	ds
	ret
file15:
	add	si,81
	loop	file10
	mov	ax,4200h	;reset file ptr
	xor	cx,cx
	xor	dx,dx
	int	21h
	clc
	pop	ds
	ret
file	endp
;on entry
;bx = filehandle
;ax = bytes already in destination buffer
;es:di->current destination buffer location
;ds:si->current string
put	proc	near
	push	cx
	xor	cx,cx
	mov	cl,[si] 	;length of string
	inc	cl		;plus length byte
	mov	dx,si
	mov	ah,40h
	int	21h
	jc	puterr1
	cmp	ax,cx
	jne	puterr2
	pop	cx
	clc
	ret
puterr1:
	mov	ax,crypt1b	;restore addressability
	mov	ds,ax
	mov	si,offset err8
puterr:
	pop	cx		;clean the stack
	mov	di,24*160+80
	call	showmsg 	;display error message
	stc			;bad return
	ret
puterr2:
	mov	ax,crypt1b	;restore addressability
	mov	ds,ax
	mov	si,offset err9
	jmp	puterr
put	endp

;else return current input line ptr (return to caller) in es:di
;     ..and character in al, and cursor position in dx
testinp proc   near
	push	ds
	pop	es		;assume command mode
	mov	di,offset command
	mov	dx,poscmd	;..cursor position
	test	[bp].modea,01h	;test command assumption
	jnz	testin10	;..it's command mode
	mov	di,offset titre ;maybe it's title
	mov	dx,poshdr
	test	[bp].modea,04h	;test title assumption
	jnz	testin10
	les	di,dword ptr [bp].gramb ;must be crypt mode
	mov	dx,posgrm	;..cursor position
testin10:
	mov	ax,savechar	;return the character
	xor	ah,ah
	ret
testinp endp

;if there is no pending message, display cursor position in msg area
platz	proc   near
	mov	cx,11
	mov	ax,0F20h	;blank, brite
	mov	es,[bp].videoa
	mov	di,24*160+80	;check if any msg is displayed
	repe scasw		;if any character on the tube but a blank
	jne	platzxt 	;..preserve the message
	mov	si,offset msg4+1 ;destination for line number
	mov	ax,1		;line number, index origin 1
	xor	dx,dx		;column number
	mov	dl,colhdr	;assume editing title
	test	[bp].modea,04h	;test assumption
	jnz	platz10 	;..editing title
	mov	dl,colcmd	;assume editing command
	test	[bp].modea,01
	jnz	platz10 	;..editing command
;editing cryptogram
	mov	dl,colgrm	;column position
	mov	al,rowgrm	;row position
	add	ax,[bp].current ;plus current top line on screen
platz10:
	call	int2asc 	;format the line number, 1 for cmds and titles
	mov	ax,dx		;column number
	inc	ax		;index origin 1
	mov	si,offset msg4+7
	call	int2asc
	mov	si,offset msg4	;display cursor position
	mov	di,24*160+80
	call	showmsg
platzxt:
	ret
platz	endp

crypt1	ends
	end
