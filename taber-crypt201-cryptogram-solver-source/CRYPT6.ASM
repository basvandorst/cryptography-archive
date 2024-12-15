	page	53,132
	include frame.str
	include motndx.str
	include crypt6b.asm
	extrn	hline:far,vline:far,showmsg:far,getcurs:far,setcurs:far
	extrn	showstat:far,savefile:far,savefree:far,freeadj:far
	extrn	openpat:far,findpat:far,showlst:far,wordown:far,wordup:far
	extrn	get1st:far,showpat:far,showprim:far,l1up:far,l1down:far
	extrn	get2nd:far,log:far,droplist:far,help:far,rsetndx:far
	extrn	eclat:far
;	extrn	voff:far,von:far
crypt6	segment 'crya'
	assume	cs:crypt6,ds:crypt6b
	public	simple
simple	proc	far
	push	ds
	mov	ax,crypt6b
	mov	ds,ax		;addressability
	mov	flag1,0 	;clear flags
	mov	area,0
;	call	voff
	mov	si,offset oldszb
	call	savefree	;save current freespace
	call	freeadj 	;adjust freespace to segment boundary
	mov	ax,[bp].current ;save the editor's current line
	mov	oldcur,ax
	mov	ax,[bp].gramb
	mov	oldgram,ax
	mov	[bp].current,0	;..then set my own
	mov	[bp].gramb,0
	mov	[bp].motcur,0	;current word list line
	mov	[bp].primcur,0	;current primary list line
	mov	[bp].secur,0	;alternate word list line
	mov	[bp].prima,0	;primary pattern list area
	mov	[bp].seca,0	;alternate pattern list area
	mov	[bp].modeb,0	;reset filter rules
	and	[bp].flaga,47h	;off list flags, index flag
	mov	alfnow,0	;start with first character of alphabet
	mov	posnow,0	;substitution cursor position
	call	getcurs 	;get cursor position
	mov	oldpos,dx	;save for restoring
	call	openpat 	;open index and dictionary
	call	clear		;..clear screen
	call	init		;initialize the screen
	mov	di,24*160+32	;identify the panel
	mov	si,offset panelid
	call	showmsg
	test	[bp].flaga,0Ch	;if dictionary not ok
	jz	simp00
	mov	si,offset msg5	;..tell the user
	mov	di,24*160+80
	call	showmsg
	or	flag1,01h	;..and flag a msg is up
simp00:
	and	[bp].flagb,0F7h ;force counting
	call	showstat
simp10:
	call	showsub 	;show the cipher with subs
;	call	von
	mov	dx,posnow	;set cursor to start
	inc	dh		;skip title line
	call	setcurs
forever:
	mov	ah,0
	int	16h		;wait for input
	mov	savechar,ax
	test	flag1,01h	;if message displayed
	jz	simp20
	and	flag1,0FEh	;..clear it
	les	di,dword ptr [bp].videob
	mov	di,24*160+80
	mov	ax,0F20h	;bright, blank
	mov	cx,40
	rep stosw
simp20:
	mov	ax,savechar	;restore keyboard char
	mov	bx,offset keytab
	xlat	keytab
	xor	bh,bh
	mov	bl,al
	shl	bx,1
	jmp	fanout1[bx]
extended:
	mov	al,ah
	mov	bx,offset scantab ;classify the scan code
	xlat	scantab
	xor	bh,bh		;index into branch table
	mov	bl,al
	shl	bx,1
	jmp	fanout2[bx]
	page
;quit
f2:
	call	droplist	;erase all temp word files
	and	[bp].flaga,4Fh		;discard word index and lists
	les	di,dword ptr oldszb	;restore editor's free space
	mov	[bp].sizeb,di		;..size
	mov	[bp].sizea,es
	les	di,dword ptr oldfrb	;..pointer
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	mov	ax,oldcur	;restore editor's current line
	mov	[bp].current,ax
	mov	ax,oldgram
	mov	[bp].gramb,ax
	mov	dx,oldpos	;restore cursor
	call	setcurs
	pop	ds
	ret

;save file
f4:
	call	savefile	;save file
	jnc	forever 	;..if error
	or	flag1,01h	;..flag msg to be cleared
	jmp	forever

;move cursor right one letter
curite:
	cmp	area,0		;can't move horizontally
	jne	curno		;..if not in substitution work area
	mov	dx,posnow
	les	di,dword ptr [bp].gramb
	inc	dl
	cmp	dl,es:[di]	;if past last character
	jb	curite00
	jmp	endline 	;..wrap to next line
curite00:
	mov	posnow,dx	;else move cursor right
	shl	dh,1		;double the row coordinate
	inc	dh		;..and skip title area
	call	setcurs
curno:
	jmp	forever

;move cursor left one letter
curleft:
	cmp	area,0
	jne	curno
	mov	dx,posnow
	cmp	dl,0		;if at 1st character
	je	curlft10	;..wrap to previous line
	dec	dl		;else cursor left 1 position
	jmp	curite00
curlft10:
	cmp	dh,0		;if at top of screen
	je	curlft20	;..try to scroll
	dec	dh		;previous screen line
	sub	[bp].gramb,81	;previous cipher line
	les	di,dword ptr [bp].gramb
	mov	dl,es:[di]	;last letter of previous cipher line
	dec	dl		;origin 0
	jmp	curite00	;display cursor
curlft20:
	cmp	[bp].current,0	;if top of screen is first cipher line
	je	curlft30	;..wrap to end of cipher
	dec	[bp].current	;else previous line is screen top
	les	di,dword ptr [bp].gramb
	sub	di,81		;previous cipher line
	mov	dl,es:[di]	;set cursor to last letter
	dec	dl
	mov	posnow,dx
	jmp	endl30		;redisplay screen and set cursor
curlft30:
	call	wrapz		;wrap to last line
	mov	ax,[bp].lines	;figure last line on screen
	sub	ax,[bp].current
	dec	ax
	mov	dh,al		;new screen row
	les	di,dword ptr [bp].gramb
	mov	dl,es:[di]
	dec	dl		;new screen column
	mov	posnow,dx
	cmp	[bp].lines,5	;if 5 lines or less
	ja	curlft50
	shl	dh,1
	inc	dh		;skip title area
	call	setcurs 	;..we don't have to scroll
	jmp	forever
curlft50:
	jmp	endl30		;redisplay the screen and set cursor

;move cursor up
curup:
	xor	bx,bx
	mov	bl,area 	;goto cursor up for this area
	shl	bx,1
	jmp	ucurs[bx]
subup:
	mov	dx,posnow	;back cursor up one row
	dec	dh
	js	curup10 	;..unless it is off screen
	les	di,dword ptr [bp].gramb
	sub	di,81		;and back up current cipher line
	mov	[bp].gramb,di
	cmp	dl,es:[di]	;force cursor to last letter
	jb	curup00
	mov	dl,es:[di]	;..if beyond the last letter
	dec	dl
curup00:
	jmp	curite00	;set the cursor to new position
curup10:
	cmp	[bp].current,0	;if screen top is 1st cipher line
	je	curup20 	;..wrap to last line
	dec	[bp].current	;else new screen top
	les	di,dword ptr [bp].gramb
	sub	di,81		;new current cipher line
	jmp	endl30		;redisplay screen
curup20:
	call	wrapz		;wrap to end of cipher
	mov	dx,posnow
	mov	ax,[bp].lines
	sub	ax,[bp].current
	dec	ax
	mov	dh,al		;new cursor row
	les	di,dword ptr [bp].gramb
	cmp	dl,es:[di]
	jb	curup30
	mov	dl,es:[di]
	dec	dl
curup30:
	cmp	[bp].lines,5
	ja	curup40
	jmp	curite00
curup40:
	mov	posnow,dx
	jmp	endl30
wup:		;cursor up for cipher word list
	mov	dx,wrdnow	;current cipher list position
	call	wordup		;up cursor one
	mov	wrdnow,dx	;update cursor position
	jmp	forever
pup:		;cursor up for primary pattern list
	mov	dx,l1now	;primary list cursor position
	call	l1up		;up one word
	mov	l1now,dx	;update cursor position
	jmp	forever
aup:		;cursor up for alternate pattern list
	mov	dx,l2now	;secondary list cursor position
	call	l1up		;up one word
	mov	l2now,dx	;update cursor position
	jmp	forever

;move cursor down
curdn:
	xor	bx,bx
	mov	bl,area 	;goto cursor routine for this area
	shl	bx,1
	jmp	dcurs[bx]
subdn:
	mov	ax,[bp].lines	;figure lines on the screen
	sub	ax,[bp].current
	mov	cx,ax		;save total lines
	cmp	ax,5
	jna	curdn00
	mov	ax,5		;no more than 5 lines on screen
curdn00:
	mov	dx,posnow
	jmp	endl02
wdn:		;cursor down for cipher word list
	mov	dx,wrdnow	;get current cursor position for word list
	call	wordown 	;put cursor on next word
	mov	wrdnow,dx	;update cursor position
	jmp	forever
pdn:		;cursor down for primary pattern list
	mov	dx,l1now	;cursor position for primary list
	call	l1down		;next word in primary list
	mov	l1now,dx	;update cursor position
	jmp	forever 	;at end of list, no action
adn:		;cursor down for alternate pattern list
	mov	dx,l2now	;cursor position for 2nd list
	call	l1down		;next word in second list
	mov	l2now,dx	;update cursor position
	jmp	forever

;next line (carriage return)
endline:
	cmp	area,0
	je	endok
	jmp	forever
endok:
	mov	ax,[bp].lines	;figure lines on the screen
	sub	ax,[bp].current
	mov	cx,ax		;save total lines
	cmp	ax,5
	jna	endl00
	mov	ax,5		;no more than 5 lines on screen
endl00:
	mov	dx,posnow
	xor	dl,dl		;force column 0
endl02:
	inc	dh		;next line
	cmp	dh,al		;if next line is off the screen
	jnb	endl10		;try to scroll one line
	add	[bp].gramb,81	;next cipher line
	les	di,dword ptr [bp].gramb
	cmp	dl,es:[di]	;if cursor would be beyond end of line
	jb	endl04
	mov	dl,es:[di]	;..force to last character
	dec	dl
endl04:
	jmp	curite00	;cursor down to next line
endl10:
	cmp	cx,5		;if more than 5 lines total
	ja	endl20		;..we can scroll
	xor	dh,dh		;else wrap to start
	mov	posnow,dx	;reset cursor to start
	mov	[bp].gramb,0	;1st gram line is current
	les	di,dword ptr [bp].gramb
	cmp	dl,es:[di]
	jb	endl11
	mov	dl,es:[di]
	dec	dl
endl11:
	cmp	[bp].current,0	;if current screen top is 0
	jne	endl12
	inc	dh		;skip title area
	call	setcurs
	jmp	forever 	;the screen doesn't have to be refreshed
endl12:
	mov	[bp].current,0	;line 0 is screen top
;	call	voff
	jmp	simp10		;redisplay from start
endl20:
	dec	dh		;don't move cursor
	mov	posnow,dx
	inc	[bp].current	;..move the lines on the screen
	les	di,dword ptr [bp].gramb
	add	di,81
endl30:
	mov	[bp].gramb,di
	cmp	dl,es:[di]
	jb	endl32
	mov	dl,es:[di]
	dec	dl
endl32:
;	call	voff
	call	showsub
;	call	von
	mov	dx,posnow
	shl	dh,1
	inc	dh		;skip title area
	call	setcurs
	jmp	forever

;home cursor
home:
	cmp	area,0		;can't home
	jne	cant		;..if not in substitution area
	mov	dx,posnow
	xor	dl,dl
	jmp	curite00

;cursor to end of line
endkey:
	cmp	area,0		;can't endkey
	jne	cant		;..if not in substitution area
	les	di,dword ptr [bp].gramb
	mov	dx,posnow
	mov	dl,es:[di]
	dec	dl
	jmp	curite00

;tab to next word
tab:
	cmp	area,0		;can't tab
	je	tabok
cant:
	jmp	forever 	;..if not in substitution area
tabok:
	call	getpos		;get scan start
	jnc	tab00		;..and find next word
tab10:
	jmp	endline 	;line exhausted, get next line
tab00:
	mov	al,' '
	cmp byte ptr es:[di],al  ;if on a blank
	je	tab20		;..scan to 1st non-blank
	repne scasb		;else scan to 1st blank (skip this word)
	jne	tab10		;no next word, get another line
	inc	cx		;adjust line count
	dec	di		;..and line ptr
tab20:
	repe scasb		;scan to 1st non-blank (next word)
	je	tab10		;no next word, get another line
	inc	cx		;adjust line count
	dec	di		;..and line ptr
	mov	ax,di		;compute cursor position
	sub	ax,[bp].gramb
	mov	dx,posnow	;new cursor coordinates
	mov	dl,al
	dec	dl		;adjust for 0 origin
	jmp	curite00

;skip to next unsolved letter if any
blank:
	cmp	area,0
	je	skip00
	jmp	forever
skip00:
	mov	ax,posnow	;save cursor position for restoring if
	mov	savpos,ax	;..skipping can't be done
	mov	ax,[bp].gramb	;figure lines from current position
	mov	bx,ax
	xor	dx,dx
	div	[bp].c81	;..to end of screen area
	mov	cx,[bp].lines
	sub	cx,ax
	cmp	cx,5		;..no more than 6 lines on screen
	jna	skip10
	mov	cx,5
skip10:
	dec	cx		;discount the current line
	push	cx
	call	getpos		;get position in current line
	jc	skip20		;on an empty line
	call	findchar	;find next unsolved character
	jc	skip20		;..none found on this line
skip12:
	pop	cx		;clear the stack
	mov	ax,di
	sub	ax,bx
	mov byte ptr posnow,al	;new cursor position
	mov	dx,posnow
	mov	[bp].gramb,bx	;new current line
	shl	dh,1		;double the rows for cursor display
	inc	dh		;skip title area
	call	setcurs 	;show the cursor
	jmp	forever
skip20:
	pop	cx
	jcxz	skip40		;may not be another line
skip30: 	;at least one more line
	add	bx,81		;next crypt line
	mov	di,bx
	inc byte ptr posnow+1	;next cursor position
	push	cx
	mov	cl,es:[di]	;..length of crypt line
	inc	di
	call	findchar
	jnc	skip12
	pop	cx
	loop	skip30
skip40:
	mov	ax,savpos	;couldn't skip so restore cursor position
	mov	posnow,ax
	jmp	forever 	;no unsolved letter found

;character routines
useasis:
punct:
numeral:
enguc:
englc:
euruc:
eurlc:
null:
	cmp	area,0		;can't enter characters
	jne	charer10	;..if I'm not in substitution area
	mov	ax,savechar
	call	checkp		;is keyboard char valid plaintext?
	jnc	char00		;keyboard char is plaintext
	mov	si,offset msg1
charerr:
	or	flag1,01h	;flag message displayed
	mov	di,24*160+80
	call	showmsg
charer10:
	jmp	forever
char00:
	call	getpos
	jnc	char02
	mov	si,offset msg2
	jmp	charerr
char02:
	mov	al,es:[di]	;get the cipher character
	call	checkc		;if valid cipher letter
	jnc	char10		;..do the substitution
	mov	si,offset msg2
	jmp	charerr
char10: 		;undo any previous substitution
	mov	ah,byte ptr savechar	;retrieve plaintext char
	call	dosub		;do substitution
	jmp	forever

;delete character at cursor position
delete:
	cmp	area,0		;if not in substitution area
	jne	delxit		;..no cipher letter to delete
	call	getpos		;get cipher letter position
	jc	delxit		;..no cipher letter at this position
	mov	al,es:[di]	;get character, supposedly cipher letter
	call	checkc		;if not a cipher letter
	jc	delxit		;..nothing to delete
	mov	bx,[bp].alfb
	cmp byte ptr es:[di][bx+514],' ' ;if no solved letter for cipher
	je	delxit			;..letter, nothing to delete
	mov	ah,es:[di][bx+514]	;else remove plaintext letter
	call	unkey			;..from K1 and K2 areas
	call	showalf 		;..show new keys
delxit:
	jmp	forever 	;get next keystroke

;build cipher word list
altq:
	cmp	area,0		;if not in substitution work area
	jne	delxit		;..ignore request
	test	[bp].flaga,0Ch	;if no dictionary
	jz	altq10
	mov	si,offset msg5	;..can't search for patterns
	jmp	charerr 	;..tell user no dictionary
altq10:
	call	findpat 	;get the list of words
	jnc	altq20
	or	flag1,01h	;flag message to be cleared
	jmp	forever 	;..no word list for this cipher word
altq20:
	mov	dx,111Ah	;set row = 18, col = 27 for word list
	mov	wrdnow,dx
altq30:
	mov	area,1		;switch to word list area
	call	showlst 	;show list of cipher words
	jmp	forever

;reset cipher word indexes to original counts
altr:
	test	[bp].flaga,10h	;if no word index
	jz	altr10		;..ignore request
	call	rsetndx 	;else reset word lists
	mov	dx,wrdnow	;redisplay index at current position
	jmp	altq30
altr10:
	jmp	forever
	page
;get primary pattern list for the selected cipher word
altw:
	cmp	area,1		;if not in cipher list area
	je	altw00
	jmp	forever 	;..ignore request
altw00:
	mov	[bp].primcur,0	;reset screen top word
	mov	l1now,0F2Dh	;reset cursor position
	mov	dx,wrdnow	;current cursor position
	call	get1st		;..get the list
	jnc	altw10		;no messages or errors
	or	flag1,01h	;flag message to be cleared
altw10:
	lea	si,[bp].primb	;source of primary list
	mov	di,12*160+86	;screen destination, primary list
	call	showpat 	;show pattern header for primary list
	jnc	altw20		;got a list
	jmp	forever 	;no list, nothing to display
altw20:
	mov	di,15*160+86
	call	showprim
	jmp	forever

;build alternate pattern list for selected cipher word
alte:
	cmp	area,1		;if not in cipher list area
	je	alte00
	jmp	forever 	;..ignore request
alte00:
	mov	[bp].primcur,0	;reset 1st list start
	mov	[bp].secur,0	;reset 2nd list start
	mov	l1now,0F2Dh	;reset 1st list cursor position
	mov	l2now,0F40h	;reset 2nd list cursor position
	mov	dx,wrdnow	;current cursor position in cipher list
	call	get2nd		;get the second list
	jnc	alte10		;if 2nd list failed
	or	flag1,01h	;..flag message to clear
	jmp	forever
;show new first list
alte10:
	lea	si,[bp].primb	;-> 1st list
	mov	di,12*160+86	;-> screen destination
	call	showpat 	;show 1st list header
	jnc	alte20		;got a list to show
	lea	si,[bp].secb	;-> 2nd list
	mov	di,12*160+124
	call	showpat 	;show 2nd list header
	jmp	forever 	;no 1st list, and no second
alte20:
	mov	di,15*160+86
	call	showprim	;show first list words
	lea	si,[bp].secb	;-> 2nd list
	mov	di,12*160+124
	call	showpat 	;show 2nd list header
	jnc	alte30
	jmp	forever
alte30:
	mov	di,15*160+124
	call	showprim	;show 2nd list words
	jmp	forever

;try selected plaintext word in cipher
alto:
	cmp	area,2		;if not in a word list
	jnb	alto00
	jmp	forever 	;..no word to try out, ignore it
alto00:
;push the current solved state (save alphabets)
	push	ds		;save addressability
	push	ds
	pop	es
	lds	si,dword ptr [bp].alfb	;address the alphabets
	mov	di,offset savalfs
	mov	cx,[si]
	add	si,514		;ds:si->solved letters
	push	cx
	push	si
	rep movsb		;save solved letters
	pop	si
	pop	cx
	add	si,256		;ds:si->inverse solved letters
	mov	di,offset savalfi
	rep movsb		;save inverse letters
	pop	ds		;recover addressability
;assume 1st word list trial
	mov	dx,l1now	;current cursor position in list
	lea	bx,[bp].primb	;..and which list
	cmp	area,2
	je	alto10		;it is 1st list
	mov	dx,l2now	;current cursor position, 2nd list
	lea	bx,[bp].secb	;..and which list
alto10:
	call	wordsub 	;substitute this word
altoxt:
	call	showalf 	;show new temp alphabets
;pop the solved state (restore alphabets)
	les	di,dword ptr [bp].alfb
	mov	cx,es:[di]	   ;now restore the alphabets
	add	di,514		;es:di->solved letters
	mov	si,offset savalfs
	push	cx
	push	di
	rep movsb
	pop	di
	pop	cx
	add	di,256
	mov	si,offset savalfi
	rep movsb
	or	flag1,04h	;flag trial word substitution
	jmp	forever

;accept word from word list
altp:
	cmp	area,2		;if not in word list
	jnb	altp00
	jmp	forever 	;..ignore request
altp00:
;assume 1st word list
	mov	dx,l1now	;current cursor position in list
	lea	bx,[bp].primb	;..and which list
	cmp	area,2
	je	altp10
	mov	dx,l2now	;current cursor position, 2nd list
	lea	bx,[bp].secb	;..and which list
altp10:
	call	wordsub 	;substitute this word
	call	showalf 	;show new alphabets
	jmp	forever

;change work areas
prewind:
	or	flag1,02h	;switch areas backwards
	dec	area		;previous area,
prew10:
	and	area,03h	;..mod 4
	cmp	area,2		;if not in word list areas
	jnb	prew20
	test	flag1,04h	;if a trial substitution was in effect
	jz	prew20
	and	flag1,0FBh
	call	showalf 	;..restore alphabets
	call	showsub 	;..restore substitutions status quo ante
prew20:
	xor	bx,bx
	mov	bl,area
	shl	bx,1
	jmp	areasw[bx]	;goto area
nxtwind:
	and	flag1,0FDh	;switch areas forwards
	inc	area		;next area
	jmp	prew10
gosub:
	mov	dx,posnow
	shl	dh,1		;doubled for substitution area
	inc	dh		;..and skip header
gocom:
	call	setcurs
	xor	bx,bx
	mov	bl,area 	;get area
	mov	dx,wrdnow
	call	eclat		;brighten the current cipher word
	jmp	forever
gowrd:
	test	[bp].flaga,10h	;if a word list isn't up
	jz	gonext		;..try next area
	mov	dx,wrdnow	;else go to word list area
	jmp	gocom
go1st:
	test	[bp].flaga,20h	;if 1st list isn't up
	jz	gonext		;..try next area
	mov	dx,l1now	;else go to 1st list area
	jmp	gocom
go2nd:
	test	[bp].flaga,80h
	jz	gonext
	mov	dx,l2now
	jmp	gocom
gonext:
	test	flag1,02h	;if going previous
	jnz	clumsy		;..try previous area
	jmp	nxtwind 	;else try next area
clumsy:
	jmp	prewind
pagdn:
	jmp	forever

pagup:
	jmp	forever
top:
	jmp	forever
;toggle contrast rule switch
a1:
	xor	[bp].modeb,01h	;contrast rule on or off
	mov	si,offset msg3	;assume rule turned off
	test	[bp].modeb,01h
	jnz	a110
	mov	si,offset msg4	;rule is on
a110:
	mov	di,24*160+80
	call	showmsg 	;tell user state of rule
	or	flag1,01h
	jmp	forever

;toggle agreement rule switch
a2:
	xor	[bp].modeb,02h	;agreement rule on or off
	mov	si,offset msg6	;assume rule turned off
	test	[bp].modeb,02h
	jnz	a110
	mov	si,offset msg7	;rule is turned on
	jmp	a110

;toggle probable fit switch
a3:
	xor	[bp].modeb,04h	;probable fit on or off
	mov	si,offset msg8	;assume fit sorting turned off
	test	[bp].modeb,04h
	jnz	a110
	mov	si,offset msg9	;sorting turned on
	jmp	a110
;log solution
f7:
	xor	ax,ax		;simple substitution logging
	call	log
	jnc	f710
	or	flag1,01h	;message to be cleared, log file error
f710:
	jmp	forever
;help requested
sf1:
	mov	ax,3		;help number for simple substitution
	call	help
	jmp	forever
backtab:
rubout:
escape:
del:
altt:
alty:
altu:
alti:
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
f5:
f6:
f8:
f9:
f10:
insert:
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
bottom:
botpg:
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
simple	endp

clear	proc	near
	les	di,dword ptr [bp].videob
	mov	di,160
	mov	cx,23*80
	mov	ax,0720h
	rep stosw
	ret
clear	endp

init	proc	near
	mov	ah,13		;draw horizontal divider line
	mov	al,1		;at 13th line
	mov	dh,80
	call	hline
	mov	ah,17		;..another at 17th
	mov	al,1
	mov	dh,40
	call	hline
	mov	ah,13
	mov	al,27
	mov	dh,5		;and vertical connector
	call	vline
	call	showalf 	;show the alphabet
	ret
init	endp

showalf proc	near
	mov	bx,alfnow	;index into alphabet
	push	ds
	lds	si,dword ptr [bp].alfb
	lodsw			;alphabet's length
	mov	cx,ax
	add	si,bx		;1st letter to be displayed
	sub	cx,bx		;length to last letter
	mov	bx,si		;save ptr
	mov	dx,cx		;save length
	mov	ax,[bp].videoa	;address video buffer
	mov	es,ax
	mov	di,14*160	;video cipher alphabet line
	mov	ah,07h
shwalf00:
	lodsb
	stosw			;format cipher alphabet in video
	loop	shwalf00
	add	bx,512		;offset to solved letters
	mov	si,bx
	mov	cx,dx
	mov	di,13*160	;offset to K1 video line
shwalf10:
	lodsb
	stosw
	loop	shwalf10
	add	bx,256		;offset to inverse solved letters
	mov	si,bx
	mov	cx,dx
	mov	di,15*160	;offset to K2 video line
shwalf20:
	lodsb
	stosw
	loop	shwalf20
	pop	ds
	ret
showalf endp

showsub proc	near
	push	ds
	mov	cx,[bp].lines	;figure number of lines to display
	sub	cx,[bp].current ;total - current top
	cmp	cx,5		;but no more than 5 at a time
	jna	shwsb00
	mov	cx,5
shwsb00:
	lds	si,dword ptr [bp].gramb ;from current line
	les	di,dword ptr [bp].videob ;to video buffer
	mov	di,160		;leave title alone
	mov	ax,[bp].current
	mul	[bp].c81
	mov	si,ax
	push	cx		;save number of lines
	mov	ah,07	;normal attribute
shwsb05:
	push	cx	;save number of lines
	mov	cx,80
	inc	si
	push	si	;save cipher line ptr
shwsb07:
	lodsb		;cipher line to video
	stosw
	loop	shwsb07
	pop	si	;recover cipher line ptr
	mov	cx,80
shwsb09:
	lodsb		;clone cipher line to next video line
	stosw
	loop	shwsb09
	pop	cx
	loop	shwsb05
	pop	dx		;number of clone lines
	mov	ax,5		;if any lines left in window
	sub	ax,dx
	push	dx		;save number of clone lines
	mov	dx,crypt6b	;restore addressability
	mov	ds,dx
	mul	c160		;number of words to clear in window
	add	ax,80
	pop	dx		;recover number of clone lines
	mov	cx,ax
	mov	ax,0720h	;blank character
	rep stosw
	mov	di,2*160	;es:di->clone line
	lds	si,dword ptr [bp].alfb
	mov	cx,[si] 	;number of cipher alphabet letters
	inc	si
shwsb20:
	inc	si		;next letter
	mov	al,[si] 	;cipher alphabet letter
	mov	ah,[si+512]	;solved letter
	cmp	ah,' '          ;if not solved
	jne	shwsb30
	mov	ah,0F8h 	;..represent with degree symbol
shwsb30:
	call	update	;replace all al characters with ah character
	loop	shwsb20 	;do for all cipher alphabetics
	pop	ds
	ret
showsub endp

wrapz	proc	near
	mov	ax,[bp].lines	;figure top line on screen
	sub	ax,4
	jns	wrapz00
	xor	ax,ax		;less than 5 lines of cipher
wrapz00:
	mov	[bp].current,ax ;new screen top
	mov	ax,[bp].lines	;figure address of last line on screen
	dec	ax		;..origin 0
	mul	[bp].c81
	mov	[bp].gramb,ax	;last line is current cipher line
	ret
wrapz	endp

;on return
;nc  cx=length remaining in current line
;    es:di->current position in current line
;cy  line exhausted
getpos	proc	near
	les	di,dword ptr [bp].gramb ;current line
	xor	cx,cx
	mov	cl,es:[di]	;line length
	jcxz	getpos10	;..empty
	push	bx
	inc	di
	mov	bx,posnow
	xor	bh,bh
	sub	cx,bx
	add	di,bx		;es:di->current character
	clc
	pop	bx
	ret
getpos10:
	stc
	ret
getpos	endp

;on entry al=character
;return offset to plaintext letter in di
checkp	proc	near
	push	bx
	push	cx
	les	di,dword ptr [bp].alfb	;check that character
	mov	cx,es:[di]		;..is a plaintext letter
	add	di,258	;defined plaintext letters
	mov	bx,di
	repne scasb
	jne	chkp10
	dec	di
	sub	di,bx
	clc
chkpxit:
	pop	cx
	pop	bx
	ret
chkp10:
	stc
	jmp	chkpxit
checkp	endp

;on entry al=character
checkc	proc	near
	push	bx
	push	cx
	les	di,dword ptr [bp].alfb
	mov	cx,es:[di]
	add	di,2		;pointing at defined cipher alphabet
	mov	bx,di
	repne scasb
	jne	chkc10
	dec	di
	sub	di,bx
	clc
chkcxit:
	pop	cx
	pop	bx
	ret
chkc10:
	stc
	jmp	chkcxit
checkc	endp

;substitute plaintext in ah for ciphertext in al
dosub	proc	near
	push	bx
	push	cx
	call	checkc		;get offset to this cipher letter
	mov	bx,[bp].alfb	;..if it had a previous solution
	cmp byte ptr es:[di][bx+514],' '
	je	dosub10
	push	ax		;..save new substitute
	mov	ah,es:[di][bx+514] ;..get old substitute
	call	unkey		;..and undo it
	pop	ax		;retrieve new substitute
dosub10:
	call	unkey		;undo any previous assigment
	call	dokey		;assign this plain to cipher in key
	call	update		;..show it on screen
	call	showalf 	;..and show new key
	pop	cx
	pop	bx
	ret
dosub	endp

;remove solved letter ah from key areas
unkey	proc	near
	push	ax
	les	di,dword ptr [bp].alfb
	mov	cx,es:[di]	;length of the alphabet
	add	di,514		;es:di->solved letters, K2 area
	xchg	ah,al		;get plain in al
	repne scasb		;find position of the plain letter
	jne	unkeyxt 	;this letter not solved
	dec	di
	mov byte ptr es:[di],' ' ;remove letter from solved letters
	mov	ah,es:[di-512]	;get the cipher letter
	call	checkp		;get plaintext letter's index in plaintext
	mov	bx,[bp].alfb	;..alphabet into di
	mov byte ptr es:[di+770][bx],' '  ;..and remove inverse in K1
	xchg	ah,al		;put cipher letter in al
	mov	ah,0F8h 	;standard 'plain', cipher in ax
	call	update		;show removal on screen
unkeyxt:
	pop	ax
	ret
unkey	endp

;update K1 and K2 keys with plaintext letter ah for cipher letter al
dokey	proc	near
	push	ax
	call	checkc		;get index to cipher letter
	mov	bx,[bp].alfb	;..and put plain letter into K2 area
	mov byte ptr es:[di+514][bx],ah
	mov	al,es:[di+258][bx] ;now get cipher's equivalent plaintext
	xchg	ah,al		;original plaintext into al
	call	checkp		;index of original plaintext
	mov	bx,[bp].alfb	;now put cipher lower case
	mov byte ptr es:[di+770][bx],ah  ;into K1 area
	pop	ax
	ret
dokey	endp

;update screen.  for all al visible on screen, replace with ah
;al = cipher letter, assumed valid
;ah = replacement character, assumed valid
update	proc	near
	push	cx
	push	bx
	push	si
	mov	cx,[bp].lines	;figure number of cipher lines
	sub	cx,[bp].current ;..from top of screen to end of cipher
	cmp	cx,5		;but no more than 5 lines on a screen
	jna	updt10
	mov	cx,5
updt10:
	push	ax		;save cipher letter and substitute
	mov	ax,[bp].current ;find 1st cipher line displayed on screen
	mul	[bp].c81	;..but in the cryptogram, not on the screen
	mov	si,ax		;si is offset to cipher line shown on screen
	pop	ax		;recover cipher letter and substitute
	push	ds		;save addressability
	mov	ds,[bp].grama	;ds:si->current 1st cipher line
	mov	es,[bp].videoa
	mov	di,160		;es:di->1st cipher line on screen
	mov	bx,di
updtlp1:	;for every cipher line that's visible
	push	cx
	mov	cl,[si] 	;length of this cipher line
	xor	ch,ch
	jcxz	updt30		;if empty get next cipher line
updtlp2:	;inner loop.  for every char in this line
	cmp	al,es:[di]	;..if same as cipher letter
	jne	updt20
	mov	es:[di+160],ah	;..replace with substitute below cipher line
updt20:
	add	di,2		;next character, this line
	loop	updtlp2
updt30:
	pop	cx		;recover count of lines
	add	bx,320		;next cipher line on screen
	mov	di,bx
	add	si,81		;next cipher line in cryptogram
	loop	updtlp1 	;do for all cipher lines
	pop	ds		;recover addressability
	pop	si
	pop	bx
	pop	cx
	ret
update	endp

;find next unsolved character in this crypt line
;es:di->current character
;cx = count of remaining characters
findchar proc	near
fndchr00:
	mov	al,es:[di]	;get the character
	push	cx
	push	di
	push	es
	les	di,dword ptr [bp].alfb
	mov	cx,es:[di]
	add	di,2		;es:di->cipher alphabet
	repne scasb
	jne	fndchr10	;..not a cipher letter
	cmp byte ptr es:[di+511],' ' ;has it been solved?
	jne	fndchr10	;yes, try next letter in crypt line
	pop	es
	pop	di
	pop	cx
	dec	di	;origin 0 offset to unsolved letter
	clc		;found it return
	ret
fndchr10:
	pop	es
	pop	di
	pop	cx
	inc	di		;next crypt letter
	loop	fndchr00
	stc			;didn't find unsolved letter return
	ret
findchar endp

;dx = cursor position in primary or alternate list
;ss:bx -> primary or alternate word list ptrs in stack
wordsub proc	near
	xor	ax,ax
	les	di,dword ptr ss:[bx+4]	  ;->current cipher word index
	mov	al,es:[di].motrln	;length of word
	mov	[bp].templen,ax 	;..and save for later multiply
	mov	al,dh		;cursor row
	sub	al,15
	add	ax,ss:[bx+12]	;plus current top word of list
	mul	[bp].templen	;dx:ax=byte offset to current plain word
	div	c16		;dx:ax = relative seg,offset to plain word
	add	dx,ss:[bx]
	add	ax,ss:[bx+2]	;ax:dx->current plain word
	mov	wordb,dx	;..and save address
	mov	worda,ax
	xor	cx,cx
;for every letter
	mov	cl,es:[di].motrln
	xor	bx,bx		;letter count index
wrdsblp:
	cmp	es:[bx][di].motacnt,0	;if letter is unique
	je	wrdsb10
	shr	bx,1		;make count index, a letter index
	xor	ax,ax
	mov	al,es:[bx][di].motwrd	;get the cipher letter
	push	es
	push	di
	les	di,dword ptr wordb	;get the plaintext letter
	mov	ah,es:[di][bx]
	call	dosub
	pop	di
	pop	es
	shl	bx,1
wrdsb10:
	add	bx,2
	loop	wrdsblp
	ret
wordsub endp

crypt6	ends
	end
