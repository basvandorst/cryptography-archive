	page	53,132
perm	struc
cycle	dw	?	;cycle length
peaks	dw	?	;number of peaks
phase	dw	?	;phase shift
limit	dw	?	;number of shifts
perm	ends

	include railog.str
	include frame.str
	include crypt20b.asm
	extrn	showmsg:far,freeadj:far,hline:far,getmem:far,setcurs:far
	extrn	int2asc:far,help:far,log:far
crypt20 segment 'crya'
	assume cs:crypt20,ds:crypt20b
	public	rail,cntlet,checkc
rail	proc	far
	push	ds
	mov	ax,crypt20b
	mov	ds,ax
	mov	oldpos,dx	;save caller's cursor position
	mov	flag,0
	mov	area,0		;default area is from field
	mov	rangea,4	;default starting depth of rail
	mov	rangeb,8	;default ending depth of rail
	mov	posrnga,0C12h	;starting cursor position, from field
	mov	posrngb,0C19h	;			   to	field
	mov	poslst,0D00h	;starting cursor position, list area
	mov	ax,'4 '
	mov word ptr von,ax	;default from in ascii
	mov	ax,'8 '
	mov word ptr ab,ax	;default to in ascii
	les	di,dword ptr [bp].freehb	;save memory
	mov	freeb,di
	mov	freea,es
	les	di,dword ptr [bp].sizeb
	mov	saveb,di
	mov	savea,es
	call	freeadj 	;adjust memory to segment boundary
	call	clrscrn 	;clear bottom of screen
;draw the bar, identify the panel
	mov	ax,0D01h	;row 13, col 1
	mov	dh,80		;length 80
	call	hline		;draw a horizontal line
	mov	si,offset panelid	;say rail fence
	mov	di,24*160+32
	call	showmsg
	call far ptr cntlet	;count cryptogram letters
	cmp	ax,0		;if there aren't any letters
	jne	rail10
	mov	si,offset msg1	;..say so
errxt:
	mov	di,24*160+80
	call	showmsg
	or	flag,80h	;..flag it
	jmp	forever 	;..and allow exit only
rail10:
	mov	[bp].bilan,ax	    ;save count for later
;get space to reformat cryptogram in
	xor	dx,dx		;round to a multiple of 80
	add	ax,79
	div	c80
	mov	rlines,ax	;number of reformated lines
	mul	c80		;number of bytes to get, multiple of 80
	mov	cx,ax
	call	getmem
	jnc	rail20		;if not enough memory
	mov	si,offset msg2	;..say so
	jmp	errxt		;..and wait for exit
rail20:
	mov	railb,di	;save pointer
	mov	raila,es
	mov	rlong,cx	;save length
	mov	al,' '          ;blank out the reformat buffer
	rep stosb
;reformat the cryptogram
	push	ds
	mov	cx,[bp].lines	;number of cryptogram lines
	les	di,dword ptr railb	;->start of reformat buffer
	lds	si,dword ptr [bp].sbuffb ;->start of cryptogram
	mov	bx,si
railp1:
	push	cx
	xor	cx,cx		;length of this line
	mov	cl,[si]
	inc	si
railp2:
	lodsb
	call far ptr checkc
	jc	rail30
	stosb
rail30:
	loop	railp2		;do all chars in this line
	add	bx,81		;next cryptogram line
	mov	si,bx
	pop	cx
	loop	railp1		;do for all lines
	pop	ds
	call	freeadj 	;initialize start of index tables
	les	di,dword ptr [bp].freehb
	mov	[bp].rlndxb,di
	mov	[bp].rlndxa,es
	les	di,dword ptr [bp].sizeb
	mov	ndxsavb,di
	mov	ndxsava,es
;display default range of depths (4 to 8) on the bar
	call	thisrng 	;show range
	mov	dx,posrnga	;set cursor to from field
	call	setcurs
;wait for keyboard interrupt
forever:
	mov	ah,0		;read from keyboard
	int	16h
	test	flag,80h	;if the cryptogram is empty
	jz	rail38
	cmp	ax,3C00h	;..ignore all keystrokes
	je	f2		;..but quit
	cmp	ax,5500h
	jne	forever
	jmp	sf2		;..and help.
rail38:
	mov	savechar,ax	;save the keystroke
	test	flag,01h	;if a message is up
	jz	rail40
	and	flag,0FEh	;..clear message flag
	mov	es,[bp].videoa	;..and displayed msg
	mov	di,24*160+80
	mov	ax,0F20h
	mov	cx,80
	rep stosw
rail40:
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
	mov	dx,oldpos	;restore caller's cursor position
	call	setcurs
	les	di,dword ptr saveb	;restore memory
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	les	di,dword ptr freeb
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	pop	ds
	ret

;move cursor right in range fields
curite:
	mov	bx,area 	;index to right cursors
	shl	bx,1
	jmp	cursor[bx]
rfrom:
	cmp byte ptr posrnga,18 ;don't move cursor out of field
	jnb	nocur
	inc byte ptr posrnga
fromcom:
	mov	dx,posrnga
	call	setcurs
nocur:
	jmp	forever
rto:
	cmp byte ptr posrngb,25 ;don't move cursor out of field
	jnb	nocur
	inc byte ptr posrngb
tocom:
	mov	dx,posrngb
	call	setcurs
	jmp	forever

;move cursor left in range fields
curleft:
	mov	bx,area
	add	bx,6		;index to left cursors
	shl	bx,1
	jmp	cursor[bx]
lfrom:
	cmp byte ptr posrnga,17
	jna	nocur
	dec byte ptr posrnga
	jmp	fromcom
lto:
	cmp byte ptr posrngb,24
	jna	nocur
	dec byte ptr posrngb
	jmp	tocom

;forward tab
tab:
	mov	bx,area 	;make area an index
	shl	bx,1		;into branch table
	jmp	tabtab[bx]
;backtab
backtab:
	mov	bx,area
	add	bx,6
	shl	bx,1
	jmp	tabtab[bx]
;go to 'to' field
tabfrom:
	mov	area,1		;mark area as to
	jmp	tocom		;set cursor to 'to' field
;go to from field
tabto:
	mov	area,0		;mark area as from
	jmp	fromcom 	;set cursor to 'from' field
;go to m list
tabw:
	mov	area,3		;mark area as m list
	mov byte ptr poslst,20	;cursor col in m list
tabcom:
	mov	dx,poslst
	call	setcurs
	jmp	forever
;go to w inverse list
tabm:
	mov	area,4		;mark area as w inverse list
	mov byte ptr poslst,40	;cursor col in w inverse
	jmp	tabcom
;go to m inverse list
tabwinv:
	mov	area,5
	mov byte ptr poslst,60
	jmp	tabcom
;go to w list
tabminv:
	mov	area,2
	mov byte ptr poslst,0
	jmp	tabcom

;accept blanks or numerals in from and to areas
numeral:
blank:
	cmp	area,2		;blanks and numerals are meaningful
	jb	blnk00		;..in from and to fields only
	jmp	forever
blnk00:
	mov	ax,savechar	;recover character
blnk10:
	cmp	area,1
	je	tblnk		;blank for the to field
;blank for the from field
	mov	si,offset von+1 ;assume current position in from field
	cmp byte ptr posrnga,18
	jb	fblnk
blnkcom:
	mov byte ptr [si],al	;blank last digit in from field
	call	thisrng 	;show this range
	jmp	forever
fblnk:
	dec	si		;blank 1st digit in from field
	mov byte ptr [si],al
	inc byte ptr posrnga	;..and advance cursor to next digit
	mov	dx,posrnga
dig1:
	call	setcurs
	call	thisrng
	jmp	forever

;put a blank in the to field
tblnk:
	mov	si,offset ab+1	;assume last digit of to field
	cmp byte ptr posrngb,25
	je	blnkcom
	dec	si		;it's the 1st digit of to field
	mov byte ptr [si],al
	inc byte ptr posrngb
	mov	dx,posrngb
	jmp	dig1

endline:
	cmp	area,2		;enter valid only in upper screen
	jb	endl00
	jmp	forever
;user has set depth range, pressed enter
endl00:
	call	clrscrn 	;remove previous display
	and	flag,0F9h	;turn off list displayed, and selected perm
	mov	si,offset von
	call	tobin		;convert 'from' to binary
	mov	rangea,ax
	mov	si,offset ab
	call	tobin		;convert 'to' to binary
	mov	rangeb,ax
;check for valid range
	cmp	ax,0		;if to is 0
	jne	endl10
	jmp	noto	       ;..from only, no range
endl10:
	cmp	rangea,0
	jne	endl20
	jmp	nofrom
;neither from nor to is 0, maybe it's a range of depths
endl20:
	call	checkr		;check to for valid depth
	jnc	endl30
endl25:
	mov	si,offset msg3
	jmp	endlerr
endl30:
	mov	ax,rangea
	call	checkr
	jc	endl25
;2<=from<=length/2	and	2<=to<=length/2
;if from = to, just one depth
;else range of depths
	cmp	ax,rangeb
	jb	endl40		;definite from < to
	je	endl50		;from = to, no range
	mov	si,offset msg4	;from > to is an error
	jmp	endlerr
endl40:
	mov	ax,rangeb
	sub	ax,rangea
	inc	ax		;number of depths
	mov	ndxes,ax
	mov	depths,ax
	jmp	figmem
endl50:
	mov	rangeb,0
	jmp short noto20
noto:
	cmp	rangea,0
	jne	noto10
	mov	si,offset msg3	;from 0 to 0?  Invalid depth
	jmp	endlerr
noto10:
	mov	ax,rangea
	call	checkr		;check from for valid values
	jnc	noto20
	mov	si,offset msg3	;depth too small or too large
	jmp	endlerr
noto20:
	mov	ndxes,1 	;only one index, depth in from field
	mov	depths,1
	jmp	figmem
nofrom: 		;but non-zero to
	mov	rangeb,0	;only one depth, force it to from field
	mov	rangea,ax
	jmp	noto10

endlerr:
	or	flag,01h	;clear message next keystroke
	mov	di,24*160+80
	call	showmsg
	jmp	forever

;figure memory needed to generate rail permutations
figmem:
	les	di,dword ptr [bp].rlndxb ;restore previous permutation memory
	mov	[bp].freehb,di		;..before getting present needs
	mov	[bp].freeha,es
	les	di,dword ptr ndxsavb
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	mov	ax,rangea	;minimum depth
	dec	ax		;..less 1
	shl	ax,1		;..doubled is cycle length
	mov	maxpeak,ax	;hold cycle length for now
	mov	ax,[bp].bilan
	xor	dx,dx
	div	maxpeak 	;cipher length/cycle length is peaks
	add	ax,2		;..plus 2 more for M rails and phase shifts
	mov	maxpeak,ax	;save peaks for later
;number of rails to be displayed
	xor	ax,ax
	mov	cx,ndxes
	mov	dx,rangea
	dec	dx
rail50:
	add	ax,dx
	inc	dx
	loop	rail50		;number of rails = number of table entries
	mov	ndxes,ax	;..save number of rails
	mov	si,offset work	;and format for display
	call	int2asc
	mov	ax,word ptr work+3
	mov word ptr nn,ax
	mov	ax,ndxes	;restore count of rails
	shl	ax,1		;table entries to bytes
	shl	ax,1
	shl	ax,1
	mov	cx,ax		;..=entries x 8
	mov	ax,maxpeak	;peaks needed to generate indexes
	add	ax,[bp].bilan	;permutation indexes
	shl	ax,1		;..to bytes
	add	cx,ax		;peaks and permutation indexes
	add	cx,[bp].bilan	;..and plaintext area
	call	getmem
	jnc	rail60
nomem:
	or	flag,01h	;flag message to clear
	mov	si,offset msg5
	mov	di,24*160+80
	call	showmsg
	jmp	forever
rail60:
	mov	[bp].rlndxb,di	    ;save permutation space ptr
	mov	[bp].rlndxa,es
;build the index table
	mov	cx,depths	;number of depths
	mov	bx,rangea
	dec	bx
rail70:
	mov	es:[di].limit,bx	;limit of phase shifts
	mov	ax,bx
	shl	ax,1			;cycle length
	mov	es:[di].cycle,ax
	mov	ax,[bp].bilan		;number of peaks
	xor	dx,dx
	div	es:[di].cycle
	add	ax,2
	mov	es:[di].peaks,ax
	mov	es:[di].phase,0 	;set the 1st phase shift
	push	cx			;clone the phase shifted entries
	mov	cx,es:[di].limit
	mov	si,di
	add	di,type perm		;next table entry
	dec	cx
	jcxz	rail90
	push	ds
	push	es
	pop	ds
rail80:
	push	cx
	mov	cx,type perm
	rep movsb
	inc	[si].phase		;set subsequent phase shifts
	pop	cx
	loop	rail80
	pop	ds
rail90:
	pop	cx
	inc	bx		;next limit
	loop	rail70		;do for all entries
rail95:
	mov	[bp].peakptr,di      ;peak work area
	mov	ax,maxpeak
	shl	ax,1
	add	di,ax
	mov	[bp].permptr,di      ;permutation ptr
	mov	ax,[bp].bilan	     ;..of length 2*letters
	shl	ax,1
	add	di,ax
	mov	[bp].plainptr,di     ;plaintext ptr
;initialize display parameters
	mov	area,2		;1st list area
	mov	poslst,0D00h	;cursor position for first list
	mov	topline,0	;display from 1st permutation
	or	flag,02h	;flag list is displayed
rail100:
	call	show		;show possible plaintext
	call	hell		;hi-lite current line
	mov	dx,poslst	;set cursor on current line
	call	setcurs
	jmp	forever 	;just for now

curdn:
	cmp	area,2
	jnb	curdn00
	jmp	forever
curdn00:
	cmp byte ptr poslst+1,23	;if at bottom of screen
	jb	curdn20
	mov	ax,ndxes		;.try to scroll
	sub	ax,topline		;if more than 11 lines to end of list
	cmp	ax,11
	ja	curdn10 		;..we can scroll
	jmp	forever 		;else we are as far as we can go
curdn10:
	inc	topline 		;scroll one line
	jmp	rail100 		;..and redisplay
curdn20:
	mov	ax,ndxes		;don't move cursor off bottom of list
	sub	ax,topline		;number of lines to end of list
	xor	dx,dx
	mov	dl,byte ptr poslst+1
	sub	dx,12			;number of lines from top of list
	cmp	dx,ax			;if cursor < lines
	jb	curdn30 		;..move cursor down
	jmp	forever 		;else cursor can't move
curdn30:
	inc byte ptr poslst+1		;bump cursor to next line
curdn40:
	mov	dx,poslst
	call	setcurs 		;set the cursor
	call	hell			;hilite the new current line
	jmp	forever

curup:
	cmp	area,2
	jnb	curup00
	jmp	forever
curup00:
	cmp byte ptr poslst+1,13	;if cursor is at top of list
	ja	curup20
	cmp	topline,0		;try to scroll up
	ja	curup10
	jmp	forever 		;..can't at top of lines
curup10:
	dec	topline
	jmp	rail100
curup20:
	dec byte ptr poslst+1		;set cursor to previous line
	jmp	curdn40

nxtwind:
	cmp	area,2		;if in list area
	jb	nxtw10
nxtw00:
	mov	area,0		;..go to from field
	mov	dx,posrnga
	call	setcurs
	jmp	forever
nxtw10:
	test	flag,02h	;if a list is up
	jnz	nxtw20		;..go to list area
	jmp	forever 	;else can't go to list area
nxtw20:
	xor	dx,dx
	xor	ax,ax
	mov	al,byte ptr poslst
	div	c20
	mov	bx,ax
	shl	bx,1
	jmp	prewtab[bx]

prewind:
	cmp	area,2		;if in list area
	jnb	nxtw00		;..go to from field
	test	flag,02h	;if list is up
	jnz	nxtw20		;..go to list area
	jmp	forever

;show the selected rail
altp:
	cmp	area,2
	jnb	altp00
	jmp	forever
altp00:
	or	flag,04h	;flag rail is selected
	xor	dx,dx		;figure shape of rail
	xor	ax,ax
	mov	al,byte ptr poslst
	div	c20
	mov	bx,ax
	shl	bx,1		;shape of rail, w m w- m-
	xor	ax,ax		;compute rail index
	mov	al,byte ptr poslst+1	;row on screen
	sub	ax,13			;..index origin 0
	add	ax,topline	;depth and phase of rail
	call	setdeep 	;format depth and phase into key msg
	jmp	railtab[bx]
railw:
	mov	deep+2,'W'      ;identify the key
	mov	deep+3,' '
	call	genw		;generate a w rail index
railw10:
	call	permg		;decode message by w index
	call	showp		;show decoded message
	jmp	forever
railm:
	mov	deep+2,'M'      ;identify the key
	mov	deep+3,' '
	call	genm		;generate an m rail index
	jmp	railw10
railwinv:
	mov	deep+2,'W'      ;identify the key
	mov	deep+3,'-'
	call	genw
railw20:
	call	permf		;encode message by permutation
	call	showp
	jmp	forever
railminv:
	mov	deep+2,'M'      ;identify the key
	mov	deep+3,'-'
	call	genm
	jmp	railw20

;help wanted
sf1:
	mov	ax,4		;help screen to display
	call	help
	jmp	forever

;log solution
f7:
	test	flag,04h	;if rail not current
	jnz	f700
	jmp	forever 	;..ignore logging
f700:
	mov	logr.railkb,offset deep
	mov	logr.railka,ds	;ptr to key in ascii
	mov	[bp].lgptrb,offset logr
	mov	[bp].lgptra,ds
	mov	ax,1		;log rail cipher
	call	log
	jnc	f710
	or	flag,01h	;message to clear
f710:
	jmp	forever

useasis:
rubout:
escape:
punct:
enguc:
englc:
del:
euruc:
eurlc:
null:
altq:
altw:
alte:
altr:
altt:
alty:
altu:
alti:
alto:
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
f8:
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
rail	endp

;return count of cryptogram letters in ax
cntlet	proc	far
	push	ds
	push	dx
	xor	dx,dx
	mov	cx,[bp].lines	;if no cryptogram lines
	jcxz	cntxt		;..return 0 count
	lds	si,dword ptr [bp].sbuffb ;start of cryptogram
cntlp:
	call	perline
	add	si,81		;next cryptogram line
	loop	cntlp
cntxt:
	mov	ax,dx
	pop	dx
	pop	ds
	ret
cntlet	endp

;count letters per cryptogram line in dx
perline proc	near
	push	si
	push	cx
	xor	cx,cx		;get count of characters in this line
	mov	cl,[si]
	jcxz	perlinxt
	inc	si
prlnlp:
	lodsb			;get a character
	call far ptr checkc
	jc	prln10		;if a cipher letter
	inc	dx		;..count it
prln10:
	loop	prlnlp		;do for all letters in this line
perlinxt:
	pop	cx
	pop	si
	ret
perline endp

;test if character in al is a cipher letter
;returns flags equal if it is, ne if it isn't
checkc	proc	far
	push	es
	push	di
	push	cx
	les	di,dword ptr [bp].alfb
	mov	cx,es:[di]	;length of alphabet
	add	di,2		;es:di-> cipher letters
	repne scasb
	jne	chkc10
	clc
chkcxt:
	pop	cx
	pop	di
	pop	es
	ret
chkc10:
	stc
	jmp	chkcxt
checkc	endp

;show from, to range message
thisrng proc	near
	mov	cx,rngmsgz	;length of range message
	mov	si,offset rngmsg
	mov	es,[bp].videoa
	mov	di,12*160
	mov	ah,07h		;normal intensity
shwrnglp:
	lodsb
	stosw
	loop	shwrnglp
	ret
thisrng endp

;convert two digit ascii to binary. si->two byte field which may contain
;blanks or numerals
tobin	proc	near
	xor	ax,ax		;clear accumulator
	mov	cx,2		;for two digits
tobinlp:
	cmp byte ptr [si],' '   ;ignore digit if blank
	je	tobin10
	mul	c10		;sets dx to 0
	mov	dl,[si]
	sub	dl,'0'
	add	ax,dx
tobin10:
	inc	si		;next digit
	loop	tobinlp
	ret
tobin	endp

;check depth for valid domain, 2<=depth<=length/2
;ax = depth
checkr	proc	near
	cmp	ax,2
	jnb	chkr10
chkr05:
	stc			;invalid depth
	ret
chkr10:
	mov	dx,[bp].bilan
	shr	dx,1
	cmp	ax,dx
	ja	chkr05
	clc			;valid depth
	ret
checkr	endp

;show possible plaintext
show	proc	near
	mov	cx,ndxes	;number of plaintext lines
	sub	cx,topline
	cmp	cx,11		;..but no more than 11
	jna	show00
	mov	cx,11
show00:
	mov	scrnptr,13*160	;screen destination w permutation
	mov	ax,topline	;generate this permutation
	push	cx
shwlp:
	call	genw
	call	permg		;permute by w perm
	call	movep		;put on screen
	add	scrnptr,80
	call	permf		;permute by inverse w perm
	call	movep
	sub	scrnptr,40
	call	genm		;generate the m permutation
	call	permg		;permute cipher letters
	call	movep		;put on screen
	add	scrnptr,80
	call	permf		;inverse m permutation
	call	movep
	add	scrnptr,40	;next screen line
	inc	ax		;next rail
	loop	shwlp
;blank remaining lines if any
	pop	ax
	mov	cx,11
	sub	cx,ax
	jcxz	shwxt
	mov	ax,cx
	mul	c80
	mov	cx,ax
	mov	ax,0720h
	rep stosw
shwxt:
	ret
show	endp

;generate a w permutation for ax rail
genw	proc	near
	push	ax		;save rail index
	push	cx
	push	ds
;address this rail index table
	lds	si,dword ptr [bp].rlndxb    ;rail index
	shl	ax,1
	shl	ax,1
	shl	ax,1
	add	si,ax		;ds:si->index entry for this rail
;generate peaks for this rail
	xor	bx,bx		;index generator, 0,1,...,peaks-1
	mov	cx,[si].peaks	;number of peaks to generate
	mov	es,[bp].rlndxa
	mov	di,[bp].peakptr      ;es:di->peak work area
genwlp1:
	mov	ax,bx
	mul	[si].cycle	;peak = i*cycle-phase
	sub	ax,[si].phase
	stosw
	inc	bx
	loop	genwlp1
;move rail tops to permutation vector
	call	movpeak
	call	geni		;generate the intermediate rows
;generate the rail bottoms
	mov	bx,[bp].peakptr
	mov	cx,[si].peaks
genwlp2:
	mov	ax,es:[bx]	;get the peak
	sub	ax,[si].limit	;phase shift to bottom
	cmp	ax,[bp].bilan	;if 0<=bottom index<letter count
	jnb	genw20
	stosw			;..keep index
genw20:
	add	bx,2		;next peak
	loop	genwlp2
	pop	ds
	pop	cx
	pop	ax		;restore rail index
	ret
genw	endp

;generate an m permutation for ax rail
genm	proc	near
	push	ax
	push	cx
	push	ds
;address this rail index table
	lds	si,dword ptr [bp].rlndxb    ;rail index
	shl	ax,1
	shl	ax,1
	shl	ax,1
	add	si,ax		;ds:si->index entry for this rail
;generate bottoms for this rail
	xor	bx,bx		;index generator, 0,1,...,peaks-1
	mov	cx,[si].peaks	;number of bottoms to generate
	mov	es,[bp].rlndxa
	mov	di,[bp].peakptr      ;es:di->peak work area
genmlp1:
	mov	ax,bx
	mul	[si].cycle	;bottom = i*cycle-phase-limit
	sub	ax,[si].phase
	sub	ax,[si].limit
	stosw
	inc	bx
	loop	genmlp1
;move rail bottoms to permutation vector
	call	movpeak
	call	geni		;generate the intermediate rows
;generate the rail tops
	mov	bx,[bp].peakptr
	mov	cx,[si].peaks
genmlp2:
	mov	ax,es:[bx]	;get the peak
	add	ax,[si].limit	;phase shift to top
	cmp	ax,[bp].bilan	;if 0<=top index<letter count
	jnb	genm20
	stosw			;..keep index
genm20:
	add	bx,2		;next bottom
	loop	genmlp2
	pop	ds
	pop	cx
	pop	ax
	ret
genm	endp

;move current peaks (bottoms) to permutation vector
;returns with es:di->next position in permutation vector
movpeak proc	near
	mov	di,[bp].permptr
	mov	bx,[bp].peakptr
	mov	cx,[si].peaks
mvpklp:
	mov	ax,es:[bx]	;if this peak
	cmp	ax,[bp].bilan	;..is in range, 0<=peak<lettercount
	jnb	mvpk10
	stosw			;..move it, valid index
mvpk10:
	add	bx,2		;next peak
	loop	mvpklp		;do for all peaks
	ret
movpeak endp

;generate intermediate rail rows
;es:di->current output position in permutation buffer
;ds:si->rail index table for this rail
geni	proc	near
	mov	cx,[si].limit	;get number of intermediate rows
	dec	cx		;and if none
	jcxz	genixt		;..leave
	mov	dx,1
genilp1:
	push	cx		;save intermediate row count
	mov	cx,[si].peaks	;for every peak (or bottom)
	mov	bx,[bp].peakptr
genilp2:
	mov	ax,es:[bx]	;..subtract row number
	sub	ax,dx
	cmp	ax,[bp].bilan	;..and if 0<=i<letter count
	jnb	geni10
	stosw			;..keep it.
geni10:
	mov	ax,es:[bx]	;..Then add row number
	add	ax,dx
	cmp	ax,[bp].bilan	;..and if 0<=i<letter count
	jnb	geni20
	stosw			;..keep it.
geni20:
	add	bx,2		;next peak (or bottom)
	loop	genilp2 	;do for all peaks (bottoms), this row
	pop	cx		;recover intermediate row count
	inc	dx		;next row
	loop	genilp1 	;do for all intermediate rows
genixt:
	ret
geni	endp

;permf.  permute cipher by index rlndxa:permptr into rlndxa:plainptr
;cipher stripped on non-alphabetics is at raila:railb
permf	proc	near
	push	ax
	push	cx
	push	ds
	lds	si,dword ptr railb	;->cipher letters
	mov	es,[bp].rlndxa
	mov	di,[bp].permptr
	push	di
	mov	di,[bp].plainptr
	mov	cx,[bp].bilan
floop:
	mov	bx,[bp].permptr
	mov	bx,es:[bx]		;get this index
	mov	al,[si][bx]		;get cipher[i]
	stosb				;..and put in plaintext vector
	add	[bp].permptr,2		;next index
	loop	floop
	pop	di
	mov	[bp].permptr,di 	;restore permptr
	pop	ds
	pop	cx
	pop	ax
	ret
permf	endp

;inversely permute cipher by permutation at rlndxa:permptr
permg	proc	near
	push	ax
	push	cx
	push	ds
	lds	si,dword ptr railb
	mov	es,[bp].rlndxa
	mov	di,[bp].permptr
	push	di
	mov	di,[bp].plainptr
	mov	cx,[bp].bilan
gloop:
	mov	bx,[bp].permptr
	mov	bx,es:[bx]
	lodsb				;get cipher letter
	mov	es:[di][bx],al		;and put it in ith position
	add	[bp].permptr,2		;next index
	loop	gloop
	pop	di
	mov	[bp].permptr,di
	pop	ds
	pop	cx
	pop	ax
	ret
permg	endp

;move plaintext at rlndxa:plainptr to screen at scrnptr
movep	proc	near
	push	ax
	push	cx
	push	ds
	mov	cx,18		;move min of 18 or cipher letter count
	cmp	cx,[bp].bilan
	jna	movep10
	mov	cx,[bp].bilan
movep10:
	mov	di,scrnptr
	mov	es,[bp].videoa	;es:di->screen destination
	mov	ds,[bp].rlndxa
	mov	si,[bp].plainptr	;ds:si->plaintext
	mov	ah,07h		;normal attribute
moveplp:
	lodsb
	stosw
	loop	moveplp
	pop	ds
	pop	cx
	pop	ax
	ret
movep	endp

;hi-lite current line
hell	proc	near
	mov	al,07h		;dull all lines first
	mov	cx,11*80
	mov	es,[bp].videoa
	mov	di,13*160+1	;es:di->attribute byte
hellp1:
	stosb
	inc	di		;next attribute
	loop	hellp1		;do all attributes
	xor	ax,ax
	mov	al,byte ptr poslst+1   ;get current screen row
	mul	c160
	inc	ax
	mov	di,ax		;es:di->attribute byte of current line
	mov	al,0Fh		;bright attribute
	mov	cx,80
hellp2:
	stosb
	inc	di
	loop	hellp2
;display the line number of the current decrypts
	xor	ax,ax
	mov	al,byte ptr poslst+1	;current line on screen
	sub	ax,12
	add	ax,topline		;..plus number of topmost line
	mov	si,offset work
	call	int2asc
	mov	ax,word ptr work+3
	mov word ptr xx,ax
	mov	ah,07h
	mov	di,12*160+134
	mov	si,offset msg7+1
	mov	cl,[si-1]
	xor	ch,ch
hellp3:
	lodsb
	stosw
	loop	hellp3
	ret
hell	endp

;clear the screen below the bar
clrscrn proc	near
	mov	cx,11*80	;number of words to clear
	mov	es,[bp].videoa
	mov	di,13*160	;1st line below the bar
	mov	ax,0720h	;normal attribute, blank
	rep stosw
	ret
clrscrn endp

;format depth and phase into key message
setdeep proc	near
	push	ax
	push	bx
	les	di,dword ptr [bp].rlndxb	;rail table
	shl	ax,1
	shl	ax,1
	shl	ax,1
	add	di,ax				;rail table[i]
	mov	ax,es:[di].limit
	inc	ax				;ax = depth, origin 1
	mov	si,offset work
	call	int2asc 	;convert depth to ascii
	mov	ax,[si+3]
	mov word ptr deep,ax	;..and plunk into message
	mov	ax,es:[di].phase
	inc	ax				;ax = phase, origin 1
	call	int2asc 	;convert phase to ascii
	mov	ax,[si+3]
	mov word ptr step,ax	;..and plunk into message
	pop	bx
	pop	ax
	ret
setdeep endp

;showp.  display rail's plaintext
showp	proc	near
	mov	si,offset msg6
	mov	di,24*160+80
	call	showmsg
	or	flag,01h	;flag message up
	push	ds
	mov	ds,[bp].rlndxa	;source of plaintext
	mov	si,[bp].plainptr
	mov	cx,[bp].bilan	;length of plaintext
	cmp	cx,11*80	;..but no more than 11 lines
	jna	shwp10
	mov	cx,11*80
shwp10:
	mov	es,[bp].videoa
	mov	di,160		;es:di->upper screen area, 2nd line
	mov	ah,07h		;normal attribute
shwplp1:
	lodsb
	stosw
	loop	shwplp1
	mov	cx,11*80
	sub	cx,[bp].bilan
	jb	shwpxt
	jcxz	shwpxt
	mov	ax,0720h	;blank the rest of the cipher area
	rep stosw
shwpxt:
	pop	ds
	ret
showp	endp

crypt20 ends
	end
