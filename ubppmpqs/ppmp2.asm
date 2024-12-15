;PPMP2.ASM 2.0
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

	org	104h
sieveout:
	mov	eax,cr0		;reset PE bit
	and	eax,07ffffffeh
	mov	cr0,eax

	db	0eah		;jmp far ptr flushrl
	dw	offset flushrl
csegmem	dw	?

	;now in real mode
flushrl:
	sti
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


	align	4
primeadrnow	dd	?
lastprime	dd	?

lpvindexadr	dd	?
lpvdataadr	dd	?

pmodecsseg	dw	?

primes		dw	?

P_OFF	DW	?
P_SEG	DW	?
Y_OFF	DW	?
Y_SEG	DW	?

L_OFF	DW	?
L_SEG	DW	?

X_OFF	DW	?
X_SEG	DW	?

D_OFF	DW	?
D_SEG	DW	?
B_OFF	DW	?
B_SEG	DW	?
M_OFF	DW	?
M_SEG	DW	?
W_OFF	DW	?
W_SEG	DW	?

Ew_OFF	DW	?
Ew_SEG	DW	?

X1_OFF	DW	?
X1_SEG	DW	?

IniLog	DW	?
CutLog	DW	?

lpvnow		dw	0
lpvcount	dw	0


; ** command branch


START0:
	MOV_AX	AR0		;ARRAY[0] is the command
	MOV	BX,offset CMD_TBL
	SHL	AX,1
	ADD	BX,AX
	JMP	CS:[BX]

CMD_TBL:
	DW	INITMPQS,SETR1R2,SIEVE,SIEVE_ANS,SETDATA
	DW	DECOMPOSE,INITMPQS2

;
; * Initialize again
; command#=6

initmpqs2:
	mov	eax,fs:[_primes3A]
	or	eax,eax
	jnz	short initmpqs2B
	mov	ebx,fs:[_primes3B]
	mov	fs:[_primes3A],ebx
	mov	fs:[_primes3B],eax

initmpqs2B:
comment %
	mov	ax,fs:[_primes1]
	mov	bx,fs:[_primes2]
	mov	cx,fs:[_primes3A]
	mov	dx,fs:[_primes3B]
	mov	si,fs:[_primes4]
	mov	di,fs:[_primes]
int 3
%
	mov	ecx,fs:[_sieveBsize]
	xor	edx,edx
	mov	eax,fs:[_sievewidth]
	shl	eax,1
	div	ecx
	mov	bx,AR0
	mov	cs:[bx],ax		;loop time
	mul	ecx
	shr	eax,1
	mov	fs:[_sievewidth],eax
	retf


;
;* test the divisivility of W by PR()
;
;COMMAND#=5

decompose:
	mov	ax,_memoseg
	mov	es,ax

	mov	ax,cs:[w_off]
	mov	es:[_absQR],ax
	mov	ax,cs:[w_seg]
	mov	es:[_absQR+2],ax

	cli

	mov	eax,cr0		;set PE bit
	or	eax,1
	mov	cr0,eax

	db	0eah		;goto protect mode
	dw	700h
	dw	offset gdt_code32-gdttbl	;selector


;
; initialize work area
;COMMAND#=0

;	V1=work(0,0)
;	V2=P
;	V3=Y
;	V4=L
;	V5=X
;	V6=D
;	V7=B
;	V8=OF
;	V9=W#
;	V10=MP2%(0)

;	AR[1]	NUMBER OF PRIMES \ 2
;	AR[2]	INITIAL LOG
;	AR[3]	LIMIT LOG
;
;	set AR[1]=0 if memory lacks

MEMORY_LACK:
	MOV	BX,AR1
	MOV	WORD PTR CS:[BX],0
	retf


INITMPQS:
	MOV	BX,V1		;check work(0,0) segment
	MOV	AX,CS:[BX]
	shr	ax,4
	add	ax,CS:[BX+2]
	cmp	ax,_memoseg
	ja	short memory_lack

	MOV	BX,V2		;set P ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[P_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[P_SEG],AX

	MOV	BX,V3		;set Y ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[Y_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[Y_SEG],AX

	MOV	BX,V4		;set L ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[L_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[L_SEG],AX

	MOV	BX,V5		;set X ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[X_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[X_SEG],AX

	MOV	BX,V6		;set D ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[D_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[D_SEG],AX

	MOV	BX,V7		;set B ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[B_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[B_SEG],AX

	MOV	BX,V8		;set Off ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[M_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[M_SEG],AX

	MOV	BX,V9		;set W# ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[W_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[W_SEG],AX

	MOV	BX,V10		;set PMODE segment
	MOV	AX,CS:[BX]
	shr	ax,4
	add	ax,CS:[BX+2]
	sub	ax,ARRAYHEADSEG
	mov	cs:[pmodecsseg],ax

	MOV_AX	AR1
	shl	ax,1
	mov	cs:[primes],ax

	MOV_AX	AR2
	MOV	CS:[IniLog],AX

	MOV_AX	AR3
	MOV	CS:[CutLog],AX

init100:
	mov	ax,cs
	mov	ds,ax
	mov	[csegmem],ax

	mov	eax,primeadr2+primeunitbytes
	mov	[primeadrnow],eax
	mov	[lastprime],0

descriptor	struc
	limit_0_15	dw	0
	base_0_15	dw	0
	base_16_23	db	0
	access		db	0
	gran		db	0
	base_24_31	db	0
descriptor	ends

code_seg_access	equ	09ah
data_seg_access	equ	092h

	; make GDT
	; 32bit code segment in protect mode

	mov	ax,[pmodecsseg]
	and	eax,0ffffh
	shl	eax,4
	mov	ebx,eax
	shr	eax,16
	mov	gdt_code32.base_0_15,bx
	mov	gdt_code32.base_16_23,al

	; 16 bit code segment in real mode for return

	xor	eax,eax
	mov	ax,cs
	shl	eax,4
	mov	ebx,eax
	shr	eax,16
	mov	gdt_code16.base_0_15,bx
	mov	gdt_code16.base_16_23,al

	; 32 bit data segment : start from 00000

	; store "number of primes" and "inilog" and "cutlog" and "M"

	mov	ax,_memoseg
	mov	es,ax

	xor	di,di
	mov	cx,80h
	xor	ax,ax
	rep	stosw

	movzx	eax,word ptr cs:[primes]
	mov	es:[_primesR],eax
	mov	ecx,primeunitbytes
	mul	ecx
	sub	eax,optionRAM		;//
	jnb	short init140
	xor	eax,eax
init140:
	add	eax,primeadr
	mov	es:[_sievetopR],eax

	mov	ax,cs:[inilog]
	mov	ah,al
	mov	es:[_inilogR],ax
	mov	es:[_inilogR+2],ax

	mov	ax,cs:[cutlog]
	mov	es:[_cutlogR],ax

	lds	si,dword ptr cs:[M_off]
	mov	di,_sievewidthR
	lodsw
	and	ax,7fffh	;make positive
	movsw
	cmp	ax,1
	je	short init150
	movsw
init150:
	mov	ebx,200000h	;memory end
	sub	ebx,es:[_sievetopR]
	mov	eax,es:[_sievewidthR]
	shl	eax,1
	cmp	eax,ebx
	jbe	short initmpqs220

	push	eax		;/*
	xor	edx,edx
	div	ebx
	or	eax,eax
	jz	short initmpqs210
	inc	eax
initmpqs210:
	mov	ebx,eax
	xor	edx,edx
	pop	eax		;*/
	div	ebx
initmpqs220:
	and	al,0f8h		;let be multiple of 8
	mov	es:[_sieveBsizeR],eax
	add	eax,es:[_sievetopR]
	mov	es:[_sieveoverR],eax

	mov	eax,es:[_sieveBsizeR]
	sub	eax,es:[_sievetopR]
	mov	es:[_sieveConstR],eax

	mov	eax,es:[_sieveBsizeR]
	shr	eax,1
	mov	es:[_sieveWsizeR],eax
	shr	eax,1
	mov	es:[_sieveDsizeR],eax
	shr	eax,sieveRepLog-2
	mov	es:[_sieveXsizeR],eax

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

	mov	ax,10h		;now in protect mode
	mov	fs,ax
	mov	gs,ax

	mov	eax,cr0		;reset PE bit
	and	eax,07ffffffeh	;
	mov	cr0,eax		;
	jmp	$+2		;flush buffer

	sti

	mov	ax,cs
	mov	ds,ax
	mov	ax,_memoseg
	shr	ax,1
	mov	bx,AR1
	mov	[bx],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax

	retf

;
; set data for each prime
;command#=4
setdata:
	mov	edi,cs:[primeadrnow]

	lds	si,dword ptr cs:[p_off]
	lodsw
	cmp	ax,1
	ja	short setdata20
	xor	eax,eax
	lodsw
	jmp	short setdata30
setdata20:	
	lodsd

setdata30:
	cmp	eax,fs:[_sieveBsize]
	jb	short setdata36
	cmp	edi,primeadr
	jae	short setdata35
	mov	edi,primeadr
setdata35:
	inc	word ptr fs:[_primes4]
	jmp	short setdata39
setdata36:
	cmp	eax,fs:[_sieveWsize]
	jb	short setdata37
	cmp	edi,mainRAMover
	jne	short setdata365
	mov	edi,primeadr
	mov	dx,fs:[_primes3B]
	mov	fs:[_primes3A],dx
	mov	word ptr fs:[_primes3B],0
setdata365:
	inc	word ptr fs:[_primes3B]
	jmp	short setdata39
setdata37:
	cmp	eax,fs:[_sieveDsize]
	jb	short setdata38
	inc	word ptr fs:[_primes2]
	jmp	short setdata39
setdata38:
	cmp	eax,fs:[_sieveXsize]
	jb	short setdata39
	inc	word ptr fs:[_primes1]
setdata39:
	mov	edx,eax
	sub	eax,cs:[lastprime]
	mov	fs:[edi],ax
	mov	cs:[lastprime],edx

	lds	si,dword ptr cs:[y_off]
	xor	eax,eax
	lodsw
	cmp	ax,1
	ja	short setdata40
	jb	short setdata50
setdata45:
	lodsw
	jmp	short setdata50
setdata40:	
	lodsd
setdata50:
	mov	fs:[edi+4],eax

	lds	si,dword ptr cs:[l_off]
	mov	ax,[si+2]
	mov	fs:[edi+2],ax

	add	edi,primeunitbytes
setdata60:
	mov	cs:[primeadrnow],edi

	mov	ax,ss
	mov	ds,ax
	retf

;
; * sieving main
;   386 protect mode
; command#=2

SIEVE:
	cli
	mov	eax,fs:[_primes1]
	mov	eax,cr0		;set PE bit
	or	eax,1
	mov	cr0,eax

	db	0eah		;goto protect mode
	dw	280h
	dw	offset gdt_code32-gdttbl	;selector

;
; get a sieved result
;
; command#=3
;                    !! not used now !!
SIEVE_ANS:
	retf

;
; set sieve start adr R1,R2 for each prime
;
; command#=1

SETR1R2:
	;set parameter

	mov	ax,_memoseg
	mov	es,ax

	;store D by 32bit form

	lds	si,dword ptr cs:[D_OFF]
	mov	di,_DR
	lodsw
	and	ax,7fffh
	mov	cx,ax
	mov	eax,3		;3*32 bits
	stosd
	rep	movsw
	xor	eax,eax
	stosd
	stosd
	stosd

	;store B by 32bit form

	lds	si,dword ptr cs:[B_OFF]
	mov	di,_BR
	lodsw
	and	ax,7fffh
	mov	cx,ax
	mov	eax,5		;6*32 bits
	stosd
	rep	movsw
	xor	eax,eax
	rept	5
	  stosd
	endm

	cli

	mov	eax,cr0		;set PE bit
	or	eax,1
	mov	cr0,eax

	db	0eah		;goto protect mode
	dw	100h
	dw	offset gdt_code32-gdttbl


gdtaddr	label	qword
	dw	4*8		;size of GDT table
	dd	?
	dw	0


	; GDT table

gdttbl	label	dword
gdt_null	descriptor <,,,,,>
gdt_code32	descriptor <0ffffh,,,code_seg_access,0cfh,0>
gdt_data32	descriptor <0ffffh,0,0,data_seg_access,0cfh,0>
gdt_code16	descriptor <0ffffh,,,code_seg_access,80h,0>


code	ends
end	start
