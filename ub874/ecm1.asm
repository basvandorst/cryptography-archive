;ECM1
;Sep 1988 by Yuji KIDA
;

	INCLUDE	UB.MAC

;   10 *ECM_SUB
;   20 v6=v4:v7=v5:v8=v4:v9=v5

	@LET	v6,v4
	@LET	v7,v5
	@LET	v8,v4
	@LET	v9,v5

;   30 v12=v6+v7:v13=v6-v7

	@PUSH	v6
	@PUSH	v7
	@ADD
	@POP	v12

	@PUSH	v6
	@PUSH	v7
	@SUB
	@POP	v13

	JMP	SHORT L40

;   40 for I%=1 to len(v3)-1

V3COUNT	DB	?
	EVEN
V3ADR	DW	?
V3SEG	DW	?
V3WORD	DW	?	

L40:
	MOV	BX,v3
	LDS	SI,CS:[BX]	;adr of counter
	MOV	AX,[SI]
	AND	AX,3FFFH
	MOV	BX,AX		;counter word length 
	MOV	CL,4
	SHL	AX,CL		;16 ”{
	MOV	CX,AX		;counter bit length
	SHL	BX,1
	MOV	AX,[BX+SI]	;HIGHEST WORD(must be <>0)
	INC	CX
L40A:
	DEC	CX
	SHL	AX,1
	JNC	L40A
	DEC	CX		;LEN-1

	ADD	SI,2
	MOV	AX,[SI]
	SHR	AX,1		;BIT0 is not needed
	MOV	CS:[V3WORD],AX
	MOV	CS:[V3COUNT],15
	MOV	CS:[V3ADR],SI
	MOV	CS:[V3SEG],DS

L40LP:
	PUSH	CX

;   50    v10=v12^2@v1:v11=v13^2@v1

	@PUSH	v12
	@P2
	@PUSH	v1
	@MOD
	@POP	v10

	@PUSH	v13
	@P2
	@PUSH	v1
	@MOD
	@POP	v11

;   60    v6=v10*v11@v1

	@PUSH	v10
	@PUSH	v11
	@MUL
	@PUSH	v1
	@MOD
	@POP	v6

;         v7=(v10-v11)*(v11+v2*(v10-v11)@v1)@v1

	@PUSH	v10
	@PUSH	v11
	@SUB
	@PUSH	v11
	@PUSH	v2
	@PUSH	v10
	@PUSH	v11
	@SUB
	@MUL
	@PUSH	v1
	@MOD
	@ADD
	@MUL
	@PUSH	v1
	@MOD
	@POP	v7

;   70    v12=v6+v7:v13=v6-v7

	@PUSH	v6
	@PUSH	v7
	@ADD
	@POP	v12

	@PUSH	v6
	@PUSH	v7
	@SUB
	@POP	v13

;   80    if bit(I%,v3)

	SHR	CS:[V3WORD],1
	PUSHF
	DEC	CS:[V3COUNT]
	JNZ	L80A
	MOV	CS:[V3COUNT],16
	LDS	SI,CS:DWORD PTR [V3ADR]
	ADD	SI,2
	MOV	CS:[V3ADR],SI
	MOV	AX,[SI]
	MOV	CS:[V3WORD],AX	
L80A:
	POPF
	JC	L90
	JMP	L110

;  90       :then v10=v13*(v4+v5)@v1:v11=v12*(v4-v5)@v1

L90:
	@PUSH	v13
	@PUSH	v4
	@PUSH	v5
	@ADD
	@MUL
	@PUSH	v1
	@MOD
	@POP	v10

	@PUSH	v12
	@PUSH	v4
	@PUSH	v5
	@SUB
	@MUL
	@PUSH	v1
	@MOD
	@POP	v11

; 100          :v4=(v10+v11)^2@v1*v9@v1:v5=(v10-v11)^2@v1*v8@v1

	@PUSH	v10
	@PUSH	v11
	@ADD
	@P2
	@PUSH	v1
	@MOD
	@PUSH	v9
	@MUL
	@PUSH	v1
	@MOD
	@POP	v4

	@PUSH	v10
	@PUSH	v11
	@SUB
	@P2
	@PUSH	v1
	@MOD
	@PUSH	v8
	@MUL
	@PUSH	v1
	@MOD
	@POP	v5

	JMP	L130

;  110       :else v10=v13*(v8+v9)@v1:v11=v12*(v8-v9)@v1

L110:
	@PUSH	v13
	@PUSH	v8
	@PUSH	v9
	@ADD
	@MUL
	@PUSH	v1
	@MOD
	@POP	v10

	@PUSH	v12
	@PUSH	v8
	@PUSH	v9
	@SUB
	@MUL
	@PUSH	v1
	@MOD
	@POP	v11

;  120          :v8=(v10+v11)^2@v1*v5@v1:v9=(v10-v11)^2@v1*v4@v1

	@PUSH	v10
	@PUSH	v11
	@ADD
	@P2
	@PUSH	v1
	@MOD
	@PUSH	v5
	@MUL
	@PUSH	v1
	@MOD
	@POP	v8

	@PUSH	v10
	@PUSH	v11
	@SUB
	@P2
	@PUSH	v1
	@MOD
	@PUSH	v4
	@MUL
	@PUSH	v1
	@MOD
	@POP	v9

; 130 next

L130:
	POP	CX
	DEC	CX
	JZ	L140
	JMP	L40LP

;  140 return

L140:
	RETURN

