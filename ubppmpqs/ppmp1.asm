;ppmp1.asm
;check if A20 line is enable.

.286p
code	segment
	assume	cs:code,ds:code
	include	ubp.mac

	org	100h
start:
	xor	ax,ax
	mov	ds,ax
	dec	ax
	mov	es,ax
	xor	si,si
	mov	di,0010h
	mov	cx,0200h
	repe	cmpsw
	je	cannotuse
	smsw	ax
	test	al,1
	jnz	virtual86
	xor	ax,ax
outret:
	mov	bx,AR0
	mov	word ptr cs:[bx],ax
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

cannotuse:
	mov	ax,1
	jmp	outret

virtual86:
	mov	ax,2
	jmp	outret

code	ends
end	start