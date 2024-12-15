	page	53,132
	.8087
	include frame.str
	include bucket.str
	include crypt7b.asm
	extrn	fp2asc:far,showmsg:far,hline:far,vline:far,int2asc:far
crypt7	segment 'crya'
	assume cs:crypt7,ds:crypt7b
	public	analyze,ic,showstat
analyze proc	far
	push	ds
	mov	ax,crypt7b
	mov	ds,ax
	cld
	mov	si,offset panelid ;say counting
	mov	di,msga
	call	showmsg 	;panel function
forever:
	mov	ah,0
	int	16h
	push	ax
	mov	si,offset panelid
	mov	di,msga 	;reidentify panel
	call	showmsg 	;..clearing subfunction and errors
	pop	ax
	cmp	al,0		;if not extended
	jne	forever 	;..not for us
	mov	al,ah
	mov	bx,offset scantab ;classify the scan code
	xlat	scantab
	xor	bh,bh		;index into branch table
	mov	bl,al
	shl	bx,1
	jmp	fanout2[bx]
	page
;frequencies
a1:
	mov	si,offset msg2	;say that we are doing frequencies
	mov	di,msgb
	call	showmsg
	call	docount 	;counts, frequencies, length, IC
	call far ptr showstat	;display the statistics
	jmp	forever
f2:			;quit, return to caller
	pop	ds
	ret
analyze endp
showstat proc	far
	push	ds
	mov	ax,crypt7b
	mov	ds,ax		;addressability
	mov	es,ax
	test	[bp].flagb,08h	;if counts not done
	jnz	stat00
	call	docount 	;..do them anyhow
	test	[bp].flagb,08h	;if still not done
	jnz	stat00
	pop	ds		;..must be empty cryptogram
	ret
stat00:
	mov	ah,13		;row 13
	mov	al,1		;col 1
	mov	dh,80		;length 80
	call	hline		;draw horizontal line
	mov	ah,17		;row 17
	mov	al,1
	mov	dh,40
	call	hline		;draw horizontal line
	mov	ah,13		;row 13
	mov	al,27		;col 27
	mov	dh,5		;length
	call	vline		;draw vertical connector
	mov	di,13*160+54	;position for 'length'
	mov	si,offset lang
	mov	cx,6
	mov	ah,07h		;normal attribute
stat05:
	lodsb
	stosw			;format 'Length' in video buffer
	loop	stat05
	mov	al,' '
	stosw			;format separating blank in video
	mov	si,offset lnasc
	mov	cx,5
stat10:
	lodsb			;format length value in video buffer
	stosw
	loop	stat10
	mov	si,offset icname
	mov	cx,2
	mov	di,14*160+54	;position for IC
stat15:
	lodsb			;format 'IC' in video buffer
	stosw
	loop	stat15
	mov	al,' '
	mov	cx,4
	rep stosw		;separating blanks
	mov	si,offset icasc
	mov	cx,5
stat20:
	lodsb
	stosw			;format IC value in video
	loop	stat20
	mov	di,17*160	;start of frequency window
	mov	cx,7		;clear the window, 7 lines
	mov	ax,0720h
stat20a:
	push	cx
	push	di
	mov	cx,40
	rep stosw
	pop	di
	pop	cx
	add	di,160
	loop	stat20a
	mov	di,17*160+2	;start of frequency data
	mov	bx,offset bucketi ;name of character
	mov	si,offset freqs ;frequency of character
	mov	ax,bucketn	;number of characters
	xor	dx,dx
	shr	ax,1		;four per line
	rcr	dl,1
	shr	ax,1
	rcr	dl,1
	mov	cx,6
	shr	dl,cl		;remainder for last line
	cmp	ax,7		;no more than 6 lines
	jb	stat22
	je	stat90		;..unless no remainder
stat21:
	mov	ax,6
stat22:
	mov	cx,ax
stat30:
	call	doline		;format a complete line of frequencies
	add	di,160		;next line
	loop	stat30		;for all lines
	mov	cx,dx		;remainder letters
	jcxz	statxit
stat35:
	call	dolettr 	;format letter frequency
	loop	stat35		;for all remaining letters
statxit:
	pop	ds
	ret
stat90:
	cmp	dx,0
	je	stat22
	jmp	stat21
showstat endp
doline	proc	near
	push	cx
	push	di
	mov	cx,4		;letters per line
dolinelp:
	call	dolettr 	;format this letter
	loop	dolinelp	;for 4 letters
	pop	di
	pop	cx
	ret
doline	endp

dolettr proc	near
	push	cx
	mov	ah,07h
	mov	al,[bx] 	;get letter
	stosw			;..put in video
	add	di,2		;blank separator
	mov	cx,5		;characters in frequency
dolet05:
	lodsb			;frequency digit
	stosw			;..to video
	loop	dolet05
	add	bx,3		;next letter
	add	di,4		;next video position
	pop	cx
	ret
dolettr endp
docount proc	near
;prepare the buckets
	push	ds
	pop	es
	mov	di,offset bucketn	;point to the buckets
	push	ds
	lds	si,dword ptr [bp].alfb	;point to the cipher alphabet
	lodsw				;get alphabet count
	stosw				;..bucket count
	mov	cx,ax
freq00:
	lodsb				;get the character
	stosb				;..into bucket
	xor	ax,ax			;zero the counter
	stosw
	loop	freq00			;for entire alphabet
	pop	ds			;restore addressability
	mov	totals,ax		;zero totals
	mov	[bp].bilan,ax
	mov	di,offset zahlen
	mov	[bp].zahlb,di	       ;anchor counts in alphabetic order
	mov	[bp].zahla,ds
	mov	cx,bucketn		;zero 2nd set of letter counters
	rep stosw
;for all lines 1 to n
;   for all characters 1 to m
;   if countable
;	count it
;	total
;sort counts and characters
;for all counts
;   compute count[i]/total
;compute Index of Coincidence
	mov	cx,[bp].lines
	les	di,dword ptr [bp].sbuffb ;start of cryptogram
freq10: 			;outer loop for all lines
	push	di
	push	cx
	mov	cl,es:[di]	;number of characters
	jcxz	freq40		;..no chars, empty line
	inc	di		;es:di->1st character
freq20: 			;inner loop for all characters
	mov	al,es:[di]	;get the character
	inc	di		;..next character
	call	chk_char	;if it's valid
	je	freq30		;..count it
	loop	freq20		;for all characters
	jmp short freq40
freq30: 		;bx=index into bucket for this character
	inc word ptr bucketi[bx+1]   ;count this character
	mov	bx,ax		;index of character
	shl	bx,1		;count again
	inc	zahlen[bx]	;..for unsorted order
	inc	totals		;and sum of all characters
	inc	[bp].bilan
	loop	freq20		;for all countable characters
freq40:
	pop	cx
	pop	di
	add	di,81		;next line
	loop	freq10		;for all lines
	cmp	totals,0	;if no characters counted
	jne	freq50
	and	[bp].flagb,0F7h
	mov	si,offset err1	;..say cryptogram is empty
	mov	di,msgc
	call	showmsg
	ret
freq50:
	call	sortb		;sort buckets descending
	call	percent 	;figure percentages
	or	[bp].flagb,08h	;flag counts made
	mov	si,offset totals
	call far ptr ic
	push	bp
	sub	sp,4	;prepare frame for conversion
	mov	bp,sp
	mov	ax,2	;three digits of precision
	mov word ptr [bp],offset fpret
	mov word ptr [bp+2],ds
	finit
	fld	icfp
	call	fp2asc	;convert to ascii
	add	sp,4
	pop	bp
	mov	ax,word ptr fpret+4 ;pick up two digits
	cmp	al,'0'  ;if leading digit not 0
	jne	freq52	;..format E notation without E
	xchg	ah,al	;else, not E notation, and has trailing 0
freq52:
	mov word ptr icasc+3,ax     ;..and format in display field
	mov	ax,totals	;convert length to ascii
	mov	si,offset lnasc
	call	int2asc
	ret
docount endp
;ds:si->buckets n,bucketi,total.  Mapped by bucket struc
;compute (+/bucketi*bucketi-1)/total*total-1
;returns bx:si->icfp, double floating point
ic	proc	far
	push	cx
	mov	cx,[si].buckn	;loop control
	lea	bx,[si].bucki	;bucket pointer
	finit			;initialize NDP
	fld1			;1
	fldz			;sum,1
iclp:
	fild word ptr [bx]	;f,sum,1
	fld	st(0)		;f,f,sum,1
	fsub	st,st(3)	;f-1,f,sum,1
	fmul			;f*f-1,sum,1
	fadd			;sum,1
	add	bx,3		;next bucket
	loop	iclp
	fild	[si].bucks	;total,sum,1
	fld	st(0)		;total,total,sum,1
	fsub	st,st(3)	;total-1,total,sum,1
	fmul			;denom,numer,1
	fdiv			;ic,1
	push	ds		;address my segment
	mov	bx,crypt7b
	mov	ds,bx
	fstp	icfp		;1
	fstp	st(0)		;empty NDP
	mov	si,offset icfp
	pop	ds
	pop	cx
	ret
ic	endp
;al=char to be validated
chk_char proc	near
	push	cx
	push	es
	push	di
	les	di,dword ptr [bp].alfb	;point to cipher alphabet
	mov	cx,es:[di]	;length of alphabet
	add	di,2		;1st character of alphabet
	mov	bx,di
	repne scasb		;find the character's position
	jne	chkchrxt	;..not in the alphabet
	dec	di		;figure offset into buckets
	mov	ax,di
	sub	ax,bx
	push	ax
	mul	c3
	mov	bx,ax		;..and return offset in bx
	pop	ax		;..and index in ax
	cmp	ax,ax		;force equal return code
chkchrxt:
	pop	di
	pop	es
	pop	cx
	ret
chk_char endp
;sort buckets in descending order
sortb	proc	near
	push	ds
	pop	es
	mov	si,offset bucketi
	mov	di,offset bucketi+3
	mov	cx,bucketn
	dec	cx
	mov	ax,cx		;compute last bucket's address
	mul	c3
	mov	bx,ax
	add	bx,si		;bx-> last bucket
sort00:
	mov	di,si
	add	di,3		;next bottom
	push	di
	mov	ax,[si+1]	;top:bottom
sort10:
	cmp	ax,[di+1]
	jae	sort20		;top >= bottom, get next bottom
	xchg	ax,[di+1]	;bucket[j]<-bucket[i]  exchange bucket counts
	mov	[si+1],ax	;bucket[i]<-bucket[j]
	mov	dl,[si] 	;exchange bucket names
	xchg	dl,[di]
	mov	[si],dl
sort20:
	add	di,3		;next bottom
	cmp	di,bx		;if still in buckets
	jna	sort10		;..continue sorting
	pop	si		;next top is old bottom
	loop	sort00
	ret
sortb	endp
percent proc	near
	mov	cx,bucketn
	mov	si,offset bucketi
	mov	di,offset freqs
	finit			;initialize 8087
	fild	c100		;100
	fild	totals		;sum,100
	push	bp
	sub	sp,4		;frame for fp2asc
	mov	bp,sp
	mov word ptr [bp],offset fpret
	mov word ptr [bp+2],ds
percntlp:
	mov	ax,4		;print precision (four significant digits)
	fild word ptr [si+1]	;count,sum,100
	fdiv	st,st(1)	;count/sum,sum,100
	fmul	st,st(2)	;percent,sum,100
	call	fp2asc		;convert to ascii, ii.dd
	call	tofreq		;format ascii in table
	add	si,3		;next count
	add	di,5		;next percent destination
	loop	percntlp
	add	sp,4
	pop	bp
percntxt:
	ret
percent endp
;es:di->frequency table of five bytes for digits of form ii.dd
;fpret = length,ascii digits
tofreq	proc	near
	mov	bx,di		;will have to restore several times
	push	cx
	push	si
	mov	cx,5		;blank the percentage destination
	mov	al,' '
	rep stosb
;scan for the decimal point
	mov	cl,fpret	;length of ascii percentage
	mov	di,offset fpret+1
	mov	al,'.'
	repne scasb
	jne	tofrq50 	;integer result
	mov	ax,di
	dec	ax		;offset to decimal point
	sub	ax,offset fpret+2	;number of integer digits
	mov	di,bx
	add	di,2		;position of decimal point in ii.ff
	sub	di,ax		;less number of integer digits
	mov	cl,fpret	;length to move
tofrq10:
	dec	cl		;ignore leading blank
	mov	si,offset fpret+2	;what to move
	rep movsb
	mov	di,bx
	pop	si
	pop	cx
	ret
tofrq50:		;integer result
	mov	di,bx
	inc	di		;assume single digit
	mov	cl,fpret	;size of integer
	cmp	cl,2
	je	tofrq10
	dec	di		;two digits
	jmp	tofrq10
tofreq	endp
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
curup:
pagup:
curleft:
curite:
endkey:
curdn:
pagdn:
insert:
delete:
sf1:
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
bottom:
botpg:
top:
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
crypt7	ends
	end
