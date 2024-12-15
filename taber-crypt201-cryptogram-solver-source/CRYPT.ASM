	page	53,132
	.8087
	include frame.str
	include cryptb.asm
	extrn	EDITOR:far
crypta	segment 'crya'
	assume	cs:crypta,ds:cryptb
crypt	proc	far
	jmp short copyjmp
 db ' (C) Copyright 1988, 1990.  John K. Taber  '
copyjmp:
;Compute the beginning of free space.
;The stack segment is forced by name and link order to be the last segment,
;thus on entry ss:sp addresses the 1st byte of free space beyond my program.
	mov	dx,sp		;figure beginning paragraph of free space
	add	dx,15		;round up to paragraph boundary
	mov	cl,4
	shr	dx,cl		;..then divide by 16
	mov	ax,ss
	add	dx,ax		;ss+sp paragraphs
	inc	dx		;..plus 1 more for good measure
;Set up data addressability, now that I'm no longer dependent on stack
;to determine free space.
	mov	ax,cryptb	;my dseg addressability
	mov	ds,ax
	cld
;Set up frame for intermodule communications in the stack
	sub	sp,type frame	;universal anchor space
	mov	bp,sp
;Start filling in the frame
	mov	[bp].freeha,dx	;begin free space paragraph
	mov	[bp].abovoa,dx	;..for resetting
	mov	[bp].pspa,es	;psp pointer
;determine video buffer address from type of monitor
	int	11h	;get configuration bits in ax
	mov	ah,al
	and	al,30h		;off all but monitor
	mov	[bp].videoa,0B000h  ;assume monochrome display
	mov	[bp].cursz,0B0Ch    ;monochrome cursor size
	mov	[bp].vport,3B8h     ;mono control port
	cmp	al,30h
	je	crypt01
	mov	[bp].videoa,0B800h  ;..it's color, correct assumption
	mov	[bp].cursz,0607h    ;..and color cursor size
	mov	[bp].vport,3D8h     ;..color control port
	or	[bp].flaga,40h	    ;..and flag color monitor
;Ensure co-processor on board, or quit
crypt01:
	test	ah,02h	;test for 8087
	jnz	crypt02 	;got it, ok
	mov	dx,offset err1	;no co-processor,
	mov	ah,9h		;..say can't run
	int	21h
	jmp	exit		;..and quit.
crypt02:
;Get monitor control bits to enable, disable video
	mov	ah,15		;get control bits for this monitor
	int	10h
	xor	bx,bx
	mov	bl,al
	mov	al,vcontrol[bx]
	xor	ah,ah
	mov	[bp].vstatus,ax
;allocate storage
	int	12h		;get memory size in ax (1K increments)
	xor	dx,dx		;figure size of free space in paragraphs
	mov	cx,6
crypt06:
	shl	ax,1
	rcl	dx,1
	loop	crypt06
	sub	ax,[bp].freeha	;less 1st paragraph of free space
	sbb	dx,0		;..is available free paragraphs
	mov	cx,4		;now put space in bytes for fine control
crypt08:
	shl	ax,1
	rcl	dx,1
	loop	crypt08
	mov	[bp].sizeb,dx	;available space in bytes
	mov	[bp].abovod,dx	;..for resetting
	mov	[bp].sizea,ax
	mov	[bp].abovoc,ax
	call	editor		;bring up full screen editing
exit:
	add	sp,type frame
	mov	ax,4C00h
	int	21h
crypt	endp

crypta	ends
	end
