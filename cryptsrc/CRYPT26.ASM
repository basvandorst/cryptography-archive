	page	53,132
	.8087
	include frame.str
	include bif.str
	include bifxact.str
	include befehl.str
	include bifsite.str
	include bifkey.str
	include bifsub.str
	include bifptdm.str
	include crypt26b.asm
	extrn	setcurs:far,getmem:far,freemem:far,freeadj:far,savefree:far
	extrn	showmsg:far
crypt26 segment 'crya'
	assume	cs:crypt26
	public	sumlet,findblnk,bifline,bifloga,biflogz,biflog,bif_keysub
	public	adjcurs,bifrite,bifleft,getoken,cmdlook,sitebif,sitip,setcip
	public	sitequ,bifnxn,cordsub,bifdim,showkey,bifperm,bifix,bifdic
;find a bifid period from the ciphertext arranged to show period
; cx = number of remaining lines
; es:bx->cryptogram line of where to start looking
;on return, ax = period or 0 if indeterminate
findblnk proc	far
	push	bx
	push	cx
fblnlp:
	push	cx
	xor	cx,cx
	mov	cl,es:[bx]	;cipher line length
	jcxz	fblnk10 	;empty line
	mov	di,bx
	inc	di		;es:di->cipher characters
	mov	al,' '
fblnlp2:
	repe scasb	;look for 1st non-blank
	je	fblnk10 	;no non-blank
	dec	di		;correct over-shoot
	inc	cx		;correct count
	mov	dx,di		;save start of string
	repne scasb		;look for a blank
	pop	cx
	dec	di		;no blank, measure to end of line
	sub	di,dx		;length to 1st blank
	mov	ax,di
	cmp	ax,79		;but no block longer than 79 ever
	jbe	fblnkxt
	mov	ax,79
fblnkxt:
	pop	cx
	pop	bx
	ret
fblnk10:
	pop	cx
	add	bx,81		;next cipher line
	loop	fblnlp		;try next line in case previous is empty
	xor	ax,ax
	jmp	fblnkxt 	;no period found
findblnk endp

;sum characters in a bifid for allocation needs
; cx=number of lines
; es:bx->1st line
; on return ax = number of letters
sumlet	proc	far
	xor	ax,ax
sumlp1:
	push	bx
	push	cx
	xor	cx,cx
	mov	cl,es:[bx]	;get line length
	jcxz	sumlet20	;empty line
sumlp2:
	cmp byte ptr es:[bx+1],' '
	je	sumlet10	;not a character
	inc	ax		;count the letter
sumlet10:
	inc	bx
	loop	sumlp2
sumlet20:
	pop	cx
	pop	bx
	add	bx,81
	loop	sumlp1
	ret
sumlet	endp

;number of lines for formatted display
;number of chars in main blocks
;ds:si->bifid parms
bifline proc	far
	mov	ax,[si].period		;period length
	inc	ax			;..plus interblock separator blank
	mov	[bp].tempcnt,ax
	mov	ax,80			;display line length
	xor	dx,dx
	div	[bp].tempcnt
	mov	[si].bperl,ax		;number of blocks per line
	mov	ax,[si].blocks		;number of regular blocks
	xor	dx,dx
	div	[si].bperl		;number of lines for regular blocks
	or	dx,dx			;if there are remaining blocks
	jnz	setln10 		;..need one more line
;no remainder, number of regular blocks happens to fill n lines exactly
	cmp	[si].kurz,0		;if there is a short block
	je	setln20
setln10:
	inc	ax			;..need one more line
setln20:
	mov	[si].dlines,ax		;number of screen lines
	ret
bifline endp

	assume	ds:crypt26b
;create transaction log
bifloga proc	far
	push	ds
	mov	ax,crypt26b
	mov	ds,ax
	mov	flag1,01h	;assume broken log
	mov	[bp].biflogn,0	;initialize log counter
	mov	ndxid,0 	;no file handles for index
	mov	logid,0 	;..or log
	mov	dx,offset filendx
	mov	ah,3Ch		;create transaction log index
	xor	cx,cx
	int	21h
	jc	biflgaxt
	mov	ndxid,ax	;save handle for later
	mov	dx,offset filelog
	mov	ah,3Ch		;create transaction log
	xor	cx,cx
	int	21h
	jc	biflgaxt
	mov	logid,ax
	mov	flag1,0 	;flag ok log
	xor	ax,ax		;good return code
biflgaxt:
	pop	ds
	ret
bifloga endp

;erase transaction log
biflogz proc	far
	push	ds
	mov	ax,crypt26b
	mov	ds,ax
	cmp	ndxid,0 	;if no index created
	je	biflgzxt	;..just exit
	mov	bx,ndxid	;else flush DOS buffers by
	mov	ah,3Eh		;..by closing index
	int	21h
	mov	dx,offset filendx ;..then
	mov	ah,41h		;erase the transaction index
	int	21h
	cmp	logid,0 	;if no log file created
	je	biflgzxt	;..just exit
	mov	bx,logid	;else close log
	mov	ah,3Eh
	int	21h
	mov	dx,offset filelog
	mov	ah,41h		;erase the log
	int	21h
biflgzxt:
	pop	ds
	ret
biflogz endp

;add to, retrieve from bifid transaction log
;dx = 0, log key, coordinates, plaintext for 1st and 2nd bifids
;dx = 1, retrieve key, coordinates, plaintext for 1st and 2nd bifids
;on return, no carry if i/o goes well, and ax = 1 for no key, 0 for key
biflog	proc	far
	push	ds
	mov	ax,crypt26b
	mov	ds,ax
	push	bx
	push	cx
	push	es
	push	di
	push	si
	test	flag1,01h	;if log is broken
	jz	biflog10
	stc			;..logging failed
	jmp	biflogxt	;..and quit
biflog10:
	or	dx,dx
	jz	biflogw 	;write to log
	jmp	biflogr 	;read to log
;write to log
biflogw:
	call	getndx		;get current lognth index
	jnc	biflog20
	or	flag1,01h	;log is broken
	stc
	jmp	biflogxt	;..so quit now
biflog20:
	mov	cx,ndxbuf.logposb	;position transaction log
	mov	dx,ndxbuf.logposa	;..at ith+1 position for writing
	add	dx,ndxbuf.bifcipl	;..ith pos + cipherlength
	jnc	biflog30
	inc	cx			;..+ carry
biflog30:
	add	dx,ndxbuf.bifragl	;..+ fragment table length
	jnc	biflog40
	inc	cx			;..+ carry
biflog40:
	mov	bx,logid
	mov	ax,4200h
	int	21h
	jnc	biflog50
biflog45:
	or	flag1,01h		;broken log
	stc
	jmp	biflogxt
biflog50:
	mov	ndxbuf.logposb,dx	;update index for ith+1 entry
	mov	ndxbuf.logposa,ax
	mov	logl,0			;zero bytes written accum
;log the present bifid state, fixed part 1st
	les	di,dword ptr [bp].bifprm1b	;log the 1st bifid
	call	pushbif
	jc	biflog45
	les	di,dword ptr [bp].bifprm2b	;log the 2nd bifid
	call	pushbif
	jc	biflog45
	mov	ax,logl
	mov	ndxbuf.bifcipl,ax	;save 1st length
	mov	logl,0
;log the key, variable part
	mov	ax,[bp].bfkeyb		;if there is no key
	or	ax,[bp].bfkeya
	jz	biflog80		;..complete and write the index
	mov	bx,logid
	push	ds
	lds	si,dword ptr [bp].bfkeyb ;->key and parms
	mov	cx,[si].bifgross	;number of bytes to write
	mov	dx,si			;from where to write
	mov	ah,40h
	int	21h
	pop	ds
	jc	biflog45	;broken log
	cmp	ax,cx
	jne	biflog45	;broken log
	mov	logl,ax
biflog80:
	mov	ax,logl
	mov	ndxbuf.bifragl,ax	;length of fragment
	mov	cx,8
	mov	ah,40h
	mov	dx,offset ndxbuf
	mov	bx,ndxid
	int	21h		;write the index
	jc	biflog45
	cmp	ax,cx
	jne	biflog45
	inc	[bp].biflogn	;count the transaction
biflog90:
	clc
biflogxt:
	pop	si
	pop	di
	pop	es
	pop	cx
	pop	bx
	pop	ds
	ret
;retrieve key and bifid
biflogr:
	cmp	[bp].biflogn,0
	je	biflog90	;nothing to retrieve
	call	getndx		;get ith index entry
	jnc	biflg100
biflog95:
	or	flag1,01h
	stc
	jmp	biflogxt
;position log at ith entry
biflg100:
	mov	cx,ndxbuf.logposb
	mov	dx,ndxbuf.logposa
	mov	ax,4200h
	mov	bx,logid
	int	21h
	jc	biflog95
;retrieve fixed part of bifid 1st
	les	di,dword ptr [bp].bifprm1b
	call	popbif		;retrieve 1st bifid
	jc	biflog95	;..broken log
	les	di,dword ptr [bp].bifprm2b
	call	popbif		;retrieve 2nd bifid
	jc	biflog95
	cmp	ndxbuf.bifragl,0	;if no key to recover
	jne	biflg110
	mov	ax,1		;signal no key
biflg105:
	lds	si,dword ptr [bp].bfkeyb  ;restore memory status quo ante
	lea	si,[si].bifmem
	call	freemem
	dec	[bp].biflogn		;..back up this transaction log
	clc				;..good return
	jmp	biflogxt		;..and leave
biflg110:				;else get the key too
	mov	cx,ndxbuf.bifragl
	mov	ah,3Fh
	mov	bx,logid
	push	ds
	lds	dx,dword ptr [bp].bfkeyb ;key destination
	int	21h
	pop	ds
	jc	biflog95
	cmp	ax,cx
	jne	biflog95
	xor	ax,ax		;signal key
	jmp	biflg105
biflog	endp

;get current logn index in ndxbuf
getndx	proc	near
	mov	ax,[bp].biflogn 	;if 0th log entry
	cmp	ax,0
	jne	getndx10
	mov	ndxbuf.logposb,ax	;..initialize for 1st entry
	mov	ndxbuf.logposa,ax
	mov	ndxbuf.bifcipl,ax
	mov	ndxbuf.bifragl,ax
	clc				;good return
	ret
getndx10:
	dec	ax			;get the ith index entry
	mov	cx,3			;8 bytes per entry
	shl	ax,cl
	mov	dx,ax		;position index to ith entry
	xor	cx,cx
	mov	bx,ndxid
	mov	ax,4200h
	int	21h
	jnc	getndx20
	ret			;positioning failed
getndx20:
	mov	dx,offset ndxbuf
	mov	bx,ndxid
	mov	ah,3Fh		;read the ith index
	mov	cx,8
	int	21h
	jnc	getndx30
	ret			;read failed
getndx30:
	cmp	ax,cx
	je	getndx40
	stc
	ret			;read failed
getndx40:
	clc			;read index successful
	ret
getndx	endp

;es:di->bifid to push
pushbif proc	near
	mov	cx,es:[di].cnt
	jcxz	pushbfxt
	shl	cx,1
	mov	bx,logid
	mov	ah,40h
	push	ds
	mov	dx,es:[di].coord1
	mov	ds,es:[di].cipa
	int	21h		;log the coordinates
	pop	ds
	jnc	pushbf10
pushbfer:
	stc			;broken log
	ret
pushbf10:
	cmp	ax,cx
	jne	pushbfer
	add	logl,ax
	mov	bx,logid
	mov	ah,40h
	mov	cx,es:[di].cnt
	push	ds
	mov	dx,es:[di].subp 	;log the plaintext
	mov	ds,es:[di].cipa
	int	21h
	pop	ds
	jc	pushbfer
	cmp	ax,cx
	jne	pushbfer
	add	logl,ax
pushbfxt:
	clc
	ret
pushbif endp

;es:di->bifid to pop
popbif	proc	near
	mov	cx,es:[di].cnt	;length of bifid
	jcxz	popbifxt	;..no bifid
	shl	cx,1		;get the coordinates
	mov	bx,logid
	mov	ah,3Fh
	push	ds
	mov	dx,es:[di].coord1
	mov	ds,es:[di].cipa
	int	21h
	pop	ds
	jnc	popbif10
popbifer:
	stc		;broken log
	ret
popbif10:
	cmp	ax,cx
	jne	popbifer	;broken log
	mov	cx,es:[di].cnt	;get the plaintext
	mov	bx,logid
	mov	ah,3Fh
	push	ds
	mov	ds,es:[di].cipa
	mov	dx,es:[di].subp
	int	21h
	pop	ds
	jc	popbifer
	cmp	ax,cx
	jne	popbifer
popbifxt:
	clc
	ret
popbif	endp
	assume	ds:nothing

;adjust cursor
;due to line move or scroll, cursor may be beyond end of new current line
;if so, adjust to end of line
;ds:si->bifid parms
adjcurs proc	far
	mov	ax,[si].period
	inc	ax		;block length + 1 for block span
	mov	[bp].tempcnt,ax ;save for later
	xor	bx,bx
	mov	bl,[si].rcurs	;get index into full,short block counts
	sub	bx,3
	mov	ax,[si][bx].l1full  ;get full block count for this line
	mov	bx,[si][bx].l1kurz  ;..and short block length this line
	mul	[bp].tempcnt	;past end of last full block
	add	ax,bx		;past end of final short block if any
	dec	ax		;index origin 0
	or	bx,bx		;if no short block
	jnz	adjcur20
	dec	ax		;remove one more trailing blank
adjcur20:
	cmp	al,[si].ccurs	;if end of line < cursor position
	jnb	adjcur30
	mov	[si].ccurs,al	;..force cursor position to end of line
adjcur30:
	mov	dx,word ptr [si].ccurs
	call	setcurs
	ret
adjcurs endp

;move cursor right in substitution area
;ds:si->bifid parms
bifrite proc	far
	mov	ax,[si].period	;figure the position of last character
	inc	ax		;allow an interblock space
	mov	[bp].tempcnt,ax ;..save for later
	xor	bx,bx		;get to current line block counts
	mov	bl,[si].rcurs
	sub	bx,3		;0, 4, 8
	mul	[si][bx].l1full
	mov	dx,[si][bx].l1kurz  ;get the short block
	add	ax,dx		;add in the short block length
	or	dx,dx		;..if there is any
	jnz	subr10		;..max length is computed
	dec	ax		;..else eliminate final blank
subr10: 	;ax now has position past last character shown on screen
	mov	ah,[si].ccurs	;if I can move cursor on this line
	inc	ah
	cmp	ah,al
	jb	subr60		;..do it
;goto next screen line if there is one
	cmp	bx,8		;if at last line
	jb	subr50
subr15:
	mov	[si].rcurs,3	;..wrap to line 1
subr20:
	mov	[si].ccurs,0
subr30:
	mov	dx,word ptr [si].ccurs
	call	setcurs
subr40:
	ret
subr50: 				;go to start of next line
	mov	ax,[si][bx].l2full	;..if there is one
	or	ax,[si][bx].l2kurz
	jz	subr15			;..there isn't, go to 1st line
	add	[si].rcurs,4		;there is one, bump cursor to it
	jmp	subr20			;..and leave
;move cursor right on same line
subr60:
	mov	[si].ccurs,ah	;move cursor right 1
	mov	al,ah		;but may need one more to bridge block span
	xor	ah,ah
	inc	ax
	xor	dx,dx
	div	[bp].tempcnt	; pos / (period+1)
	or	dx,dx		;if no remainder
	jnz	subr30
	inc	[si].ccurs	;..we are at a block span
	jmp	subr30
bifrite endp

;move cursor left in bifid substitution area
;ds:si->bifid parms
bifleft proc	far
	mov	ax,[si].period
	inc	ax
	mov	[bp].tempcnt,ax ;save period+1 for later
	xor	bx,bx		;prepare to seek active line
	xor	dx,dx		;prepare for division
	cmp	[si].ccurs,0	;try to back up
	je	subl30		;..can't back up, goto end previous line
;cursor left on same line
	dec	[si].ccurs
	mov	al,[si].ccurs
	inc	ax		;position+/(period+1)
	div	[bp].tempcnt
	or	dx,dx		;if there is no remainder
	jnz	subl20		;..we are at a block span
	dec	[si].ccurs	;..and need to back up one more
subl20:
	mov	dx,word ptr [si].ccurs
	call	setcurs
	ret
;cursor to end of previous line
subl30: 	;find a previous line to go to
	sub	[si].rcurs,4	;back up one line
	jnb	subl40
	mov	[si].rcurs,11	;backed up too far, wrap to end
subl40:
	mov	bl,[si].rcurs	;this test line
	sub	bx,3		;0, 4, 8 = line1, line2, line3
	or	bx,bx		;can always go to line 0
	jz	subl50
	mov	ax,[si][bx].l1full
	or	ax,[si][bx].l1kurz
	jz	subl30		;this line is empty, can't go to it
subl50: 		;line bx not empty
	mov	ax,[si][bx].l1full	;this lines's full blocks
	mov	bx,[si][bx].l1kurz	;..and short block
	mul	[bp].tempcnt
	add	ax,bx
	dec	ax		;index origin 0
	cmp	bx,0		;if no short block on this line
	jne	subl60
	dec	ax		;..get rid of 1 blank too many
subl60:
	mov	[si].ccurs,al
	jmp	subl20
bifleft endp

;getoken and command lookup
; cmdlook(cmdline, cmdleng,cmdtabp, cmdn, cmdndx, tokptr, toklen)
; cmdline->command line (80 bytes usually)
; cmdleng = length of command line
; cmdtabp -> command table
; cmdn = number of command table entries
; cmdndx = returned index of found command
; tokptr -> returned token, or command lookup argument
; toklen = returned token length, or command arg length
getoken proc	far
	push	bp
	mov	bp,sp		;parm addressability
	push	ds
	push	si
	push	es
	push	di
	push	cx
	push	bx
	push	dx
	lds	si,dword ptr [bp+6]
	call	toka		;find start of token
	jc	getokxt
	call	tokz		;find end of token
	clc			;flag token isolated
getokxt:
	pop	dx
	pop	bx
	pop	cx
	pop	di
	pop	es
	pop	si
	pop	ds
	pop	bp
	ret 4
getoken endp

;getoken and command lookup
; cmdlook(cmdline, cmdleng,cmdtabp, cmdn, cmdndx, tokptr, toklen)
; cmdline->command line (80 bytes usually)
; cmdleng = length of command line
; cmdtabp -> command table
; cmdn = number of command table entries
; cmdndx = returned index of found command
; tokptr -> returned token, or command lookup argument
; toklen = returned token length, or command arg length
cmdlook proc	far
	push	bp
	mov	bp,sp
	push	ds
	push	si
	push	es
	push	di
	push	cx
	push	dx
	push	bx
	lds	si,dword ptr [bp+6]
	mov	cx,[si].toklen
	mov	dx,cx
	cmp	cx,8
	jna	cmdlk10 	;check supposed command for length
	stc			;flag invalid command
	jmp short cmdlkxt
cmdlk10:
	mov	si,[si].tokptr
	mov	ax,crypt26b
	mov	es,ax
	mov	di,offset hold
cmdlkup:
	lodsb		;fold to lower case
	cmp	al,'A'
	jb	cmdlk15
	cmp	al,'Z'
	ja	cmdlk15
	or	al,20h
cmdlk15:
	stosb
	loop	cmdlkup
	lds	si,dword ptr [bp+6]	;restore parm addressability
	mov	cx,[si].cmdn	;number of commands to check
	lds	bx,[si].cmdtabp ;->command table
	xor	ax,ax
cmdlk20:
	mov	si,bx		;->command length
	mov	al,[si] 	;minimum length
	inc	si		;->command in table
	cmp	dx,ax		;if token length < min length
	jb	cmdlk30 	;..can't compare this table entry
	push	cx
	mov	cx,dx		;length for comparison is token length
	mov	di,offset hold
	repe cmpsb		;is this the command?
	pop	cx
	je	cmdhit		;..yes it is
cmdlk30:
	add	bx,9		;next command table entry
	loop	cmdlk20
	stc			;command not found
cmdlkxt:
	pop	bx
	pop	dx
	pop	cx
	pop	di
	pop	es
	pop	si
	pop	ds
	pop	bp
	ret 4
cmdhit:
	lds	si,dword ptr [bp+6]	;addressability again
	mov	ax,[si].cmdn		;figure index into command table
	sub	ax,cx
	mov	[si].cmdndx,ax		;..return index for caller
	clc			;flag command found
	jmp	cmdlkxt
cmdlook endp

;find end of token (to 1st blank or end of line)
tokz	proc	near
	mov	cx,[si].cmdleng
	les	di,[si].cmdline
	mov	al,' '
	repne scasb		;scan for 1st blank (end of token)
	jne	tokz10		;..no blank, token ended by end of line
;token ended by blank before end of line
	dec	di		;correct overshoot
	mov word ptr [si].cmdline,di	;next scan position
	sub	di,[si].tokptr	;length of this token
	mov	[si].toklen,di
	mov	ax,[si].cmdleng
	sub	ax,di		;length of remaining command line
	mov	[si].cmdleng,ax
	ret
tokz10: 		;token ended by end-of-line
	mov word ptr [si].cmdline,di	;next ptr
	mov	[si].cmdleng,cx 	;zero length left
	sub	di,[si].tokptr		;length of token
	mov	[si].toklen,di
	ret
tokz	endp

;find start of token.  we allow any number of beginning blanks
toka	proc	near
	mov	cx,[si].cmdleng
	les	di,[si].cmdline
	jcxz	toka10
	mov	al,' '
	repe scasb		;scan for 1st non-blank
	je	toka10		;..none found, empty line
	dec	di		;else correct overshoot
	inc	cx
	mov	[si].cmdleng,cx ;update length
	mov word ptr [si].cmdline,di	;update current ptr
	mov	[si].tokptr,di		;token start
	clc				;flag token start found
	ret
toka10:
	mov	[si].toklen,0	;empty token
	stc			;flag not found
	ret
toka	endp

;test this position in cipher as site for tip
;returns coincidences found as count in ax
;uses bifsite, same parms as sitip
;bp-2	dw	coincidence count, returned in ax
;bp	dw	bp save
;bp+2	dd	return address
;bp+6	dd	parm struc ptr mapped by bifsite
sitebif proc	far
	push	bp
	mov	bp,sp		;parameter addressability
	xor	ax,ax
	push	ax		;bp-2 is coincidence count
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	es
	les	di,dword ptr [bp+6]
	lds	si,es:[di].sitcrib ;->supposed crib
	mov	cx,es:[di].sitcril ;length of crib
	xor	bx,bx		   ;cipher offset
siteblp:
;test position+bx for even or odd position within current block
	mov	ax,bx
	add	ax,es:[di].siti    ;ax = index into cipher
	xor	dx,dx
	div	es:[di].sitper
;ax = which block i'm in, dx = which letter in the block, even or odd
	test	dl,01h		;set flag for even or odd
	jnz	sitebrt 	;ignore odd positions
	mov	dx,es:[di].sitper  ;assume i'm in a regular block
	cmp	ax,es:[di].sitblkn ;then test assumption
	jb	siteb10 	;yes
	mov	dx,es:[di].sitkurz ;no, last block which may be short
;am I testing for naturals in odd length block or identities in even length?
siteb10:		;dx now contains the block length
	test	dl,01h		;is block length even or odd?
	jz	siteb50 	;it's even, look for identity
;odd block length, look for full natural
	push	bx		;save tip index
	add	bx,es:[di].siti
	les	di,es:[di].sitcipb ;bot cipher line
	mov	ah,es:[di][bx]	;get bot line letter
	les	di,dword ptr [bp+6]
	les	di,es:[di].sitcipt ;top cipher line
	mov	al,es:[di][bx]	;get row letter
	pop	bx		;restore tip index
	les	di,dword ptr [bp+6]
	cmp	ah,al		;if row = col
	jne	siteb30
	cmp	ah,[si][bx]	;..full natural, must agree with tip letter
	je	siteb20
;this site is not possible
	mov word ptr [bp-2],0	;..doesn't agree, this site not possible
	stc			;flag impossible site
	jmp short sitebxt
sitebrt:
	inc	bx		;next position in tip
	loop	siteblp
	clc
sitebxt:
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax		;return count of coincidences
	pop	bp
	ret
;count a full natural
siteb20:
	inc word ptr [bp-2]
	inc word ptr [bp-2]
	jmp	sitebrt
;check for half naturals
siteb30:
	cmp	ah,[si][bx]	;if bot line letter matches tip letter
	je	siteb40 	;..count a half natural
	cmp	al,[si][bx]	;if top line letter matches tip letter
	jne	sitebrt
;count a half natural
siteb40:
	inc word ptr [bp-2]	;..count a half natural
	jmp	sitebrt
;even periods
;check for an identity, as in .G AZ PA CH O.
; Ar Ac  in ciphertext, 	A Z	in tip
; Zr Zc
siteb50:
	cmp	cx,1		;if at last letter of tip
	je	sitebrt 	;..can't check for identity
	push	bx		;save tip index
	add	bx,es:[di].siti    ;index into ciphertext
	les	di,es:[di].sitcipt ;top cipher line
	mov	ah,es:[di][bx]	 ;get row letter from ciphertext
	les	di,dword ptr [bp+6]
	les	di,es:[di].sitcipb
	mov	al,es:[di][bx]	 ;get col letter from ciphertext
	pop	bx		;restore tip index
	les	di,dword ptr [bp+6]
	cmp	ah,[si][bx]
	je	siteb60
	jmp	sitebrt
siteb60:
	cmp	al,[si+1][bx]
	je	siteb70
	jmp	sitebrt
siteb70:
	inc word ptr [bp-2]	;count the identity as 2
	inc word ptr [bp-2]
	jmp	sitebrt
sitebif endp

;initialize the tip by expanding it in row, column form at destination
;tipexpnd, for example ORDERI SBETTERTH AN becomes
; OrRrDrErRrIrSrBrErTrTrErRrTrHrArNr
; OcRcDcEcRcIcScBcEcTcTcEcRcTcHcAcNc
;this is in preparation for gathering equivalent ciphertext/plaintext letters
;bp	 dw	 ?	 ;save bp
;bp+2	 dd	 ?	 ;return address
;bp+6	 dd	 ?	 ;->bifsite struct
sitip	proc	far
	push	bp
	mov	bp,sp
	push	cx
	push	ds
	push	es
	push	si
	push	di
	les	di,dword ptr [bp+6]
	lds	si,es:[di].sitcrib	;->crib
	mov	cx,es:[di].sitcril	;length of crib
	les	di,es:[di].sitpair	;expanded tip destination
	push	cx
	push	si
	mov	ah,'r'          ;first, expand tip as row letters
sitipl1:
	lodsb
	stosw
	add	di,2		;leave a hole for ct top letter pair
	loop	sitipl1
	pop	si
	pop	cx
	mov	ah,'c'          ;then, expand tip as column letters
sitipl2:
	lodsb
	stosw
	add	di,2		;leave a hole for ct bot letter pair
	loop	sitipl2
	pop	di
	pop	si
	pop	es
	pop	ds
	pop	cx
	pop	bp
	ret
sitip	endp

;initialize the cipher as row, col letters, for this site, for example:
;	HHBBPPGGI VVEEYYOOH HHIIAAVVQ
;	IOOCCUUSS HOOTTCCII QZZAAIIMM
;	   ORDERI SBETTERTH AN
;becomes
;	 Or  Rr  Dr  Er  Rr  Ir  Sr  Br  Er  Tr  Tr  Er  Rr  Tr  Hr  Ar  Nr
;	   Bc  Pr  Pc  Gr  Gc  Ir  Vr  Vc  Er  Ec  Yr  Yc  Or  Oc  Hr  Hr  Hc
;
;	 Oc  Rc  Dc  Ec  Rc  Ic  Sc  Bc  Ec  Tc  Tc  Ec  Rc  Tc  Hc  Ac  Nc
;	   Cr  Cc  Ur  Uc  Sr  Sc  Hc  Or  Oc  Tr  Tc  Cr  Cc  Ir  Ic  Qc  Zr
;uses bifsite, same parms as sitip
;bp-8	dw	number of blocks
;bp-6	dw	short block length
;bp-4	dw	block length of full blocks
;bp-2	dw	site start offset
;bp	dw	bp save
;bp+2	dd	return address
;bp+6	dd	->bifsite structure
setcip	proc	far
	push	bp
	mov	bp,sp
	sub	sp,8		;local vars
	push	bx
	push	cx
	push	dx
	push	ds
	push	es
	push	di
	push	si
	les	di,dword ptr [bp+6]	;parms ptr
	mov	ax,es:[di].siti
	mov	[bp-2],ax		;offset to current site
	mov	ax,es:[di].sitper
	mov	[bp-4],ax		;period
	mov	ax,es:[di].sitkurz
	mov	[bp-6],ax		;short block length
	mov	ax,es:[di].sitblkn
	mov	[bp-8],ax
	mov	cx,es:[di].sitcril	;length of crib
	lds	si,es:[di].sitcipt	;top cipher line ptr
	add	si,es:[di].siti 	;->begin at this letter
	les	di,es:[di].sitpair	;->destination of dimensioned letters
	xor	bx,bx		;index into cipher segment
	push	cx
setcipl1:
	mov	ax,bx
	add	ax,[bp-2]	;index into cipher
	xor	dx,dx
	div word ptr [bp-4]	;index into cipher block
	lodsb			;get the cipher letter
	mov	ah,'r'          ;assume it is a row letter
	test	dl,01h		;if index is even,
	jz	setcip10	;..we have a row letter
	mov	ah,'c'          ;else we have a column letter
setcip10:
	stosw		;stuff the expanded letter
	add	di,2	;skip dimensioned plaintext letter in paired list
	inc	bx		;next letter
	loop	setcipl1	;do for all top line letters
	pop	cx
	lds	si,dword ptr [bp+6]
	lds	si,[si].sitcipb ;cipher bottom line letters
	add	si,[bp-2]	;start at this letter
	xor	bx,bx
setcipl2:
	mov	ax,bx		;figure index within the current block
	add	ax,[bp-2]
	xor	dx,dx
	div word ptr [bp-4]
	cmp	ax,[bp-8]	;decide for full block or short block
	mov	ax,[bp-4]	;assume regular blocks
	jb	setcip20	;..it is a full block
	mov	ax,[bp-6]	;no, it's final maybe short block
setcip20:
;1 index even	 block length odd	 column letter
;2 index odd	 block length odd	 row letter
;3 index even	 block length even	 row letter
;4 index odd	 block length even	 col letter
	mov	ah,'r'          ;assume row letter
	test	al,01h
	jnz	setcip30		;block length odd
;block length even
	test	dl,01h
	jz	setcip40	;..even index, got it right
	mov	ah,'c'          ;correct assumption
	jmp short setcip40
;block length odd
setcip30:
	test	dl,01h
	jnz	setcip40	;..odd index, got it right
	mov	ah,'c'          ;correct row assumption
setcip40:
	lodsb		;get cipher letter
	stosw		;..and stuff expansion
	add	di,2	;skip over plaintext dimensioned letter
	inc	bx	;next cipher letter
	loop	setcipl2	;do for all letters
	pop	si
	pop	di
	pop	es
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	add	sp,8		;discard local vars
	pop	bp
	ret
setcip	endp

;pull string of equivalent letters from expanded tip and current site
;returns carry if conflict discovered
;else returns 0 if string is null, meaning equivalents are exhausted
;else returns in ax number of letters in an equivalent string, and
;address of string in coordequ
;sitequ should be called repeatedly until returned length is 0.
;uses bifsite, same parms as sitip
;
;it is essential that sitpair be segment aligned, offset 0
;bp-18	dw	current count of pairs
;bp-16	dw	offset of current position in pairs
;bp-14	dw	segment
;bp-12	dw	root, 5 or 6
;bp-10	dw	paired length for convenience
;bp-8	dw	diagonal counter
;bp-6	dw	row counter
;bp-4	dw	column counter
;bp-2	dw	collected count, returned
;bp	dw	save bp
;bp+2	dd	return address
;bp+6	dd	->site parms
sitequ	proc	far
	push	bp
	mov	bp,sp
	xor	ax,ax
	push	ax	;collected count
	push	ax	;column counter
	push	ax	;row counter
	push	ax	;diagonal counter
	push	ax	;tip length, in stack for convenience
	sub	sp,8	;for root, pair ptr, remaining count
	push	bx
	push	cx
	push	dx
	push	es
	push	ds
	push	di
	push	si
	lds	si,dword ptr [bp+6]	;->parms
	mov	ax,[si].sitroot
	mov	[bp-12],ax		;root,5 or 6
	les	di,[si].sitfrag 	;clear return area
	mov	al,' '
	mov	cx,24
	rep stosb		;clear the equivalent string
	mov	cx,[si].sitcril ;get tip length
	mov word ptr [bp-10],cx  ;..save in stack for convenience

;initialize collection
;look for any dimensioned letter
sitequ05:
	mov	cx,[bp-10]
	shl	cx,1
	shl	cx,1
	les	di,[si].sitpair
sitequ07:
	jcxz	sitequ09
	mov	ax,2020h	;look for a non-eliminated dimensioned letter
	repe scasw
	jne	sitequ10	;got a dimensioned letter
;no dimensioned letters left
sitequ09:
	clc			;good return flag
	jmp	sitequxt	;..but no string, end of equivalents
;found a pair, get both pt and ct
sitequ10:
	mov	ax,es:[di-2]	;dimensioned plaintext
	mov	dx,es:[di]	;dimensioned ciphertext
	mov word ptr es:[di-2],2020h	;and blot them out
	mov word ptr es:[di],2020h
	cmp	ax,dx		;if tip and cipher letters are same
	je	sitequ07	;..forget it

;we have beginning of equivalency string
	mov	[bp-16],di	;save where we left off
	mov	[bp-14],es
	mov	[bp-18],cx
	les	di,[si].sitfrag
	mov	es:[di],ax
	mov	es:[di+2],dx
	mov word ptr [bp-2],2  ;two equivalents
	cmp	ah,'r'
	je	sitequ32
	inc word ptr [bp-4]	;count the column letter
	jmp short sitequ34
sitequ32:
	inc word ptr [bp-6]	;count the row letter
sitequ34:
	cmp	dh,'r'
	je	sitequ36
	inc word ptr [bp-4]	;count the column letter
	jmp short sitequ38
sitequ36:
	inc word ptr [bp-6]	;count the row letter
sitequ38:
	cmp	al,dl
	jne	sitequ40
	mov word ptr [bp-8],1	;count a diagonal letter
sitequ40:
	lds	si,[si].sitfrag
	xor	dx,dx		;collected equivalents index

;begin collection loop
sitequl1:
	cmp	dx,[bp-2]      ;if all collected letters tried
	jb	sitequ50
	clc			;..good flag
	jmp	sitequxt	;..and we are done with this string
sitequ50:
	mov	bx,dx
	shl	bx,1
	mov	ax,[si][bx]	;get collected letter,designation
	mov	cx,[bp-18]	 ;length to scan
	les	di,dword ptr [bp-16]
	repne scasw		;see if same letter is anyplace
	je	sitequ60	;..yep
;nothing found
	inc	dx		;try next collected letter
	jmp	sitequl1	;do for all collected letters
;something found
sitequ60:
	mov word ptr es:[di-2],2020h	;cross off matching letter
;now get its paired letter, ct or pt, whichever
	mov	cx,di		;assume we stopped on pt letter and di
	sub	cx,2		;..points to corresponding ct letter
	shr	cx,1
	test	cl,01h		;if it's even our assumption is correct
	jz	sitequ62	;..we have a plaintext letter in ax
	sub	di,4		;else ct letter in ax, get plaintext before it
sitequ62:
	mov	cx,es:[di]		;get paired letter
	mov word ptr es:[di],2020h	;..and cross it off too
;ax = collected[i], cx = corresponding equivalent from pt or cipher
	cmp	ax,cx	;if they are the same
	jne	sitequ80
	jmp	sitequl1 ;..throw them away
sitequ80:
	mov	ax,cx	;see if we already collected the corresponding letter
	les	di,dword ptr [bp+6]
	les	di,es:[di].sitfrag
	mov	cx,[bp-2]
	repne scasw
	jne	sitequ90
	jmp	sitequl1	;already got it, throw this one away
;test for conflicts
;	no more than root row letters in collected
;	no more than root col letters in collected
;	no more than 1 diagonal letter in collected -- xrow == xcol
sitequ90:
	mov	cx,[bp-12]	;get the root
	mov	bx,ax		;save the letter
	cmp	ah,'r'
	jne	siteq110
	mov	ah,'c'          ;prepare for diagonal check
	inc word ptr [bp-6]	;count a row letter
	cmp	cx,[bp-6]	;no more than root rows
	jnb	siteq120
siteq100:
	stc		;flag a conflict, this site can't possible be right
	jmp short sitequxt
siteq110:
	mov	ah,'r'          ;prepare for diagonal check
	inc word ptr [bp-4]	;count the column letter
	cmp	cx,[bp-4]	;no more than root columns
	jb	siteq100
siteq120:
	mov	cx,[bp-2]      ;check for diagonal letter
	mov	di,si
	repne scasw
	jne	siteq130
	cmp word ptr [bp-8],1	;..only one diagonal
	jnb	siteq100
	mov word ptr [bp-8],1	;only one, but flag it

;we have a valid corresponding letter
siteq130:
	mov	ax,bx		;recover the letter
	mov	bx,[bp-2]      ;and stuff it away
	shl	bx,1		;each letter takes two bytes
	mov	[si][bx],ax
	inc word ptr [bp-2]    ;count one more collected letter
	jmp	sitequl1

sitequxt:
	pop	si
	pop	di
	pop	ds
	pop	es
	pop	dx
	pop	cx
	pop	bx
	jc	siteqno
	add	sp,16	;collapse locals
	pop	ax	;return count of equivalents
	pop	bp
	clc
	ret
siteqno:		;conflict exit
	add	sp,16
	pop	ax
	pop	bp
	stc
	ret
sitequ	endp

;make a bifid 5x5 or 6x6 key from fragment table
;es:di->fragn	dw	number of fragments
;	fragb	dw	offset to frag table
;	frags	dw	segment
;ax = root, 5 or 6
;a fragment table entry is
;nn dimensioned letters 	where nn = number of dimensioned letters
;frag tables are sorted in descending order by nn
;on return
; if ax = 0, es:di->key
; if ax = -1, fragment table is conflicting, no key built
; if ax = 1, insufficient memory
	assume	ds:crypt26b
bifnxn	proc	far
	push	ds
	push	bx
	mov	bx,crypt26b
	mov	ds,bx
	mov	root,ax 	;save root till later
	mov	parmb,di	;save ptr to frag table parms
	mov	parma,es
	les	di,dword ptr [bp].bfkeyb	;save old key ptr if any
	mov word ptr oldkeyp,di
	mov word ptr oldkeyp+2,es
	push	cx
	push	dx
	push	si
	call	freeadj 	;align free mem to segment boundary
;figure memory needs
	mov	ax,root
	les	di,dword ptr parmb
	add	ax,es:[di]	;side = fragn+root
	dec	ax
	mov	sidehold,ax
	mul	root		;root x side
	mov	fitsize,ax	;..= max size of fits table
	mov	ax,sidehold
	mul	sidehold	;size of side x side
	mov	[bp].bifsqr,ax
	mov	cx,ax		;3 x side x side for three letter tables
	shl	ax,1
	add	cx,ax
	add	cx,fitsize	;..plus fit table size
	add	cx,type bifkey	;..plus key anchor size
	mov	ax,sidehold
	shl	ax,1
	mov	counters,ax
	shl	ax,1
	add	cx,ax
	call	getmem
	jnc	bifnxn10
bifnxn00:
	mov	ax,1		;not enough memory
	jmp	bifnxnxt
bifnxn10:
	mov	[bp].bfkeyb,di ;anchor the key
	mov	[bp].bfkeya,es
	mov	es:[di],cx	;fill in size for transaction logging
	mov	si,offset save2b
	call	savefree
	push	ds		;also save mem parms to restore
	push	es		;memory parms after transaction retrieval
	pop	ds
	lea	si,es:[di].bifmem
	call	savefree
	pop	ds
	call	getmem
	jnc	bifnxn20	;get another area for key saving
	mov	si,offset save2b
	call	freemem 	;quit, not enough memory
	jmp	bifnxn00
bifnxn20:
	mov	[bp].bifk2b,di	;temp key area
	mov	[bp].bifk2a,es
	les	di,dword ptr [bp].bfkeyb ;offset to row letter counts
	mov	ax,root
	mov	es:[di].bifgrund,ax ;root of key
	mov	[bp].bifpale,ax 	;start of fake coordinates
	mov	ax,sidehold
	mov	es:[di].bifside,ax
	mov	bx,di
	add	bx,type bifkey
	mov	es:[di].bifrwcnt,bx
	add	bx,counters
	mov	es:[di].bifclcnt,bx ;..offset to column letter counts
	add	bx,counters	;offset to fit table
	mov	es:[di].bifit,bx
	add	bx,fitsize
	mov	es:[di].bifrow,bx ;->row matrix
	add	bx,[bp].bifsqr
	mov	es:[di].bifcol,bx ;->col matrix
	add	bx,[bp].bifsqr
	mov	es:[di].bifsect,bx ;->intersect matrix
	mov	bx,[bp].bfkeyb
	mov	cx,counters	;clear the counters
	mov	di,es:[bx].bifrwcnt
	xor	ax,ax
	rep stosw
;clear roots to nothing fits
	mov	cx,root
	mov	di,es:[bx].bifit
nxnlp0:
	push	cx
	mov	cx,root
	xor	ax,ax
nxnlp01:
	stosb			;fill each fit table root entry
	inc	ax		;..with 0,1,2,3,,,root-1
	loop	nxnlp01
	pop	cx
	loop	nxnlp0
;now correct roots to each fits only itself
	mov	al,0FFh
	mov	cx,root
	mov	di,es:[bx].bifit
nxnlp02:			;fill in major diagonal with
	stosb			;..does fit = null
	add	di,root
	loop	nxnlp02
;fill in the pale coordinates with fits everything (nulls throughout)
	sub	di,root 	;begin pale entries
	mov	ax,es:[bx].bifside ;N
	sub	ax,root 	   ;N-root = number of pale entries
	mul	root		   ;..x root (width) = number of
	mov	cx,ax		   ;..coordinates
	jcxz	bifnxn25	   ;no pale if only 1 fragment
	mov	al,0FFh 	;does fit = null
	rep stosb
bifnxn25:
	mov	di,es:[bx].bifrow
	mov	cx,[bp].bifsqr
	shl	cx,1
	add	cx,[bp].bifsqr
	mov	al,' '
	rep stosb		;clear the matrixes to nothing solved
	les	di,dword ptr parmb  ;->parms
	mov	cx,es:[di]	;number of fragments
	les	di,es:[di+2]	   ;->fragment table
nxnlp1: 		;per fragment outer loop
	call	cntdim		;count rows and columns in frag[i]
	call	rootfit 	;attempt to fit frag[i] in a root coordinate
	or	ax,ax
	jz	bifnxn30
	les	di,oldkeyp	;reset key ptr to old key
	mov	[bp].bfkeyb,di
	mov	[bp].bfkeya,es
	jmp	bifnxnxt	;bad keysquare
bifnxn30:
	add	di,26		;next fragment
	loop	nxnlp1
	les	di,dword ptr [bp].bfkeyb
	mov	ax,[bp].bifpale
	mov	es:[di].bifmaxn,ax
	xor	ax,ax		;good return
bifnxnxt:
	mov	si,offset save2b
	call	freemem
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ds
	ret
bifnxn	endp

;es:di->frag[i].  set rows, cols
cntdim	proc	near
	push	di
	push	cx
	mov	cx,es:[di]	;count of dimensioned letters in frag[i]
	add	di,2		;->dimensioned letter
	mov	[bp].tempcnt,0	;row counter
	mov	[bp].templen,0	;col counter
cntdimlp:
	cmp byte ptr es:[di+1],'r'
	je	cntr
	inc	[bp].templen	;count the column
cntdim10:
	add	di,2		;next dimensioned letter in frag[i]
	loop	cntdimlp	;do for all dimensioned letters
	pop	cx
	pop	di
	ret
cntr:
	inc	[bp].tempcnt	;count the row
	jmp	cntdim10
cntdim	endp

;attempt to fit frag[i] in root
;for all roots,
; if root is unused and no previous 'fits', splice (sets return)
; if root is unused and previous 'fits',
;   mark pale, count another fit
; if root is used, test frag for fit
;   if it fits, frag must go to pale, mark pale, count fit, try more roots
;   if it doesn't fit, try more roots
rootfit proc	near
	push	cx
	push	ds
	push	si
	push	es
	push	di
	mov	[bp].bifragib,di
	mov	[bp].bifragia,es
	mov	cx,root
	lds	si,dword ptr [bp].bfkeyb
	mov	bx,[si].bifclcnt     ;->col counters
	mov	si,[si].bifrwcnt     ;->row counters
	xor	dx,dx		;dh = fit counter, dl = fit index
rootlp:
	mov	ax,[si] 	;get rowcount[j]
	or	ax,[bx] 	;column count[j]
	jnz	root20		;coordinate is occupied
	or	dh,dh
	jnz	root50		;frag[i] fit elsewhere and may go here too

;unused root coordinate j, no previous fits, frag[i] better fit here
	inc	dh		;fits = 1
	mov byte ptr [bp].fitj,dl	;coordinate where frag[i] fits
	jmp	rootend 	;quit the loop

;used root coordinate j, frag[i] may fit here
root20:
	call	testfit 	;test at coordinate j
	or	ax,ax
	jz	root25		;fits at j
	call	markpale	;record that frag[i] doesn't fit at j
	jmp short root30
root25:
	inc	dh		;count the fit
	mov byte ptr [bp].fitj,dl	;record the root coordinate
root30:
	inc	dl		;next root coordinate
	add	si,2		;next row counter
	add	bx,2		;next col counter
	loop	rootlp		;do for all root coordinates
;root coordinates tried.
	jmp	rootend
;unused root coordinate j, but frag[i] fits previous to j, must go to pale
root50:
	inc	dh		;count the fit
	jmp	root30		;try next root
;end of root coordinate testing
rootend:
	or	dh,dh
	jnz	root70
	mov	ax,-1		;frag[i] couldn't fit in any root
	jmp	rootxt		;..invalid keysquare
root70:
	cmp	dh,1		;frag[i] fit at only one root
	jne	root80
;unmark table[pale], in case it is marked
	mov	ax,[bp].bifpale
	les	bx,dword ptr [bp].bfkeyb
	mul	es:[bx].bifgrund
	mov	di,es:[bx].bifit
	add	di,ax		;es:di->table[pale][0]
	cmp	di,es:[bx].bifrow ;if there was only one fragment
	jnb	root75		  ;..there is no pale, don't clobber the rows
	mov	al,0FFh 	;noplace coordinate pale doesn't fit
	mov	cx,es:[bx].bifgrund ;bytes in an entry = root
	rep stosb
root75:
	mov	dx,[bp].fitj
	call	splice		;splice at jth root
	jmp short rootxt	;..and return with splice's code
root80:
	mov	dx,[bp].bifpale ;frag[i] fit at more than one root
	call	splice		;splice at the pale, return with splice's code
	inc	[bp].bifpale	;next pale coordinate
rootxt:
	pop	di
	pop	es
	pop	si
	pop	ds
	pop	cx
	ret
rootfit endp
	assume	ds:nothing

;save current key, splice frag[i] at dl, restore current key, return
;splice's return code
;dl = coordinate at which to test fit of es:di->frag[i]
;ds:si->row count	[bp].tempcnt is frag[i] rows
;ds:bx->col count	[bp].templen is frag[i] cols
testfit proc	near
	push	bx
	push	cx
	push	dx
	push	ds
	push	si
	push	es
	push	di
	lds	si,dword ptr [bp].bfkeyb       ;current key
	mov	cx,[si] 	;length of key
	les	di,dword ptr [bp].bifk2b	;save area destination
	rep movsb
	call	splice		;splice es:di->frag[i] at coordinate dl
	lds	si,dword ptr [bp].bifk2b	;restore current key
	mov	cx,[si]
	les	di,dword ptr [bp].bfkeyb
	rep movsb
	pop	di
	pop	es
	pop	si
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	ret
testfit endp

;splice es:di->frag[i] at coordinate dl
;dl = coordinate at which to test fit of es:di->frag[i]
;tempcnt is frag[i] row count
;templen is frag[i] col count
splice	proc	near
	xor	dh,dh
	mov	[bp].fitith,dx
	shl	dx,1
	lds	si,dword ptr [bp].bfkeyb
	mov	bx,[si].bifrwcnt
	add	bx,dx		;bx->row_count[j]
	mov	ax,[bx] 	;current count of rows
	add	ax,[bp].tempcnt ;total row letters <= root
	cmp	ax,[si].bifgrund
	jbe	splc10
splc00:
	mov	ax,-1
	ret			;invalid splice
;append row letters to row[j]
splc10:
	push	si
	mov	ax,[si].bifside
	push	dx
	mul	[bp].fitith
	pop	dx
	mov	si,[si].bifrow
	add	si,ax		;si->row_letters[j]
	add	si,[bx] 	;si->row_letters[j][nth]
	les	di,dword ptr [bp].bifragib ;->frag[i]
	push	di
	mov	cx,es:[di]	;number of frag[i] dimensioned letters
splclp1:
	add	di,2
	mov	ax,es:[di]	;get fragment letter
	cmp	ah,'r'          ;if row dimension
	jne	splc20
	mov	[si],al 	;..append to row[j] table
	inc word ptr [bx]	;..and count it
	inc	si		;..next nth
splc20:
	loop	splclp1
	pop	di
	pop	si
	mov	bx,[si].bifclcnt
	add	bx,dx		;bx->col_count[j]
	mov	ax,[bx] 	;current count of cols
	add	ax,[bp].templen ;total col letters <= root
	cmp	ax,[si].bifgrund
	ja	splc00
;append the column letters to col[j]
	push	si
	mov	ax,[si].bifside
	mul	[bp].fitith
	mov	si,[si].bifcol
	add	si,ax		;si->col_letters[j]
	add	si,[bx] 	;si->col_letters[j][nth]
	mov	cx,es:[di]	;number of frag[i] dimensioned letters
splclp2:
	add	di,2
	mov	ax,es:[di]	;get fragment letter
	cmp	ah,'c'          ;if col dimension
	jne	splc30
	mov	[si],al 	;..append to col[j] table
	inc word ptr [bx]	;..and count it
	inc	si		;..next nth
splc30:
	loop	splclp2
	pop	si
	mov	ax,[si].bifside
	mov	[bp].bside,ax	;save n
	mov	si,[si].bifrow	;ds:si->row square
	mov	[bp].browp,si	;save start of row square
	les	bx,dword ptr [bp].bfkeyb
	mov	ax,es:[bx].bifcol
	mov	[bp].bcolp,ax
	mov	di,es:[bx].bifsect
	mov	bx,di		;es:bx->intersect square
	mov	[bp].bsectp,bx
	mov	cx,[bp].bifsqr
	mov	al,' '
	rep stosb		;clear the intersect
	mov	cx,[bp].bifsqr
splclp3:
	lodsb		;get the row letter
	cmp	al,' '
	je	splc50		;letter not solved yet
	mov	di,[bp].bcolp	;find row letter in column letters
	push	cx
	mov	cx,[bp].bifsqr
	repne scasb
	pop	cx
	jne	splc50		;not solved yet
	dec	di		;solved, row and col, correct overshoot
	mov	ax,si		;figure row coordinate
	dec	ax
	sub	ax,[bp].browp
	xor	dx,dx
	div	[bp].bside	;ax = row
	mul	[bp].bside
	mov	bx,ax		;bx = intersect[row]
	mov	ax,di		;now figure column
	sub	ax,[bp].bcolp
	div	[bp].bside
	add	bx,ax		;bx = intersect[row][col] offset
	mov	al,[si-1]	;retrieve the row,col solved letter
	mov	di,[bp].bsectp
	cmp byte ptr es:[di][bx],' ' ;intersect should be empty
	je	splc40
	mov	ax,-1		     ;if not empty
	ret			     ;..it's collision, invalid keysquare
splc40:
	mov	es:[di][bx],al	     ;else fill in the intersect
splc50:
	loop	splclp3
	xor	ax,ax
	ret
splice	endp

;markpale.   fitable[pale][k] = dl = j
;record in what root coordinate (dl), this frag[i] may go
markpale proc	near
	push	bx
	push	cx
	push	dx
	push	es
	push	di
	les	bx,dword ptr [bp].bfkeyb
	mov	di,es:[bx].bifit	;es:di->fit table
	mov	ax,[bp].bifpale
	mov	cx,dx		;save j
	mul	es:[bx].bifgrund
	add	di,ax		;es:di->table[pale][0]
	xor	ch,ch
	add	di,cx		;es:di->table[pale][j]
	mov	es:[di],cl	;cross off j, pale fits in root j
	pop	di
	pop	es
	pop	dx
	pop	cx
	pop	bx
	ret
markpale endp

;substitute coordinates from plaintext and keysquare letters
;uses bifsub, bif, bifkey
;on return, ax = -1 if conflicts detected
;bifsub  struc		 ;parms for substitution from keysquare
;bp-24	dw	?	 N**2
;bp-22	dw		 ciphertext top or bottom line origin
;bp-20	dw	?	 return code, 0 = no conflicts, -1 = conflicts
;bp-18	db	?	 flaga
;			 01 = conflict detected
;			 02 = this block is odd length
;			 04 = this letter index is odd
;			 08 = ciphertext bottom line
;			 10 = use column key letters, 00 use row key letters
;			 80 = plaintext / keysquare coordinate conflict
;bp-17	db		 flagb for filler
;bp-16	dw	?	 length of a short block
;bp-14	dw	?	 length of a full block
;bp-12	dw	?	 number of full blocks
;bp-10	dw	?	 start of top row coordinates
;bp-8	dw	?	 key start offset
;bp-6	dw	?	 col start offset
;bp-4	dw	?	 row start offset
;bp-2	dw	?	 N for address arith of NxN matrix
;	 dw	 ?	 ;bp save
;	 dd	 ?	 ;return address
;keyp2	 dd	 ?	 ;->keysquare parms, mapped by bifkey
;bifp	 dd	 ?	 ;->bifid parms, mapped by bif
;bifsub  ends
cordsub proc	far
	push	bp
	mov	bp,sp
	sub	sp,24		;working storage
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	ds
	push	es
	mov word ptr [bp-20],0	;assume good return code
	mov word ptr [bp-18],0	;clear flags
	les	di,[bp].keyp2	;retrieve keysquare parms
	mov	ax,es:[di].bifside ;The N of NxN
	mov	[bp-2],ax	   ;..for coordinate arithmetic
	mul	es:[di].bifside    ;N**2
	mov	[bp-24],ax
	mov	bx,es:[di].bifrow  ;start of solved row letters
	mov	[bp-4],bx
	mov	bx,es:[di].bifcol  ;start of solved column letters
	mov	[bp-6],bx
	mov	bx,es:[di].bifsect ;start of intersect keysquare
	mov	[bp-8],bx
	lds	si,[bp].bifp	   ;retrieve bifid parms
	mov	ax,[si].blocks
	mov	[bp-12],ax	;number of blocks
	mov	ax,[si].period
	mov	[bp-14],ax	;length of full block
	mov	ax,[si].kurz
	mov	[bp-16],ax	;length of short block
	mov	es,[si].cipa
	mov	di,[si].coord1	;top row of coordinates
	mov	[bp-10],di
	mov	cx,[si].cnt	;total letters of plaintext
	mov	bx,cx		;..also offset to bottom row of coordinates
;initialize coordinates
	mov	al,0FFh 	;clear coordinates
	shl	cx,1
	rep stosb		;..to foxes
	mov	di,[bp-10]	;start of coordinates
	mov	cx,bx
	mov	ax,[si].cipa
	mov	si,[si].subp
	mov	ds,ax		;ds:si->plaintext
;translate solved plaintext letters into coordinates
crdsubl1:		;for all solved plaintext
	lodsb
	cmp	al,' '          ;if not solved
	jne	crdsub10
	inc	di		;..skip plaintext letter
	jmp short crdsub20
crdsub10:			;else
	call	ptlkup		;..translate to coordinates
	stosb			;..and put row in coordinate buffers
	mov	es:[di-1][bx],ah ;..and column
crdsub20:
	loop	crdsubl1
;now translate ciphertext letters to coordinates
	lds	si,[bp].bifp
	lds	si,dword ptr [si].cipb
	mov	[bp-22],si
	mov	cx,bx
	mov	di,[bp-10]	;top row coordinates
crdsubl2:
	lodsb			;get the ciphertext top letter
	call	ctlkup	       ;translate to row coordinate
;check for conflict between plaintext and ciphertext coordinates
	cmp byte ptr es:[di],0FFh ;if coordinate unknown
	je	crdsub30	;..ok to replace
	cmp	es:[di],al	;else better be
	je	crdsub30	;..the same coordinate
	or byte ptr [bp-18],80h ;remember conflict for return code
crdsub30:
	stosb			;enter the coordinate
	loop	crdsubl2
	mov	[bp-22],si	;start of ct bottom line
	or byte ptr [bp-18],08h ;flag at ct bottom line
	mov	cx,bx
crdsubl3:
	lodsb			;get ciphertext bottom letter
	call	ctlkup		;translate to coordinate
	cmp byte ptr es:[di],0FFh ;if coordinate previously unknown
	je	crdsub40	 ;..ok to replace
	cmp	es:[di],al	 ;else if this coord matches previous
	je	crdsub40	 ;..still ok to replace
	or byte ptr [bp-18],80h  ;else flag plaintext-keysquare conflict
crdsub40:
	stosb			 ;enter the coordinate
	loop	crdsubl3
;make plaintext substitutions according to coordinates
	mov	cx,bx
	lds	si,[bp].bifp
	mov	di,[si].subp	;es:di->plaintext buffer
	mov	ds,[si].cipa
	mov	si,[bp-10]	;ds:si->top coordinate line
crdsubl4:
	lodsb			;get top coordinate
	cmp	al,0FFh 	;if top coordinate isn't solved
	jne	crdsub50	;..get bottom coordinate
	inc	di
	jmp short crdsub80
crdsub50:
	mov	ah,[si-1][bx]	;get bottom coordinate
	cmp	ah,0FFh
	jne	crdsub60
	inc	di
	jmp short crdsub80
crdsub60:
	call	ptsub		;translate to letter from keysquare
	cmp	al,' '          ;if not solved
	jne	crdsub70	;..ignore substitution
	inc	di
	jmp short crdsub80
crdsub70:
	stosb
crdsub80:
	loop	crdsubl4
	mov	ax,0		;assume good return
	test byte ptr [bp-18],80h
	jz	crdsubxt
	mov	ax,-1		;coordinate conflicts
crdsubxt:
	pop	es
	pop	ds
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	add	sp,24
	pop	bp
	ret	8
cordsub endp

;translate a plaintext letter to row, col coordinates according to keysquare
;al = letter
;on return, ah = column, al = row, in ascii, origin 1
ptlkup	proc   near
	push	es
	push	di
	push	cx
	push	bx
	push	ax		;save plaintext letter
	mov	bx,-1		;assume neither row nor column solved
	mov	cx,[bp-24]	;solved row letters length
	mov	es,word ptr [bp].keyp2+2	;key segment
	mov	di,[bp-4]			;..offset
	repne scasb
	jne	ptlkup10       ;not solved, try as a column letter
	mov	ax,di		;get row position
	dec	ax
	sub	ax,[bp-4]
	xor	dx,dx
	div word ptr [bp-2]
	mov	bl,al		;save for return
ptlkup10:
	pop	ax		;recover plaintext letter
	mov	di,[bp-6]	;now look in solved column letters
	mov	cx,[bp-24]	;solved column letters length
	repne scasb
	jne	ptlkup20
	mov	ax,di		;get column position
	dec	ax
	sub	ax,[bp-6]
	xor	dx,dx
	div word ptr [bp-2]
	mov	bh,al
ptlkup20:
	mov	ax,bx
	pop	bx
	pop	cx
	pop	di
	pop	es
	ret
ptlkup	endp

;translate a ciphertext letter to si (row or column) coordinate
;al = ciphertext letter, si-1 = offset into ciphertext line
ctlkup	proc	near
	push	es
	push	di
	push	cx
	push	bx
	push	ax
	and byte ptr [bp-18],0E9h  ;reset odd/even flags
;decide whether using row key letters or column key letters
	mov	ax,si
	dec	ax
	sub	ax,[bp-22]	;ciphertext index
	xor	dx,dx
	div word ptr [bp-14]	;ax = block index, dx = block letter index
	mov	bx,[bp-14]	;assume full block length
	cmp	ax,[bp-12]	;..test using full block length
	jb	ctlkup10	;..yep
	mov	bx,[bp-16]	;..nope, using short block length
ctlkup10:
	test	bl,01h		;if this block length is even
	jz	ctlkup20	;..even/odd length set to even
	or byte ptr [bp-18],02h ;else set length to odd
ctlkup20:
	test	dl,01h		;if this index is even
	jz	ctlkupcn	;..even/odd index set to even
	or byte ptr [bp-18],04h ;else set index to odd
ctlkupcn:
	mov	bx,[bp-18]	;get even odd top bot bit settings
	xor	bh,bh
	and	bl,0Eh		;turn off all but top, index, length bits
	push	ds
	mov	ax,crypt26b
	assume	ds:crypt26b
	mov	ds,ax
	jmp	rowcol[bx]	;set row/col key table bit accordingly
	assume	ds:nothing
ctcol:
	or byte ptr [bp-18],10h ;use column key letters
ctrow:
	pop	ds
	pop	ax		;recover ciphertext letter
	mov	es,word ptr [bp].keyp2+2	;keysquare addressability
	mov	ah,0FFh 	;assume key letter not solved yet
	mov	di,[bp-4]	;assume using key row letters
	test byte ptr [bp-18],10h	;..and say it's so
	jz	ctlkup30
	mov	di,[bp-6]		;nope it ain't, use column key
ctlkup30:
	mov	bx,di
	mov	cx,[bp-24]	;length of row or col letters
	repne scasb		;see if this letter has been solved
	jne	ctlkupxt	;..nope it hasn't
	mov	ax,di
	dec	ax
	sub	ax,bx		;keysquare index
	xor	dx,dx
	div word ptr [bp-2]	;..to coordinate
	mov	ah,al
ctlkupxt:
	mov	al,ah		;return the coordinate
	pop	bx
	pop	cx
	pop	di
	pop	es
	ret
ctlkup	endp

;translate coordinates to plaintext letter
;al = row coordinate
;ah = col coordinate
;on return al = ' ' if not solved, else plaintext letter
ptsub	proc	near
	push	bx
	push	cx
	push	di
	push	es
	xor	bx,bx
	mov	bl,ah		;save column coordinate
	mul byte ptr [bp-2]	;row index
	add	bx,ax
	mov	es,word ptr [bp].keyp2+2 ;get key letters
	mov	di,[bp-8]	;es:di->key letters
	mov	al,es:[di][bx]
	pop	es
	pop	di
	pop	cx
	pop	bx
	ret
ptsub	endp

;bifdim -- dimension plaintext, then ciphertext letters that correspond to
;	   plaintext.  return count in ax
;uses bifptdm for this routine's parms
;     bif for bifid parms
;dimension plaintext and ciphertext letters parms
;bifptdm struc
;bp-14	dw	?	 count of cipher letters
;bp-12	dw	?	 period
;bp-10	dw	?	 kurz
;bp-8	dw	?	 blocks
;bp-6	dw	?	 start of plaintext
;bp-4	dw	?	 letter counter
;bp-2	dw	?	 root for dimension arithmetic
;	 dw	 ?	 ;bp save
;	 dd	 ?	 ;return address
;bifp2	 dd	 ?	 ;->cipher parms
;workp	 dd	 ?	 ;->dimensioning work area
;bifptdm ends
bifdim	proc	far
	push	bp
	mov	bp,sp
	sub	sp,14
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	es
	mov word ptr [bp-4],0	;for letter counting
	les	bx,[bp].bifp2	;bif parms
	mov	ax,es:[bx].root1
	mov	[bp-2],ax	;for coordinate arithmetic
	mov	cx,es:[bx].cnt	   ;max count
	mov	[bp-14],cx
	mov	di,es:[bx].subp    ;es:di->plaintext
	mov	[bp-6],di	;save origin for offset computing
	mov	ax,es:[bx].blocks
	mov	[bp-8],ax
	mov	ax,es:[bx].kurz
	mov	[bp-10],ax
	mov	ax,es:[bx].period
	mov	[bp-12],ax
	mov	es,es:[bx].cipa
bifdiml1:
	mov	al,' '          ;scan for non-blanks
	jcxz	bifdimxt
	repe scasb
	je	bifdimxt	;nothing found
	inc word ptr [bp-4]	;count the letter
	call	dimpt		;dimension the pt letter
	call	dimct		;dimension the corresponding ct letters
	add word ptr [bp].workp,8 ;next work buff position
	jmp	bifdiml1
bifdimxt:
	mov	ax,[bp-4]	;return count
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	add	sp,14
	pop	bp
	ret	8	;pops stack
bifdim	endp

;es:[di-1]->plaintext letter of interest
dimpt	proc	near
	push	es
	push	di
	mov	al,es:[di-1]	;get the letter
	mov	ah,'r'          ;make it a row 1st time
	les	di,[bp].workp
	mov	es:[di],ax
	mov	ah,'c'          ;then make it a column
	mov	es:[di+4],ax	;..leaving a hole for ct top
	pop	di
	pop	es
	ret
dimpt	endp

;dimension corresponding ciphertext letters
dimct	proc	near
	push	es
	push	di
	push	cx
	mov	bx,di
	dec	bx
	sub	bx,[bp-6]	;offset to ciphertext letter
	mov	ax,bx		;figure how to dimension both ct letters
	xor	dx,dx
	div word ptr [bp-12]
	mov	cx,[bp-12]	;assume using full blocks
	cmp	ax,[bp-8]
	jb	dimct10
	mov	cx,[bp-10]	;..using final short block
dimct10:
	mov	ah,'r'          ;assume odd block, even pos
	mov	al,'c'
	test	cl,01h		;block odd or even?
	jnz	dimct20
	mov	al,'r'          ;even block, assume r,r for even pos
	test	dl,01h
	jz	dimctcom	;even block, even pos should be r,r
	mov	ah,'c'
	mov	al,ah		;even block, odd pos should be c,c
	jmp short dimctcom
dimct20:
	test	dl,01h		;odd block, even or odd pos?
	jz	dimctcom	;even pos
	xchg	ah,al		;odd pos, should be c,r
dimctcom:
	mov	dx,ax		;dh is top ct dim, dl is bot ct dim
	lds	si,[bp].bifp2
	lds	si,dword ptr [si].cipb	;get the cipher letters
	mov	al,[si][bx]
	les	di,[bp].workp
	mov	es:[di+2],ax
	add	si,[bp-14]
	mov	al,[si][bx]
	mov	ah,dl
	mov	es:[di+6],ax
	pop	cx
	pop	di
	pop	es
	ret
dimct	endp

;show current key
showkey proc	far
	push	ds
	push	es
	push	bx
	push	cx
	push	si
	push	di
	lds	si,dword ptr [bp].bifalfb
	les	di,dword ptr [bp].bifalfb
	add	di,36
	mov	cx,36
	rep movsb
	lds	bx,dword ptr [bp].bfkeyb
	mov	es,[bp].videoa
	mov	si,[bx].bifsect ;display key intersect characters
	mov	di,16*160+50
	mov	ah,07h
	mov	cx,[bx].bifgrund
shwkylp1:
	push	cx
	push	si
	push	di
	mov	cx,[bx].bifgrund
shwkylp2:
	lodsb
	cmp	al,' '
	je	shwkey30
	push	es
	push	di
	push	cx
	les	di,dword ptr [bp].bifalfb
	add	di,36
	mov	cx,36
	repne scasb
	jne	shwkey20
	mov byte ptr es:[di-1],' '      ;mark solved letter off
shwkey20:
	pop	cx
	pop	di
	pop	es
shwkey30:
	stosw
	add	di,2
	loop	shwkylp2	;show this row of letters
	pop	di
	pop	si
	pop	cx
	add	di,160		;next screen row
	add	si,[bx].bifside  ;next key row
	loop	shwkylp1	;show all rows
	mov	di,23*160+46
	mov	cx,36
	lds	si,dword ptr [bp].bifalfb
	add	si,36
shwkylp3:			;show unsolved letters
	lodsb
	stosw
	loop	shwkylp3
	pop	di
	pop	si
	pop	cx
	pop	bx
	pop	es
	pop	ds
	ret
showkey endp

;show permutation of keysquare root according to permvec
;es:di->permvec
;dl = 2 for display, 0 for no display
	assume	ds:crypt26b
bifperm proc	far
	push	ds
	mov	ax,crypt26b
	mov	ds,ax
	and	flag1,0FFh-02h	;turn off display flag
	or	flag1,dl	;set display flag
	mov	[bp].quodb,di	;save permvec ptr
	mov	[bp].quoda,es
	lds	si,dword ptr [bp].bfkeyb
	mov	si,[si].bifsect ;permute intersect
	call	permrow 	;..by rows
	mov	di,offset hold+36	;now permute by the columns
	mov	cx,[bp].tempcnt
	mov	bx,[bp].quodb
bfprmlp2:
	mov	ds,[bp].quoda
	mov	ax,[bx] 		;permutation index
	mov	si,offset hold
	add	si,ax
	push	es
	pop	ds
	push	cx
	push	di
	mov	cx,[bp].tempcnt
bfprmlp3:
	mov	al,[si] 		;get letter in this column
	mov	[di],al
	add	si,[bp].tempcnt 	;next row, same column
	add	di,[bp].tempcnt
	loop	bfprmlp3		;this col all rows
	pop	di
	pop	cx
	add	bx,2			;next column
	inc	di
	loop	bfprmlp2		;for all columns
;clear any previous display
	call	permklar
	test	flag1,02h	;if no display wanted
	jz	bfprmxt 	;..exit
;show permuted coordinates
	lds	si,dword ptr [bp].quodb
	mov	cx,[bp].tempcnt
	mov	di,15*160+84
bfprmlp5:			;show column coordinates
	lodsw
	add	al,'1'
	mov	ah,0Fh
	stosw
	add	di,2
	loop	bfprmlp5
	lds	si,dword ptr [bp].quodb
	mov	cx,[bp].tempcnt
	mov	di,16*160+80
bfprmlp6:			;show row coordinates
	lodsw
	add	al,'1'
	mov	ah,0Fh
	stosw
	add	di,158
	loop	bfprmlp6
;show the permuted square
	mov	ax,crypt26b
	mov	ds,ax
	mov	si,offset hold+36
	mov	bx,16*160+84
	mov	ah,07h
	mov	cx,[bp].tempcnt
bfprmlp7:
	push	cx
	mov	cx,[bp].tempcnt
	mov	di,bx
bfprmlp8:
	lodsb
	stosw
	add	di,2
	loop	bfprmlp8
	pop	cx
	add	bx,160
	loop	bfprmlp7
bfprmxt:
	mov	ax,crypt26b	;return ptr to permuted key
	mov	es,ax
	mov	di,offset hold+36
	pop	ds
	ret
bifperm endp

;update key according to permutation
;es:di->permutation vector
;permuted root dimensions already in hold+36
bifix	proc	far
	push	ds
	mov	ax,crypt26b
	mov	ds,ax
	mov	[bp].quodb,di		;save perm vector ptr
	mov	[bp].quoda,es
;fix up the key rows letters
	lds	si,dword ptr [bp].bfkeyb
	mov	si,[si].bifrow	;permute the row table by rows
	push	si
	call	permrow
	mov	ax,crypt26b
	mov	ds,ax		;then move to row table
	mov	si,offset hold
	pop	di
	call	reprect
;fix up the key col letters
	lds	si,dword ptr [bp].bfkeyb
	mov	si,[si].bifcol
	push	si
	call	permrow
	mov	ax,crypt26b
	mov	ds,ax
	mov	si,offset hold
	pop	di
	call	reprect
;fix up the row counts
	lds	si,dword ptr [bp].bfkeyb
	mov	cx,[si].bifgrund
	mov	si,[si].bifrwcnt
	call	permcnt 	;permute the row counts
;fix up the col counts
	lds	si,dword ptr [bp].bfkeyb
	mov	cx,[si].bifgrund
	mov	si,[si].bifclcnt
	call	permcnt
;fix up the pale fits table if any
	les	bx,dword ptr [bp].bfkeyb
	mov	ax,es:[bx].bifgrund	;total fit table size
	mul	es:[bx].bifmaxn
	mov	cx,ax
	mov	ax,es:[bx].bifgrund
	mul	es:[bx].bifgrund
	sub	cx,ax			;pale coord fits size
	jcxz	bifix30 		;..none
	mov	[bp].tempcnt,cx 	;save for scanning
	mov	di,es:[bx].bifit
	add	di,ax			;start of pale cord fits
	mov	cx,es:[bx].bifgrund
	lds	si,dword ptr [bp].quodb
	xor	ax,ax
bifixlp:
	mov	dx,[si] 	;change all occurences of i to j
	or	dl,80h		;temporarily mark j
	push	di
	push	cx
	mov	cx,[bp].tempcnt
bifxlp2:
	jcxz	bifix10
	repne scasb
	jne	bifix10
	mov	es:[di-1],dl
	jmp	bifxlp2
bifix10:
	pop	cx
	pop	di
	add	si,2		;next j
	inc	ax		;next i
	loop	bifixlp
	mov	cx,[bp].tempcnt
bifxlp3:
	cmp byte ptr es:[di],0FFh
	je	bifix20
	and byte ptr es:[di],7Fh
bifix20:
	inc	di
	loop	bifxlp3
bifix30:
	call	newsect 	;rebuild keysquare intersect
	call	permklar
	pop	ds
	ret
bifix	endp

;clear previous permutation display
permklar proc near
	mov	es,[bp].videoa
	mov	bx,15*160+80
	mov	cx,7
	mov	ax,0720h
prmklrlp:
	push	cx
	mov	cx,13
	mov	di,bx
	rep stosw
	pop	cx
	add	bx,160
	loop	prmklrlp
	ret
permklar endp

;si = offset to rectangle, bifsect, bifrow, or bifcol
permrow proc   near
	les	di,dword ptr [bp].bfkeyb
	mov	cx,es:[di].bifgrund	;retrieve root
	mov	[bp].tempcnt,cx
	mov	dx,es:[di].bifside	;retrieve actual keysquare side
	mov	[bp].templen,dx
	mov	ax,crypt26b
	mov	es,ax
	mov	di,offset hold		;row perm destination
	mov	bx,[bp].quodb	;offset to permutation vector
prmrwlp1:			;permute by the rows into hold
	mov	ds,[bp].quoda	;get row index
	mov	ax,[bx]
	mul	[bp].templen
	mov	ds,[bp].bfkeya	;point to the rectangle
	push	cx
	push	si
	add	si,ax		;ith rectangle row
	mov	cx,[bp].tempcnt
	rep movsb
	pop	si
	pop	cx
	add	bx,2
	loop	prmrwlp1
	ret
permrow endp

;ds:si -> source
;di = offset to key rectangle, bifsect, bifrow, bifcol
reprect proc	near
	les	bx,dword ptr [bp].bfkeyb
	mov	cx,es:[bx].bifgrund
rprctlp1:
	push	cx
	push	di
	mov	cx,es:[bx].bifgrund
	rep movsb			;move one row to key
	pop	di
	pop	cx
	add	di,es:[bx].bifside	;next destination row
	loop	rprctlp1		;move all rows
	ret
reprect endp

;si->source counters
;cx = number of counters to permute
permcnt proc	near
	push	cx
	push	ds
	push	si
	mov	ax,crypt26b
	mov	es,ax
	mov	di,offset hold
	push	es
	push	di
	mov	bx,[bp].quodb		;offset to permutation vector
prmcntlp1:
	push	bx		;save permutation ptr
	push	ds		;save counters segment
	mov	ds,[bp].quoda	;ds:bx->permutation vector, ith item
	mov	bx,[bx]
	pop	ds		;recover counters segment
	shl	bx,1		;make ith word index
	mov	ax,[si][bx]	;get ith count
	stosw			;..and put it in temp hold
	pop	bx		;recover permutation ptr
	add	bx,2		;next permutation index
	loop	prmcntlp1
	pop	si
	pop	ds
	pop	di
	pop	es
	pop	cx
	rep movsw
	ret
permcnt endp

;put char in al into keysquare at bh,bl if no conflicts
;bx = keysquare position in root x root
bif_keysub proc far
	push	ds
	mov	dx,crypt26b
	mov	ds,dx
	mov	savechar,al
	mov	keycurs,bx
	and	flag1,01h	;turn off all flags but broken log
;check that keysquare position isn't solved
	les	bx,dword ptr [bp].bfkeyb
	xor	ax,ax
	mov	al,keyrow
	sub	ax,16
	mov	destrow,ax		;destination row index
	mul	es:[bx].bifside 	;row index
	mov	di,ax
	xor	ax,ax
	mov	al,keycol
	sub	ax,25
	shr	ax,1			;col index
	mov	destcol,ax		;destination column index
	add	di,ax
	mul	es:[bx].bifside
	add	di,es:[bx].bifsect	;row,col intersect
	cmp byte ptr es:[di],' '        ;if row,col is already solved
	je	keysub10
	mov	si,offset msg1		;..it can't be directly changed
keysuber:
	mov	di,24*160+80
	call	showmsg
	xor	ax,ax
	stc
	jmp	keysubxt
;check if character is already in row and/or column squares
keysub10:
	mov	ax,es:[bx].bifside
	mul	es:[bx].bifside
	mov	[bp].tempcnt,ax 	;save for col square scan
	mov	cx,ax
	mov	al,savechar		;scan the row square
	mov	di,es:[bx].bifrow
	repne scasb
	jne	keysub20		;doesn't exist in the rows
	mov	ax,di
	dec	ax
	sub	ax,es:[bx].bifrow
	xor	dx,dx
	div	es:[bx].bifside
	mov	orgrow,ax		;row index
	or	flag1,04h		;letter exists in row
keysub20:
	mov	al,savechar
	mov	cx,[bp].tempcnt
	mov	di,es:[bx].bifcol	;scan the col square
	repne scasb
	jne	keysub30		;doesn't exist in cols
	mov	ax,di
	dec	ax
	sub	ax,es:[bx].bifcol
	xor	dx,dx
	div	es:[bx].bifside
	mov	orgcol,ax		;col index
	or	flag1,08h		;letter exists in col
keysub30:
	xor	dx,dx		;log current state
	call far ptr biflog
	jnc	keysub40
keysub35:
	mov	ax,-1		;broken log, must quit
	stc
	jmp	keysubxt
keysub40:
	xor	bx,bx		;go to row, col, row/col, or none
	mov	bl,flag1
	and	bl,0FFh-01h
	shr	bx,1
	jmp	keycase[bx]
;entirely new letter, not matching any in current keysquare
keyl0:
	call	rowapp		;append letter to destination row
	jnc	keyl010
keyl005:
	mov	dx,1		;undo the log
	call far ptr biflog
	jc	keysub35	;catastrophic log failure
	mov	si,offset msg2	;doesn't fit in destination row
	jmp	keysuber
keyl010:
	call	colapp		;append letter to destination col
	jc	keyl005
;rebuild the keysquare
keyl020:
	call	newsect
	jc	keyl005 	;keysquare conflict detected
	jmp	keysubok

;new letter matches only a row letter in current key
keylr:
	call	colapp
	jc	keyl005
	mov	ax,orgrow
	cmp	ax,es:[bx].bifgrund
	ja	keylr10 		;pale coordinate
;original root coordinate, must match destination row
	cmp	ax,destrow
	jne	keyl005
	jmp	keyl020
;pale coordinate, collapse original row cord for both row and col into dest
keylr10:
	mov	ax,orgrow	;original pale coordinate
	mov	voncord,ax
	mov	ax,destrow	;destination root coordinate
	mov	zucord,ax
keylr20:
	call	movepale	;merge pale coordinate into root
	jc	keyl005 	;..won't merge
	dec	es:[bx].bifmaxn
	jmp	keyl020

;new letter matches only a col letter in current key
keylc:
	call	rowapp
	jc	keyl005
	mov	ax,orgcol
	cmp	ax,es:[bx].bifgrund
	ja	keylc10
	cmp	ax,destcol
	jne	keyl005
	jmp	keyl020
;pale coordinate, collapse original col cord for both row and col into dest
keylc10:
	mov	ax,orgcol
	mov	voncord,ax
	mov	ax,destcol
	mov	zucord,ax
	call	movepale
	jmp	keylr20

;new letter matches both a row and a col letter in current key
keylrc:
	mov	di,[bp].bfkeyb
	mov	ax,es:[di].bifgrund
	xor	bx,bx
	cmp	ax,orgrow
	ja	keylrc10
	or	bl,02h		;flag original row in pale
keylrc10:
	cmp	ax,orgcol
	ja	keylrc20
	or	bl,04h		;flag original col in pale
keylrc20:
	jmp	keycas2[bx]

;both coordinates in the pale
keylrc30:
	mov	ax,orgrow
	mov	voncord,ax
	mov	ax,destrow
	mov	zucord,ax
	call	movepale
	jnc	keylrc40
	jmp	keyl005
keylrc40:
	dec	es:[bx].bifmaxn
	mov	ax,orgcol
	cmp	ax,orgrow
	jne	keylc10
	jmp	keyl020

keysubok:
	xor	ax,ax
keysubxt:
	pop	ds
	ret
bif_keysub endp

;build new intersect
newsect proc	near
	push	ds
	les	di,dword ptr [bp].bfkeyb
	mov	ax,es:[di].bifside
	mul	es:[di].bifside
	mov	[bp].tempcnt,ax
	mov	cx,ax
	mov	al,' '
	mov	di,es:[di].bifsect
	rep stosb		;clear row,col intersect
	lds	bx,dword ptr [bp].bfkeyb
	mov	si,[bx].bifrow
	mov	cx,[bp].tempcnt
bifxlp4:
	lodsb
	cmp	al,' '          ;if not solved
	je	bifix40 	;..skip letter
	mov	di,[bx].bifcol	;else see if it is also in cols
	push	cx
	mov	cx,[bp].tempcnt
	repne scasb
	pop	cx
	jne	bifix40 	;..not a col letter, no intersect
	push	ax		;save the letter
	mov	ax,di		;figure the column index
	dec	ax
	sub	ax,[bx].bifcol
	xor	dx,dx
	div	[bx].bifside
	mov	di,ax		;save col index
	mov	ax,si
	dec	ax
	sub	ax,[bx].bifrow
	xor	dx,dx
	div	[bx].bifside
	mul	[bx].bifside
	add	di,ax
	pop	ax		;recover the letter
	add	di,[bx].bifsect
	cmp byte ptr es:[di],' '        ;better be blank
	je	bifix35
	stc			;conflicting key
	jmp short bifixt
bifix35:
	stosb
bifix40:
	loop	bifxlp4
	clc			;good return
bifixt:
	pop	ds
	ret
newsect endp

;append savechar to destrow row
rowapp	proc	near
	mov	bx,[bp].bfkeyb
	mov	di,destrow	;test for row fit
	shl	di,1
	add	di,es:[bx].bifrwcnt
	mov	cx,es:[di]	;get this row's count
	cmp	cx,es:[bx].bifgrund
	jb	rowapp10
	stc			;letter can't fit in row
	ret
rowapp10:
	inc word ptr es:[di]	;count the row letter
	mov	ax,destrow	;figure which row in row square
	mul	es:[bx].bifside
	mov	di,ax
	add	di,cx		;ith letter in this row
	add	di,es:[bx].bifrow
	mov	al,savechar
	mov	es:[di],al	;..and put char into row square
	clc
	ret
rowapp	endp

;append savechar to destcol column
colapp	proc	near
	mov	bx,[bp].bfkeyb
	mov	di,destcol	;test for column fit
	shl	di,1
	add	di,es:[bx].bifclcnt
	mov	cx,es:[di]	;get this col's count
	cmp	cx,es:[bx].bifgrund
	jb	colapp10
	stc			;doesn't fit in destination col
	ret
colapp10:
	inc word ptr es:[di]	;count the column letter
	mov	ax,destcol
	mul	es:[bx].bifside
	mov	di,ax
	add	di,cx
	add	di,es:[bx].bifcol
	mov	al,savechar
	mov	es:[di],al
	clc
	ret
colapp	endp

;move pale coordinate from voncord to root coordinate zucord
movepale proc	near
	mov	bx,[bp].bfkeyb
	mov	ax,voncord
	mul	es:[bx].bifside
	mov	[bp].quoda,ax	;from offset
	mov	ax,zucord
	mul	es:[bx].bifside
	mov	[bp].quodb,ax	;to offset
;move row letters from pale to root
	mov	di,es:[bx].bifrwcnt
	mov	si,di		;get destination letter count
	add	di,zucord
	add	di,zucord
	mov	[bp].templen,di ;save counter ptr
	mov	ax,es:[di]
	mov	di,[bp].quodb
	add	di,ax
	add	di,es:[bx].bifrow	;es:di->destination, past last letter
	add	si,voncord
	add	si,voncord
	mov	cx,es:[si]	;how many bytes to move, could be 0
	jcxz	mvpal20 	;..nothing to move
	add	ax,cx
	cmp	ax,es:[bx].bifgrund
	jna	mvpal10 	;source will fit in destination
	stc
	ret
mvpal10:
	mov word ptr es:[si],0	;no more letters in source
	mov	si,[bp].templen ;recover destination counter ptr
	mov	es:[si],ax	;update count
	mov	si,[bp].quoda
	add	si,es:[bx].bifrow	;es:si->source, 1st letter
	push	ds
	push	cx
	push	si
	mov	ds,[bp].bfkeya
	rep movsb
	pop	di
	pop	cx
	mov	al,' '
	rep stosb
	pop	ds
;move  pale column letters to root
mvpal20:
	mov	di,es:[bx].bifclcnt
	mov	si,di
	add	di,zucord
	add	di,zucord
	mov	[bp].templen,di ;save destination counter ptr
	mov	ax,es:[di]
	mov	di,[bp].quodb
	add	di,ax
	add	di,es:[bx].bifcol	;es:di->destination, past last letter
	add	si,voncord
	add	si,voncord
	mov	cx,es:[si]	;how many bytes to move, could be 0
	jcxz	mvpal40
	add	ax,cx
	cmp	ax,es:[bx].bifgrund
	jna	mvpal30 	;source will fit in destination
	stc
	ret
mvpal30:
	mov word ptr es:[si],0	;no more letters in source
	mov	si,[bp].templen ;recover destination counter ptr
	mov	es:[si],ax	;..and update destination letter count
	mov	si,[bp].quoda
	add	si,es:[bx].bifcol	;es:si->source, 1st letter
	push	ds
	push	cx
	push	si
	mov	ds,[bp].bfkeya
	rep movsb
	pop	di
	pop	cx
	mov	al,' '
	rep stosb
	pop	ds
;eliminate fit table entry
mvpal40:
	mov	ax,voncord		;first clear it
	mul	es:[bx].bifgrund	;..in case it's the last coord
	mov	di,es:[bx].bifit	;..and there is nothing to scoot
	add	di,ax			;..down
	push	di
	mov	cx,es:[bx].bifgrund
	mov	al,0FFh
	rep stosb
	mov	si,di
	pop	di
	mov	ax,es:[bx].bifside
	mul	es:[bx].bifgrund
	mov	cx,ax
	mov	ax,voncord
	inc	ax
	mul	es:[bx].bifgrund
	sub	cx,ax
	jcxz	mvpal50
	push	ds
	mov	ds,[bp].bfkeya
	rep movsb
	pop	ds
mvpal50:
	clc
	ret
movepale endp

;compute digraphic index of coincidence for es:di->bifid
bifdic	proc	far
	push	ds
	mov	ax,crypt26b	;data seg addressability
	mov	ds,ax
	push	cx
	push	es
	push	di
	push	si
	mov	[bp].quodb,di
	mov	[bp].quoda,es
	mov	si,offset save2b
	call	savefree
	call	freeadj
	les	di,dword ptr [bp].quodb
	mov	ax,es:[di].root1	;figure number of digraphs
	mul	es:[di].root1
	mov	[bp].bifsqr,ax
	mul	[bp].bifsqr		;625 or 1290 possible digraphs
	mov	digtot,ax		;save for later
	shl	ax,1			;get counter space
	mov	cx,ax
	call	getmem
	jnc	bifdic10
	stc			;flag failure
	jmp	bifdicxt
bifdic10:
	mov	digcntb,di
	mov	digcnta,es
	xor	ax,ax
	mov	cx,digtot
	rep stosw		;zero digraph counters
	les	di,dword ptr [bp].quodb
	mov	ax,es:[di].period ;figure number of digraphs per block
	mov	dx,ax
	shr	ax,1
	test byte ptr es:[di].period,01h	;if odd
	jz	bifdic20
	inc	ax				;..add one more
bifdic20:
	mov	[bp].tempcnt,ax
	mov	cx,es:[di].blocks	;for all full blocks
	push	ds
	lds	si,dword ptr es:[di].cipb
	mov	bx,es:[di].cnt
	pop	es
	mov	di,offset digcntb
	les	di,dword ptr es:[di]
bifdiclp:
	call	digblk			;count digraphs in this block
	add	si,dx			;next block
	loop	bifdiclp		;do for all blocks
;do short block if any and if it makes sense
	les	di,dword ptr [bp].quodb
	cmp	es:[di].kurz,0
	je	bifdic40
;only if period and short block agree on evenness or oddness
	mov	ax,es:[di].kurz
	xor	ax,es:[di].period
	test	al,01h
	jnz	bifdic40	;one is even, other odd,
	mov	ax,es:[di].kurz ;both even or both odd, count meaningful
	shr	ax,1
	test byte ptr es:[di].kurz,01h
	jz	bifdic30
	inc	ax
bifdic30:
	mov	[bp].tempcnt,ax
	mov	ax,crypt26b
	mov	es,ax
	mov	di,offset digcntb
	les	di,dword ptr es:[di]
	call	digblk			;count digraphs in last block
bifdic40:
	mov	ax,crypt26b	;re-establish addressability
	mov	ds,ax
;compute +/counts x counts-1
	les	di,dword ptr digcntb
	mov	cx,digtot
	mov	[bp].tempcnt,0		;zero non-zero digraph counter
	xor	bx,bx			;accumulator
bfdclp2:
	mov	ax,es:[di]	;this digraph count
	add	[bp].tempcnt,ax ;sum of all digraphs
	cmp	ax,1		;if 0 = count*count-1
	jbe	bifdic50	;..don't bother with arithmetic
	dec	ax		;count-1
	mul word ptr es:[di]	;count*(count-1)
	add	bx,ax		;sigma count[i]*count[i]-1
bifdic50:
	add	di,2
	loop	bfdclp2 	;do for all counts
	mov	[bp].templen,bx ;save sigma counts
;compute (SIGMA counts)/digcnt(digcnt-1)/digtot
	finit
	fild	[bp].templen	;sigmacounts
	fild	[bp].tempcnt	;digcounts,sigmacount
	dec	[bp].tempcnt
	fimul	[bp].tempcnt	;dig(dig-1),sigmacount
	fild	digtot		;ie 625, dig(dig-1), sigmacount
	fdiv			;dig(dig-1)/625, sigmacount
	fdiv			;dic = sigmacount/(dig*(dig-1)/625)
;return with DIC in st(0)
	clc			;good return
bifdicxt:
	mov	si,offset save2b	;return counter space
	call	freemem
	pop	si
	pop	di
	pop	es
	pop	cx
	pop	ds
	ret
bifdic	endp
	assume	ds:nothing

;count digraphs in a block
digblk	proc	near
	push	cx
	push	dx
	push	si
	mov	cx,[bp].tempcnt
digblklp:
	mov	al,[si] 	;get hi order letter
	push	cx
	push	es
	push	di
	les	di,dword ptr [bp].bifalfb	;convert to alf index al
	mov	cx,[bp].bifsqr
	repne scasb
	dec	di
	sub	di,[bp].bifalfb
	mov	[bp].templen,di 	;save hi order digit base bifsqr
	mov	al,[si][bx]		;get lo order letter
	mov	di,[bp].bifalfb 	;convert to alf index al
	mov	cx,[bp].bifsqr
	repne scasb
	mov	ax,di
	dec	ax
	sub	ax,[bp].bifalfb
	mul	[bp].bifsqr		;bifsqr decode hi,lo
	add	ax,[bp].templen
	shl	ax,1
	pop	di
	pop	es
	push	di
	add	di,ax
	inc word ptr es:[di]	;count this digraph
	pop	di
	pop	cx
	add	si,2		;skip col,row letters
	loop	digblklp	;do all digraphs this block
	pop	si
	pop	dx
	pop	cx
	ret
digblk	endp

crypt26 ends
	end
