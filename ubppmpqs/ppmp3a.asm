;PPMP3a.ASM 2.0
;	MACHINE LANGUAGE SUBROUTINES
;	FOR PPMPQS
;	1991/92 by YUJI KIDA
;
.386P

code	segment	use16
	assume cs:code,ds:code


	INCLUDE	UBP.MAC
	include	ppmp.h

	org	100h
start:
	JMP	START0


	align	4
R1_OFF	DW	?
R1_SEG	DW	?
R2_OFF	DW	?
R2_SEG	DW	?
X_OFF	DW	?
X_SEG	DW	?

lpptop		dd	?
lppptr		dd	?
lppover		dd	?
lppcount	dd	?
baseXbytes	dw	?,0
baseunitbytes	dw	?,0

basefileptr	dd	?
getreladr	dd	?

basehandle	dw	?
combihandle	dw	?

spmem		dw	?

R1_buffer	db	4 dup(?)
R2_buffer	db	4 dup(?)
X_buffer	db	maxXbytes dup(?)

bufferptr	dw	?
buffer1		db	COMBIUNITBYTES dup (?)
buffer2		db	COMBIUNITBYTES dup (?)

elimcount	dd	?
exchangesw	db	?


;* branching


START0:
	MOV_AX	AR0		;ARRAY[0] is the command number
	MOV	BX,offset CMD_TBL
	SHL	AX,1
	ADD	BX,AX
	JMP	CS:[BX]

CMD_TBL:
	dw	ALLINIT,lppINIT,SETDATA,QSORT,ISORT
	dw	ELIMINATION,GETRELATION,GETBASES,CLOSEHANDLE


; sweep down the '1'

sweep1:
	mov	esi,[lpptop]
	mov	edi,ebx			;input from the main
	mov	ebx,edi			;last '1'
	mov	edx,RAMUNITBYTES
sweepLP:
	mov	eax,1

	;find '1' from the top
sweep10:
	add	esi,edx
	cmp	eax,fs:[esi]
	jne	sweep10

	;find non '1' from the bottom
sweep20:
	sub	edi,edx
	cmp	eax,fs:[edi]
	je	sweep20
	cmp	esi,edi
	ja	sweepret

	mov	eax,fs:[esi]
	xchg	eax,fs:[edi]
	mov	fs:[esi],eax
	mov	eax,fs:[esi+4]
	xchg	eax,fs:[edi+4]
	mov	fs:[esi+4],eax
	mov	eax,fs:[esi+8]
	xchg	eax,fs:[edi+8]
	mov	fs:[esi+8],eax
	mov	ebx,edi			;last '1'
	jmp	sweepLP
sweepret:
	ret

	
; check whether the sorting worked well
check:
	mov	esi,[lpptop]
	add	esi,12
	mov	ecx,[lppcount]
	dec	ecx
check10:
	mov	eax,fs:[esi]
	add	esi,12
	cmp	eax,fs:[esi]
	jb	check20
	dec	ecx
	jnz	check10
	clc
	ret
check20:
	stc
	ret

;
; close&delete base- and combination- files
; command#=8

CLOSEHANDLE:
	mov	ax,cs
	mov	ds,ax
	mov	[spmem],sp

	mov	ah,3eh
	mov	bx,[basehandle]
	int	21h

	mov	ah,3eh
	mov	bx,[combihandle]
	int	21h

	mov	ah,41h
	mov	dx,offset BASEFILENAME
	int	21h

	mov	ah,41h
	mov	dx,offset COMBIFILENAME
	int	21h

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

;
; get combination members
;
GETBASES:
	mov	ax,cs
	mov	ds,ax

	mov	si,[bufferptr]
	mov	dx,[si]
	mov	cx,[si+2]	;cx:dx = file ptr
	mov	ax,dx
	or	ax,cx
	jz	getbasesnomore
	add	si,4
	mov	[bufferptr],si
	mov	ah,42h		;move file ptr
	mov	al,0		;absolute
	mov	bx,[basehandle]
	int	21h

	mov	ah,3fh		;read handle
	mov	dx,offset R1_buffer
	mov	cx,[baseunitbytes]
	int	21h

	;set R1,R2,X,F

	les	di,dword ptr [R1_off]
	mov	eax,dword ptr [R1_buffer]
	mov	es:[di+2],eax
	mov	cx,1
	xor	ax,ax
	or	eax,eax
	jz	short getbase50
	inc	cx
getbase50:
	mov	es:[di],cx

	les	di,dword ptr [R2_off]
	mov	eax,dword ptr [R2_buffer]
	mov	es:[di+2],eax
	mov	cx,1
	xor	ax,ax
	or	eax,eax
	jz	short getbase60
	inc	cx
getbase60:
	mov	es:[di],cx

	les	di,dword ptr [X_off]
	mov	si,offset X_buffer
	mov	cx,[si]
	and	cx,LENMASK
	inc	cx
	rep	movsw

getbasesret:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

getbasesnomore:
	les	di,dword ptr [R1_off]	;set R1=0
	mov	word ptr es:[di],0
	jmp	getbasesret


;
; find relation
; command#=6

GETRELATION:
	mov	ax,cs
	mov	ds,ax
	mov	[spmem],sp

getrelationLP:
	mov	ebx,[getreladr]
	sub	ebx,RAMUNITBYTES
	mov	[getreladr],ebx

	cmp	dword ptr fs:[ebx],1
	jne	short nomorerelation

	mov	eax,fs:[ebx+8]
	or	eax,eax
	jz	getrelationLP		;erased data

	mov	dx,offset buffer1
	call	getcombi
	mov	si,offset buffer1
	mov	[bufferptr],si
getrelation50:
	add	si,4
	cmp	dword ptr [si-4],COMBIMASK
	jb	getrelation50
	xor	dword ptr [si-4],COMBIMASK
	mov	dword ptr [si],0	;end mark

getrelationret:
	mov	ax,ss
	mov	ds,ax
	retf

nomorerelation:
	mov	bx,AR0
	mov	word ptr [bx],0
	jmp	getrelationret

		
;
; pseudo-Gaussian elimination
; command#=5

ELIMINATION:
	mov	ax,cs
	mov	ds,ax
	mov	[spmem],sp
elimIN:
	mov	[exchangesw],0
	mov	ebx,[lpptop]
	add	ebx,RAMUNITBYTES

elimLP1:
	mov	eax,fs:[ebx]	;R
	cmp	eax,1
	je	short elimdone
	mov	esi,ebx
elimLP2:
	add	esi,RAMUNITBYTES
	cmp	eax,fs:[esi]	;next R
	je	short elimmatch
	mov	ebx,esi
	jmp	short elimLP1

elimdone:
	call	sweep1		;sweep down the '1's (use ebx)
	mov	eax,ebx
	sub	eax,[lpptop]
	xor	edx,edx
	mov	ecx,RAMUNITBYTES
	div	ecx
	dec	eax
	mov	[elimcount],eax

	cmp	[exchangesw],0
	je	elim_alldone

	mov	ecx,eax
	push	ecx
	mov	ebx,1
	call	qsortmain
	pop	ecx
;	call	ssortmain
	call	isortmain
;	call	check
	jmp	elimIN

elim_alldone:
	call	number_relations

	mov	eax,[lppover]
	mov	[getreladr],eax

	lds	si,dword ptr [X_OFF]
	mov	[si+2],ecx
	xor	ax,ax
elim90:
	jecxz	short elim100
	inc	ax
	shr	ecx,16
	jmp	elim90
elim100:	
	mov	[si],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

elimmatch:
	mov	[exchangesw],-1
elimmatchLP:
	mov	ecx,fs:[ebx+4]
	mov	edx,fs:[esi+4]
	cmp	ecx,edx
	je	short fullmatch
;	cmp	eax,fs:[esi+RAMUNITBYTES]
;	jne	short halfmatch
;	cmp	edx,fs:[esi+RAMUNITBYTES+4]
;	je	short fullmatch_next
halfmatch:
	mov	fs:[esi],edx	;new R1 = max of old R2 & R2'
	mov	fs:[esi+4],ecx	;new R2 = min of old R2 & R2'
	jmp	short rewritecombi

fullmatch:
	mov	dword ptr fs:[esi],1	;new R1 = 1
	mov	dword ptr fs:[esi+4],1	;new R2 = 1
rewritecombi:
	push	ebx			;/*
	mov	eax,fs:[esi+8]
	mov	edx,offset buffer1
	call	getcombi
	pop	ebx			;*/
	push	ebx			;/*
	mov	eax,fs:[ebx+8]
	mov	edx,offset buffer2
	call	getcombi
	push	si			;/**
	call	linkcombi
	pop	si			;**/
	sub	di,offset buffer1
	cmp	di,COMBIUNITBYTES
	ja	short toolonglink
	call	putnewcombi
	mov	fs:[esi+8],eax		;new file ptr
	pop	ebx			;*/
	jmp	elimLP2

fullmatch_next:
	push	ebx			;/*
	mov	dword ptr fs:[esi],1	;new R1 = 1
	mov	dword ptr fs:[esi+4],1	;new R2 = 1
	mov	eax,fs:[esi+8]
	mov	edx,offset buffer1
	call	getcombi
	mov	eax,fs:[esi+RAMUNITBYTES+8]
	mov	edx,offset buffer2
	call	getcombi
	push	si			;/**
	call	linkcombi
	pop	si			;**/
	sub	di,offset buffer1
	cmp	di,COMBIUNITBYTES
	ja	short toolonglink
	call	putnewcombi
	mov	fs:[esi+8],eax		;new ptr
	pop	ebx			;*/
	mov	eax,fs:[ebx]
	add	esi,RAMUNITBYTES
	jmp	elimmatchLP

toolonglink:
	mov	dword ptr fs:[esi],1	;erased marks
	mov	dword ptr fs:[esi+4],1	;
	mov	dword ptr fs:[esi+8],0	;
	pop	ebx
	jmp	elimLP2

getcombi:
	cmp	eax,COMBIMASK
	jae	short getcombidirect

	push	dx
	mov	dx,ax
	shr	eax,16
	mov	cx,ax		;cx:dx = file ptr
	mov	ah,42h		;move pointer
	mov	al,0		;absolute
	mov	bx,[combihandle]
	int	21h

	pop	dx
	mov	ah,3fh		;read handle
	mov	cx,COMBIUNITBYTES
	int	21h
	ret
getcombidirect:
	mov	bx,dx
	mov	[bx],eax
	ret


linkcombi:
	mov	di,offset buffer1 - 4
	mov	si,offset buffer2
linkcombi10:
	add	di,4
	cmp	dword ptr [di],COMBIMASK
	jb	linkcombi10
	xor	dword ptr [di],COMBIMASK
	add	di,4
linkcombi20:
	mov	eax,[si]
	mov	[di],eax
	add	si,4
	add	di,4
	cmp	eax,COMBIMASK
	jb	linkcombi20
	ret				;di = next of endmark
	
putnewcombi:
	push	di		;/* size
	mov	ah,42h		;move pointer
	mov	al,2		;to the end
	mov	bx,[combihandle]
	xor	cx,cx
	xor	dx,dx
	int	21h

	pop	cx		;*/
	push	dx		;/* dx:ax = new ptr
	push	ax		;/**

	mov	dx,offset buffer1
	mov	ah,40h		;write handle
	int	21h
	cmp	ax,cx
	jne	short combidiskfull
	pop	eax		;**/ */ eax = new ptr
	clc
	ret

writeinitcombi:
	mov	dx,offset buffer1
	mov	ah,40h		;write handle
	mov	bx,[combihandle]
	mov	cx,4
	int	21h
	cmp	ax,cx
	jne	short combidiskfull
	clc
	ret

combidiskfull:
	mov	word ptr ds:[AR0],8001h
	mov	sp,cs:[spmem]
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


number_relations:
	mov	eax,[elimcount]
	mov	edx,RAMUNITBYTES
	mul	edx
	add	eax,[lpptop]
	mov	ebx,eax
	xor	ecx,ecx
numrel10:
	add	ebx,RAMUNITBYTES
	cmp	ebx,[lppover]
	jae	short numrel100

	cmp	dword ptr fs:[ebx+8],0
	je	numrel10		;erased data
	inc	ecx
	jmp	numrel10
numrel100:
	mov	eax,ecx
	ret			;eax = ecx = result


;
; insertion sort wrt R1 as 1stKey and R2 as 2ndKey
; command#=4

copySIDI	macro
	mov	eax,fs:[esi]	;12 bytes
	mov	fs:[edi],eax
	mov	eax,fs:[esi+4]
	mov	fs:[edi+4],eax
	mov	eax,fs:[esi+8]
	mov	fs:[edi+8],eax
endm

transadr	macro
	shl	eax,2		;mult 12
	mov	edx,eax		;
	shl	eax,1		;
	add	eax,edx		;
	add	eax,[lpptop]
endm

ISORT:
	mov	ax,cs
	mov	ds,ax
	mov	[spmem],sp

	mov	ecx,[lppcount]
;	call	ssortmain
	call	isortmain
;	call	check

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

	align	4
isortmain:
	dec	ecx		;loop times
	mov	ebx,2*RAMUNITBYTES
	add	ebx,[lpptop]	;ebx = adr of a[2]
isort10:
	; if a[i] <= a[i-1] then skip this

	mov	eax,fs:[ebx]
	cmp	eax,fs:[ebx-RAMUNITBYTES]
	ja	short isort15
	jb	isortskip
	mov	eax,fs:[ebx+4]
	cmp	eax,fs:[ebx-RAMUNITBYTES+4]
	jae	isortskip		;R2 is reverse ordered
isort15:
	; w:= a[i]; a[0]:= w;

	mov	esi,ebx
	mov	edi,_W
	copySIDI

	mov	esi,_W
	mov	edi,[lpptop]
	copySIDI

	; j:= i-1;

	mov	esi,ebx
	sub	esi,RAMUNITBYTES

	; while w > a[j] do

	mov	edx,fs:[_W+4]
isort20:
	mov	eax,fs:[_W]
	cmp	eax,fs:[esi]
	jb	short isort50
	ja	short isort30
	cmp	edx,fs:[esi+4]
	jae	short isort50		;R2's are reverse ordered
isort30:
	; a[j+1]:= a[j]; j:= j-1;

	mov	edi,esi
	add	edi,RAMUNITBYTES
	copySIDI
	sub	esi,RAMUNITBYTES
	jmp	isort20
isort50:
	; a[j+1]:= w;

	mov	edi,esi
	add	edi,RAMUNITBYTES
	mov	esi,_W
	copySIDI

isortskip:
	add	ebx,RAMUNITBYTES
	dec	ecx
	jnz	isort10
	ret

;
; shell sort
;

	align	4
varH	dd	?
varN	dd	?
Hskip	dd	?

ssortmain:
	; h:= 1;
	; while h<n do h:= h*3+1;

	mov	[varN],ecx
	mov	eax,1
ssort5:
	mov	edx,eax
	shl	eax,1
	add	eax,edx
	inc	eax
	cmp	eax,ecx		;//qsortcutoff
	jb	ssort5
	mov	[varH],eax

	; while h>1 do
ssort8:
	mov	eax,[varH]
	cmp	eax,1
	jbe	ssortout

	; h:= h div 3;

	xor	edx,edx
	mov	ebx,3
	div	ebx
	mov	[varH],eax
	mov	edx,RAMUNITBYTES
	mul	edx
	mov	[Hskip],eax

	; for i:= h+1 to n do

	mov	ecx,[varN]
	mov	eax,[varH]
	sub	ecx,eax
	inc	eax
	transadr
	mov	ebx,eax
ssort10:
	push	ecx		;/*

	; w:= a[i]; j:= i-h;

	mov	esi,ebx
	mov	edi,_W
	copySIDI

	mov	esi,ebx
	sub	esi,[Hskip]

	; while w>a[j] do
ssort30:
	mov	eax,fs:[_W]
	cmp	eax,fs:[esi]
	jb	short ssortLABEL1
	ja	short ssort40
	mov	eax,fs:[_W+4]
	cmp	eax,fs:[esi+4]
	jae	short ssortLABEL1
ssort40:
	; a[j+h]:= a[j]; j:= j-h;

	mov	edi,esi
	add	edi,[Hskip]
	copySIDI

	sub	esi,[Hskip]

	; if j<=0 then goto 1

	cmp	esi,[lpptop]
	ja	ssort30

	; a[j+h]:= w;

ssortLABEL1:
	mov	edi,esi
	add	edi,[Hskip]
	mov	esi,_W
	copySIDI
ssort50:
	add	ebx,RAMUNITBYTES
	pop	ecx			;*/
	dec	ecx
	jnz	ssort10
	jmp	ssort8
ssortout:
	ret

;
; quick sort
; command#=3

QSORT:
	mov	ax,cs
	mov	ds,ax
	mov	[spmem],sp

	mov	dx,offset COMBIFILENAME	;create combination file
	mov	ah,3ch		;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[combihandle],ax
	mov	bx,ax
	call	writeinitcombi		;dummy

	mov	eax,[lppptr]
	mov	[lppover],eax

	mov	dword ptr fs:[eax],1	;sentinel

	sub	eax,[lpptop]
	xor	edx,edx
	mov	ecx,RAMUNITBYTES
	div	ecx
	dec	eax
	mov	[lppcount],eax		;# of data
	mov	ecx,eax

	;go to qsort

	mov	ebx,1		;start index
	mov	ecx,[lppcount]	;end index
	call	qsortmain

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

;
; quick sort
;

_s	equ	0
_e	equ	4
_i	equ	8
_j	equ	12
_pivot	equ	16

align	4
qsortmain:				;ebx=S,ecx=E
	push	ebp
	sub	sp,24

	mov	bp,sp
	mov	[bp+_s],ebx
	mov	[bp+_i],ebx
	mov	[bp+_e],ecx
	mov	[bp+_j],ecx
	mov	eax,ebx
	add	eax,qsortcutoff
	cmp	eax,ecx
	jae	qsortret
	sub	eax,qsortcutoff
	add	eax,ecx
	shr	eax,1		;eax=(E+S)/2
	transadr
	mov	eax,fs:[eax]
	mov	[bp+_pivot],eax
qsort10:
	mov	eax,[bp+_i]
	transadr
	mov	eax,fs:[eax]
	cmp	eax,[bp+_pivot]
	jbe	short qsort20
	inc	dword ptr [bp+_i]
	jmp	qsort10
qsort20:			
	mov	eax,[bp+_j]
	transadr
	mov	eax,fs:[eax]
	cmp	eax,[bp+_pivot]
	jae	short qsort30
	dec	dword ptr [bp+_j]
	jmp	qsort20
qsort30:
	mov	eax,[bp+_i]
	cmp	eax,[bp+_j]
	ja	short qsort50
	transadr
	mov	esi,eax

	mov	eax,[bp+_j]
	transadr
	mov	edi,eax

	mov	eax,fs:[esi]
	xchg	eax,fs:[edi]
	mov	fs:[esi],eax
	mov	eax,fs:[esi+4]
	xchg	eax,fs:[edi+4]
	mov	fs:[esi+4],eax
	mov	eax,fs:[esi+8]
	xchg	eax,fs:[edi+8]
	mov	fs:[esi+8],eax

	inc	dword ptr [bp+_i]
	dec	dword ptr [bp+_j]
qsort50:
	mov	eax,[bp+_i]
	cmp	eax,[bp+_j]
	jbe	qsort10
	mov	ebx,[bp+_s]
	mov	ecx,[bp+_j]
	call	qsortmain
	mov	ebx,[bp+_i]
	mov	ecx,[bp+_e]
	call	qsortmain
qsortret:
	add	sp,24
	pop	ebp
	ret

;
; set lpp data
; command#=2

SETDATA:
	mov	cs:[spmem],sp

	;set R1,R2,X,F on buffer

	lds	si,dword ptr cs:[R1_OFF]
	call	getEAX
	mov	dword ptr cs:[R1_buffer],eax	;set R1

	lds	si,dword ptr cs:[R2_OFF]
	call	getEAX
	mov	dword ptr cs:[R2_buffer],eax	;set R2

	mov	ax,cs			;set X
	mov	es,ax
	mov	di,offset X_buffer
	lds	si,dword ptr cs:[X_OFF]
	mov	cx,[si]
	inc	cx
	rep	movsw

	;write to disk

	mov	ax,cs
	mov	ds,ax
	mov	bx,[basehandle]
	mov	dx,offset R1_buffer
	mov	cx,[baseunitbytes]
	mov	ah,40h			;write handle
	int	21h
	jc	diskerror

	;

	mov	ebx,[lppptr]
	mov	eax,dword ptr [R1_buffer]
	mov	fs:[ebx],eax
	mov	eax,dword ptr [R2_buffer]
	mov	fs:[ebx+4],eax
	mov	eax,[basefileptr]
	or	eax,COMBIMASK		;direct pointer to BASEFILE
	mov	fs:[ebx+8],eax
	xor	eax,COMBIMASK
	add	eax,dword ptr [baseunitbytes]
	mov	[basefileptr],eax
	add	ebx,RAMUNITBYTES
	mov	[lppptr],ebx

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

getEAX:				;get DWORD to EAX from DS:SI
	xor	eax,eax
	lodsw
	and	ax,LENMASK
	mov	cx,ax
	lodsd
	cmp	cx,1
	jne	short getEAX10
	and	eax,0000ffffh
getEAX10:
	ret

;
; lpp initialize
; command#=1

lppINIT:
	mov	ax,cs
	mov	ds,ax
	mov	[spmem],sp

	mov	eax,pmodeworkadr
	mov	[lpptop],eax

	mov	ebx,[lpptop]
	xor	eax,eax
	mov	fs:[ebx],eax		;dummy data
	mov	fs:[ebx+4],eax		;
	mov	fs:[ebx+8],eax		;
	add	ebx,RAMUNITBYTES		
	mov	[lppptr],ebx

	mov	dx,offset BASEFILENAME	;create base file
	mov	ah,3ch			;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[basehandle],ax

	mov	bx,ax			;write dummy
	mov	dx,offset R1_buffer
	mov	cx,[baseunitbytes]
	mov	ah,40h			;write handle
	int	21h
	jc	diskerror

	mov	eax,dword ptr [baseunitbytes]
	mov	[basefileptr],eax

	mov	ax,ss
	mov	ds,ax
	retf

diskerror:
	mov	word ptr ds:[AR0],8001h
	mov	sp,cs:[spmem]
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


BASEFILENAME	db	'PPMPBASE.PPM',0
COMBIFILENAME	db	'PPMPCOMB.PPM',0

;
; initialize work area
;COMMAND#=0

;	V1=R1
;	V2=R2
;	V3=X
;	V4=F
;	V5=MP2%(0)

ALLINIT:
	mov	cs:[spmem],sp

	MOV	BX,V1		;set R1 ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[R1_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[R1_SEG],AX

	MOV	BX,V2		;set R2 ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[R2_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[R2_SEG],AX

	MOV	BX,V3		;set X ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[X_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[X_SEG],AX

	mov_ax	AR1
	mov	cs:[baseXbytes],ax
	add	ax,R1_BYTES+R2_BYTES
	mov	cs:[baseunitbytes],ax

	mov	ax,_memoseg
	mov	es,ax

	xor	di,di
	mov	cx,80h
	xor	ax,ax
	rep	stosw

init100:
	mov	ax,cs
	mov	ds,ax

descriptor	struc
	limit_0_15	dw	0
	base_0_15	dw	0
	base_16_23	db	0
	access		db	0
	gran		db	0
	base_24_31	db	0
descriptor	ends

data_seg_access	equ	092h

	; set GDT address at GDTR

	mov	ax,cs
	mov	ds,ax
	mov	si,offset gdttbl
	mov	ax,_gdttblseg
	mov	es,ax
	mov	di,0
	mov	cx,10h
	rep	movsw
	
	mov	eax,_gdttbladr
	mov	dword ptr gdtaddr+2,eax
	lgdt	gdtaddr

	cli

	mov	eax,cr0		;set PE bit
	or	eax,1		;
	mov	cr0,eax		;
	jmp	$+2		;flush buffer

	mov	ax,8		;now in protect mode
	mov	fs,ax
	mov	gs,ax

	mov	eax,cr0		;reset PE bit
	and	eax,07ffffffeh	;
	mov	cr0,eax		;
	jmp	$+2		;flush buffer

	sti
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


gdtaddr	label	qword
	dw	2*8		;size of GDT table
	dd	?
	dw	0

	; GDT table

gdttbl	label	dword
gdt_null	descriptor <,,,,,>
gdt_data32	descriptor <0ffffh,0,0,data_seg_access,0cfh,0>


code	ends
end	start
