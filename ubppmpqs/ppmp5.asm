;PPMP5.ASM 2.0
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
GaussLongbase	dd	?
GaussLongNow	dd	?
XMEMsize	dd	?

MATbytes	dw	0,0
MATbytesW	dw	0,0
MATbytesW4	dw	0,0
MATwords	dw	0,0
MATwordsW	dw	0,0
MATdwords	dw	0,0
MATdwordsW	dw	0,0

makexorDwordsW	dw	0,0
makexorskip	dw	0,0
loadsavebytes	dw	?

unitrows	dw	0
startrow	dw	?
checkingrow	dw	?
localrank	dw	?

MATsize		dw	?

handle		dw	?
anshandle	dw	?
handle1		dw	?
size1		dw	?
handle2		dw	?

RWrow		dw	?

answers		dw	?

spmem		dw	?


;branch

START0:
	MOV_AX	AR0		;ARRAY[0] is the command number
	MOV	BX,OFFSET CMD_TBL
	SHL	AX,1
	ADD	BX,AX
	JMP	CS:[BX]

CMD_TBL:
	DW	gauss_INIT,GAUSS,closefiles

;
; * Close File
;
;command#=2

closefiles:
	mov	ax,cs
	mov	ds,ax

	mov	ah,3eh
	mov	bx,[handle1]
	int	21h

	mov	ah,3eh
	mov	bx,[handle2]
	int	21h

	mov	ah,3eh
	mov	bx,[anshandle]
	int	21h

	mov	ax,[answers]
	mov	bx,AR0
	mov	[bx],ax
	
	mov	ax,ss
	mov	ds,ax
	retf


;
;** Gaussin elimination
;
;command#=1

Gauss:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	[spmem],sp

	mov	ax,[startrow]
	mov	[checkingrow],ax
	call	getpivotrows
	or	ax,ax
	jz	short gauss100
	mov	[localrank],ax
	mov	ax,[checkingrow]
	mov	[startrow],ax
	call	erase_below
gauss100:
	mov	ax,[startrow]
	cmp	ax,[MATsize]
	jb	short gaussret
	mov	bx,AR0
	mov	word ptr [bx],0
gaussret:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


getpivotrows:
	mov	ax,[MATbytesW]
	mov	[loadsavebytes],ax
	mov	eax,[GaussLongbase]
	mov	[GaussLongNow],eax
	mov	cx,[unitrows]
	mov	ax,[checkingrow]
getpivot10:
	cmp	ax,[MATsize]
	jae	short getpivot50	;data ended without filling all

	push	cx
	mov	[RWrow],ax
	call	load_row

	mov	di,offset BUFFER
	call	calcMSB
	or	ax,ax
	jz	short getpivot20	;erased data
	mov	[MSBoffset],cx
	mov	[MSBmask],ax
	call	erasebyothers
	jc	short getpivot20	;there is a vanished data
	call	store2extended
	mov	ax,[checkingrow]
	inc	ax
	mov	[checkingrow],ax
	pop	cx
	loop	getpivot10

getpivot50:
	mov	ax,[unitrows]
	sub	ax,cx			;ax = rank increased now
	ret

getpivot20:
	pop	cx
	mov	ax,[checkingrow]
	inc	ax
	mov	[checkingrow],ax
	jmp	getpivot10


erasemac	macro
	mov	eax,fs:[esi]
	xor	[di],eax
	add	esi,4
	add	di,4
endm

erasebyothers:
	mov	esi,[GaussLongBase]
	cmp	esi,[GaussLongNow]
	je	eraseret		;no former
erase10:
	mov	bx,fs:[esi]		;word offset of MSB
	mov	ax,fs:[esi+2]		;bit pattern of word
	mov	di,offset BUFFER
	test	ax,[bx+di]
	jz	erase20
	add	esi,4

	mov	cx,[MATdwordsW]		;=dword full row length
	shr	cx,1
	jnc	short erase15
	erasemac
erase15:
	shr	cx,1
	jnc	short erase16
	erasemac
	erasemac
erase16:
	jcxz	erase18
	align	4
erase17:
	erasemac
	erasemac
	erasemac
	erasemac
	loop	erase17
erase18:
	jmp	short erase25
erase20:
	add	esi,dword ptr [MATbytesW4]
erase25:
	cmp	esi,[GaussLongNow]
	jb	erase10		

	mov	di,offset BUFFER
	call	calcMSB
	mov	[di-4],cx
	mov	[di-2],ax
	or	ax,ax
	jz	erasegetans1		;find an answer
	clc
eraseret:
	ret

erasegetans1:				;if the new member becomes an answer
	mov	ah,40h			;write
	mov	bx,[ANShandle]
	mov	cx,[MATbytes]
	mov	dx,offset BUFFER
	int	21h
	jc	ansdiskerror
	inc	[answers]
	stc
	jmp	eraseret


ansdiskerror:
	mov	sp,[spmem]
	mov	bx,AR0
	mov	word ptr [bx],8001h
	jmp	gaussret


storemac	macro
	lodsd
	mov	fs:[edi],eax
	add	edi,4
endm

store2extended:
	mov	si,offset MSBoffset
	mov	edi,[GaussLongNow]
	lodsd				;for MSB data
	mov	fs:[edi],eax
	add	edi,4
	mov	cx,[MATdwordsW]
	shr	cx,1
	jnc	short store15
	storemac
store15:
	shr	cx,1
	jnc	short store16
	storemac
	storemac
store16:
	jcxz	store18
	align	4
store17:
	storemac
	storemac
	storemac
	storemac
	loop	store17
store18:
	mov	[GaussLongNow],edi
	ret


erase_below:
	mov	ax,[checkingrow]
	mov	[RWrow],ax
	mov	cx,[MATsize]
	sub	cx,ax
	jz	short erasebelowret

	push	cx			;/*

	mov	ebx,[GAUSSlongbase]
	mov	cx,[localrank]
	xor	ax,ax
eraseb10:
	cmp	ax,fs:[ebx]
	jae	short eraseb20
	mov	ax,fs:[ebx]
eraseb20:
	add	ebx,dword ptr [MATbytesW4]
	loop	eraseb10
	shr	ax,2
	inc	ax
	mov	[makexorDwordsW],ax
	shl	ax,2
	mov	[loadsavebytes],ax
	sub	ax,[MATbytesW]
	neg	ax
	mov	[makexorskip],ax

	pop	cx			;*/

erasebelowloop:
	push	cx			;/*

	call	load_row
	call	make_xor		;if zero must save as answer and 
	call	save_row		;delete from the matrix

	pop	cx			;*/
	inc	[RWrow]
	loop	erasebelowloop
erasebelowret:
	ret


xormac	macro
	mov	eax,fs:[esi]
	xor	[di],eax
	add	esi,4
	add	di,4
endm

make_xor:
	mov	esi,[GaussLongBase]
	mov	cx,[localrank]
makexor10:
	push	cx
	mov	di,word ptr fs:[esi]	;MSB word adr
	mov	ax,fs:[esi+2]		;mask
	add	di,offset BUFFER
	test	[di],ax
	jz	makexor60

	add	esi,4
	mov	di,offset BUFFER
	mov	cx,[makexordwordsW]
	shr	cx,1
	jnc	short makexor15
	xormac
makexor15:
	shr	cx,1
	jnc	short makexor16
	xormac
	xormac
makexor16:
	jcxz	makexor18
	align	4
makexor17:
	xormac
	xormac
	xormac
	xormac
	loop	makexor17
makexor18:
	add	esi,dword ptr [makexorskip]
	jmp	short makexor70
makexor60:
	add	esi,dword ptr [MATbytesW4]
makexor70:
	pop	cx
	dec	cx
	jnz	makexor10
	ret


load_row:
	mov	ax,[RWrow]
	
	mov	bx,[handle1]
	xor	dx,dx
	cmp	ax,[size1]
	jb	load_row10

	mov	bx,[handle2]
	mov	dx,[size1]
load_row10:
	mov	[handle],bx

	sub	ax,dx
	mov	dx,[MATbytesW]
	mul	dx
	mov	cx,dx
	mov	dx,ax			;cx:dx = file ptr

	mov	ah,42h			;move file ptr
	mov	al,0			;absolute
	mov	bx,[handle]
	int	21h
load_row20:
	mov	ah,3fh			;read
	mov	bx,[handle]
	mov	cx,[loadsavebytes]
	mov	dx,offset BUFFER
	int	21h

	ret


save_row:
	mov	ax,[RWrow]
	
	mov	bx,[handle1]
	xor	dx,dx
	cmp	ax,[size1]
	jb	short save_row10

	mov	bx,[handle2]
	mov	dx,[size1]
save_row10:
	mov	[handle],bx

	sub	ax,dx
	mov	dx,[MATbytesW]
	mul	dx
	mov	cx,dx
	mov	dx,ax			;cx:dx = file ptr

	mov	ah,42h			;move file ptr
	mov	al,0			;absolute
	mov	bx,[handle]
	int	21h
save_row20:
	mov	ah,40h			;write
	mov	bx,[handle]
	mov	cx,[loadsavebytes]
	mov	dx,offset BUFFER
	int	21h

	ret


calcMSB:
	;in: di = base adr
	;out: ax = mask of MSB, cx = offset adr of MSB
	;destroy: nothing

	push	di
	add	di,[MATbytesW]
	sub	di,4
	xor	eax,eax
	mov	cx,[MATdwords]
	std
	repe	scasd
	cld
	je	short calcMSBzero	;ax=cx=0
	add	di,6
	mov	ax,[di]
	or	ax,ax
	jnz	short calcMSB5
	sub	di,2
	mov	ax,[di]
calcMSB5:
	or	ah,ah
	jnz	short calcMSB10		;if on higher
	mov	ah,al
	bsr	cx,ax
	sub	cx,8
	mov	ax,1
	shl	ax,cl
	mov	cx,di
	pop	di
	sub	cx,di
	ret

calcMSB10:
	bsr	cx,ax
	mov	ax,1
	shl	ax,cl
	mov	cx,di
	pop	di
	sub	cx,di
	ret

calcMSBzero:				;if 0 then return with ax=0
	pop	di			;also cx=0 but has no meaning
	ret

	
;
;* Initialize for Gaussian elimination
;
;command#=0

gauss_init:
	;get parameters

	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	[spmem],sp

	xor	eax,eax
	mov_ax	AR1
	shl	eax,10
	mov	[XMEMsize],eax	

	mov_ax	AR2
	shl	ax,1
	mov	[MATsize],ax
	shr	ax,2			;* 2 / 8
	mov	[MATbytesW],ax
	add	ax,4
	mov	[MATbytesW4],ax
	sub	ax,4
	shr	ax,1
	mov	[MATbytes],ax
	mov	[MATwordsW],ax
	shr	ax,1
	mov	[MATwords],ax
	mov	[MATdwordsW],ax
	shr	ax,1
	mov	[MATdwords],ax

	mov	[startrow],0
	mov	[answers],0

	call	setPROTECTfs

	mov	eax,GaussAdr
	mov	[GaussLongbase],eax

	mov	eax,[XMEMsize]
	add	eax,pmodeworkadr
	sub	eax,[GaussLongbase]
	jb	lackofextendedmemory
	xor	edx,edx			;eax = size of freearea
	mov	ecx,dword ptr [MATbytesW4]
	div	ecx			;eax = max number of rows
	movzx	ecx,[MATsize]
	cmp	eax,ecx
	jbe	short checkmemory140
	mov	eax,ecx		
checkmemory140:
	mov	[unitrows],ax

	call	openMATRIXfiles
	call	createANSfile

	mov	ax,[unitrows]
gauss_init_out:
	mov	bx,AR0
	mov	[bx],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

diskerror:
	mov	sp,[spmem]
	mov	ax,8001h	;set -1
	jmp	gauss_init_out

lackofextendedmemory:
	mov	sp,[spmem]
	mov	ax,8002h	;set -2
	jmp	gauss_init_out


createansfile:
	mov	dx,offset ANSFILENAME	;create ANS file
	mov	ah,3ch			;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[ANShandle],ax

	mov	ah,40h			;write matrixsize
	mov	bx,[ANShandle]
	mov	cx,2
	mov	dx,offset MATsize
	int	21h
	jc	diskerror
	ret

ANSFILENAME	db	'PPMPANS.PPM',0


openMATRIXfiles:
	mov	bx,offset MATRIXNAME1
	MOV_AX	AR3
	or	ax,ax
	jz	short openmat170
	sub	bx,2
	add	al,'@'
	mov	[bx],al
openmat170:
	mov	dx,bx
	mov	ah,3dh
	mov	al,2		;read/write
	int	21h
	jc	diskerror
	mov	[handle1],ax
	MOV_AX	AR4
	shl	ax,1
	mov	[size1],ax

	mov	bx,offset MATRIXNAME2
	MOV_AX	AR5
	or	ax,ax
	jz	short openmat180
	sub	bx,2
	add	al,'@'
	mov	[bx],al
openmat180:
	mov	dx,bx
	mov	ah,3dh
	mov	al,2		;read/write
	int	21h
	jc	diskerror
	mov	[handle2],ax
	ret

		db	' :'
MATRIXNAME1	db	'PPMPMAT1.PPM',0
		db	' :'
MATRIXNAME2	db	'PPMPMAT2.PPM',0


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
	jmp	$+2		;flush

	mov	ax,8		;now in protect mode
	mov	fs,ax

	mov	eax,cr0		;reset PE bit
	and	eax,07ffffffeh	;
	mov	cr0,eax		;
	jmp	$+2		;flush

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


align 4
MSBoffset	dw	?
MSBmask		dw	?
BUFFER		dw	maxbufferwords dup(0)

PPMP5END:

CODE	ENDS
END	START
