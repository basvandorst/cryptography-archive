	page	53,132
	include frame.str
	include index.str
;if this structure changes, make same changes to motndx.str
motndx	struc
motoff	dw	0	;offset to start of cipher word
motaln	db	0	;apparent length of word
motrln	db	0	;real length of word
motflg	db	0	;00 = in dictionary
			;01 = in word file
			;80 = not a word
			;40 = not in dictionary
motsarg db 8 dup(0)	;pattern search argument
motlocb dw	0	;file offset to word list
motloca dw	0
motlng	dw	0	;list length
motwcnt dw	0	;initial word count
motpcnt dw	0	;present word count
motno	dw	0	;word number
motdf	dw	0	;degrees of freedom = count of unique letters
motacnt dw 16 dup (0)	;cipher letter counts; 0=duplicated letter
motndx	ends
	include crypt10b.asm
	extrn	showmsg:far,getmem:far,qsort:far
crypt10 segment 'crya'
	assume	cs:crypt10,ds:crypt10b
	public	openpat,mot
openpat proc	far
	push	ds
	mov	ax,crypt10b
	mov	ds,ax		;data addressability
	mov	es,ax
	test	[bp].flaga,04h	;if no index, or bad index
	jnz	opnpat40	;..exit
	cmp	[bp].hndx,0	;if not opened
	jne	opnpat40
	mov	ax,3D00h	;..open the index
	mov	dx,offset fndx
	int	21h
	jnc	opnpat10
opnpat05:
	or	[bp].flaga,04h	;flag bad or no index
	jmp short opnpat40
opnpat10:
	mov	[bp].hndx,ax	;save index handle
	mov	bx,ax		;verify that this is the index file
	mov	cx,16
	mov	ah,3Fh
	mov	dx,offset hdr
	int	21h
	jc	opnpat05
	mov	si,offset hdr	;header must agree with filemark
	mov	di,offset filemark
	mov	cx,12
	repe cmpsb
	jne	opnpat05
	mov	ax,hdrcnt	;save the count of patterns
	mov	[bp].ndxcnt,ax
	mov	ax,4200h	;..then reset index to start
	xor	cx,cx
	xor	dx,dx
	int	21h
	test	[bp].flaga,08h	;if bad or no dictionary
	jne	opnpat40	;..leave
	cmp	[bp].hdct,0	;if dictionary not opened
	jne	opnpat40
	mov	dx,offset fdct	;open it
	mov	ax,3D00h
	int	21h
	jnc	opnpat30
	or	[bp].flaga,08h	;flag bad or unfound dictionary
	jmp short opnpat40
opnpat30:
	mov	[bp].hdct,ax	;save file handle
opnpat40:
	pop	ds
	ret
openpat endp

;build an index for every word in cipher
mot	proc	far
	push	ds
	mov	ax,crypt10b
	mov	ds,ax		;data addressability
	les	di,dword ptr [bp].freehb
	mov	[bp].motb,di	;start of index, offset
	mov	[bp].mota,es	;..segment
	mov	[bp].motzahl,0	;count of cipher words
	les	di,dword ptr [bp].sizeb ;save present freespace
	mov	saveb,di		;..in case we must restore
	mov	savea,es
	mov	cx,[bp].lines	;for every line
	les	di,dword ptr [bp].sbuffb
motlp:
	call	moline		;parse for words
	test	[bp].flaga,0Ch	;if fatal error
	jnz	bailout 	;..restore memory and quit
	add	di,81		;next line
	loop	motlp
	cmp	[bp].motzahl,0	;if no indexable words found
	je	empty		;..say empty index and quit
	or	[bp].flaga,10h	;flag word indexes made
	mov	ax,[bp].motzahl ;set up for sort
	mov	[bp].srtn,ax	;..number of items
	les	di,dword ptr [bp].motb
	mov	[bp].srtentb,di ;..where they start
	mov	[bp].srtenta,es
	mov	[bp].srtcmpb,offset compare	;..my compare routine
	mov	[bp].srtcmpa,cs
	mov	ax,type motndx
	add	ax,32
	mov	[bp].srtlen,ax	;..length of a word index
	mov	[bp].srtdir,0	;..sort ascending
	call	qsort
	les	di,dword ptr [bp].srtcmpb	;..indexes for indexes
	mov	[bp].motib,di
	mov	[bp].motia,es
	pop	ds
	ret
;no indexable words found
empty:
	mov	si,offset msg3	;tell user empty index
	mov	di,24*160+80
	call	showmsg
	stc
;fatal error exit, file error or no memory
bailout:
	mov	[bp].motzahl,0	;no word index
	les	di,dword ptr saveb	;restore memory size
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	les	di,dword ptr [bp].motb
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	pop	ds
	ret
mot	endp
moline	proc	near		;for one line
	push	cx
	push	di
	xor	cx,cx
	mov	cl,es:[di]	;length of line
	inc	di
motlp2:
	jcxz	motxit		;..empty line
	mov	al,' '
	repe scasb		;look for start of word
	je	motxit		;..no word start
	dec	di
	inc	cx
	mov	motemp.motoff,di	;mark start of cipher word
	mov	si,di
	repne scasb		;look for end of word
	jne	motl10		;end of line is end of word
	dec	di
	inc	cx
motl10:
	mov	dx,di
	sub	dx,si		;apparent length of word
	mov	motemp.motaln,dl
	xchg	si,di
	call	extract 	;extract cipher letters
	xchg	si,di
	call	dupword 	;if this is a dup word
	jc	motlp2		;..ignore it
	call	makepat 	;make canonical pattern
	jc	motlp2		;skip word if no pattern
	call	duppat		;check if already got this pattern
	jnc	motl20		;..yes, don't seek
	call	seek		;seek pattern in dictionary index
	jc	motl30		;not here, or file error
	mov	ax,hdr.patlocb	;complete word index, file location
	mov	motemp.motlocb,ax
	mov	ax,hdr.patloca
	mov	motemp.motloca,ax
	mov	ax,hdr.listlen	;..length of word list
	mov	motemp.motlng,ax
	mov	ax,hdr.wcount
	mov	motemp.motwcnt,ax	;initial word count
	mov	motemp.motpcnt,ax	;present word count
	mov	motemp.motflg,0 ;flag entry in dictionary file
motl20:
	call	movndx		;move the word index
	jnc	motl25		;move ok
	mov	si,offset msg2	;tell user not enough memory
	mov	di,24*160+80
	call	showmsg
	or	[bp].flaga,0Ch	;flag patterns broken
	jmp short motxit
motl25:
	jmp	motlp2
motl30:
	test	[bp].flaga,04h	;if index file isn't broken
	jz	motl20		;we don't have this entry
motxit:
	pop	di
	pop	cx
	ret
moline	endp

;on entry, es:di-> start of cipher word
;extract cipher letters only, and find real length
extract proc	near
	push	cx
	push	di
	push	si
	push	bx
	call	blnkwork	;blank the work area
	xor	si,si	;index into cryptword, letter offsets
	mov	motemp.motrln,0 ;zero real length
	xor	cx,cx
	mov	cl,motemp.motaln	;apparent length of word
xtrctlp:
	mov	al,es:[di]	;get supposed cipher letter
	call	checkc	;if not a cipher letter
	jc	xtrct10 ;..ignore it
	mov	cryword[si],al	;else save it in work area
	mov	cryoff[si],bl	;..with its offset
	inc	motemp.motrln	;..and count real letters
	inc	si	;next cipher letter destination
xtrct10:
	inc	di		;next cipher letter
	loop	xtrctlp
	pop	bx
	pop	si
	pop	di
	pop	cx
	ret
extract endp

blnkwork proc	near
	push	cx
	push	di
	push	es
	push	ds
	pop	es
	mov	di,offset cryword	;blank cipher word area
	mov	al,' '
	mov	cx,80
	rep stosb
	mov	di,offset cryoff	;zero cipher letter offsets
	xor	al,al
	mov	cx,80
	rep stosb
	pop	es
	pop	di
	pop	cx
	ret
blnkwork endp

;check if al is a defined cipher letter, jne if it isn't
;					 je if it is
checkc	proc near
	push	es
	push	di
	push	cx
	les	di,dword ptr [bp].alfb	;defined cipher alphabet
	mov	cx,es:[di]		;..length
	add	di,2
	mov	bx,di
	repne scasb
	jne	chkc10		;not a cipher letter
	dec	di
	sub	di,bx
	mov	bx,di		;cipher letter offset
	clc			;flag letter found
chkcxit:
	pop	cx
	pop	di
	pop	es
	ret
chkc10:
	stc			;flag letter not found
	jmp	chkcxit
checkc	endp

makepat proc	near
	push	cx
	push	bx
	push	di
	push	si
	push	es
	push	ds
	pop	es
	mov	motemp.motflg,080h	;assume non-word
	cmp	motemp.motrln,0 ;if only a decoration
	jne	mkpt00
	jmp	nopat		;..assumption ok
mkpt00:
	mov	motemp.motflg,040h	;assume not in dictionary
	cmp	motemp.motrln,16	;if too long
	jna	mkpt02
	jmp	nopat			;..assumption ok
mkpt02:
;word may be in dictionary
	xor	ax,ax		;clear pattern work area
	mov	di,offset patwork
	mov	cx,8
	rep stosw
	mov	di,offset cryword
	mov	si,offset patwork
	xor	cx,cx
	mov	cl,motemp.motrln
	mov	ah,1
mkptlp:
	cmp byte ptr [si],0	;if this letter is done
	jne	mkpt10		;..try next letter
	call	dorep		;else replace all occurences with number
	inc	ah		;next pattern number
mkpt10:
	inc	si	;next pattern byte
	inc	di	;next cipher letter
	loop	mkptlp
	dec	ah	;count of unique letters
	cmp	ah,15	;if more than 15 unique letters
	ja	nopat	;..word can't be in dictionary
	mov byte ptr motemp.motdf,ah	;set degrees of freedom
	mov	di,offset motemp.motsarg ;pack the pattern
	mov	cx,4
	xor	ax,ax
	rep stosw		;clear the seek argument
	std			;go backwards
	mov	di,offset motemp.motsarg+7
	mov	si,offset patwork
	mov	cl,motemp.motrln
	mov	cryword+79,0
	add	si,cx
	sub	si,2
	shr	cx,1
mkptlp2:
	jcxz	mkpt20
	lodsw			;pick up two bytes
	shl	al,1		;pack hi-order byte
	shl	al,1
	shl	al,1
	shl	al,1
	or	al,ah		;with lo-order byte
	stosb			;stuff away
	loop	mkptlp2
mkpt20:
	test	motemp.motrln,01h	;if length is even
	jz	mkpt30			;..we're done
	mov	al,[si+1]		;else get last digit
	stosb
mkpt30:
	cld			;forward direction
;get cipher letter counts for every unique letter
	xor	si,si		;index into patwork, cryoff
	xor	bx,bx		;index into motacnt
	xor	dx,dx		;highest unique letter
	mov	cl,motemp.motrln ;number of total letters
mkptlp3:
	mov	motemp.motacnt[bx],0	;assume letter occurred before
	cmp	patwork[si],dl
	jna	mkpt40
;first occurrence of this letter
	mov	dl,patwork[si]	;next unique letter
	les	di,dword ptr [bp].zahlb ;->letter counts
	xor	ax,ax
	mov	al,cryoff[si]	;alphabetic offset
	shl	ax,1
	add	di,ax
	mov	ax,es:[di]	;letter count
	mov	motemp.motacnt[bx],ax
mkpt40:
	inc	si
	add	bx,2
	loop	mkptlp3
	clc			;flag we have a pattern
mkptxit:
	pop	es
	pop	si
	pop	di
	pop	bx
	pop	cx
	ret
nopat:
	stc
	jmp	mkptxit
makepat endp
dorep	proc	near
	push	cx
	push	di
	mov	al,[di] 	;get the cipher letter
doreplp:
	jcxz	dorep10 	;while there are remaining letters
	repne scasb		;find all occurences
	jne	dorep10
	mov	bx,di
	dec	bx
	sub	bx,offset cryword ;offset of this letter's occurence
	mov	patwork[bx],ah
	jmp	doreplp
dorep10:
	pop	di
	pop	cx
	ret
dorep	endp

;Chances are good we already have found this pattern, if so
;fill it in without searching file
duppat	proc	near
	cmp	[bp].motzahl,0
	jne	duppat00
	stc		;flag new pattern, search
	ret		;..the index file
duppat00:
	push	cx
	push	es
	push	si
	push	di
	mov	cx,[bp].motzahl 	;count of word indexes
	les	di,dword ptr [bp].motb	;->start of word indexes
	lea	di,[di].motsarg
	lea	si,motemp.motsarg	;->start of current pattern
duppatlp:
	push	cx
	push	di
	push	si
	mov	cx,8
	repe cmpsb			;look for same pattern
	pop	si
	pop	di
	pop	cx
	je	duppat10	;got it
	add	di,32+type motndx	;next word index
	loop	duppatlp
	stc			;new pattern
duppatxt:
	pop	di
	pop	si
	pop	es
	pop	cx
	ret
duppat10:
	mov	al,es:[di-1]	;word flags
	mov	motemp.motflg,al
	mov	ax,es:[di+8]	;file location, hi order
	mov	motemp.motlocb,ax
	mov	ax,es:[di+10]	;..		lo order
	mov	motemp.motloca,ax
	mov	ax,es:[di+12]	;byte length of list
	mov	motemp.motlng,ax
	mov	ax,es:[di+14]	;number of words in list
	mov	motemp.motwcnt,ax
	mov	motemp.motpcnt,ax
	clc
	jmp	duppatxt
duppat	endp

;binary search of ENGLISH.NDX for seekarg, built by makepat
seek	proc	near
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	es
	push	ds
	pop	es
;assume pattern not in dictionary
	mov	cx,5		;zero fields filled from pattern index
	xor	ax,ax	;location, length, counts
	mov	di,offset motemp.motlocb
	rep stosw
	mov	ax,[bp].ndxcnt	;count of index entries
	mov	top,0
	inc	ax		;on purpose, one more than is there
new_bot:
	mov	bottom,ax	;..for bottom (makes math easier)
seek00:
	mov	ax,top		;compute the middle
	add	ax,bottom	;top+bottom/2
	shr	ax,1		;new middle
	cmp	ax,top		;if middle = top
	je	not_here	;..argument not in index
	cmp	ax,bottom	;if middle = bottom
	je	not_here	;..argument not in index
	call	getndx		;get english.ndx[middle]
	jc	seekxt		;abandon ship, file error
	mov	cx,8		;compare argument to index entry
	mov	si,offset motemp.motsarg
	mov	di,offset hdr
	repe cmpsb
	je	found		;got it!
	jb	new_bot 	;must be in lower half
;new top, must be in upper half
	mov	top,ax
	jmp	seek00
not_here:
	stc
	jmp short seekxt
found:
	clc
seekxt:
	pop	es
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	ret
seek	endp
getndx	proc	near
	push	ax
	mul	c16		;convert index origin 1 to absolute
	mov	cx,dx		;set up regs for dos
	mov	dx,ax		;cx,dx byte location in file of this entry
	mov	ax,4200h	;lseek
	mov	bx,[bp].hndx	;index file handle
	int	21h		;set the file pointer
	mov	dx,offset hdr	;16 byte buffer mapped by index.str
	mov	cx,16		;one entry
	mov	ah,3Fh		;read
	int	21h
	jc	geterr
	cmp	ax,cx
	jne	geterr
	clc			;read ok
	pop	ax
	ret
geterr:
	mov	si,offset msg1	;report the file error
	mov	di,24*160+80
	call	showmsg
	or	[bp].flaga,04h	;mark the index as broken
	pop	ax
	stc			;bad news
	ret
getndx	endp

;move cipher word index to buffer
movndx	proc	near
	push	cx
	push	si
	push	di
	push	es
	mov	cx,type motndx	;size of index
	add	cx,32		;..plus max cipher word length and offsets
	call	getmem		;get storage for index entry
	jc	mvndxt		;..failed
	mov	ax,[bp].motzahl ;count the index entry
	inc	ax
	mov	[bp].motzahl,ax
	mov	motemp.motno,ax ;..and identify this entry
	mov	cx,type motndx
	mov	si,offset motemp	;move 1st part of index
	rep movsb
	mov	cx,16		;move extracted cipher word
	mov	si,offset cryword
	rep movsb
	mov	cx,16		;move letter offsets
	mov	si,offset cryoff
	rep movsb
	clc			;ok
mvndxt:
	pop	es
	pop	di
	pop	si
	pop	cx
	ret
movndx	endp

;if this word duplicates a previous word, set flag to ignore it
dupword proc	near
	push	es
	push	cx
	push	si
	push	di
	mov	cx,[bp].motzahl ;count of previous words
	jcxz	dupw20		;..aren't any
	les	di,dword ptr [bp].motb	;->previous words
dupwlp:
	mov	bx,di
	mov	al,motemp.motrln	;compare word lengths
	cmp	al,es:[di].motrln
	jne	dupw10			;..try next previous word
	mov	si,offset cryword	;->current word
	push	cx
	xor	cx,cx
	mov	cl,motemp.motrln	;length of current word
	add	di,type motndx		;->previous word
	repe cmpsb		;if current word matches previous
	pop	cx
	je	dupw30		;..flag ignore it
dupw10:
	add	bx,32+type motndx	;next word
	mov	di,bx
	loop	dupwlp
dupw20:
	clc			;flag new word
dupwxt:
	pop	di
	pop	si
	pop	cx
	pop	es
	ret
dupw30:
	stc			;flag old word
	jmp	dupwxt
dupword endp

	assume	ds:nothing
compare proc	far
	mov	ax,[si].motwcnt ;get list count of A
	cmp	ax,es:[di].motwcnt	;and compare to B's
	ret
compare endp
crypt10 ends
	end
