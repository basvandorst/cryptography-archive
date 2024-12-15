;ECM2
;Sep 1988 by Yuji KIDA


	INCLUDE	UB.MAC

;N ,M# ,V0(),V1(),V2(),V3(),W1,W2,W3,W4
;v1,v2 ,v3  ,v4  ,v5  ,v6  ,v7,v8,v9,v10

;	reserve address of array 

	MOV	AX,CS
	MOV	DS,AX

	MOV	SI,v3
	MOV	AX,[SI]
	MOV	[MEM1],AX

	MOV	SI,v4
	MOV	AX,[SI]
	MOV	[MEM2],AX

	MOV	SI,v5
	MOV	AX,[SI]
	MOV	[MEM3],AX

	MOV	SI,v6
	MOV	AX,[SI]
	MOV	[MEM4],AX

	JMP	SHORT L10

	EVEN
MEM1	DW	?
MEM2	DW	?
MEM3	DW	?
MEM4	DW	?
J_COUNT	DW	?


L10:

;    10 v2=(v10-v6(0)*v9+v5(0)*v8-v4(0)*v7+v3(0))@v1

	@PUSH	v10
	@PUSH	v6
	@PUSH	v9
	@MUL
	@SUB
	@PUSH	v5
	@PUSH	v8
	@MUL
	@ADD
	@PUSH	v4
	@PUSH	v7
	@MUL
	@SUB
	@PUSH	v3
	@ADD
	@PUSH	v1
	@MOD

	;do not POP

;    20 for J%=1 to 11

	MOV	CS:[J_COUNT],11

L20:
	;go ahed POINTERs

	MOV	AX,CS
	MOV	DS,AX
	MOV	SI,LSIZE
	MOV	AX,[SI]
	MOV	SI,v3
	ADD	[SI],AX
	MOV	SI,v4
	ADD	[SI],AX
	MOV	SI,v5
	ADD	[SI],AX
	MOV	SI,v6
	ADD	[SI],AX

;    30 v2=v2*((v10-v6(J%)*v9+v5(J%)*v8-v4(J%)*v7+v3(J%))@v1)@v1
;       v2 is already in STACK

	@PUSH	v10
	@PUSH	v6
	@PUSH	v9
	@MUL
	@SUB
	@PUSH	v5
	@PUSH	v8
	@MUL
	@ADD
	@PUSH	v4
	@PUSH	v7
	@MUL
	@SUB
	@PUSH	v3
	@ADD
	@PUSH	v1
	@MOD

	@MUL		;with v2
	@PUSH	v1
	@MOD

	;do not POP

;    40 next 

	DEC	CS:[J_COUNT]
	JZ	L50
	JMP	L20

L50:
	;finally pop v2

	@POP	v2

	;restore POINTERs of array

	MOV	AX,CS
	MOV	DS,AX

	MOV	AX,[MEM1]
	MOV	SI,v3
	MOV	[SI],AX

	MOV	AX,[MEM2]
	MOV	SI,v4
	MOV	[SI],AX

	MOV	AX,[MEM3]
	MOV	SI,v5
	MOV	[SI],AX

	MOV	AX,[MEM4]
	MOV	SI,v6
	MOV	[SI],AX

;    60 return

	RETURN
