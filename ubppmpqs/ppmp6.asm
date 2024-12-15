;PPMP6.ASM 2.0
;	MACHINE LANGUAGE SUBROUTINES
;	FOR PPMPQS
;	1991/92 by YUJI KIDA
;
.386p

code	segment	use16
	assume	cs:code,ds:code


	org	100h
start:

	INCLUDE	UBP.MAC
	include	ppmp.h


	JMP	start0

  align	4
XMEMsize		dd	?
ROWSbase		dd	?
decompdatabase		dd	?
decomppointerbase	dd	?
historybase		dd	?
freebase		dd	?

totaloperations		dd	?

P_OFF		dw	?
P_SEG		dw	?

E_OFF		dw	?
E_SEG		dw	?

Y_OFF		dw	?
Y_SEG		dw	?

DRV_OFF		dw	?
DRV_SEG		dw	?

primes		dw	?
FBsize		dw	?
Rmatrixsize	dw	?

primehandle	dw	?
ROWShandle	dw	?
decomphandle	dw	?
anshandle	dw	?
histhandle	dw	?

databuffer	dw	?
ans_seg		dw	?

Rbytes		dw	?
Rwords		dw	?

Tbytes		dw	?
Twords		dw	?
Tdwords		dw	?

spmem		dw	?


;branch

START0:
	MOV_AX	AR0		;ARRAY[0] is the command number
	MOV	BX,OFFSET CMD_TBL
	SHL	AX,1
	ADD	BX,AX
	JMP	CS:[BX]

CMD_TBL:
	DW	final_init,square_ans,putPandE,closefiles

;
; close files
;
;command#=3

closefiles:
	mov	ah,3eh
	mov	bx,cs:[anshandle]
	or	bx,bx
	jz	short closefnext
	int	21h
closefnext:
consefret:
	retf

;
; answer exponents of p
;
;command#=2

putPandE:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	ebx,primeadr+12		;exp part of 2
	mov	cx,[primes]
putPE10:
	mov	eax,fs:[ebx]
	or	eax,eax
	jnz	short putPE100
	add	ebx,8
	loop	putPE10

	lds	si,dword ptr [P_OFF]
	mov	word ptr [si],0		;set 0 = no more data

putPEret:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

putPE100:
	mov	dword ptr fs:[ebx],0	;clear
	lds	si,dword ptr [E_OFF]
	call	seteax
	mov	eax,fs:[ebx-4]
	lds	si,dword ptr cs:[P_OFF]
	call	seteax
	jmp	putPEret

seteax:
	mov	cx,2
	cmp	eax,10000h
	jae	short seteaxjp
	dec	cx
	or	ax,ax
	jnz	short seteaxjp
	dec	cx
seteaxjp:
	mov	[si],cx
	mov	[si+2],eax
	ret	
	

;
; answer square data
;
;command#=1

square_ans:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	[spmem],sp

ans_again:
	mov	dx,offset BUFFER	;read answer
	mov	cx,[Rbytes]
	mov	bx,[anshandle]
	mov	ah,3fh
	int	21h
	or	ax,ax
	jz	nomoreanswer

	add	ax,offset BUFFER
	add	ax,3
	and	ax,0fffch
	mov	[databuffer],ax

	call	translate
	call	backsubstitution

	mov	si,[databuffer]
	mov	cx,[Tbytes]
	mov	di,si
	add	di,cx
	sub	di,2		;di = highest word adr
	shr	cx,1

	xor	ax,ax
	std
	repe	scasw
	cld

	je	ans_again	;illegal answer

	inc	cx		;word length

	push	es
	mov	es,[ans_seg]
	xor	di,di
	mov	ax,cx
	stosw			;word length
	rep	movsw
	pop	es

	call	countexponents

squareret:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

nomoreanswer:
	mov	bx,AR0
	mov	word ptr [bx],0
	jmp	squareret


translate:
	mov	di,[databuffer]
	mov	cx,[Tdwords]
	xor	eax,eax
	rep	stosd

	mov	ebx,[ROWSbase]
	mov	si,offset BUFFER
	mov	cx,[Rwords]
trans10:
	push	cx
	lodsw
	mov	cx,16
trans20:
	shr	ax,1
	jnc	short trans30

	push	cx
	push	ax

	mov	di,fs:[ebx]
	mov	cx,di
	and	cl,7
	shr	di,3
	mov	al,1
	shl	al,cl
	add	di,[databuffer]
	or	[di],al

	pop	ax
	pop	cx

trans30:
	add	ebx,2
	loop	trans20

	pop	cx
	loop	trans10
	ret


backsubstitution:
	mov	bx,[databuffer]
	mov	edx,[totaloperations]
	mov	esi,edx
	dec	esi
	shl	esi,2
	add	esi,[historybase]
  align	4
backsubst10:
	mov	edi,fs:[esi]
	mov	cx,di
	and	cl,7
	shr	di,3
	mov	al,1
	shl	al,cl
	test	[di+bx],al
	jnz	short backsubst30
backsubst20:
	sub	esi,4
	dec	edx
	jnz	backsubst10
	ret

  align	4
backsubst30:
	shr	edi,16			;source
	mov	cx,di
	and	cl,7
	shr	di,3
	mov	al,1
	shl	al,cl
	xor	[di+bx],al
	jmp	backsubst20


countexponents:
	mov	edi,primeadr+4		;adr of exponents
	mov	cx,[FBsize]
	xor	eax,eax
ctexp10:
	mov	fs:[edi],eax		;clear exponents
	add	edi,8
	loop	ctexp10

	mov	ebx,[decomppointerbase]
	mov	si,[databuffer]
	mov	cx,[Twords]
ctexp20:
	push	cx
	lodsw
	mov	cx,16
ctexp30:
	shr	ax,1
	jc	short ctexp100
ctexp40:
	add	ebx,4
	loop	ctexp30

	pop	cx
	loop	ctexp20
	ret

ctexp100:
	push	ax
	push	cx

	mov	edi,fs:[ebx]
	mov	cx,fs:[edi]
  align	4
ctexp110:
	add	edi,2
	movzx	eax,word ptr fs:[edi]
	shl	eax,3			;*8
	add	eax,primeadr+4
	inc	dword ptr fs:[eax]	;inc exponents
	loop	ctexp110

	pop	cx
	pop	ax
	jmp	ctexp40

	
;
;* Initialize
;
;COMMAND#=0
;

final_init:
	;get parameters

	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	[spmem],sp

	MOV_AX	V1+2		;memo the segment of ans%(0)
	ADD	AX,ARRAYHEADSEG
	MOV	[ANS_SEG],AX

	MOV	BX,V2		;memo the address of P
	MOV	AX,[BX]
	MOV	[P_OFF],AX
	MOV	AX,[BX+2]
	MOV	[P_SEG],AX

	MOV	BX,V3		;memo the address of E
	MOV	AX,[BX]
	MOV	[E_OFF],AX
	MOV	AX,[BX+2]
	MOV	[E_SEG],AX

	MOV	BX,V4		;memo the address of Y
	MOV	AX,[BX]
	MOV	[Y_OFF],AX
	MOV	AX,[BX+2]
	MOV	[Y_SEG],AX

	MOV	BX,V5		;memo the address of DataDrv$
	MOV	AX,[BX]
	MOV	[DRV_OFF],AX
	MOV	AX,[BX+2]
	MOV	[DRV_SEG],AX

	xor	eax,eax
	mov_ax	AR1
	shl	eax,10
	mov	[XMEMsize],eax

	mov_ax	AR2
	shl	ax,1
	mov	[FBsize],ax
	dec	ax
	mov	[primes],ax
	inc	ax
	shr	ax,3			;/ 8
	mov	[Tbytes],ax
	shr	ax,1
	mov	[Twords],ax
	shr	ax,1
	mov	[Tdwords],ax

	call	setPROTECTfs

	call	openANSFILE
	call	getPRIMES
	call	getROWS
	call	getDECOMPDATA
	call	gethistDATA
	mov	eax,[XMEMsize]
	add	eax,pmodeworkadr
	cmp	eax,[freebase]
	jb	short lackofmemory
initout:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

diskerror:
	mov	ax,8001h
initerror:
	mov	ds:[AR0],ax
	mov	sp,[spmem]
	jmp	initout

lackofmemory:
	mov	ax,8002h
	jmp	initerror


openANSFILE:
	mov	dx,offset ansfilename	;read matrix size
	mov	ax,3d00h		;from answer file
	int	21h
	jc	diskerror
	mov	[anshandle],ax

	mov	dx,offset Rmatrixsize	;get matrixsize
	mov	cx,2
	mov	bx,ax
	mov	ah,3fh			;read
	int	21h
	jc	diskerror

	mov	ax,[Rmatrixsize]
	shr	ax,3
	mov	[Rbytes],ax
	shr	ax,1
	mov	[Rwords],ax
	ret


gethistDATA:
	mov	ax,3d00h		;open for read
	mov	dx,offset historyfilename
	int	21h
	jc	diskerror
	mov	[histhandle],ax

	mov	edi,[historybase]
gethist10:
	mov	dx,offset BUFFER
	mov	cx,400h
	mov	bx,[histhandle]
	mov	ah,3fh
	int	21h
	or	ax,ax
	jz	short gethisteof

	mov	cx,ax
	shr	cx,2
	mov	si,offset BUFFER
gethist20:
	lodsd
	mov	fs:[edi],eax
	add	edi,4
	loop	gethist20
	jmp	gethist10

gethisteof:
	mov	[freebase],edi
	sub	edi,[historybase]
	shr	edi,2			;4bytes/unit
	mov	[totaloperations],edi

	mov	ah,3eh			;close file
	mov	bx,[histhandle]
	int	21h
	ret


getDECOMPDATA:
	mov	si,offset decompfilename
	call	setpathname
	mov	dx,offset BUFFER
	mov	ax,3d00h		;open for read
	int	21h
	jc	diskerror
	mov	[decomphandle],ax

	mov	edi,[decompdatabase]
getdecomp10:
	mov	dx,offset BUFFER
	mov	cx,400h
	mov	bx,[decomphandle]
	mov	ah,3fh
	int	21h
	or	ax,ax
	jz	short getdecompeof

	mov	cx,ax
	shr	cx,1
	mov	si,offset BUFFER
getdecomp20:
	lodsw
	mov	fs:[edi],ax
	add	edi,2
	loop	getdecomp20
	jmp	getdecomp10

getdecompeof:
	add	edi,3
	and	di,0fffch
	mov	[decomppointerbase],edi

	mov	ah,3eh			;close file
	mov	bx,[decomphandle]
	int	21h

	mov	ebx,[decomppointerbase]
	mov	eax,[decompdatabase]
	xor	edx,edx
getdecomp100:
	mov	fs:[ebx],eax
	mov	dx,fs:[eax]
	inc	dx
	shl	dx,1
	add	eax,edx
	add	ebx,4
	cmp	eax,[decomppointerbase]
	jb	getdecomp100
	mov	[historybase],ebx
	ret


getROWS:
	mov	ax,3d00h		;open for read
	mov	dx,offset ROWSfilename
	int	21h
	jc	diskerror
	mov	[ROWShandle],ax

	mov	edi,[ROWSbase]
getROWS10:
	mov	dx,offset BUFFER
	mov	cx,400h			;1024bytes unit
	mov	bx,[ROWShandle]
	mov	ah,3fh			;read
	int	21h
	or	ax,ax
	jz	short getROWSeof

	mov	cx,ax
	shr	cx,1
	mov	si,offset BUFFER
getROWS20:
	lodsw
	mov	fs:[edi],ax
	add	edi,2
	loop	getROWS20
	jmp	getROWS10

getROWSeof:
	mov	eax,edi
	add	eax,3
	and	ax,0fffch

	mov	[decompdatabase],eax
comment %
	mov	eax,edi
	sub	eax,[ROWSbase]
	shr	eax,1
	mov	[ROWSsize],ax
%
	mov	ah,3eh			;close file
	mov	bx,[ROWShandle]
	int	21h
	ret


getPRIMES:
	mov	si,offset primefilename
	call	setpathname
	mov	dx,offset BUFFER
	mov	ax,3d00h		;open for read
	int	21h
	jc	diskerror
	mov	[primehandle],ax

	mov	edi,primeadr
primelp:
	mov	dx,offset BUFFER
	mov	cx,400h
	mov	bx,[primehandle]
	mov	ah,3fh
	int	21h
	or	ax,ax
	jz	short primeeof

	shr	ax,2
	mov	cx,ax
	mov	si,offset BUFFER
prime50:
	lodsd
	mov	fs:[edi],eax
	add	edi,8
	loop	prime50
	jmp	primelp

primeeof:
	mov	[ROWSbase],edi

	mov	ah,3eh
	mov	bx,[primehandle]
	int	21h
	ret


setpathname:				;input si
	push	ax
	push	cx
	push	di

	push	ds			;/*
	push	si

	lds	si,dword ptr [Drv_off]
	mov	di,offset BUFFER
	lodsw
	mov	cx,ax
	shl	cx,1
	and	cx,00ffh		;cut 255 chars
	shl	ax,1
	jnc	short setpath10
	dec	cx
setpath10:
	rep	movsb

	pop	si
	pop	ds			;*/

setpath20:
	lodsb
	stosb
	or	al,al
	jnz	setpath20

	pop	di
	pop	cx
	pop	ax
	ret


primefilename	db	'PPMPPRMS.PPM',0
ansfilename	db	'PPMPANS.PPM',0
historyfilename	db	'PPMPHIST.PPM',0
ROWSFILENAME	db	'PPMPROWS.PPM',0
DECOMPFILENAME	db	'PPMPDCMP.PPM',0


setPROTECTfs:
	pushf

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

	popf
	ret

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


  align	4
BUFFER		dw	maxbufferwords dup(0)

CODE	ENDS
END	START
