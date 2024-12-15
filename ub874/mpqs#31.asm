;MPQS#31.ASM
;	MACHINE LANGUAGE SUBROUTINE
;	FOR MULTIPLE POLYNOMIAL QUADRATIC SIEVE
;
;	Original Version           1987 by YUJI KIDA
;	Hard Disk Version          1992 by Yuji KIDA

	INCLUDE	UB.MAC

primeunitbytes	equ	10
maxFBsize	equ	3072
maxbufferwords	equ	maxFBsize/8	;*2/16

	JMP	START0

	EVEN
N_OFF	DW	?
N_SEG	DW	?
X_OFF	DW	?
X_SEG	DW	?
Y_OFF	DW	?
Y_SEG	DW	?
W_OFF	DW	?
W_SEG	DW	?
R_OFF	DW	?
R_SEG	DW	?

D_OFF	DW	?
D_SEG	DW	?
B_OFF	DW	?
B_SEG	DW	?
Dmemadr	dw	?
Bmemadr	dw	?

ANS_OFF	dw	?
ANS_SEG	dw	?

inilog	dw	?
cutlog	dw	?

FBsize		dw	?
primes		dw	?

primeadrnow	dw	?
primeseg	dw	?

lppadrnow	equ	this word
sieveansnow	dw	?
gaussseg	equ	this word
lppseg		equ	this word
sieveseg	dw	?

lppindexnow	dw	?
lppindexseg	dw	?
lppall		dw	?

sievewidth	dw	?,?
MIA2P		dw	?

rownumbernow	dw	?
ansnow		dw	?
masknow		dw	?
offnow		dw	?
rightmattop	dw	?

spmem		dw	?
startrow	dw	?
checkingrow	dw	?
localrank	dw	?
FBbytes		dw	?
FBbytesW	dw	?
FBbytesW4	dw	?
FBwords		dw	?
FBwordsW	dw	?
loadsavebytes	dw	?
unitrows	dw	?
GaussLongNow	dw	?
RWrow		dw	?
mathandle	dw	?

ANShandle	dw	?
answers		dw	?

makexorwordsw	dw	?
makexorskip	dw	?

sieveleftsw	dw	?


;** COMMAND BRANCH


START0:
	MOV_AX	AR0		;get COMMAND frm ARRAY[0]
	MOV	BX,OFFSET CMD_TBL
	SHL	AX,1
	ADD	BX,AX
	JMP	CS:[BX]

CMD_TBL:
	dw	init,setprime,setr1r2,sieve,sieve_ans
	dw	test_div,lppinit,setlppdata,sortlppdata,getlppresult
	dw	setrowinit,setrow,gaussinit,gauss,setanswer
	dw	countpower,closeansfile

;
; * close answer file
;
;command#=16

closeansfile:
	mov	ah,3eh
	mov	bx,cs:[ANShandle]
	or	bx,bx
	jz	close10
	int	21h
close10:
	retf


;
; * count power for each prime
;
;command#=15

countpower:
	PUSH	BP

	LDS	SI,DWORD PTR CS:[w_OFF]
	MOV	BX,SI
	MOV	ES,CS:[primeseg]
	MOV	DI,primeunitbytes	;from 2

	STD
	MOV	CX,CS:[primes]
ctpowLP:
	PUSH	CX
	MOV	BP,ES:[DI]	;numerator
ctpow10:
	MOV	SI,BX
	MOV	AX,[SI]
	MOV	CX,AX		;WORDS
	SHL	AX,1
	ADD	SI,AX		;highest word adr
	XOR	DX,DX	
ctpow20:
	LODSW
	DIV	BP
	LOOP	ctpow20
	OR	DX,DX
	JZ	ctpow100	;divide if possible

	ADD	DI,primeunitbytes
	POP	CX
	LOOP	ctpowLP

	mov	ax,ss
	mov	ds,ax
	mov	es,ax

	POP	BP
	CLD
	RETF

ctpow100:
	inc	word ptr es:[di+6]	;count power
	MOV	SI,BX
	MOV	AX,[SI]
	MOV	CX,AX		;WORDS
	SHL	AX,1
	ADD	SI,AX		;highest word adr
	XOR	DX,DX	
	LODSW
	DIV	BP
	PUSH	AX		;value of highest word
	JMP	SHORT ctpow120
ctpow110:
	LODSW
	DIV	BP
ctpow120:
	MOV	[SI+2],AX
	LOOP	ctpow110
	POP	AX		;value of highest word
	OR	AX,AX
	JNZ	ctpow10
	DEC	WORD PTR [SI]	;dec len if highest word=0
	JMP	ctpow10

;
; * set answer
;
;command#=14

setanswer:	
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	ah,3fh
	mov	bx,[ANShandle]
	mov	cx,[FBbytes]
	mov	dx,offset BUFFER
	int	21h
	cmp	ax,[FBbytes]
	jne	answernomore

	mov	di,[FBbytes]
	add	di,offset BUFFER
	sub	di,2			;highest word adr
	xor	ax,ax
	mov	cx,[FBwords]
	std
	repe	scasw
	cld
	je	setans50
	inc	cx
setans50:
	mov	word ptr [BUFFER-2],cx
	les	di,dword ptr [ans_off]
	mov	si,offset BUFFER-2
	inc	cx
	rep	movsw

	mov	cx,[primes]
	mov	ds,[primeseg]
	mov	si,6+primeunitbytes	;offset of start1
	xor	ax,ax			;use for count power
setans60:
	mov	[si],ax
	add	si,primeunitbytes
	loop	setans60
	 
answerret:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

answernomore:
	mov	bx,AR0
	mov	word ptr [bx],8001h
	jmp	answerret


;
;** Gaussin elimination
;
;command#=13

Gauss:
	mov	ax,cs
	mov	ds,ax
	mov	[spmem],sp
	mov	es,[gaussseg]

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
	cmp	ax,[FBsize]
	jb	short gaussret

	mov	bx,[mathandle]
	mov	ah,3eh			;close
	int	21h

	mov	bx,[ANShandle]
	mov	ah,3eh			;close
	int	21h

	mov	dx,offset ANSFILENAME
	mov	ah,3dh			;open for read
	mov	al,0
	int	21h
	mov	[ANShandle],ax

	mov	bx,AR0
	mov	word ptr [bx],8001h

gaussret:
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


getpivotrows:
	mov	ax,[FBbytesW]
	mov	[loadsavebytes],ax
	xor	ax,ax			;mov	ax,[GaussLongbase]
	mov	[GaussLongNow],ax
	mov	cx,[unitrows]
	mov	ax,[checkingrow]
getpivot10:
	cmp	ax,[FBsize]
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


erasebyothers:
	xor	si,si
	cmp	si,[GaussLongNow]
	je	eraseret		;no former
erase10:
	mov	bx,es:[si]		;word offset of MSB
	mov	ax,es:[si+2]		;bit pattern of word
	mov	di,offset BUFFER
	test	ax,[bx+di]
	jz	erase20
	add	si,4

	mov	cx,[FBwordsW]		;=word full row length
erase15:
	mov	ax,es:[si]
	xor	[di],ax
	add	si,2
	add	di,2
	loop	erase15
erase18:
	jmp	short erase25
erase20:
	add	si,[FBbytesW4]
erase25:
	cmp	si,[GaussLongNow]
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
	mov	cx,[FBbytes]
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


store2extended:
	mov	si,offset MSBoffset
	mov	di,[GaussLongNow]
	movsw
	movsw
	mov	cx,[FBwordsW]
	rep	movsw
	mov	[GaussLongNow],di
	ret


erase_below:
	mov	ax,[checkingrow]
	mov	[RWrow],ax
	mov	cx,[FBsize]
	sub	cx,ax
	jz	short erasebelowret

	push	cx			;/*

	xor	bx,bx			;mov	bx,[GAUSSlongbase]
	mov	cx,[localrank]
	xor	ax,ax
eraseb10:
	cmp	ax,es:[bx]
	jae	short eraseb20
	mov	ax,es:[bx]
eraseb20:
	add	bx,[FBbytesW4]
	loop	eraseb10
	shr	ax,1
	inc	ax
	mov	[makexorwordsW],ax
	shl	ax,1
	mov	[loadsavebytes],ax
	sub	ax,[FBbytesW]
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


make_xor:
	xor	si,si			;mov	si,[GaussLongBase]
	mov	cx,[localrank]
makexor10:
	push	cx
	mov	di,es:[si]		;MSB word adr
	mov	ax,es:[si+2]		;mask
	add	di,offset BUFFER
	test	[di],ax
	jz	makexor60

	add	si,4
	mov	di,offset BUFFER
	mov	cx,[makexorwordsW]
makexor15:
	mov	ax,es:[si]
	xor	[di],ax
	add	si,2
	add	di,2
	loop	makexor15
makexor18:
	add	si,[makexorskip]
	jmp	short makexor70
makexor60:
	add	si,[FBbytesW4]
makexor70:
	pop	cx
	loop	makexor10
	ret


load_row:
	mov	ax,[RWrow]
	mov	dx,[FBbytesW]
	mul	dx
	mov	cx,dx
	mov	dx,ax			;cx:dx = file ptr

	mov	ah,42h			;move file ptr
	mov	al,0			;absolute
	mov	bx,[mathandle]
	int	21h
load_row20:
	mov	ah,3fh			;read
	mov	bx,[mathandle]
	mov	cx,[loadsavebytes]
	mov	dx,offset BUFFER
	int	21h
	ret


save_row:
	mov	ax,[RWrow]
	mov	dx,[FBbytesW]
	mul	dx
	mov	cx,dx
	mov	dx,ax			;cx:dx = file ptr

	mov	ah,42h			;move file ptr
	mov	al,0			;absolute
	mov	bx,[mathandle]
	int	21h
save_row20:
	mov	ah,40h			;write
	mov	bx,[mathandle]
	mov	cx,[loadsavebytes]
	mov	dx,offset BUFFER
	int	21h
	ret


calcMSB:
	;in: di = base adr
	;out: ax = mask of MSB, cx = offset adr of MSB
	;destroy: nothing

	push	di
	add	di,[FBbytesW]
	xor	ax,ax
	mov	cx,[FBwords]
calcMSB10:
	sub	di,2
	cmp	ax,[di]
	loope	calcMSB10
	je	short calcMSBzero	;ax=cx=0
	mov	ax,[di]
	or	ah,ah
	jnz	short calcMSB20		;if on higher
	mov	ah,al
	mov	cx,7
calcMSB12:
	shl	ax,1
	jc	calcMSB14
	loop	calcMSB12
calcMSB14:
	mov	ax,1
calcMSBret:
	shl	ax,cl
	mov	cx,di
	pop	di
	sub	cx,di
	ret

calcMSB20:
	mov	cx,7
calcMSB22:
	shl	ax,1
	jc	calcMSB24
	loop	calcMSB22
calcMSB24:
	mov	ax,0100h
	jmp	calcMSBret

calcMSBzero:				;if 0 then return with ax=0
	pop	di			;also cx=0 but has no meaning
	ret

;
; * gaussian elimination initialize
;
;command#=12

gaussinit:
	mov	ax,cs
	mov	ds,ax

	mov	ax,[FBsize]
	shr	ax,1
	shr	ax,1
	shr	ax,1
	mov	[FBbytes],ax
	mov	[FBwordsW],ax
	shr	ax,1
	mov	[FBwords],ax
	shl	ax,1
	shl	ax,1
	mov	[FBbytesW],ax
	add	ax,4
	mov	[FBbytesW4],ax

	mov	bx,ax
	mov	dx,1			;DX:AX = 64kbytes
	xor	ax,ax			;
	div	bx
	mov	[unitrows],ax
	mov	bx,AR1
	mov	[bx],ax

	mov	[startrow],0
	mov	[answers],0

	mov	dx,offset ANSFILENAME	;create ANS file
	mov	ah,3ch			;create handle
	mov	cx,0
	int	21h
	jc	gaussinitdiskerror
	mov	[ANShandle],ax

gaussinitret:
	mov	ax,ss
	mov	ds,ax
	retf

gaussinitdiskerror:
	mov	bx,AR0
	mov	word ptr [bx],8001h
	jmp	gaussinitret

ANSFILENAME	db	'MPQSANS.MPQ',0

;
; ** set a row
;    make row data from W satisfying W=X^2

;COMMAND#=11

SETROW:
	PUSH	BP

	mov	ax,cs
	mov	es,ax
	mov	di,offset BUFFER
	mov	cx,cs:[FBsize]
	shr	cx,1
	shr	cx,1
	shr	cx,1
	xor	ax,ax
	rep	stosw

	mov	cx,cs:[rownumbernow]
	mov	bx,cx
	shr	bx,1
	shr	bx,1
	shr	bx,1
	add	bx,offset BUFFER
	and	cl,7
	mov	al,1
	shl	al,cl
	mov	cs:[bx],al

	lds	si,dword ptr cs:[w_off]
	mov	ax,[si]
	mov	cx,ax
	and	ax,8000h
	rol	ax,1

	and	cx,lenmask
	mov	[si],cx
	mov	bx,si			;memo top of W

	mov	es,cs:[primeseg]
	mov	di,primeunitbytes

	MOV	CS:[ANSNOW],AX		;16 bit result
	MOV	CS:[MASKNOW],2
	mov	ax,cs:[rightmattop]
	MOV	CS:[OFFNOW],ax

	STD

	MOV	CX,CS:[primes]
DIVLP:
	MOV	BP,ES:[DI]	;divisor
	ADD	DI,primeunitbytes
	PUSH	CX
DIV10:
	MOV	SI,BX
	MOV	AX,[SI]
	MOV	CX,AX		;number of words
	SHL	AX,1
	ADD	SI,AX		;highest word adr
	XOR	DX,DX	
DIV20:
	LODSW
	DIV	BP
	LOOP	DIV20
	OR	DX,DX
	JZ	DIV100		;divide if possible

	ROL	CS:[MASKNOW],1
	JNC	DIV30

	PUSH	BX
	MOV	BX,CS:[OFFNOW]
	MOV	AX,CS:[ANSNOW]
	MOV	cs:[BX],AX		;set result
	MOV	CS:[ANSNOW],0
	ADD	BX,2
	MOV	CS:[OFFNOW],BX
	POP	BX
DIV30:	
	POP	CX
	LOOP	DIVLP
	CLD

	; write to file

	mov	ax,cs
	mov	ds,ax

	mov	dx,offset BUFFER
	mov	cx,[FBsize]
	shr	cx,1
	shr	cx,1
	mov	bx,[mathandle]
	mov	ah,40h			;write data
	int	21h
	jc	diskfull

	inc	[rownumbernow]
divret:
	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	POP	BP
	RETF			;EXIT POINT

diskfull:
	mov	bx,[mathandle]
	mov	ah,3eh			;close file
	int	21h
	mov	bx,AR0
	mov	word ptr [bx],8001h
	jmp	divret


DIV100:
	MOV	AX,CS:[MASKNOW]
	XOR	CS:[ANSNOW],AX

	MOV	SI,BX
	MOV	AX,[SI]
	MOV	CX,AX		;WORDS
	SHL	AX,1
	ADD	SI,AX		;highest word adr
	XOR	DX,DX	
	LODSW
	DIV	BP
	PUSH	AX		;value of highest word
	JMP	SHORT DIV120
DIV110:
	LODSW
	DIV	BP
DIV120:
	MOV	[SI+2],AX
	LOOP	DIV110
	POP	AX		;value of highest word
	OR	AX,AX
	JNZ	DIV130
	DEC	WORD PTR [SI]	;dec len if highest word = 0
DIV130:
	JMP	DIV10

;
; * set row init
;
;command#=10

setrowinit:
	mov	ax,cs
	mov	ds,ax

	mov	ax,[FBsize]
	shr	ax,1
	shr	ax,1
	shr	ax,1
	add	ax,offset BUFFER
	mov	[rightmattop],ax
	mov	[rownumbernow],0

	mov	dx,offset mpqsmatname
	mov	ah,3ch		;create handle
	mov	cx,0
	int	21h
	jc	diskerror
	mov	[mathandle],ax
setrowinitret:
	mov	ax,ss
	mov	ds,ax
	retf

diskerror:
	mov	bx,AR0
	mov	word ptr [bx],8001h
	jmp	setrowinitret

mpqsmatname	db	'mpqsmat.mpq',0

;
; * get lpp result
;
;command#=9

getlppresult:
	lds	si,dword ptr cs:[lppadrnow]
getres10:
	add	si,4
	mov	dx,[si]
	cmp	dx,0ffffh
	je	getres50		;end mark
	cmp	dx,[si+4]
	jne	getres10
	mov	ax,[si+2]
	cmp	ax,[si+6]
	jne	getres10
	mov	cs:[lppadrnow],si

	lds	bx,dword ptr cs:[R_OFF]
	mov	[bx+2],dx
	mov	[bx+4],ax
	mov	cx,2
	or	ax,ax
	jnz	short getres20
	dec	cx
	or	dx,dx
	jnz	short getres20
	dec	cx
getres20:
	mov	[bx],cx
	
	mov	ds,cs:[lppindexseg]
	mov	si,cs:[lppadrnow]
	shr	si,1
	mov	ax,[si]
	mov	bx,AR2
	mov	cs:[bx],ax
	mov	ax,[si+2]
getresret:
	mov	bx,AR1
	mov	cs:[bx],ax

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

getres50:
	mov	ax,8001h
	jmp	getresret


;
; * sort lpp data
;
;command#=8

sortlppdata:
	les	di,dword ptr cs:[lppadrnow]
	mov	ax,0ffffh
	stosw
	stosw				;endmark

	call	qsort
	call	isort

	mov	cs:[lppadrnow],0
	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

; insertion sort

_ii	equ	0
_jj	equ	2
_ww	equ	4
_wwhigh	equ	6

isort:
	push	bp
	sub	sp,8
	mov	bp,sp

	mov	cx,cs:[lppall]
	dec	cx
	mov	ax,2
	mov	[bp+_ii],ax
isort10:
	push	cx
	mov	ax,[bp+_ii]
	call	getlpp
	mov	[bp+_wwhigh],dx
	mov	[bp+_ww],cx
	xor	ax,ax
	call	putlpp

	mov	ax,[bp+_ii]
	dec	ax
	mov	[bp+_jj],ax

  ;while
isort40:
	mov	ax,[bp+_jj]
	call	getlpp
	cmp	dx,[bp+_wwhigh]
	ja	isort45
	jb	isort50
	cmp	cx,[bp+_ww]
	jbe	isort50
isort45:
	mov	ax,[bp+_jj]
	inc	ax
	call	putlpp
	dec	word ptr [bp+_jj]
	jmp	isort40

isort50:
	xor	ax,ax
	call	getlpp
	mov	ax,[bp+_jj]
	inc	ax
	call	putlpp

	pop	cx
	inc	word ptr [bp+_ii]
	loop	isort10

	add	sp,8
	pop	bp
	ret


getlpp:
	; in : ax offset
	; out: dx:cx data, bx = datanumber

	mov	bx,ax
	shl	bx,1
	mov	ds,cs:[lppindexseg]
	mov	ax,[bx]
	shl	bx,1
	mov	ds,cs:[lppseg]
	mov	cx,[bx]
	mov	dx,[bx+2]
	mov	bx,ax
	ret

putlpp:
	;in: ax offset
	;    dx:cx data, bx data number

	shl	ax,1
	xchg	bx,ax
	mov	ds,cs:[lppindexseg]
	mov	[bx],ax
	mov	ds,cs:[lppseg]
	shl	bx,1
	mov	[bx],cx
	mov	[bx+2],dx
	ret


; quick sort

_s	equ	0
_e	equ	2
_i	equ	4
_j	equ	6

_pivot		equ	8
_pivothigh	equ	10

qsort:
	mov	bx,1
	mov	cx,cs:[lppall]
	call	qsortmain
	ret

qsortmain:				;bx=S,cx=E
	push	bp
	sub	sp,12

	mov	bp,sp
	mov	[bp+_s],bx
	mov	[bp+_i],bx
	mov	[bp+_e],cx
	mov	[bp+_j],cx
	mov	ax,bx
	add	ax,5
	cmp	ax,cx
	jae	qsortret
	sub	ax,5
	add	ax,cx
	shr	ax,1		;ax=(E+S)/2
	call	getlpp
	mov	[bp+_pivot],cx
	mov	[bp+_pivothigh],dx
qsort10:
	mov	ax,[bp+_i]
	call	getlpp
	cmp	dx,[bp+_pivothigh]
	jb	qsort15
	ja	qsort20
	cmp	cx,[bp+_pivot]
	jae	qsort20
qsort15:
	inc	word ptr [bp+_i]
	jmp	qsort10
qsort20:			
	mov	ax,[bp+_j]
	call	getlpp
	cmp	dx,[bp+_pivothigh]
	jb	qsort30
	cmp	cx,[bp+_pivot]
	jbe	qsort30
	dec	word ptr [bp+_j]
	jmp	qsort20
qsort30:
	mov	si,[bp+_i]
	mov	di,[bp+_j]
	cmp	si,di
	ja	short qsort50

	call	swap_sidi
	inc	word ptr [bp+_i]
	dec	word ptr [bp+_j]

qsort50:
	mov	ax,[bp+_i]
	cmp	ax,[bp+_j]
	jbe	qsort10
	mov	bx,[bp+_s]
	mov	cx,[bp+_j]
	call	qsortmain
	mov	bx,[bp+_i]
	mov	cx,[bp+_e]
	call	qsortmain
qsortret:
	add	sp,12
	pop	bp
	ret

swap_sidi:
	shl	si,1			;swap index
	shl	di,1
	mov	ds,cs:[lppindexseg]
	mov	ax,[si]
	xchg	ax,[di]
	mov	[si],ax

	shl	si,1			;swap data
	shl	di,1
	mov	ds,cs:[lppseg]
	mov	ax,[si]
	xchg	ax,[di]
	mov	[si],ax
	mov	ax,[si+2]
	xchg	ax,[di+2]
	mov	[si+2],ax
	ret

;
; * set lpp data
;
;command#=7

setlppdata:
	lds	si,dword ptr cs:[R_OFF]
	les	di,dword ptr cs:[lppadrnow]

	xor	ax,ax
	mov	es:[di],ax
	mov	es:[di+2],ax
	lodsw
	and	ax,lenmask
	jz	short setlpp10
	movsw
	dec	ax
	jz	short setlpp10
	movsw
setlpp10:
	add	cs:[lppadrnow],4

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

;
; * lpp initialize
;
;command#=6

lppinit:
	mov_ax	AR1
	mov	cs:[lppall],ax
	add	ax,15
	shr	ax,1			;*4/16
	shr	ax,1
	inc	ax			;one more
	add	ax,cs:[lppseg]
	mov	cs:[lppindexseg],ax
	mov	es,ax
	xor	ax,ax
	xor	di,di
	mov	cx,cs:[lppall]
	inc	cx			;0-th data is a dummy
lppinitlp:
	stosw
	inc	ax
	loop	lppinitlp

	mov	cs:[lppadrnow],4

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

;
;** check whether w is divisible by PR%()
;
;command#=5

TEST_DIV:
	PUSH	BP

	LDS	SI,DWORD PTR CS:[w_OFF]
	MOV	BX,SI
	MOV	ES,CS:[primeseg]
	MOV	DI,primeunitbytes	;from 2

	STD
	MOV	CX,CS:[primes]
TDIVLP:
	PUSH	CX
	MOV	BP,ES:[DI]	;numerator
	ADD	DI,primeunitbytes
TDIV10:
	MOV	SI,BX
	MOV	AX,[SI]
	MOV	CX,AX		;WORDS
	SHL	AX,1
	ADD	SI,AX		;highest word adr
	XOR	DX,DX	
TDIV20:
	LODSW
	DIV	BP
	LOOP	TDIV20
	OR	DX,DX
	JZ	TDIV100		;do divide if divisible

	POP	CX
	LOOP	TDIVLP

	mov	ax,ss
	mov	ds,ax
	mov	es,ax

	POP	BP
	CLD
	RETF

TDIV100:
	MOV	SI,BX
	MOV	AX,[SI]
	MOV	CX,AX		;WORDS
	SHL	AX,1
	ADD	SI,AX		;highest word adr
	XOR	DX,DX	
	LODSW
	DIV	BP
	PUSH	AX		;value of highest word
	JMP	SHORT TDIV120
TDIV110:
	LODSW
	DIV	BP
TDIV120:
	MOV	[SI+2],AX
	LOOP	TDIV110
	POP	AX		;value of highest word
	OR	AX,AX
	JNZ	TDIV10
	DEC	WORD PTR [SI]	;dec len if highest word=0
	JMP	TDIV10

;
;** return a value got by SIEVE
;
;command#=4
;OUTPUT	X=OFFSET 
;	 =-1 NO MORE DATA

SIEVE_ANS:
	cmp	cs:[sieveleftsw],0
	je	sieveansnomore
	les	di,dword ptr cs:[sieveansnow]
	MOV	AX,CS:[cutlog]
	MOV	CX,DI
	NEG	CX
even
GETDLP:
	SCASB
	JAE	GETIT
	LOOP	GETDLP

sieveansnomore:
	mov	cs:[sieveleftsw],1
	mov	cs:[sieveansnow],0
	les	di,dword ptr cs:[x_off]
	mov	word ptr es:[di],8001h
	mov	word ptr es:[di+2],1
	MOV	AX,SS
	MOV	ES,AX
	RETF

GETIT:
	mov	cs:[sieveleftsw],di	;0 iff no more
	mov	cs:[sieveansnow],di

	dec	di
	mov	ax,di
	les	di,dword ptr cs:[x_off]
	mov	word ptr es:[di],1
	mov	es:[di+2],ax
	or	ax,ax
	jnz	getit10
	stosw
getit10:
	MOV	AX,SS
	MOV	ES,AX
	RETF


;
;** SIEVING main
;
;command#=3

SIEVE:
	;initialize sieve

	MOV	AX,CS:[sieveseg]
	MOV	DS,AX
	MOV	ES,AX

	mov	cs:[sieveansnow],0
	mov	cs:[sieveleftsw],1
	XOR	DI,DI
	MOV	CX,8000H
	MOV	AX,CS:[inilog]
	REP	STOSW

	MOV	AX,CS:[primeseg]
	MOV	ES,AX	
	MOV	BX,primeunitbytes*2	;skip -1,2

	;SIEVE MAIN

	MOV	CX,CS:[primes]
	DEC	CX		;skip 2

SIEVE_MAINLP:
	MOV	DI,ES:[BX]	;STEP
	MOV	AX,ES:[BX+4]	;LOG

	MOV	SI,ES:[BX+6]	;START
even
SIEVE20:
	SUB	[SI],AL
	ADD	SI,DI
	jnc	SIEVE20
	MOV	ES:[BX+6],SI	;R1_OFF(for next entry)

	MOV	SI,ES:[BX+8]	;R2_OFF START
even
SIEVE40:
	SUB	[SI],AL
	ADD	SI,DI
	jnc	SIEVE40
	MOV	ES:[BX+8],SI	;R2_OFF(for next entry)

	ADD	BX,primeunitbytes
	LOOP	SIEVE_MAINLP

	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	RETF
	
;
; set sieve start adr R1,R2 for each prime
;
; command#=2

SETR1R2:
	;set parameter

	mov	ax,cs
	mov	es,ax

	;store D

	lds	si,dword ptr cs:[D_OFF]
	mov	di,offset BUFFER
	mov	cs:[Dmemadr],di
	lodsw
	and	ax,lenmask
	stosw
	mov	cx,ax
	rep	movsw

	;store B

	lds	si,dword ptr cs:[B_OFF]
	mov	cs:[Bmemadr],di
	lodsw
	and	ax,lenmask
	stosw
	mov	cx,ax
	rep	movsw

	; loop for primes

	mov	es,cs:[primeseg]
	mov	si,2*primeunitbytes	;skip sign & 2
	mov	cx,cs:[primes]
	dec	cx			;skip sign & 2
	even
R1R2loop:
	push	cx
	mov	bx,es:[si]
	call	R1R2setparam
	add	si,primeunitbytes
	pop	cx
	loop	R1R2loop

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf

	;data structure
	; prime 	2bytes
	; sqrt(WN) @ P	2bytes
	; log(P) 	2bytes(higher=0)
	; start1	2 bytes
	; start2	2 bytes
	; ( total	10 bytes=primeunitbytes)

	; calc parameters for each prime
R1R2setparam:
	push	si		;*

	mov	si,cs
	mov	ds,si
	mov	si,[Dmemadr]
	mov	ax,[si]
	mov	cx,ax
	shl	ax,1
	add	si,ax

	xor	dx,dx
	std
	even
R1R2setp10:
	lodsw
	div	bx
	loop	R1R2setp10
	cld

	mov	ax,dx
	mul	dx
	div	bx
	mov	ax,dx
	shl	ax,1
	jnc	R1R2setp15	
	sub	ax,bx
R1R2setp15:
	call	ax_modinv_bx

	mov	[MIA2P],ax	;set 1/(2A) @ P

	mov	si,[Bmemadr]
	mov	ax,[si]
	mov	cx,ax
	shl	ax,1
	add	si,ax

	xor	dx,dx
	std
	even
R1R2setp20:
	lodsw
	div	bx
	loop	R1R2setp20
	cld

	pop	si		;*

	mov	ax,es:[si+2]
	add	dx,ax
	cmp	dx,bx
	jbe	short setparamj1
	sub	dx,bx
setparamj1:
	mov	ax,bx
	sub	ax,dx			;(-B-R)@P

	mul	[MIA2P]
	div	bx			;dx = (-B-R)@P/2A@P

	mov	ax,dx
	xor	dx,dx
	add	ax,[sievewidth]
	adc	dx,0
	div	bx

	push	dx
	xor	dx,dx
	mov	ax,[sievewidth+2]
	div	bx
	pop	ax
	div	bx
	mov	es:[si+6],dx		;((-B-R)@P/2A+M)@P

	mov	ax,es:[si+2]
	shl	ax,1
	mul	[MIA2P]			;2R/2A
	div	bx			;dx = 2R/2A@P
	add	dx,es:[si+6]
	cmp	dx,bx
	jb	short setparamj3
	sub	dx,bx
setparamj3:
	mov	es:[si+8],dx		;((-B+R)@P/2A+M)@P
	ret

;
; inverse modulo prime
; 
; inp : ax
; out : ax = 1/ax mod bx
; destroy : cx,si,di

ax_modinv_bx:
	cmp	ax,1
	je	short modinvret
	mov	cx,bx
	cmp	ax,bx
	jb	short modinv10
	xor	dx,dx
	div	bx
	mov	ax,dx
	cmp	ax,1
	je	short modinvret
modinv10:
	xchg	ax,cx
	xor	si,si		;coef1
	mov	di,1		;coef2
modinv20:
	xor	dx,dx
	div	cx
	sub	cx,dx
	cmp	dx,cx
	jbe	short modinv60

	push	cx		;new remainder
	add	cx,dx
	inc	ax		;inc quotient

	mul	di
	div	bx
	mov	ax,dx
	sub	ax,si
	jae	short modinv30
	add	ax,bx
modinv30:
	mov	si,di		;new coef1=old coef2
	mov	di,ax		;new coef2=quotient*coef2-coef1

	mov	ax,cx
	pop	cx
	cmp	cx,1
	jne	modinv20	;GCD must 1 otherwise endlessloop
	mov	ax,di
modinvret:
	ret
	
modinv60:
	add	cx,dx
	push	dx		;remainder

	mul	di
	div	bx
	mov	ax,si
	sub	ax,dx
	jae	short modinv70
	add	ax,bx
modinv70:
	mov	si,di		;new coef1=old coef2
	mov	di,ax		;new coef2=coef1-quotient*coef2

	mov	ax,cx
	pop	cx
	cmp	cx,1
	jne	modinv20	;GCD must 1 otherwise endlessloop
	mov	ax,di
	ret


;
; * set data for each prime
;
;command#=1

setprime:
	mov	ax,cs
	mov	ds,ax

	les	di,dword ptr cs:[primeadrnow]
	mov_ax	AR1		;prime
	shl	ax,1
	inc	ax
	stosw
	mov_ax	AR2		;sqrt of N mod p
	stosw
	mov_ax	AR3		;log
	stosw
	add	di,primeunitbytes-6
	mov	word ptr cs:[primeadrnow],di

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf


;
;** initialize work area

;command#=0

;	V1 = N
;	V2 = X
;	V3 = Y
;	V4 = W
;	V5 = D
;	V6 = B
;	V7 = R
;	V8 = PRM()
;	V9 = SIEVE()

;	AR[1]	SZ
;	AR[2]	INITIAL LOG
;	AR[3]	LIMIT LOG
;	AR[4]	sievearea/2^16

INIT:
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	MOV	BX,V1		;set N ADDRESS
	MOV	AX,[BX]
	MOV	[N_OFF],AX
	MOV	AX,[BX+2]
	MOV	[N_SEG],AX

	MOV	BX,V2		;set X ADDRESS
	MOV	AX,[BX]
	MOV	[X_OFF],AX
	MOV	AX,[BX+2]
	MOV	[X_SEG],AX

	MOV	BX,V3		;set Y ADDRESS
	MOV	AX,[BX]
	MOV	[Y_OFF],AX
	MOV	AX,[BX+2]
	MOV	[Y_SEG],AX

	MOV	BX,V4		;set W ADDRESS
	MOV	AX,[BX]
	MOV	[W_OFF],AX
	MOV	AX,[BX+2]
	MOV	[W_SEG],AX

	MOV	BX,V5		;set D ADDRESS
	MOV	AX,[BX]
	MOV	[D_OFF],AX
	MOV	AX,[BX+2]
	MOV	[D_SEG],AX

	MOV	BX,V6		;set B ADDRESS
	MOV	AX,[BX]
	MOV	[B_OFF],AX
	MOV	AX,[BX+2]
	MOV	[B_SEG],AX

	MOV	BX,V7		;set R ADDRESS
	MOV	AX,[BX]
	MOV	[R_OFF],AX
	MOV	AX,[BX+2]
	MOV	[R_SEG],AX

	MOV	BX,V8		;set prm() ADDRESS
	mov	ax,[bx]
	shr	ax,1
	shr	ax,1
	shr	ax,1
	shr	ax,1
	add	ax,[bx+2]
	mov	[primeseg],ax

	MOV	BX,V9		;set sieve() ADDRESS
	mov	ax,[bx]
	shr	ax,1
	shr	ax,1
	shr	ax,1
	shr	ax,1
	add	ax,[bx+2]
	mov	[sieveseg],ax

	MOV	BX,V10		;set ans# ADDRESS
	MOV	AX,[BX]
	MOV	[ANS_OFF],AX
	MOV	AX,[BX+2]
	MOV	[ANS_SEG],AX

	MOV_AX	AR1		;SIZE OF MATRIX
	mov	[FBsize],AX
	sub	ax,17
	mov	[primes],ax

	MOV_AX	AR2		;init log
	mov	ah,al
	mov	[inilog],ax

	MOV_AX	AR3		;error log
	mov	[cutlog],ax

	xor	ax,ax
	mov	[sievewidth],ax
	mov	[sievewidth+2],ax
	MOV_AX	AR4		;sievewidth/256
	mov	[sievewidth+1],ax

	mov	es,[primeseg]
	mov	bx,primeunitbytes	;skip -1
	mov	word ptr es:[bx],2
	mov	word ptr es:[bx+2],1
	MOV_AX	AR5			;log of 2
	mov	es:[bx+4],ax
	mov	[primeadrnow],2*primeunitbytes

	mov	ax,ss
	mov	ds,ax
	mov	es,ax
	retf



	EVEN
MSBoffset	dw	?
MSBmask		dw	?
BUFFER		dw	maxbufferwords dup (0)
BUFFER2		dw	maxbufferwords dup (0)


CODE	ENDS
END	START
