;PPMP2A.ASM 1.0
;	MACHINE LANGUAGE SUBROUTINES
;	FOR PPMPQS
;	1991 by YUJI KIDA
;
.386P

code	segment	use16
	assume cs:code,ds:code


	INCLUDE	UBP.MAC

open	macro	fname
	mov	ah,3dh
	mov	al,2
	mov	dx,offset fname
	int	21h
endm

	org	100h
start:
	JMP	START0

even
handle1	dw	?
handle2	dw	?


F1	db	32 dup(?)
F2	db	'PPMP-F.UBD',0
P1	db	32 dup(?)
P2	db	'PPMP-P.UBD',0
PP1	db	32 dup(?)
PP2	db	'PPMP-PP.UBD',0
F1name	db	'PPMP-F.UBD',0
P1name	db	'PPMP-P.UBD',0
PP1name	db	'PPMP-PP.UBD',0

start0:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	bx,V1
	lds	si,[bx]
	lodsw
	and	ax,81ffh	;cut types
	shl	ax,1
	sbb	ax,0
	mov	cx,ax

	push	cx
	mov	di,offset F1
	rep	movsb
	mov	ax,cs
	mov	ds,ax
	mov	si,offset F1name
	mov	cx,16
	rep	movsb

	pop	cx
	push	cx
	mov	si,offset F1
	mov	di,offset P1
	rep	movsb
	mov	si,offset P1name
	mov	cx,16
	rep	movsb

	pop	cx
	mov	si,offset F1
	mov	di,offset PP1
	rep	movsb
	mov	si,offset PP1name
	mov	cx,16
	rep	movsb

	open	F1
	mov	[handle1],ax
	open	F2

	mov	[handle2],ax
	call	copy
	jc	errorret

	open	P1
	mov	[handle1],ax
	open	P2

	mov	[handle2],ax
	call	copy
	jc	errorret

	open	PP1
	mov	[handle1],ax
	open	PP2

	mov	[handle2],ax
	call	copy
	jc	errorret
retout:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

errorret:
	mov	bx,AR0
	mov	word ptr [bx],8001h	;set -1 for error code
	jmp	retout

copy:
	mov	ah,42h		;move file ptr
	mov	al,0
	mov	bx,[handle1]
	xor	cx,cx
	mov	dx,16		;skip header
	int	21h

	mov	ah,42h		;move file ptr
	mov	al,2
	mov	bx,[handle2]
	xor	cx,cx
	xor	dx,dx		;move to file end
	int	21h

appendlp:
	mov	ah,3fh		;read
	mov	dx,offset buffer
	mov	cx,8000h
	mov	bx,[handle1]
	int	21h
	jc	appenderror

	mov	cx,ax
	mov	ah,40h		;write
	mov	dx,offset buffer
	mov	bx,[handle2]
	int	21h
	jc	appenderror

	cmp	ax,cx
	jne	appenderror

	cmp	ax,8000h
	je	appendlp

	mov	ah,3eh
	mov	bx,[handle1]
	int	21h

	mov	ah,3eh
	mov	bx,[handle2]
	int	21h

	clc
	ret

appenderror:
	mov	ah,3eh
	mov	bx,[handle1]
	int	21h

	mov	ah,3eh
	mov	bx,[handle2]
	int	21h

	stc
	ret
		

even
buffer	db	8000h dup(0)
	
code	ends
end	start
