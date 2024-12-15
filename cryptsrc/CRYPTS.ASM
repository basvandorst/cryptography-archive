	page	53,132
	include frame.str
	include cryptsb.asm
	extrn	getmem:far,freeadj:far,savefree:far
crypts	segment 'crya'
	assume	cs:crypts,ds:cryptsb
	public	qsort,psort,isort
;all parameters are passed in the stack.  On entry, these are:
;[bp].srtcmpa,srtcmpb -> caller's far comparison routine
;[bp].srtenta,srtentb -> 1st element of items to be sorted
;[bp].srtlen  =  length in bytes of an entry
;[bp].srtn    =  number of items to sort
;[bp].srtdir	 0 = sort ascending, 1 = sort descending

;on return, [bp].srtcmpa,srtcmpb -> indexes in sorted order of elements
;	    and carry=no.  The original elements are undisturbed.
;or carry=yes, meaning not enough memory to sort
qsort	proc	far
	push	ds
	push	es
	push	si
	push	di
	push	cx
	push	ax
	mov	ax,cryptsb	;data addressability
	mov	ds,ax
	call	freeadj 	;adjust freespace ptr to segment boundary
	mov	cx,[bp].srtn	;get number of items to sort
	shl	cx,1		;double for index space
	jnc	qort00
	jmp	qortxt		;too many items, > 32767
qort00:
	call	getmem		;get index space
	jnc	qort10
	jmp	qortxt		;not enough memory
qort10:
	mov	indexb,di	;save index ptr for later
	mov	indexa,es
	mov	aptr,di 	;top index ptr
;initialize indexes
	mov	cx,[bp].srtn	;number of items
	xor	ax,ax		;index number
qinitlp:
	stosw
	inc	ax
	loop	qinitlp
	sub	di,2		;repoint to last index
	mov	zptr,di 	;bottom index ptr
	mov	dir,offset up	;assume up (ascending) sort
	test	[bp].srtdir,01h
	jz	qort20
	mov	dir,offset down ;correct the assumption
qort20:
	mov	ax,zptr 	;find the median element
	mov	dx,aptr 	;..in this partition
	jmp short part		;first time entry
qsrtlp:
	cmp	haufen,0	;if no partitions remain
	jne	qort30
	les	di,dword ptr indexb
	mov	[bp].srtcmpb,di ;..return index ptr
	mov	[bp].srtcmpa,es
	clc			;..good return
	jmp	qortxt		;..and we are done
qort30:
	pop	ax		;recover partition end
	pop	dx		;recover partition start
	dec	haufen		;count the pop
partret:
	mov	zptr,ax 	;set current partition end
	mov	aptr,dx 	;..and start.
part:			;partition the current partition
	sub	ax,dx		;check how many elements
	shr	ax,1
	jz	qsrtlp	;1 element, this partition is done
	cmp	ax,1	;if two elements
	jne	qort40
	mov	bx,aptr ;handle special (no median)
	call	getadd		;get 1st item address
	mov	firstb,dx
	mov	firsta,ax
	mov	bx,zptr 	;get 2nd item address
	call	getadd
	mov	lastb,dx
	mov	lasta,ax
	mov	si,offset firstb
	mov	di,offset lastb
	call	compare 	;1st:last
	je	qsrtlp		;1st=last, doesn't matter
	jb	qort37
;1st>last
	cmp	[bp].srtdir,01h ;if descending sort
	je	qsrtlp		;..already in correct order
;The 1st shall be last, and the last shall be 1st, yea verily even in
;programming.
qort35:
	mov	bx,aptr
	mov	di,zptr
	mov	ax,es:[bx]
	xchg	ax,es:[di]
	mov	es:[bx],ax
	jmp	qsrtlp		;get another partition
;1st<last
qort37:
	cmp	[bp].srtdir,00h ;if ascending order
	je	qsrtlp		;..already in correct order
	jmp	qort35

;choose median	element as the pivot
qort40:
	mov	bx,aptr ;1st index
	call	getadd	;compute address of 1st element
	mov	firstb,dx
	mov	firsta,ax
	mov	bx,zptr ;last element
	call	getadd	;compute address of last element
	mov	lastb,dx
	mov	lasta,ax
	add	bx,aptr ;get address of mid element = (1st+last)/2
	shr	bx,1
	test	bl,01h	;if odd
	jz	qort42
	inc	bx	;..round up to even
qort42:
	mov	midptr,bx
	call	getadd
	mov	midb,dx
	mov	mida,ax
	mov	si,offset firstb
	mov	di,offset midb
	call	compare 	;first:middle
	jb	qort65		;first<middle
;mid <= first, compare first to last
	mov	si,offset firstb
	mov	di,offset lastb
	call	compare 	;first:last
	jb	qort60		;mid<first<last
;first is greatest, compare mid to last
	mov	si,offset midb
	mov	di,offset lastb
	call	compare 	;mid:last
	jb	qort50		;mid<last<first
;last<mid<first 	;mid is pivot
qort45:
	mov	bx,midptr
	jmp short qort70
qort50: 	;last is pivot
	mov	bx,zptr ;last is pivot
	jmp short qort70
qort60: 	;first is pivot
	mov	bx,aptr ;first is pivot
	jmp short qort70
qort65: 	;first < mid, compare mid to last
	mov	si,offset midb
	mov	di,offset lastb
	call	compare 	;mid:last
	jb	qort45		;mid<last, mid is pivot
;last<mid, compare 1st to last
	mov	si,offset firstb
	mov	di,offset lastb
	call	compare 	;1st:last
	jb	qort50		;last is pivot
	jmp	qort60		;1st is pivot
qort70: 	;pivot is chosen, bx-> pivot index
	mov	ax,es:[bx]	;save the pivot index
	mov	pivot,ax
	call	getadd		;get address of pivot element
	mov	pivb,dx 	;..and save for comparisons
	mov	piva,ax
	mov	di,aptr 	;get first index ptr
	mov	destptr,di	;..to initialize destination ptr
	mov	ax,es:[di]	;..get first index
	mov	es:[bx],ax	;..and move to former pivot index
;initialize partition ends pointers
	mov	bx,di		;bx is left (lo mem) pointer
	add	bx,2		;skip vacated destination slot
	mov	cx,zptr 	;cx is right (hi mem) pointer
;at this point, the partitioning proper begins.  We find the right slot for
;the pivot item, and move its index to the corresponding position in the
;index table.  This partitions the items into two parts - below, all items
;are less (more) than the pivot.  Above, all items are more (less) than the
;pivot.  Either partition may consist of 0, 1, or more items.

;throughout, bx is the left index ptr, cx the right index ptr
	mov	flag,01h ;start from the right end of current partition
partlp:
	cmp	bx,cx	;while left hasn't crossed right
	jna	qort80	;..continue partitioning
	jmp	qnext	;else set up next partitions
qort80:
	jmp	[dir]		;sort up or sort down

;ascending order sort
up:
	test	flag,01h ;if comparing from the right
	jnz	upright  ;..go there
;comparing from the left
	call	getadd	;get the left element's address
	mov	compb,dx
	mov	compa,ax
	mov	si,offset pivb	;pivot element:left element
	mov	di,offset compb
	call	compare 	;if pivot >= left
	jnb	qort90		;..try next left
qort85:
	mov	di,destptr	;else move left index to destination
	mov	ax,es:[bx]
	mov	es:[di],ax
	mov	destptr,bx	;next destination is current left
	xor	flag,01h	;toggle left/right flag
qort90:
	add	bx,2		;next left
	jmp	partlp		;continue partitioning
upright:
	xchg	bx,cx	;get the right element's address
	call	getadd
	mov	compb,dx
	mov	compa,ax
	mov	si,offset pivb	;compare pivot element to right element
	mov	di,offset compb
	call	compare 	;if pivot <= right
	jna	qort100 	;..try next right
qort95:
	mov	di,destptr	;else move right index to destination
	mov	ax,es:[bx]
	mov	es:[di],ax
	mov	destptr,bx	;next destination is current right
	xor	flag,01h	;toggle left/right flag
qort100:
	xchg	bx,cx
	sub	cx,2		;next right goes downward
	jmp	partlp

;descending order sort
down:
	test	flag,01h	;if comparing from the right
	jnz	dnright 	;..go there
;sorting descending, comparing from left
	call	getadd		;get left element's address
	mov	compb,dx
	mov	compa,ax
	mov	si,offset pivb	;compare pivot:left
	mov	di,offset compb
	call	compare 	;if pivot <= left
	jna	qort90		;..try next left
	jmp	qort85		;else move current left to destination
;sorting descending, comparing from right
dnright:
	xchg	bx,cx
	call	getadd		;get right element's address
	mov	compb,dx
	mov	compa,ax
	mov	si,offset pivb	;compare pivot:right
	mov	di,offset compb
	call	compare 	;if pivot < right
	jb	qort95		;..move current right to destination
	jmp	qort100 	;else try next right

;the current partition is partitioned
qnext:
	mov	bx,destptr	;the pivot index goes to current destination
	mov	ax,pivot
	mov	es:[bx],ax
;compute size of left partition
	mov	half1,0 	;assume no left partition
	cmp	bx,aptr
	je	qort110
	mov	dx,bx		;new zptr for left
	sub	dx,2		;correct assumption
	mov	ax,dx
	sub	ax,aptr
	shr	ax,1
	inc	ax
	mov	half1,ax
qort110:
	mov	half2,0 	;assume no right partition
	cmp	bx,zptr
	je	qort120
	add	bx,2		;new aptr for right
	mov	ax,zptr 	;correct assumption
	sub	ax,bx
	shr	ax,1
	inc	ax
	mov	half2,ax
;stack the larger partition
qort120:
	mov	ax,aptr 	;ax,dx are aptr zptr for left
	mov	cx,zptr 	;bx,cx are aptr zptr for right
	cmp	half1,0 	;if no left partition
	jne	qort130
	cmp	half2,0 	;..and no right partition
	jne	qort140
	jmp	qsrtlp		;pop a previous partition
qort130:		;there is a left partition
	cmp	half2,0
	jne	qort150
;set up left partition, then partition it
qort135:
	xchg	ax,dx
	jmp	partret
;set up right partition, then partition it
qort140:
	mov	ax,cx
	mov	dx,bx
	jmp	partret
;there is both a left and a right partition, stack the larger
qort150:
	mov	si,half1
	cmp	si,half2
	jb	qort160
;left is larger, stack it and partition right
	push	ax		;stack aptr for left
	push	dx		;..    zptr for left
	inc	haufen		;count stacked partition
	jmp	qort140 	;..and set up right for partitioning
qort160:
;right is larger, stack it, and partition left
	push	bx		;aptr for right
	push	cx		;zptr for right
	inc	haufen		;count stacked partition
	jmp	qort135 	;do the left half

;sort done exit
qortxt:
	pop	ax
	pop	cx
	pop	di
	pop	si
	pop	es
	pop	ds
	ret
qsort	endp

;pocket (radix partition) sort.  for sorting word lists.  this is supposed
;to be the fastest of all possible sorts, running in O(n) time.  See Knuth.
psort	proc	far
	push	ds
	mov	ax,cryptsb	;data addressability
	mov	ds,ax
;save memory in case of bailout
	les	di,dword ptr [bp].freehb
	mov	freeb,di
	mov	freea,es
	les	di,dword ptr [bp].sizeb
	mov	saveb,di
	mov	savea,es
;set up the pocket chain heads
	call	freeadj 	;align freespace to segment boundary
	mov	ax,[bp].pocket	;get the number of pockets needed
	shl	ax,1		;..times 6 bytes per pocket
	mov	cx,ax
	shl	ax,1
	add	cx,ax		;..is number of bytes to get for chain heads
	call	getmem
	jnc	port10		;got the memory
nomem:
	les	di,dword ptr freeb	;insufficient memory to sort
	mov	[bp].freehb,di		;..restore memory status quo ante
	mov	[bp].freeha,es
	les	di,dword ptr saveb
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	jmp	portxit 		;..and leave. sorting not possible
port10:
	mov	[bp].chainb,di		;save pocket chain head ptr
	mov	[bp].chaina,es
;set up the source indexes, 1st buffer
	call	freeadj 	;align freespace to segment boundary
	mov	cx,[bp].psortn	;number of items to sort
	shl	cx,1		;..times 4
	shl	cx,1
	call	getmem		;..is number of bytes to get
	jc	nomem
	mov	[bp].ndx1b,di	;start of 1st buffer to hold indexes into list
	mov	[bp].ndx1a,es
;initialize indexes into the word list
	xor	ax,ax
	mov	cx,[bp].psortn	;number of index entries
portlp1:
	stosw			;index i
	add	di,2		;next entry
	mov	es:[di-2],di	;offset to next i
	inc	ax		;next i
	loop	portlp1 	;do for all index entries
;set up destination indexes, 2nd buffer
	call	freeadj
	mov	cx,[bp].psortn	;number of items in word list
	shl	cx,1
	shl	cx,1
	call	getmem
	jnc	port20
	jmp	nomem
port20:
	mov	[bp].ndx2b,di	;save 2nd buffer ptr
	mov	[bp].ndx2a,es
;retrieve column sort order vector
	push	ds
	pop	es		;destination, my segment
	mov	di,offset colordn
	lds	si,dword ptr [bp].prtcolb
	lodsw		;retrieve number of columns
	stosw
	mov	cx,ax
	rep movsb
	push	es		;restore addressability
	pop	ds
;define the source indexes and the destination indexes
	lea	ax,[bp].ndx1b
	mov	inptr,ax
	lea	ax,[bp].ndx2b
	mov	outptr,ax
	mov	headptr,0	;head of chain ptr
	page
;begin the sort
;outer loop, per column
portlp2:
;get the offset for this column
	mov	si,offset colordn
	lodsw
	add	si,ax		;points past current last offset
	dec	si
	lodsb			;ax = offset into list column
	xor	ah,ah
	mov	[bp].psrtcol,ax ;..and save in stack
;reset out buffer allocator
	mov	di,outptr	;reset out index offset to 0
	mov word ptr ss:[di],0
;clear the chain heads
	mov	ax,[bp].pocket	;number of pockets
	mov	cx,ax		;..n
	shl	ax,1		;..2n
	add	cx,ax		;3n = 1n + 2n, words to clear
	les	di,dword ptr [bp].chainb
	xor	ax,ax
	rep stosw
;number of items to sort
	mov	cx,[bp].psortn	;inner loop control
;start at head of chain, input buffer
	mov	di,inptr	;ptr to input buffer, this round
	les	di,dword ptr ss:[di]
	mov	di,headptr	;..but start at head of chain
	and	[bp].flagb,0EFh ;turn off head of chain found
;inner loop, per item
portlp3:
	mov	ax,es:[di]		;get item index
	mov	bx,ax			;save item index for destination
	mul	[bp].prtlen		;..times length of the word
	div	c16			;..divide by 16 for para,offset
	add	dx,[bp].psrtentb
	add	ax,[bp].psrtenta	;ax:dx->item
	add	dx,[bp].psrtcol 	;ax:dx->letter to sort on
	push	es			;get the index's destination
	push	di
	mov	si,outptr	;ss:si->outbuffer ptr
	les	di,dword ptr ss:[si]  ;es:di->1st available loc in output buf
	add word ptr ss:[si],4	     ;next output buffer location
;get the letter from the word in the list
	push	ds
	mov	ds,ax
	mov	si,dx
	lodsb
;find its pocket
	xor	ah,ah
;note: the following indexing is valid only for English lower case.
;To be more general, the code should call a weight routine to adjust the
;'letter' to a pocket index.  In other words, this works only for lower case
;letters a-z.  It doesn't handle accented letters, scharf s, thorn and eth,
;etc.
	sub	al,'a'          ;valid only for English lower case
	shl	ax,1
	mov	dx,ax
	shl	ax,1
	add	ax,dx		;index into pockets, based on letter
	lds	si,dword ptr [bp].chainb
	add	si,ax		;ds:si->pocket chain header
;put the index to this word into output buffer
	mov word ptr es:[di],bx
	cmp word ptr [si+4],0	;if this is the first item in this pocket
	jne	port30
	mov word ptr [si],di	;initialize chain head, offset to 1st index
	mov word ptr [si+2],di	;..and last index for this pocket
	jmp short port40
;chain previous entry in this pocket to this entry
port30:
	mov	bx,[si+2]	;get previous end of pocket
	mov word ptr es:[bx+2],di ;..and point to new end of pocket
	mov word ptr [si+2],di	;..update last pocket item in chain head
port40:
	inc word ptr [si+4]	;count items in this pocket
	pop	ds
	pop	di
	pop	es
	mov	di,es:[di+2]	;next index
	loop	portlp3 	;do for all items
;link up the pockets
	mov	di,outptr
	mov word ptr ss:[di],0	;reset buffer offset
	les	di,dword ptr ss:[di]
	mov	cx,[bp].pocket	;number of pockets
	dec	cx		;..less one
	push	ds
	lds	si,dword ptr [bp].chainb
	xor	ax,ax		;assume chain head ptr of 0
portlp4:
	mov	bx,si
	add	bx,6		;->next pocket
	cmp word ptr [si+4],0	;if this pocket is empty
	je	port60		;..skip it, nothing to link
	test	[bp].flagb,10h	;if first used pocket found
	jnz	port42		;..skip saving chain head
	or	[bp].flagb,10h	;else flag 1st used pocket found
	mov	ax,[si] 	;..and save chain head ptr
port42:
	mov	di,[si+2]	;get the last of this pocket
;es:di -> last item in this pocket
;seek 1st item in next pocket
portlp5:
	cmp word ptr [bx+4],0	;if next pocket is empty
	je	port50		;..skip to next pocket
	mov	dx,[bx] 	;first item of next occupied pocket
;connect last item of 1st pocket to 1st of next pocket
	mov	es:[di+2],dx
	mov	si,bx		;next occupied pocket becomes current
	loop	portlp4 	;..pocket
	jmp short port70
port50:
	add	bx,6
	loop	portlp5
	jmp short port70
port60:
	add	si,6		;next pocket
	loop	portlp4
port70:
	pop	ds
	dec	colordn 	;count this column sorted
	jz	port80		;done if all columns sorted
	mov	headptr,ax	;update start of chain
;switch in and out buffers
	mov	ax,inptr
	xchg	ax,outptr
	mov	inptr,ax
	jmp	portlp2 	;do sort on next column
;return indexes to the items in sorted order to caller
port80:
	mov	[bp].prtcolb,ax ;offset of chained indexes, 1st link
	mov	[bp].prtcola,es ;segment of indexes
;return index buffers memory
	les	di,dword ptr freeb
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	les	di,dword ptr saveb
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	clc			;good return code
portxit:
	pop	ds
	ret
psort	endp

;insertion sort, same stack parameters as for qsort
isort	proc	far
	cmp	[bp].srtn,1		;a single item
	ja	iort05
	clc				;..must already be sorted
	ret
iort05:
	push	ds
	push	es
	push	ax
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	mov	ax,cryptsb
	mov	ds,ax
	mov	si,offset saveb 	;save memory
	call	savefree
	mov	cx,[bp].srtlen		;get space for temp hold area
	call	getmem
	jnc	iort10
	jmp	iortxit 		;..no space for sorting
iort10:
	mov	holdb,di		;temp hold
	mov	holda,es
	mov	ax,[bp].srtn
	mov	isortn,ax		;number of items to compare
	les	di,dword ptr [bp].srtentb
	mov	aptra,es
	mov	bptra,es
iortlp1:
	dec	isortn
	mov	cx,isortn
	jcxz	iort20			;all done
	mov	bx,di
iortlp2:
	add	bx,[bp].srtlen		;es:bx-> b
	call	icomp
	jb	iort30			;A < B
	ja	iort40			;A > B
iort15:
	loop	iortlp2
	add	di,[bp].srtlen
	jmp	iortlp1
iort20:
	les	di,dword ptr saveb
	mov	[bp].sizeb,di
	mov	[bp].sizea,es
	les	di,dword ptr freeb
	mov	[bp].freehb,di
	mov	[bp].freeha,es
	clc
	jmp short iortxit
iort30: 		;A < B
	cmp	[bp].srtdir,0
	je	iort15
;exchange items A and B
iortx:
	push	cx
	mov	aptrb,di
	mov	bptrb,bx
	mov	dx,ds
	mov	bx,[bp].srtlen
	les	di,dword ptr holdb
	lds	si,dword ptr bptrb
	mov	cx,bx
	rep movsb
	mov	ds,dx
	les	di,dword ptr bptrb
	lds	si,dword ptr aptrb
	mov	cx,bx
	rep movsb
	mov	ds,dx
	les	di,dword ptr aptrb
	lds	si,dword ptr holdb
	mov	cx,bx
	rep movsb
	mov	ds,dx
	pop	cx
	mov	di,aptrb
	mov	bx,bptrb
	jmp	iort15
iort40:
	cmp	[bp].srtdir,1
	je	iort15
	jmp	iortx
iortxit:
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	es
	pop	ds
	ret
isort	endp

; on entry, bx -> index entry
getadd	proc	near
	mov	ax,es:[bx]	;get the index entry
	mul	[bp].srtlen	;..times sort item length
	div	c16		;..and to paragraphs and offset
	add	ax,[bp].srtenta ;ax = paragraph
	add	dx,[bp].srtentb ;dx = offset
	ret
getadd	endp

;compare A:B with callers comparison routine
;on entry, si -> intersegment address of A
;	   di -> intersegment address of B
compare proc	near
	push	ds
	push	es
	push	bx
	push	cx
	les	di,[di] 	;-> B
	lds	si,[si] 	;-> A
	call dword ptr [bp].srtcmpb	;compare A:B
	pop	cx
	pop	bx
	pop	es
	pop	ds
	ret
compare endp

icomp	proc	near
	push	ds
	push	es
	push	bx
	push	cx
	push	di
	push	es
	pop	ds
	mov	si,di			;ds:si->A
	mov	di,bx			;es:di->B
	call dword ptr [bp].srtcmpb	;compare A:B
	pop	di
	pop	cx
	pop	bx
	pop	es
	pop	ds
	ret
icomp	endp
crypts	ends
	end
