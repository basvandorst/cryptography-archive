;MPQS#10.ASM
;	MACHINE LANGUAGE SUBROUTINE
;	FOR MULTIPLE POLYNOMIAL QUADRATIC SIEVE
;
;	Original Version           1987 by YUJI KIDA
;	Fantastic? English Version 1988 by Yuji KIDA

	INCLUDE	UB.MAC

	JMP	START0

	EVEN
SIEVESEG	DW	?

SEGSTEP	DW	?
SEGTOP	DW	?
SEGEND	DW	?

MSIZE	DW	?
MWORDS	DW	?
ROWWORDS	DW	?
OFF2ND	DW	?
OFF3RD	DW	?

ROWINDEXMIN	DW	?
ROWINDEXNOW	DW	?
ROWINDEXNOW2	DW	?
ROWINDEXLIM	DW	?
ROWINDEXMAX	DW	?
DS_INDEX	DW	?

SEGNOW	DW	?
OFFNOW	DW	?
MASKNOW	DW	?
ANSNOW	DW	?

OFF1	DW	?
SEG1	DW	?
OFF2	DW	?
SEG2	DW	?

N_OFF	DW	?
N_SEG	DW	?
X_OFF	DW	?
X_SEG	DW	?
Y_OFF	DW	?
Y_SEG	DW	?
WW_OFF	DW	?
WW_SEG	DW	?
XA_SEG	DW	?
YA_SEG	DW	?

YR_SEG	DW	?
YR_SIZE	DW	?
YR_KOSU	DW	?
MAXLOW	DW	?
MAXHIGH	DW	?

P_OFF	DW	?
P_SEG	DW	?
P_NOW2	DW	?

STOP_PTR	DW	?

ADR_NOW	DW	?
REENT	DB	?,?

LOG_INI	DW	?
LOG_MAX	DW	?



;** COMMAND BRANCH


START0:
	MOV_AX	AR0		;get COMMAND frm ARRAY[0]
	MOV	BX,OFFSET CMD_TBL
	SHL	AX,1
	ADD	BX,AX
	JMP	CS:[BX]

CMD_TBL:
	DW	INIT,SIEVE,SIEVE_ANS,TEST_DIV,LPV
	DW	SET_ROW,GAUSS,SQUARE_ANS,GET_DIV,RE_INIT


;
;** reinitilization if one could not find a factor after 1st 
;   GAUSSian elimination
;
; COMMAND#=9

RE_INIT:
	;erase the column corresponding to discarded X

	MOV	SI,CS:[ROWINDEXNOW]
REINI10:
	CMP	SI,CS:[ROWINDEXMAX]
	JB	REINI15

	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	RETF

REINI15:
	MOV	ES,CS:[SI]	;*keep ES,SI through main loop
	MOV	DI,CS:[OFF3RD]
	SUB	DI,2
	MOV	CX,CS:[MWORDS]
	INC	CX
	XOR	AX,AX
	STD
	REP	SCASW
	CLD
	LEA	BX,[DI+2]
	MOV	AX,ES:[BX]
	MOV	DX,8000H
REINI20:
	TEST	DX,AX
	JNZ	REINI30
	SHR	DX,1
	JMP	REINI20		
REINI30:				
	;NOW DX is the highest bit of [BX]
	;erase this bit in the other rows

	MOV	DI,CS:[ROWINDEXMIN]
REINI40:
	CMP	DI,SI
	JE	REINI60		;do not erase the bit itself
	CMP	DI,CS:[ROWINDEXMAX]
	JAE	REINI70		;last pass
	MOV	DS,CS:[DI]
	TEST	[BX],DX
	JZ	REINI60		;not match

	PUSH	SI
	MOV	SI,CS:[OFF2ND]
	MOV	CX,CS:[MWORDS]
	INC	CX
REINI50:
	MOV	AX,ES:[SI]
	XOR	[SI],AX
	ADD	SI,2
	LOOP	REINI50
	POP	SI

REINI60:
	ADD	DI,2
	JMP	REINI40

REINI70:
	;set corresponding unit vector

	MOV	DI,CS:[OFF2ND]
	MOV	CX,CS:[MWORDS]
	INC	CX
	XOR	AX,AX
	REP	STOSW		;SET 07s
	MOV	ES:[BX],DX	;SET 1

	;set pointers to X(),Y()

	SUB	BX,CS:[OFF2ND]
	SHL	BX,1
	SHL	BX,1
	SHL	BX,1
REINI110:
	INC	BX
	SHR	DX,1
	JNC	REINI110
	DEC	BX
	MOV_AX	LSIZE
	MUL	BX
	MOV	BX,CS:[OFF3RD]
	MOV	ES:[BX],AX

	ADD	SI,2
	JMP	REINI10

;
;** get exponents of each prime in the factorization of X
;
; COMMAND#=8
; INPUT Y
; OUTPUT SV%()

GET_DIV:
	PUSH	BP

	LDS	SI,DWORD PTR CS:[Y_OFF]
	MOV	BX,SI		;MEMO dividend
	MOV	ES,CS:[P_SEG]
	MOV	DI,AR0+10	;start at 2
	MOV	CS:[OFFNOW],0

	STD
	MOV	CX,CS:[MSIZE]
	DEC	CX		;skip -1
GDIVLP:
	MOV	BP,ES:[DI]	;divisor
	ADD	CS:[OFFNOW],2
	ADD	DI,10
	PUSH	CX
GDIV10:
	MOV	SI,BX
	MOV	AX,[SI]
	MOV	CX,AX		;WORDS
	SHL	AX,1
	ADD	SI,AX		;highest word adr
	XOR	DX,DX	
GDIV20:
	LODSW
	DIV	BP
	LOOP	GDIV20
	OR	DX,DX
	JZ	GDIV100		;divide if divisible

	POP	CX
	LOOP	GDIVLP

	POP	BP
	CLD
	RETF

GDIV100:
	MOV	AX,DS
	MOV	DS,CS:[SIEVESEG]
	MOV	SI,CS:[OFFNOW]
	INC	WORD PTR [SI]
	MOV	DS,AX
	MOV	SI,BX
	MOV	AX,[SI]
	MOV	CX,AX		;WORDS
	SHL	AX,1
	ADD	SI,AX		;highest word adr
	XOR	DX,DX	
	LODSW
	DIV	BP
	PUSH	AX		;value of highest word
	JMP	SHORT GDIV120
GDIV110:
	LODSW
	DIV	BP
GDIV120:
	MOV	[SI+2],AX
	LOOP	GDIV110
	POP	AX		;value of highest word
	OR	AX,AX
	JNZ	GDIV10
	DEC	WORD PTR [SI]	;dec len if highest word=0
	JMP	GDIV10

;
;** GAUSSian elimination
;
;COMMAND#=4
;

GO_GAUSSNEXT_C:
	JMP	GAUSSNEXT_C


GAUSS:
	MOV	SI,CS:[ROWINDEXNOW]
	MOV	CS:[ROWINDEXLIM],SI
	MOV	ES,CS:[SI]
	XOR	DI,DI
	MOV	CX,CS:[MWORDS]
	MOV	AX,0FFFFH
	REP	STOSW		;STOPPER

	MOV	SI,CS:[ROWINDEXMIN]
	MOV	CS:[ROWINDEXNOW],SI
	MOV	ES,CS:[SI]
	MOV	BX,CS:[OFF2ND]	;START OFFSET(right column first)
	SUB	BX,2		;
	MOV	DX,8000H	;BIT MASK

;* search this column and exchange the row with 1st nonzero

GAUSS10:
	MOV	SI,CS:[ROWINDEXNOW]
	JMP	SHORT GAUSS25
GAUSS20:
	MOV	SI,CS:[DS_INDEX]
	ADD	SI,2
GAUSS25:
	MOV	CS:[DS_INDEX],SI
	MOV	DS,CS:[SI]
	TEST	[BX],DX
	JZ	GAUSS20

	CMP	SI,CS:[ROWINDEXLIM]
	JAE	GO_GAUSSNEXT_C	;go to next row for no NONZERO
	CMP	SI,CS:[ROWINDEXNOW]
	JE	GAUSS40		;already NONZERO

	;swap POINTERs

	MOV	SI,CS:[DS_INDEX]
	MOV	DI,CS:[ROWINDEXNOW]
	MOV	AX,CS:[SI]
	XCHG	AX,CS:[DI]
	MOV	CS:[SI],AX

	MOV	AX,DS
	MOV	ES,AX

;* eliminate this column

GAUSS40:
	MOV	SI,CS:[DS_INDEX]
	ADD	SI,2
	MOV	CS:[DS_INDEX],SI
	MOV	DS,CS:[SI]

	TEST	[BX],DX
	JZ	GAUSS40		;do not change this row

	CMP	SI,CS:[ROWINDEXLIM]
	JAE	GAUSSNEXT_RC	;go to next row&column

	MOV	CX,CS:[ROWWORDS]
	DEC	CX
	XOR	SI,SI
	EVEN
GAUSS60:
	MOV	AX,ES:[SI]
	XOR	[SI],AX
	INC	SI
	INC	SI
	LOOP	GAUSS60

	JMP	GAUSS40

	;next column or next row&column

GAUSSNEXT_RC:
	MOV	SI,CS:[ROWINDEXNOW]
	ADD	SI,2
	MOV	CS:[ROWINDEXNOW],SI
	MOV	ES,CS:[SI]
GAUSSNEXT_C:
	ROR	DX,1
	JNC	GAUSS100
	SUB	BX,2
	JC	GAUSSEND
GAUSS100:
	JMP	GAUSS10
GAUSSEND:
	MOV	AX,CS:[ROWINDEXNOW]
	MOV	CS:[ROWINDEXNOW2],AX
	SUB	AX,CS:[ROWINDEXMIN]
	SHR	AX,1
	MOV	BX,AR3		;return current rank in
	MOV	CS:[BX],AX	;ARRAY[3]

	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	RETF

;
;** return a value got by SIEVE
;
;COMMAND#=5
;OUTPUT	X=OFFSET 
;	 =-1 NO MORE DATA

SIEVE_ANS:
	MOV	ES,CS:[SIEVESEG]
	MOV	DI,CS:[ADR_NOW]
	OR	DI,DI
	JZ	GETD100
GETD10:
	MOV	AX,CS:[LOG_MAX]
	MOV	CX,DI
	NEG	CX
GETDLP:
	SCASB
	JAE	GETIT
	LOOP	GETDLP
GETNO:
	MOV	AX,8001H
	MOV	DX,1
	MOV	BX,DI
GETOUT:
	LES	DI,DWORD PTR CS:[X_OFF]
	STOSW
	MOV	ES:[DI],DX
	MOV	CS:[ADR_NOW],BX
	MOV	AX,SS
	MOV	ES,AX
	RETF

GETIT:
	MOV	AX,1
	MOV	DX,DI
	DEC	DX
	JNZ	GETIT20
	XOR	AX,AX
GETIT20:
	MOV	BX,DI
	JMP	GETOUT	


GETD100:
	TEST	CS:[REENT],-1
	JNZ	GETNO
	MOV	CS:[REENT],-1
	JMP	GETD10

	
;** do the SIEVING

;COMMAND#=1

SIEVE:
	;initialize sieve

	MOV	AX,CS:[SIEVESEG]
	MOV	DS,AX
	MOV	ES,AX

	XOR	DI,DI
	MOV	CX,8000H
	MOV	AX,CS:[LOG_INI]
	REP	STOSW

	MOV	CS:[ADR_NOW],0
	MOV	CS:[REENT],0

	MOV	AX,CS:[P_SEG]
	MOV	ES,AX	
	MOV	BX,AR0+20	;skip -1,2

	;SIEVE MAIN

	MOV	CX,CS:[MSIZE]
	DEC	CX		;skip -1
	DEC	CX		;skip 2

SIEVE_MAINLP:
	MOV	DI,ES:[BX]	;STEP
	MOV	ax,ES:[BX+8]	;LOG

	MOV	SI,ES:[BX+4]	;START
SIEVE20:
	SUB	[SI],AL
	ADD	SI,DI
	jnc	SIEVE20
	MOV	ES:[BX+4],SI	;R1_OFF(for next entry)

	MOV	SI,ES:[BX+6]	;R2_OFF START
SIEVE40:
	SUB	[SI],AL
	ADD	SI,DI
	jnc	SIEVE40
	MOV	ES:[BX+6],SI	;R2_OFF(for next entry)

	ADD	BX,10		;update POINTER
	LOOP	SIEVE_MAINLP

	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	RETF
	

;
;* use large 'primes' left after division
;	(CALLED LARGE PRIME VARIATION)

;COMMAND#=3
;OUTPUT
;exists:  AR[0]=1,AR[1]=OLD INDEX
;insert:  AR[0]=0,AR[1]=NEW INDEX
;discard: AR[0]=0,AR[1]=-1

LPV:
	; GET W

	LDS	SI,DWORD PTR CS:[WW_OFF]
	LODSW
	MOV	CX,AX		;LEN
	XOR	DX,DX
	LODSW
	CMP	CX,2
	JB	LARGE10
	JA	LARGE200
	MOV	DX,[SI]
LARGE10:
	;SEARCH FOR DX:AX IN ZR
	; SO ONLY NUMBERS <=2 WORDS CAN BE TREATED

	CMP	DX,CS:[MAXHIGH]
	JA	LARGE200
	JB	LARGE15
	CMP	AX,CS:[MAXLOW]
	JA	LARGE200
LARGE15:
	MOV	SI,CS:[YR_SEG]
	MOV	DS,SI
	MOV	ES,SI	

	;FIRST BINARY SEARCH

	MOV	BX,CS:[YR_KOSU]
	SHL	BX,1
	MOV	DI,OFFSET INDEX
LARGE20:
	SHR	BX,1
	CMP	BX,1
	JBE	LARGE100

	MOV	CX,BX
	AND	CL,0FEH
	ADD	DI,CX
	MOV	SI,CS:[DI]
	CMP	DX,[SI+2]
	JB	LARGE40		;D:A<MEM
	JA	LARGE45		;D:A>MEM
	CMP	AX,[SI]
	JA	LARGE45		;D:A>MEM
	JE	LARGE50		;D:A=MEM
LARGE40:
	SUB	DI,CX
	JMP	LARGE20

LARGE45:
	ADD	DI,2
	DEC	BX
	JMP	LARGE20

LARGE50:
	;if 2 large primes are equal

	SHR	SI,1
	SHR	SI,1		;offset of the old 'prime'
	MOV	BX,AR1
	MOV	CS:[BX],SI	;return it in AR[1]

	MOV	BX,AR0
	MOV	WORD PTR CS:[BX],1	;signal the match
	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	RETF

LARGE200:
	MOV	SI,8001H
	MOV	BX,AR1		;return -1 in AR[1]
	MOV	CS:[BX],SI
	JMP	LARGERET


	;NEXT CHECK CURRENT POSITION

LARGE100:
	MOV	SI,CS:[DI]
	CMP	DX,[SI+2]
	JB	LARGE140	;D:A<MEM
	JA	LARGE110
	CMP	AX,[SI]
	JE	LARGE50		;D:A=MEM
	JB	LARGE140	;D:A<MEM
LARGE110:
	ADD	DI,2
	MOV	SI,CS:[DI]

LARGE140:
	CMP	[SI+2],0FFFFH	;STOPPER
	JE	LARGE200	;no free space
	CMP	[SI+2],0FFFEH	;DUMMY DATA
	JNE	LARGE160

	;insert it here

	INC	CS:[YR_KOSU]
	MOV	[SI],AX
	MOV	[SI+2],DX
	SHR	SI,1
	SHR	SI,1
	MOV	BX,AR1		;return array offset
	MOV	CS:[BX],SI	;in AR[1]
	JMPS	LARGERET	

LARGE160:
	;free a space in the INDEX AREA

	PUSH	AX		;*
	MOV	BX,DI		;MEMO PTR
	MOV	DI,CS:[YR_KOSU]
	CMP	DI,CS:[YR_SIZE]
	JB	LARGE165
	DEC	DI
LARGE165:
	SHL	DI,1
	ADD	DI,OFFSET INDEX
	PUSH	WORD PTR CS:[DI]	;** offset for new prime
	LEA	SI,[DI-2]
	MOV	CX,DI
	SUB	CX,BX
	SHR	CX,1
	PUSH	DS		;***
	MOV	AX,CS
	MOV	DS,AX
	MOV	ES,AX
	STD
	REP	MOVSW
	CLD
	POP	AX		;***
	MOV	DS,AX
	MOV	ES,AX

	MOV	DI,BX
	POP	SI		;**
	MOV	CS:[DI],SI
	POP	AX		;*
	MOV	[SI],AX
	MOV	[SI+2],DX
	SHR	SI,1
	SHR	SI,1
	MOV	BX,AR1		;return array offset
	MOV	CS:[BX],SI	;in AR[1]

	;is YR full?

	MOV	AX,CS:[YR_KOSU]
	MOV	BX,CS:[YR_SIZE]
	CMP	AX,BX
	JAE	LARGE170	;full
	INC	AX
	MOV	CS:[YR_KOSU],AX
	CMP	AX,BX
	JAE	LARGE170	;full

LARGERET:
	MOV	BX,AR0
	MOV	WORD PTR CS:[BX],0	;mark of non_existence
	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	RETF	
	
	;update LOG LIMIT
	;to (maximum LP)^2

LARGE170:
	DEC	BX
	SHL	BX,1
	ADD	BX,OFFSET INDEX
	MOV	SI,CS:[BX]	
	LODSW
	MOV	CS:[MAXLOW],AX
	MOV	CX,16
	MOV	DX,[SI]
	MOV	CS:[MAXHIGH],DX
	OR	DX,DX
	JZ	LARGE190
	MOV	CX,32
	MOV	AX,DX
LARGE190:
	DEC	CX
	SHL	AX,1
	JNC	LARGE190

	MOV	AX,CX
	SHL	AX,1
	SHL	AX,1
	CMP	AX,CS:[LOG_MAX]
	JAE	LARGE195
	MOV	CS:[LOG_MAX],AX
LARGE195:
	JMP	LARGERET


;** check whether W# is divisible by PR%()

;COMMAND#=7

TEST_DIV:
	PUSH	BP

	LDS	SI,DWORD PTR CS:[WW_OFF]
	MOV	BX,SI		;MEMO dividend
	MOV	ES,CS:[P_SEG]
	MOV	DI,AR0+10	;from 2

	STD
	MOV	CX,CS:[MSIZE]
	DEC	CX		;pass -1
TDIVLP:
	PUSH	CX
	MOV	BP,ES:[DI]	;numerator
	ADD	DI,10
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
;** initialize work area

;COMMAND#=0

;	V1=PR%(0)
;	V2=W#
;	V3=N
;	V4=X
;	V5=Y
;	V6=X(0)
;	V7=Y(0)
;	V8=YR%(0)
;	V9=SV%(0)

;	AR[1]	SIZE OF MATRIX
;	AR[2]	SIZE OF LARGE PRIMES BUFFER
;	AR[3]	INITIAL LOG
;	AR[4]	LIMIT LOG

INIT:
	PUSH	BP

	MOV	BX,V1		;set PR%(0,0) ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[P_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[P_SEG],AX

	MOV	BX,V2		;set W# ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[WW_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[WW_SEG],AX

	MOV	BX,V3		;set N ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[N_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[N_SEG],AX

	MOV	BX,V4		;set X ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[X_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[X_SEG],AX

	MOV	BX,V5		;set Y ADDRESS
	MOV	AX,CS:[BX]
	MOV	CS:[Y_OFF],AX
	MOV	AX,CS:[BX+2]
	MOV	CS:[Y_SEG],AX

	MOV_AX	V6+2		;set X(0) ADDRESS
	ADD	AX,ARRAYHEADSEG
	MOV	CS:[XA_SEG],AX

	MOV_AX	V7+2		;set Y(0) ADDRESS
	ADD	AX,ARRAYHEADSEG
	MOV	CS:[YA_SEG],AX

	MOV_AX	V9+2		;WORK AREA FOR SIEVING
	ADD	AX,ARRAYHEADSEG
	MOV	CS:[SIEVESEG],AX

	MOV_AX	AR1		;SIZE OF MATRIX
	MOV	CS:[MSIZE],AX

	;set index area for YR()

	MOV_AX	AR2		;number of YR()'s
	MOV	CS:[YR_SIZE],AX
	INC	AX		;1 more for end effect
	SHL	AX,1
	SHL	AX,1		;4 BYTES per element
	ADD	AX,OFFSET INDEX

	;set index area for GAUSSian elimination

	MOV	CS:[ROWINDEXMIN],AX
	MOV	CS:[ROWINDEXNOW],AX
	MOV	BX,CS:[MSIZE]	
	INC	BX		;ADDITIONAL ROW
	INC	BX		;STOPPER ROW
	SHL	BX,1		;2 BYTES per element
	ADD	AX,BX		;AX=OFFSET OF NEXT BYTE OF INDEX AREA

	;set MATRIX AREA

	ADD	AX,15
	SHR	AX,1
	SHR	AX,1
	SHR	AX,1
	SHR	AX,1
	MOV	BX,CS
	ADD	AX,BX
	MOV	CS:[SEGTOP],AX	;first segment of matrix

	MOV	AX,CS:[MSIZE]	;MUST BE A MUTIPLE OF 16
	SHR	AX,1
	SHR	AX,1
	SHR	AX,1
	MOV	CS:[OFF2ND],AX	;OFFSET OF 2ND MATRIX
	SHR	AX,1
	MOV	CS:[MWORDS],AX	;MATRIX WORD SIZE
	SHL	AX,1		;use 2 matrices
	INC	AX		;2nd matrix needs 1 bit more
	SHL	AX,1
	MOV	CS:[OFF3RD],AX
	SHR	AX,1
	INC	AX		;for pointers to X(),Y()
	MOV	CS:[ROWWORDS],AX	;number of words in 1 row
	ADD	AX,7		;round up for SEGMENTs
	SHR	AX,1
	SHR	AX,1
	SHR	AX,1
	MOV	CS:[SEGSTEP],AX

	MOV	AX,CS:[MSIZE]
	INC	AX		;ADDITIONAL ROW
	MUL	CS:[SEGSTEP]
	ADD	AX,CS:[SEGTOP]
	MOV	CS:[SEGEND],AX	;first segment after the matrix
	
	;set 2ND MATRIX to unit matrix

	MOV	BX,CS:[SEGTOP]
	MOV	DI,CS:[OFF2ND]
	MOV	CX,CS:[MSIZE]
	INC	CX		;ADDITIONAL ROW
	XOR	AX,AX
	MOV	DX,CS:[SEGSTEP]
INIT10:
	PUSH	CX
	MOV	ES,BX
	MOV	DI,CS:[OFF2ND]
	MOV	CX,CS:[MWORDS]
	REP	STOSW
	STOSW			;ADDITIONAL COLUMN
	ADD	BX,DX
	POP	CX
	LOOP	INIT10

	MOV	AX,CS:[SEGTOP]
	MOV	SI,CS:[OFF2ND]
	MOV	BX,CS:[SEGSTEP]
	MOV	CX,CS:[MWORDS]	;MSIZE/16
	MOV	DX,1		;DATA
INIT20:
	MOV	DS,AX
	MOV	[SI],DX
	ADD	AX,BX
	ROL	DX,1
	JNC	INIT20	
	ADD	SI,2
	LOOP	INIT20
	MOV	DS,AX
	MOV	[SI],DX		;ADDITIONAL ROW & COLUMN

	;set pointers to X(),Y()

	MOV_AX	LSIZE
	MOV	DX,AX		;number of bytes of X(),Y()
	MOV	AX,CS:[SEGTOP]
	MOV	SI,CS:[OFF3RD]
	MOV	BX,CS:[SEGSTEP]
	MOV	CX,CS:[MSIZE]
	INC	CX
	XOR	DI,DI
INIT30:
	MOV	DS,AX
	MOV	[SI],DI
	ADD	AX,BX
	ADD	DI,DX
	LOOP	INIT30
	MOV	DS,AX
	MOV	[SI],DI

	;initialize index for GAUSSian elimination

	MOV	AX,CS
	MOV	ES,AX
	MOV	DI,CS:[ROWINDEXMIN]
	MOV	CX,CS:[MSIZE]
	MOV	AX,CS:[SEGTOP]
	MOV	DX,CS:[SEGSTEP]
INIT40:
	STOSW
	ADD	AX,DX
	LOOP	INIT40
	STOSW			;ADDITIONAL ROW
	MOV	CS:[ROWINDEXMAX],DI	;STOPPER POSITION
	ADD	AX,DX
	STOSW			;STOPPER ROW

	;initialize YR%()

	MOV	BX,V8
	MOV	AX,CS:[BX+2]
	ADD	AX,ARRAYHEADSEG
	MOV	CS:[YR_SEG],AX
	MOV	ES,AX
	MOV	CX,CS:[YR_SIZE]
	XOR	DI,DI
	XOR	AX,AX
	MOV	BX,0FFFEH	;DUMMY DATA
INIT60:
	STOSW
	MOV	ES:[DI],BX
	ADD	DI,2
	LOOP	INIT60	
	STOSW
	INC	BX
	MOV	ES:[DI],BX	;FFFFH=STOPPER
	MOV	CS:[STOP_PTR],DI

	;initialize index for YR%()

	MOV	AX,CS
	MOV	ES,AX
	MOV	DI,OFFSET INDEX
	XOR	AX,AX
	MOV	CS:[YR_KOSU],AX
	DEC	AX
	MOV	CS:[MAXLOW],AX
	MOV	CS:[MAXHIGH],AX
	INC	AX
	MOV	CX,CS:[YR_SIZE]
INIT70:
	STOSW
	ADD	AX,4
	LOOP	INIT70	
	STOSW

	MOV_AX	AR3
	MOV	AH,AL
	MOV	CS:[LOG_INI],AX
	MOV_AX	AR4
	MOV	CS:[LOG_MAX],AX

	;clear index

	XOR	AX,AX
	MOV	[ROWINDEXNOW],AX

	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	POP	BP
	RETF


;
; ** set a row
;    make row data from W#,X satisfying W#=X^2

;COMMAND#=2

SET_ROW:
	PUSH	BP

	;copy X into X()

	MOV	SI,CS:[ROWINDEXNOW]
	MOV	AX,CS:[SI]
	MOV	CS:[SEGNOW],AX
	MOV	DS,AX
	MOV	BX,CS:[OFF3RD]
	MOV	DI,[BX]	
	MOV	ES,CS:[XA_SEG]
	LDS	SI,DWORD PTR CS:[X_OFF]
	MOV	CX,[SI]
	AND	CX,3FFFH
	INC	CX
	REP	MOVSW

	;copy W into Y()

	MOV	DS,CS:[SEGNOW]
	MOV	BX,CS:[OFF3RD]
	MOV	DI,[BX]	
	MOV	ES,CS:[YA_SEG]
	LDS	SI,DWORD PTR CS:[WW_OFF]
	MOV	CX,[SI]
	AND	CX,3FFFH
	INC	CX
	REP	MOVSW

;
;* divide W#
;

	LDS	SI,DWORD PTR CS:[WW_OFF]
	MOV	AX,[SI]
	AND	AX,8000H
	ROL	AX,1
	AND	WORD PTR [SI],3FFFH
	MOV	BX,SI		;MEMO dividend
	MOV	ES,CS:[P_SEG]
	MOV	DI,AR0+10	;pointer to PR%(1,0)

	MOV	CS:[ANSNOW],AX	;16 bit result
	MOV	CS:[MASKNOW],2
	MOV	CS:[OFFNOW],0

	STD

	MOV	CX,CS:[MSIZE]
	DEC	CX		;skip -1
DIVLP:
	MOV	BP,ES:[DI]	;divisor
	ADD	DI,10
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
	JZ	DIV100		;divide if divisible

	ROL	CS:[MASKNOW],1
	JNC	DIV30
	PUSH	DS		;set result after each word
	PUSH	BX
	MOV	DS,CS:[SEGNOW]
	MOV	BX,CS:[OFFNOW]
	MOV	AX,CS:[ANSNOW]
	MOV	[BX],AX		;set result
	MOV	CS:[ANSNOW],0
	ADD	BX,2
	MOV	CS:[OFFNOW],BX
	POP	BX
	POP	DS
DIV30:	
	POP	CX
	LOOP	DIVLP
	CLD

	ADD	CS:[ROWINDEXNOW],2

	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	POP	BP
	RETF			;EXIT POINT


	;if completely decomposed
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


;** answer: square number got by GAUSSian elimination
;     return BIT PATTERN in W#

;COMMAND#=6

	;STARTS HERE

SQUARE_ANS:
	MOV	SI,CS:[ROWINDEXNOW2]
	CMP	SI,CS:[ROWINDEXLIM]
	JAE	NO_ANS

	MOV	ES,CS:[SI]
	MOV	CX,CS:[MWORDS]
	MOV	DI,CX
	INC	CX
	SHL	DI,1
	SHL	DI,1		;ADR OF LAST WORD OF 2ND MATRIX
	XOR	AX,AX
	STD
	REP	SCASW	
	CLD
	INC	CX		;WORDS

	MOV	AX,ES
	MOV	DS,AX
	MOV	SI,CS:[OFF2ND]

	LES	DI,DWORD PTR CS:[WW_OFF]		

	MOV	AX,CX
	STOSW
	REP	MOVSW

	ADD	CS:[ROWINDEXNOW2],2
ANS100:
	MOV	AX,SS
	MOV	DS,AX
	MOV	ES,AX
	RETF


NO_ANS:
	LES	DI,DWORD PTR CS:[WW_OFF]		
	XOR	AX,AX
	STOSW
	JMP	ANS100


	EVEN
INDEX	DW	?

CODE	ENDS
END	START
