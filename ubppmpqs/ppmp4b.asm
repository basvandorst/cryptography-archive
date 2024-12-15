;PPMP4b.ASM 2.0
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

; branch

	MOV_AX	AR0		;ARRAY[0] is the command
	MOV	BX,OFFSET CMD_TBL
	SHL	AX,1
	ADD	BX,AX
	JMP	CS:[BX]

CMD_TBL:
	dw	INITIALIZE,SETMATRIX

	even
spmem		dw	?
ssmem		dw	?

drv_off		dw	?
drv_seg		dw	?

FBsize		dw	?		;size of original FactorBase
MATsize		dw	?		;size of matrix
ROWSsize	dw	?		;number of ROW data

histhandle	dw	?
RFBhandle	dw	?
ROWShandle	dw	?
decomphandle	dw	?

handle1		dw	?
size1		dw	?
handle2		dw	?

FBmemberID	dw	?
MATmemberID	dw	?

matrixrightbase	dw	?
databuffer	dw	?


	align	4
XMEMsize		dd	?
RFBbase			dd	?
ROWSbase		dd	?
decompdatabase		dd	?
decomppointerbase	dd	?
historybase		dd	?
freebase		dd	?

totaloperations		dd	?

;
; set matrix
;
;command#=1

setmatrix:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	[spmem],sp

	mov	ax,[MATsize]
	mov	cx,[ROWSsize]
	cmp	cx,ax
	jbe	short setmat5
	mov	cx,ax
setmat5:
	xor	dx,dx			;MATmemberID counter
	mov	ebx,[ROWSbase]
setmat10:
	push	cx
	push	dx
	push	ebx
	mov	ax,fs:[ebx]
	mov	[FBmemberID],ax
	mov	[MATmemberID],dx

	call	clear3rdbuffer
	call	backsubstitution
	call	clear2ndbuffer
	call	set2ndpart
	call	clear1stbuffer
	call	set1stpart
	call	writethisrow

setmat20:
	pop	ebx
	add	ebx,2
	pop	dx
	inc	dx
	pop	cx
	loop	setmat10

	call	fillrests

	call	closefiles

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


fillrests:
	mov	cx,[MATsize]
	sub	cx,[ROWSsize]
	jbe	short fillrestsret
	mov	dx,[ROWSsize]
fillrests10:
	push	cx
	push	dx
	mov	[MATmemberID],dx
	call	clear2ndbuffer
	call	clear1stbuffer
	call	set1stpart
	call	writethisrow
	pop	dx
	inc	dx
	pop	cx
	loop	fillrests10
fillrestsret:
	ret


backsubstitution:
	mov	bx,[databuffer]

	mov	di,[FBmemberID]		;initial set unit vector
	mov	cx,di
	and	cl,7
	shr	di,3
	mov	al,1
	shl	al,cl
	xor	[di+bx],al

	mov	edx,[totaloperations]
	mov	esi,edx
	dec	esi
	shl	esi,2
	add	esi,[historybase]
backsubst10:
	mov	edi,fs:[esi]		;lower = dest, higher = source
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

backsubst30:
	shr	edi,16		;source
	mov	cx,di
	and	cl,7
	shr	di,3
	mov	al,1
	shl	al,cl
	xor	[di+bx],al
	jmp	backsubst20


set2ndpart:
	mov	ebx,[decomppointerbase]
	mov	si,[databuffer]
	mov	cx,[FBsize]
	shr	cx,4			;1/16
set2nd10:
	push	cx
	lodsw
	push	si
	mov	cx,16
set2nd20:
	shr	ax,1
	jnc	set2nd30
	push	ax
	push	cx

countthisX:
	mov	esi,fs:[ebx]		;top of the decomposition data
	mov	cx,fs:[esi]
	jcxz	countthisret
countthis10:
	add	esi,2
	movzx	eax,word ptr fs:[esi]
	shl	eax,1
	add	eax,[RFBbase]
	mov	ax,fs:[eax]		;RFB prime number
	cmp	ax,0ffffh
	je	short countthis20

	push	cx
	mov	cl,al
	and	cl,7
	shr	ax,3
	add	ax,[matrixrightbase]
	mov	di,ax
	mov	al,1
	shl	al,cl
	xor	[di],al
	pop	cx

countthis20:
	loop	countthis10
countthisret:

	pop	cx
	pop	ax
set2nd30:
	add	ebx,4
	loop	set2nd20
	pop	si
	pop	cx
	loop	set2nd10
	ret




set1stpart:
	mov	ax,[MATmemberID]
	mov	cl,al
	and	cl,7
	shr	ax,3
	add	ax,offset BUFFER
	mov	di,ax
	mov	al,1
	shl	al,cl
	mov	[di],al
	ret


clear1stbuffer:
	mov	di,offset BUFFER
clearbuffer10:
	mov	cx,[MATsize]
clearbuffer20:
	shr	cx,5			;1/32
	xor	eax,eax
	rep	stosd
	ret
clear2ndbuffer:
	mov	di,[matrixrightbase]
	jmp	clearbuffer10
clear3rdbuffer:
	mov	di,[databuffer]
	mov	cx,[FBsize]
	jmp	clearbuffer20


writethisrow:
	mov	dx,offset BUFFER
	mov	cx,[MATsize]
	shr	cx,2			;*2/8
	mov	ax,[MATmemberID]
	mov	bx,[handle1]
	cmp	ax,[size1]
	jb	short setrow100
	mov	bx,[handle2]
setrow100:
	mov	ah,40h			;write data
	int	21h
	jc	diskerror
	ret

;

closefiles:
	mov	ax,cs
	mov	ds,ax

	mov	bx,[histhandle]
	or	bx,bx
	jz	short closef10		;not opened
	mov	ah,3eh			;close file
	int	21h
closef10:
	mov	bx,[handle1]
	or	bx,bx
	jz	short closef20		;not opened
	mov	ah,3eh			;close file
	int	21h
closef20:
	mov	bx,[handle2]
	or	bx,bx
	jz	short closef30		;not opened
	mov	ah,3eh			;close file
	int	21h
closef30:
	ret


;
;* Initialize
;
;COMMAND#=0

;	V1 = datadrive
;	V2 = mebers
;	V3 = maxlength
;	V4 = minlength
;	V5 = totallength

INITIALIZE:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	[spmem],sp

	MOV	BX,V1		;adr of drv
	MOV	AX,[BX]
	MOV	[drv_OFF],AX
	MOV	AX,[BX+2]
	MOV	[drv_SEG],AX

	xor	eax,eax
	mov_ax	AR1
	shl	eax,10
	mov	[XMEMsize],eax

	mov_ax	AR2
	shl	ax,1
	mov	[MATsize],ax

	shr	ax,3
	mov	bx,offset BUFFER
	add	bx,ax
	mov	[matrixrightbase],bx
	add	bx,ax
	mov	[databuffer],bx

	call	setPROTECTfs

	call	getRFB
	call	getROWS
	call	getDECOMPDATA
	call	gethistDATA
	call	createMATRIXfiles
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
errorout:
	mov	bx,AR0
	mov	word ptr [bx],ax
	mov	sp,[spmem]
	call	closefiles
	jmp	initout

initlackofdata:
	mov	ax,8002h
	jmp	errorout
lackofmemory:
	mov	ax,8003h
	jmp	errorout


getRFB:
	mov	ax,3d00h		;open for read
	mov	dx,offset RFBfilename
	int	21h
	jc	diskerror
	mov	[RFBhandle],ax

	mov	edi,pmodeworkadr
	mov	[RFBbase],edi
getRFB10:
	mov	dx,offset BUFFER
	mov	cx,400h			;1024bytes unit
	mov	bx,[RFBhandle]
	mov	ah,3fh			;read
	int	21h
	or	ax,ax
	jz	short getRFBeof

	mov	cx,ax
	shr	cx,1
	mov	si,offset BUFFER
getRFB20:
	lodsw
	mov	fs:[edi],ax
	add	edi,2
	loop	getRFB20
	jmp	getRFB10

getRFBeof:
	mov	eax,edi
	add	eax,3
	and	ax,0fffch

	mov	[ROWSbase],eax

	mov	eax,edi
	sub	eax,[RFBbase]
	shr	eax,1
	mov	[FBsize],ax		;original FBsize

	mov	ah,3eh			;close file
	mov	bx,[RFBhandle]
	int	21h
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

	mov	eax,edi
	sub	eax,[ROWSbase]
	shr	eax,1
	mov	[ROWSsize],ax

	mov	ah,3eh			;close file
	mov	bx,[ROWShandle]
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
	add	ebx,3
	and	bx,0fffch
	mov	[historybase],ebx
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

createMATRIXfiles:
	mov	bx,offset MATRIXNAME1
	MOV_AX	AR3
	or	ax,ax
	jz	short createmat10
	sub	bx,2
	add	al,'@'
	mov	[bx],al
createmat10:
	mov	dx,bx
	mov	ah,3ch		;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[handle1],ax
	MOV_AX	AR4
	shl	ax,1
	MOV	[size1],ax

	mov	bx,offset MATRIXNAME2
	MOV_AX	AR5
	or	ax,ax
	jz	short createmat20
	sub	bx,2
	add	al,'@'
	mov	[bx],al
createmat20:
	mov	dx,bx
	mov	ah,3ch		;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[handle2],ax
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



historyfilename	db	'PPMPHIST.PPM',0
RFBFILENAME	db	'PPMPRFB.PPM',0
ROWSFILENAME	db	'PPMPROWS.PPM',0
DECOMPFILENAME	db	'PPMPDCMP.PPM',0
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


	even
BUFFER0		dw	?
BUFFER		dw	maxbufferwords dup(0)
		dw	maxbufferbytes dup(0)

CODE	ENDS
END	START
