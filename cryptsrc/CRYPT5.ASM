	page	53,132
	include frame.str
	include crypt5b.asm
	extrn	showmsg:far,getcurs:far,setcurs:far,help:far
	extrn	simple:far,rail:far,xpose:far,bifid:far
crypt5	segment 'crya'
	assume	cs:crypt5,ds:crypt5b
	public	menu,hline,box,vline
menu	proc	far
	push	ds
	mov	ax,crypt5b	;addressability
	mov	ds,ax
	call	getcurs 	;get cursor position
	mov	oldpos,dx	;..to restore later
	mov	ah,13		;row 13
	mov	al,1		;col 1
	mov	dh,80		;length
	call far ptr hline	;draw a horizontal line
	call	drawbox 	;draw a box for menu
	mov	si,offset panelid ;say choices
	mov	di,24*160+32
	call	showmsg
	mov	choicen,offset choicea
	call	display
	mov	dh,13		;set cursor to first choice
	mov	dl,41
menu10:
	call	setcurs
	call	brite
forever:
	mov	ah,0
	int	16h		;wait for input
	push	ax
	mov	si,offset panelid ;say choices
	mov	di,24*160+32
	call	showmsg
	pop	ax
	cmp	al,0Dh		;check for enter
	je	enter
	cmp	al,0		;check for extended
	jne	forever
	mov	al,ah
	mov	bx,offset scantab ;classify the scan code
	xlat	scantab
	xor	bh,bh		;index into branch table
	mov	bl,al
	shl	bx,1
	jmp	fanout2[bx]
	page
curup:
	cmp	dh,13		;scroll up beyond choices?
	ja	curup10
	mov	ax,choicen	;already at top choice?
	cmp	ax,offset choicea
	ja	curup20
	jmp	forever 	;can't scroll up past 1st choice
curup10:		;move cursor to previous choice
	call	nobrite
	dec	dh
	jmp	menu10
curup20:		;scroll window down 1 choice
	sub	ax,4	;one choice up
	mov	choicen,ax
	call	display
	call	brite
	jmp	forever
curdn:
	mov	ax,offset choicen	;define down's limit
	sub	ax,choicen	;bottommost-current top
	shr	ax,1
	shr	ax,1		;number of choices
	add	ax,12		;bottommost row
	cmp	ax,22		;if bottommost row is off the screen
	jbe	curdn00
	mov	al,22		;..use screen bottom instead
curdn00:
	cmp	dh,al		;if cursor can be moved down
	jb	curdn20 	;..do it
	mov	ax,offset choicen
	sub	ax,choicen	;else scroll screen up
	shr	ax,1		;..if more than 1 screen's worth of
	shr	ax,1		;..choices remain
	cmp	ax,10
	ja	curdn10
	jmp	forever 	;out of choices
curdn10:
	mov	ax,choicen	;scroll up one choice
	add	ax,4
	mov	choicen,ax
	call	display
	call	brite
	jmp	forever
curdn20:
	call	nobrite
	inc	dh		;next choice
	jmp	menu10
enter:
	xor	bx,bx
	mov	bl,dh
	sub	bl,13
	mov	ax,choicen
	sub	ax,offset choicea
	shr	ax,1
	shr	ax,1
	add	bx,ax
	cmp	bx,5		;just five choices for now
	jb	enter10
	mov	si,offset msg2	;say not available yet
	mov	di,24*160+80
	call	showmsg
	jmp	forever
enter10:
	call	cleanup 	;remove the menu
	cmp	bx,3		;if below a bifid
	jb	enter20 	;..go to routine
	mov	ax,5		;root if 5x5 bifid
	je	enter20 	;..it is.
	dec	bx		;only one function for two bifids
	mov	ax,6		;root if 6x6 bifid
enter20:
	shl	bx,1
	shl	bx,1
	push	dx
	call	choisi[bx]
	call	drawbox 	;redraw the menu box
	pop	dx
	mov	si,offset panelid ;say choices
	mov	di,24*160+32
	call	showmsg
	call	display
	call	brite
	jmp	forever

;help wanted
sf1:
	mov	ax,2		;help screen wanted
	call	help
	jmp	forever
pagup:
pagdn:
bottom:
top:
null:
backtab:
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
f1:
f3:
f4:
f5:
f6:
f7:
f8:
f9:
f10:
home:
curleft:
curite:
endkey:
insert:
delete:
sf2:
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
prewind:
nxtwind:
botpg:
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
toppg:
	jmp	forever
f2:				;exit
	call	nobrite
	mov	dx,oldpos
	call	setcurs
	pop	ds
	ret
menu	endp
nobrite proc	near
	call	videoptr
	mov	al,07h
	mov	cx,23
nobritlp:
	inc	di
	stosb
	loop	nobritlp
	ret
nobrite endp
brite	proc	near
	call	videoptr
	mov	al,0Fh
	mov	cx,23
britelp:
	inc	di
	stosb
	loop	britelp
	ret
brite	endp
videoptr proc	near
	xor	ax,ax
	mov	al,dh
	mul	c160
	mov	di,ax
	xor	ax,ax
	mov	al,dl
	shl	ax,1
	add	di,ax
	mov	ax,[bp].videoa
	mov	es,ax
	ret
videoptr endp
display proc	near
	mov	bx,choicen	;offset to current top choice
	mov	cx,offset choicen ;number of choices
	sub	cx,bx
	shr	cx,1
	shr	cx,1
	cmp	cx,10
	jbe	disp05
	mov	cx,10
disp05:
	push	cx
	mov	ax,[bp].videoa	;video buffer segment
	mov	es,ax
	mov	di,13*160+41*2	;row 14 column 42
disp10:
	push	cx
	push	di
	mov	si,[bx] 	;ds:si->text for choice
	mov	cx,[bx+2]	;cx=length of choice
	mov	ah,07h		;normal attribute
disp20:
	lodsb
	stosw
	loop	disp20
	mov	cx,23		;max length of text
	sub	cx,[bx+2]	;blank out rest of box
	jcxz	disp30
	mov	al,' '
	rep stosw
disp30:
	pop	di
	pop	cx
	add	bx,4		;next index
	add	di,160		;next line in window
	loop	disp10
	mov	cx,10
	pop	ax
	sub	cx,ax
	jcxz	dispxit 	;full window
	mov	ax,0720h	;blank out rest of window
disp40:
	push	cx
	push	di
	rep stosw
	pop	di
	pop	cx
	add	di,160
	loop	disp40
dispxit:
	ret
display endp
drawbox proc	near
	mov	ah,13		;row
	mov	al,41		;column
	mov	dh,25		;length
	mov	dl,12		;depth
	call far ptr box	;draw a box
	ret
drawbox endp
;ah,al = origin = row,col
;dh,dl = length,depth, for example 25x12
box	proc	far
	push	bx
	push	ds
	mov	bx,crypt5b	;local addressability
	mov	ds,bx
	mov	bx,ax		;save origin
	dec	bh
	dec	bl		;origin 0
	dec	dh		;left corner
	dec	dh		;right corner
	dec	dl		;top line
	dec	dl		;bottom line
	les	di,dword ptr [bp].videob
	mov	al,bh
	mul	c160
	mov	di,ax
	xor	ax,ax
	mov	al,bl
	shl	ax,1
	add	di,ax		;es:di -> upper left corner
	push	di
	mov	al,201		;upper left corner
	mov	ah,07h		;normal attribute
	stosw
	xor	cx,cx
	mov	cl,dh
	mov	al,205		;horizontal line
	rep stosw
	mov	al,187		;upper right corner
	stosw
	pop	di
	mov	cl,dl
	add	di,160
boxlp:
	push	di
	push	cx
	mov	al,186		;vertical line
	stosw
	mov	al,' '
	mov	cl,dh
	rep stosw
	mov	al,186
	stosw
	pop	cx
	pop	di
	add	di,160
	loop	boxlp
	mov	al,200		;lower left corner
	stosw
	mov	al,205		;horizontal line
	mov	cl,dh
	rep stosw
	mov	al,188		;lower right corner
	stosw
	pop	ds
	pop	bx
	ret
box	endp
;ah=row
;al=col
;dh=length
hline	proc	far
	push	ds
	dec	ah
	dec	al
	push	dx
	mov	dx,ax
	mov	ax,crypt5b
	mov	ds,ax		;local addressability
	call	videoptr
	pop	dx		;es:di->video buffer
	mov	al,205		;horizontal line character
	mov	ah,07h		;normal attribute
	xor	cx,cx
	mov	cl,dh		;length
	rep stosw		;draw horizontal line
	pop	ds
	ret
hline	endp
;ah=row
;al=column
;dh=length
vline	proc	far
	push	ds
	dec	ah
	dec	al
	push	dx
	mov	dx,ax		;save origin
	mov	ax,crypt5b
	mov	ds,ax
	call	videoptr
	pop	dx
	mov	al,203		;top tee
	mov	ah,07h		;normal attribute
	mov	es:[di],ax
	xor	cx,cx
	mov	cl,dh		;length
	dec	cx
	dec	cx
	mov	al,186		;vertical line
	add	di,160
vlinlp:
	mov	es:[di],ax
	add	di,160
	loop	vlinlp
	mov	al,202		;bottom tee
	mov	es:[di],ax
	pop	ds
	ret
vline	endp
cleanup proc	near
	push	bx
	mov	ax,[bp].videoa
	mov	es,ax
	mov	di,12*160+80
	mov byte ptr es:[di],205 ;remove upper left corner
	mov byte ptr es:[di+48],205 ;and upper right corner
	add	di,160
	mov	bx,di
	mov	ax,0720h	;remove the choices
	mov	cx,11		;depth of box - 1
cleanlp:
	push	cx
	mov	cx,25		;width of box
	rep stosw
	pop	cx
	add	bx,160
	mov	di,bx
	loop	cleanlp
	pop	bx
	ret
cleanup endp
crypt5	ends
	end
