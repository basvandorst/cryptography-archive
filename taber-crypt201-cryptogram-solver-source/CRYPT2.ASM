	page	53,132
	include frame.str
crypt2b segment 'cryb'
savechar dw	0	;save cryptogram letter
wrapa	dw	0	;offset to beginning of word to wrap
crypt2b ends
	extrn	get_line:far
crypt2	segment 'crya'
	assume	cs:crypt2,ds:crypt2b
	public	cry_char,rub_out,del_char
;put a cryptogram character into the cryptogram buffer
;al = character to put on the screen.
;dx = current cursor position
;es:di->current line or command line
cry_char proc	far
	push	ds		;save caller's segment
	mov	bx,crypt2b
	mov	ds,bx		;establish addressability
	test	[bp].flagb,04h	;if in character insert mode
	jz	crylet00
	jmp	chari		;..don't even think of handling it here
;character replace mode
crylet00:
	cmp	dl,80		;if off the line
	jb	crylet20	;..(it's not, the easy case)
	test	[bp].modea,05h	;if command mode
	jnz	beep		;..no continuation line
	mov	savechar,ax	;save keystroke
	call	get_line ;es:di->old line, gramline->new inserted line
	jnc	crylet01
	jmp	cryletxt	;getline failed
crylet01:
	mov	dl,0		;reset cursor column
	cmp	dh,23		;if at bottom of screen
	jnb	crylet02	;..leave it there
	inc	dh		;move to next row on screen
crylet02:
;   if word split
	test	[bp].flagb,01h	     ;if blanks are literals
	jnz	crylet10	;..just continue with new line, can't split
	cmp	al,' '          ;if current keystroke is blank
	je	cryletxt	;..nothing to split, and we're done
	call	wraplen 	;find length of wrap word
	jcxz	crylet10	;..nothing to wrap
	mov	di,[bp].gramb
	call	wrapit		;move end word to start of new line
crylet10:
	mov	di,[bp].gramb ;repoint to new line
	mov	dl,es:[di]	;reset current column
	mov	al,byte ptr savechar	;restore the keystroke
crylet20:
	xor	bx,bx		;replace char at cursor position
	mov	bl,dl
	inc	bx		;skip past count byte
	mov byte ptr es:[di][bx],al ;and put char in the line
	inc	dl		;next cursor position
;update the line length
	cmp	es:[di],dl	;if length >= cursor pos
	ja	cryletxt	;..we overstruck char, and length not changed
	mov	es:[di],dl	;else we added a char to the line
	clc
cryletxt:
	les	di,dword ptr [bp].gramb
	call	recount 	;always recount length of current line
	pop	ds
	ret
chari:
	mov	savechar,ax	;save the character
	cmp	dl,80		;if end of line
	jb	chari10
beep:
	mov	ax,0E07h	;..beep the user
	push	bp
	int	10h
	pop	bp
	clc
	jmp	cryletxt
chari10:
	cmp	dl,es:[di]	;if cursor past meaningful characters
	jnb	crylet20	;..do a replace
;there are characters after the cursor
	cmp byte ptr es:[di],80 ;if line is full
	jnb	beep		;..beep the user
;there's room for one more character on this line
	inc byte ptr es:[di]	;count the character
	mov	cx,79		;figure letters to scoot right
	sub	cl,dl	;max length - cursor position
	add	di,80		;es:di->last char position, destination
	push	ds		;save addressability
	push	es		;either command or cryptogram line
	pop	ds
	mov	si,di
	dec	si		;ds:si->2nd last char position, source
	std
	rep movsb		;scoot backwards
	cld
	pop	ds		;restore addressability
	stosb		;put char in line
	inc	dl	;next position in line
	clc
	jmp	cryletxt
cry_char endp
recount proc	near
	pushf	;save the return code
	test	[bp].flagb,01h	;if blanks are literals
	jnz	recntxit	;..can't adjust count
	test	[bp].modea,01h	;if in command mode
	jnz	recntxit	;..can't adjust count
	push	di
	add	di,80		;-> last possible char on line
	mov	cx,80		;max letters per line
	mov	al,' '
	std			;scan backwards
	repe scasb		;..for non-blank
	cld
	je	recnt10 	;all blanks
	inc	cl		;adjust count
recnt10:
	pop	di
	mov	es:[di],cl
recntxit:
	popf
	ret
recount endp

;es:di->new line
;cx = number of bytes to move
wrapit	proc	near
	push	ds		;save addressability
	mov	si,wrapa	;beginning of word
	push	es
	pop	ds		;ds:si->beginning of wrap word
	push	si
	push	cx
	mov	es:[di],cl	;update length
	inc	di
	rep movsb		;move the wrap word
	pop	cx
	pop	di
	mov	al,' '          ;vacate the wrap word
	rep stosb
	sub	di,81		;adjust true length of old line
	pop	ds		;restore addressability
	call	recount
	ret
wrapit	endp
wraplen proc	near
	cmp byte ptr es:[di+80],' '    ;if last char is blank
	jne	wrapln02
wrapln00:
	xor	cx,cx			;..no word wrap
	ret
wrapln02:
	mov	cx,40	;search backwards last half of line
	add	di,80	;es:di->last char on line
wrapln04:
	std
	mov	al,' '  ;search backwards for a blank
	repne scasb
	cld
	je	wrapln10	;blank was found in 40 bytes or less
	ret		;word too long for wrapping
wrapln10:
	mov	ax,cx
	mov	cx,39	;40-ax = number of letters to move
	sub	cx,ax
	add	di,2	;->1st letter to move
	mov	wrapa,di
	ret
wraplen endp
;es:di->current line
;dx=cursor position
rub_out proc	far
	xor	ax,ax
	cmp byte ptr es:[di],0	;if the line is empty
	je	ruboutxt	;..just backspace the cursor
	mov	al,es:[di]
	cmp	dl,al		;if cursor well past last char
	ja	rubout50	;..just backspace the cursor
	dec	al
	mov	es:[di],al	;new length
	push	ds
	inc	di		;compute destination
	xor	bx,bx
	mov	bl,dl
	dec	bx
	js	rubout10	;destination is letter at cursor position
	add	di,bx		;destination is letter before cursor position
rubout10:
	mov	si,di
	inc	si
	push	es
	pop	ds		;ds:si->letter at or after cursor position
	mov	cx,ax		;figure letters to shift left
	sub	cl,dl		;length-1 less cursor position
	inc	cl
	jcxz	rubout12
	rep movsb
rubout12:
	mov byte ptr es:[di],' '     ;blot out last character
	pop	ds
ruboutxt:
	cmp	dl,0		;if cursor column not 0
	je	rubret
rubout50:
	dec	dl		;go to previous column
rubret:
	ret
rub_out endp
;es:di->line
;dx=cursor position
del_char proc	far
	xor	ax,ax
	cmp	dl,80		;can't delete at end of line
	jnb	delchrxt
	cmp byte ptr es:[di],0	;if line is empty
	je	delchrxt	;..there is no character to delete
	cmp	dl,es:[di]	;if cursor is past meaningful chars
	jnb	delchrxt	;..there is no character to delete
	dec byte ptr es:[di]	;discount erased character
	mov	al,dl
	add	di,ax
	inc	di		;es:di->character to erase at cursor position
	mov	cx,79
	sub	cx,ax		;cx=remaining chars on line
	jz	delchr10	;..unless there isn't any
	push	ds		;save caller's addressability
	push	es		;either command or cryptogram line
	pop	ds
	mov	si,di
	inc	si		;ds:si->char to move to cursor position
	rep movsb		;move remaining chars over erased char
	pop	ds		;restore caller's addressability
;erase last character
delchr10:
	mov byte ptr es:[di],' '
delchrxt:
	ret
del_char endp
crypt2	ends
	end
