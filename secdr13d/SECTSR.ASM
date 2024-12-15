; TSR disk encryptor V1.3d

DISKDATA STRUC
DDDRV	  DB ?	   ;PHYSICAL DRIVE (HD)
DRVLET	  DB ?	   ;DRIVE LETTER
FIRSTCYL  DW ?	   ;FIRST CYLINDER TO ENCRYPT
FIRSTHD   DB ?	   ;FIRST HEAD TO ENCRYPT
FIRSTSEC  DB ?	   ;FIRST SECTOR (BOOT SECTOR)
LASTCYL   DW ?	   ;LAST CYLINDER TO ENCRYPT
MAXSEC	  DB ?	   ;NUMBER OF SECTORS ON DEVICE
MAXHD	  DB ?	   ;NUMBER OF HEADS ON DEVICE
SECSIZE   DW ?	   ;BLOCK SIZE IN BYTES
SERIAL	  DW ?	   ;DISK SERIAL NUMBER, FOR IV
	  DW ?	   ;REST OF DISK SERIAL NUMBER
ACTIVE	  DB ?	   ;CRYPT THIS DISK? FLAG
FLAG2M	  DB ?	   ;2M-FORMATTED DISKETTE IF NZ 	 1.3d
FATSIZ	  DW ?	   ;FAT SIZE FOR 2M-FORMATTED DISKETTES  1.3d
DISKDATA ENDS

MAXDRV=4	   ;MAX HD PARTITIONS IN SAFE MODE EWS

;; .MODEL TINY
;; .CODE
_TEXT SEGMENT PARA PUBLIC 'CODE'
 ASSUME CS:_TEXT,DS:_TEXT
 ORG 0100h
START:
 MOV AX,0800h
 MOV DL,0F0h
 INT 13h
 MOV DX,OFFSET ALRINS
 CMP AX,0EDCCh ;1.3 Already Installed?
 JE  ERREX1
 CMP AX,0EDCBh ;IS Previous Version ALREADY INSTALLED?
 JNE LOADTSR
ERREX1:
 MOV AH,9
 INT 21h ;PRINT ERROR AND EXIT
 INT 20h
LOADTSR:
 MOV AH,9
 MOV DX,OFFSET FKEY
 INT 21h ;PRINT SIGN-ON

 MOV AX,3513h
 INT 21h ;GET INT VECTOR
 MOV WORD PTR[INT_JOUT+1],BX
 MOV WORD PTR[INT_JOUT+3],ES
 MOV WORD PTR REALBIOS,BX
 MOV WORD PTR REALBIOS+2,ES

 MOV DX,OFFSET(I13_ENTRY)
 MOV AX,2513h
 INT 21h ;SET NEW INT VECTOR

 MOV AX,3100h
 MOV DX,(END_OF_FILE-START)/16+17
 INT 21h ;GO TSR
 ALIGN 16
TSRSTACK: ;OUR STACK GOES HERE
 NOP

I13_ENTRY:
 CLI
 MOV WORD PTR CS:STKSEG,SS
 MOV WORD PTR CS:STKOFS,SP
 MOV WORD PTR CS:IV,CS
 MOV SS,WORD PTR CS:IV
 MOV SP,OFFSET TSRSTACK ;SETUP OUR STACK
 STI
 PUSH AX
 PUSH BX
 PUSH CX
 PUSH DX
 PUSH SI
 PUSH DI
 PUSH BP
 PUSH DS
 PUSH ES
 PUSH CS
 POP DS

;DECIDE IF THIS REQUEST NEEDS PROCESSING
 CMP AH,2 ;READ SECTOR
 JE RDORWR
 CMP AH,3 ;WRITE SECTOR
 JE RDORWR
 CMP AH,4 ;WRITE VERIFY
 JE RDORWRC
 CMP AH,8 ;REQUEST FOR ADDRESS?
 JNE TONOPROC
 CMP DL,0F0h ;CORRECT DRIVE NUMBER?
 JNE TONOPROC
 MOV BP,SP
 MOV [BP+10],OFFSET LOADDATA ;DX=DATA ADDRESS
 MOV [BP+12],CS ;CX=CODE SEGMENT
 MOV AX,0EDCCh; ;THIS MEANS V 1.3+ INSTALLED (V1.2- = EDCB)
 JMP RETPROG

TONOPROC:
 JMP NOPROC
;
;2M13-formatted encrypted diskettes must have the buffer
;encryted before & decrypted after operation, same as real write.
;This is because 2M13 moves data from its internal buffers to user
;buffers on write verify.  I believe this is an error and have
;contacted the author of 2M13.	Until a fixed is released, this
;code intercepting write verifies is necessary.
;
RDORWRC:		 ;Write verify's come here 1.3d
 CMP DL,1
 JA  TONOPROC		 ;NO Proc on hard disk
 MOV BP,OFFSET FDB   ;SET DISKDATA BLOCK
 JE  RDO10
 MOV BP,OFFSET FDA   ;SET DISKDATA BLOCK
RDO10:
 CMP [BP].FLAG2M,0   ;IS THIS DISKETTE 2M-FORMATTED?
 JE  TONOPROC	     ;NO
 CMP [BP].ACTIVE,0   ;IS THIS DISKETTE ENCRYPTED & ACTIVE?
 JE  TONOPROC	     ;NO
RDORWR:
 MOV BYTE PTR RQTYPE,AH ;STORE REQUEST TYPE
 MOV BYTE PTR NUMSEC,AL ;STORE NUMBER OF SECTORS
 MOV BYTE PTR STSEC,CL	;STORE SECTOR; MUST BE MASKED
 AND BYTE PTR STSEC,00111111b ;MASK OFF HIGH 2 BITS
 MOV WORD PTR CALLCX,CX ;KEEP CX FOR DISK ACCESS
 MOV AH,CL		;WE NEED TO...
 MOV AL,CH		;SWAP THESE
 MOV CL,6		;ROTATE 6 BITS...
 SHR AH,CL		;TO THE RIGHT, NOW AX CONTAINS CYLINDER
 MOV BYTE PTR DRIVE,DL	;STORE DRIVE
 MOV WORD PTR STCYL,AX	;STORE CYLINDER
 MOV BYTE PTR STHD,DH	;STORE HEAD
 MOV WORD PTR BUFOS,BX	;STORE BUFFER ADDRESS
 MOV WORD PTR BUFSG,ES	;STORE BUFFER SEGMENT

 CMP DL,1	       ;FLOPPY DRIVE? EWS
 JBE FLOPPY
;;; CMP BYTE PTR HDNUM,DL ;IS IT ACTIVE HARD DRIVE?
;;; JNE NOTHARD
 MOV BP,OFFSET HD1   ;SEARCH DISKDATA BLOCKS
 MOV CX,MAXDRV
SHDLP:
 CMP DL,[BP].DDDRV
 JNE SHDCNT
 CMP AX,[BP].FIRSTCYL
 JB  SHDCNT
 CMP AX,[BP].LASTCYL
 JA  SHDCNT
 CMP [BP].ACTIVE,1    ;ACTIVE HARD DISK PARTITION?
 JE  SHDFND	      ;YES
 MOV BYTE PTR BADKEY,1
 JMP BACKFROMBIOS     ;NO. TO RETURN NOT READY
SHDCNT:
 ADD BP,HD2-HD1
 LOOP SHDLP
 JMP NOPROC	      ;NOT ACTIVE & NOT SAFE. PASS THRU
SHDFND:
 MOV AX,OFFSET HKEY
 MOV WORD PTR KEY,AX ;SET KEY
 JMP PROCRQ
NOTHARD:
FLOPPY:
 MOV AX,OFFSET FKEY
 MOV WORD PTR KEY,AX ;SET FLOPPY KEY
 CMP DL,0	     ;IS IT DRIVE A?
 JNE NOTDRIVEA
 MOV BP,OFFSET FDA   ;SET DISKDATA BLOCK
 JMP PROCRQ
NOTDRIVEA:
 CMP DL,1	     ;IS IT DRIVE B?
 JNE NOTDRIVEB
 MOV BP,OFFSET FDB   ;SET DISKDATA BLOCK
 JMP PROCRQ
NOTDRIVEB:
NOPROC2:
 JMP NOPROC

PROCRQ: ;RIGHT DRIVE

 MOV BYTE PTR BADKEY,0 ;CLEAR BAD KEY FLAG

;HERE WE ENCRYPT THE DATA IF IT WAS A WRITE
 CMP BYTE PTR RQTYPE,4 ;WRITE VERIFY ?
 JE  DOENC
 CMP BYTE PTR RQTYPE,3 ;IS IT WRITE?
 JNE NOENCRYPT ;IF NOT, DON'T ENCRYPT
DOENC:
 MOV BYTE PTR ENCRYPT,1 ;SET ENCRYPT MODE
 CALL CRYPTIT ;ENCRYPT THE DATA
 CMP BYTE PTR BADKEY,1
 JE BACKFROMBIOS ;SKIP WRITE IF BAD KEY/NOT LOGGED IN

;HERE WE DO THE ACTUAL DISK OPERATION
NOENCRYPT:
 PUSHF
 PUSH CS
 MOV AX,OFFSET BACKFROMBIOS
 PUSH AX ;HERE WE ARE FAKING AN INT
 MOV AH,BYTE PTR RQTYPE
 MOV AL,BYTE PTR NUMSEC
 MOV CX,WORD PTR CALLCX
 MOV DH,BYTE PTR STHD
 MOV DL,BYTE PTR DRIVE
 MOV ES,WORD PTR BUFSG
 MOV BX,WORD PTR BUFOS
 JMP INT_JOUT ;GO TO BIOS
BACKFROMBIOS:
 PUSHF
 PUSH AX

;HERE WE DECRYPT THE DATA
 MOV BYTE PTR ENCRYPT,0 ;SET DECRYPT MODE
 CALL CRYPTIT ;DECRYPT THE DATA

;HERE WE RETURN TO THE CALLING PROGRAM
 POP AX ;INT RETURN CODE
 POP BX ;FLAGS

 CMP BYTE PTR BADKEY,1 ;RETURN FAKE ERROR?
 JNE NOERRRET
 OR BL,1      ;SET CARRY IF KEY BAD
 MOV AH,80h   ;SIMULATE NO DISK PRESENT ERROR

NOERRRET:
 MOV ES,WORD PTR STKSEG
 MOV SI,WORD PTR STKOFS
 MOV ES:[SI+4],BX ;PUT NEW FLAGS WHERE IRET WILL RECALL THEM

RETPROG:
 POP ES
 POP DS
 POP BP
 POP DI
 POP SI
 POP DX
 POP CX
 POP BX
 CLI
 MOV SS,WORD PTR CS:STKSEG
 MOV SP,WORD PTR CS:STKOFS ;RESTORE USER STACK
 STI
 IRET ;RETURN TO PROGRAM, LOADING MODIFIED FLAGS

NOPROC: ;REQUESTS NOT PROCESSED JUMP HERE
 POP ES
 POP DS
 POP BP
 POP DI
 POP SI
 POP DX
 POP CX
 POP BX
 POP AX
 CLI
 MOV SS,WORD PTR CS:STKSEG
 MOV SP,WORD PTR CS:STKOFS ;RESTORE USER STACK
 STI

INT_JOUT:
 DB 0EAh,00,00,00,00 ;JMP FAR

CRYPTIT: ;THIS ENCRYPTS OR DECRYPTS THE BUFFER
 MOV AL,BYTE PTR NUMSEC
 MOV BYTE PTR SECLFT,AL ;LOAD SECTOR COUNTER
 MOV AX,WORD PTR STCYL
 MOV WORD PTR CURCYL,AX ;LOAD CURRENT CYLINDER
 MOV AL,BYTE PTR STHD
 MOV BYTE PTR CURHD,AL	;LOAD CURRENT HEAD
 MOV AL,BYTE PTR STSEC
 MOV BYTE PTR CURSEC,AL ;LOAD CURRENT SECTOR
 MOV AX,WORD PTR BUFOS
 MOV WORD PTR BUFPS,AX	;LOAD BUFFER POINTER

CRYPTLOOP:
 CMP BYTE PTR SECLFT,0
 JE DONECRYPT ;CHECK FOR LAST SECTOR

 MOV AX,WORD PTR CURCYL
 CMP AX,[BP].FIRSTCYL
 JB DONTCRYPTSEC
 JNE NOTBOOTSEC

 MOV AL,BYTE PTR CURHD
 CMP AL,[BP].FIRSTHD
 JB DONTCRYPTSEC
 JNE NOTBOOTSEC

 MOV AL,BYTE PTR CURSEC
 CMP AL,[BP].FIRSTSEC
;DO WE NEED A JB HERE?
 JNE NOTBOOTSEC

 CALL LOADBS ;LOAD DATA FROM BOOT SECTOR
 JMP DONTCRYPTSEC

NOTBOOTSEC:
 MOV AX,WORD PTR CURCYL
 CMP AX,[BP].LASTCYL
 JA DONTCRYPTSEC

 CMP [BP].ACTIVE,1
 JE DOCRYPTSEC	  ;CHECK ACTIVE FLAG
 CMP BYTE PTR DRIVE,1
 JBE DONTCRYPTSEC
 MOV BYTE PTR BADKEY,1
 JMP DONTCRYPTSEC
DOCRYPTSEC:
 CALL CRYPTSEC ;ENCRYPT CURRENT SECTOR
DONTCRYPTSEC:
 CALL NEXTSEC  ;GO ON TO NEXT SECTOR
 JMP CRYPTLOOP	   ;LOOP
DONECRYPT: RET

CRYPTSEC: ;HERE WE CRYPT ONE SECTOR
;SET UP IV
;For 2M-formatted diskettes, IV must be the same for FAT1 and FAT2
;because 2M omits FAT2 and remaps read requests to FAT1  1.3d
 CMP [BP].FLAG2M,0     ;Must be 2M-formatted diskette
 JE  NOT_FAT2
 MOV AX,WORD PTR CURCYL
 CMP AX,2
 JA  NOT_FAT2
 SHL AL,1		 ;ALWAYS 2 HD/CYL FOR DISKETTES
 PUSH CX
 MOV CL,BYTE PTR CURHD
 MOV CH,0
 ADD AX,CX
 MUL   [BP].MAXSEC
 MOV CL,BYTE PTR CURSEC
 MOV CH,0
 ADD   AX,CX		 ; sector
 DEC   AX		 ; AX = Relative DOS Sector
 MOV CX,[BP].FATSIZ
 CMP AX,CX
 JNA NOT_FAT2P	       ;Below FAT 2
 SHL CX,1
 CMP AX,CX
 JA  NOT_FAT2P	       ;Above FAT 2
 SUB AX,[BP].FATSIZ    ;in FAT 2 of 2M diskette. Dup FAT1 IV
 DIV   [BP].MAXSEC
 INC   AH		 ; Numbered from 1...
 MOV   BYTE PTR IV+3,AH  ; ...the Remainder is the sector
 SHR   AL,1
 MOV   AH,0
 MOV   WORD PTR IV,AX	 ; Cylinder
 RCL   AL,1
 AND   AL,1
 MOV   BYTE PTR IV+2,AL  ; Head
 POP   CX
 JMP FATCOM
NOT_FAT2P:
 POP CX
NOT_FAT2:
 MOV AX,WORD PTR CURCYL
 MOV WORD PTR IV,AX
 MOV AL,BYTE PTR CURHD
 MOV BYTE PTR IV+2,AL
 MOV AL,BYTE PTR CURSEC
 MOV BYTE PTR IV+3,AL
FATCOM:
 MOV AX,[BP].SERIAL
 MOV WORD PTR IV+4,AX
 MOV AX,[BP].SERIAL+2
 MOV WORD PTR IV+6,AX ;SET UP IV

;PRE-ENCRYPT IV
 MOV AX,1  ;ACTUALLY ZERO BLOCKS, WILL JUST PRE-ENCRYPT IV
 PUSH AX   ;STORE NUMBER OF BLOCKS
 SUB SP,8 ;PRETEND TO PUSH PLAINTEXT/CIPHERTEXT ADDRESSES
 MOV AX,WORD PTR KEY
 PUSH CS
 PUSH AX
 MOV AX,OFFSET IV
 PUSH CS
 PUSH AX
 PUSH AX ;MAKE IT LOOK LIKE A FAR CALL FOR IDEACFB
 CALL _IDEACFB
 ADD SP,20 ;REMOVE EXTRA WORD

;ENCRYPT/DECRYPT THE BLOCK
 MOV AX,[BP].SECSIZE
 MOV CL,3
 SHR AX,CL ;SECSIZE/8
 INC AX    ;ONE BLOCK (IV) NOT USED
 PUSH AX   ;STORE NUMBER OF BLOCKS
 MOV AX,WORD PTR BUFSG
 MOV BX,WORD PTR BUFPS
 PUSH AX
 PUSH BX
 PUSH AX
 PUSH BX
 MOV AX,WORD PTR KEY
 PUSH CS
 PUSH AX
 MOV AX,OFFSET IV
 PUSH CS
 PUSH AX
 PUSH AX ;MAKE IT LOOK LIKE A FAR CALL FOR IDEACFB
 CMP BYTE PTR ENCRYPT,1
 JNE DECRYPT
 CALL _IDEACFB
 JMP DONE_CRYPTSEC
DECRYPT:
 CALL _IDEACFBX
DONE_CRYPTSEC:
; ADD SP,18
 ADD SP,20 ;REMOVE EXTRA WORD
 RET

NEXTSEC: ;INCREMENT HEAD, SECTOR, CYLINDER AS NEEDED TO GO TO NEXT SECTOR
 MOV AX,[BP].SECSIZE
 ADD WORD PTR BUFPS,AX ;GO TO NEXT BLOCK IN MEMORY
 DEC BYTE PTR SECLFT   ;COUNT DOWN SECTORS LEFT
 INC BYTE PTR CURSEC   ;INCREMENT SECTOR
 MOV AL,BYTE PTR CURSEC
 CMP AL,[BP].MAXSEC
 JBE DONE_COUNT
 MOV BYTE PTR CURSEC,1 ;RESET SECTOR
 INC BYTE PTR CURHD    ;INCREMENT HEAD
 MOV AL,BYTE PTR CURHD
 CMP AL,[BP].MAXHD
 JB DONE_COUNT
 MOV BYTE PTR CURHD,0  ;RESET HEAD
 INC WORD PTR CURCYL   ;THEN INCREMENT CYLINDER
DONE_COUNT: RET

LOADBS:
 CMP BYTE PTR DRIVE,1
 JA IGBS			 ;DON'T ALTER PARAMS FOR HARD DRIVE
 MOV [BP].ACTIVE,0		 ;DEFAULT OFF
 MOV [BP].FLAG2M,0		 ;DEFAULT NOT 2M-FORMATTED 1.3d
 MOV ES,WORD PTR BUFSG
 MOV SI,WORD PTR BUFPS		 ;GET ACCESS TO BUFFER
 CMP WORD PTR ES:[SI+510],0AA55h ;CHECK FOR BOOT-RECORD SIGNATURE
 JNE IGBS			 ;BAILOUT IF BOOT SECTOR LOOKS BAD
 PUSH SI			 ;Check both offsets for CRYP 1.3C
 ADD  SI,WORD PTR INCFOF
 CALL CKCRPF
 POP  SI
 JE   DKCRYPT
 PUSH SI
 ADD  SI,WORD PTR OUTCFOF
 CALL CKCRPF
 POP  SI
 JE   DKCRYPT
 JMP  NOTCRYPT
IGBS: JMP IGNOREBS
DKCRYPT:
 PUSH SI			;CHECK BOTH CKWD OFFSETS 1.3B
 ADD  SI,WORD PTR INCFOF
 CALL CKKEY
 POP  SI
 JE   GOODKEY
 PUSH SI
 ADD  SI,WORD PTR OUTCFOF
 CALL CKKEY
 POP  SI
 JE   GOODKEY
 JMP  NOTGOODKEY


NOTGOODKEY:
 MOV BYTE PTR BADKEY,1
 JMP NOTCRYPT
GOODKEY:
 MOV [BP].ACTIVE,1 ;TURN ON ENCRYPTION
 PUSH SI
 MOV  DI,SI
 ADD  DI,3	   ;ES:DI -> SYSTEM ID IN BOOT SECT
 MOV  SI,OFFSET ID2M
 MOV  CX,6
 CLD
 REPE CMPSB
 POP  SI
 JNE  NOTCRYPT
 MOV [BP].FLAG2M,1		 ;2M-FORMATTED 1.3d
 MOV AX,ES:[SI+22]		 ;SECTORS FOR EACH FAT 1.3d
 MOV [BP].FATSIZ,AX
NOTCRYPT:
 MOV AL,ES:[SI+18h]
 MOV [BP].MAXSEC,AL ;STORE MAX SECTOR
 MOV AL,ES:[SI+1Ah]
 MOV [BP].MAXHD,AL  ;STORE MAX HEAD
 MOV AX,ES:[SI+0Bh]
 MOV [BP].SECSIZE,AX ;STORE BYTES PER SECTOR
 MOV AX,ES:[SI+27h]
 MOV [BP].SERIAL,AX  ;STORE FIRST WORD OF SERIAL NUMBER
 MOV AX,ES:[SI+29h]
 MOV [BP].SERIAL+2,AX ;STORE SECOND WORD OF SERIAL NUMBER
IGNOREBS:
 RET

CKCRPF PROC NEAR
 CMP WORD PTR ES:[SI],'RC'
 JNE CKCRPFR
 CMP WORD PTR ES:[SI+2],'PY'
CKCRPFR:
 RET
CKCRPF ENDP

CKKEY PROC NEAR
 MOV AX,WORD PTR FKEYCHK
 CMP WORD PTR ES:[SI+4],AX
 JNE CKKEYRET
 MOV AX,WORD PTR FKEYCHK+2
 CMP WORD PTR ES:[SI+6],AX
CKKEYRET:
 RET
CKKEY ENDP

;VARIABLES HERE WILL BE LOADED BY LOGIN
	ALIGN 4
LOADDATA:
TSRVER	DB '130D'			;VERSION FOR CHECK
REALBIOS LABEL DWORD
	DW 0,0
FKEYCHK:DW 2 DUP (0ffffh)
FKEY:	DB 13,10,'Secure Drive Version 1.3d',13,10
	DB 'TSR Installed',13,10,'$'
ALRINS: DB 13,10,'Secure Drive TSR already resident',13,10,'$'
	DB (104-($-FKEY)) DUP (0aah)	  ;FLOPPY ENCRYPTION KEY
FKEYV10 DB 0
	DB "PPP"			  ;PAD FOR ALIGNMENT
HKEYCHK:DW 2 DUP (0ffffh)
HKEY	LABEL BYTE
	DB 104 DUP (0bbh)		  ;HARD DRIVE ENCRYPTION KEY
HKEYV10 DB 0
	DB "PPP"			  ;PAD FOR ALIGNMENT
INCFOF	DW 3				  ;INPUT CHECKWORD OFFSET IN BOOT REC  1.3B
OUTCFOF DW 502				  ;OUTPUT CHECKWORD OFFSET IN BOOT REC 1.3B
FDA:	DISKDATA <0,'A',0,0,1,0FFFFh,18,2,512,0,0,0,0,0>
FDB:	DISKDATA <0,'B',0,0,1,0FFFFh,18,2,512,0,0,0,0,0>
HD1:	DISKDATA <0,'Z',0FFFFh,0,0,0,0,0,512,0,0,0,0,0>
HD2	LABEL BYTE
	REPT MAXDRV-1
	DISKDATA <0,'Z',0FFFFh,0,0,0,0,0,512,0,0,0,0,0>
	ENDM

	ALIGN 4
;VARIABLES HERE STAY PUT
STKSEG: DW ? ;SEGMENT OF USER STACK
STKOFS: DW ? ;OFFSET OF USER STACK

RQTYPE: DB ? ;REQUEST TYPE, AH FROM CALL
DRIVE:	DB ? ;DRIVE FROM CALL
NUMSEC: DB ? ;NUMBER OF SECTORS TO READ (AL FROM CALL)
SECLFT: DB ? ;NUMBER OF SECTORS LEFT TO PROCESS
CALLCX: DW ? ;ORIGINAL CX
KEY:	DW ? ;CURRENTLY SELECTED KEY
BADKEY: DB ? ;KEY BAD FLAG

STCYL:	DW ? ;START CYLINDER FROM CALL
CURCYL: DW ? ;CURRENT CYLINDER BEING PROCESSED

STSEC:	DB ? ;START SECTOR FROM CALL
CURSEC: DB ? ;CURRENT SECTOR BEING PROCESSED

STHD:	DB ? ;START HEAD FROM CALL
CURHD:	DB ? ;CURRENT HEAD BEING PROCESSED

BUFOS:	DW ? ;OFFSET OF TARGET BUFFER
BUFPS:	DW ? ;CURRENT POSITION WITHIN BUFFER
BUFSG:	DW ? ;SEGMENT OF TARGET BUFFER

ENCRYPT: DB ? ;1 FOR ENCRYPT, 0 FOR DECRYPT
IV:  DB 8 DUP (?)   ;IV FOR CFB
ID2M	 DB    "2M-STV"     ; ID FOR 2M-FORMATTED DISKETTE

; Copyright (c) 1993 Colin Plumb.  This code may be freely
; distributed under the terms of the GNU General Public Licence.

; A core operation in IDEA is multiplication modulo 65537.
; The valid inputs, 1 through 66636 inclusive are represented in
; 16-bit registers modulo 65536.  I.e. a value of 0 means 65536,
; or -1.  Thus, we need to test for that specially.  -x, modulo
; 65537, is 65537-x = 1-x.
; For any other number, represent the product as a*65536+b.  Since
; 65536 = -1 (mod 65537), this is the same number as b-a.  Should
; this result be negautive (generate a borrow), -n mod 65537 = 1-n
; mod 65536.  Or in other words, if you add the borrow bit back on,
; you get the right answer.

; This is what the assembly code does.	It forms a zero, and adds
; that on with carry.

; Another useful optimisation takes advantage of the fact that
; a and b are equal only if the answer is congruent to 0 mod 65537.
; Since 65537 is prime, this happens only if one of the inputs is
; congruent to 0 mod 65537.  Since the inputs are all less than 65537,
; this means it must have been zero.

; The code below tests for a zero result of the subtraction, and if
; one arises, it branches out of line to figure out what happened.


; This code implemets the IDEA encryption algorithm.
; It follows in pseudo-C, where the * operator operates
; modulo 65537, as Idea needs.	(If you don't understand,
; learn IDEA better.)

; IDEA is works on 16-bit units.  If you're processing bytes,
; it's defined to be big-endian, so an Intel machine needs to
; swap the bytes around.

; void Idea(u_int16 *in, u_int16 *out, u_int16 *key)
; {
;	 register u+int16 x0, x1, x2, x3, s1, s2, round;
;
;	 x0 = *in++;  x1 = *in++;  x2 = *in++;	x3 = *in;
;
;	 for (round = 0; round < 8; round++) {
;		 x0 *= *key++;
;		 x1 += *key++;
;		 x2 += *key++;
;		 x3 *= *key++;
;
;		 s1  = x1;  s2	= x2;
;		 x2 ^= x0;  x1 ^= x3;
;
;		 x2 *= *key++;
;		 x1 += x2;
;		 x1 *= *key++;
;		 x2 += x1;
;
;		 x0 ^= x1;  x3 ^= x2;
;		 x1 ^= s2;  x2 ^= s1;
;	 }
;	 *out++ = x0 * *key++;
;	 *out++ = x2 + *key++;	      /* Yes, this is x2, not x1 */
;	 *out++ = x1 + *key++;
;	 *out	= x3 * *key;
; }

; ds:si points to key, ax, dx are temps, args in bx, cx, di, bp
; Trashes *all* registers.  direction flag must be clear.
; Leaves es zero.

; Since there is no spare register to hold the loop count, I make
; clever use of the stack, pushing the start of the loop several
; times and using a ret instruction to do the return.

; Annoyingly, lods is fastest on 8086's, but other techniques are
; best on 386's.  Well, that's what the manual says, but real
; life is different.  USELODS wins on a 386SX, at least.
; Leave it set for all platforms.

USELODS        equ	  1

; bp must be x0 for some of the code below to work
x0	  equ	     bp
x1	  equ	     bx
x2	  equ	     cx
x3	  equ	     di
; di must be x3 for some of the code below to work

;; Now, this is rather interesting.  We test for zero arguments
;; after the multiply.	Assuming random inputs, one or both are
;; zero (2^17-1)/2^32, or approximately 1/32786 of the time.
;; Encryption in any feedback mode produces essentially random
;; inputs, so average-case analysis is okay.  While we don't
;; want the out-of-line code to waste time, it is not worth
;; slowing down the in-line case to speed it up.
;;
;; Basically, we start inverting the source x, and if that was 0,
;; we use the inverse of the key instead.

Core1Z:
	neg	   x0
	jnz	   Core1Za
if USELODS
	sub	   x0,[si-2]
else
	sub	   x0,[si]
endif
Core1Za:
	inc	   x0
	jmp	   Core1done
Core2Z:
	neg	   x3
	jnz	   Core2Za
if USELODS
	sub	   x3,[si-2]
else
	sub	   x3,[si+6]
endif
Core2Za:
	inc	   x3
	jmp	   Core2done
Core3Z:
	neg	   x2
	jnz	   Core3Za
if USELODS
	sub	   x2,[si-2]
else
	sub	   x2,[si+8]
endif
Core3Za:
	inc	   x2
	jmp	   Core3done
Core4Z:
	neg	   x1
	jnz	   Core4Za
if USELODS
	sub	   x1,[si-2]
else
	sub	   x1,[si+10]
endif
Core4Za:
	inc	   x1
	jmp	   Core4done

; We need a constant 0 that we can move into a register without affecting
; the carry flag (as the classic xor ax,ax is wont to do), so we use the
; es register for a constant 0 source.	This is okay even in protected
; mode.  (I *told* you this was tricky code!)

; BTW, since you wanted to know, this is 8 + 78*4 + 16 = 336 instructions.

Core	    proc	near
	xor	   ax,ax
	mov	   es,ax
	mov	   ax,OFFSET Finish
	push	    ax
	mov	   ax,OFFSET Coreloop
	push	    ax	      ; Loop 3 times, then return
	push	    ax
	push	    ax

Coreloop:
if USELODS
	lodsw
else
	mov	   ax,[si]		  ; x0 *= *key++
endif
	mul	   x0
	sub	   ax,dx
	jz	  Core1Z
	mov	   x0,es
	adc	   x0,ax
Core1done:

if USELODS
	lodsw
	add	   x1,ax
	lodsw
	add	   x2,ax
else
	add	   x1,[si+2]	    ; x1 += *key++
	add	   x2,[si+4]	    ; x2 += *key++
endif

if USELODS
	lodsw
else
	mov	   ax,[si+6]	    ; x3 += *key++
endif
	mul	   x3
	sub	   ax,dx
	jz	  Core2Z
	mov	   x3,es
	adc	   x3,ax
Core2done:

	push	    x1			; s1 = x1
	push	    x2		      ; s2 = x2

	xor	   x1,x3		; x1 ^= x3
	xor	   x2,x0		; x2 ^= x0

if USELODS
	lodsw
else
	mov	   ax,[si+8]	    ; x2 *= *key++
endif
	mul	   x2
	sub	   ax,dx
	jz	  Core3Z
	mov	   x2,es
	adc	   x2,ax
Core3done:

	add	   x1,x2		; x1 += x2

if USELODS
	lodsw
else
	mov	   ax,[si+10]	     ; x1 *= *key++
endif
	mul	   x1
	sub	   ax,dx
	jz	  Core4Z
	mov	   x1,es
	adc	   x1,ax
Core4done:

	add	   x2,x1		; x2 += x1

	xor	   x0,x1		; x0 ^= x1
	xor	   x3,x2		; x3 ^= x2

	pop	   dx
	xor	   x1,dx		; x1 ^= s2
	pop	   dx
	xor	   x2,dx		; x2 ^= s1

; Second unrolling of loop
if USELODS
	lodsw
else
	mov	ax,[si+12]	    ; x0 *= *key++
endif
	mul	   x0
	sub	   ax,dx
	jz	  Core5Z
	mov	   x0,es
	adc	   x0,ax
Core5done:

if USELODS
	lodsw
	add	   x1,ax
	lodsw
	add	   x2,ax
else
	add	   x1,[si+14]	     ; x1 += *key++
	add	   x2,[si+16]	     ; x2 += *key++
endif

if USELODS
	lodsw
else
	mov	   ax,[si+18]	     ; x3 *= *key++
endif
	mul	   x3
	sub	   ax,dx
	jz	  Core6Z
	mov	   x3,es
	adc	   x3,ax
Core6done:

	push	    x1			 ; s1 = x1
	push	    x2		      ; s2 = x2

	xor	   x1,x3		; x1 ^= x3
	xor	   x2,x0		; x2 ^= x0

if USELODS
	lodsw
else
	mov	   ax,[si+20]	     ; x2 *= *key++
endif
	mul	   x2
	sub	   ax,dx
	jz	  Core7Z
	mov	   x2,es
	adc	   x2,ax
Core7done:

	add	   x1,x2		; x1 += x2

if USELODS
	lodsw
else
	mov	   ax,[si+22]	     ; x1 *= *key++
endif
	mul	   x1
	sub	   ax,dx
	jz	  Core8Z
	mov	   x1,es
	adc	   x1,ax
Core8done:

	add	   x2,x1		; x2 += x1

	xor	   x0,x1		; x0 ^= x1
	xor	   x3,x2		; x3 ^= x2

	pop	   dx
	xor	   x1,dx		; x1 ^= s2
	pop	   dx
	xor	   x2,dx		; x2 ^= s1

ife USELODS
	lea	   si,[si+24]
endif

	ret	   ; Used as a loop instruction!

Core5Z:
	neg	   x0
	jnz	   Core5Za
if USELODS
	sub	   x0,[si-2]
else
	sub	   x0,[si+12]
endif
Core5Za:
	inc	   x0
	jmp	   Core5done
Core6Z:
	neg	   x3
	jnz	   Core6Za
if USELODS
	sub	   x3,[si-2]
else
	sub	   x3,[si+18]
endif
Core6Za:
	inc	   x3
	jmp	   Core6done
Core7Z:
	neg	   x2
	jnz	   Core7Za
if USELODS
	sub	   x2,[si-2]
else
	sub	   x2,[si+20]
endif
Core7Za:
	inc	   x2
	jmp	   Core7done
Core8Z:
	neg	   x1
	jnz	   Core8Za
if USELODS
	sub	   x1,[si-2]
else
	sub	   x1,[si+22]
endif
Core8Za:
	inc	   x1
	jmp	   Core8done
Core9Z:
	neg	   x0
	jnz	   Core9Za
if USELODS
	sub	   x0,[si-2]
else
	sub	   x0,[si]
endif
Core9Za:
	inc	   x0
	jmp	   Core9done
; Special: compute into dx (zero on entry)
Core10Z:
	sub	   dx,x3
	jnz	   Core10Za
if USELODS
	sub	   dx,[si-2]
else
	sub	   dx,[si+6]
endif
Core10Za:
	inc	   dx
;	 jmp	    Core10done
	ret


Finish:
if USELODS
	lodsw
else
	mov	   ax,[si]		  ; x0 *= *key++
endif
	mul	   x0
	sub	   ax,dx
	jz	  Core9Z
	mov	   x0,es
	adc	   x0,ax
Core9done:

	xchg	    x1,x2
if USELODS
	lodsw
	add	   x1,ax
	lodsw
	add	   x2,ax
else
	add	   x1,[si+2]	    ; x1 += *key++
	add	   x2,[si+4]	   ; x2 += *key++
endif

; This is special: compute into dx, not x3
if USELODS
	lodsw
else
	mov	   ax,[si+6]	    ; x3 *= *key++
endif
	mul	   x3
	sub	   ax,dx
	mov	   dx,es
	jz	  Core10Z
	adc	   dx,ax
Core10done:

	ret

	endp

; Okay, the basic plan for the CFB kernel is
; get x0,x1,x2,x3
; get key pointer
; call core
; get buffer pointers
;Loop:
; lodsw
; xor	     ax,x0
; mov	x0,ax
; stosw
; lodsw
; xor	     ax,x1
; mov	     x0,ax
; stosw
; lodsw
; xor	     ax,x2
; mov	     x0,ax
; stosw
; lodsw
; xor	     ax,x3
; mov	     x3,ax
; stosw
; push buffer pointers
; get key pointer
; call	      core
; pop buffer pointers
; loop
; lodsw/xor/etc.
;
;
; This function is designed to go in the middle of a byte-granularity
; CFB engine.  It performs "len" encryptions of the IV, encrypting
; 8*(len-1) bytes from the source to the destination.  The idea is
; that you first xor any odd leading bytes, then call this function,
; then xor up to 8 trailing bytes.

; The main loop in this is 38 instructions, plus the 336 for the core
; makes 374 total.  That's 46.75 instructions per byte.
; (It's the same for IdeaCFBx)

; IV, key, plain, cipher, len
;	 public        _IdeaCFB
;_IdeaCFB proc far	 ; Args are at [sp+4]
_IDEACFB:
	cld
	push	    bp
	push	    si
	push	    di
	push	    ds	      ; 8 more words here, so args are at [sp+12]
; To be precise, IV is at 12, key at 16, plain at 20,
; cipher at 24 and len at 28
	mov	   bp,sp
	lds	   si,[bp+12]	     ; IV
; Load and byte-swap IV
	mov	   ax,[si]
	xchg	    ah,al
	mov	   x1,[si+2]
	mov	   x2,[si+4]
	xchg	    bh,bl
	xchg	    ch,cl
	mov	   dx,[si+6]
	xchg	    dh,dl

	lds	   si,[bp+16]	     ; Key
	mov	   x0,ax
	mov	   x3,dx

	call	    Core
IdeaCFBLoop:
;	 mov	    ax,x0
;	 mov	    bp,sp
;	 dec	    WORD PTR [bp+28]	    ; Decrement count
;	 jz	   IdeaCFBEnd
;	 lds	    si,[bp+20]
;	 les	    di,[bp+24]
;	 mov	    x0,ax
; Alternate code: (which is faster?  Two moves or three segment overrides?)
	mov	   si,sp
	dec	   WORD PTR ss:[si+28]
	jz	  IdeaCFBEnd
	les	   di,ss:[si+24]
	lds	   si,ss:[si+20]

	lodsw
	xchg	    ah,al
	xor	   ax,x0
	mov	   x0,ax
	xchg	    ah,al
	stosw
	lodsw
	xchg	    ah,al
	xor	   ax,x1
	mov	   x1,ax
	xchg	    ah,al
	stosw
	lodsw
	xchg	    ah,al
	xor	   ax,x2
	mov	   x2,ax
	xchg	    ah,al
	stosw
	lodsw
	xchg	    ah,al
	xor	   ax,dx
	mov	   dx,ax
	xchg	    ah,al
	stosw

;	 mov	    ax,x0
;	 mov	    bp,sp
;	 mov	    [bp+20],si	      ; Save source offset
;	 mov	    [bp+24],di	      ; Save destination offset
;	 lds	    si,[bp+16]	      ; Key
;	 mov	    x0,ax		 ; Get x0 in place for another iteration
; Alternate code for the above: (which is faster?  One move or three ss:?)
	mov	   ax,si
	mov	   si,sp
	mov	   ss:[si+20],ax
	mov	   ss:[si+24],di
	lds	   si,ss:[si+16]

	mov	   x3,dx		; Get x3 in place
	mov	   ax,OFFSET IdeaCFBLoop
	push	    ax
	jmp	   Core

IdeaCFBEnd:
;	 lds	    si,[bp+12]
	lds	   di,ss:[si+12]	; Get IV for writing back

	mov	   ax,x0
	xchg	    ah,al
	mov	   [di],ax		  ; Use stosw?
	xchg	    bh,bl
	xchg	    ch,cl
	mov	   [di+2],x1
	mov	   [di+4],x2
	xchg	    dh,dl
	mov	   [di+6],dx

	pop	   ds
	pop	   di
	pop	   si
	pop	   bp

	ret

	endp

; This decoding step is similar, except that instead of
;	 lods
;	 xor	    x0,ax
;	 mov	    ax,x0
;	  stos
; the feedback step is
;	 lods
;	 xchg	     x0,ax
;	 xor	    ax,x0
;	 stos

; IV, key, cipher, plain, len
;	 public        _IdeaCFBx
;_IdeaCFBx proc far	  ; Args are at [sp+4]
_IDEACFBX:
	cld
	push	    bp
	push	    si
	push	    di
	push	    ds	      ; 8 more words here, so args are at [sp+12]
	mov	   bp,sp
	lds	   si,[bp+12]	     ; IV
; Load and byte-swap IV
	mov	   ax,[si]
	xchg	    ah,al
	mov	   x1,[si+2]
	mov	   x2,[si+4]
	xchg	    bh,bl
	xchg	    ch,cl
	mov	   dx,[si+6]
	xchg	    dh,dl

	lds	   si,[bp+16]	     ; Key
	mov	   x0,ax
	mov	   x3,dx

	call	    Core
IdeaCFBxLoop:
;	 mov	    ax,x0
;	 mov	    bp,sp
;	 dec	    WORD PTR [bp+28]	    ; Decrement count
;	 jz	   IdeaCFBxEnd
;	 lds	    si,[bp+20]
;	 les	    di,[bp+24]
;	 mov	    x0,ax
; Alternate code: (which is faster?  Two moves or three segment overrides)
	mov	   si,sp
	dec	   WORD PTR ss:[si+28]
	jz	  IdeaCFBxEnd
	les	   di,ss:[si+24]
	lds	   si,ss:[si+20]

	lodsw
	xchg	    ah,al
	xchg	    x0,ax
	xor	   ax,x0
	xchg	    ah,al
	stosw
	lodsw
	xchg	    ah,al
	xchg	    x1,ax
	xor	   ax,x1
	xchg	    ah,al
	stosw
	lodsw
	xchg	    ah,al
	xchg	    x2,ax
	xor	   ax,x2
	xchg	    ah,al
	stosw
	lodsw
	xchg	    ah,al
	xchg	    dx,ax
	xor	   ax,dx
	xchg	    ah,al
	stosw

;	 mov	    ax,x0
;	 mov	    bp,sp
;	 mov	    [bp+20],si	      ; Save source offset
;	 mov	    [bp+24],di	      ; Save destination offset
;	 lds	    si,[bp+16]	      ; Key
;	 mov	    x0,ax		 ; Get x0 in place for another iteration
; Alternate code for the above: (which is faster?  One move or three ss:?)
	mov	   ax,si
	mov	   si,sp
	mov	   ss:[si+20],ax
	mov	   ss:[si+24],di
	lds	   si,ss:[si+16]

	mov	   x3,dx		; Get x3 in place
	mov	   ax,OFFSET IdeaCFBxLoop
	push	    ax
	jmp	   Core

IdeaCFBxEnd:
;	 lds	    si:[bp+12]
	lds	   di,ss:[si+12]	; Get IV for writing back

	mov	   ax,x0
	xchg	    ah,al
	mov	   [di],ax		  ; Use stosw?
	xchg	    bh,bl
	xchg	    ch,cl
	mov	   [di+2],x1
	mov	   [di+4],x2
	xchg	    dh,dl
	mov	   [di+6],dx


	pop	   ds
	pop	   di
	pop	   si
	pop	   bp

	ret

	endp
END_OF_FILE:
_TEXT ENDS
END START
