	page	56,132
	.8087
dimen	struc
r	dw	?		;row
c	dw	?		;column
sumrc	dw	?		;sum of row and column for sorting
varnce	dd	?		;group0 letters variance for this dimension
dimen	ends

	include frame.str
	include xposprm.str
	extrn	savefree:far,getcurs:far,cntlet:far,freeadj:far,getmem:far
	extrn	setcurs:far,factor:far,isort:far,vline:far,hline:far
	extrn	int2asc:far,fp2asc:far,showmsg:far,checkc:far,permf:far
	extrn	i_col:far,i_row:far,i_diag:far,i_spiral:far,i_zcol:far
	extrn	i_zrow:far,i_zdiag:far,i_revrow:far,i_revcol:far,i_tran:far
	extrn	i_atran:far,permg:far,log:far,help:far
	include crypt21b.asm
crypt21 segment 'crya'
	assume	cs:crypt21,ds:crypt21b
	public	xpose,checkv
xpose	proc	far
	push	ds
	mov	ax,crypt21b
	mov	ds,ax			;addressability
	mov	vowel,0
	mov	area,0
	mov	rtepos,0200h
	mov	dimpos,0B00h
	mov	logpos,0210h
	mov	curdim,0
	mov	flag1,0
	mov	flag2,0
	mov	rtflag,0
	mov	si,offset saveb 	;save freespace
	call	savefree		;..for restoring on exit
	call	freeadj 		;adjust freespace to paragraph
	call	getcurs 		;save old cursor position
	mov	oldpos,dx		;..for restoring on exit
	call	cntlet			;get number of letters in cryptogram
	mov	letcnt,ax
;figure memory needs
	shl	ax,1		;orig cipher letters, permuted cipher letters
	shl	ax,1		;..refresh and solved area
	shl	ax,1		;..plus index and index work areas
	mov	cx,ax
	call	getmem
	jnc	xpose10
nomem:
	mov	si,offset msg1
	mov	di,24*160+80
xposerr:
	call	showmsg
	jmp	xposxit
xpose10:			;carve up obtained memory
	mov	cipherb,di	;base cipher letters
	mov	ciphera,es	;segment
	mov	ax,letcnt
	add	di,ax
	mov	rectb,di	;permuted cipher work area
	add	di,ax
	mov	refresh,di	;permuted cipher refresh area
	add	di,ax
	mov	solveb,di	;solving rectangle offset
	add	di,ax
	mov	indexb,di	;permutation index area
	shl	ax,1
	mov	workb,di	;permutation work area
;get all factors of length of cipher
	mov	ax,letcnt
	call	factor
	jnc	xpose20
	jmp	nomem
xpose20:
	or	ax,ax		;if no factors
	jnz	xpose30
	mov	si,offset msg2	;..this can't be a rectangular cipher
	jmp	xposerr
xpose30:
	mov	factn,ax	;save number of factor pairs
	mov	factb,di	;..and their address
	mov	facta,es
;count number of dimensions  5 4 is two, allowing for 4 5
;			     8 8 is one
	mov	cx,ax
	xor	ax,ax
xpose35:
	mov	dx,es:[di]	;get one half of dimension
	inc	ax		;count at least one dimension
	cmp	dx,es:[di+2]	;..and if not the same as other half
	je	xpose38
	inc	ax		;..count another dimension
xpose38:
	add	di,4		;next pair
	loop	xpose35 	;do for all pairs
	mov	dimn,ax 	;number of dimensions
	mov	[bp].srtn,ax	;prepare for sort

;build dimension table for display
	mov	ax,type dimen	;size of dimension table entry
	mov	[bp].srtlen,ax	;prepare for sort
	mul	dimn		;..times number of entries
	mov	cx,ax
	call	getmem
	jnc	xpose40
	jmp	nomem
xpose40:
	mov	dimenb,di	;anchor the dimension table
	mov	dimena,es
	mov	[bp].srtentb,di ;prepare for sort
	mov	[bp].srtenta,es
	mov	cx,factn	;number of factor pairs
	finit
	fldz
	push	ds
	lds	si,dword ptr factb
xposlp1:
	lodsw			;get row
	mov	es:[di].r,ax	;..into dimension table
	mov	dx,ax		;hold row
	lodsw			;get column
	mov	es:[di].c,ax	;..into dimension table
	mov	bx,ax		;hold column
	add	ax,dx		;row + column for sorting
	mov	es:[di].sumrc,ax
	fst	dword ptr es:[di].varnce
	add	di,type dimen	;next entry
	cmp	dx,bx		;if perfect square
	je	xpose45 	;..don't build the flipped entry
	mov	es:[di].c,dx	     ;flipped entry column
	mov	es:[di].r,bx	     ;flipped entry row
	mov	es:[di].sumrc,ax
	fst	dword ptr es:[di].varnce
	add	di,type dimen	      ;next entry
xpose45:
	loop	xposlp1
	pop	ds
	fstp	st(0)		;clear the NDP
;sort by sum of row and column to put most likely dimension near top of
;list, which is the one closest to a square with more rows than columns
	mov	ax,offset comp1        ;address of comparison routine
	mov	[bp].srtcmpb,ax
	mov	[bp].srtcmpa,cs
	mov	[bp].srtdir,0	;sort ascending
	call	isort		;sort in place with insertion sort
	jnc	xpose50
	jmp	nomem

;reserve space for row, column indexes
xpose50:
	mov	ax,type dimen	;get last dimension table entry
	mul	dimn
	les	di,dword ptr dimenb
	add	di,ax		;es:di->last entry
	mov	cx,es:[di].sumrc	;number of rows + number of columns
	shl	cx,1			;number of word indexes, rows and cols
	inc	cx			;guarantee word alignment
	call	getmem
	jnc	xpose55
	jmp	nomem
xpose55:
	test	di,01h			;if odd address
	jz	xpose56
	inc	di			;..adjust to even
xpose56:
	mov	colptrb,di		;ciphera:colptrb->column indexes
	mov	rowptrb,di		;temp, not fixed until dimension chosen

;prepare the screen for display
	mov	cx,23*80
	mov	ax,0720H	;blank char, normal attributes
	mov	es,[bp].videoa
	mov	di,160
	rep stosw		;clear the screen
	mov	di,24*160+32
	mov	si,offset panelid
	call	showmsg 	;identify the screen
	mov	ah,2
	mov	al,13
	mov	dh,23
	call	vline		;draw vertical divider line
	mov	ah,2
	mov	al,47
	mov	dh,21
	call	vline		;draw solving area divider line
	mov	ah,10
	mov	al,1
	mov	dh,12
	call	hline		;draw routes, dim horizontal divider
	mov byte ptr es:[di],185	;left tee
	mov	ah,22		;row 22
	mov	al,13		;col 13
	mov	dh,80-12	;length
	call	hline		;draw computed route display box
	mov byte ptr es:[di-136],204   ;right tee
	mov byte ptr es:[di-68],202    ;up tee
;show the route choices

	mov	di,160
	mov	si,offset routes
	mov	cx,8
	mov	ah,07		;normal attribute
xpos60:
	push	cx
	mov	cx,12
xpos70:
	lodsb
	stosw
	loop	xpos70
	pop	cx
	add	di,160-24
	loop	xpos60
;show the dimensional choices
	mov	si,offset dimtit
	mov	di,10*160
	mov	cx,12
xpos80:
	lodsb
	stosw
	loop	xpos80
	call	showdim
	mov	dx,rtepos		;set cursor at route choices
	call	setcurs
	mov	di,2*160
	mov	cx,12
	call	hell			;hilite the first choice
;copy the cryptogram letters only to prepare for permuting
	mov	cx,[bp].lines
	les	di,dword ptr cipherb
	push	ds
	lds	si,dword ptr [bp].sbuffb	;cryptogram source letters
	xor	dx,dx		;vowel counter
xpos82:
	push	cx
	push	si
	xor	cx,cx
	mov	cl,[si]
	inc	si
	jcxz	xpos88		;beware of empty lines
xpos84:
	lodsb			;get cryptogram character
	call	checkc		;if it is a recognized letter
	jc	xpos86
	stosb			;..stash it away
	call far ptr checkv	;..count if vowel
	jc	xpos86
	inc	dx
xpos86:
	loop	xpos84
xpos88:
	pop	si
	pop	cx
	add	si,81		;next cryptogram line
	loop	xpos82
	pop	ds
	mov	vowel,dx
	cmp	dimn,1		;if there is only 1 dimension to choose
	jne	forever
	les	di,dword ptr dimenb	;..consider it chosen
	mov	ax,[di].r		;..choose the row
	mov	chosen3,ax
	mov	ax,[di].c		;..choose the column
	mov	chosen4,ax
	or	flag1,20h		;..mark fixed dimension
	page
;screen, keyboard do forever
forever:
	mov	ah,0			;wait for user to enter something
	int	16h
	mov	savechar,ax		;save keystroke
	test	flag1,08h		;if there is a pending message
	jz	xpose90
	and	flag1,0F7h		;..clear it
	mov	es,[bp].videoa
	mov	di,24*160+80
	mov	ax,0F20h
	mov	cx,40
	rep stosw
	mov	ax,savechar
xpose90:
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
	test	flag2,01h		;if not in logging mode
	jz	xposxit 		;..quit is wanted
f205:
	mov	di,offset klarcip	;else cancel logging
	call	klar			;clean up the screen
	call	showcol
	mov	di,3*160+26		  ;refresh columns in cipher area
	call	showrow
	xor	dx,dx			;show cipher
	call	showcip
	and	flag2,0FEh		;leave logging mode
	mov	dx,savepos
	call	setcurs 		;return to solving mode
	jmp	forever

xposxit:
	les	di,dword ptr saveb	;restore freespace
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	les	di,dword ptr freeb
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	mov	dx,oldpos
	call	setcurs
	pop	ds
	ret

;move cursor down
curdn:
	test	flag2,01h	;if in logging mode
	jz	curdn10
	cmp byte ptr logpos+1,4 ;..handle separately
	jb	logdn10 	;don't cursor off list
	jmp	forever
logdn10:
	xor	ax,ax
	mov	al,byte ptr logpos+1
	mul	c160
	add	ax,32		;screen offset of current log choice
	mov	di,ax
	mov	cx,8
	call	dunkel		;normal intensity for current choice
	add	di,160
	call	hell		;brighten next choice
	inc byte ptr logpos+1
	mov	dx,logpos
	call	setcurs
	jmp	forever

curdn10:
	xor	bx,bx		;find screen area for cursor movement
	mov	bl,area
	shl	bx,1
	jmp	cursdn[bx]
;move cursor down in route list
rtdown:
	cmp byte ptr rtepos+1,8 	;don't cursor off screen
	jb	rtdn10
	jmp	forever 		;ignore cursor movement
rtdn10:
	xor	ax,ax			;restore normal intensity
	mov	al,byte ptr rtepos+1
	mul	c160			;screen position of current route
	mov	di,ax
	mov	cx,12
	call	dunkel
	add	di,160			;brighten next route
	call	hell
	inc byte ptr rtepos+1		;bump cursor row
	mov	dx,rtepos		;set cursor
	call	setcurs
	jmp	forever

;move cursor down in dimension list
dmdown:
	xor	ax,ax
	mov	al,byte ptr dimpos+1
	sub	ax,10			;adjust to origin 1
	add	ax,curdim		;ith dimension entry
	cmp	ax,dimn 		;don't cursor off the list
	jb	dmdn10
	jmp	forever 		;at end of list, no action
dmdn10:
	cmp byte ptr dimpos+1,23	;don't cursor off the window
	jb	dmdn20
	inc	curdim			;scroll list one line down
dmupdn:
	call	showdim
	xor	ax,ax
	mov	al,byte ptr dimpos+1	;hilite new current dimension
	sub	ax,11
	mul	c160
	mov	cx,12
	mov	di,ax
	call	hell
	jmp	forever
dmdn20:
	xor	ax,ax
	mov	al,byte ptr dimpos+1	;return old dimension to normal
	mul	c160
	mov	cx,12
	mov	di,ax
	call	dunkel
	add	di,160			;hilite new dimension
	call	hell
	inc byte ptr dimpos+1
	mov	dx,dimpos
	call	setcurs
	jmp	forever

;move cursor down in in rows of cryptogram
r1down:
;move cursor down in rows of solving area
r2down:

;move cursor up
curup:
	test	flag2,01h
	jz	curup10
	cmp byte ptr logpos+1,2 ;don't cursor above list
	ja	logup10
	jmp	forever
logup10:
	xor	ax,ax
	mov	al,byte ptr logpos+1
	mul	c160
	add	ax,32
	mov	di,ax		;screen offset of current choice
	mov	cx,8
	call	dunkel		;normal intensity for current choice
	sub	di,160
	call	hell		;brighten next choice
	dec byte ptr logpos+1	;back up in list
	mov	dx,logpos
	call	setcurs
	jmp	forever

curup10:
	xor	bx,bx		;find area to go to
	mov	bl,area
	shl	bx,1
	jmp	cursup[bx]
rtup:
	cmp byte ptr rtepos+1,2 	;don't move out of routes
	ja	rtup10
	jmp	forever
rtup10:
	xor	ax,ax			;restore normal intensity
	mov	al,byte ptr rtepos+1
	mul	c160			;screen position of current route
	mov	di,ax
	mov	cx,12
	call	dunkel
	sub	di,160			;brighten previous route
	call	hell
	dec byte ptr rtepos+1		;back up cursor row
	mov	dx,rtepos		;set cursor
	call	setcurs
	jmp	forever

;move cursor up in dimension list
dmup:
	cmp byte ptr dimpos+1,11	;don't cursor out of window
	ja	dmup10
	cmp	curdim,0		;don't cursor out of list
	ja	dmup20
	jmp	forever 	;at top of list, can't move further up
dmup10:
	xor	ax,ax
	mov	al,byte ptr dimpos+1	;return old dimension to normal
	mul	c160
	mov	cx,12
	mov	di,ax
	call	dunkel
	sub	di,160			;hilite new dimension
	call	hell
	dec byte ptr dimpos+1
	mov	dx,dimpos
	call	setcurs
	jmp	forever
dmup20:
	dec	curdim
	jmp	dmupdn		;common dimension list scroll


r1up:
r2up:
	jmp	forever

;move cursor right in cipher or solving area
curite:
	test	flag2,01h	;if in logging mode,
	jz	curit00
	jmp	forever 	;..horizontal cursor movement not meaningful
curit00:
	xor	bx,bx		;figure area for right cursor movement
	mov	bl,area
	shl	bx,1
	jmp	curight[bx]	;go to cipher or solution right

;move cursor left in cipher or solving area
curleft:
	test	flag2,01h	;if in logging mode
	jz	curlef00
	jmp	forever 	;..horizontal cursor movement not meaningful
curlef00:
	xor	bx,bx		;figure area for left cursor movement
	mov	bl,area
	shl	bx,1
	jmp	curlef[bx]	;..and go there

;cipher cursor right
ciprit:
	xor	ax,ax		;get current column position
	mov	al,byte ptr cippos
	sub	al,16		;..index origin 0
	shr	ax,1		;for visibility, 2 cursor positions = 1 col
	inc	ax		;..index origin 1
	mov	bx,chosen	;indirect pointer to number of cipher columns
	mov	cl,3
	shl	bx,cl
	cmp	ax,15		;don't move cursor out of area
	jb	ciprit10
;try to scroll cipher right
	add	ax,curcol1	;current left cipher column + 15
	cmp	ax,chosen4[bx]	;if more columns out of view
	jb	ciprit30	;..ok to scroll
	jmp	forever 	;else at end of cipher display

;cursor can still go right in cipher area
ciprit10:
	cmp	ax,chosen4[bx]	;don't move past cipher's last column
	jb	ciprit20
	jmp	forever 	;..at end of cipher
ciprit20:
	mov	bx,cippos	;dull current coordinates
	mov	si,26		;offset of cipher rows
	mov	al,07h
	call	coord
	test	flag2,02h	;if pending mark
	jz	ciprit25
	mov	dl,70h		;..restore marked column
	call	markcol
ciprit25:
	add byte ptr cippos,2	;two cursor positions for one cipher column
	jmp	area220 	;brighten coordinates

;scroll cipher right one column
ciprit30:
	inc	curcol1
ciprit40:
	mov	di,offset klarcip  ;blank out previous cipher
	call	klar
	call	showcol 	;show new column numbers for cipher
	mov	di,3*160+26	;blank out previous cipher
	call	showrow 	;refresh row numbers for cipher
	xor	dx,dx		;show cipher
	call	showcip 	;show new columns for cipher
	mov	bx,cippos
	mov	si,26		;offset to start of cipher area rows
	mov	al,0Fh
	call	coord		;refresh coordinate hilites
	test	flag2,02h	;if a mark is pending
	jz	ciprit50
	mov	dl,70h		;reverse video
	call	markcol 	;..refresh marked column
ciprit50:
	jmp	forever

;solution cursor right
solrit:
	xor	ax,ax		;get current solution column position
	mov	al,byte ptr solpos
	sub	al,50		;..index origin 0
	shr	ax,1		;2 cursor positions = 1 col
	inc	ax		;..index origin 1
	cmp	ax,15		;don't cursor out of area
	jb	solrit10	;..ok to move cursor
;can't move cursor, try to scroll solution
	add	ax,curcol2	;current solution left col + 15
	cmp	ax,solcols	;if more columns to show than are on screen
	jb	solrit30	;..ok to scroll
	jmp	forever 	;else at end of display

solrit10:
	cmp	ax,solcols	;don't move cursor past end of solution
	jb	solrit20
	jmp	forever 	;at end of solution
solrit20:
	mov	bx,solpos
	mov	si,94
	mov	al,07h
	call	coord		;turn off previous position coordinates
	add byte ptr solpos,2	;two cursor positions for each column
	jmp	area320 	;move cursor and brighten coordinates

;scroll solution right one column
solrit30:
	inc	curcol2
solrit40:
	mov	di,offset klarsol  ;clear previous solution display
	call	klar
	call	showprm 	;show new column numbers for solution
	call	rowsol		;show new rows
	mov	dx,1		;show new solution
	call	showcip
	mov	bx,solpos	;refresh coordinate hilites
	mov	si,94
	mov	al,0Fh
	call	coord
	jmp	forever

;cipher cursor left
ciplef:
	cmp byte ptr cippos,16	;don't move cursor out of area
	ja	ciplef10	;..can move cursor
;cursor at left margin, try to scroll cipher
	cmp	curcol1,0	;if cipher not at left margin
	ja	ciplef20	;..can scroll cipher
	jmp	forever 	;else at far left already
ciplef10:
	mov	bx,cippos	;turn off previous coordinates
	mov	si,26		;offset of cipher rows
	mov	al,07h
	call	coord
	test	flag2,02h	;refresh mark if needed
	jz	ciplef15
	mov	dl,70h
	call	markcol
ciplef15:
	sub byte ptr cippos,2	;move cursor left
	jmp	area220

;scroll cipher to left
ciplef20:
	dec	curcol1
	jmp	ciprit40

;solution cursor left
sollef:
	cmp byte ptr solpos,50	;don't cursor off left margin
	ja	sollef10
;at left margin, try to scroll
	cmp	curcol2,0	;don't scroll off solution
	ja	sollef20
	jmp	forever
sollef10:
	mov	bx,solpos	;dull current coordinates
	mov	si,94
	mov	al,07h
	call	coord
	sub byte ptr solpos,2	;two cursor positions for every column
	jmp	area320 	;show new cursor position and coordinates
sollef20:
	dec	curcol2 	;scroll left one column
	jmp	solrit40

;blank, +, - only, and only in route area
blank:
punct:
	test	flag2,01h
	jz	dir05
	jmp	forever
dir05:
	cmp	area,0		;meaningful only in route area
	je	dir10
	jmp	forever
dir10:
	mov	cl,byte ptr rtepos+1
	sub	cl,2		;adjust current location to origin 0
	mov	al,1
	shl	al,cl		;bit position corresponds to route
	cmp byte ptr savechar,' '
	je	dirpos		;positive direction
	cmp byte ptr savechar,'+'
	je	dirpos
	cmp byte ptr savechar,'-'
	je	dirneg		;inverse direction
	jmp	forever 	;not a meaningful character
dirpos:
	not	al		;for positive, turn off the bit
	and	rtflag,al
dircom:
	xor	ax,ax		;figure screen location
	mov	al,byte ptr rtepos+1
	mul	c160
	mov	di,ax
	mov	ax,savechar	;get the character
	mov	es,[bp].videoa
	mov	es:[di],al	;..and show it on the screen
	jmp	forever
dirneg:
	or	rtflag,al      ;for inverse, turn on the bit
	jmp	dircom

;carriage return, a route or a dimension or log choice is selected
endline:
	and	flag1,0BFh	;turn off extant solution
	xor	bx,bx		;jump to rtsel or dmsel
	test	flag2,01h
	jz	endl10
	mov	bl,byte ptr logpos+1
	sub	bx,2
	shl	bx,1
	jmp	logkind[bx]
endl10:
	mov	bl,area
	shl	bx,1
	jmp	select[bx]
;a route is selected
rtsel:
	test	flag1,10h	;if takeout route
	jz	inroute
	jmp	outroute
inroute:
	or	flag1,04h	;enable going to dimension list
	push	bp		;global area ptr if ever needed
	mov	ax,ciphera	   ;->output indexes
	push	ax
	mov	ax,indexb
	push	ax
	les	di,dword ptr dimenb	;es:di->dimension table
	call	getdir	    ;get direction of route in ax, route index in bx
	mov	chosen2,ax	;direction, 0 = normal, 1 = inverse
	push	ax		;pass direction in parameter list
	mov	cx,dimn 	;number of dimensions
	mov	chosen1,bx	;selected route index
	shl	bx,1
	shl	bx,1		;call table index
rtelp:
	mov	ax,es:[di].c
	push	ax		;number of cols
	mov	ax,es:[di].r
	push	ax		;number of rows
	call	iotagen[bx]	;generate the indexes for this route
	add	sp,4		;clear previous row and column
;compute the variance
	call	dovar		;get variance in st(0)
	fstp	es:[di].varnce	;..and store it away for this dimension
	add	di,type dimen	;next dimensions
	loop	rtelp		;do for all dimensions
	add	sp,8		;clear the stack
	call	showdim 	;show the dimensions and their vowel variance
	test	flag1,20h	;if dimension was chosen 1st
	jnz	rtsel20 	;..show effect of this route per dimension
	mov	si,offset msg5	;else tell user to now choose dimension
	mov	di,24*160+80
	call	showmsg
	or	flag1,08h	;flag msg to clear
	mov	area,1		;force switch to dimension area
	mov	dx,dimpos
	call	setcurs
	jmp	forever
;dimension was chosen 1st.  Show results of this route
rtsel20:
	jmp	dmsel10


;a route is selected for output
outroute:
	push	bp		;global area ptr if ever needed
	mov	ax,ciphera	;->output indexes
	push	ax
	mov	ax,indexb
	push	ax
	les	di,dword ptr dimenb	;es:di->dimension table
	call	getdir	    ;get direction of route in ax, route index in bx
	push	ax		;pass direction in parameter list
	mov	chosen9+2,ax	;out route direction selected
	mov	chosen9,bx	;out route selected
	mov	dx,bx		;save route index
	mov	cx,3
	mov	bx,chosen	;get the dimensions
	shl	bx,cl
	mov	ax,chosen4[bx]	;..column
	push	ax		;..to parameter list
	mov	chosen9+6,ax	;..and mark
	mov	ax,chosen3[bx]	;..row
	push	ax		;..to parameter list
	mov	chosen9+4,ax	;..and mark
	mov	bx,dx		;restore route index
	shl	bx,1
	shl	bx,1		;call table index
	call	iotagen[bx]	;generate indexes for this route, dimensions
	add	sp,12
;permute the ciphertext for this route and dimensions
	mov	ax,ciphera	;->source, rectangle
	push	ax		;->source, rectangle
	mov	ax,rectb
	push	ax
	mov	ax,ciphera
	push	ax		;->destination, solved area
	mov	ax,solveb
	push	ax
	mov	ax,ciphera
	push	ax		;->indexes
	mov	ax,indexb
	push	ax
	mov	ax,letcnt
	push	ax		;number of elements to permute
	call	permg		;permute the cipher letters
	add	sp,14
	call	showout 	;show results of this out route
	or	flag1,40h	;flag extant solution
	jmp	forever


;a dimension is selected
dmsel:
	test	flag1,20h	;if choosing dimension 1st
	jz	dmsel10
	xor	ax,ax		;fix this selected dimension
	mov	al,byte ptr dimpos+1
	sub	ax,11		;index of dimension list item
	mov	chosen3,ax
	mov	ax,type dimen
	mul	chosen3 	;offset to dimension item
	les	di,dword ptr dimenb
	add	di,ax		;es:di->dimension list item
	mov	ax,es:[di].c	;get the row
	mov	chosen4,ax
	mov	ax,es:[di].r	;get the column
	mov	chosen3,ax
	mov	si,offset msg7	;tell user to now choose a route
	jmp	altp10		;..go to route choice

dmsel10:
	or	flag1,01h	;enable going to cipher rectangle
	push	bp		;global area ptr if ever needed
	mov	ax,ciphera
	push	ax		;->output indexes
	mov	ax,indexb
	push	ax
	mov	ax,chosen2	;get route direction
	push	ax
	xor	ax,ax		;get row and column coordinates
	mov	al,byte ptr dimpos+1
	sub	ax,11		;index of dimension list item
	mov	chosen3,ax
	mov	ax,type dimen
	mul	chosen3 	;offset to dimension item
	les	di,dword ptr dimenb
	add	di,ax		;es:di->dimension list item
	mov	ax,es:[di].c	;get the row
	mov	chosen4,ax
	push	ax
	mov	ax,es:[di].r	;get the column
	mov	chosen3,ax
	push	ax
	mov	bx,chosen1	;generate chosen route index
	shl	bx,1		;..for chosen dimension
	shl	bx,1
	call	iotagen[bx]
	add	sp,12
	mov	ax,ciphera
	push	ax		;->source
	mov	ax,cipherb
	push	ax
	mov	ax,ciphera
	push	ax		;->destination
	mov	ax,rectb
	push	ax
	mov	ax,ciphera
	push	ax		;->indexes
	mov	ax,indexb
	push	ax
	mov	ax,letcnt
	push	ax		;number of elements to permute
	call	permf		;permute the cipher letters
	add	sp,14
	mov	chosen,0
selcom:
	mov	flag2,0
	mov	curcol1,0	;cipher leftmost column
	mov	currow1,0	;cipher topmost row
	mov	curcol2,0	;solution leftmost column
	mov	currow2,0	;solution topmost row
	mov	solrows,0	;number of rows in solution area
	mov	solcols,0	;number of columns in solution area
	mov	cippos,0310h	;cursor position for cipher area
	mov	solpos,0332h	;cursor position for solving area
;clone permuted ciphertext for refreshing
	mov	es,ciphera
	mov	di,refresh
	mov	cx,letcnt
	mov	si,rectb
	push	ds
	push	es
	pop	ds
	rep movsb
	pop	ds
;initialize the row indexes for the solution area
	mov	bx,chosen
	mov	cx,3
	shl	bx,cl
	mov	cx,chosen3[bx]		;number of rows, this permutation
	mov	di,colptrb
	add	di,chosen4[bx]		;fix row index address
	add	di,chosen4[bx]
	mov	rowptrb,di
	xor	ax,ax
selcomlp:
	stosw
	inc	ax
	loop	selcomlp
;display present results
	mov	di,offset klarcip  ;screen destination
	call	klar		;blank out previous cipher
	mov	di,offset klarsol  ;blank out previous solution
	call	klar
	call	showcol 	;show column numbers for cipher
	mov	di,3*160+26	;screen destination
	call	showrow 	;show row numbers for cipher
	xor	dx,dx		;show cipher
	call	showcip 	;show cipher for this row,col origin
	jmp	forever

;logging routines
logrte: 		;log a route transposition
	call	dim2asc 	;format the in dimensions
	xor	bx,bx
	mov	bl,trankey
	cmp	chosen2,0	;if forward route
	je	logrt10 	;..blank is same as +
	mov	trankey+1[bx],'-' ;else mark reversed route
	inc	bx
logrt10:
	mov	ax,chosen1	;format in route as ascii
	call	rt2asc
	cmp	chosen9+2,0	;check out route direction
	je	logrt20
	mov	trankey+1[bx],'-'
	inc	bx
logrt20:
	mov	ax,chosen9	;format out route as ascii
	call	rt2asc
	mov	trankey,bl	;total length of key
	mov	logarea.kind,0	;mark route transposition
logcom:
	mov	logarea.xposkyb,offset trankey
	mov	logarea.xposkya,ds
	mov	ax,letcnt
	mov	logarea.xposcnt,ax	;count of plaintext letters
	mov	ax,solveb	;plaintext location
	mov	logarea.xposptb,ax
	mov	ax,ciphera
	mov	logarea.xpospta,ax
	mov	[bp].lgptrb,offset logarea
	mov	[bp].lgptra,ds ;tell log routine where parameters are
	mov	ax,2		;complete transpostion logging
	call	log		;log this solution
	jnc	logrt30
	or	flag1,08h	;log error, flag msg to clear
logrt30:
	jmp	f205		;leave logging mode

logcol:
	mov	logarea.kind,1	;column transposition kind
logcol10:
	call	dim2asc 	;format dimensions for logging
	call	col2asc 	;format permuted columns for logging
	jmp	logcom		;do the logging

lognih:
	mov	logarea.kind,2	;nihilist transposition
	jmp	logcol10

;change windows
;go to next area
nxtwind:
	test	flag2,01h	;not valid in logging mode
	jz	nxtw10
	jmp	forever
nxtw10:
	or	flag1,80h	;going forward
	xor	bx,bx
	mov	bl,area
	shl	bx,1
	jmp	avance[bx]
;go to previous area
prewind:
	test	flag2,01h
	jz	prew10
	jmp	forever
prew10:
	and	flag1,7Fh	;going backwards
	xor	bx,bx
	mov	bl,area
	shl	bx,1
	jmp	reverse[bx]
;route list destination
area0:
	mov	area,0			;set area to route list
	mov	dx,rtepos		;set cursor at route list
	call	setcurs
	jmp	forever
;dimension list destination
area1:
	test	flag1,04h	;if no route yet selected
	jnz	area110
	mov	si,offset msg3	;..say select a route
	mov	di,24*160+80
	call	showmsg
	or	flag1,08h
	jmp	forever
area110:
	xor	ax,ax		;turn on hiliting in case it isn't on
	mov	al,byte ptr dimpos+1	;screen position
	mul	c160
	mov	di,ax
	mov	cx,12			;length of screen item
	call	hell			;hilite current line
	mov	area,1			;set area to dimension list
	mov	dx,dimpos		;set cursor at dimension list
	call	setcurs
	jmp	forever

;cipher rectangle destination
area2:
	test	flag1,01h		;if it is possible to goto cipher area
	jnz	area210 		;..do it
area205:
	test	flag1,80h		;else if going forward
	jnz	area0			;..go to route list
	jmp	area1			;else goto dimension list
area210:	;switch to cipher list area
	mov	area,2
area220:
	mov	dx,cippos
	call	setcurs
	mov	bx,dx		;brighten coordinates
	mov	si,26		;offset to cipher area rows
	mov	al,0Fh
	call	coord
	jmp	forever

;solving area destination
area3:
	test	flag1,01h		;if possible to go to solving area
	jnz	area310 		;..do it
	jmp	area205 		;else skip some areas
area310:	;switch to solving area
	mov	area,3
area320:
	mov	dx,solpos
	call	setcurs
	mov	bx,dx		;brighten coordinates
	mov	si,94		;offset to solution area rows
	mov	al,0Fh
	call	coord
	jmp	forever

;reverse along row axis (reverses columns)
a1:
	mov	ax,7		;row axis reversal is 7
	jmp short xfrmcom	;join common code
;reverse along column axis (reverses rows)
a2:
	mov	ax,8		;column axis reversal is 8
	jmp short xfrmcom
;transpose axes (transpose)
a3:
	mov	ax,9		;transpose is 9
	jmp short xfrmcom
;transpose axes and reverse rows and columns (anti-transpose)
a4:
	mov	ax,10		;anti-transpose is 10
xfrmcom:
	test	flag1,01h	;both route and dimension must be chosen
	jnz	xfrm00
	jmp	forever 	;..or ignore row axis reverse
xfrm00:
	test	flag2,01h
	jz	xfrm10
	jmp	forever
xfrm10:
	mov	dx,ciphera
	push	dx		;source to be transformed
	mov	dx,rectb
	push	dx
	mov	dx,ciphera
	push	dx		;index generation area
	mov	dx,indexb
	push	dx
	mov	dx,ciphera
	push	dx		;work area either index or permutation
	mov	dx,workb
	push	dx
	mov	bx,chosen	;get current chosen dimensions
	mov	cx,3
	shl	bx,cl
	mov	chosen5,ax	;mark this transform
	mov	ax,chosen4[bx]	;get column of this transform
	mov	dx,chosen3[bx]	;get row of this transform
	push	ax		;column is parameter
	push	dx		;row is parameter
	cmp	chosen5,9		;reverse row,col for transposes
	jb	xfrm20			;keep row,col order for reverses
	xchg	ax,dx
xfrm20:
	mov	chosen5+6,ax	;mark column dimension
	mov	chosen5+4,dx	;mark row dimension
	mov	chosen,1	;mark transform dimensions
	mov	bx,chosen5	;get the transform again
	shl	bx,1
	shl	bx,1
	call	iotagen[bx]	;call transform routine
	add	sp,16		;collapse the parm stack
	jmp	selcom		  ;display transformed cipher

;fix input route and dimension
altp:
	test	flag2,01h
	jz	altp05
	jmp	forever
altp05:
	cmp	solcols,0	;if no anagramming
	jz	altp07		;..assume route transposition
	jmp	altp20		;else check for complete anagramming
altp07:
	or	flag1,10h	;flag input route and dimension chosen
	mov	si,offset msg6	;tell user to choose output route
altp10:
	mov	di,24*160+80
	call	showmsg
	or	flag1,08h	;message to clear
	mov	area,0		;force route selection
	xor	ax,ax		;clear hiliting
	mov	al,byte ptr rtepos+1
	mul	c160
	mov	di,ax
	mov	cx,12
	call	dunkel
	mov	di,2*160	;new hilite
	call	hell
	mov	rtepos,0200h	;reset to start of list
	mov	dx,rtepos
	call	setcurs
	jmp	forever
;column anagramming
altp20:
	mov	bx,chosen	;find the number of columns
	mov	cx,3
	shl	bx,cl
	mov	ax,solcols
	cmp	ax,chosen4[bx]	;if all columns accounted for
	je	altp30		;..allow choice of out route
	mov	si,offset msg15 ;else tell user to complete the solution
	jmp	f7xit
altp30: 			;restore permuted ciphertext
	mov	cx,letcnt
	mov	es,ciphera
	mov	di,rectb	;es:di->ciphertext work area
	mov	si,solveb
	push	ds
	push	es
	pop	ds		;ds:si->anagrammed ciphertext
	rep movsb
	pop	ds
altp40:
	mov	di,offset klarcip  ;clean up the screen
	call	klar
	mov	di,offset klarsol
	call	klar
	call	showcol 	;show anagrammed cipher text
	mov	di,3*160+26
	call	showrow
	xor	dx,dx
	call	showcip
	jmp	altp07		;restore to route selection

;nihilist transposition.  permute rows like the columns
altn:
	mov	bx,chosen	;find the number of columns
	mov	cx,3
	shl	bx,cl
	mov	ax,solcols
	cmp	ax,chosen4[bx]	;if all columns accounted for
	je	altn10		;..check that it is a nihilist
	mov	si,offset msg15 ;else tell user to complete the solution
	jmp	f7xit
altn10:
	cmp	ax,chosen3[bx]	;rows must be same as columns
	je	altn20
	mov	si,offset msg16 ;else tell user cipher isn't nihilist
	jmp	f7xit
altn20:
	mov	[bp].rowcnt,ax	;number of rows, or columns
	mov	cx,ax
	mov	es,ciphera
	mov	bx,colptrb	;es:bx->column indexes
	mov	si,solveb	;es:si->column permuted solution
	mov	di,rectb	;es:di->cipher work area
	push	ds		;move the rows in column order
	push	es
	pop	ds
altnlp:
	mov	ax,es:[bx]	;row index
	mul	[bp].rowcnt	;ith row
	push	si
	add	si,ax		;ds:si->ith row
	push	cx
	mov	cx,[bp].rowcnt	;number of columns to move
	rep movsb
	pop	cx
	pop	si
	add	bx,2		;next column index
	loop	altnlp
	pop	ds
	jmp	altp40		;redisplay, enable takeout

;dimension/route selection toggle
;if dimension, fix selected dimension and show cipher routes as selected
;if route, fix selected route and show cipher dimensions as selected
f8:
	test	flag2,01h
	jz	f802
	jmp	forever
f802:
	and	flag1,0EFh	;turn off out route
	cmp	dimn,1		;if there is no choice of dimensions
	jne	f805
	jmp	forever 	;..ignore the toggle
f805:
	xor	flag1,20h	;toggle previous setting
	test	flag1,20h	;if route first
	jnz	f810
	mov	si,offset msg7	;..tell user to choose in route
	jmp	altp10		;..and force route selection
f810:				;else dimension first
	cmp	dimn,1		;if there is only one dimension choice
	jne	f820
	les	di,dword ptr dimenb	;..consider it chosen
	mov	ax,[di].r	;..fix the only row choice
	mov	chosen3,ax
	mov	ax,[di].c	;..fix the only column choice
	mov	chosen4,ax
	mov	si,offset msg8	;..tell user only dimension is chosen
	jmp	altp10
f820:
	mov	si,offset msg9	;tell user to choose a dimension
	mov	di,24*160+80
	call	showmsg
	or	flag1,08h	;message to clear
	mov	area,1		;force dimension selection
	mov	dimpos,0B00h	;reset to start of dimensions
	mov	dx,dimpos
	call	setcurs
	jmp	forever

;log solution
f7:
	test	flag1,40h	;if no current solution
	jnz	f710
	mov	si,offset msg4	;..tell user
f7xit:
	mov	di,24*160+80
	call	showmsg
	or	flag1,08h
	jmp	forever 	;..and disallow logging
;find out what kind of transposition to record keys properly
f710:
	test	flag2,01h	;if in logging mode already
	jz	f720
	jmp	forever 	;..ignore request
f720:
	call	getcurs 	;save current cursor position
	mov	savepos,dx
	mov	si,offset kinds ;display logging window
	mov	es,[bp].videoa
	mov	di,160+28	;screen destination for window
	mov	cx,5		;five lines
	mov	ah,07h		;normal attribute
f7lp1:
	push	di
	push	cx
	mov	cx,12		;length of a line
f7lp2:
	lodsb
	stosw
	loop	f7lp2
	pop	cx
	pop	di
	add	di,160
	loop	f7lp1
	mov	di,2*160+32	  ;hilite the first choice
	mov	cx,8
	call	hell
	mov	dx,0210h
	mov	logpos,dx
	call	setcurs
	or	flag2,01h	;mark logging mode
	mov	si,offset msg10 ;tell user to designate cipher type
	jmp	f7xit

;mark column for moving
altq:
	test	flag2,01h	;ignore mark if logging
	jz	altq00
	jmp	forever
altq00:
	cmp	area,2		;ignore mark if not in cipher or solution
	jnb	altq10
	jmp	forever
altq10:
	test	flag2,02h	;if mark already pending
	jz	altq20
	mov	si,offset msg11 ;..tell user
	jmp	f7xit		;..and ignore
altq20:
	cmp	area,2
	je	aqcc		;cipher column marking
	jmp	aqsc		;solution column marking
;cipher area column marking
aqcc:
	xor	ax,ax		;get the current column index
	mov	al,byte ptr cippos
	sub	al,16		;..origin 0
	shr	ax,1
	add	ax,curcol1
;test if this column is empty
	mov	cx,solcols
	jcxz	aqcc10		;nothing moved, can't be empty
	mov	es,ciphera
	mov	di,colptrb	;es:di->moved columns
	repne scasw
	jne	aqcc10		;not moved yet
	mov	si,offset msg12 ;tell user column is empty
	jmp	f7xit		;..and ignore marking
aqcc10:
	or	flag2,02h	;pending mark
	mov	markdcol,ax	;absolute column number marked
	mov	al,area
	mov	markarea,al	;..and area
;show the column marked
	xor	ax,ax
	mov	al,byte ptr cippos
	shl	ax,1
	mov	di,3*160
	add	di,ax
	mov	es,[bp].videoa	;es:di->column start on screen
	mov	bx,chosen
	mov	cx,3
	shl	bx,cl
	mov	cx,chosen3[bx]	;number of rows
	cmp	cx,18		;no more than 18 rows
	jna	aqcclp
	mov	cx,18
aqcclp:
	mov byte ptr es:[di+1],70h	;reverse video
	add	di,160		;next row, same column
	loop	aqcclp		;do for all rows
	jmp	forever

;solution area column marking
aqsc:
	jmp	forever

;unmark column or row mark
altw:
	test	flag2,02h		;if no mark
	jnz	altw00
	jmp	forever 		;..it's already unmarked
altw00:
	mov	dl,07h			;normal intensity
	call	markcol
	and	flag2,0FDh		;turn off pending mark
	mov	bx,cippos
	mov	al,0Fh			;restore hilite in case mark
	mov	si,26			;happens to be in cursor area
	call	coord
	jmp	forever

;move marked column before cursor position
alta:
	call	movcom		;no return if error
;es:di->index destination, cx = number of indexes to move
alta00:
	jcxz	alta10		;nothing to scoot
	mov	si,di
	sub	si,2
	push	ds
	mov	ax,ciphera
	mov	ds,ax		;ds:si->last index
	std
	rep movsw		;make a hole for new index
	cld
	pop	ds
;move in new index
alta10:
	mov	ax,markdcol	;number of marked column
	mov	es:[di],ax	;..to permuted col index area
	inc	solcols 	;one more solved column, we hope
	call	cipsol		;display cipher and solution
	mov	bx,solpos	;hilite coordinates
	mov	si,94
	mov	al,0Fh
	call	coord
	jmp	forever

;move marked column after cursor position
alts:
	call	movcom		;no return if error
	jcxz	alts10		;skip move if nothing to move
	dec	cx		;..else adjust for move after
alts10:
	jmp	alta00		;..and join common code

;undo solution column (restore to ciphertext)
altu:
	cmp	area,3		;has no meaning
	je	altu00		;..except in solution area
	jmp	forever
altu00:
	cmp	solcols,0	;..and if there are solution
	ja	altu10		;..columns in effect
	jmp	forever
altu10:
	xor	bx,bx		;figure out what column to restore
	mov	bl,byte ptr solpos	;screen column
	sub	bx,50			;..less offset from screen left margin
	shr	bx,1			;..2 screen columns per cipher col
	add	bx,curcol2		;..plus offset from cipher left margin
	mov	scol,bx 	;save for later cursor adjustment
	mov	cx,solcols
	sub	cx,bx		;number of indexes to scoot left
	dec	cx
	jcxz	altu15
	shl	bx,1		;adjust for words
	mov	di,bx
	add	di,colptrb
	mov	es,ciphera	;es:di->column index to be squeezed out
	mov	si,di
	add	si,2
	push	ds
	push	es
	pop	ds
	rep movsw		;squeeze out the index number
	pop	ds
altu15:
	dec	solcols
	mov	di,offset klarsol  ;clear the solution display
	call	klar
	call	cipsol		;show both new cipher and solution
	mov	ax,scol 	;adjust cursor if needed
	cmp	ax,solcols
	jb	altu20		;cursor still in solution
	jmp	sollef		;..cursor solution leftward
altu20:
	jmp	forever

;help
sf1:
	mov	ax,5		;number of help screen
	call	help
	jmp	forever
useasis:
rubout:
tab:
backtab:
escape:
numeral:
enguc:
englc:
del:
euruc:
eurlc:
null:
alte:
altr:
altt:
alty:
alti:
alto:
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
altm:
f1:
f3:
f4:
f5:
f6:
f9:
f10:
home:
pagup:
endkey:
pagdn:
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
bottom:
botpg:
top:
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




xpose	endp

;check al for membership in group0, that is, for English, is it a vowel?
; if vowel, carry = no, if not vowel, carry = yes
checkv	proc	far
	push	cx
	push	es
	push	di
	push	ds
	mov	cx,crypt21b		;addressability
	mov	ds,cx
	mov	es,cx
	mov	di,offset group0
	mov	cx,[di] ;number of language dependent constant characters
	add	di,2	;es:di->vowels or language dependent constant chars
	repne scasb
	je	chkv20
	stc		;not a constant char
chkvret:
	pop	ds
	pop	di
	pop	es
	pop	cx
	ret
chkv20:
	clc	       ;is a constant char
	jmp	chkvret
checkv	endp

;display the dimensional choices
showdim proc	near
	mov	cx,dimn
	sub	cx,curdim	;number of dimensions to display
	cmp	cx,13		;but no more than 13
	jna	shwdm10
	mov	cx,13
shwdm10:
	mov	ax,type dimen	;figure top of box dimension
	mul	curdim
	mov	di,ax
	add	di,dimenb
	mov	es,dimena	;es:di->top dimension
	mov	bx,11*160	;->screen line
shwdmlp:
	call	formdim 	;format and display this dimension
	add	di,type dimen	;next dimension
	add	bx,160		;next screen line
	loop	shwdmlp 	;do for all dims that will fit on this screen
	ret
showdim endp

formdim proc	near
	push	bx
	push	cx
	push	es
	push	di
;display row dimension at screen location bx
	mov	ax,es:[di].r	;row
	mov	si,offset ascfp ;->work area
	call	int2asc
	add	si,3		;->last two significant digits
	push	es
	push	di
	mov	es,[bp].videoa
	mov	di,bx		;es:di->screen location
	mov	cx,12
	mov	ax,0720h
	rep stosw		;clear the screen line
	mov	cx,2
	mov	di,bx
frmdmlp1:
	lodsb
	stosw
	loop	frmdmlp1
	pop	di
	pop	es
;display column dimension at bx location on screen
	add	bx,6		;next screen location
	mov	ax,es:[di].c	;column
	mov	si,offset ascfp
	call	int2asc
	add	si,3		;->last two significant digits
	push	es
	push	di
	mov	es,[bp].videoa
	mov	cx,2
	mov	di,bx		;es:di->screen location
	mov	ah,07h
frmdmlp2:
	lodsb
	stosw
	loop	frmdmlp2
	pop	di
	pop	es
;display vowel variance at bx location on screen
	add	bx,6		;next screen location
	fld	es:[di].varnce
	push	bp
	sub	sp,4
	mov	bp,sp
	mov word ptr [bp],offset ascfp
	mov	[bp+2],ds
	mov	ax,4		;four significant digits
	call	fp2asc
	add	sp,4
	pop	bp
	mov	es,[bp].videoa
	mov	di,bx
	xor	cx,cx
	mov	cl,ascfp	;length of string
	mov	si,offset ascfp+1
	mov	ah,07h
	cmp	cx,6		;no more than 5 bytes
	jna	frmdm10
	mov	cx,6
frmdm10:
	lodsb
	stosw
	loop	frmdm10
	pop	di
	pop	es
	pop	cx
	pop	bx
	ret
formdim endp

;di->screen offset
;cx = number of bytes to hilight
hell	proc	near
	mov	es,[bp].videoa
	mov	al,0Fh		;bright attribute
hellp:
	inc	di
	stosb
	loop	hellp
	ret
hell	endp

;di->screen position
;cx = number of bytes to restore to normal intensity
dunkel	proc	near
	push	cx
	push	di
	mov	es,[bp].videoa
	mov	al,07h		;normal attribute
dunklp:
	inc	di
	stosb
	loop	dunklp
	pop	di
	pop	cx
	ret
dunkel	endp

;compute vowel variance for this route
;es:di->dimension table, must be preserved
;cx = number of table entries, not used, but must be preserved
dovar	proc	near
	push	cx
	push	si
	fldz			;accum
;compute expected number of vowels in a row, xbar
	fild	vowel		;vowels 	accum
	fild	letcnt		;letters	vowels	accum
	fdiv			;vowels/letters accum
	fild	es:[di].c	;cols	vowelratio	accum
	fmul			;xbar	accum
	mov	cx,es:[di].r	;for every row
	mov	si,indexb
dovarlp:
	call	cntvowl 	;count vowels in this row
	mov	vcount,dx
	fild	vcount		;rowvowels	xbar	accum
	fsub	st,st(1)	;vowels-xbar	xbar	accum
	fmul	st,st(0)	;(vow-xbar)**2	xbar	accum
	faddp	st(2),st	;xbar		accum
	add	si,es:[di].c	;next bunch of indexes
	add	si,es:[di].c
	loop	dovarlp
	fstp	st(0)		;accum		get rid of xbar
	fild	es:[di].r	;row	accum	rows are number of counts
	fdiv			;variance in st(0)
	pop	si
	pop	cx
	ret
dovar	endp

;count vowels in a row, rather complex structure
;si->indexes for this row
;es:di->dimension table
;ciphera:cipherb->cryptogram letters
;for column times, get letters in cipher and test for vowels.
;return vowel count in dx
cntvowl proc	near
	push	cx
	push	es
	push	di
	push	bx
	push	si
	push	ds
	xor	dx,dx		;vowel counter
	mov	cx,es:[di].c	;number of letters in a row is number of cols
	mov	es,ciphera
	mov	di,si		;es:di->indexes
	lds	si,dword ptr cipherb	;ds:si->ciphertext
cntvlp:
	mov	bx,es:[di]	;get an index
	mov	al,[si][bx]	;get indexed letter
	call far ptr checkv	;if it is a vowel
	jc	cntv10
	inc	dx		;..count it
cntv10:
	add	di,2		;next index
	loop	cntvlp
	pop	ds
	pop	si
	pop	bx
	pop	di
	pop	es
	pop	cx
	ret
cntvowl endp

;show results of this out route
;solveb->output text
showout proc	near
	mov	di,offset klarsol   ;screen display area
	call	klar		;blank output display area
	mov	ax,letcnt
	xor	dx,dx
	div	c33		;number of lines to display
	cmp	ax,20		;show no more than 20 lines
	jna	shwout10
	mov	ax,20
shwout10:
	mov	cx,ax
	push	ax		;save lines to test showing remainder, dx
	mov	ah,07h		;attribute
	mov	es,[bp].videoa
	mov	di,160+94	;es:di->screen destination
	mov	si,solveb
	mov	ds,ciphera	;ds:si->fixed permuted cipher text
	jcxz	shwout20
shwoutl1:
	push	cx
	push	di
	mov	cx,33		;show one line
shwoutl2:
	lodsb
	stosw
	loop	shwoutl2
	pop	di
	pop	cx
	add	di,160		;next screen line
	loop	shwoutl1
shwout20:
	pop	cx		;restore lines shown
	cmp	cx,20		;if 33 lines shown
	je	shwout30	;..can't show remainder
	mov	cx,dx		;else show remainder
	jcxz	shwout30	;..if there are any remaining letters
shwoutl3:
	lodsb
	stosw
	loop	shwoutl3
shwout30:
	mov	ax,crypt21b	;restore addressability
	mov	ds,ax
	ret
showout endp

;blank out cipher or solving area
;di->screen area start parms
klar	proc	near
	push	dx
	mov	ax,0720h
	mov	es,[bp].videoa
	mov	cx,[di+2]	;rows to clear
	mov	dx,[di+4]	;columns to clear
	mov	di,[di] 	;origin of rect to clear
klarlp:
	push	di
	push	cx
	mov	cx,dx		;columns to clear
	rep	stosw		;clear this row
	pop	cx
	pop	di
	add	di,160		;next row
	loop	klarlp
	pop	dx
	ret
klar	endp

;show cipher column numbers for current position
showcol proc	near
	mov	bx,chosen	;get current column
	mov	cx,3
	shl	bx,cl
	mov	cx,chosen4[bx]	;get number of columns
	sub	cx,curcol1	;..to display
	cmp	cx,15		;but no more than 15 columns
	jna	shwcol10
	mov	cx,15
shwcol10:
	mov	bx,curcol1	;first column to show
	mov	es,[bp].videoa
	mov	di,160+32	;es:di->where to show it
shwcolp:
	inc	bx		;..origin 1
	mov	ax,bx
	xor	dx,dx
	div	c100		;col number mod 100
	mov	ax,dx
	xor	dx,dx
	div	c10
	or	ax,ax		;if tens digit is 0
	jz	shwcol20	;..make it a blank
	add	al,'0'          ;else convert to ascii
	jmp short shwcol30
shwcol20:
	mov	al,' '
shwcol30:
	add	dl,'0'
	mov	ah,07h		;normal attribute
	stosw			;..to the screen, tens above
	mov	al,dl
	mov	es:[di+158],ax	;..digits below
	add	di,2		;next screen position
	loop	shwcolp 	;do for all column numbers
	ret
showcol endp

;show solved rows, may be permuted
rowsol	proc	near
	mov	bx,chosen	;need number of rows, this permutation
	mov	cx,3
	shl	bx,cl
	mov	cx,chosen3[bx]	;number of rows
	mov	es,ciphera
	mov	bx,currow2	;starting row
	shl	bx,1
	add	bx,rowptrb	;es:bx->row indexes
	mov	di,3*160+94	;row numerals screen destination
	cmp	cx,18
	jna	rowsolp
	mov	cx,18
rowsolp:
	mov	ax,es:[bx]	;this row number
	inc	ax		;..origin 1
	xor	dx,dx
	div	c100		;use row number mod 100
	mov	ax,dx
	xor	dx,dx
	div	c10
	add	dl,'0'          ;..digit in ascii
	mov	dh,07h		;..normal attribute
	push	es
	mov	es,[bp].videoa
	mov	es:[di+2],dx
	mov word ptr es:[di],0720h	;blank ten's digit
	or	ax,ax		;if ten's digit is 0
	jz	rowsol10	;..leave screen digit blank
	add	al,'0'          ;else make ascii digit
	mov	ah,07h		;..normal attribute
	mov	es:[di],ax
rowsol10:
	pop	es
	add	di,160		;next screen row
	add	bx,2		;next row coordinate
	loop	rowsolp
	ret
rowsol	endp

;show row numbers in cipher area
;di->screen area
showrow proc	near
	mov	bx,chosen	;get current row
	mov	cx,3
	shl	bx,cl
	mov	cx,chosen3[bx]	;get number of rows
	sub	cx,currow1	;..to display
	cmp	cx,18		;..but no more than 18
	jna	shwrow10
	mov	cx,18
shwrow10:
	mov	bx,currow1	;starting row number
	mov	es,[bp].videoa
shwrowlp:
	inc	bx		;..origin 1
	mov	ax,bx		;row number
	mov	si,offset ascfp
	call	int2asc 	;convert to ascii
	push	di
	push	cx
	add	si,3		;point at last 2 digits
	mov	cx,2
	mov	ah,07h		;normal attribute
shwrwlp2:
	lodsb
	stosw
	loop	shwrwlp2
	pop	cx
	pop	di
	add	di,160		;next row
	loop	shwrowlp
	ret
showrow endp

;show cipher or solution as a rectangle
showcip proc	near
	mov	ax,currow1	;assume showing cipher
	mov	bx,curcol1
	mov	di,3*160+32
	mov	si,rectb
	or	dx,dx		;test assumption
	jz	shwcip05	;..showing cipher
	mov	ax,currow2	;showing solution
	mov	bx,curcol2
	mov	di,3*160+100
	mov	si,solveb
shwcip05:
	mov	thisrow,ax
	mov	thiscol,bx
	mov	screenp,di
	mov	sourcep,si
	mov	bx,chosen
	mov	cx,3
	shl	bx,cl
	mov	ax,thisrow	;compute origin row and column
	mul	chosen4[bx]	;beginning row
	add	ax,thiscol	;..plus beginning column
	mov	si,ax		;..save origin for later
	mov	dx,chosen4[bx]	;row dimensional length
	mov	cx,chosen3[bx]	;number of rows to show
	sub	cx,thisrow
	cmp	cx,18		;..but no more than 18
	jna	shwcip10
	mov	cx,18
shwcip10:
	mov	ax,chosen4[bx]	;number of columns to show
	sub	ax,thiscol
	cmp	ax,15		;..but no more than 15
	jna	shwcip20
	mov	ax,15
shwcip20:
	mov	bx,ax		;save col length for later
	mov	es,[bp].videoa
	mov	di,screenp	;screen destination
	add	si,sourcep	;offset to permuted cipher letters
	mov	ax,ciphera
	push	ds		;save addressability
	mov	ds,ax		;ds:si->permuted cipher letters
	mov	ah,07h		;normal attribute
shwciplp:
	push	cx
	push	si
	push	di
	mov	cx,bx		;move n column letters
shwcipl2:
	lodsb			;..from permuted cipher
	stosw			;..to screen
	add	di,2
	loop	shwcipl2
	pop	di
	pop	si
	pop	cx
	add	si,dx		;next cipher row,col
	add	di,160		;next screen row
	loop	shwciplp
	pop	ds		;restore addressability
	ret
showcip endp

;show permuted columns in solution area
showprm proc	near
	mov	cx,solcols	;number of columns to show
	sub	cx,curcol2	;from left margin
	cmp	cx,15		;but no more than 15
	jna	shwprm10
	mov	cx,15
shwprm10:
	mov	bx,curcol2
	add	bx,colptrb	;first column to show
	mov	di,160+100	;first screen position
	mov	es,[bp].videoa
shwprmlp:
	push	es
	xor	dx,dx
	mov	es,ciphera
	mov	ax,es:[bx]	;get column
	pop	es
	inc	ax
	div	c100		;col number mod 100
	mov	ax,dx
	xor	dx,dx
	div	c10
	or	ax,ax		;if there is a tens digit
	jz	shwprm20
	add	al,'0'          ;..convert to ascii numeral
	jmp short shwprm30
shwprm20:
	mov	al,' '          ;else convert 0s tens digit to blank
shwprm30:
	add	dl,'0'          ;convert units digit to ascii
	mov	ah,07h
	stosw			;tens to screen
	mov	al,dl
	mov	es:[di+158],ax	;units to screen
	add	di,2		;next screen position
	add	bx,2		;next column
	loop	shwprmlp
	ret
showprm endp

;return route direction in ax
getdir	proc	near
	mov	cl,byte ptr rtepos+1	;set bit mask to test for inverse
	sub	cl,2
	mov	bl,cl		;save for call to generator routine
	mov	ax,1
	shl	al,cl		;al = bit mask
	and	al,rtflag	;0 = normal, 1 = inverse
	ret
getdir	endp

;format dimension for logging
dim2asc proc	near
	mov	al,' '                  ;blank the ascii key area
	mov	cx,80
	mov	di,offset trankey+1
	push	ds
	pop	es
	rep stosb
	mov	trankey+1,'('
	mov	ax,chosen3	;row dimension of in route
	mov	si,offset ascfp
	call	int2asc
	mov	ax,word ptr ascfp+3
	mov word ptr trankey+2,ax
	mov	trankey+4,','
	mov	si,offset ascfp
	mov	ax,chosen4
	call	int2asc
	mov	ax,word ptr ascfp+3
	mov word ptr trankey+5,ax
	mov	trankey+7,')'
	mov	trankey,8	;length of ascii key so far
	ret
dim2asc endp

;bx = offset from trankey+1, ax = route index
rt2asc	proc	near
	inc	ax
	mul	c12
	mov	si,offset routes
	add	si,ax
	inc	si		;ds:si->route name
	lea	di,trankey+1[bx]
	push	ds
	pop	es
	mov	cx,10
	rep movsb
	add	bx,10		;adjust index according to bytes moved
	ret
rt2asc	endp

col2asc proc	near
	mov	es,ciphera
	mov	di,colptrb	;es:di->permuted columns
	mov	cx,chosen4	;number of columns
	xor	bx,bx
	mov	bl,trankey	;current position in key
	mov	si,offset ascfp
col2ascl:
	mov	ax,es:[di]	;col coordinate
	inc	ax		;index origin 1
	call	int2asc 	;..convert to ascii
	mov	ax,word ptr ascfp+3
	cmp	al,' '          ;if leading digit is blank
	je	col2asc3	;..only one digit
	cmp	bx,77
	jnb	col2ascx
	mov word ptr trankey+1[bx],ax
	add	bx,3
	jmp short col2com
col2asc3:
	cmp	bx,77
	jnb	col2ascx
	mov	trankey+1[bx],ah
	add	bx,2
col2com:
	add	di,2		;next column coordinate
	loop	col2ascl
col2ascx:
	mov	trankey,bl	;update key length
	ret
col2asc endp

;bx = row,col -- turn off row and column coordinates
;si = offset to cipher or solution rows
;al = 07h for dull, 0Fh for bright
coord	proc	near
	push	ax
	xor	ax,ax
	mov	al,bh		;compute the row
	mul	c160
	mov	di,ax
	pop	ax
	add	di,si
	mov	es,[bp].videoa
	mov	es:[di+1],al	  ;normal attribute for row coordinate
	mov	es:[di+3],al	  ;..both digits
	add	di,6		;do the whole row
	mov	cx,15
coordl1:
	mov	es:[di+1],al
	add	di,4
	loop	coordl1
	mov	dl,bl		;compute column
	shl	dx,1
	mov	di,160
	add	di,dx
	mov	es:[di+1],al	;dull column coordinate
	mov	cx,19
coordl2:
	add	di,160		;do the whole column
	mov	es:[di+1],al
	loop	coordl2
	ret
coord	endp

;show marked column in its new position after a scroll
;markdcol = column number
;markarea = area in which marked
;dl = 07h	restore normal intensity (unmark)
;dl = 70h	reverse video (mark)
markcol proc	near
	cmp	markarea,2
	je	mrkcol50
	ret			;solution area mark not implemented yet
mrkcol50:
	mov	ax,curcol1	;figure last column on screen
	add	ax,16		;last possible column
	mov	bx,chosen	;compare to actual last column
	mov	cx,3
	shl	bx,cl
	cmp	ax,chosen4[bx]	;if leftcol + 15 is beyond max columns
	jna	mrkcol60
	mov	ax,chosen4[bx]	;number of columns is last possible on screen
mrkcol60:
	dec	ax
	mov	di,markdcol
	cmp	ax,di
	jnb	mrkcol65
	ret
mrkcol65:
	sub	di,curcol1
	jnb	mrkcol70
	ret			;marked column left of left margin
mrkcol70:			;marked column on screen
	shl	di,1		;two screen cols per cipher column
	shl	di,1		;..plus two bytes per column
	add	di,3*160+32	;..plus area start
	mov	es,[bp].videoa
	mov	cx,chosen3[bx]	;number of rows
	cmp	cx,18
	jna	mrkcolp
	mov	cx,18
mrkcolp:
	mov byte ptr es:[di+1],dl	;reverse video
	add	di,160			;next row
	loop	mrkcolp 		;do for all rows
	ret
markcol endp

;set up to move solved column indexes, no return to caller if error
;on return, es:di->move index destination
;	    cx = number of indexes to move for before move
movcom	proc	near
	cmp	area,3		;can move marks only into area 3
	je	mvcom00
	mov	si,offset msg13 ;tell user not in right area
	pop	ax		;clear stack
	jmp	f7xit
mvcom00:
	test	flag2,02h	;must be a mark to move it
	jnz	mvcom05
	mov	si,offset msg14 ;tell user no mark to move
	pop	ax		;clear stack
	jmp	f7xit
mvcom05:
	xor	ax,ax		;figure number of columns to scoot
	mov	al,byte ptr solpos
	sub	ax,50		;index origin 0
	shr	ax,1		;compensate for display's doubled columns
	add	ax,curcol2	;destination column
	mov	dx,ax		;temp save
	mov	ax,solcols	;number of solved columns
	sub	ax,dx		;number of indexes to scoot
;scoot indexes to make room for moved index
	mov	cx,ax		;number of indexes to scoot
	mov	di,solcols
	shl	di,1
	add	di,colptrb
	mov	es,ciphera	;es:di->end of indexes
	ret
movcom	endp

;display cipher and anagrammed solution
cipsol	proc	near
;refresh permuted cipher
	mov	es,ciphera
	mov	di,rectb
	mov	si,refresh
	mov	cx,letcnt
	push	ds
	push	es
	pop	ds
	rep movsb
	pop	ds
;clear solution area
	mov	di,solveb
	mov	cx,letcnt
	mov	al,' '
	rep stosb
;for all permuted columns
;  move column to solved area
;  blank moved column in cipher area
	mov	bx,chosen	;we will need cipher's dimensions
	mov	cx,3
	shl	bx,cl
	mov	ax,chosen3[bx]	;rows
	mov	[bp].rowcnt,ax	;..save for later
	mov	ax,chosen4[bx]	;columns
	mov	[bp].colcnt,ax	;..save for later
	mov	bx,colptrb	;es:bx->column indexes
	mov	dx,rectb	;ds:dx will point to permuted ciphertext
	mov	cx,solcols	;number of columns to retrieve
	jcxz	cipsol20
	mov	di,solveb	;es:di->solved area destination
	push	ds
	push	es
	pop	ds
altalp1:
	push	di
	push	cx
	mov	si,dx
	add	si,es:[bx]	;ds:si->this column to move
	mov	cx,[bp].rowcnt	;number of rows, this column
altalp2:
	mov	al,[si] 	;move character
	mov byte ptr [si],' '   ;clear source
	mov	es:[di],al	;..to destination
	add	di,[bp].colcnt	;next destination row
	add	si,[bp].colcnt	;next source row
	loop	altalp2 	;do for all rows, this column
	pop	cx
	pop	di
	add	bx,2		;next source column
	inc	di		;next destination column
	loop	altalp1
	pop	ds	;restore addressability
;show new display
	mov	dl,07h		;clear the mark
	call	markcol
	and	flag2,0FDh	;clear pending mark
	call	rowsol		;show row coordinates, may be permuted
	call	showprm 	;show permuted columns
	mov	dx,1		;show solution
	call	showcip
cipsol20:
	xor	dx,dx		;reshow cipher
	call	showcip 	;..because column has been moved
	ret
cipsol	endp

	assume	ds:nothing
comp1	proc	far
	mov	ax,[si].sumrc	;compare A:B dimension table entries
	cmp	ax,es:[di].sumrc
	ret
comp1	endp

comp2	proc	far
	std			;short floating point A:B dimension table
	mov	cx,4
	lea	si,[si].varnce+3
	lea	di,[di].varnce+3
	rep cmpsb
	cld
	ret
comp2	endp

crypt21 ends
	end
