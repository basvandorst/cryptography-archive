;PPMP3b.ASM 2.0
;	MACHINE LANGUAGE SUBROUTINES
;	FOR PPMPQS
;	1991/92 by YUJI KIDA
;
.386P

code	segment	use16
	assume	cs:code,ds:code

	INCLUDE	UBP.MAC
	include	ppmp.h

org	100h

start:

;branch

	MOV_AX	AR0		;ARRAY[0] is the command
	MOV	BX,OFFSET CMD_TBL
	SHL	AX,1
	ADD	BX,AX
	JMP	CS:[BX]

CMD_TBL:
	dw	initialize,getprime,storedecomdata,closefiles

	even
P_OFF		dw	?
P_SEG		dw	?
primeadrnow	dd	?

C_OFF		dw	?
C_SEG		dw	?

primehandle	dw	?
decomhandle	dw	?

FBsize		dw	?
primes		dw	?

rowstotal	dw	?
rowslimit	dw	?
rowsnow		dw	?

primenow	dw	?
offnow		dw	?

;
; close all files
;
;command#=3

closefiles:
	mov	ah,3eh
	mov	bx,cs:[decomhandle]
	int	21h

	mov	ah,3eh
	mov	bx,cs:[primehandle]
	int	21h
	retf

;
; get and set primes
;
;command#=1

getprime:
	lds	si,dword ptr cs:[P_OFF]

	lodsw
	shl	ax,1
	add	ax,si
	mov	bx,ax
	mov	word ptr [bx],0
	mov	eax,[si]
	mov	dword ptr cs:[BUFFER],eax
	mov	edi,cs:[primeadrnow]
	mov	fs:[edi],eax
	add	edi,4
	mov	cs:[primeadrnow],edi

	mov	ax,cs
	mov	ds,ax

	mov	dx,offset BUFFER
	mov	cx,4
	mov	bx,[primehandle]
	mov	ah,40h
	int	21h
	jc	short primediskerror

primeret:
	mov	ax,ss
	mov	ds,ax
	retf

primediskerror:
	mov	bx,AR0
	mov	word ptr [bx],8001h
	jmp	primeret

;
; write factored data
;
;command#=2

setdata:
	push	bx
	mov	bx,[offnow]
	mov	ax,[primenow]
	mov	[bx],ax			;set prime#
	add	bx,2
	mov	[offnow],bx
	pop	bx
	ret

storedecomdata:
	push	bp

	mov	ax,cs:[rowstotal]
	cmp	ax,cs:[rowslimit]
	jae	div32incomplete		;neglect this data

	inc	ax
	mov	cs:[rowstotal],ax
	mov	cs:[rowsnow],ax

	mov	cs:[primenow],0
	mov	cs:[offnow],offset BUFFER

	lds	si,dword ptr cs:[C_off]
	mov	ax,cs		;transfer to 32 bit format
	mov	es,ax		;and copy to WBUFFER in cs
	mov	bx,offset CBUFFER	;reserve BX henceforce
	mov	di,bx
	xor	eax,eax
	lodsw
	test	ax,8000h	;check sign
	pushf			;*
	and	ax,1fffh
	mov	cx,ax
	inc	ax
	shr	ax,1
	stosd
	rep	movsw
	xor	ax,ax
	stosw

	mov	ax,cs
	mov	ds,ax

	popf			;*
	jz	div32positive
	call	setdata
div32positive:
	mov	edi,primeadr+4	;primeunitbytes	;offset of 2

	STD

	movzx	ecx,[primes]
div32LP:
	push	ecx
	inc	[primenow]
	MOV	eBP,fs:[eDI]	;ebp = denominator
div3210:
	MOV	SI,BX
	MOV	AX,[SI]
	MOV	CX,AX		;dword length
	SHL	AX,2
	ADD	SI,AX		;dword highest adr
	XOR	eDX,eDX	
	shr	cx,1
	pushf			;*
	jcxz	div3221
	align	4
div3220:
	LODSd
	DIV	eBP
	LODSd
	DIV	eBP
	LOOP	div3220
div3221:
	popf			;*
	jnc	div3222
	LODSd
	DIV	eBP
div3222:
	OR	eDX,eDX
	JZ	short div32100	;divide exactly if it can

	add	edi,4		;primeunitbytes
	pop	ecx
	LOOP	div32LP

	cmp	dword ptr [bx],1
	jne	short div32incomplete
	cmp	dword ptr [bx+4],1
	je	short div200

div32incomplete:		;neglect this data
	POP	BP
	mov	bx,AR0
	mov	word ptr [bx],101	;error mark

	cld
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


div32100:
	call	setdata

	MOV	SI,BX
	MOV	AX,[SI]
	xor	ecx,ecx
	MOV	CX,AX		;dword length
	SHL	AX,2
	ADD	SI,AX		;highest adr
	XOR	eDX,eDX	

	LODSd
	DIV	eBP
	PUSH	eAX		;push highst dword
	JMPS	div32120
	align	4
div32110:
	LODSd
	DIV	eBP
div32120:
	MOV	[SI+4],eAX
	LOOP	div32110

	POP	eAX		;check highest dword
	OR	eAX,eAX
	JNZ	div32130
	dec	dword ptr [bx]	;dec length if highest=0
div32130:
	JMP	div3210


	;write result
div200:
	cld
	mov	ax,[offnow]
	mov	bx,offset BUFFER0
	sub	ax,bx
	mov	cx,ax			;byte size
	sub	ax,2
	shr	ax,1
	mov	[bx],ax			;number of factors
	mov	dx,bx
	mov	bx,[decomhandle]
	mov	ah,40h			
	int	21h
	jc	short decomdiskerror

decomret:
	pop	bp
	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	RETF

decomdiskerror:
	mov	bx,AR0
	mov	word ptr [bx],8001h
	jmp	decomret

;
;* Initialize
;COMMAND#=0

;	V1=P
;	V2=C#
;	AR[1]	TOTAL SIZE \ 2

INITIALIZE:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	MOV	BX,V1		;adr of p
	MOV	AX,[BX]
	MOV	[P_OFF],AX
	MOV	AX,[BX+2]
	MOV	[P_SEG],AX

	MOV	BX,V2		;adr of C#
	MOV	AX,[BX]
	MOV	[C_OFF],AX
	MOV	AX,[BX+2]
	MOV	[C_SEG],AX

	MOV_AX	AR1
	shl	ax,1
	mov	[FBsize],ax
	dec	ax
	mov	[primes],ax

	mov	[rowstotal],0
	mov	[rowsnow],0

	mov	eax,primeadr
	mov	[primeadrnow],eax
	movzx	ebx,[FBsize]
	mov	ecx,ebx
	mov	[rowslimit],bx

	;create files

	mov	dx,offset primefilename
	mov	ah,3ch			;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[primehandle],ax

	mov	dx,offset decomfilename
	mov	ah,3ch			;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[decomhandle],ax

	; make GDT

	mov	ax,cs
	and	eax,0ffffh
	shl	eax,4
	add	eax,offset gdttbl
	mov	dword ptr cs:[gdtaddr+2],eax
	lgdt	cs:[gdtaddr]

	cld
	cli

	mov	eax,cr0		;set PE bit
	or	eax,1		;
	mov	cr0,eax		;
	jmp	$+2		;flush buffer

	mov	ax,8		;now in protect mode
	mov	fs,ax

	mov	eax,cr0		;reset PE bit
	and	eax,07ffffffeh	;
	mov	cr0,eax		;
	jmp	$+2		;flush buffer

	sti

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

diskerror:
	mov	bx,AR0
	mov	word ptr [bx],8001h
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


descriptor	struc
	limit_0_15	dw	0
	base_0_15	dw	0
	base_16_23	db	0
	access		db	0
	gran		db	0
	base_24_31	db	0
descriptor	ends

data_seg_access	equ	092h

gdtaddr	label	qword
	dw	2*8		;size of GDT table
	dd	?
	dw	0
gdttbl	label	dword

gdt_null	descriptor <,,,,,>
gdt_extra	descriptor <0ffffh,0,0,data_seg_access,0cfh,0>


primefilename	db	'PPMPPRMS.PPM',0
decomfilename	db	'PPMPDCMP.PPM',0


	even
CBUFFER		dw	600 dup(0)
BUFFER0	dw	?
BUFFER	dw	1024 dup(0)

CODE	ENDS
END	START
