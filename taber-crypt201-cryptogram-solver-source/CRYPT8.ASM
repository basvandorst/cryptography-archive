	page	53,132
	.8087
	include frame.str
	include motndx.str
	include crypt8b.asm
	extrn	showmsg:far,mot:far,int2asc:far,setcurs:far
	extrn	chi_squ:far,chi_prob:far,accord:far,droplist:far
crypt8	segment 'crya'
	assume	cs:crypt8,ds:crypt8b
	public	findpat,showlst,wordown,wordup,showpat,showprim,l1up,l1down
	public	rsetndx,eclat
;prepare cipher words list
findpat proc	far
	push	ds
	mov	ax,crypt8b	;data addressability
	mov	ds,ax
	test	[bp].flaga,10h	;if word index not prepared
	jnz	fpatxit
	call	mot		;..prepare word index
	jc	fpatxit 	;if index built
	les	di,dword ptr [bp].freehb	;..save memory
	mov	freeb,di			;..in case of reset
	mov	freea,es
	les	di,dword ptr [bp].sizeb
	mov	saveb,di
	mov	savea,es
fpatxit:
	pop	ds
	ret
findpat endp

;show cipher words list from current position in dx
showlst proc	far
	push	ds
	mov	ax,crypt8b	;data addressability
	mov	ds,ax
	mov	wrdpos,dx	;set current cursor position
	call	neword		;show new word number
	mov	ax,[bp].motzahl ;total words
	mov	si,offset msg1+10
	call	int2asc 	;..into message
	mov	si,offset msg1
	mov	di,16*160+40	;screen area to display
	call	showinf 	;..info, n of nnn words
	mov	cx,[bp].motzahl ;total words
	sub	cx,[bp].motcur	;..less current top of list
	cmp	cx,7		;..but no more than seven
	jna	shwlst10
	mov	cx,7
shwlst10:
	mov	bx,[bp].motcur	;current list top
	shl	bx,1
	mov	sptr,17*160+40	;current screen top
	push	cx
shwlstlp:
	les	di,dword ptr [bp].motib ;sorted index to list
	mov	ax,es:[di][bx]	;index to this word
	mul	motlang 	;..times length of entry
	div	c16		;into paragraphs, offset
	add	ax,[bp].mota
	add	dx,[bp].motb
	mov	itema,ax
	mov	itemb,dx	;itema:itemb->current index entry
	les	di,dword ptr itemb
	mov	ax,es:[di].motpcnt	;count of dictionary words
	mov	si,offset items+1
	call	int2asc
	mov	si,offset items 	;..display count
	mov	di,sptr 		;..on screen
	call	showinf
	call	showmot 		;display crypt word on screen
	add	sptr,160	;next screen line
	add	bx,2		;next sorted index
	loop	shwlstlp
	mov	cx,7		;clean up screen if less than 7 lines shown
	pop	ax
	sub	cx,ax
	jcxz	shwlst20
	mov	di,sptr
	mov	es,[bp].videoa
	mov	ax,0720h
shwlstl2:
	push	cx
	mov	cx,22
	rep stosw		;blank this line
	pop	cx
	add	di,2*68 	;next line
	loop	shwlstl2
;set the cursor
shwlst20:
	mov	dx,wrdpos
	call	setcurs
	call	hell		;brighten current word
	call	comlet		;brighten letters shared with primary word
	pop	ds
	ret
showlst endp

;move cursor down in cipher word list
;dx=current cursor position
wordown proc	far
	push	ds
	mov	ax,crypt8b
	mov	ds,ax		;data addressability
	mov	wrdpos,dx
	cmp	dh,23		;if end of display
	jne	wrdn20
	mov	ax,[bp].motcur	;..try to scroll list
	add	ax,7		;if 7 or less words remain,
	cmp	ax,[bp].motzahl
	jb	wrdn10
	jmp short wrdnxit	;..ignore cursor down
;can scroll list
wrdn10:
	inc	[bp].motcur	;next top of list
	call far ptr showlst	;scroll list down
	jmp short wrdnxit
;not end of display
wrdn20:
	xor	ax,ax		;don't fall off bottom of list
	mov	al,dh		;cursor row
	sub	ax,17		;..less bias
	add	ax,[bp].motcur	;current position in list
	inc	ax		;..origin 1
	cmp	ax,[bp].motzahl
	jnb	wrdnxit
	inc byte ptr wrdpos+1	;move cursor down one
	call	neword		;show new word number
	mov	si,offset msg1
	mov	di,16*160+40	;screen area to display
	call	showinf 	;..info, n of nnn words
	mov	dx,wrdpos	;new cursor position
	call	setcurs
	call	hell		;hilite this word
	call	comlet		;brighten letters shared with primary word
wrdnxit:
	mov	dx,wrdpos	;return new cursor position
	pop	ds
	ret
wordown endp

;move cursor up in cipher word list
;dx = current cursor position
wordup	proc	far
	push	ds
	mov	ax,crypt8b
	mov	ds,ax		;data addressability
	mov	wrdpos,dx
	cmp	dh,17
	jne	wrup10
;cursor at top of list, try to scroll up
	cmp	[bp].motcur,0
	je	wrupxit 	;can't scroll up
	dec	[bp].motcur
	call far ptr showlst
wrupxit:
	pop	ds
	ret
wrup10:
	dec byte ptr wrdpos+1
	call	neword
	mov	si,offset msg1
	mov	di,16*160+40	;screen area to display
	call	showinf 	;..info, n of nnn words
	mov	dx,wrdpos
	call	setcurs
	call	hell		;hilite this word
	call	comlet		;bright letters shared with primary word
	jmp	wrupxit
wordup	endp

;move cursor down to next word in plaintext list
;dx = current cursor position
l1down	proc	far
	push	ds
	mov	ax,crypt8b
	mov	ds,ax		;addressability
	mov	l1pos,dx	;save cursor position
	lea	bx,[bp].primb	;assume primary list scrolling
	cmp byte ptr l1pos,2Dh
	je	l1dn10		;..it is primary
	lea	bx,[bp].secb	;it is secondary
l1dn10:
	cmp	dh,23		;if last line on screen
	jne	l1dn20
	mov	ax,ss:[bx+12]	;..try to scroll primary list
	add	ax,9		;if 9 or less words remain in list
	les	di,dword ptr ss:[bx+4] ;point to word index
	cmp	ax,es:[di].motpcnt
	jnb	l1dnxt		;..can't scroll, ignore cursor down
;can scroll list, cursor on last word on screen
	inc word ptr ss:[bx+12] ;next list word
	mov	si,bx		;say which list
	mov	di,15*160+86	;assume first list
	cmp byte ptr l1pos,2Dh
	je	l1dn15
	mov	di,15*160+124	;..it's 2nd list
l1dn15:
	call far ptr showprim	;show list scrolled one down
	jmp short l1dn30	;..and update word number
;move cursor down 1 word in primary list
l1dn20:
	xor	ax,ax		;don't fall off bottom of list
	mov	al,dh		;cursor row
	sub	ax,15		;..less bias
	add	ax,ss:[bx+12]	;current position in list
	inc	ax		;..origin 1
	les	di,dword ptr ss:[bx+4] ;word index
	cmp	ax,es:[di].motpcnt
	jnb	l1dnxt		;can't fall off bottom of list
	inc byte ptr l1pos+1
	mov	dx,l1pos
	call	setcurs
l1dn30:
	mov	di,bx
	add	di,4		;-> word index ptr
	mov	ax,ss:[bx+12]	;number of the current word at top of list
	mov	sptr,12*160+86	;screen position for display
	cmp byte ptr l1pos,2Dh
	je	l1dn35
	mov	sptr,12*160+124
l1dn35:
	mov	dx,l1pos
	call	newlist 	;display position in list
l1dnxt:
	pop	ds
	ret
l1down	endp

;move cursor up to next word in primary list
l1up	proc	far
	push	ds
	mov	ax,crypt8b
	mov	ds,ax		;addressability
	mov	l1pos,dx	;cursor position
	lea	bx,[bp].primb	;assume 1st list scrolling
	cmp byte ptr l1pos,2Dh
	je	l1up05
	lea	bx,[bp].secb	;..it's 2nd list scrolling
l1up05:
	cmp	dh,15		;if cursor is at top of screen list
	jne	l1up20
	cmp word ptr ss:[bx+12],0	;..try to scroll up
	je	l1upxt		;if at top of list, can't scroll
;scroll up one word
	dec word ptr ss:[bx+12] ;previous word
	mov	si,bx
	mov	di,15*160+86
	cmp byte ptr l1pos,2Dh
	je	l1up08
	mov	di,15*160+124
l1up08:
	call far ptr showprim	;display list
l1up10:
	mov	sptr,12*160+86	;list position destination on screen
	cmp byte ptr l1pos,2Dh
	je	l1up15
	mov	sptr,12*160+124
l1up15:
	mov	dx,l1pos	;cursor position
	mov	ax,ss:[bx+12]	;current screen top
	mov	di,bx
	add	di,4		;->word index ptr
	call	newlist 	;display position in list
l1upxt:
	pop	ds
	ret
l1up20:
	dec byte ptr l1pos+1
	mov	dx,l1pos
	call	setcurs
	jmp	l1up10
l1up	endp

;display new position in primary or slave list
;ax = number of current list top
;dh = cursor row
;di -> word index ptr in stack
;sptr -> screen destination
newlist proc	near
	push	dx
	mov	dl,dh
	xor	dh,dh
	sub	dl,15
	add	ax,dx
	inc	ax		;position in list, origin 1
	mov	si,offset msg2+1
	call	int2asc 	;..format in message
	les	di,dword ptr ss:[di]	;get total number of words
	mov	ax,es:[di].motpcnt
	mov	si,offset msg2+10	;..and format in message
	call	int2asc
	mov	si,offset msg2	;display the position
	mov	di,sptr
	call	showinf
	pop	dx
	ret
newlist endp

;set the current word to high intensity
hell	proc	near
	push	dx
	call	dull		;return screen to normal attribute
	call	getaddr 	;get address of word index for current word
	call	onscrn		;if word not on screen
	jc	hellxt		;..leave
	les	di,dword ptr itemb
	call	scrnadr 	;screen offset returned in ax
	xor	cx,cx
	mov	cl,es:[di].motaln	;apparent word length
	mov	di,ax		;di=screen offset of word
	mov	es,[bp].videoa
	mov	al,0Fh		;bright attribute
hellp:
	inc	di
	stosb
	loop	hellp
hellxt:
	pop	dx
	ret
hell	endp

;changed areas.  dull the previous crypt word in the cryptogram, and if in
;a list area, brighten its corresponding crypt word.
;bx = area code
;dx = cursor position, meaningful only for cipher word list
eclat	proc	far
	push	ds
	mov	ax,crypt8b
	mov	ds,ax
	mov	wrdpos,dx	;in case we're in cipher word list
	shl	bx,1		;we can use existing code
	jmp	area[bx]
eclatc: 		;primary list area
	lea	bx,[bp].word1b
	jmp short eclat00
eclatd: 		;secondary list area
	lea	bx,[bp].word2b
eclat00:
	call	dull		;turn out the lights
	les	di,ss:[bx]	;tell other routines which word index
	mov	itemb,di
	mov	itema,es
	call	onscrn		;if current cipher word not on the screen
	jc	eclatxt 	;..just leave
	les	di,dword ptr itemb
	call	scrnadr 	;screen offset returned in ax
	xor	cx,cx
	mov	cl,es:[di].motaln	;apparent word length
	mov	di,ax		;di=screen offset of word
	mov	es,[bp].videoa
	mov	al,0Fh		;bright attribute
eclatp:
	inc	di
	stosb
	loop	eclatp
	jmp short eclatxt
eclatb: 		;cipher word list area
	call	hell		;turn on the current cipher word if possible
	jmp short eclatxt
eclata: 		;substitution area
	call	dull		;just turn off the lights
eclatxt:
	pop	ds
	ret
eclat	endp

;return display to normal intensity
dull	proc	near
	mov	cx,[bp].lines		;total cipher lines
	sub	cx,[bp].current 	;less screen's topmost line
	cmp	cx,5			;..but no more than 5
	jna	dull10
	mov	cx,5			;force 5 lines
dull10:
	mov	di,161			;skip header to 1st cipher line attr
	mov	al,07h			;normal attribute
	mov	es,[bp].videoa
dullp1: 		;for every line on the screen
	push	cx
	mov	cx,80			;length of screen line
dullp2:
	stosb				;force attribute to normal
	inc	di			;next attribute
	loop	dullp2		;for all attributes on this line
	pop	cx
	add	di,160		;skip the substitution line
	loop	dullp1		;for all lines
	ret
dull	endp

;if cipher cursor word has letters in common with primary cursor word
;highlight the common letters
comlet	proc	near
	push	dx		;must save dx
	test	[bp].flaga,20h	;if 1st list not up yet
	jnz	comlet00
	jmp	comletxt	;..nothing to hilite, just leave
comlet00:
;lolite the index words
	mov	al,07h		;normal attribute
	mov	di,17*160+53	;video destination...ATTRIBUTE BYTE
	mov	es,[bp].videoa
	mov	cx,7		;seven lines
cmltlp1:
	push	cx
	mov	cx,16
cmltlp2:
	stosb			;normal attribute, this character
	inc	di		;next attribute
	loop	cmltlp2 	;do attributes for all characters
	pop	cx
	add	di,128		;next video line
	loop	cmltlp1 	;do all word indexes
	mov	di,13*160+91	;lolite primary cipher word
	mov	cx,16
cmltlp3:
	stosb
	inc	di
	loop	cmltlp3
;calculate which word the cursor word is
	call	getaddr 	;ax:dx = seg:offset of cursor word
	cmp	ax,[bp].word1a	;if cursor word is primary word
	jne	cmlt10
	cmp	dx,[bp].word1b
	jne	cmlt10
	jmp	comletxt	;..nothing to hilite, just exit
;cursor word is not the primary word
cmlt10:
	push	ds		;retrieve crypt words into my dseg
	push	ds
	pop	es
	mov	di,offset wort1
	mov	cx,32		;blank out word areas
	mov	al,' '
	rep stosb
	mov	al,07h		;assume normal attribute
	mov	cx,32
	mov	al,07h
	rep stosb
	lds	si,dword ptr itemb	;retrieve cursor word
	assume	ds:nothing,es:crypt8b
	mov	cl,[si].motrln
	mov	w1len,cx
	lea	si,[si].motwrd
	mov	di,offset wort1
	rep movsb
	lds	si,dword ptr [bp].word1b	;retrieve primary word
	mov	cl,[si].motrln
	mov	w2len,cx
	lea	si,[si].motwrd
	mov	di,offset wort2
	rep movsb
	pop	ds
	assume	es:nothing,ds:crypt8b
	mov	si,offset wort1
	mov	cx,w1len
cmltlp4:
	lodsb
	push	cx
	mov	cx,w2len	;check if this cursor word letter
	mov	di,offset wort2 ;..is in primary word
	repne scasb
	jne	cmlt20
	mov	bx,si
	sub	bx,offset wort1
	dec	bx
	mov	w1onof[bx],0Fh	;hilite this cursor letter
cmltlp5:
	mov	bx,di
	sub	bx,offset wort2
	dec	bx
	mov	w2onof[bx],0Fh	;hilite this primary letter
	jcxz	cmlt20
	repne	scasb
	je	cmltlp5
cmlt20:
	pop	cx
	loop	cmltlp4
;cursor word screen destination
	xor	ax,ax
	mov	al,byte ptr wrdpos+1
	mul	c160
	add	ax,53
	mov	di,ax
	mov	es,[bp].videoa	;es:di->attribute byte of cursor word
	mov	cx,w1len
	mov	si,offset w1onof
cmltlp6:			;set attribute bytes for cursor word
	movsb
	inc	di
	loop	cmltlp6
	mov	di,13*160+91	;es:di->attribute byte of primary word
	mov	cx,w2len
	mov	si,offset w2onof
cmltlp7:
	movsb
	inc	di
	loop	cmltlp7
comletxt:
	pop	dx
	ret
comlet	endp

;compute word index ptr for this word, cursor_row + current_top_word
getaddr proc	near
	xor	bx,bx
	mov	bl,byte ptr wrdpos+1	;cursor row
	sub	bl,17			;..ith item on screen
	add	bx,[bp].motcur		;..plus current screen top item
	shl	bx,1			;..is ith list item's index
	les	di,dword ptr [bp].motib ;->index of word indexes
	mov	ax,es:[di][bx]		;ith index to word indexes
	mul	motlang 		;..times length of each entry
	div	c16			;..into paras and offset
	add	ax,[bp].mota
	add	dx,[bp].motb		;ax:dx->word_index[i]
	mov	itemb,dx
	mov	itema,ax
	ret
getaddr endp

;check if current word is on screen
onscrn	proc	near
	mov	ax,[bp].current ;current cryptogram line, top of screen
	mul	[bp].c81	;minimum offset
	mov	dx,ax
	add	ax,81*5 	;maximum offset
	les	di,dword ptr itemb	;->word index
	cmp	ax,es:[di].motoff	;offset must be less than end screen
	jna	noscrn		;word off end of screen
	cmp	dx,es:[di].motoff	;offset must be => start of screen
	ja	noscrn
	clc
	ret
noscrn:
	stc
	ret
onscrn	endp

;compute screen location corresponding to cipher word
;es:di->word index entry
scrnadr proc	near
	mov	ax,es:[di].motoff	;get cipher word offset
	xor	dx,dx
	div	[bp].c81	;lines, remainder
	mov	bx,dx		;save remainder
	sub	ax,[bp].current
	shl	ax,1		;2 screen lines for every cipher line
	inc	ax		;..and one header line
	mul	c160		;screen line
	shl	bx,1		;double remainder
	add	ax,bx		;ax = screen offset of word, (I hope)
	sub	ax,2
	ret
scrnadr endp

;show pattern header
;si->primary or alternate pattern ptrs
;di->screen destination
showpat proc   far
	push	ds
	push	si
	mov	ax,crypt8b
	mov	ds,ax		;addressability
	mov	sptr,di 	;save screen destination
	mov	lstptr,si	;save list source
	call	clearlst	;clear plaintext list area
	mov	di,12*160+124
	cmp	di,sptr
	je	shwpat00
	call	clearlst	;clear second list area
shwpat00:
	les	di,dword ptr ss:[si+4]	;get the word index
	test	es:[di].motflg,0C0h	;if no words
	jz	shwpat10
shwpat05:
	stc				;..flag no list to show
	jmp	shwpatxt		;..and leave
shwpat10:
	cmp	es:[di].motpcnt,0	;if no words
	je	shwpat05		;.. leave
;show the count
	mov	ax,es:[di].motpcnt
	mov	si,offset msg2+10	;total words
	call	int2asc
	mov	ax,1			;current word
	mov	si,offset msg2+1
	call	int2asc
	mov	si,offset msg2
	mov	di,sptr
	call	showinf
;show the cipher word
	add	sptr,164	;next screen line plus two characters
	mov	di,sptr
	mov	si,lstptr
	lds	si,dword ptr ss:[si+4]
	push	si		;save word index base
	lea	si,[si].motwrd	;->cipher word
	mov	ah,07h		;normal attribute
	mov	bx,di
	mov	cx,16
shwptlp1:
	lodsb			;move cipher character to screen
	stosw
	loop	shwptlp1	;all characters
	add	bx,160		;next screen line
	mov	di,bx		;solved letters destination
	pop	si		;recover word index base
	mov	cl,[si].motrln	;get cipher word length
	mov	dx,cx
	lea	si,[si].motalf	;->alphabetic offsets
	mov	ah,01h		;if monochrome, underline
	test	[bp].flaga,40h	;..test assumption
	jz	shwptlp2
	mov	ah,07h		;..it's color, normal attribute
shwptlp2:
	lodsb
	push	es
	push	di
	push	bx
	xor	bx,bx
	mov	bl,al
	les	di,dword ptr [bp].alfb
	mov	al,es:[di+514][bx] ;return solved letter
	pop	bx
	pop	di
	pop	es
	stosw			;..and put in screen
	loop	shwptlp2
	mov	cx,16		;blank rest of line
	sub	cx,dx
	jz	shwpat20	;no more line
	mov	ax,0720h	;normal attribute, blank
	rep stosw
shwpat20:
	clc			;something to show
shwpatxt:
	pop	si
	pop	ds
	ret
showpat endp

;si -> source list ptrs of list to be displayed
;di -> screen destination to display list
;	1st list 15*160+86  screen location
;	2nd list 15*160+124 screen location
;show plaintext word list, either primary or secondary list
showprim proc far
	push	ds
	push	bx
	mov	ax,crypt8b
	mov	ds,ax		;addressability
	finit			;initialize NDP
	mov	[bp].plains,si	;save ptr to source ptrs
	mov	[bp].plaind,di	;save screen destination
	les	di,dword ptr ss:[si+4]	;->index of word list
	mov	[bp].quodb,di	;tell accord which list
	mov	[bp].quoda,es
	xor	ax,ax		;retrieve plaintext word length
	mov	al,es:[di].motrln
	mov	[bp].templen,ax ;save for later
	mov	ax,ss:[si+12]	;current top word in list
	mul	[bp].templen	;->offset to 1st word to show
	mov	cx,es:[di].motpcnt	;retrieve number of entries
	mov	di,[bp].plaind	;screen destination
	mov	ds,ss:[si+2]	;plaintext list segment
	mov	es,[bp].videoa	;screen segment
	mov	bx,di
	mov	si,ax
	sub	cx,[bp].primcur
	cmp	cx,9
	jna	shwprm10
	mov	cx,9
shwprm10:
	mov	dx,cx		;save for later
shwprmlp:
	push	cx
	push	bx
	mov	bx,[bp].plains	;plaintext source list
	call	getprob 	;get probability group in ax
	mov	ah,07		;normal attribute
	stosw			;..put to screen
	add	di,2		;leave a blank
	mov	cx,[bp].templen ;length of word
	call	agree		;set bright if word is in accord
	pop	bx
shwprml2:
	lodsb			;move letter to screen
	stosw
	loop	shwprml2	;next letter
	mov	cx,16		;blank rest of word
	sub	cx,[bp].templen
	jz	shwprm20
	mov	al,' '
	rep stosw
shwprm20:
	pop	cx
	add	bx,160		;next screen line
	mov	di,bx
	loop	shwprmlp	;next word
	mov	cx,9
	sub	cx,dx
	jz	shwprm30
	mov	al,' '          ;blank out remaining lines
shwprml3:
	push	cx
	mov	cx,18
	rep stosw
	pop	cx
	add	bx,160
	mov	di,bx
	loop	shwprml3
shwprm30:
	pop	bx
	pop	ds
	ret
showprim endp

;reset all indexes
rsetndx proc	far
	push	ds
	mov	ax,crypt8b	;establish addressability
	mov	ds,ax
	call	droplist	;erase all temporary word lists
	mov	cx,[bp].motzahl ;count of indexes
	les	di,dword ptr [bp].motb	;address first index
rndxlp:
	and	es:[di].motflg,0FCh	;reset wordfile/reduced flags
	mov	ax,es:[di].motwcnt	;restore original count of words
	mov	es:[di].motpcnt,ax	;..to present count
	add	di,motlang		;next index
	loop	rndxlp			;do for all indexes
	mov	di,12*160+86	;clear screen of primary list
	call	clearlst
	mov	di,12*160+124	;clear screen of secondary list
	call	clearlst
	les	di,dword ptr freeb	;reset memory to end of indexes
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr saveb
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	and	[bp].flaga,5Fh		;clear 1st and 2nd lists flags
	pop	ds
	ret
rsetndx endp

;si-> message to be displayed
;di-> screen area to display it
showinf proc	near
	push	cx
	mov	es,[bp].videoa	;es:di->destination
	xor	cx,cx
	mov	cl,[si] 	;length of info
	mov	ah,07h		;normal attribute
	inc	si
shwinflp:
	lodsb
	stosw
	loop	shwinflp
	pop	cx
	ret
showinf endp

;es:di->screen destination area
showmot proc	near
	push	cx
	push	ds
	mov	cx,16
	lds	si,dword ptr itemb	;->word entry
	lea	si,[si].motwrd
	mov	ah,07h		;normal attribute
shwmotlp:
	lodsb
	stosw
	loop	shwmotlp
	pop	ds
	pop	cx
	ret
showmot endp

;put the new word number into msg
neword	proc	near
	xor	ax,ax
	mov	al,byte ptr wrdpos+1	;cursor row
	sub	ax,17		;current word
	add	ax,[bp].motcur
	inc	ax		;origin 1
	mov	si,offset msg1+1
	call	int2asc 	;..word number into message
	ret
neword	endp

;di -> list destination on screen
;clear current word list
clearlst proc near
	mov	bx,di		;save screen starting destination
	mov	ah,07h		;normal attribute
	mov	al,205		;horizontal double line
	mov	es,[bp].videoa
	mov	cx,18		;line length
	rep stosw		;restore the bar
	mov	al,' '          ;blank rest of list
	mov	cx,11		;11 lines
clrlstlp:
	add	bx,160
	push	cx
	mov	cx,18
	mov	di,bx
	rep stosw
	pop	cx
	loop	clrlstlp
	ret
clearlst endp

;bx-> word list ptr
;ds:si->current word in list
getprob proc	near
	push	ds
	push	es
	push	di
	push	si
	push	ds		;swap segments for chi_squ
	pop	es
	push	si
	pop	di		;es:di->current word
	mov	si,bx
	call	chi_squ 	;recompute chi-squared
	lds	si,ss:[si+4]	;get the degrees of freedom
	mov	ax,[si].motdf
	call	chi_prob	;compute absolute probability
	mov	ax,crypt8b
	mov	ds,ax		;addressability again
;compute scale of fit
	fldz	;0,probability
	fcomp		;if 0 >= probability
	fstsw	status
	fwait
	mov	ah,byte ptr status+1
	sahf		;..set condition code
	jnb	fit40	;..ignore
	fxtract 	;mantissa,characteristic
	fstp	st(0)	;characteristic
	fabs		;approx lg 2 of probability
	fldlg2		;lg 10,lg p
	fmul		;approx log p		scale of probability
	fistp	scale	;		empty stack
	fwait
	mov	ax,scale	;rate probability on scale of 1-9
	cmp	ax,0		;if rounded to zero
	jne	dofit05
	mov	ax,1		;..force it to 1
dofit05:			;if scale>9, force it to 0
	cmp	ax,9		;1    >p>=.05	      1  approximately
	jna	dofit10 	;.05  >p>=.005	      2
dofit07:		;force a zero
	xor	ax,ax		;.005 >p>=.0005       3
dofit10:			;.0005>p>=.00005      4  etc.
	add	al,'0'
	pop	si
	pop	di
	pop	es
	pop	ds
	ret
fit40:			;invalid probability
	fstp	st(0)		;clear the NDP
	jmp	dofit07 	;..and force a zero display
getprob endp

;if plaintext letters are in agreement with previously solved letters, set
;ah=0Fh for bright to show agreement
;bx->word list ptr
agree	proc	near
	push	es
	push	di
	push	ds
	push	si
	push	ds
	push	si
	mov	si,bx	;accord expects ptr in si
	pop	bx	;..and plaintext ptr in es:bx
	pop	es
	call	accord
	mov	ah,07h	;assume no agreement, dull letters
	jc	agreext
	mov	ah,0Fh	;agreement, bright letters
agreext:
	pop	si
	pop	ds
	pop	di
	pop	es
	ret
agree	endp
crypt8	ends
	end
