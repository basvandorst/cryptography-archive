	page	53,132
	include frame.str
	include motndx.str
	include crypt9b.asm
	extrn	freeadj:far,getmem:far,showmsg:far,fit:far,qsort:far
	extrn	int2asc:far,psort:far,savelist:far,readlist:far
crypt9	segment 'crya'
	assume	cs:crypt9,ds:crypt9b
	public	get1st,get2nd,accord
;get first list of pattern words
;dx=current cursor position in word index list
get1st	proc	far
	push	ds
	mov	ax,crypt9b	;data addressability
	mov	ds,ax
	mov	wrdpos,dx	;save cursor position
;figure item's index on display
	xor	ax,ax
	mov	al,dh		;screen row number in al
	sub	al,17		;item index, this screen
	add	ax,[bp].motcur	;item index
	mov	olditem,ax	;save item index for 2nd list exit
	call	getptr		;get ptr to current word index in itema:itemb
	les	di,dword ptr itemb
	test	es:[di].motflg,0C0h	;if we don't have pattern
	jz	get1s10
	mov	si,offset msg1		;..tell user
get1s08:
	mov	di,24*160+80
	call	showmsg
	stc
	jmp	get1xt
get1s10:
	test	[bp].flaga,0A0h ;if we have any list up
	jz	get1s30
	les	di,dword ptr freeb ;..return memory
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr saveb
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	test	[bp].flaga,80h	;save second list if any
	jz	get1s20
	and	[bp].flaga,7Fh	;clear second list flag
	lea	bx,[bp].secb
	call	savelist	;save 2nd list
get1s20:
	and	[bp].flaga,0DFh ;clear first list flag
	lea	bx,[bp].primb
	call	savelist	;save first list
get1s30:
	les	di,dword ptr itemb	;set word index ptr
	mov	[bp].word1b,di
	mov	[bp].word1a,es
	mov	[bp].quodb,di		;tell accord which word
	mov	[bp].quoda,es
	les	di,dword ptr [bp].freehb	;save memory
	mov	freeb,di			;..in case of failure
	mov	freea,es			;..or repeat
	les	di,dword ptr [bp].sizeb
	mov	saveb,di
	mov	savea,es
;figure space to reserve in case of sort
	les	di,dword ptr itemb
	xor	ax,ax
	mov	al,es:[di].motrln	;length of word
	mul	es:[di].motpcnt 	;..times number of words
	mov	cx,ax
	test	[bp].modeb,04h		;if sorting
	jnz	nosort
	jmp	sorting 		;..reserve the hole
;not sorting
nosort:
	test	es:[di].motflg,01h	;if list in file, got right length
	jz	nosrt50
	mov	bx,offset freeb
	call	tomem
	jnc	nosrt10
nosrt08:
	mov	si,offset msg3		;tell user no memory
	jmp	get1s08 		;and leave
nosrt10:
	mov	[bp].primb,di		;destination of list
	mov	[bp].prima,es
	lea	bx,[bp].primb
	call	readlist		;get the list
	jnc	nosrt20
	mov	si,offset msg12 	;tell user file error
	jmp	get1s58 		;restore free space and leave
nosrt20:
	lea	bx,[bp].primb	;1st list ptr
	call	agree		;purge list of contradictory words
	les	di,dword ptr [bp].word1b
	mov	ax,es:[di].motpcnt
	call	newnum		;display new total of words
	cmp	es:[di].motpcnt,0	;list may be empty
	jne	nosrt30
nosrt28:
	mov	si,offset msg2		;tell user list is empty
	jmp	get1s58 		;return storage and leave
nosrt30:
	jmp	get1s110	;list read, no clean up, take good exit

nosrt50:			;get list from dictionary
	mov	cx,es:[di].motlng
	mov	bx,offset freeb
	call	tomem
	jc	nosrt08
	mov	[bp].primb,di
	mov	[bp].prima,es
	lea	bx,[bp].primb
	call	getlist
	jnc	nosrt60
	mov	si,offset msg4		;tell user dictionary broken
	jmp	get1s58 		;free space and leave
nosrt60:
	les	di,dword ptr [bp].word1b
	mov	ax,es:[di].motpcnt
	call	newnum		;display new number of words
	cmp	es:[di].motpcnt,0	;nothing may have survived filtering
	je	nosrt28
	mov	bx,offset freeb ;free up excess storage
	lea	si,[bp].primb
	call	adjfree
	jmp	get1s110
;sorting
sorting:
	mov	bx,offset freeb
	call	tomem
	jnc	get1s42
	mov	si,offset msg3
	jmp	get1s08 	;no memory
get1s42:
	mov	destb,di	;final destination
	mov	desta,es
	les	di,dword ptr [bp].word1b
	test	es:[di].motflg,01h	;if list in file
	jnz	get1s50 		;..got right size to read list
	mov	cx,es:[di].motlng	;else get dictionary size
get1s50:
	mov	bx,offset freeb
	call	tomem
	jnc	get1s52
	mov	si,offset msg5
	jmp	get1s08
get1s52:
	mov	[bp].primb,di	;list destination for sorting
	mov	[bp].prima,es
	lea	bx,[bp].primb
	les	di,dword ptr [bp].word1b
	test	es:[di].motflg,01h	;if list in dictionary
	jz	get1s60 		;..use getlist from dictionary
	call	readlist		;else get list from its file
	jc	get1s56
	lea	bx,[bp].primb		;using 1st list
	call	agree			;purge contradictory words
	jmp short get1s62
get1s56:
	mov	si,offset msg12 	;tell user file error
get1s58:
	les	di,dword ptr freeb	;restore free space
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr saveb
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	jmp	get1s08 		;leave with message
get1s60:
	call	getlist
	jc	get1s58
get1s62:
	les	di,dword ptr [bp].word1b
	mov	ax,es:[di].motpcnt
	call	newnum			;display number of words in list
	cmp	es:[di].motpcnt,0	;possible no words survived filtering
	jne	get1s70
	mov	si,offset msg2		;tell user list is empty
	jmp	get1s58 		;restore space and leave
get1s70:
	mov	ax,es:[di].motpcnt	;get space for fitting
	mov	cl,3		;number of words
	shl	ax,cl		;..times 8
	mov	cx,ax
	mov	bx,offset freeb
	call	tomem
	jnc	get1s90
;problem here, can't sort by probabilities
;move list to its destination, free up excess memory, and tell user
get1s88:
	call	movlist 	;simple move, no index
	mov	si,offset msg5	;tell user list can't be sorted
	jmp	get1s08
;fit the list to cipher word by chi-squared, goodness of fit
get1s90:
	mov	[bp].prob1b,di	;anchor probability buffer
	mov	[bp].prob1a,es
	lea	si,[bp].primb	;point to this list and
	call	fit		;..compute chi-squared for each word
;now sort in probability order
	les	di,dword ptr [bp].word1b
	cmp	es:[di].motpcnt,1 ;if only one item
	jne	get1s100	;..skip the sort
	call	movlist 	;there are lots of lists with only one item
	jmp	get1s105	;..free storage and leave
;we really have to sort
get1s100:			;prepare sort's parameters
	mov	ax,es:[di].motpcnt
	mov	[bp].srtn,ax	;number of things to sort
	mov	[bp].srtcmpb,offset compare	;compare routine
	mov	[bp].srtcmpa,cs
	les	di,dword ptr [bp].prob1b
	mov	[bp].srtentb,di ;things to sort (chi-squared values)
	mov	[bp].srtenta,es
	mov	[bp].srtlen,8	;size of things to sort
	mov	[bp].srtdir,0	;sort ascending
	call	qsort		;if not enough memory to sort
	jnc	get1s102
;sort failed, return memory, tell user to use unsorted list
	mov	bx,offset freeb ;to reset storage ab ovo
	lea	si,[bp].primb	;which list to reset for
	call	adjfree 	;fit final storage to actual used size
	jmp	get1s88 ;move list to final destination, flag list up, etc
;now, order the list in its final destination
get1s102:
	les	di,dword ptr itemb	;get the word length
	mov	al,es:[di].motrln
	mov byte ptr [bp].srtlen,al
	mov	cx,es:[di].motpcnt	;for all words
	les	di,dword ptr destb	;->destination ptr
	push	ds
	lds	si,dword ptr [bp].srtcmpb ;->indexes ptr
get1slp2:
	lodsw				;get the index
	mul	[bp].srtlen	;offset to source
	push	ds
	push	si
	push	cx
	mov	cx,[bp].srtlen	;item length
	mov	ds,[bp].prima	;source paragraph
	mov	si,ax
	rep movsb
	pop	cx
	pop	si
	pop	ds
	loop	get1slp2
	pop	ds
get1s105:
	mov	bx,offset freeb
	lea	si,[bp].primb
	call	adjfree 	;return temporary storage
	les	di,dword ptr destb
	mov	[bp].primb,di
	mov	[bp].prima,es
get1s110:
	or	[bp].flaga,20h	;flag 1st list is up
;note: above OR clears carry flag, thus indicating good return
get1xt:
	pop	ds
	ret
get1st	endp

;get 2nd list of pattern words
;dx=current cursor position in word index list
get2nd	proc	far
	push	ds
	mov	ax,crypt9b	;data addressability
	mov	ds,ax
;you don't get a second list unless you have a 1st list
	test	[bp].flaga,20h	;if first list is up
	jnz	gt2n00		;..ok
	mov	si,offset msg7	;else say choose primary list first
gt2ner:
	mov	di,24*160+80
	call	showmsg
	stc			;flag error to be cleared
	jmp	gt2nxt		;..and leave
;there is a primary list.  is it ok to get a secondary list?
gt2n00:
	mov	wrdpos,dx	;save cursor position
;set itemb:itema to point to word index
	call	getptr		;get ptr to selected word index
;don't get a second list if the first list is empty
	les	di,dword ptr [bp].word1b
	cmp	es:[di].motpcnt,0
	jne	gt2n02
	mov	si,offset msg10 ;first list is empty
	jmp	gt2ner
gt2n02:
;don't allow secondary list to be the same as the primary list
	cmp	[bp].word1b,dx	;if secondary word index
	jne	gt2n10
	cmp	[bp].word1a,ax	;..is the same as primary
	jne	gt2n10
	mov	si,offset msg8	;..tell user of error
	jmp	gt2ner		;..and leave.
;secondary word is properly different from primary word
gt2n10:
;is there a secondary word list for this word?
	les	di,dword ptr itemb	;itemb set by getptr
	test	es:[di].motflg,0C0h	;if no associated dictionary list
	jz	gt2n20
	mov	si,offset msg1		;..tell user
	jmp	gt2ner			;..and leave.
;there is a list in the dictionary for this word
gt2n20:
	cmp	es:[di].motpcnt,0 ;if 2nd list is empty
	jne	gt2n22
	mov	si,offset msg11   ;..tell user
	jmp	gt2ner		  ;..and leave
gt2n22:
	test	[bp].flaga,80h	;if there is a current 2nd list
	jz	gt2n30
	and	[bp].flaga,7Fh	;..save it and reclaim space
	lea	bx,[bp].secb	;indirect ptr to 2nd word list
	call	savelist	;save the list as a disk file
	jnc	gt2n24		;save list succeeded
	mov	si,offset msg12 ;..else tell user
	jmp	gt2ner		;..and leave
gt2n24:
	les	di,dword ptr free2b	;reset free ptr
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr save2b	;reset byte count
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
;current second list saved and cleared
gt2n30:
	les	di,dword ptr itemb
	mov	[bp].word2b,di	;set second word index ptr
	mov	[bp].word2a,es
	mov	[bp].quodb,di		;tell accord which word
	mov	[bp].quoda,es
	les	di,dword ptr [bp].freehb	;save memory
	mov	free2b,di			;..in case of failure
	mov	free2a,es			;..or repeat
	les	di,dword ptr [bp].sizeb
	mov	save2b,di
	mov	save2a,es
;if there are letters in common between the 1st and second lists,
;sort the 1st list on common letters, then get second list and sort it on
;common letters.  if no letters in common, the lists are irreducible, so
;just get the second list.
	push	ds		;switch segments
	pop	es
	assume	es:crypt9b,ds:nothing
;gather word index info in one segment for ease
	lds	si,dword ptr [bp].word1b
	xor	cx,cx
	mov	cl,[si].motrln	;length of first word
	mov	w1len,cx
	mov	di,offset w1cnt
	mov	cx,16
	lea	si,[si].motacnt ;retrieve the counts
	rep movsw		;..into this data segment
	mov	cx,16		;length again
	rep movsb		;retrieve cipher letters into this segment
	lds	si,dword ptr [bp].word2b
	mov	cl,[si].motrln	;length of second word
	mov	w2len,cx
	lea	si,[si].motwrd	;retrieve cipher letters into this segment
	mov	di,offset word2
	mov	cx,16
	rep movsb
	push	es		;restore segments
	pop	ds
	assume	ds:crypt9b,es:nothing
	mov	shrcnt,0	;clear shared letters count
	xor	bx,bx		;index into counts
	xor	dx,dx		;index into common letter offsets
	mov	cx,w1len	;length of 1st word
	mov	si,offset word1
gt2lp1:
	lodsb		;get 1st word letter
	cmp	w1cnt[bx],0	;skip if letter is duplicated in this word
	je	gt2n40
	mov	di,offset word2
	push	cx
	mov	cx,w2len	;length of second word
	repne scasb		;see if 1st word's letter occurs in 2nd word
	pop	cx
	jne	gt2n40		;..it doesn't
	inc	shrcnt		;count the common letter
	xchg	bx,dx
	mov	ax,si		;offset to coincident letter in 1st word
	dec	ax
	sub	ax,offset word1
	mov	shroff[bx],al	;..and save
	mov	ax,di		;offset to coincident letter in 2nd word
	dec	ax
	sub	ax,offset word2
	mov	shroff[bx+1],al ;..and save
	add	bx,2
	xchg	bx,dx
gt2n40:
	add	bx,2		;next count
	loop	gt2lp1		;do for all letters of 1st word
;create the combined pattern of the two words together
	mov	cx,32
	xor	al,al
	mov	di,offset joinpta	;clear cipher words pattern area
	rep stosb
	mov	cx,w1len	;abut the two words
	mov	si,offset word1
	mov	di,offset joinwd
	mov	joinln,cx	;initialize joined word length
	rep movsb
	mov	cx,w2len
	add	joinln,cx
	mov	si,offset word2
	rep movsb
	mov	cx,joinln
	mov	si,offset joinwd
	xor	ah,ah
	xor	bx,bx
gt2lp2:
	mov	di,si
	lodsb			;get the letter
	cmp	joinpta[bx],0	;if we did this letter
	jne	gt2n54		;..skip it
	inc	ah
	push	cx
gt2n42:
	jcxz	gt2n50
	repne scasb
	jne	gt2n50
	push	di
	sub	di,offset joinwd
	dec	di
	mov	joinpta[di],ah
	pop	di
	jmp	gt2n42
gt2n50:
	pop	cx
gt2n54:
	inc	bx
	loop	gt2lp2
	cmp	shrcnt,0	;if common letters
	je	gt2n56
	jmp	gt2n60		;..try to reduce both lists
;simple get of 2nd list, neither list has common letters, so neither can be
;reduced.
gt2n56:
	call	freeadj 	;align memory request to para boundary
	les	di,dword ptr [bp].word2b
	mov	cx,es:[di].motlng	;dictionary length to read
	test	es:[di].motflg,01h	;if list in dictionary
	jz	rdlst20 		;get it and clean up
	xor	ax,ax		;else get list from its own file
	mov	al,es:[di].motrln	;length of word
	mul	es:[di].motpcnt 	;..times number of words
	mov	cx,ax			;..bytes to reserve
	call	getmem
	jnc	rdlst10
rdlst08:
	mov	si,offset msg3		;tell user no memory
	jmp	gt2ner			;..and leave
;read list from its own file
rdlst10:
	mov	[bp].secb,di		;anchor the list
	mov	[bp].seca,es
	lea	bx,[bp].secb
	call	readlist
	jc	rdlst12 		;read failed
	lea	bx,[bp].secb		;using 2nd list
	call	agree			;purge contradictory words
	jmp	rdlst30 		;read succeeded, good exit
rdlst12:
	les	di,dword ptr free2b	;bad read, return storage
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr save2b
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	mov	si,offset msg12 	;flag file error and leave
	jmp	gt2ner
;read list from dictionary, then clean it up
rdlst20:
	call	getmem
	jc	rdlst08 	;no storage
	mov	[bp].secb,di	;anchor 2nd list
	mov	[bp].seca,es
	lea	bx,[bp].secb
	call	getlist
	jnc	rdlst30
	jmp	gt2nomem	;broken dictionary
rdlst30:	;update display of word count
	les	di,dword ptr [bp].word2b
	mov	ax,es:[di].motpcnt
	call	newnum
	cmp	es:[di].motpcnt,0 ;nothing may have survived filtering
	jne	rdlst40
	mov	si,offset msg11 ;tell user list is empty
	jmp	gt2nomem	;free memory and leave
rdlst40:
	jmp	reduc90 	;got list ok, tell user and leave


;if both patterns are identical
;	and shared letters are in same position
;	and neither list has been reduced
; then reduction is not possible
gt2n60:
	push	ds		;save addressability
	lds	si,dword ptr [bp].word1b
	les	di,dword ptr [bp].word2b
	lea	si,[si].motsarg ;->pattern 1
	lea	di,[di].motsarg ;->pattern 2
	mov	cx,8
	repe cmpsb
	pop	ds		;restore addressability
	jne	gt2n70		;not the same pattern
;uh-oh, both lists same pattern.  Are the identical letters in identical
;positions?
	mov	cx,shrcnt
	mov	si,offset shroff
gt2lp3:
	lodsw			;if at least one common letter not in
	cmp	ah,al		;..same position
	jne	gt2n70		;..further reduction is possible
	loop	gt2lp3
;uh-oh, the shared letters are in identical positions for the same pattern.
;unless one or the other list has been reduced, reduction isn't possible.
;what we are talking about here is two words such as
;	    A K S  and	A K Q
;which are two words in the same pattern list.	Since the A and K
;occur in the same positions in both words, any word that exists in the
;first list must also exist in the second list, unless one or the other
;list has somehow been reduced, because the two lists are identical until
;either has been reduced.  On the other hand, two words such as
;	   H O B N and O B H S
;are reducible, even though both are the same pattern list, and identical
;letters occur, but not in identical positions.  AKS and AKQ turned out to be
;BOY - BOX.  Naturally, there are many pairs that would also fit, AND - ANY
;for example.  H O B N and O B H S turned out to be LUGS - UGLY.  There are
;valid words of 1 2 3 x that are not valid words re-arranged 3 1 2 x.  For
;example, there is no word corresponding to WHAT rearranged AWHx.  So, both
;lists are reducible because WHAT may be removed from both.
	les	di,dword ptr [bp].word1b
	test	es:[di].motflg,02h	;if this list was reduced
	jnz	gt2n70			;..further reduction is possible
	les	di,dword ptr [bp].word2b
	test	es:[di].motflg,02h	;or if this list was reduced
	jnz	gt2n70			;..further reduction is possible
	test	[bp].modeb,01h		;or if contrast rule is on
	jz	gt2n70			;..assume something was deleted
	jmp	gt2n56	;no reduction, just a simple get list
	page
;reducible lists

;to avoid an m x n search, we sort both lists on common letters, then prune
;both lists on common letters.	To use our above example of HOBN, OBHS we
;sort HOBN on columns 1 2 3 in that order; and O B H S on columns 3 1 2 in
;that order.  Then we compare 1:3 2:1 3:2.  While list1 words < list2 word
;we can prune the list1 words.	Conversely, while list1 word > list2 words
;we can prune the list2 words.	While a list1 word =  list2 words we can
;prune the list1 word if for all list2 words equal to it, their combined
;pattern doesn't match.  Similarly, a list2 word can be pruned if all list1
;words fail the combined pattern match.

;sort the primary list.  we use a pocket sort as optimal for this application.
gt2n70:
	les	di,dword ptr [bp].word1b	;sorting not needed
	cmp	es:[di].motpcnt,1 ;if first list has only one word
	ja	gt2n72
	jmp	gt2n100 	;skip entire sort of first list
gt2n72:
;slide 1st list up in memory to make hole in low memory for receiving the
;1st list in sorted order
	les	di,dword ptr [bp].freehb	;save current mem
	mov	free2b,di			;..to restore later
	mov	free2a,es
	les	di,dword ptr [bp].sizeb
	mov	save2b,di
	mov	save2a,es
	call	freeadj 	;align freespace at segment boundary
	les	di,dword ptr [bp].word1b
	mov	ax,es:[di].motpcnt	;count of actual words
	mov	[bp].psortn,ax	;number of items to sort
	mul	w1len		;times length of each word
	mov	cx,ax		;..bytes to get
	call	getmem
	jnc	gt2n80		;got the memory
	mov	si,offset msg9	;not enough memory
	jmp	gt2ner		;..msg, and leave
gt2n80:
	mov	[bp].psrtentb,di	;destination of scoot
	mov	[bp].psrtenta,es	;and ptr to list to sort
	push	ds
	lds	si,dword ptr [bp].primb ;source of scoot
	rep movsb
	pop	ds
;set up columns to sort on
	push	ds
	pop	es
	mov	si,offset shrcnt	;->shared letters count and offsets
	mov	di,offset colordr	;->column order parameter
	mov	[bp].prtcolb,di 	;tell pocket sort
	mov	[bp].prtcola,es
	lodsw				;number of columns
	stosw				;..to column order parameter
	mov	cx,ax
gt2lp4:
	lodsw
	stosb
	loop	gt2lp4
	mov	ax,w1len		;length of a word in this list
	mov	[bp].prtlen,ax
	mov	[bp].pocket,26		;radix 26
	call	psort			;sort the list
;return memory
	les	di,dword ptr free2b
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr save2b
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
;now test if sort went ok
	jnc	gt2n90
	mov	si,offset msg9	;sort memory failure
	jmp	gt2ner		;..leave
;return items in sorted order to bottom memory
gt2n90:
	les	di,dword ptr [bp].primb ;destination of words
	push	ds
	lds	si,dword ptr [bp].prtcolb	;index of words
	mov	cx,[bp].psortn		;number of words
	mov	[bp].pocket,16	;need this for the divide
gt2lp5:
	lodsw		;get word's index
	mul	[bp].prtlen	;bytes to word[i]
	div	[bp].pocket	;..into paragraphs and offset
	add	ax,[bp].psrtenta
	add	dx,[bp].psrtentb	;ax:dx -> source word
	push	ds
	push	si
	push	cx
	mov	ds,ax
	mov	si,dx		;ds:si->source word
	mov	cx,[bp].prtlen	;length of word
	rep movsb
	pop	cx
	pop	si
	pop	ds
	mov	si,[si] 	;next index
	loop	gt2lp5
	pop	ds
;only now do we get the second list
gt2n100:
;reserve a hole to receive the sorted list
	les	di,dword ptr [bp].freehb
	mov	free2b,di	;save current memory in case of
	mov	free2a,es	;..failure
	les	di,dword ptr [bp].sizeb
	mov	save2b,di
	mov	save2a,es
	call	freeadj 	;..align free memory to segment
	les	di,dword ptr itemb	;->word index
	xor	ax,ax
	mov	al,es:[di].motrln	;length of each word
	mul	es:[di].motpcnt 	;..times number of words
	mov	maxlen,ax	;size of reserved hole
	mov	cx,ax
	call	getmem		;get the hole to receive sorted list
	jnc	gt2n110
	mov	si,offset msg9	;not enough memory for a second list
	jmp	gt2ner		;..so quit
gt2n110:
	mov	destb,di	;save final destination ptr
	mov	desta,es
;get space to read list into
	call	freeadj 	;align memory to segment
	les	di,dword ptr itemb	;get length of list
	mov	ax,es:[di].motlng	;assume list is in dictionary
	test	es:[di].motflg,01h	;..if so
	jz	gt2n120 		;..got the right space
	mov	ax,maxlen		;else use computed length
gt2n120:
	mov	cx,ax
	call	getmem		;memory to read list from dictionary
	jnc	gt2n130
gt2rder:
	mov	si,offset msg9		;say no memory for 2nd list
gt2nomem:
	les	di,dword ptr free2b	;no memory, restore
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr save2b
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	jmp	gt2ner			;and leave
gt2n130:
	mov	[bp].secb,di	;beginning of second list
	mov	[bp].seca,es	;..will be changed later
	lea	bx,[bp].secb
;read from dictionary or from word file
	les	di,dword ptr itemb
	test	es:[di].motflg,01h	;if list in dictionary
	jz	gt2n138 		;..get from dictionary
	call	readlist		;else get from word file
	jc	gt2n136
	lea	bx,[bp].secb	;using 2nd list
	call	agree		;purge list of contradictory words
	jmp short gt2n150	;go on to pocket sort
gt2n136:
	mov	si,offset msg12 ;else tell user file error
	jmp	gt2nomem	;free storage and leave
gt2n138:
	call	getlist 	;get list from dictionary
	jnc	gt2n140
	les	di,dword ptr free2b	;return memory
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr save2b
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	stc
	jmp	gt2nxt
gt2n140:
	cmp	[bp].tempcnt,0		;if nothing survived cleanup
	jne	gt2n150
	xor	ax,ax			;..we have an empty list
	call	newnum			;..show the count of 0
	mov	si,offset msg11
	jmp	gt2nomem		;..tell user and leave
;if only one item in list, no need to sort
gt2n150:
	les	di,dword ptr itemb
	mov	ax,es:[di].motpcnt	;count of words
	cmp	ax,1			;if present count of words > 1
	ja	gt2n160 		;..do the sort
	xor	cx,cx			;else move word to its destination
	mov	cl,es:[di].motrln	;length of word is length of list
	les	di,dword ptr destb	;reserved hole is destination
	push	ds
	lds	si,dword ptr [bp].secb	;source
	mov	[bp].secb,di		;update final destination
	mov	[bp].seca,es
	rep movsb			;move word
	pop	ds			;restore addressability
	les	di,dword ptr free2b	;free 2nd list memory
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr save2b
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	jmp	reduce			;reduce the two lists
gt2n160:
;set up the pocket sort of second list on the common letters
	mov	[bp].psortn,ax	;number of items to sort
	les	di,dword ptr [bp].secb	;where items are
	mov	[bp].psrtentb,di
	mov	[bp].psrtenta,es
	push	ds			;set up columns and order
	pop	es			;..to sort on
	mov	si,offset shrcnt
	mov	di,offset colordr
	mov	[bp].prtcolb,di ;tell psort where the columns are
	mov	[bp].prtcola,es
	lodsw				;number of columns
	stosw
	mov	cx,ax
gt2nlp6:
	lodsw
	xchg	ah,al		;columns and order
	stosb
	loop	gt2nlp6
	mov	ax,w2len	;length of a word in this list
	mov	[bp].prtlen,ax	;tell psort
	mov	[bp].pocket,26	;tell psort the radix
	call	psort
;return all 2nd list memory
	les	di,dword ptr free2b
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr save2b
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	jnc	gt2n220 	;if sort failed
	mov	si,offset msg9	;..tell user
	jmp	gt2nomem	;..and leave
gt2n220:
	les	di,dword ptr destb	;destination of words
	mov	[bp].secb,di	;correct list ptr
	mov	[bp].seca,es
	push	ds
	lds	si,dword ptr [bp].prtcolb	;index of words
	mov	cx,[bp].psortn		;number of words
	mov	[bp].pocket,16	;need this for the divide
gt2lp7:
	lodsw		;get word's index
	mul	[bp].prtlen	;bytes to word[i]
	div	[bp].pocket	;..into paragraphs and offset
	add	ax,[bp].psrtenta
	add	dx,[bp].psrtentb	;ax:dx -> source word
	push	ds
	push	si
	push	cx
	mov	ds,ax
	mov	si,dx		;ds:si->source word
	mov	cx,[bp].prtlen	;length of word
	rep movsb
	pop	cx
	pop	si
	pop	ds
	mov	si,[si] 	;next index
	loop	gt2lp7
	pop	ds
	page
;begin the list reduction proper
reduce:
	mov	flag1,0 	;clear reduction flags
	mov	i,0		;index into 1st list
	mov	j,0		;index into 2nd list
	les	di,dword ptr [bp].word1b
	mov	ax,es:[di].motpcnt
	mov	list1n,ax	;items in 1st list
	les	di,dword ptr [bp].word2b
	mov	ax,es:[di].motpcnt
	mov	list2n,ax	;items in 2nd list
reduclp:
	mov	ax,i		;if done with 1st list
	cmp	ax,list1n
	jne	reduc10
	mov	ax,j		;..and done with 2nd list
	cmp	ax,list2n
	jne	reduc20
	jmp	reducxt 	;..list reduction done
reduc10:			;1st list not done
	mov	ax,j		;if 2nd list not done
	cmp	ax,list2n
	jne	reduc30
;run out 1st list
	mov	ax,i		;the ith item in list1
	mov	cx,list1n	;..of n items
	sub	cx,ax		;..how many left
	mul	w1len
	div	c16		;ax:dx	=  segment, offset
	lea	bx,[bp].primb	;which list
	mov	si,w1len
	or	flag1,01h	;flag reduction in 1st list
	call	runout
	jmp	reducxt
;run out 2nd list
reduc20:
	mov	ax,j
	mov	cx,list2n
	sub	cx,ax		;number of items left in 2nd list
	mul	w2len
	div	c16
	lea	bx,[bp].secb	;which list
	mov	si,w2len
	or	flag1,02h	;flag reduction in 2nd list
	call	runout
	jmp	reducxt
;find list1[i] = list2[j]
reduc30:
	call	lstcmp		;compare list1[i]:list2[j]
	jb	reduc40 	;list1 < list2
	je	reduc50
;list1 > list2, eliminate list2[j]
	or	flag1,02h	;flag reduction in list2
	les	di,dword ptr list2b
	inc	j
reduc32:
	mov byte ptr es:[di],0FFh
	jmp	reduclp
;list1 < list2, eliminate list1[i]
reduc40:
	or	flag1,01h	;flag reduction in list1
	les	di,dword ptr list1b
	inc	i
	jmp	reduc32
;list1[i] = list2[j]
;m x n reduction
;if joint pattern of list1[i],list2[j,j+1,..etc] doesn't match jointpat
;then list1[i] can be eliminated.
;similarly if joint pattern of list1[i,i+1,..etc],list2[j] doesn't match
;jointpat, then list2[j] can be eliminated
reduc50:
	mov	ax,i
	mov	m,ax	;save index i
	mov	ax,j
	mov	n,ax	;save index j
redulp2:
	inc	i
	mov	ax,i
	cmp	ax,list1n	;count eligible list1 words
	je	reduc60 	;out of eligible words
	call	lstcmp		;is next i eligible
	je	redulp2 	;..yes
reduc60:
	mov	cx,i
	mov	ax,m
	sub	cx,ax		;number of list1 words with same letters
	mov	i,ax		;restore i
	mov	sub1,cx 	;save subcount1
redulp3:
	inc	j
	mov	ax,j
	cmp	ax,list2n	;count eligible list2 words
	je	reduc70 	;out of eligible words
	call	lstcmp		;is next j eligible?
	je	redulp3 	;..yes
reduc70:
	mov	cx,j
	mov	ax,n
	sub	cx,ax		;number of eligible list2 words
	mov	j,ax		;restore j
	mov	sub2,cx
	mov	cx,sub1 	;for all i words
reduci:
	mov	ax,i		;set i word address
	mul	w1len
	div	c16
	add	ax,[bp].prima
	add	dx,[bp].primb
	mov	list1b,dx
	mov	list1a,ax
	call	checkj
	inc	i
	loop	reduci
	mov	ax,m		;reset i
	mov	i,ax
	mov	cx,sub2
reducj:
	mov	ax,j		;set j word address
	mul	w2len
	div	c16
	add	ax,[bp].seca
	add	dx,[bp].secb
	mov	list2b,dx
	mov	list2a,ax
	call	checki
	inc	j
	loop	reducj
	mov	ax,m		;bump i past all list1 similar words
	add	ax,sub1
	mov	i,ax
	mov	ax,n		;bump j past all list2 similar words
	add	ax,sub2
	mov	j,ax
	jmp	reduclp 	;continue reduction till lists exhausted
reducxt:
	test	flag1,01h	;if list1 reduced
	jz	reduc78
	lea	bx,[bp].primb	;..clean it up
	call	clean2
reduc78:
;try to update 1st list (may not be on screen)
	mov	ax,olditem
	cmp	ax,[bp].motcur	;top of screen cipher word
	jb	reduc80 	;current 1st cipher word off top of screen
	mov	dx,[bp].motcur
	add	dx,7
	cmp	ax,dx
	jnb	reduc80 	;current 1st cipher word off bottom of screen
;1st cipher word is on the screen
	sub	ax,[bp].motcur	;index of position on screen
	add	al,17		;expected 1st row offset
	mov	dx,wrdpos
	mov	calcpos,dx
;trick newnum by diddling wrdpos
	mov byte ptr wrdpos+1,al
	les	di,dword ptr [bp].word1b
	mov	ax,es:[di].motpcnt
	call	newnum
	mov	ax,calcpos
	mov	wrdpos,ax
reduc80:
	test	flag1,02h	;if list2 reduced
	jz	reduc88
	lea	bx,[bp].secb	;..clean it up
	call	clean2
reduc88:
;update display of 2nd list number of words
	les	di,dword ptr [bp].word2b
	mov	ax,es:[di].motpcnt
	call	newnum
reduc90:
	mov	bx,offset free2b	;adjust final memory to
	lea	si,[bp].secb		;size actually needed
	call	adjfree
	or	[bp].flaga,80h	;flag we have a 2nd list
;note: above OR clears carry flag to indicate good return
gt2nxt:
	pop	ds
	ret
get2nd	endp

;compute address of current word index, returned in itema:itemb
getptr	proc	near
	xor	bx,bx
	mov	bl,byte ptr wrdpos+1	;cursor position in list
	sub	bx,17			;..adjust to origin 0
	add	bx,[bp].motcur		;..+ current top of list
	shl	bx,1			;..index into indexes
	les	di,dword ptr [bp].motib ;->ptr to indexes
	mov	ax,es:[di][bx]		;ax = index of ith item
	mul	motlang 		;..times entry length
	div	c16			;..adjust to paras, offset
	add	ax,[bp].mota
	add	dx,[bp].motb		;ax:dx->current entry
	mov	itemb,dx		;return ptr
	mov	itema,ax
	ret
getptr	endp

;read list from dictionary, and clean it up
;bx -> which list, stack anchors set
;returns new count of words in ax
getlist proc	near
	mov	lstptr,bx	;save, will be needed several times
	les	di,dword ptr ss:[bx+4]	;word index ptr
	call	lseek
	jnc	gtlst10
gtlster:
	mov	si,offset msg4	;tell user dictionary is broken
	mov	di,24*160+80
	call	showmsg
	or	[bp].flaga,08h	;flag broken dictionary
	stc			;bad return
	ret
gtlst10:
	mov	cx,es:[di].motlng
	mov	si,lstptr
	push	ds
	lds	dx,dword ptr ss:[si]
	mov	ah,3Fh		 ;read the list
	int	21h
	pop	ds
	jc	gtlster 	;broken dictionary
	cmp	ax,cx
	jne	gtlster
	mov	bx,lstptr
	push	ds
	lds	si,dword ptr ss:[bx] ;point to the list
	call	cleanup 	;clean up the list
	pop	ds
	mov	bx,lstptr	;adjust word count
	les	di,dword ptr ss:[bx+4]
	mov	ax,[bp].tempcnt ;new count of words
	mov	es:[di].motpcnt,ax
	clc			;good return
	ret
getlist endp

;es:di-> word index.  Position dictionary file at word list
lseek	proc	near
	mov	cx,es:[di].motlocb	;hi order file position
	mov	dx,es:[di].motloca	;lo order file position
	mov	bx,[bp].hdct		;dictionary file handle
	mov	ax,4200h
	int	21h		;set file position to prepare for read
	ret
lseek	endp

;clean up and filter a list from the dictionary
;es:di->word index
;ds:si->list buffer
cleanup proc	near
	mov	cx,es:[di].motlng	;for n bytes
	xor	dx,dx
	mov	dl,es:[di].motrln	;..of length l words
	push	ds
	pop	es
	mov	di,si		;es:di->destination
	mov	[bp].tempcnt,0	;zero count of filtered words
cleanlp:
	jcxz	clnxit
	cmp byte ptr [si],'a'   ;if white space
	jb	cln30		;..ignore byte
;found start of word
	mov	bx,di		;save start of word
	push	cx
	mov	cx,dx		;length of word
	rep movsb
	pop	cx
	test	[bp].modeb,01h	;if contrast rule on
	jnz	cln10
	call	contrast	;..test for contrast
	jc	cln40		;..ignore this word, doesn't contrast
cln10:
	test	[bp].modeb,02h	;if agreement rule on, check accord
	jnz	cln18		;...(rule off)
	call far ptr accord	;..with previously assigned letters
	jc	cln40		;..this word doesn't agree.
cln18:
	inc	[bp].tempcnt	;count word
cln20:
	sub	cx,dx		;deduct bytes moved
	jmp	cleanlp
cln30:
	inc	si
	loop	cleanlp
clnxit:
	ret
cln40:
	mov	di,bx		;keep destination in place
	jmp	cln20
cleanup endp

;contrast filter.  Every letter of plaintext must disagree with corresponding
;letter of cipher word.
;es:bx->plaintext word
contrast proc	near
	push	ds
	push	si
	push	ax
	push	bx
	push	di
	mov	ax,crypt9b	;addressability again
	mov	ds,ax
	lds	si,dword ptr itemb	;recover word index
	lea	si,[si].motalf
	mov	di,bx
	push	cx
	mov	cx,dx		;length of word
	xor	ax,ax
contralp:
	lodsb		;get alphabetic offset of cipher letter
	call	getplain	;get corresponding plaintext letter
	cmp	al,es:[di]
	je	contra10	;does not contrast
	inc	di	;next plaintext
	loop	contralp
	clc
contraxt:
	pop	cx
	pop	di
	pop	bx
	pop	ax
	pop	si
	pop	ds
	ret
contra10:
	stc
	jmp	contraxt
contrast endp

getplain proc	near
	push	es
	push	di
	les	di,dword ptr [bp].alfb	;address the alphabets
	add	di,258		;address the plaintext alphabet
	add	di,ax		;plus offset
	mov	al,es:[di]	;get plaintext letter
	pop	di
	pop	es
	ret
getplain endp

;simple, unindexed list move, and storage return
movlist proc	near
	les	di,dword ptr [bp].word1b	;get final list length
	xor	ax,ax
	mov	al,es:[di].motrln	;length of word
	mul	es:[di].motpcnt 	;..times number of words
	mov	cx,ax			;bytes to move
	les	di,dword ptr destb	;final list destination
	push	ds
	lds	si,dword ptr [bp].primb ;source of list
	rep movsb	;move entire list to final resting place
	pop	ds
	mov	bx,offset freeb
	lea	si,[bp].primb
	call	adjfree 	;fix up storage
	les	di,dword ptr destb
	mov	[bp].primb,di	;finally anchor the list
	mov	[bp].prima,es
	or	[bp].flaga,20h	;flag primary list is up
	ret
movlist endp

;ax = count of words
;update count of words displayed in word index
newnum	proc	near
	push	es
	push	di
	mov	si,offset msg6	;convert count to ascii
	call	int2asc
	xor	ax,ax
	mov	al,byte ptr wrdpos+1	;figure screen location
	mul	c160
	add	ax,40
	mov	es,[bp].videoa	;screen destination
	mov	di,ax
	mov	cx,5
	mov	ah,07		;normal attribute
get1lp:
	lodsb
	stosw
	loop	get1lp
	pop	di
	pop	es
	ret
newnum	endp

;tomem, get cx bytes.  if error clean up
;bx -> free space save area
tomem	proc	near
	push	cx	;save requested length
	call	freeadj ;adjust memory to paragraph boundary
	pop	cx
	call	getmem
	jnc	tomemxt
	les	di,dword ptr [bx]
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr [bx+4]
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
tomemxt:
	ret
tomem	endp
;adjust freespace to final list size
;bx -> saved freespace ptrs and size
;si -> list ptr
adjfree proc	near
	les	di,dword ptr [bx]	;return space ptr
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr [bx+4]	;return space bytes
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	call	freeadj 	;force alignment to paragraph
	les	di,dword ptr ss:[si+4]	;word index
	xor	ax,ax
	mov	al,es:[di].motrln
	mul	es:[di].motpcnt
	mov	cx,ax
	call	getmem		;failure not possible
	ret
adjfree endp

;es:bx->plaintext word
;quoda:quodb->current word index
;check plaintext word for agreement with previous plaintext letter
;assignments.
accord	proc	far
	push	ds
	push	di
	push	bx
	push	cx
	push	si
	lds	si,dword ptr [bp].quodb      ;ds:si->word index
	mov	di,bx		;es:di->plaintext word
	xor	cx,cx
	mov	cl,[si].motrln	;length of plaintext word
	lea	si,[si].motalf	;ds:si->cipher letter alphabetic offsets
acrdlp1:
	call	checkc		;get solved letter for this cipher letter
	cmp	al,' '          ;if blank, not solved yet
	je	acrd08		;..ignore it
	cmp	al,es:[di]	;else it must agree with plaintext letter
	jne	acrdno		;..doesn't, no accord
acrd08:
	call	checks		;get solved letter offset
	jne	acrd10		;..letter not solved yet
	cmp	al,[si] 	;if not same as cipher letter offset
	jne	acrdno		;..plaintext word doesn't agree
acrd10:
	inc	si		;next alphabetic offset
	inc	di		;next plaintext letter
	loop	acrdlp1 	;do for all letters
	clc			;plaintext word agrees
acrdxt:
	pop	si
	pop	cx
	pop	bx
	pop	di
	pop	ds
	ret
acrdno:
	stc			;no agreement
	jmp	acrdxt
accord	endp

;ds:si->cipher letter offsets
;get solved letter in al for cipher text letter
checkc	proc	near
	push	es
	push	di
	xor	ax,ax
	mov	al,[si] 	;cipher letter offset
	les	di,dword ptr [bp].alfb
	add	di,514		;es:di->solved letters
	add	di,ax
	mov	al,es:[di]
	pop	di
	pop	es
	ret
checkc	endp
;es:di->plaintext letter, if solved, return its offset in al
checks	proc	near
	push	es
	push	di
	push	cx
	mov	al,es:[di]	;get plaintext letter
	les	di,dword ptr [bp].alfb	;address the alphabets
	mov	cx,es:[di]	;alphabet length
	add	di,514		;->solved letters
	mov	bx,di
	repne scasb		;scan for plaintext letter
	jne	chksxt		;..not solved yet
	dec	di
	sub	di,bx		;offset to solved letter
	mov	ax,di
	cmp	ax,ax		;force condition code to equal
chksxt:
	pop	cx
	pop	di
	pop	es
	ret
checks	endp

runout	proc	near
	add	ax,ss:[bx+2]	;segment of list
	add	dx,ss:[bx]	;offset of list
	mov	es,ax
	mov	di,dx
	mov	al,0FFh
runoutlp:
	mov	es:[di],al
runout10:
	add	di,si		;next item
	jc	runout20	;correct wrap
	loop	runoutlp
	ret
runout20:
	sub	di,si
	mov	ax,es		;fix up address for next segment
	mov	dx,di
	div	c16
	mov	es,ax
	mov	di,dx
	mov	al,0FFh
	jmp	runout10
runout	endp

lstcmp	proc	near
	mov	ax,i
	mul	w1len
	div	c16
	add	ax,[bp].prima
	add	dx,[bp].primb
	mov	list1b,dx	;address of list1[i]
	mov	list1a,ax
	mov	ax,j
	mul	w2len
	div	c16
	add	ax,[bp].seca
	add	dx,[bp].secb
	mov	list2b,dx	;address of list2[i]
	mov	list2a,ax
	mov	cx,shrcnt
	mov	si,offset shroff
	xor	bx,bx
lstcmplp:
	mov	bl,[si] 	;offset of common letter in list1
	les	di,dword ptr list1b
	mov	al,es:[di][bx]	;get list1 common letter
	mov	bl,[si+1]	;offset of common letter in list2
	les	di,dword ptr list2b
	cmp	al,es:[di][bx]
	jne	lstcmpxt	;return at once if not equal
	add	si,2		;next pair of common letters
	loop	lstcmplp
	cmp	cx,cx		;force equal flags
lstcmpxt:
	ret
lstcmp	endp

checkj	proc	near	;check i against all js
	push	cx
	mov	cx,sub2
chkjlp:
	mov	ax,j	;compute address of j word
	mul	w2len
	div	c16
	add	ax,[bp].seca
	add	dx,[bp].secb
	mov	list2b,dx
	mov	list2a,ax
	call	cmpat	;any matching pattern means
	je	chkjxt	;..accept i
	inc	j
	loop	chkjlp
	les	di,dword ptr list1b	;no matching pattern means
	mov byte ptr es:[di],0FFh	;..eliminate list1 word
	or	flag1,01h
chkjxt:
	mov	ax,n	;restore j
	mov	j,ax
	pop	cx
	ret
checkj	endp

checki	proc	near	;check j against all is
	push	cx
	mov	cx,sub1
chkilp:
	mov	ax,i
	mul	w1len
	div	c16	;address of i word
	add	ax,[bp].prima
	add	dx,[bp].primb
	mov	list1b,dx
	mov	list1a,ax
	les	di,dword ptr list1b	;if i word was eliminated
	cmp byte ptr es:[di],0FFh
	je	chki10			;..get next i word
	call	cmpat
	je	chkixt
chki10:
	inc	i
	loop	chkilp
	les	di,dword ptr list2b	;eliminate this j word
	mov byte ptr es:[di],0FFh
	or	flag1,02h
chkixt:
	mov	ax,m
	mov	i,ax		;restore i
	pop	cx
	ret
checki	endp

cmpat	proc	near
	push	cx
	push	ds
	pop	es
	assume	ds:nothing,es:crypt9b
	mov	di,offset joinwd	;get list1 word
	mov	cx,w1len
	lds	si,dword ptr list1b
	rep movsb
	mov	cx,w2len		;get list2 word abutted to 1st word
	lds	si,dword ptr list2b
	rep movsb
	push	es
	pop	ds
	assume	ds:crypt9b,es:nothing
	mov	di,offset joinptb
	xor	ax,ax		;clear the joint pattern area
	mov	cx,16
	rep stosw
	mov	cx,joinln	;length of joined words
	mov	si,offset joinwd	;start of joined words
	xor	ah,ah
	xor	bx,bx
cmpatlp:
	mov	di,si
	lodsb			;get joined words letter
	cmp	joinptb[bx],0	;if we did this letter
	jne	cmpat20
	inc	ah		;pattern number
	push	cx
cmpatlp2:
	jcxz	cmpat10
	repne scasb		;look for identical letters
	jne	cmpat10
	push	di
	sub	di,offset joinwd
	dec	di
	mov	joinptb[di],ah
	pop	di
	jmp	cmpatlp2	;look for more identical letters
cmpat10:
	pop	cx
cmpat20:
	inc	bx
	loop	cmpatlp
;compare the cipher words pattern to the plaintext words pattern
	mov	cx,joinln
	mov	si,offset joinpta
	mov	di,offset joinptb
	repe cmpsb
	pop	cx
	ret
cmpat	endp

clean2	proc	near
	les	di,dword ptr ss:[bx+4] ;list to be cleaned
	or	es:[di].motflg,02h     ;flag it as reduced
	mov	[bp].tempcnt,0	;counter for remaining words
	mov	cx,es:[di].motpcnt	;previous total words
	mov	dl,es:[di].motrln	;length of words in this list
	xor	dh,dh
	push	ds
	lds	si,dword ptr ss:[bx]	;->word list
	les	di,dword ptr ss:[bx]
cln2lp:
	cmp byte ptr [si],0FFh	;if word not deleted
	jne	cln210		;..move and count it
	add	si,dx		;else skip this word
	jmp short cln220
cln210:
	inc	[bp].tempcnt
	push	cx
	mov	cx,dx
	rep movsb
	pop	cx
cln220:
	loop	cln2lp
	pop	ds
	les	di,dword ptr ss:[bx+4]	;correct list word count
	mov	ax,[bp].tempcnt
	mov	es:[di].motpcnt,ax
	ret
clean2	endp

;if agreement required, then for all words in this list,
;	check that every letter in this word agrees with previous
;	plaintext assignments.
;bx->which word list
agree	proc	near
	test	[bp].modeb,02h	;if agreement required
	jz	agree00 	;..then for all words in list
	ret
agree00:			;..check each for agreement
	push	ds
	les	di,dword ptr ss:[bx+4]
	mov	cx,es:[di].motpcnt ;all words in this list
	jcxz	agreext 	   ;..empty list
	xor	dx,dx
	mov	dl,es:[di].motrln  ;length of each word
	mov	[bp].tempcnt,0	;agreeable word count
	les	di,dword ptr ss:[bx]	;list destination
	push	bx
	lds	si,dword ptr ss:[bx]	;list source
agreelp:
	mov	bx,si		;start of word
	call far ptr accord	;test for agreement
	jc	agreeno
	inc	[bp].tempcnt	;word agrees, count it
	push	cx		;..and move it
	mov	cx,dx		;length of word
	rep movsb
	pop	cx
agree10:
	loop	agreelp
	pop	bx
	les	di,dword ptr ss:[bx+4]	;update present word count of list
	mov	ax,[bp].tempcnt
	mov	es:[di].motpcnt,ax
agreext:
	pop	ds
	ret
agreeno:
	add	si,dx		;next source word
	jmp	agree10
agree	endp

;compare fp qword number ds:si to es:di
	assume	ds:nothing
compare proc	far
	std			;compare backwards, since numbers are
	push	cx		;in Intel byte order
	mov	cx,8
	add	si,7
	add	di,7
	repe cmpsb		;set the condition code
	pop	cx		;restore cx
	cld			;restore direction
	ret
compare endp

crypt9	ends
	end
