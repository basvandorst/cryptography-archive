	page	53,132
	.8087
	include frame.str
	include bif.str 	;bifid parameters
	include befehl.str	;command parser parameters
	include bifsite.str	;siting, pairing parameters
	include bifkey.str	;keysquare and parms
	include bifmark.str	;matched text controls
	include crypt25b.asm
	extrn	savefree:far,getcurs:far,freeadj:far,int2asc:far,getmem:far
	extrn	findblnk:far,sumlet:far,bifline:far,bifloga:far,hline:far
	extrn	bifloga:far,showmsg:far,setcurs:far,freemem:far,biflogz:far
	extrn	adjcurs:far,bifrite:far,bifleft:far,del_char:far,rub_out:far
	extrn	getoken:far,cmdlook:far,sitebif:far,sitip:far,setcip:far
	extrn	sitequ:far,isort:far,bifnxn:far,cordsub:far,biflog:far
	extrn	bifdim:far,clear_line:far,showkey:far,bifperm:far,bifix:far
	extrn	bif_keysub:far,asc2int:far,bifdic:far,fp2asc:far
	extrn	log:far,help:far
crypt25 segment 'crya'
	assume	cs:crypt25,ds:crypt25b
	public	bifid
;on entry, ax = 5, the root for a 5x5
;	     or 6, the root for a 6x6
bifid	proc	far
	push	ds
	mov	dx,crypt25b
	mov	ds,dx
	mov	root,ax 		;preserve the root
	mul	root			;compute the square size
	mov	square,ax		;..and save
	mov	si,offset saveb 	;save current memory
	call	savefree
	call	getcurs 		;save old cursor position
	mov	oldpos,dx
	mov	[bp].bifalfb,offset bifalf
	mov	[bp].bifalfa,ds
	call	freeadj 		;align memory to seg boundary
	mov	cx,clearb-cleara	;clear areas
	push	ds
	pop	es
	mov	di,offset cleara
	mov	al,0
	rep stosb
	mov	ax,root
	mov	bifa.root1,ax
	mov	bifb.root1,ax
	mov	[bp].bfkeyb,0	;clear the key anchor
	mov	[bp].bfkeya,0
	mov	[bp].srtcmpa,cs
	mov	[bp].srtdir,1		;descending sort
;initialize command line
	mov word ptr cmdprm.cmdline+2,ds	  ;tell parser where cmd line
	mov word ptr cmdprm.cmdtabp,offset cmdtaba ;.where parsing table is
	mov word ptr cmdprm.cmdtabp+2,ds
	mov	cmdprm.cmdn,cmdtabn		;..number of table entries
	mov	j2i,'J'
	mov	j2i+1,'I'
	mov	di,offset bifalf
	mov	al,' '
	mov	cx,2*36
	rep stosb
	mov	di,offset bifalf
	mov	si,offset alf5		;assume 5x5 alphabet
	mov	cx,square
	cmp	cx,25
	je	bif00
	mov word ptr j2i,2020h
	mov	si,offset alf6		;it's 6x6 alphabet
bif00:
	rep movsb		;load the appropriate alphabet
	mov	al,' '
	mov	di,offset cmdbuf	;clear command buffer
	mov	cx,80
	rep stosb
	mov	di,offset msg3+18	;clear period(s)
	mov	cx,5
	rep stosb
	mov	cmdlen,0	;clear command length
	mov	cmdcol,0
	mov	word ptr bifa.ccurs,0300h ;set plaintext cursor start col
	mov	word ptr bifb.ccurs,0300h
	mov	keycurs,1019h
;determine 1st period
	mov	cx,[bp].lines		;number of apparent cipher lines
	les	bx,dword ptr [bp].sbuffb ;start of lines
	mov	bifa.srcptr,bx		;offset to 1st unformatted bifid
	call	findblnk		;find 1st bifid period if any
	mov	bifa.period,ax
	mov	si,offset toasc
	call	int2asc
	mov	ax,word ptr toasc+3
	mov	word ptr msg3+18,ax	;period to messsage
	mov	ax,root
	add	al,'0'          ;identify bifid as 5x5 or 6x6
	mov	panel1+7,al
	mov	panel1+9,al
;find start of 2nd bifid, if any
	xor	ax,ax		;count lines of 1st bifid
biflp1:
	cmp byte ptr es:[bx],0		;look for an empty line
	jne	bif10
	mov	bifa.ln,ax		 ;lines of 1st unformatted bifid
	dec	cx			;remaining lines
	jcxz	bif20
	mov	bifb.ln,cx		 ;..are 2nd bifid
	add	bx,81			;start of 2nd bifid
	mov	bifb.srcptr,bx
	or	flag1,01h		;flag twin bifid
	call	findblnk	;find period of 2nd bifid
	mov	bifb.period,ax
	mov	si,offset toasc
	call	int2asc
	mov	ax,word ptr toasc+3
	mov	word ptr msg3+21,ax
	jmp short bif20
bif10:
	inc	ax		;count the line
	add	bx,81		;next line
	loop	biflp1		;do for all lines
	mov	bifa.ln,ax		 ;lines of 1st and only bifid
	mov	bifb.ln,cx		 ;no lines of 2nd bifid
bif20:
	les	bx,dword ptr [bp].sbuffb  ;count the chars of the 1st bifid
	mov	cx,bifa.ln	;number of cipher lines
	call	sumlet		;..count will be corrected later when
	mov	bifa.cnt,ax	;..alphabet is finally known
	test	flag1,01h	;if there is a second bifid
	jz	bif30
	mov	ax,bifb.ln	 ;..count its letters too
	mov	bx,bifb.srcptr	;start of twin cipher
	mov	cx,bifb.ln	;number of lines
	call	sumlet
	mov	bifb.cnt,ax	;save letter count
bif30:
	cmp	bifa.cnt,0
	jne	bif40
	or	flag1,80h	;empty 1st bifid, must quit
	or	flager,01h
	jmp	nogo
bif40:
	mov	ax,bifa.cnt	;check for max size
	add	ax,bifb.cnt
	jc	bif41		;greater than 64K
	cmp	ax,3000 	;we can handle cipher(s) under 3000
	jna	bif45		;bytes long
bif41:
	or	flag1,80h	;too big
	or	flager,08h
	sub	ax,3000 	;tell user by how much it's too big
	mov	si,offset msg25+22
	call	int2asc
	jmp	nogo
bif45:
	add	ax,75		;..plus max matched ciphertext
	mov	cx,3		;..times 8 for dimensioning 2 top lines
	shl	ax,cl		;..and 2 bottom lines
	mov	worklen,ax	;..save length for later
	mov	cx,ax
	call	getmem
	jnc	bif48
nomem:
	or	flag1,80h	;no memory, must quit
	or	flager,02h
	jmp	nogo
bif48:
	mov	workb,di	;->expansion work area
	mov	worka,es
	mov word ptr pairs.sitpair,di
	mov word ptr pairs.sitpair+2,es
	mov	si,offset bifa	;get 5N memory
	call	get5n
	jc	nomem
	test	flag1,01h	;if there is a 2nd bifid
	jz	bif70
	cmp	bifb.cnt,0	;..check that it has a count
	jne	bif60
	or	flag1,80	;..empty 2nd bifid, something wrong
	or	flager,04h	;..flag error
	jmp	nogo		;..and wait to quit
bif60:
	mov	si,offset bifb
	call	get5n		;get memory for 2nd bifid
	jc	nomem
bif70:
	call	bifloga 	;start the transaction log
	jnc	bif75
	or	flag1,80h	;nogo error
	or	flager,10h
	jmp	nogo
bif75:
	call	freeadj 	;set memory to segment boundary
	mov	si,offset bifa	;dimension 1st bifid
	call	expand
	test	flag1,01h	;if twin bifid
	jz	bif80
	mov	si,offset bifb	;..dimension it too
	call	expand
bif80:
	mov	si,offset bifa	;set number of displayable lines
	call	bifline
	test	flag1,01h		;if there is a 2nd bifid
	jz	bif90
	mov	si,offset bifb		;..set its display lines
	call	bifline
bif90:
;set up transaction logging
	mov	si,offset root
	mov	[bp].bifkeyb,si 	;->bifid key
	mov	[bp].bifkeya,ds
	mov	si,offset bifa
	mov	[bp].bifprm1b,si	;1st bifid parms
	mov	[bp].bifprm1a,ds
	mov	si,offset bifb		;2nd bifid parms
	mov	[bp].bifprm2b,si
	mov	[bp].bifprm2a,ds
;save volatile memory for various resets
	mov	si,offset save2b	;site table or matched text string
	call	savefree
	call	freeadj 		;align to segment boundary
	mov	si,offset save3b	;site frag table, relocated key
	call	savefree		;..for key, must be segment aligned
;prepare screen for display
	mov	ax,0720h		;clear the screen
	mov	cx,23*80
	mov	es,[bp].videoa
	mov	di,160
	rep stosw
	mov	ah,13		;draw the bar at line 13 on screen
	mov	al,1
	mov	dh,80
	call	hline
	mov	si,offset panel1	;assume single bifid
	test	flag1,01h
	jz	bif110
	mov	si,offset panel2
bif110:
	mov	di,24*160+32		;panel id location
	call	showmsg
	mov	di,24*160+80	;show assumed periods
	mov	si,offset msg3
	call	showmsg
	or	flag1,20h	;flag pending message to clear
	mov	cx,root
	mov	es,[bp].videoa
	mov	di,15*160+50	;display key column coordinates
	mov	ax,0F31h
biflp2:
	stosw
	inc	al
	add	di,2
	loop	biflp2
	mov	cx,root
	mov	di,16*160+46	;display key row coordinates
	mov	al,'1'
biflp3:
	stosw
	inc	al
	add	di,158
	loop	biflp3

refresh:
	mov	es,[bp].videoa	;display unsolved key letters
	mov	di,23*160+46
	mov	si,offset bifalf
	mov	cx,36
	mov	ah,07h
biflp4:
	lodsb
	stosw
	loop	biflp4
;we will need to know how many regular blocks per line and whether or not
;a line has a final short block.  There will be 1, 2 or 3 lines to display.
	mov	si,offset bifa
	call	setblks 		;set parameters for displayed lines
	mov	si,offset bifa
	call	setnat			;set full naturals in plaintext if any
	test	flag1,01h		;if twin bifid
	jz	bif120
	mov	si,offset bifb		;..set parms for 2nd bifid
	call	setblks
	call	setnat			;full naturals in 2nd bifid if any
bif120:
	mov	si,offset bifa		;display 1st bifid
	call	showcip
	call	showsub 	;show plaintext
	call	showelg 	;show eligible substitutes
	call	showpos 	;show position
	mov	dx,word ptr [si].ccurs
	call	setcurs 	;set cursor for substitution
;await keyboard input
forever:
	mov	ah,0		;wait for keyboard entry
	int	16h
	test	flag1,80h	;if any fatal error pending
	jnz	bifxit		;..must quit regardless of keystroke
	mov	savechar,ax	;save keystroke
	test	flag1,20h	;if message pending
	jz	bif140
	test	flag2,04h	;..and marks pending
	jz	bif130
	mov	si,offset msg14 ;..reestablish marks pending msg
	mov	di,24*160+80
	call	showmsg
	jmp short bif140
bif130:
	and	flag1,0DFh	;..or turn of msg pending flag
	mov	es,[bp].videoa	;..and clear the message
	mov	di,24*160+80
	mov	ax,0F20h
	mov	cx,40
	rep stosw
	mov	ax,savechar	;restore keyboard char
bif140:
	mov	bx,offset keytab ;fanout on keystroke
	xlat	keytab
	xor	bh,bh
	mov	bl,al
	shl	bx,1
	jmp	fanout1[bx]
extended:			;extended keystroke, next fanout
	mov	al,ah
	mov	bx,offset scantab
	xlat	scantab
	xor	bh,bh
	mov	bl,al
	shl	bx,1
	jmp	fanout2[bx]

;exit bifid solver
f2:
bifxit:
	mov	si,offset saveb ;restore memory
	call	freemem
	call	biflogz 	;erase transaction log
	mov	dx,oldpos	;restore old cursor position
	call	setcurs
	pop	ds
	ret

nogo:
	mov	si,offset msg2		;assume error is empty bifid
	test	flager,05h		;..either 1st or 2nd
	jnz	nogo10
	mov	si,offset msg1		;maybe it's no memory
	test	flager,02h
	jnz	nogo10
	mov	si,offset msg25 	;maybe ciphers too big
	test	flager,08h
	jnz	nogo10
	mov	si,offset msg20 	;maybe transaction log is broken
	test	flager,10h
	jnz	nogo10
nogo10:
	mov	di,24*160+80		;say what the error is
	call	showmsg
	jmp	forever 		;then wait till quit is forced

;command toggle
escape:
	xor	flag1,40h	;toggle command switch
	test	flag1,40h
	jnz	esc10		;go to command entry
;return from command mode
	mov	si,offset savebar  ;restore screen line
	mov	es,[bp].videoa
	mov	di,12*160
	mov	cx,80
	rep movsw
	mov	dx,savecurs	;restore cursor
	call	setcurs
	jmp	forever
;command mode
esc10:
	call	getcurs 	;save non-command cursor position
	mov	savecurs,dx
	push	ds		;save bar line on screen
	push	ds
	pop	es
	mov	di,offset savebar
	mov	ds,[bp].videoa
	mov	si,12*160
	mov	cx,80
	rep movsw
	pop	ds
	call	showcmd 	;display current command
	jmp	forever

;skip to unsolved character
blank:
	test	flag1,40h	;if in command mode
	jz	skip
	jmp	enguc		;..treat as normal char
skip:
	cmp	area,0		;if not in substitution area
	je	skip00
	jmp	forever 	;..ignore
skip00:
	mov	si,offset bifa	;1st or 2nd bifid
	test	flag2,02h
	jz	skip10
	mov	si,offset bifb
skip10:
	mov	ax,[si].period
	inc	ax
	mov	[bp].tempcnt,ax ;period + 1 to account for block spaces
	mov	es,[si].cipa	;test if I can skip
	mov	di,[si].subp
	call	getpos		;get current position in bx
	add	di,bx
	cmp byte ptr es:[di],' ' ;..if it's blank
	je	skipxt		;..stay put
	mov	ax,[si].l1full	;figure number of chars on screen
	add	ax,[si].l2full	;total full blocks on screen
	add	ax,[si].l3full
	mul	[si].period	;blocks x period
	add	ax,[si].l1kurz	;..+ final short blocks
	add	ax,[si].l2kurz
	add	ax,[si].l3kurz
	mov	cx,ax
	mov	ax,[si].period	;chars per block
	mul	[si].bperl	;..x blocks per line
	mul	[si].curln	;..x number of lines off screen top
	add	cx,ax		;chars from start of bifid to last screen char
	sub	cx,bx		;chars from position to last screen char
	jcxz	skipxt		;..none, at the last char already
	mov	al,' '          ;look for unsolved letter to skip to
	repne scasb
	jne	skipxt		;no unsolved letter to skip to
	dec	di		;correct overshoot
	sub	di,[si].subp	;new position
	mov	ax,di
	div	[si].period	;ax = block number
	push	dx		;save column
	xor	dx,dx
	div	[si].bperl	;ax = line number, dx = block on line
	sub	ax,[si].curln	;ax = 0, 1, or 2 -- 1st, 2nd, or 3rd line
	shl	ax,1		;adjust to screen row
	shl	ax,1
	add	ax,3
	mov	[si].rcurs,al	;set cursor row
skip20:
	mov	ax,dx		;recover block on line
	mul	[bp].tempcnt
	pop	dx
	add	ax,dx
	mov	[si].ccurs,al	;new column for cursor
	call	showelg
	mov	dx,word ptr [si].ccurs
	call	setcurs
	call	showpos
skipxt:
	jmp	forever

useasis:
punct:
numeral:
enguc:
englc:
euruc:
eurlc:
	mov	ax,savechar
	test	flag1,40h	;if command mode
	jnz	cmdchar 	;..accept input for command line
	cmp	area,0		;if substitution area
	je	sublet		;..try to substitute a letter
	cmp	area,3		;if key area
	jne	char10
	jmp	subkey		;..try to fill a key letter
char10:
	jmp	forever 	;else ignore for now
cmdchar:
	cmp	cmdlen,80	;if command line is full
	jb	cmdchr10
	mov	ax,0E07h	;..beep the user
	push	bp
	int	10h
	pop	bp
	jmp	forever 	;..and ignore keystroke
cmdchr10:
	mov	si,offset cmdbuf ;put char into buffer
	xor	bx,bx
	mov	bl,cmdcol	;..at the cursor position
	mov	[si][bx],al
	mov	al,cmdlen	;bump command length to
	cmp	al,bl		;..greater of cursor pos, or command length
	jae	cmdchr20
	mov	al,bl
cmdchr20:
	inc	al
	mov	cmdlen,al
	call	showcmd 	;display updated command
	jmp	curite		;update cursor

;keystroke in substitution area
sublet:
	call	chklet		;validate the letter
	jnc	sublet10
	jmp	forever 	;letter was invalid
sublet10:
	mov	savechar,ax	;valid letter, save for later
	mov	si,offset bifa	;assume 1st bifid
	test	flag2,02h
	jz	sublet40
	mov	si,offset bifb
sublet40:
	call	getpos		;get offset into cipher in bx
	mov	es,[si].cipa
	mov	di,[si].subp	;es:di->plaintext
	cmp byte ptr es:[di][bx],' ' ;can only substitute for unsolved pos
	je	sublet50
	mov	si,offset msg26 ;tell user to undo, can't change
	jmp	endl22
sublet50:
	call	pushbif 	;save current state in case of invalid sub
	mov	ax,savechar	;retrieve char to be substituted
	mov	es:[di][bx],al	;plunk letter into plaintext
	test	flag2,08h	;if matched twins
	jz	sublet58
	mov	di,offset mark1 ;try to put char in both ciphers
	mov	si,offset bifa	;try 1st marked area
	cmp	[di].markcip,1
	je	sublet52
	mov	si,offset bifb
sublet52:
	cmp	bx,[di].marka
	jb	sublet55	;doesn't go in first mark
	cmp	bx,[di].markz
	ja	sublet55	;doesn't go in first mark
	mov	es,[si].cipa	;it does go in marked area
	mov	di,[si].subp
	mov	es:[di][bx],al
sublet55:
	mov	di,offset mark2 ;try 2nd marked area
	mov	si,offset bifb
	cmp	[di].markcip,2
	je	sublet57
	mov	si,offset bifa
sublet57:
	cmp	bx,[di].marka
	jb	sublet58	;out of marked area
	cmp	bx,[di].markz
	ja	sublet58	;out of marked area
	mov	es,[si].cipa	;in marked area
	mov	di,[si].subp
	mov	es:[di][bx],al
sublet58:
	les	di,dword ptr workb
	push	es
	push	di		;work area ptr
	mov	cx,worklen	;clear dimensioning area
	mov	al,' '
	rep stosb
	push	ds
	mov	si,offset bifa
	push	si		;cipher parm ptr
	call	bifdim		;dimension plaintext and corresponding cipher
	mov	pairlen,ax	;save number of pairs dimensioned
	test	flag1,01h	;if there is a 2nd bifid
	jz	sublet60
	mov	cl,3		;..dimension it too
	shl	ax,cl		;eight bytes per pair
	les	di,dword ptr workb
	add	di,ax		;dimensioning area for 2nd bifid
	push	es
	push	di
	push	ds
	mov	si,offset bifb
	push	si
	call	bifdim		;dimension 2nd bifid
	add	pairlen,ax	;..and count its paired letters
sublet60:
	test	flag2,08h	;if we have marked ct for twins
	jz	sublet70
	mov	ax,pairlen
	mov	cl,3		;..abut paired ct too
	shl	ax,cl		;eight bytes per pair
	les	di,dword ptr workb
	add	di,ax
	mov	ax,marklen	;length of marked paired letters
	shl	ax,cl		;eight bytes per pair
	mov	cx,ax
	push	ds
	lds	si,dword ptr marksvb
	rep movsb
	pop	ds
sublet70:
	mov	ax,pairlen	;prepare siting parameters
	test	flag2,08h	;if there is matched twin text
	jz	sublet80
	add	ax,marklen	;add it too
sublet80:
	mov	pairs.sitcril,ax ;total number of paired letters
	mov word ptr pairs.sitfrag+2,ds
	mov word ptr pairs.sitfrag,offset collect
	mov	ax,root
	mov	pairs.sitroot,ax
	mov	fragn,0 	;start fragment counter
	les	di,dword ptr [bp].freehb
	mov	fragb,di
	mov	fraga,es
	mov	[bp].srtentb,di
	mov	[bp].srtenta,es
	push	ds
	mov	si,offset pairs
	push	si
subltlp1:
	call	sitequ		;pull a fragment string
	jnc	sublet90
;keysquare conflict, this letter not a valid substitution
	add	sp,4		;invalid substitution
	mov	si,offset msg10 ;say invalid substitution
sublter1:
	mov	di,24*160+80
	call	showmsg
	or	flag1,20h
sublter2:
	call	popbif		;restore status quo ante
	test	flag2,80h	;if log is broken
	jz	sublet85
	mov	si,offset msg23 ;say unrecoverable error
	mov	di,24*160+80	;key is in contradictory, unknown state
	call	showmsg
	or	flag1,80h	;must quit
sublet85:
	jmp	forever
sublet90:
	cmp	ax,0		;if null string
	je	sublet120	;..end of fragment testing
	call	movfrag 	;move fragment to fragment table
	jc	subletmem	;memory failure
	inc	fragn		;count the fragment
	jmp	subltlp1
subletmem:
	add	sp,4		;collapse frag building stack
sublet110:
	mov	si,offset msg1	;say no memory
	jmp	sublter1
;end of fragment list building
sublet120:
	add	sp,4		;collapse frag building stack
	mov	[bp].srtcmpb,offset compare
	mov	[bp].srtlen,26	;sort the fragment list -- entry length
	mov	ax,fragn
	mov	[bp].srtn,ax	;number of items to sort
	call	isort
	jc	subletmem	;not enough memory to sort
;build a new keysquare
	push	ds
	pop	es
	mov	di,offset fragn
	mov	ax,root
	call	bifnxn		;build an NxN key for root 5 or 6
	or	ax,ax		;if all went well
	jz	sublet130	;..relocate keysquare
	cmp	ax,1		;if memory failure
	je	subletmem	;..say so
	mov	si,offset msg10 ;else
	jmp	sublter1	;..say invalid substitution
sublet130:
	or	flag1,02h	;we have a key
	call	relocate	;relocate key to compact memory
	jmp	sitendok	;build and show new plaintext

;put a letter into the keysquare
subkey:
	call	chklet		;validate the letter
	jnc	subk10
	jmp	forever 	;invalid letter
subk10:
	mov	bx,keycurs
	call	bif_keysub	;put al letter into keysquare if no conflict
	jnc	subk20		;..at position bx
	cmp	ax,-1
	jne	subk15
	or	flager,10h	;broken log
	or	flag1,80h	;must quit error
	mov	si,offset msg23
	mov	di,24*160+80
	call	showmsg
subk15:
	or	flag1,20h	;message to clear
	jmp	forever
subk20:
	jmp	sitendok	;new key and plaintext

;delete character at cursor position
delete:
	test	flag1,40h
	jnz	delt10
	jmp	forever
delt10:
	mov	dx,cmdcurs
	push	ds
	pop	es
	mov	di,offset cmdlen
	call	del_char		;use editor's delchar routine
	call	showcmd
	jmp	forever

;clear command line
f5:
	test	flag1,40h	;ignore clear line
	jnz	f510
	jmp	forever 	;..if not in command mode
f510:
	mov	cmdcol,0	;column 0
f520:
	push	ds
	pop	es
	mov	di,offset cmdlen	;->l,command buffer
	mov	dx,cmdcurs
	call	clear_line
	call	showcmd
	jmp	forever

;clear command line from cursor position
f6:
	test	flag1,40h
	jnz	f520
	jmp	forever

;delete char at command line cursor position
rubout:
	test	flag1,40h
	jnz	rubout10
	jmp	forever
rubout10:
	push	ds
	pop	es
	mov	di,offset cmdlen	;es:di->command length, buffer
	mov	dx,cmdcurs		;command line cursor position
	call	rub_out 	;use the editor's rubout
	mov	cmdcurs,dx	;update command cursor
	call	showcmd 	;show command and cursor
	jmp	forever

;enter (end of line)
endline:
	test	flag1,40h	;if not command mode
	jnz	endl10
	mov	bx,area
	shl	bx,1
	jmp	ftab4[bx]
endl10: 			;else set up command parsing parms
	mov word ptr cmdprm.cmdline,offset cmdbuf  ;tell parser where cmd is
	xor	ax,ax
	mov	al,cmdlen
	mov	cmdprm.cmdleng,ax		;..how long line is
	mov	si,offset cmdprm	;tell parser where parms are
	push	ds
	push	si
	call	getoken 	;determine the command
	jnc	endl30
	cmp	cmdprm.toklen,0 ;no command found, if empty line
	jne	endl20
	jmp	forever 	;..just ignore the enter
endl20:
	mov	si,offset msg4	;else say bad command
endl22:
	mov	di,24*160+80
	call	showmsg
	or	flag1,20h	;..flag msg to clear
	jmp	forever
endl30: 			;found a token
	push	ds
	push	si
	call	cmdlook 	;find the command's routine
	jc	endl20		;..invalid command
	mov	bx,cmdprm.cmdndx ;get routine's index
	shl	bx,1
	jmp	cmdtabb[bx]	;and go to it.

;enter key in substitution area, next line if any
subend:
	mov	si,offset bifa	;assume 1st bifid
	test	flag2,02h
	jz	subend10
	mov	si,offset bifb	;..it's 2nd bifid
subend10:
	mov	[si].ccurs,0	;set the column to 0
	jmp	subdn10

;skip to start of next block
tab:
	test	flag1,40h	;tab not valid in command mode
	jz	tab10
	jmp	forever
tab10:
	cmp	area,0		;tab valid only in substitution area
	je	tab20
	jmp	forever
tab20:
	mov	si,offset bifa	;figure out which cipher is current
	test	flag2,02h
	jz	tab30
	mov	si,offset bifb
tab30:
	mov	ax,[si].period	;figure position of end of line
	inc	ax		;interblock space
	mov	[bp].tempcnt,ax ;save for later
	xor	bx,bx
	mov	bl,[si].rcurs
	sub	bx,3		;0, 4 , 8 - line 1, 2 or 3
	mul	[si][bx].l1full
	mov	dx,[si][bx].l1kurz	;get short block
	add	ax,dx			;..count its length too
	or	dx,dx			;..if any
	jnz	tab40
	dec	ax			;..else decrement, no final space
tab40:
	mov	[bp].templen,ax ;position to end of line
	xor	ax,ax
	mov	al,[si].ccurs	;current position in line
	xor	dx,dx
	div	[bp].tempcnt	;number of blocks to position
	inc	ax		;next block
	mul	[bp].tempcnt	;start of next block with interblock space
;check whether next block is this or next line
	cmp	ax,[bp].templen
	ja	tab50		;next line, cursor right from end of line
;this line, back up to end of current block
	sub	ax,2		;this line, cursor right from block-1
	mov	[bp].templen,ax
tab50:
	mov	ax,[bp].templen
	mov	[si].ccurs,al
	jmp	subr10		;join common cursor right code

;enter key in site area -- a trial site is picked
sitend:
	mov	si,offset save3b ;return keysquare memory
	call	freemem
	xor	ax,ax		;don't reexpand
	call	reset		;but otherwise reset plaintext, coords
	mov	[bp].biflogn,0	;reset the log
	mov	[bp].bfkeyb,0	;no key for transaction logging
	mov	[bp].bfkeya,0
	and	flag1,0FFh-02h	;no key
	xor	ax,ax		;get the site selected
	mov	al,sitrow
	sub	ax,14
	add	ax,sitea
	mov	cx,3
	shl	ax,cl
	les	di,dword ptr sitabb
	add	di,ax		;es:di->site entry
	mov	sitib,di	;..save for later, ith site
	mov	sitia,es
	mov	ax,26		;two chars per item, max 12 items, length
	mul word ptr es:[di+4]	;..times number of strings
	mov	cx,ax		;number of bites needed for fragment table
	mov	ax,es:[di+4]
	mov	fragn,ax	;save max fragments
	mov	[bp].srtn,ax
	mov	ax,es:[di+2]	;cipher offset of tip
	mov	pairs.siti,ax
	call	getmem
	jnc	sitend10
sitend05:
	mov	si,offset msg1	;say not enough memory
sitender:
	mov	di,24*160+80
	call	showmsg
	or	flag1,20h	;flag message to clear
	jmp	forever 	;await next key stroke
sitend10:
	mov	fragb,di	;fragment list anchor, later keysquare
	mov	fraga,es
	mov	[bp].srtentb,di
	mov	[bp].srtenta,es
	mov word ptr pairs.sitfrag,di	;where to put fragments
	add word ptr pairs.sitfrag,2
	mov word ptr pairs.sitfrag+2,es
	xor	ax,ax
	mov	al,tip
	mov	pairs.sitcril,ax	;crib length
	mov	si,offset pairs 	;siting parameters
	push	ds
	push	si
	call	sitip		;dimension the tip
	add word ptr pairs.sitpair,2
	call	setcip		;dimension the cipher segment
	sub word ptr pairs.sitpair,2
sitendlp:
	call	sitequ		;put an equivalent string into frag table
	cmp	ax,0
	je	sitend20
	mov	es:[di],ax	;save fragment length
	add word ptr pairs.sitfrag,26
	add	di,26
	jmp	sitendlp
sitend20:
	add	sp,4		;collapse the stack
	mov	[bp].srtcmpb,offset compare
	mov	[bp].srtlen,26
	call	isort		;insertion sort, list s/b small
	jnc	sitend30	;if sort failed
	mov	si,offset save3b
	call	freemem
	jmp	sitend05	;..say no memory
sitend30:
	push	ds
	pop	es
	mov	di,offset fragn
	mov	ax,root
	call	bifnxn		;build an NxN key for root 5 or 6
	or	ax,ax		;if all went well
	jz	sitend40	;..build the results
	mov	si,offset msg7	;internal error
	jmp	sitender
sitend40:
	call	relocate	;relocate the keysquare
	or	flag1,02h	;flag we have a keysquare
	mov	si,forbif	;move the crib to the plaintext
	mov	es,[si].cipa
	mov	di,[si].subp
	add	di,pairs.siti
	xor	cx,cx
	mov	cl,tip
	mov	si,offset tip+1
	rep movsb
sitendok:
	push	ds		;do the coordinate substitutions
	mov	si,offset bifa	;..for 1st bifid
	push	si
	les	di,dword ptr [bp].bfkeyb
	push	es
	push	di
	call	cordsub 	;stack cleared on return
	or	ax,ax		;if plaintext-ciphertext conflicts
	jz	sitend50
	mov	si,offset msg8	;..say so
	mov	di,24*160+80
	call	showmsg
	or	flag1,20h	;..msg to clear on next keystroke
sitend50:
	test	flag1,01h	;if there is a 2nd bifid
	jz	sitend60
	push	ds		;do substitutes into 2nd bifid
	mov	si,offset bifb
	push	si
	push	es
	push	di
	call	cordsub
	or	ax,ax		;if any coordinate conflicts detected
	jz	sitend60
	mov	si,offset msg8	;..tell user
	mov	di,24*160+80
	call	showmsg
	or	flag1,20h	;message pending
sitend60:
undoret:
	mov	si,offset bifa	;assume 1st bifid displayed
	test	flag2,02h
	jz	sitend70
	mov	si,offset bifb
sitend70:
	call	showsub 	;show new plaintext
	call	showelg 	;show new substitutes
	call	showkey 	;show current keysquare
	test	flag1,04h	;if we have a permuted keysquare display
	jz	sitend90
	and	flag1,0FFh-04h	;..cancel it because we now have
	mov	es,[bp].videoa	;..a new keysquare for which
	mov	bx,15*160+80	;..this permutation may not be valid
	mov	cx,7
	mov	ax,0720h
sitend80:
	push	cx
	mov	di,bx
	mov	cx,13
	rep stosw
	pop	cx
	add	bx,160
	loop	sitend80
sitend90:
	jmp	forever

;enter in period list
perend:
	xor	ax,ax
	mov	al,perrow
	sub	ax,14
	shl	ax,1
	mov	dx,ax
	shl	ax,1
	add	ax,dx		;6 x current entry
	mov	bx,offset dictab
	add	bx,ax		;->ith entry
	mov	si,forbif2	;->cipher for which this block length pertains
	mov	ax,[bx] 	;period selected
	jmp	bloc30

;enter in key square
keyend:
	mov	keycol,25	;1st column
	jmp	keydown 	;..next row

;change area control
prewind:
	mov	si,offset ftab7
	mov	ax,-2
prev10:
	test	flag1,40h	;disallow area changes
	jz	prev20
	jmp	forever 	;..in command mode
prev20:
	mov	bx,area
	shl	bx,1
	jmp word ptr [si][bx]
nxtwind:
	mov	ax,2
	mov	si,offset ftab8
	jmp	prev10

;go to substitution area, 0
area0:		;change to substitution area
	mov	area,0
	mov	dx,word ptr bifa.ccurs	;assume 1st bifid
	test	flag2,02h
	jz	area010
	mov	dx,word ptr bifb.ccurs	;nope, 2nd bifid wanted
area010:
	call	setcurs
	jmp	forever

;go to site area, 1
area1:		;change to site area
	test	flag2,01h	;can't go to site list
	jnz	area120 	;..unless we have one
area110:
	add	bx,ax		;next area
	and	bx,0007h
	jmp word ptr [si][bx]
area120:
	mov	area,1		;mark site list area
	mov	dx,sitcurs	;..and set cursor
	jmp	area010

;go to period list area, 2
area2:
	test	flag1,08h	;can't go to period list
	jz	area110 	;..if we don't have one
	mov	area,2		;mark period list area
	mov	dx,percurs
	jmp	area010

;go to key area, 3
area3:
	test	flag1,02h	;can't go to key
	jz	area110 	;..unless we have one
	mov	area,3
	mov	dx,keycurs
	jmp	area010

;move cursor right
curite:
	test	flag1,40h
	jnz	cmdrit10
	mov	bx,area 	;cursor to right depending on area
	shl	bx,1
	jmp	ftab1[bx]
cmdrit10:
	cmp	cmdcol,79	;don't cursor off the command line
	jb	cmdrit20
	jmp	forever
cmdrit20:
	inc	cmdcol
	mov	dx,cmdcurs
	call	setcurs
	jmp	forever

;cursor right in substitution area
subrite:
	mov	si,offset bifa	;assume 1st bifid
	test	flag2,02h
	jz	subr10
	mov	si,offset bifb	;..it's 2nd bifid
subr10: 	;ds:si->bif parms
	call	bifrite 	;move cursor right
	call	showelg 	;show eligible substitutes
	call	showpos 	;update location display
	jmp	forever

;cursor right in key area
keyrite:
	xor	ax,ax		;check for right limit
	mov	al,keycol
	sub	ax,25		;origin 0
	shr	ax,1		;..and two positions per coordinate
	inc	ax		;origin 1
	cmp	ax,root
	jb	keyr10
	mov	keycol,25	;at right limit, try going to next row
	jmp	keydown 	;..1st column
keyr10:
	add	keycol,2	;two cursor positions per column
	mov	dx,keycurs
	call	setcurs
	jmp	forever

;cursor left
curleft:
	test	flag1,40h
	jnz	cmdleft
	mov	bx,area 	;cursor left in current area
	shl	bx,1
	jmp	ftab2[bx]
cmdleft:
	cmp	cmdcol,0	;don't cursor off command line
	je	cmdlftxt
	dec	cmdcol
	mov	dx,cmdcurs
	call	setcurs
cmdlftxt:
	jmp	forever

;cursor left in substitution area
subleft:
	mov	si,offset bifa	;assume 1st bifid
	test	flag2,02h
	jz	subl10
	mov	si,offset bifb	;..it's 2nd bifid
subl10:
	call	bifleft 	;move cursor left
	call	showelg 	;show eligible substitutes
	call	showpos 	;update location display
	jmp	forever

;cursor left in key area
keyleft:
	cmp	keycol,25	;see if I can move left
	ja	keyl10
	mov	ax,root 	;..no, go up a row
	dec	ax
	shl	ax,1		;..last column
	add	ax,25
	mov	keycol,al
	jmp	keyup
keyl10:
	sub	keycol,2
	mov	dx,keycurs
	call	setcurs
	jmp	forever

;cursor down
curdn:
	test	flag1,40h	;disallow down cursor
	jz	curdn10
	jmp	forever 	;..in command mode
curdn10:
	mov	bx,area
	shl	bx,1
	jmp	ftab5[bx]

;cursor down in substitution area
subdown:
	mov	si,offset bifa	;assume 1st bifid
	test	flag2,02h
	jz	subdn10
	mov	si,offset bifb	;..correct assumption, it's 2nd bifid
subdn10:
	cmp	[si].rcurs,11	;can I cursor down?
	jb	subdn30 	;..yes
	mov	ax,[si].curln	;else try to scroll down
	add	ax,3		;total lines on one scroll
	cmp	ax,[si].dlines	;if some lines still off screen
	jb	subdn20 	;..we can scroll
	jmp	forever 	;but don't scroll off cipher
subdn20:
	inc	[si].curln	;scroll one line
subdn25:
	call	showcip 	;display scrolled cipher
	call	setblks 	;recompute displayed blocks for these lines
	call	showsub 	;new plaintext
subdn27:
	call	adjcurs 	;adjust cursor to end of line if needed
	call	showelg 	;show eligible substitutes
	call	showpos 	;display new location
	jmp	forever
subdn30:
	xor	ax,ax		;try to move cursor down one line
	mov	al,[si].rcurs
	sub	ax,3		;0, 4, 8
	shr	ax,1
	shr	ax,1		;0, 1, 2
	add	ax,[si].curln	;ax = current cipher line
	inc	ax
	cmp	ax,[si].dlines	;if less than total cipher lines
	jb	subdn40 	;..we can move cursor down 1 line
	jmp	forever 	;else ignore down cursor
subdn40:
	add	[si].rcurs,4	;next cipher line
	jmp	subdn27

;cursor down in site list
sitdown:
	cmp	sitrow,23	;if cursor not at bottom of screen
	jb	sitdn20 	;..try to move it down one row
	mov	ax,sitea	;else scroll the sites if possible
	add	ax,10
	cmp	ax,siten	;don't scroll off the sites
	jb	sitdn10
	jmp	forever 	;at end of site list
sitdn10:
	inc	sitea		;new top line for sites
sitdn12:
	call	showsite	;show scrolled list
sitdn15:
	call	show_cur_site	;hilite current choice
	jmp	forever
sitdn20:
	xor	ax,ax		;get current row
	mov	al,sitrow
	inc	al
	sub	ax,14		;index origin 1
	cmp	ax,siten	;don't cursor off the list
	jb	sitdn30
	jmp	forever
sitdn30:
	inc	sitrow
	jmp	sitdn15
	jmp	forever

;cursor down in period list
perdown:
	cmp	perrow,23
	jb	perdn10
	jmp	forever
perdn10:
	inc	perrow
	call	blkcurs
	mov	dx,percurs
	call	setcurs
	jmp	forever

;cursor down in key
keydown:
	xor	ax,ax		;test if I can cursor down in keysquare
	mov	al,keyrow
	sub	ax,16
	inc	ax
	cmp	ax,root
	jb	keyd20		;..yes
	mov	keyrow,16	;no, wrap to 1st row
keyd10:
	mov	dx,keycurs
	call	setcurs
	jmp	forever
keyd20:
	inc	keyrow
	jmp	keyd10

;cursor up
curup:
	test	flag1,40h	;disallow up cursor
	jz	curup10
	jmp	forever 	;..in command mode
curup10:
	mov	bx,area
	shl	bx,1
	jmp	ftab6[bx]
	jmp	forever

;cursor up in substitution area
subup:
	mov	si,offset bifa	;assume 1st bifid
	test	flag2,02h
	jz	subup05
	mov	si,offset bifb	;..it's 2nd bifid
subup05:
	cmp	[si].rcurs,3	;try to cursor up
	ja	subup20
	cmp	[si].curln,0	;..can't, try to scroll up
	ja	subup10
	jmp	forever 	;..can't scroll up, already at top of cipher
subup10:
	dec	[si].curln	;scroll up one line
	jmp	subdn25 	;..and redisplay cipher, solution, etc.
subup20:
	sub	[si].rcurs,4	;move cursor up one cipher line
	jmp	subdn27

;cursor up in site list
situp:
	cmp	sitrow,14	;try to cursor up
	ja	situp20
	cmp	sitea,0 	;..can't, try to scroll up
	ja	situp10
	jmp	forever 	;..can't scroll, already at list top
situp10:
	dec	sitea		;scroll up
	jmp	sitdn12
situp20:
	dec	sitrow
	jmp	sitdn15

;cursor up in period list
perup:
	cmp	perrow,14
	ja	perup10
	jmp	forever
perup10:
	dec	perrow
	call	blkcurs
	mov	dx,percurs
	call	setcurs
	jmp	forever

;cursor up in key
keyup:
	cmp	keyrow,16
	ja	keyup10
	mov	ax,root
	dec	ax
	add	ax,16
	mov	keyrow,al
	jmp	keyd10
keyup10:
	dec	keyrow
	jmp	keyd10

;home on current line
home:
	test	flag1,40h	;if command mode
	jnz	cmdhome 	;..reset command line pos
	cmp	area,0		;if substitution mode
	je	subhome 	;..reset substitution line pos
	jmp	forever 	;else ignore
subhome:
	mov	si,offset bifa
	test	flag2,02h
	jz	subhom10
	mov	si,offset bifb
subhom10:
	mov	[si].ccurs,0
	mov	dx,word ptr [si].ccurs
	call	setcurs
	call	showpos
	call	showelg
	jmp	forever
cmdhome:
	mov	cmdcol,0
	mov	dx,cmdcurs
	call	setcurs
	jmp	forever

;COMMANDS
;site		site user specified crib
;noj		set default 5x5 English alphabet, no J, Js trapped to Is
;nok		set 5x5 French or Spanish alphabet, no K
;alphabet	set user specified alphabet with or without trap letter
;permute	permute keysquare
;bloc		determine block length

;site command, example:
;   site orderi sbetterth
;find best positions in cipher for "orderi sbetterth"
;note that 1st intervening blank is significant and identifies where tip
;spans ciphertext blocks, thus limiting the number of positions to test.
;if no intervening blank, the tip is dragged through all possible positions.
site:
	push	ds
	push	si
	push	ds
	pop	es
	mov	di,offset tip
	call	getoken 	;get tip or part of tip
	jnc	site00
	jmp	endl20
site00:
	mov	cx,[si].toklen
	mov	[di],cl 	;set tip length so far
	mov	tipblock,cx	;save for figuring if tip is sited at span
	inc	di
	mov	si,[si].tokptr	;fold to upper case
sitelp:
	lodsb
	cmp	al,'a'
	jb	site10
	cmp	al,'z'
	ja	site10
	and	al,0DFh
site10:
	stosb
	loop	sitelp
sitelp2:
	mov	si,offset cmdprm
	push	ds
	push	si
	call	getoken 	;keep getting tip pieces
	jc	site30		;..until out of pieces
	mov	ax,[si].toklen	;accumulate tip length
	mov	cx,ax
	add	al,tip
	mov	tip,al
	mov	si,[si].tokptr
sitelp3:
	lodsb
	cmp	al,'a'
	jb	site20
	cmp	al,'z'
	ja	site20
	and	al,0DFh
site20:
	stosb
	loop	sitelp3
	jmp	sitelp2
site30:
	mov	ax,word ptr j2i ;fix up any j's (or k's)
	xor	cx,cx
	mov	cl,tip
	mov	di,offset tip+1
sitelp4:
	repne scasb
	jne	site40
	dec	di
	inc	cx
	mov	[di],ah
	jmp	sitelp4
site40:
	mov	draginc,1	;assume no block spanning
	mov	draga,0 	;assume start dragging at start of cipher
	mov	si,offset bifa	;allow dragging in either twin
	test	flag2,02h
	jz	site42
	mov	si,offset bifb
site42:
	mov	forbif,si	;save which cipher we are siting
	mov	ax,[si].cnt	;figure number of times to drag
	xor	dx,dx
	mov	dl,tip		;cipher length - tip length + 1
	sub	ax,dx
	inc	ax
	mov	dragn,ax	;..is number of positions to drag
	mov	ax,tipblock	;figure if tip must span blocks
	cmp	al,tip		;if only one operand of site command
	je	site50		;..tip must be dragged thru entire cipher
;the tip may be in the form ORDERISBETTERTH AN with an implied blocking of
;ORDERI SBETTERTH AN, assuming period 9.
	xor	dx,dx		;force tip adjustment to blocking so that
	div	[si].period	;1st part of tip is <= period
	mov	tipblock,dx
	mov	ax,[si].period	;set the increment to the period
	mov	draginc,ax
	sub	ax,tipblock	;figure beginning offset, 1st position
	mov	draga,ax	;..to drag
	mov	dx,ax		;figure number of positions to drag
	add	dl,tip		;expand tip to complete block for 1st part
	mov	ax,[si].cnt	;cipherlength - expanded tip
	sub	ax,dx
	inc	ax		;+ 1 is length to test
	xor	dx,dx
	div	[si].period	;../ by period is number of times to test
	or	dx,dx		;if there is a remainder
	jz	site45
	inc	ax		;..add one more time to test
site45:
	mov	dragn,ax	;number of positions in cipher to drag
;reset to solving start
site50:
	mov	si,offset save2b ;volatile memory anchor
	call	freemem
	mov	fragn,0 	;no fragment list
	mov	siten,0 	;no site list, no matched
	and	flag2,0F2h	;ciphertext, no marks
	and	flag1,0FFh-02h	;no keysquare, we just threw it away
	mov	[bp].biflogn,0	;reset transaction log to start
	xor	ax,ax
	call	reset		;clear any plaintext
	mov	cx,dragn	;get memory for coincidence/site table
	shl	cx,1	;coincidence, offset, strings, longest
	shl	cx,1
	shl	cx,1
	call	getmem
	jnc	site70
site68:
	mov	si,offset msg1	;say not enough memory
siterr:
	mov	[bp].bfkeyb,0	;no key for transaction logging
	mov	[bp].bfkeya,0
	jmp	endl22

;site table entry
;	dw		coincidence score
;	dw		cipher text offset to this site
;	dw		number of equivalent strings
;	dw		longest string
site70:
	mov	sitabb,di	;anchor site table
	mov	sitaba,es
	call	freeadj 	;preserve the site table
	mov	si,offset save3b ;change the frag and key origin
	call	savefree
;construct parameter list for site routines
	mov word ptr pairs.sitfrag,offset collect
	mov word ptr pairs.sitfrag+2,ds ;->frag string destination
	les	di,dword ptr workb
	mov word ptr pairs.sitpair,di
	mov word ptr pairs.sitpair+2,es ;->paired letters destination
	mov	al,' '
	mov	cx,worklen
	rep stosb
	mov word ptr pairs.sitcrib,offset tip+1
	mov word ptr pairs.sitcrib+2,ds ;->tip source
	xor	bx,bx
	mov	bl,tip
	mov	pairs.sitcril,bx	;crib length
	mov	bx,root
	mov	pairs.sitroot,bx	;bifid root, 5 or 6
	mov	si,forbif		;get the cipher parms
	les	di,dword ptr [si].cipb
	mov word ptr pairs.sitcipt,di
	mov word ptr pairs.sitcipt+2,es ;->cipher top line source
	add	di,[si].cnt
	mov word ptr pairs.sitcipb,di
	mov word ptr pairs.sitcipb+2,es ;->cipher bot line source
	mov	bx,[si].period
	mov	pairs.sitper,bx 	;cipher block length
	mov	bx,[si].kurz
	mov	pairs.sitkurz,bx	;short block length
	mov	bx,[si].blocks
	mov	pairs.sitblkn,bx	;number of full blocks
	les	di,dword ptr [bp].freehb
	mov	fragb,di	;initialize fragment table
	mov	fraga,es
	mov	[bp].srtentb,di ;initialize fragment table sorting
	mov	[bp].srtenta,es
	mov	[bp].srtlen,26	;length of a fragment entry
	les	di,dword ptr sitabb	;destination of site table
	mov	cx,dragn	;number of times to site
	mov	si,offset pairs
	push	ds		;->siting parms
	push	si
	mov	bx,draga	;cipher offset at which to site
draglp:
	mov	pairs.siti,bx	;test this site
	call	sitebif 	;..returns coincidences in ax
	jc	dragret
	mov	es:[di],ax	;save coincidences in site table
	mov	es:[di+2],bx	;save site location
	mov word ptr es:[di+4],0 ;clear count of strings
	mov word ptr es:[di+6],0 ;clear length of longest string
	call	sitip		;expand the tip for this site
	add word ptr pairs.sitpair,2 ;prepare for cipher expansion
	call	setcip		;expand the cipher for this site
	sub word ptr pairs.sitpair,2 ;restore tip expansion
draglp2:
	call	sitequ		;get an equal coordinate string
	jc	dragret 	;if conflict, on to next site
	cmp	ax,0		;if null string,
	je	site75		;..no more strings left
	inc word ptr es:[di+4]	;else count number of strings
	cmp	ax,es:[di+6]	;..if this string is longest so far
	jna	site72		;....(it isn't)
	mov	es:[di+6],ax	;....update longest so far
site72:
	call	movfrag 	;add to fragment table
	jnc	draglp2
sitemem:
	add	sp,4		;memory failure, pop stack
sitemem2:
	mov	si,offset save2b
	call	freemem 	;return memory
	mov	si,offset msg1
	jmp	siterr		;say no memory, await keyboard input
dragret:
;throw the fragment table away
	mov	si,offset save3b
	call	freemem
	add	bx,draginc	;next site position
	loop	draglp
	add	sp,4		;collapse the stack
	cmp	siten,0 	;if crib couldn't be sited
	jne	site80
	mov	si,offset msg5	;..tell user
	jmp	siterr
;build the key for further testing at this point
site75:
	mov	ax,es:[di+4]	;number of fragments
	mov	[bp].srtcmpb,offset compare
	mov	[bp].srtn,ax
	mov	fragn,ax
	call	isort
	jc	sitemem 	;memory failure
	push	es
	push	di
	push	ds
	pop	es
	mov	di,offset fragn
	mov	ax,root
	call	bifnxn		;build an NxN key for root 5 or 6
	pop	di
	pop	es
	cmp	ax,-1		;if key is invalid
	je	dragret 	;..throw away this site
	cmp	ax,1		;if no memory
	je	sitemem 	;..say so
	add	di,8		;else good site, next site table position
	inc	siten		;count the valid site
	jmp	dragret
site80: 		;sort the list of sites descending by score
	les	di,dword ptr sitabb	;site table to be sorted
	mov	[bp].srtcmpb,offset compare
	mov	[bp].srtentb,di
	mov	[bp].srtenta,es
	mov	[bp].srtlen,8		;entry length
	mov	ax,siten
	mov	[bp].srtn,ax		;number of items to sort
	call	isort			;insertion sort, list s/b small
	jnc	site90			;if sort failed
	jmp	sitemem2
site90:
	and	flag1,0FFh-40h	    ;leave command mode
	mov	si,offset savebar  ;restore screen line
	mov	es,[bp].videoa
	mov	di,12*160
	mov	cx,80
	rep movsw
	mov	sitea,0 	;1st of n sites
	mov	area,1		;go to area 1, the site table
	mov	sitcurs,0E00h	;site list cursor position, 14,0
	mov	[bp].bfkeyb,0	;no key for transaction logging
	mov	[bp].bfkeya,0
	call	showsite	;show sites, let user pick one
	call	show_cur_site	;mark the current site
	or	flag2,01h	;flag site list up
	jmp	forever

;restore english no j alphabet
noj:
	cmp	root,5		;not valid except for 5x5
	je	noj10
nojer:
	mov	si,offset msg29 ;say not valid
	jmp	endl22
noj10:
	mov	j2i,'J'         ;restore trap
	mov	j2i+1,'I'
	mov	si,offset alf5
noj20:
	push	ds
	pop	es
	mov	di,offset bifalf
	mov	cx,square
	rep movsb
	mov	si,offset save2b	;throw everything away
	call	savefree
	mov	[bp].biflogn,0		;clear the log
	and	flag1,01h		;clear all flags but the twin
	mov	flag2,0
	mov	ax,1		;re-expand and reset
	call	reset
	and	flag1,0FFh-40h	;leave command mode
	mov	si,offset savebar  ;restore screen line
	mov	es,[bp].videoa
	mov	di,12*160
	mov	cx,80
	rep movsw
	jmp	refresh

;set a french or spanish alphabet, no k
nok:
	cmp	root,5		;must be 5x5 bifid
	jne	nojer		;..or it's an error
	mov word ptr j2i,2020h	;clear trap letter
	mov	si,offset alfnok ;point to no k alphabet
	jmp	noj20		;..and join common code

;user defined alphabet, perhaps no z
alphabet:
	push	ds
	push	si		;command parms ptr
	call	getoken 	;get the user alphabet
	jnc	alf10
	jmp	endl20		;..say bad command, missing parm
alf10:
	mov	cx,[si].toklen	;alphabet length must match root*root
	cmp	cx,square
	je	alf20
	mov	si,offset msg17 ;..say invalid length
	jmp	endl22
alf20:
	push	ds
	pop	es
	mov	bx,si		;save token ptr
	mov	si,[si].tokptr
	dec	cx
alflp1:
	push	cx
	lodsb		;check that each char occurs but once
	mov	di,si
	repne scasb
	pop	cx
	jne	alf24
	mov	msg18+27,al	;put duplicated letter into msg
	mov	si,offset msg18 ;say non-unique chars
	jmp	endl22
alf24:
	loop	alflp1
	mov	si,bx		;recover token ptr
	mov	di,offset specalf
	mov	cx,[si].toklen	;get length again
	mov	si,[si].tokptr	;get the alphabet
alflp2:
	lodsb
	cmp	al,'a'
	jb	alf30
	cmp	al,'z'
	ja	alf30
	and	al,0DFh 	;fold to upper case
alf30:
	stosb
	loop	alflp2
	mov	si,bx		;recover token ptr
	push	ds
	push	si
	call	getoken 	;get the trap letters if any
	jnc	alf40
	mov	j2i,' '         ;..no trap letters
	mov	j2i+1,' '
	jmp short alf70
alf40:
	cmp	[si].toklen,2	;if trap letters aren't 2 long
	je	alf50
	mov	si,offset msg19
	jmp	endl22		;..say trap letters not a pair
alf50:
	mov	cx,[si].toklen
	mov	si,[si].tokptr	;get trap letters
	mov	di,offset traplet
alflp3:
	lodsb
	cmp	al,'a'
	jb	alf60
	cmp	al,'z'
	ja	alf60
	and	al,0DFh
alf60:
	stosb
	loop	alflp3
	mov	ax,word ptr traplet
	mov word ptr j2i,ax
alf70:
	mov	si,offset specalf
	jmp	noj20

;permute keysquare
;first collect the desired permutation and validate
permute:
	and	flag1,0FFh-04h		;cancel any permutation
	push	ds
	pop	es
	mov	di,offset permvec
	xor	dx,dx			;token counter
	xor	ax,ax
permlp1:
	push	ds			;->parsing parms
	push	si
	call	getoken 		;getoken clears stack
	jc	perm30		;end of tokens
	cmp	[si].toklen,1		;should be single digit, 1 to root
	je	perm10
permerr:
	mov	si,offset msg30 	;too many digits
	jmp	endl22
perm10:
	mov	bx,[si].tokptr
	mov	al,[bx] 	;get the character
	cmp	al,'1'
	jb	permerr 	;not a number
	sub	al,'1'
	cmp	ax,root
	jnb	permerr 	;out of range
	cmp	dx,root
	jnb	permerr 	;too many digits
	inc	dx		;count the digit
	stosw			;save it
	jmp	permlp1 	;and get more digits
perm30:
	cmp	dx,root 	;must be exactly root digits
	jne	permerr
	mov	si,offset permvec ;must be exactly one of each digit
	mov	cx,dx
	dec	cx
permlp2:
	lodsw			;get digit
	mov	di,si
	push	cx
	repne scasw		;..must be unique
	pop	cx
	je	permerr 	;..duplicated number
	loop	permlp2 	;check all digits
	test	flag1,02h	;if we don't have a keysquare
	jnz	perm40
	jmp	forever 	;..consider it permuted
perm40:
	mov	di,offset permvec
	mov	dl,2		;signal display wanted
	call	bifperm
	or	flag1,04h	;flag outstanding permutation
	jmp	forever

;determine block length (period)
bloc:
	push	ds
	push	si
	call	getoken 	;get block operand
	jnc	bloc10
	mov	si,offset msg4	;missing operand is bad command
	jmp	endl22
bloc10:
	mov	cx,[si].toklen
	mov	si,[si].tokptr
	call	asc2int
	jnc	bloc20
bloc12:
	mov	si,offset msg31 ;invalid block length
	jmp	endl22
bloc20: 			;ax = desired block length
	cmp	ax,1		;is the block length silly?
	jbe	bloc12
	cmp	ax,79		;is it too big?
	ja	bloc12
	mov	si,offset bifa	;assume block length for 1st bifid
	test	flag2,02h
	jz	bloc30
	mov	si,offset bifb
bloc30:
	cmp	ax,[si].cnt	;block length <= cipherlength
	jbe	bloc40
	mov	si,offset msg32 ;block length too big
	jmp	endl22
bloc40:
	mov	[si].period,ax	;new period
	mov	forbif3,si
;reset everything
	mov	si,offset save2b ;return all memory
	call	freemem
	mov	si,offset save3b ;..and reset memory save areas
	call	savefree
	and	flag2,02h	;nothing but 2nd bifid up
	mov	siten,0
	mov	[bp].biflogn,0	;no log
	and	flag1,0C9h	;nothing but twin, period, error, cmd
	mov	[bp].bfkeya,0	;no key ptr
;clean the screen
	mov	ah,13		;new bar
	mov	al,1
	mov	dh,80
	call	hline
	mov	es,[bp].videoa
	mov	di,13*160	;clear below the bar
	mov	cx,80*11
	mov	ax,0720h
	rep	stosw
	mov	cx,root 	;redraw key coordinates
	mov	di,15*160+50	;columns first
	mov	ax,0F31h
bloclp1:
	stosw
	inc	al
	add	di,2
	loop	bloclp1
	mov	cx,root
	mov	di,16*160+46	;then rows
	mov	al,'1'
bloclp2:
	stosw
	inc	al
	add	di,158
	loop	bloclp2
	test	flag1,08h	;if there is a period list
	jz	bloc50
	call	showblk 	;..redisplay it
bloc50:
	mov	si,forbif3
	mov word ptr [si].ccurs,0300h ;reset bifid cursor
	call	expand		;reexpand cipher for this period
	call	bifline 	;recompute blocks per line for this period
	call	setblks 	;set display parms for this period
	xor	ax,ax
	call	reset
	mov	si,forbif3
	call	setnat		;set naturals for this period
	test	flag1,40h	;if not command mode
	jz	bloc70		;..dic already computed and displayed
	push	ds
	pop	es
	mov	di,forbif3
	call	bifdic		;compute digraphic ic for this period
	jnc	bloc60
blocer:
	or	flag1,80h	;fatal no memory error
	or	flager,02h
	mov	si,offset msg1
	jmp	endl22
;convert the dic to ascii
bloc60:
	push	bp
	sub	sp,4		;where to put the result
	mov	bp,sp
	mov word ptr [bp],offset collect
	mov	[bp+2],ds
	mov	ax,4		;four digits wanted
	call	fp2asc
	add	sp,4
	pop	bp
;display the dic
	mov	si,offset collect
	mov	di,24*160+80
	or	flag1,20h
	call	showmsg
bloc70:
	mov	si,forbif3	;redisplay cipher
	call	showcip 	;show cipher with this block
	call	showsub 	;substitutes, just naturals if any
	call	showelg 	;eligible letters
	call	showpos 	;show position
	test	flag1,40h	;if command mode
	jz	bloc80
	mov	dx,word ptr [si].ccurs
	call	setcurs 	;..leave it
	and	flag1,0FFh-40h
bloc80:
	mov	si,offset bifalf
	mov	di,23*160+46	;show unsolved key letters
	mov	ah,07h
	mov	cx,square
bloclp3:
	lodsb
	stosw
	loop	bloclp3
	jmp	forever


;mark matching text, mainly for twins
altm:
	test	flag1,40h	;if command mode
	jz	mark10
	jmp	forever 	;..ignore mark request
mark10:
	cmp	area,0		;allow marks only in sub area
	je	mark20
	jmp	forever
mark20:
	test	flag2,04h	;if mark starting
	jnz	mark30
	push	ds		;..clear mark controls
	pop	es
	mov	di,offset mark1
	mov	cx,type bifmark
	xor	ax,ax
	rep stosw
	or	flag2,04h	;..and start the marks
	mov	markcnt,0
	and	flag2,0FFh-08h	;..and turn off saved marks flag
mark30:
	mov	dx,1
	mov	si,offset bifa	;assume 1st bifid
	test	flag2,02h
	jz	mark40
	mov	dx,2
	mov	si,offset bifb
mark40:
	call	getpos		;bx = offset into cipher
	mov	di,offset mark1 ;assume 1st mark pair
	cmp	markcnt,2	;0,1 1st pair.	2,3 second pair
	jb	mark50
	mov	di,offset mark2 ;..it's 2nd pair
mark50:
	mov	es,[si].cipa	;segment of current cipher
	cmp	[di].markcip,0
	jne	mark60
	mov	[di].markcip,dx 	;flag which cipher marks belong to
	mov	[di].markseg,es 	;..and its segment
mark60:
	cmp	dx,[di].markcip 	;marks must pair in same cipher
	je	mark70
	and	flag2,0FFh-04h	;quit marking
	mov	si,offset msg11 ;say marks must pair in same cipher
	jmp	endl22
mark70:
	test byte ptr markcnt,01h	;if start of pair
	jnz	mark80
	mov	[di].marka,bx		;..use 1st mark position
	inc	markcnt 		;..count the mark
mark72:
	mov	si,offset msg14 	;assume begin 1st pair
	cmp	markcnt,1
	je	mark78
	mov	si,offset msg15 	;assume end 1st pair
	cmp	markcnt,2
	je	mark78
	mov	si,offset msg27 	;must be start of 2nd pair
mark78:
	mov	di,24*160+80	;..but don't erase msg until all marks
	call	showmsg 	;..are entered
	call	showpos 	;..show or clear match distance
	jmp	forever 		;..and await next mark
mark80:
	mov	ax,[di].marka		;else use end of pair and normalize
	cmp	ax,bx			;..so that 1st <= 2nd mark
	jb	mark90
	xchg	ax,bx
mark90:
	mov	[di].marka,ax
	mov	[di].markz,bx
	inc	markcnt
	call	showpos
	cmp	markcnt,4	;if this is not the last mark
	jne	mark72		;..figure out which msg to display
;else make sure marks agree in length
	and	flag2,0FFh-04h	;end of marking
	mov	es,[bp].videoa	;..remove marks pending message
	mov	di,24*160+80
	mov	ax,0F20h
	mov	cx,40
	rep stosw
	mov	ax,mark1.markz
	sub	ax,mark1.marka	;length of first mark
	mov	dx,mark2.markz
	sub	dx,mark2.marka
	cmp	ax,dx
	je	mark110
	mov	si,offset msg12
	jmp	endl22		;say mark lengths disagree
mark110:
	inc	ax		;number of bytes marked
	cmp	ax,75		;don't exceed max marked text
	jna	mark112
	mov	si,offset msg16 ;say matched text chopped to 75
	mov	di,24*160+80
	call	showmsg
	or	flag1,20h
	mov	ax,75		;..and chop to 75
mark112:
	mov	marklen,ax
	mov	si,offset save2b
	call	freemem
	and	flag2,0FFh-01h	;no site list
	mov	siten,0
	xor	ax,ax		;don't reexpand
	call	reset		;but otherwise reset
	mov	[bp].biflogn,0	;reset the log
;get enough memory for for matched text save area
	mov	ax,marklen
	mov	cx,3
	shl	ax,cl		;matched length x 8 is expansion needs
	mov	cx,ax
	call	getmem
	jnc	mark120
	mov	si,offset msg1	;say not enough memory
	jmp	endl22
mark120:
	mov	marksvb,di	;matched text save area anchor
	mov	marksva,es
	call	freeadj 	;segment boundary
	mov	si,offset save3b ;save frag list, key area start
	call	savefree
;set up siting structure to expand 1st cipher matched text
	les	di,dword ptr workb
	mov word ptr pairs.sitpair,di	;paired list destination
	mov word ptr pairs.sitpair+2,es
	mov	cx,worklen
	mov	al,' '                  ;..and clear paired list area
	rep stosb
	mov	ax,marklen		;length of mark
	mov	pairs.sitcril,ax
	mov	bx,offset mark1 	;load 1st cipher's parms
	mov	si,offset bifa		;into site parms
	cmp	[bx].markcip,1
	je	mark130
	mov	si,offset bifb
mark130:
	call	markset 	;set siting parms for this bifid
	push	ds		;setcip parms ptr
	mov	ax,offset pairs
	push	ax
	call	setcip		;dimension 1st marked bifid
	add word ptr pairs.sitpair,2	;2nd bifid paired list destination
	mov	bx,offset mark2
	mov	si,offset bifb
	cmp	[bx].markcip,2
	je	mark140
	mov	si,offset bifa
mark140:
	call	markset 	;set siting parms for this bifid
	call	setcip		;dimension 2nd marked bifid
	les	di,dword ptr marksvb	;now save the expanded matched text
	mov	cx,marklen
	shl	cx,1		;number of words to save
	shl	cx,1
	push	ds
	lds	si,dword ptr workb
	rep movsw
	pop	ds
;complete parm list for making fragment strings, setequ
	mov word ptr pairs.sitfrag,offset collect
	mov word ptr pairs.sitfrag+2,ds
	mov	ax,root
	mov	pairs.sitroot,ax
	sub word ptr pairs.sitpair,2	;correct paired list location
	les	di,dword ptr [bp].freehb  ;initialize fragment list location
	mov	fragb,di
	mov	fraga,es
	mov	fragn,0        ;number of fragments
marklp:
	call	sitequ		;get a fragment
	jnc	mark150
	add	sp,4		;whoops, matched text doesn't match
	mov	si,offset msg13 ;say matched text error
markquit:
	push	si		;save msg ptr
	mov	si,offset save2b ;reset memory
	call	freemem
	pop	si
	jmp	endl22		;report error
mark150:
	cmp	ax,0		;if fragment is null
	je	mark160 	;..quit fragment collection
	call	movfrag 	;else move fragment to fragment table
	jc	mark168
	inc	fragn		;..count the fragment
	jmp	marklp	;do until no more fragments are found
mark160:
	add	sp,4		;done, collapse parm stack
;sort the fragment table by length of fragments
	mov	[bp].srtcmpb,offset compare
	les	di,dword ptr fragb	;site table to be sorted
	mov	[bp].srtentb,di
	mov	[bp].srtenta,es
	mov	[bp].srtlen,26		;entry length
	mov	ax,fragn
	mov	[bp].srtn,ax		;number of items to sort
	mov	[bp].srtdir,1		;descending sort
	call	isort			;insertion sort, list s/b small
	jnc	mark170
mark168:
	mov	si,offset msg1	;say no memory
	jmp	markquit	;free storage
;build a new keysquare
mark170:
	push	ds
	pop	es
	mov	di,offset fragn
	mov	ax,root
	call	bifnxn		;build an NxN key for root 5 or 6
	or	ax,ax		;if all went well
	jz	mark180 	;..relocate keysquare
	cmp	ax,1		;if memory failure
	je	mark168 	;..say so
	mov	si,offset msg13 ;else
	jmp	markquit	;..say invalid substitution
mark180:
	or	flag1,02h	;we have a key
	or	flag2,08h	;we have saved marked text
	call	relocate	;relocate key to compact memory
	jmp	sitendok	;build and show new plaintext

;undo last change
altu:
	cmp	[bp].biflogn,0	;if no prior changes
	jne	undo10
	mov	si,offset msg24 ;..say nothing to undo
	jmp	endl22
undo10:
	call	popbif		;restore previous state
	test	flag2,80h	;if log is broken
	jz	undo20
	jmp	forever 	;..ignore request
undo20:
	jmp	undoret 	;else redisplay new current state

;toggle between 1st and 2nd bifids
altt:
	test	flag1,01h	;if not a twin
	jnz	altt10
	jmp	forever 	;..nothing to toggle
altt10:
	mov	es,[bp].videoa	;assume going to 2
	mov	di,24*160+54
	mov byte ptr es:[di],'2'
	mov	si,offset bifb	;assume going to 2nd bifid
	xor	flag2,02h	;toggle the bifids
	test	flag2,02h
	jnz	altt20		;display 2nd bifid
	mov	si,offset bifa	;display 1st bifid
	mov byte ptr es:[di],'1'
altt20:
	call	showcip 	;display now current bifid
	call	setblks
	call	showsub 	;..its plaintext
	call	showelg 	;..eligible letters at current pos
	call	showpos 	;..current pos
	cmp	area,0		;if not in substitution area
	je	altt30
	jmp	forever 	;..leave cursor where it is
altt30:
	mov	dx,word ptr [si].ccurs
	call	setcurs 	;..else change cursor to sub
	jmp	forever

;change key to current permutation
altp:
	test	flag1,04h	;if we don't have a permutation
	jnz	altp10
	jmp	forever 	;..we can't permute the key
altp10:
	and	flag1,0FFh-04h	;turn off permutation, don't want P(P(x))
	push	ds		;->permutation vector
	pop	es
	mov	di,offset permvec
	call	bifix		;move permutation to key
	test	flag1,40h	;if command mode
	jz	altp20
	and	flag1,0FFh-40h	;..leave command mode
	mov	si,offset savebar  ;restore screen line
	mov	es,[bp].videoa
	mov	di,12*160
	mov	cx,80
	rep movsw
	mov	dx,savecurs	;restore cursor
	call	setcurs
altp20:
	jmp	sitendok

;compute dics for the block length table
altb:
	and	flag1,0FFh-08h	;throw away old period list
	push	ds
	pop	es
	mov	di,offset dictab	;restore block lengths
	mov	ax,5
	mov	cx,10
altblp:
	stosw
	inc	ax
	add	di,4
	loop	altblp
	mov	si,offset bifa
	test	flag2,02h
	jz	altb10
	mov	si,offset bifb
altb10:
	mov	forbif2,si
	mov	ax,[si].period	;save existing period
	mov	oldblk,ax	;..to restore until period is chosen
	mov	di,offset dictab
	mov	cx,10
altblp2:
	mov	ax,es:[di]	;get trial period
	mov	[si].period,ax
	call	expand
	call	bifline
	call	setblks
	push	di
	mov	di,si
	call	bifdic		;compute dic for this block length
	pop	di
	jnc	altb20
	jmp	blocer		;fatal no memory error
altb20:
	fstp dword ptr es:[di+2]
	add	di,6		;next table entry
	loop	altblp2 	;do for all table entries
	mov	ax,oldblk	;restore cipher
	mov	[si].period,ax
	call	expand
	call	bifline
	call	setblks
	mov	[bp].srtcmpb,offset comp2
	mov	[bp].srtentb,offset dictab
	mov	[bp].srtenta,ds
	mov	[bp].srtlen,6
	mov	[bp].srtn,10
	call	isort
	jnc	altb30
	mov	si,offset msg1	;insufficient memory to sort
	jmp	endl22
altb30:
	or	flag1,08h	;flag we have a period list
	mov	percurs,0E0Bh	;reset period list cursor
	call	showblk
	mov	area,2
	mov	dx,percurs
	call	setcurs
	jmp	forever

;log solution
f7:
	mov	ax,3		;log bifid
	call	log
	jnc	f710
	or	flag1,20h	;flag message to clear
f710:
	jmp	forever

;help for bifid
sf1:
	mov	ax,6		;help screen number
	call	help
	jmp	forever

del:		;current line delete
null:
backtab:
altq:
altw:
alte:
altr:
alty:
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
altn:
f1:
f3:
f4:
f8:
f9:
f10:
endkey:
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
pagup:
pagdn:
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
bifid	endp

;get 5n memory for this bifid
;si->bifid parms
get5n	proc	near
	mov	ax,[si].cnt	;get memory for 1st bifid areas
	mov	cx,ax		;..5N bytes
	shl	ax,1		;..2N for dimensioned cipher
	mov	bx,ax		;..save 2N for later
	shl	ax,1		;..2N for coordinates
	add	cx,ax		;..1N for plaintext
	call	getmem
	jnc	get5n10
	ret
get5n10:
	mov	[si].cipa,es	;anchor dimensioned cipher
	mov	[si].cipb,di
	add	di,bx		;..+ 2N
	mov	[si].coord1,di	;anchor coordinate top line
	add	di,[si].cnt	;..+ 1N
	add	di,[si].cnt	;..+ 1N
	mov	[si].subp,di	;anchor plaintext
	mov	al,' '          ;clear plaintext area
	mov	cx,[si].cnt
	rep stosb
	clc
	ret
get5n	endp

;expand a bifid by doubling its letters to show row,col equiv.
;worka:workb is expansion area
;ds:si->bifid parms for this bifid
;also, provides correct letter count per current alphabet
expand	proc	near
	push	cx
	push	es
	push	di
	push	si
	push	dx
	push	ds
	push	si
	les	di,dword ptr workb	;destination for now
	xor	dx,dx		;valid letter counter
	mov	cx,[si].ln	 ;number of lines in unformatted cipher
	mov	bx,[si].srcptr
	mov	ds,[bp].sbuffa	;ds:bx->unformatted cipher lines
exp20:
	push	cx		;save number of lines
	xor	cx,cx
	mov	cl,[bx] 	;length of this cipher line
	jcxz	exp40		;..empty line
	mov	si,bx
	inc	si		;ds:si->this cipher line
explp1:
	lodsb
	push	ds		;re-establish addressability
	push	dx		;..to check this letter
	mov	dx,crypt25b
	mov	ds,dx
	pop	dx
	cmp	square,25	;if 25 letter alphabet
	jne	exp30
	cmp	al,j2i		;trap Js or whatever
	jne	exp30
	mov	al,j2i+1	;replace J with I or whatever
exp30:
	push	cx		;verify the letter
	push	es
	push	di
	push	ds
	pop	es
	mov	di,offset bifalf
	mov	cx,square
	repne scasb
	pop	di
	pop	es
	pop	cx
	pop	ds
	jne	exp40		;not a cipher letter
	mov	ah,al		;double the letter
	stosw			;char to work area
	inc	dx		;count valid letter
exp40:
	loop	explp1		;for all chars this line
	pop	cx		;recover cipher lines
	add	bx,81		;next line
	loop	exp20		;for all lines, this bifid
	pop	si
	pop	ds		;recover addressability
	mov	[si].cnt,dx	;update letter count
	mov	ax,dx		 ;length of 1st bifid
	xor	dx,dx
	div	[si].period
	mov	[si].blocks,ax	    ;number of complete blocks, 1st bifid
	mov	[si].kurz,dx	   ;length of last block, if 0, none
;move expanded letters into block form
;TTGGYYWWIIOOHHTTTT HHBBPPGGIIOOCCUUSS ... becomes
;TTGGYYWWI HHBBPPGGI
;IOOHHTTTT IOOCCUUSS without interblock spaces, however.
	push	ds
	mov	cx,[si].blocks	    ;number of blocks
	les	di,dword ptr [si].cipb	      ;es:di->destination
	mov	bx,[si].period	    ;block length
	mov	dx,[si].kurz	    ;length of last block
	lds	si,dword ptr workb  ;doubled letter source
	mov	ax,si
	add	ax,bx
	push	ax		;save pt column letter ptr
	push	cx		;save number of blocks
;move the plaintext row coordinates
explp2:
	push	cx
	mov	cx,bx		;length of block
explp3:
	lodsb			;ith letter of this block
	stosb			;ith letter to row line
	loop	explp3		;do for all letters of this block
	pop	cx
	add	si,bx		;skip to start of next block
	loop	explp2		;do for all complete blocks
;finish up with short block
	mov	cx,dx
	jcxz	exp70		;no short block
explp4:
	lodsb
	stosb
	loop	explp4
;move the column coordinates
exp70:
	pop	cx		;recover number of blocks
	pop	si		;recover ds:si->column source
explp5:
	push	cx
	mov	cx,bx
explp6:
	lodsb			;move the columns
	stosb
	loop	explp6
	pop	cx
	add	si,bx		;skip row letters
	loop	explp5		;do for all full blocks
	mov	cx,dx		;complete short last block
	jcxz	expxit
	sub	si,bx		;correct column letters overshoot
	add	si,dx		;ds:si->short block columns
explp7:
	lodsb
	stosb
	loop	explp7
expxit:
	pop	ds
	pop	dx
	pop	si
	pop	di
	pop	es
	pop	cx
	ret
expand	endp

;set how many regular blocks per displayed line, and whether or not a line
;is terminated with a short block
;ds:si->bifid parameter block
setblks proc	near
	push	bx
	push	di
	push	cx
	mov	[si].l1full,0	;clear all line block counts
	mov	[si].l1kurz,0	;..and no short block
	mov	[si].l2full,0	;ditto for 2nd line
	mov	[si].l2kurz,0
	mov	[si].l3full,0	;ditto for 3rd line
	mov	[si].l3kurz,0
	mov	ax,[si].bperl
	mul	[si].curln	;ax = number of blocks off top of screen
	mov	bx,[si].blocks
	sub	bx,ax
	mov	ax,bx		;number of remaining blocks we can display
	div	[si].bperl	;ax = number of lines, dx = number of leftover
	mov	cx,ax		;..blocks
	mov	ax,[si].bperl
	xor	bx,bx
	jcxz	setblk20	;not even the 1st line is full
	cmp	cx,3
	jb	setblk10
	mov	cx,3		;and no more than three lines
setblk10:
	push	cx
setblklp:		;fill in up to three full blocks
	mov	[si][bx].l1full,ax
	add	bx,4
	loop	setblklp
	pop	cx
	cmp	cx,3		;if 3 full blocks filled in
	je	setblkxt	;..there can't be anything else
setblk20:
	mov	di,[si].kurz
;for 0, 1, or 2 lines, we can have one more line with some or zero full blocks
	mov	[si][bx].l1full,dx	;number of blocks last line if any
	mov	[si][bx].l1kurz,di	;..and short block if any
setblkxt:
	pop	cx
	pop	di
	pop	bx
	ret
setblks endp

;set full naturals if any
;ds:si->bifid parameters
setnat	proc	near
	push	ds
	push	si
	push	dx
	les	di,dword ptr [si].cipb	;es:di->ciphertext top row
	mov	bx,di
	add	bx,[si].cnt	;es:bx->ciphertext bot row
	mov	cx,[si].blocks	;number of full blocks
	mov	dx,[si].period	;block length
	mov	ax,[si].kurz	;last short block length
	push	ax
	mov	si,[si].subp	;plaintext string
	push	es
	pop	ds
setnatlp:
	call	testnat
	loop	setnatlp
	pop	dx
	call	testnat
	pop	dx
	pop	si
	pop	ds
	ret
setnat	endp

;test this block for naturals if dx is odd
;dx = block length
;source, es:di->plaintext row
;	 es:bx->plaintext col
;dest, ds:si->substituted string
testnat proc	near
	test	dl,01h		;if even, no full naturals are possible
	jnz	tstnat10
	add	di,dx		;next block, rows
	add	bx,dx		;next block, cols
	add	si,dx		;next block, substitutes
	ret
tstnat10:		;odd block length,
	push	cx	;if rowletter = columnletter, then substitute letter
	mov	cx,dx
	shr	cx,1
	inc	cx
tstnatlp:
	mov	al,es:[di]	;get a row letter
	cmp	al,es:[bx]	;if same as corresponding column letter
	jne	tstnat20
	mov	[si],al 	;..substitute letter is a full natural
tstnat20:
	add	di,2		;next row,col pair (skip col,row pair)
	add	bx,2
	add	si,2
	loop	tstnatlp
	dec	di		;overshot by 1 because of block break
	dec	bx
	dec	si
	pop	cx
	ret
testnat endp

;show bifid
;si->bifid parms
showcip proc	near
	push	ds
	push	si
	mov	es,[bp].videoa		;clear substitution area
	mov	di,160
	mov	cx,11*80
	mov	ax,0720h
	rep stosw
	and	flag1,0EFh		;clear short block flag
	mov	ax,[si].curln		;figure start of source
	mul	[si].bperl		;..top line x blocks per line
	mov	cx,[si].blocks		;total blocks
	sub	cx,ax			;..less blocks shown is remaining
	mul	[si].period		;..x chars per block
	add	ax,[si].cipb
	mov	[si].dsptr1,ax		;1st half 1st char to show
	add	ax,[si].cnt
	mov	[si].dsptr2,ax		;2nd half 1st char to show
	mov	ax,[si].bperl
	mov	dx,ax
	shl	ax,1			;three lines worth of blocks
	add	ax,dx
	cmp	ax,cx
	jbe	shwcip10
	or	flag1,10h		;flag show short block
	jmp short shwcip20
shwcip10:
	mov	cx,ax			;number of blocks to show
shwcip20:
	push	si
	mov	ax,[si].period		;length of a single block
	mov	[bp].tempcnt,ax
	mov	bx,[si].dsptr2		;->ct bottom letters
	mov	ax,[si].cipa
	mov	si,[si].dsptr1		;->ct top letters
	push	ds
	mov	ds,ax			;ds:si->plaintext row columns
	mov	di,160			;screen destination
	mov	ah,07h
shwcipl1:
	jcxz	shwcip40
	cmp	cx,dx
	jae	shwcip30
	mov	dx,cx		;less than full line of blocks to show
shwcip30:
	call	showdx		;show dx worth of blocks
	sub	cx,dx		;next bunch of blocks
	add	di,4*160	;next screen line
	jmp	shwcipl1
shwcip40:
	pop	ds
	mov	cx,si		;funny place to save si
	pop	si		;recover parms
	test	flag1,10h	;if short block to be displayed
	jz	shwcipxt
	cmp	[si].kurz,0	;..and if there is a short block
	je	shwcipxt
	cmp	dx,[si].bperl	;if the last line was full
	jnb	shwcip50	;..leave screen ptr at next line
	mov	di,[bp].templen ;..else restore last line's hi water ptr
shwcip50:			;..because short block will fit on that line
	mov	dx,1		;show one block
	mov	ax,[si].kurz
	mov	[bp].tempcnt,ax ;..of kurz length
	mov	ax,[si].cipa
	mov	si,cx		;restore source letters ptr
	mov	ds,ax
	call	showdx
shwcipxt:
	pop	si
	pop	ds
	ret
showcip endp

;show one line's worth of blocks
showdx	proc	near
	push	di
	push	cx
	mov	ah,07h
	mov	cx,dx
shwdxl1:
	push	cx
	mov	cx,[bp].tempcnt
shwdxl2:
	mov	al,[si]
	mov	es:[di],ax		;row letter
	mov	al,[bx]
	mov	es:[di+160],ax		;column letter
	inc	si
	inc	bx
	add	di,2
	loop	shwdxl2
	mov	al,' '
	stosw
	pop	cx
	loop	shwdxl1
	mov	[bp].templen,di 	;save this line's hi water
	pop	cx
	pop	di
	ret
showdx	endp

;show plaintext
;ds:si->bifid parameters
showsub proc	near
	mov	ax,[si].curln	;get starting character index
	mul	[si].bperl
	mul	[si].period
	add	ax,[si].subp
	mov	bx,ax		;cipa:bx->plaintext start
	mov	es,[bp].videoa
	mov	di,3*160	;screen destination, 1st line
	mov	dx,[si].period	;dx = block length
	mov	cx,[si].l1full	;full blocks, 1st line
	jcxz	shwsub10	;no full blocks
;show full blocks if any line 1
shwsubl1:
	call	showpt		;show dx plaintext
	loop	shwsubl1	;bx,di	updated by showpt
;show short block if any line 1
shwsub10:
	cmp	[si].l1kurz,0	;if short block
	je	shwsub20
	mov	dx,[si].l1kurz	;..display its plaintext
	call	showpt
	ret
;show full blocks if any line 2
shwsub20:
	mov	dx,[si].period
	mov	di,7*160
	mov	cx,[si].l2full
	jcxz	shwsub30
shwsubl2:
	call	showpt
	loop	shwsubl2
;show short block if any line 2
shwsub30:
	cmp	[si].l2kurz,0
	je	shwsub40
	mov	dx,[si].l2kurz
	call	showpt
	ret
;show full blocks if any line 3
shwsub40:
	mov	dx,[si].period
	mov	di,11*160
	mov	cx,[si].l3full
	jcxz	shwsub50
shwsubl3:
	call	showpt
	loop	shwsubl3
;show short block if any line 3
shwsub50:
	cmp	[si].l3kurz,0
	je	shwsub60
	mov	dx,[si].l3kurz
	call	showpt
shwsub60:
	ret
showsub endp

;show plaintext per block
;cipa:bx -> current plaintext ptr
;es:di->screen destination
;dx = length of block
showpt	proc	near
	push	ds
	push	si
	push	cx
	lds	si,dword ptr [si].cipb
	mov	si,bx
	mov	cx,dx
	mov	ah,07h
shwptlp:
	lodsb
	cmp	al,' '
	jne	shwpt10
	mov	al,248		;replace unsolved letters with little circle
shwpt10:
	stosw
	loop	shwptlp
	add	di,2		;block gap
	mov	bx,si		;update current plaintext ptr
	pop	cx
	pop	si
	pop	ds
	ret
showpt	endp

;show position in either 1st or 2nd bifid
showpos proc	near
	push	bx
	push	cx
	push	si
	mov	si,offset bifa	;assume 1st bifid
	test	flag2,02h
	jz	shwpos10
	mov	si,offset bifb
shwpos10:
	call	getpos		;get offset into bifid in bx
	mov	ax,bx
	inc	ax		;index origin 1
	mov	si,offset toasc
	call	int2asc 	;convert to ascii
	mov	si,offset toasc
	mov	cx,5
	mov	es,[bp].videoa
	mov	di,12*160+150
	mov	ah,07h
shwposlp:
	lodsb
	stosw
	loop	shwposlp
	test	flag2,04h	;if a mark is pending
	jz	shwposxt
	test byte ptr markcnt,01h ;..and it's complete
	jnz	shwpos15
	mov	ax,0D3Ch	;..(row 13, col 60)
	mov	dh,14		;..(length 14)
	call	hline		;..restore the bar, removing distance
	jmp short shwposxt	;..and leave
shwpos15:			;else show distance
	test	flag2,02h
	jnz	shwpos20
;1st bifid on the screen, find its mark
	mov	si,offset mark1 ;assume 1st mark parms for 1st cipher
	cmp	[si].markcip,1
	je	shwpos30	;..it is
	mov	si,offset mark2 ;2nd parms for 1st cipher
	jmp short shwpos30
shwpos20:
	mov	si,offset mark2 ;assume 2nd parms for 2nd cipher
	cmp	[si].markcip,2
	je	shwpos30	;..it is
	mov	si,offset mark1 ;1st parms for 2nd cipher
shwpos30:
	mov	ax,[si].marka	;get start of mark
	cmp	ax,bx		;subtract smaller from larger
	jae	shwpos40	;current position <= start of mark
	xchg	ax,bx		;current position > start of mark
shwpos40:
	sub	ax,bx		;distance from start to current pos
	inc	ax
	mov	si,offset msg28+10
	call	int2asc
	mov	cl,msg28
	mov	si,offset msg28+1
	mov	di,12*160+118
	mov	ah,07h
shwposl2:
	lodsb
	stosw
	loop	shwposl2
shwposxt:
	pop	si
	pop	cx
	pop	bx
	ret
showpos endp

;get current position for si->bifparms
;return offset in bx, 0 to cnt-1
getpos	proc	near
	push	dx
	xor	ax,ax
	mov	al,[si].rcurs		;get current row
	sub	ax,3			;..less offset to 1st row
	shr	ax,1			;..4 rows per line
	shr	ax,1
getpos10:
	add	ax,[si].curln		;..plus lines from start
	mul	[si].bperl		;..x number of blocks per line
	mul	[si].period		;..x chars per block
	mov	bx,ax			;save row offset for later
	mov	ax,[si].period		;chars per block
	inc	ax			;..plus interblock space
	mov	[bp].tempcnt,ax
	xor	ax,ax
	mov	al,[si].ccurs		;get cursor column
	div	[bp].tempcnt
	add	bx,dx			;add position within block
	mul	[si].period		;offset to block, this line
	add	bx,ax			;add bytes/block this line
	pop	dx
	ret
getpos	endp

;show eligible substitutions for this position
;si->current bifid
showelg proc	near
	push	si
	mov	es,[bp].videoa	;clear eligible letters
	mov	di,13*160+46
	mov	ax,0720h
	mov	cx,36
	rep stosw
	call	getpos		;get position in bx
	mov	es,[si].cipa
	mov	di,[si].subp
	cmp byte ptr es:[di][bx],' '    ;if letter is solved
	je	shwelg10
	jmp	shwelgxt		;..there are no eligible substitutes
shwelg10:
	push	si
	push	es
	push	ds
	pop	es
	mov	di,offset eligible	;copy the current alphabet
	mov	si,offset bifalf
	mov	cx,36
	rep movsb
	pop	es
	pop	si
	test	flag1,02h	;if we don't have a key square
	jnz	shwelg20
shwelg15:
	mov	si,offset eligible ;..all bifid alphabet is eligible
	mov	cx,square
	mov	di,13*160+46
	mov	es,[bp].videoa
	mov	ah,07h
shwelgl1:
	lodsb
	stosw
	loop	shwelgl1
shwelgxt:
	pop	si
	ret
;figure out the eligible letters from the keysquare
shwelg20:
	mov	di,[si].coord1	;get top coordinate at position bx
	mov	al,es:[di][bx]
	add	di,[si].cnt	;get bottom coordinate at position bx
	mov	ah,es:[di][bx]	;ah = col coordinate, al = row coordinate
	les	bx,dword ptr [bp].bfkeyb
	mov	di,es:[bx].bifrow	;assume active row coordinate
	cmp	al,0FFh
	je	shwelg30
	call	cull			;cull incompatible row letters
shwelg30:
	mov	di,es:[bx].bifcol	;assume active column coordinate
	cmp	ah,0FFh
	je	shwelg15
	mov	al,ah
	call	cull
	jmp	shwelg15
showelg endp

;es:bx->key parms
;es:di->row or column letters
;al = coordinate
cull	proc	near
	push	es
	push	ax
	push	bx
	push	di
	xor	ah,ah
	mov	cullcord,ax
	mov	culldim,di
	mov	di,es:[bx].bifit	;es:di-> fit table
	cmp	ax,es:[bx].bifgrund
	jnb	cullpale
;root coordinate, must scan entire fit table for this coord
;every fit table entry found, eliminate corresponding key entry's letters
;from eligible
	mov	ax,es:[bx].bifmaxn	;scan active fit table for this
	mul	es:[bx].bifgrund	;..root coordinate
	mov	cx,ax		;number of bytes to scan for this coord
cullrlp:
	jcxz	cullxit
	mov	ax,cullcord
	repne scasb
	jne	cullxit
	mov	ax,di
	dec	ax
	sub	ax,es:[bx].bifit
	call	cullet			;cull letters found in ith entry
	jmp	cullrlp ;look for more fit table entries with this coord
cullxit:
	pop	di
	pop	bx
	pop	ax
	pop	es
	ret
;beyond the pale coordinate.  limit search to root fit table
cullpale:
	mov	ax,es:[bx].bifgrund	;scan root part only of fit table
	mul	es:[bx].bifgrund	;..for this pale coordinate
	mov	cx,ax
	jmp	cullrlp
cull	endp

;ax = index of ith key table entry
cullet	proc	near
	push	cx
	push	di
	xor	dx,dx
	div	es:[bx].bifgrund	;ax = ith fit table entry
	mul	es:[bx].bifside 	;ax = ith key table entry
	mov	di,culldim	;row or col key letters
	add	di,ax		;es:di->key ith
	mov	cx,es:[bx].bifmaxn	;scan ith key entry for letters
culletlp:
	jcxz	culletxt	;end of this entry
	mov	al,' '          ;look for non-blanks
	repe scasb
	je	culletxt	;no more letters found
	push	es
	push	di
	push	cx
	mov	al,es:[di-1]	;cross this letter off
	push	ds
	pop	es
	mov	di,offset eligible
	mov	cx,square
	repne scasb
	jne	cullet10
	mov byte ptr es:[di-1],' '
cullet10:
	pop	cx
	pop	di
	pop	es
	jmp	culletlp	;find another letter to cross off
culletxt:
	pop	di
	pop	cx
	ret
cullet	endp

;show command line
showcmd proc	near
	mov	si,offset cmdbuf
	mov	di,12*160
	mov	es,[bp].videoa
	mov	ah,70h		;reverse video
	mov	cx,80
shwcmdlp:
	lodsb
	stosw
	loop	shwcmdlp
	mov	dx,cmdcurs	;set cursor for commands
	call	setcurs
	ret
showcmd endp

;move fragment from collect to fragment table
;ax = number of equivalent coordinate letters
movfrag proc	near
	push	cx
	push	di
	push	es
	mov	cx,26		;get fragment table entry
	call	getmem
	jc	movfrgxt	;no memory
movfrg10:
	push	di
	mov	si,offset collect
	mov	cx,24
	add	di,2
	rep movsb
	pop	di
	mov	es:[di],ax
	clc
movfrgxt:
	pop	es
	pop	di
	pop	cx
	ret
movfrag endp

;show site list, one or more sites that fit crib
showsite proc	near
	mov	es,[bp].videoa
	mov	di,13*160	;show site title
	mov	si,offset sititle+1
	xor	cx,cx
	mov	cl,sititle
	mov	ah,07h
shwsitl1:			;display site title
	lodsb
	stosw
	loop	shwsitl1
	mov	bx,14*160	;screen destination of list
	mov	cx,siten	;number of sites
	mov	ax,sitea
	sub	cx,ax		;..to show on screen
	cmp	cx,10		;..but no more than 10
	jbe	shwsit10
	mov	cx,10
shwsit10:
	push	ds
	lds	si,dword ptr sitabb
	shl	ax,1
	shl	ax,1
	shl	ax,1
	add	si,ax	;current top of list site entry
	push	cx
shwsitlp:
	mov	di,bx
	mov	ax,[si] 	;get the coincidence score
	push	ds
	push	si
	mov	dx,crypt25b
	mov	ds,dx
	mov	si,offset toasc
	call	int2asc 	;convert to ascii
	add	si,3
	push	cx
	mov	cx,2
	mov	ah,07h
shwsitl2:
	lodsb
	stosw
	loop	shwsitl2
	pop	cx
	pop	si
	pop	ds
	mov	ax,[si+2]	;get the ciphertext offset
	inc	ax
	push	ds
	push	si
	mov	dx,crypt25b
	mov	ds,dx
	mov	si,offset toasc
	call	int2asc 	;convert to ascii
	add	si,2
	push	cx
	mov	cx,3
	mov	ah,07h
	add	di,8
shwsitl3:
	lodsb
	stosw
	loop	shwsitl3
	pop	cx
	pop	si
	pop	ds
	add	bx,160
	add	si,8
	loop	shwsitlp
	pop	ax		;recover number of sites shown
	pop	ds		;recover addressability
	mov	cx,10
	sub	cx,ax		;if full 10 lines not shown
	jcxz	shwsit20
	mov	ax,0720h	;..clear remainder of list area
shwsitl4:
	mov	di,bx
	push	cx
	mov	cx,9
	rep stosw
	pop	cx
	add	bx,160
	loop	shwsitl4
shwsit20:
	ret
showsite endp

;mark current site, update x of n on bar line
show_cur_site proc near
	mov	cx,siten	;highlight one of up to 10 sites
	sub	cx,sitea
	cmp	cx,10
	jbe	shwcs10
	mov	cx,10
shwcs10:
	mov	es,[bp].videoa	;clear highlighting
	mov	bx,14*160
	mov	al,07h
shwcsl1:
	push	cx
	mov	cx,9
	mov	di,bx
shwcsl2:
	mov	es:[di+1],al
	add	di,2
	loop	shwcsl2
	pop	cx
	add	bx,160
	loop	shwcsl1
;highlight the current choice
	xor	ax,ax
	mov	al,sitrow
	mul	c160
	mov	di,ax		;es:di->current site choice
	mov	cx,9
	mov	al,0Fh		;bright
shwcsl3:
	mov	es:[di+1],al
	add	di,2
	loop	shwcsl3
	mov	di,12*160	;clear the bar area
	mov	ah,07h
	mov	cx,9
	mov	si,offset msg6
shwcsl4:
	lodsb
	stosw
	loop	shwcsl4
	mov	ax,siten	;display number of sites
	mov	si,offset toasc
	call	int2asc
	add	si,2
	mov	di,12*160+12	;total sites destination
	mov	ah,07h
	mov	cx,3
shwcsl5:
	lodsb
	stosw
	loop	shwcsl5
	xor	ax,ax		;display number of the current site
	mov	al,sitrow
	sub	al,14
	add	ax,sitea
	inc	ax
	mov	si,offset toasc
	call	int2asc
	add	si,3
	mov	di,12*160	;this site number destination
	mov	ah,07h
	mov	cx,2
shwcsl6:
	lodsb
	stosw
	loop	shwcsl6
	mov	dx,sitcurs
	call	setcurs
	ret
show_cur_site endp

;reset bifid for new alphabet or period
;ax=0, don't expand.  ax=1 expand
reset	proc	near
	mov	dx,ax
	mov	es,bifa.cipa	;clear 1st bifid plaintext
	mov	di,bifa.subp
	mov	cx,bifa.cnt
	mov	al,' '
	rep stosb
	test	flag1,01h	;if there is a 2nd bifid
	jz	resetxt
	mov	es,bifb.cipa	;..clear it too
	mov	di,bifb.subp
	mov	cx,bifb.cnt
	mov	al,' '
	rep stosb
	mov	si,offset bifb
	or	dx,dx
	jz	reset10
	call	expand
reset10:
	call	setnat
resetxt:
	mov	si,offset bifa
	or	dx,dx
	jz	reset20
	call	expand
reset20:
	call	setnat
	les	di,dword ptr workb
	mov	cx,worklen	;clear the work area
	mov	al,' '
	rep stosb
	ret
reset	endp

;preserve present key and bifid(s) in transaction log
pushbif proc	near
	push	dx
	push	es
	push	di
	push	si
	test	flag2,80h	;if log is broken
	jnz	pushbfxt	;..ignore request
	xor	dx,dx		;push wanted
	call	biflog
	jnc	pushbfxt
	or	flag2,80h	;log is broken
	mov	si,offset msg21 ;say write error
	mov	di,24*160+80
	call	showmsg
	or	flag1,20h	;flag msg to clear
pushbfxt:
	pop	si
	pop	di
	pop	es
	pop	dx
	ret
pushbif endp

;recover previous key and bifids
popbif	proc	near
	push	dx
	push	es
	push	di
	push	si
	test	flag2,80h	;if log is broken
	jnz	popbifxt
	mov	dx,1		;recover wanted
	call	biflog
	jnc	popbif10
	or	flag2,80h	;flag broken log
	mov	si,offset msg22
	mov	di,24*160+80
	call	showmsg
	or	flag1,20h
popbif10:
	or	ax,ax		;if no key retrieved
	jz	popbifxt
	and	flag1,0FFh-02h	;..turn off key flag
popbifxt:
	pop	si
	pop	di
	pop	es
	pop	dx
	ret
popbif	endp

;relocate a new keysquare to compact memory
relocate proc	near
	mov	si,offset save3b
	call	freemem 		;reset memory
	les	di,dword ptr free3b	;old keysquare is destination
	push	ds
	lds	si,dword ptr [bp].bfkeyb ;->new key
	mov word ptr [si].bifree,di   ;correct key location for
	mov word ptr [si].bifree+2,es	;..transaction logging
	mov	cx,[si].bifgross	;length of new key
	push	cx
	rep movsb
	pop	cx
	pop	ds
	call	getmem		;adjust avail memory to new key size
	les	di,dword ptr free3b
	mov	[bp].bfkeyb,di		;correct key's anchor
	mov	[bp].bfkeya,es
	push	ds
	push	es
	pop	ds
	lea	si,[di].bifmem		;record current memory
	call	savefree
	pop	ds
	ret
relocate endp

;set siting parms for matched twins
;bx->mark structure, 1 or 2
;si->bifid parms, 1 or 2
markset proc   near
	mov	ax,[bx].marka		;offset to beginning of matched text
	mov	pairs.siti,ax
	mov	ax,[si].period		;period of this bifid
	mov	pairs.sitper,ax
	mov	ax,[si].kurz		;short block length
	mov	pairs.sitkurz,ax
	mov	ax,[si].blocks		;number of blocks
	mov	pairs.sitblkn,ax
	mov	ax,[si].cipa		;bifid segment
	mov word ptr pairs.sitcipt+2,ax ;..top line
	mov word ptr pairs.sitcipb+2,ax ;..bot line
	mov	ax,[si].cipb
	mov word ptr pairs.sitcipt,ax	;..top line offset
	add	ax,[si].cnt
	mov word ptr pairs.sitcipb,ax	;..bot line offset
	ret
markset endp

;check user entered char in al for validity
chklet	proc	near
	cmp	al,'a'          ;if in range,
	jb	chklet10
	cmp	al,'z'
	ja	chklet10
	and	al,0DFh 	;..fold to upper case
chklet10:
	cmp	al,j2i		;if a trap letter (eg, J)
	jne	chklet20
	mov	al,j2i+1	;..replace with its substitute (eg, I)
chklet20:
	push	ds		;validate the letter
	pop	es
	mov	di,offset bifalf   ;current bifid alphabet
	mov	cx,square
	repne scasb
	je	chklet30
	mov	si,offset msg9	;say letter is invalid
	mov	di,24*160+80
	call	showmsg
	or	flag1,20h	;message to clear
	stc
	ret
chklet30:
	clc
	ret
chklet	endp

;show period list
showblk proc	near
	mov	cx,8
	mov	es,[bp].videoa
	mov	di,13*160+22		;show title
	mov	si,offset blktit
	mov	ah,07h
shwblkl1:
	lodsb
	stosw
	loop	shwblkl1
	mov	cx,10			;show table entries
	mov	si,offset dictab
	mov	di,14*160+22
shwblklp:
	push	di
	mov	ax,[si]
	push	si
	mov	si,offset toasc ;get this block length
	call	int2asc
	pop	si
	mov	ax,word ptr toasc+3
	mov	es:[di],al	;period to screen
	mov	es:[di+2],ah
	add	di,6
	fld dword ptr [si+2]	;get the dic
	push	bp
	sub	sp,4
	mov	bp,sp
	mov word ptr [bp],offset collect
	mov word ptr [bp+2],ds
	mov	ax,4
	call	fp2asc
	add	sp,4
	pop	bp
	push	si
	push	cx
	mov	si,offset collect+1
	mov	cl,collect
	mov	ah,07h
	cmp	cl,6
	jna	shwblk10
	mov	cl,6
shwblk10:
	cmp byte ptr [si+3],'.'         ;try to align decimal points
	je	shwblkl2		;two leading digits, ok
	add	di,2		;to one more position
	cmp	cl,5		;..but no more than 5
	jna	shwblkl2
	mov	cl,5
shwblkl2:
	lodsb
	stosw
	loop	shwblkl2
shwblk30:
	pop	cx
	pop	si
	pop	di
	add	di,160		;next screen line
	add	si,6		;next dic entry
	loop	shwblklp
	call	blkcurs 	;highlight current choice
	ret
showblk endp

blkcurs proc	near
	xor	ax,ax
	mov	al,perrow
	sub	ax,13
	mov	si,offset toasc
	call	int2asc
	mov	ax,word ptr toasc+3	 ;current selection to screen
	mov word ptr msg33,ax
	mov	es,[bp].videoa
	mov	di,12*160+22
	mov	si,offset msg33
	mov	cx,9
	mov	ah,07h
blkcurl0:
	lodsb
	stosw
	loop	blkcurl0
	mov	di,14*160+23		;clear all hilites
	mov	cx,10
	mov	al,07h
blkcurl1:			;clear any highlights
	push	di
	push	cx
	mov	cx,9
blkcurl2:
	stosb
	inc	di
	loop	blkcurl2
	pop	cx
	pop	di
	add	di,160
	loop	blkcurl1
	xor	ax,ax
	mov	al,perrow
	sub	ax,14
	mul	c160
	mov	di,ax
	add	di,14*160+23
	mov	al,0Fh
	mov	cx,9
blkcurl3:
	stosb
	inc	di
	loop	blkcurl3
	ret
blkcurs endp

;site table compare routine for sort
	assume	ds:nothing,es:nothing
compare proc	far
	mov	bx,[si] 	;compare A:B
	cmp	bx,es:[di]
	ret
compare endp

;floating point compare routine for sort
comp2	proc	far
	push	si
	push	di
	push	cx
	add	si,5
	add	di,5
	mov	cx,4
	std
	repe cmpsb
	cld
	pop	cx
	pop	di
	pop	si
	ret
comp2	endp

crypt25 ends
	end
