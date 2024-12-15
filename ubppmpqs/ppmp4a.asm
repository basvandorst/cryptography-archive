;PPMP4a.ASM 2.0
;	MACHINE LANGUAGE SUBROUTINES
;	FOR PPMPQS
;	1991/92 by YUJI KIDA
;
.386P

code	segment	use16
	assume	cs:code,ds:code

	INCLUDE	UBP.MAC
	include	ppmp.h


inactivesteplog	equ	9


org	100h

start:

;branch

	MOV_AX	AR0		;ARRAY[0] is the command
	MOV	BX,OFFSET CMD_TBL
	SHL	AX,1
	ADD	BX,AX
	JMP	CS:[BX]

CMD_TBL:
	dw	INITIALIZE,GETPRIME,STORE_DATA,SETACTIVE,COUNTCOLUMN
	dw	DELETEROW,GAUSS1,GAUSS2,CUTEXCESSROW,MAKERFB,GETMEMORYSIZE

	even
maxlen		dw	0
P_OFF		dw	?
P_SEG		dw	?
primeadrnow	dd	?

W_OFF		dw	?
W_SEG		dw	?

Drv_OFF		dw	?
Drv_SEG		dw	?

FBsize		dw	?
primes		dw	?
rowstotal	dw	?
rowsnow		dw	?
columnsweight0	dw	?
excessrows	dw	?

heavycolweight	dw	?

offnow		dw	?
primenow	dw	?

count0		dw	?
count1		dw	?
count2		dw	?

inactivenow	dw	?
inactivenext	dw	?
inactivecount	dw	?

primehandle	dw	?
decomhandle	dw	?
histhandle	dw	?

ptrbuffer2	dw	?
ptrbuffer3	dw	?
length1		dw	?
length2		dw	?
spmem		dw	?

didsomesw	dw	0

	align	4
pointernow	dd	?
datanow		dd	?
columncount	dd	?

counterbase	dd	?
pointerbase	dd	?
database	dd	?
activeflgbase	dd	?


;
; get memory size
;
;command#=10

getmemorysize:
	mov	esi,pmodeworkadr-4
	mov	eax,19510701h
	mov	edx,00000400h		;1Kbytes
	mov	cx,31*1024		;max 31Mbytes
checkmemory100:
	inc	eax
	add	esi,edx
	mov	fs:[esi],eax
	cmp	eax,fs:[esi]
	loope	checkmemory100

	mov	esi,pmodeworkadr-4	;chack again for cache memory
	mov	eax,19510701h
	mov	edx,00000400h		;1Kbytes
	mov	cx,31*1024		;max 31Mbytes
checkmemory110:
	inc	eax
	add	esi,edx
	cmp	eax,fs:[esi]
	loope	checkmemory110

	mov	eax,esi
	sub	eax,edx
	sub	eax,pmodeworkadr-4
	shr	eax,10			;Kbytes
	mov	bx,AR0
	mov	cs:[bx],ax
	retf
	

;
; make reduced FactorBase file
; and revived Row number file
;
;command#=9

RFBhandle	dw	?
RFBFILENAME	db	'PPMPRFB.PPM',0
ROWShandle	dw	?
ROWSFILENAME	db	'PPMPROWS.PPM',0

makeRFB:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	ah,3eh			;close history
	mov	bx,[histhandle]
	or	bx,bx
	jz	short makeRFB0
	int	21h
makeRFB0:
	mov	dx,offset RFBFILENAME
	mov	ah,3ch			;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[RFBhandle],ax

	xor	dx,dx			;counter
	mov	ebx,[activeflgbase]
	mov	cx,[FBsize]
makeRFB10:
	push	ebx
	push	cx

	mov	ax,0ffffh		;non use mark
	cmp	byte ptr fs:[ebx],1
	jne	short makeRFB20
	mov	ax,dx
	inc	dx
makeRFB20:
	push	dx
	mov	bx,offset BUFFER
	mov	[bx],ax
	mov	dx,bx
	mov	cx,2
	mov	bx,[RFBhandle]
	mov	ah,40h			;write
	int	21h
	pop	dx
	pop	cx
	pop	ebx
	jc	diskerror	
	inc	ebx
	loop	makeRFB10

	cmp	dx,[inactivenow]
	jne	makeRFBdataerror

	push	dx			;/*

	mov	ah,3eh			;close
	mov	bx,[RFBhandle]
	or	bx,bx
	jz	short makeROWS
	int	21h

makeROWS:
	mov	dx,offset ROWSFILENAME
	mov	ah,3ch			;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[ROWShandle],ax

	xor	dx,dx			;counter
	mov	ebx,[pointerbase]
	mov	cx,[FBsize]
makeROWS10:
	test	dword ptr fs:[ebx],deletedmark4B
	jnz	short makeROWS20

	push	bx
	push	cx
	push	dx

	mov	bx,offset BUFFER
	mov	ax,[FBsize]
	sub	ax,cx
	mov	[bx],ax
	mov	dx,bx
	mov	cx,2
	mov	bx,[ROWShandle]
	mov	ah,40h			;write
	int	21h
	jc	diskerror

	pop	dx
	pop	cx
	pop	bx

	inc	dx

makeROWS20:
	add	ebx,4
	loop	makeROWS10

	pop	ax			;*/
	cmp	ax,dx
	jae	short makeROWS30
	mov	ax,dx
makeROWS30:
	add	ax,matrixunitsize-1
	and	ax,matrixunitmask
	shr	ax,1			;reduced matrix size / 2

makeRFBret:
	mov	bx,AR0
	mov	word ptr [bx],ax

	mov	ah,3eh			;close
	mov	bx,[RFBhandle]
	or	bx,bx
	jz	short makeRFBskip
	int	21h

makeRFBskip:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

makeRFBdiskerror:
	mov	ax,8001h
	jmp	makeRFBret

makeRFBdataerror:
	mov	ax,8002h		;data error
	jmp	makeRFBret

;
; cut excess rows
;
;command#=8

CUTEXCESSROW:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	[excessrows],0
	mov	ax,[rowsnow]
	mov	dx,[FBsize]
	add	dx,[inactivenow]
	sub	dx,[columnsweight0]
	add	dx,8			;for safety
	sub	ax,dx
	jbe	short excessout		;no excess
	mov	[excessrows],ax

	mov	cx,ax			;clear buffer
	shl	cx,1
	mov	di,offset BUFFER
	xor	ax,ax
	rep	stosw
	dec	ax
	stosw				;sentinel

	mov	cx,[rowstotal]
	mov	ebx,[pointerbase]
	xor	dx,dx			;row#
excess10:
	push	cx
	mov	esi,fs:[ebx]
	test	esi,deletedmark4B
	jnz	short excess20

	mov	ax,fs:[esi]		;weight
	mov	di,offset BUFFER
	cmp	ax,[di]
	jb	short excess20
	call	storethisrownumber
excess20:
	add	ebx,4
	inc	dx
	pop	cx
	loop	excess10

	;now delete highest weight rows

	mov	si,offset BUFFER
	mov	cx,[excessrows]
excess50:
	xor	eax,eax
	add	si,2
	lodsw
	or	ax,ax
	jz	short excess60
	shl	eax,2
	add	eax,[pointerbase]
	or	dword ptr fs:[eax],deletedmark4B
	dec	[rowsnow]
excess60:
	loop	excess50
	
excessout:
	mov	ax,[excessrows]
	mov	bx,AR0
	mov	[bx],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

storethisrownumber:
	push	ebx
storethisrow10:
	mov	ebx,[di+4]
	cmp	ax,bx
	jb	short storethisrow50
	mov	[di],ebx
	add	di,4
	jmp	storethisrow10
storethisrow50:
	mov	[di],ax			;weight
	mov	[di+2],dx		;row#
	pop	ebx
	ret


;
; GAUSSian elimination using the rows of weight 2
;
;command#=7

GAUSS2:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	[spmem],sp
	mov	[didsomesw],0

	mov	ebx,[pointerbase]
	mov	cx,[rowstotal]
gauss210:
	push	cx
	push	ebx
	mov	esi,fs:[ebx]
	test	esi,deletedmark4b
	jnz	short gauss220

	mov	ax,fs:[esi]		;weight
	cmp	ax,2
	jne	short gauss220

	or	dword ptr fs:[ebx],deletedmark4B
	dec	[rowsnow]
	call	setpivotnumber
	call	gauss2sub
	call	appendhistory
gauss220:
	pop	ebx
	add	ebx,4
	pop	cx
	loop	gauss210

	mov	ax,[didsomesw]
	mov	bx,AR0
	mov	[bx],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


gauss2sub:
	mov	dx,fs:[esi+2]		;dx = pivot less value
	mov	ax,fs:[esi+4]		;ax = pivot larger value
	
	mov	ebx,[pointerbase]
	mov	cx,[rowstotal]
gauss2submainlp:
	push	cx			;*
	mov	esi,fs:[ebx]
	test	esi,deletedmark4b
	jnz	short gauss2sub20

	mov	cx,fs:[esi]
	jcxz	short gauss2sub20
gauss2sub12:
	add	esi,2
	cmp	ax,fs:[esi]
	jb	short gauss2sub20
	je	short gauss2sub30
	loop	gauss2sub12

gauss2sub20:
	add	ebx,4
	pop	cx			;*
	loop	gauss2submainlp
	ret

gauss2sub30:
	push	eax
	call	replacethis
	call	memooperation
	pop	eax
	jmp	gauss2sub20

replacethis:
	mov	edi,fs:[ebx]		;top adr
	mov	ecx,esi
	sub	ecx,edi
	shr	cx,1
	dec	cx			;data# to be checked
	jz	short replacethisret
replacethislp:
	mov	ax,fs:[esi-2]
	cmp	ax,dx
	jb	short replacethisret
	je	short cutthis2
	mov	fs:[esi],ax
	sub	esi,2
	loop	replacethislp
replacethisret:
	mov	fs:[esi],dx
replacethisret2:
	ret

cutthis2:
	mov	ax,fs:[edi]		;weight
	mov	cx,ax
	sub	ax,2
	mov	fs:[edi],ax
	jz	replacethisret2
	mov	eax,esi
	sub	eax,edi
	shr	ax,1
	sub	cx,ax	
	jz	replacethisret2
cutthis2lp:
	add	esi,2
	mov	ax,fs:[esi]
	mov	fs:[esi-4],ax
	loop	cutthis2lp
	jmp	replacethisret2

;
; GAUSSian elimination using the rows of weight 1
;
;command#=6

GAUSS1:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	[spmem],sp
	mov	[didsomesw],0

gauss1mainlp:
	mov	ebx,[pointerbase]
	mov	cx,[rowstotal]
gauss110:
	push	cx
	push	ebx
	mov	esi,fs:[ebx]
	test	esi,deletedmark4b
	jnz	short gauss120

	mov	ax,fs:[esi]		;weight
	cmp	ax,1
	jne	short gauss120
gaussweight1:
	or	dword ptr fs:[ebx],deletedmark4B
	dec	[rowsnow]
	call	setpivotnumber
	call	gauss1sub
	call	appendhistory
gauss120:
	pop	ebx
	add	ebx,4
	pop	cx
	loop	gauss110
	mov	ax,[didsomesw]
	mov	bx,AR0
	mov	[bx],ax
gauss1out:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

appendhistory:
	mov	dx,offset BUFFER
	mov	cx,[ptrbuffer2]
	sub	cx,dx
	jbe	short historyret
	mov	bx,[histhandle]
	mov	ah,40h			;write
	int	21h
	jc	gauss1diskerror
	mov	[didsomesw],1
historyret:
	ret

	
setpivotnumber:
	mov	eax,ebx			;pivot element number
	sub	eax,[pointerbase]
	shr	eax,2
	mov	di,offset BUFFER0
	stosw
	mov	[ptrbuffer2],di
	ret


gauss1sub:
	mov	ax,fs:[esi+2]		;ax = pivot value
	mov	ebx,[pointerbase]
	mov	cx,[rowstotal]
gauss1submainlp:
	push	cx			;*
	mov	esi,fs:[ebx]
	test	esi,deletedmark4b
	jnz	short gauss1sub20

	mov	cx,fs:[esi]
	jcxz	short gauss1sub20
gauss1sub12:
	add	esi,2
	cmp	ax,fs:[esi]
	jb	short gauss1sub20
	je	short gauss1sub30
	loop	gauss1sub12

gauss1sub20:
	add	ebx,4
	pop	cx			;*
	loop	gauss1submainlp
	ret

gauss1sub30:
	push	eax
	call	cutthis
	call	memooperation
	pop	eax
	jmp	gauss1sub20

cutthis:
	mov	edi,fs:[ebx]		;top adr
	mov	cx,fs:[edi]
	dec	cx
	mov	fs:[edi],cx
	inc	cx			;total data#
	mov	eax,esi
	sub	eax,edi
	shr	ax,1			;checked data#
	sub	cx,ax			;remaining data#
	jbe	short cutthisret	;if last data
cutthislp:
	mov	ax,fs:[esi+2]
	mov	fs:[esi],ax
	add	esi,2
	loop	cutthislp
cutthisret:
	ret

memooperation:
	mov	di,[ptrbuffer2]
	mov	eax,ebx
	sub	eax,[pointerbase]
	shr	eax,2
	stosw				;memo target
	mov	ax,[BUFFER0]
	stosw				;memo pivot
	mov	[ptrbuffer2],di
	ret

GAUSS1diskerror:
	mov	sp,[spmem]
	mov	bx,AR0
	mov	word ptr [bx],8001h
	jmp	gauss1out


;
; delete row containing 1 data column
;
;command#=5

deleterow:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	[count0],0
	mov	[count1],0

	mov	cx,[FBsize]
	mov	ebx,[counterbase]
delrow10:
	cmp	word ptr fs:[ebx],1
	ja	short delrow20
	je	short delrow15
	inc	[count0]
	jmp	short delrow20
delrow15:
	inc	[count1]
	movzx	eax,word ptr fs:[ebx+2]
	shl	eax,2
	add	eax,[pointerbase]
	or	dword ptr fs:[eax],deletedmark4b
	dec	[rowsnow]
delrow20:
	add	ebx,4
	loop	delrow10

	mov	ax,[count0]
	mov	bx,AR0
	mov	cs:[bx],ax
	mov	ax,[count1]
	mov	bx,AR1
	mov	cs:[bx],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf	


;
; count column weight
;
;command#=4

countcolumn:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	edi,[counterbase]
	xor	eax,eax			
	mov	cx,[FBsize]
countc10:				;clear counter
	mov	fs:[edi],eax
	add	edi,4
	loop	countc10

	mov	ebx,[pointerbase]
	mov	cx,[rowstotal]
	xor	dx,dx			;row#
countc20:
	push	cx			;/*

	mov	esi,fs:[ebx]
	test	esi,deletedmark4b
	jnz	short countc50

	mov	cx,fs:[esi]
	jcxz	short countc50
countc30:
	add	esi,2
	movzx	eax,word ptr fs:[esi]
	shl	eax,2
	add	eax,[counterbase]
	inc	word ptr fs:[eax]	;count
	mov	fs:[eax+2],dx		;row#
	loop	countc30
countc50:
	inc	dx
	add	ebx,4
	pop	cx			;*/
	loop	countc20

	mov	[count0],0
	mov	[count1],0

	mov	esi,[counterbase]
	mov	cx,[FBsize]
countc60:
	cmp	word ptr fs:[esi],1
	jbe	short countc70
countc65:
	add	esi,4
	loop	countc60
	jmp	short countc90

countc70:
	je	short countc80
	inc	[count0]
	jmp	countc65
countc80:
	inc	[count1]
	jmp	countc65

countc90:
	mov	ax,[count0]
	mov	bx,AR0
	mov	[bx],ax
	mov	[columnsweight0],ax

	mov	ax,[count1]
	mov	bx,AR1
	mov	[bx],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

;
; set active/inactive
;
;command#=3

SETACTIVE:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	;count column weight

	mov	edi,[counterbase]
	xor	eax,eax			
	mov	cx,[FBsize]
setact10:				;clear counter
	mov	fs:[edi],eax
	add	edi,4
	loop	setact10

	mov	ebx,[pointerbase]
	mov	cx,[rowstotal]
setact20:
	push	cx			;*

	mov	esi,fs:[ebx]
	test	esi,deletedmark4b
	jnz	short setact50

	mov	cx,fs:[esi]
	jcxz	short setact50
setact30:
	add	esi,2
	movzx	eax,word ptr fs:[esi]
	shl	eax,2
	add	eax,[counterbase]
	inc	word ptr fs:[eax]	;count
	loop	setact30
setact50:
	add	ebx,4
	pop	cx			;*
	loop	setact20

	;count rows for each weight
setact100:
	mov	di,offset BUFFER
	mov	cx,1024			;max column weight
	xor	ax,ax
	rep	stosw

	xor	dx,dx			;maxweight
	mov	ebx,[counterbase]
	mov	cx,[FBsize]
setact110:
	mov	ax,fs:[ebx]
	cmp	ax,1024
	jb	short setact120
	mov	ax,1023
setact120:
	cmp	ax,dx
	jbe	short setact130
	mov	dx,ax
setact130:
	shl	ax,1
	add	ax,offset BUFFER
	mov	si,ax
	inc	word ptr [si]		;count this
	add	ebx,4
	loop	setact110

	;decide heavy weight

	or	dx,dx
	jz	setactdone

	mov	bx,dx
	mov	cx,dx
	shl	bx,1
	add	bx,offset BUFFER+2
	mov	ax,[inactivenow]
	push	ax			;/*
setact200:
	sub	bx,2
	add	ax,[bx]
	cmp	ax,[inactivenext]
	jae	short setact220
	loop	setact200
	jmp	short setact225
setact220:
	mov	ax,[inactivenext]
setact225:
	mov	[inactivenow],ax
	mov	dx,ax
	mov	ax,[FBsize]
	sub	ax,dx
	shr	ax,inactivesteplog
	inc	ax			;avoid 0
	add	ax,dx
	cmp	ax,[FBsize]
	jbe	short setact230
	mov	ax,[FBsize]
setact230:
	mov	[inactivenext],ax

	mov	ax,bx
	sub	ax,offset BUFFER
	shr	ax,1			;define heavy weight >= ax
	mov	[heavycolweight],ax

	;make inactive

	pop	dx			;*/
	mov	ax,[inactivenow]
	sub	ax,dx
	jz	short setact330
	mov	[inactivecount],ax

	xor	dx,dx			;column#
	mov	ebx,[counterbase]
	dec	dx
	sub	ebx,4
setact310:
	inc	dx
	add	ebx,4
	mov	ax,fs:[ebx]
	cmp	ax,[heavycolweight]
	jb	setact310
	call	deleteDX
	dec	[inactivecount]
	jnz	setact310

setact330:
	mov	ax,[heavycolweight]
setactret:
	mov	bx,AR1
	mov	[bx],ax

	mov	ax,[inactivenow]
	mov	bx,AR0
	mov	[bx],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

setactdone:
	xor	ax,ax
	jmp	setactret

deleteDX:
	push	ebx
	xor	ebx,ebx
	mov	bx,dx
	add	ebx,[activeflgbase]
;	cmp	byte ptr fs:[ebx],1
;	je	short deldxerr		;already inactive
	mov	byte ptr fs:[ebx],1

	mov	ebx,[pointerbase]
	mov	cx,[rowstotal]
deldx10:
	push	cx
	mov	esi,fs:[ebx]
	test	esi,deletedmark4B
	jnz	short deldx40

	mov	cx,fs:[esi]
	jcxz	short deldx40
deldx20:
	add	esi,2
;deldx30:
	cmp	dx,fs:[esi]
	jb	short deldx40
	je	short deldx50
	loop	deldx20	
deldx40:
	add	ebx,4
	pop	cx
	loop	deldx10

deldxret:
	pop	ebx
	ret

deldx50:
	mov	edi,fs:[ebx]		;top adr
	dec	word ptr fs:[edi]	;dec elements#
	dec	cx
	jcxz	deldx40			;if last element
	push	esi
	push	cx
deldx60:
	mov	ax,fs:[esi+2]		;slide down
	mov	fs:[esi],ax		;
	add	esi,2
	loop	deldx60
	pop	cx
	pop	esi
	jmp	deldx40

;
; set row data
;
;COMMAND#=2

STORE_DATA:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	si,offset decomfilename
	call	setpathname
	mov	dx,offset BUFFER
	mov	ax,3d00h		;open for read
	int	21h
	jc	storediskerror
	mov	[decomhandle],ax

	mov	[rowstotal],0

	;read decomposition data and set in memory
storelp:
	mov	dx,offset BUFFER0
	mov	cx,2
	mov	bx,[decomhandle]
	mov	ah,3fh
	int	21h
	or	ax,ax
	jz	decomeof

	mov	bx,offset BUFFER0
	lea	dx,[bx+2]
	mov	cx,[bx]
	shl	cx,1
	mov	bx,[decomhandle]
	mov	ah,3fh
	int	21h

	mov	ebx,[pointernow]
	mov	edi,[datanow]
	mov	fs:[ebx],edi
	add	ebx,4
	mov	[pointernow],ebx

	; set to protect memory neglecting duplicate data

 	mov	si,offset BUFFER0
	lodsw
	mov	fs:[edi],ax
	add	edi,2
	mov	cx,ax
	jcxz	short setdatajp
setdata2:				;cut small primes
	lodsw
	cmp	ax,[inactivenow]
	jae	short setdata6
	loop	setdata2
	jmp	short setdatajp

setdata5:
	lodsw
setdata6:
	dec	cx
	jz	short setdata20
setdatalp:
	mov	dx,ax			;old value
	lodsw
	cmp	ax,dx
	jne	short setdata10
	dec	cx
	jz	short setdatajp
	jmp	setdata5
setdata10:
	mov	fs:[edi],dx
	add	edi,2
	loop	setdatalp
setdata20:
	mov	fs:[edi],ax
	add	edi,2
setdatajp:
	mov	ebx,[datanow]
	mov	[datanow],edi
	sub	edi,ebx
	shr	edi,1
	dec	di
	mov	fs:[ebx],di		;# of elements
	inc	[rowstotal]
	jmp	storelp


decomeof:
	mov	ax,[rowstotal]
	mov	[rowsnow],ax

	mov	ah,3eh			;close
	mov	bx,[decomhandle]
	or	bx,bx
	jz	short decompeofskip
	int	21h

decompeofskip:

	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	RETF

storediskerror:
	mov	bx,AR0
	mov	word ptr [bx],8001h
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

decomfilename	db	'PPMPDCMP.PPM',0

;
; get and set primes
;
;command#=1

getprime:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	si,offset primefilename
	call	setpathname
	mov	dx,offset BUFFER
	mov	ax,3d00h		;open for read
	int	21h
	jc	getprimediskerror
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
	add	edi,4
	loop	prime50
	jmp	primelp

primeeof:
	mov	ah,3eh
	mov	bx,[primehandle]
	or	bx,bx
	jz	short primeeofskip
	int	21h

primeeofskip:

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

getprimediskerror:
	mov	bx,AR0
	mov	word ptr [bx],8001h
	jmp	primeeof

primefilename	db	'PPMPPRMS.PPM',0


;
;* Initialize
;COMMAND#=0

;	V1 = P
;	V2 = W#
;	V3 = datadrivename
;	AR[1]	TOTAL SIZE \ 2
;	AR[2]	DRIVE NUMBER

INITIALIZE:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	MOV	BX,V1		;adr of P
	MOV	AX,[BX]
	MOV	[P_OFF],AX
	MOV	AX,[BX+2]
	MOV	[P_SEG],AX

	MOV	BX,V2		;adr of W#
	MOV	AX,[BX]
	MOV	[W_OFF],AX
	MOV	AX,[BX+2]
	MOV	[W_SEG],AX

	MOV	BX,V3		;set the address of Drv$
	MOV	AX,[BX]
	MOV	[Drv_OFF],AX
	MOV	AX,[BX+2]
	MOV	[Drv_SEG],AX

	MOV_AX	AR1
	shl	ax,1
	mov	[FBsize],ax
	dec	ax
	mov	[primes],ax
	inc	ax

	shr	ax,5			;start from 1/32(//)
	sub	ax,16
	mov	[inactivenext],ax
	add	ax,16
	shr	ax,1
	mov	[inactivenow],ax

	mov	eax,primeadr
	mov	[primeadrnow],eax
	movzx	ebx,[FBsize]
	mov	ecx,ebx
	shl	ebx,2
	add	eax,ebx
	mov	[activeflgbase],eax
	add	eax,ecx
	mov	[counterbase],eax
	add	eax,ebx
	mov	[pointerbase],eax
	mov	[pointernow],eax
	add	eax,ebx
	mov	[database],eax
	mov	[datanow],eax

	call	setPROTECTfs

	mov	esi,[activeflgbase]	;set active/inactive
	mov	al,1
	mov	cx,[inactivenow]
init80:
	mov	fs:[esi],al
	inc	esi
	loop	init80

	xor	al,al
	mov	cx,[FBsize]
	sub	cx,[inactivenow]
init90:
	mov	fs:[esi],al
	inc	esi
	loop	init90

inithistoryfile:
	mov	dx,offset historyfilename
	mov	ah,3ch			;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[histhandle],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

diskerrorpop2:
	add	sp,2
diskerror:
	mov	bx,AR0
	mov	word ptr [bx],8001h
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


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


historyfilename	db	'PPMPHIST.PPM',0

	even
BUFFER0	dw	?
BUFFER	dw	2000h dup(0)


CODE	ENDS
END	START
