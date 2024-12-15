; assemble with "TASM /mx MFLOATA"

PUBLIC ADDM, SUBM, NEGM, MULTM, DIVM, MULTI, DIVI, FABSM, SQRTM, TRUNCM, \\
       GTM, EQM, DTOMF, LDTOMF, MFTOD_, MFTOLD_, INTTOMF, LDEXPM, SINBASIS, \\
       SINHBASIS, ATANBASIS, ARTANHBASIS, MODULOM, EQUM, MFTOINT, PIM, \\
       LN2M, LN10M, ONEM, MFLOATTEST80386

  EXTRN MFLOATERROR                             ;global procedure
  EXTRN MFLOATSTACKOVERFLOW                     ;global procedure
  EXTRN MANTISSAWORDS:WORD                      ;global variable
  EXTRN USE386INSTRUCTIONS:BYTE                 ;global variable

.MODEL LARGE

%out
%out
%out Attention!
%out
%out This is only a dummy file for understanding!
%out
%out Only headers of the procedures are available!
%out
%out

.CODE

INCLUDE mfloatc.asm                             ;constants

mfloatwordsP1 = mfloatwords + 1;
lmfloatwords = (mfloatwords + 1) / 2
lmfloatwordsP1 = lmfloatwords + 1

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;-------------- determine processor type ---------------------------------

MFLOATTEST80386 PROC PASCAL FAR

       xor ax, ax                               ;default return value = 0
       pushf
       pop bx                                   ;bx <- flags
       and bh, 0FH                              ;clear bit 12-15 of bx
       push bx
       popf                                     ;flags <- bx
       pushf
       pop cx                                   ;cx <- flags
       and ch, 0F0H                             ;clear bit 0-11 of cx
       cmp ch, 0F0H                             ;bit 12-15 set?
       jz t386
       or bh, 0F0H                              ;set bit 12-15 of bx
       push bx
       popf                                     ;flags <- bx
       pushf
       pop cx                                   ;cx <- flags
       and ch, 0F0H                             ;bit 12-15 cleared?
       jz t386
       inc ax                                   ;its a 80386 or compatible
t386:  mov bx, seg USE386INSTRUCTIONS
       mov es, bx
       mov es:[USE386INSTRUCTIONS], al          ;USE386INSTRUCTIONS <- al
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;-------------- mfloat subtraction using 8086 instructions ---------------

SUBM PROC PASCAL FAR
ARG    a:DWORD, b:DWORD
LOCAL  blocal:WORD:mfloatwords, sign:WORD, wordshift:WORD, bitshift:WORD, \\
       carryword:WORD, mantwords:WORD

; remark: no stack check necessary, since made in procedure "ADDM"

       mov dl, 80h                              ;subtraction-flag
       jmp short add1
SUBM   ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;-------------- mfloat addition using 8086 instructions ------------------

ADDM   PROC PASCAL FAR
ARG    a:DWORD, b:DWORD
LOCAL  blocal:WORD:mfloatwords, sign:WORD, wordshift:WORD, bitshift:WORD, \\
       carryword:WORD, mantwords:WORD

       mov dl, 0                                ;addition - flag
add1:  jb add2                                  ;carry from "sub sp,arglen"
       cmp sp, 250H                             ;minimum stack remains
       ja add3
add2:  mov sp, bp                               ;release local variables
       call far ptr MFLOATSTACKOVERFLOW
add3:  push si                                  ;save registers
       push di
       push ds
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si                                   ;end of adjustment
       cld                                      ;for code optimizer
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat sign change using 8086 instructions only -------------------

NEGM   PROC PASCAL FAR
ARG    a:DWORD

       les bx, [a]
       xor byte ptr es:[bx + 3], 80H            ;change sign bit
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat multiplication using 8086 instructions only ---------------

MULTM8086 PROC PASCAL
ARG    a:DWORD, b:DWORD
       ; a further argument is the register ax: number of mantissawords
LOCAL  amult:WORD:mfloatwords, ahelp:WORD:mfloatwords,  \\
       blocal:WORD:mfloatwords, sign:WORD, bpointer:WORD,  \\
       aend:WORD, ahelpend:WORD, amultend:WORD, wordnumber:WORD,  \\
       multnumber:WORD, aextra:WORD, mantwords:WORD

; remark: no stack check necessary, since made in procedure "MULTM"
;         no far call necessary!

       push si                                  ;save registers
       push di
       push ds
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat multiplication using 80386 or 8086 instructions -----------

MULTM PROC PASCAL FAR
ARG    a:DWORD, b:DWORD
LOCAL  amult:DWORD:lmfloatwords, ahelp:DWORD:lmfloatwords,  \\
       blocal:DWORD:lmfloatwords, sign:WORD, bpointer:WORD,  \\
       aend:WORD, ahelpend:WORD, amultend:WORD, wordnumber:WORD,  \\
       multnumber:WORD, aextra:DWORD, lmantwords:WORD

       jb mult13                                ;carry from "sub sp,arglen"
       cmp sp, 250H                             ;minimum stack remains
       ja mult14
mult13:mov sp, bp                               ;release local variables
       call far ptr MFLOATSTACKOVERFLOW
mult14:mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov bx, seg USE386INSTRUCTIONS
       mov es, bx
       cmp byte ptr es:[USE386INSTRUCTIONS], 0  ;80386 instructions allowed?
       jz mult15
       test ax, 1
       jz mult16                                ;jump at even matissawords
mult15:mov sp, bp                               ;remove local variables
       push word ptr [a+2]                      ;segment of a
       push word ptr [a]                        ;offset of a
       push word ptr [b+2]                      ;segment of b
       push word ptr [b]                        ;offset of b
       call MULTM8086
       RET
.386
mult16:push si                                  ;save registers
       push di
       push ds
       shr ax, 1
       mov [lmantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat division using 8086 instructions only ---------------------

DIVM8086 PROC PASCAL
ARG    a:DWORD, b:DWORD
       ; a further argument is the register ax: number of mantissawords
LOCAL  blocal:WORD:mfloatwords, ahelp:WORD:mfloatwords, \\
       rest:WORD:mfloatwords, b1word:WORD, ahelpend:WORD, bend:WORD, \\
       wordnumber:WORD, restend:WORD, sign:WORD, apointer:WORD, \\
       mantwords:WORD

; remark: no stack check necessary, since made in procedure "DIVM"
;         no far call necessary!

       push si                                  ;save registers
       push di
       push ds
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat division using 80386 or 8086 instructions -----------------

DIVM   PROC PASCAL FAR
ARG    a:DWORD, b:DWORD
LOCAL  blocal:DWORD:lmfloatwords, ahelp:DWORD:lmfloatwords, \\
       rest:DWORD:lmfloatwords, b1word:DWORD, ahelpend:WORD, bend:WORD, \\
       wordnumber:WORD, restend:WORD, sign:WORD, apointer:WORD, \\
       lmantwords:WORD

       jb div17                                 ;carry from "sub sp,arglen"
       cmp sp, 250H                             ;minimum stack remains
       ja div18
div17: mov sp, bp                               ;release local variables
       call far ptr MFLOATSTACKOVERFLOW
div18: mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov bx, seg USE386INSTRUCTIONS
       mov es, bx
       cmp byte ptr es:[USE386INSTRUCTIONS], 0  ;80386 instructions allowed?
       jz div19
       test ax, 1
       jz div20                                 ;jump at even matissawords
div19: mov sp, bp                               ;remove local variables
       push word ptr [a+2]                      ;segment of a
       push word ptr [a]                        ;offset of a
       push word ptr [b+2]                      ;segment of b
       push word ptr [b]                        ;offset of b
       call DIVM8086
       RET
.386
div20: push si                                  ;save registers
       push di
       push ds
       shr ax, 1
       mov [lmantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------- mfloat integer multiplication using 8086 instructions only ------

MULTI  PROC PASCAL FAR
ARG    a:DWORD, b:WORD
LOCAL  sign:WORD, aextra:WORD, mantwords:WORD

       push si                                  ;save registers
       push di
       push ds
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat integer division using 8086 instructions only -------------

DIVI   PROC PASCAL FAR
ARG    a:DWORD, b:WORD
LOCAL  sign:WORD, aextra:WORD, mantwords:WORD

       push si                                  ;save registers
       push di
       push ds
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat absolute value using 8086 instructions only ---------------

FABSM   PROC PASCAL FAR
ARG    a:DWORD

       les bx, [a]
       and es:byte ptr [bx + 3], 7FH            ;delete sign bit
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat square root using 8086 instructions only ------------------

SQRTM8086 PROC PASCAL
ARG    a:DWORD
       ; a further argument is the register ax: number of mantissawords
LOCAL  rest:WORD:mfloatwords, atimesb:WORD:mfloatwords, apoint:WORD, \\
       firstword:WORD, atimesbend:WORD, aend:WORD, wordnumber:WORD, \\
       restend:WORD, logvar:WORD, mantwords:WORD

; remark: no stack check necessary, since made in procedure "SQRTM"
;         no far call necessary!

       push si                                  ;save registers
       push di
       push ds
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat square root using 80386 or 8086 instructions --------------

SQRTM  PROC PASCAL FAR
ARG    a:DWORD
LOCAL  rest:DWORD:lmfloatwords, atimesb:DWORD:lmfloatwords, apoint:WORD, \\
       firstword:DWORD, atimesbend:WORD, aend:WORD, wordnumber:WORD, \\
       restend:WORD, logvar:WORD, lmantwords:WORD

       jb sqrt34                                ;carry from "sub sp,arglen"
       cmp sp, 250H                             ;minimum stack remains
       ja sqrt35
sqrt34:mov sp, bp                               ;release local variables
       call far ptr MFLOATSTACKOVERFLOW
sqrt35:mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov bx, seg USE386INSTRUCTIONS
       mov es, bx
       cmp byte ptr es:[USE386INSTRUCTIONS], 0  ;80386 instructions allowed?
       jz sqrt36
       test ax, 1
       jz sqrt37                                ;jump at even matissawords
sqrt36:mov sp, bp                               ;remove local variables
       push word ptr [a+2]                      ;segment of a
       push word ptr [a]                        ;offset of a
       call SQRTM8086
       RET
.386
sqrt37:push si                                  ;save registers
       push di
       push ds
       shr ax, 1
       mov [lmantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;---- Conversion double to mfloat using 8086 instructions only -----------

DTOMF PROC PASCAL FAR
ARG    a:DWORD, b:QWORD
LOCAL  mantwords:WORD

       push si                                  ;save registers
       push di
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       cld
       les di, [a]
       mov ax, word ptr [b + 6]                 ;load 4th word
       mov bx, ax                               ;store ax
       and ah, 7FH                              ;delete sign bit
       mov cl, 4
       shr ax, cl                               ;4 shift right commands
       test ax, ax
       jnz doub1                                ;jump, if exponent <> 0
       stosw                                    ;store number zero
       jmp short doub2
doub1: add ax, 7C02H                            ;offset of the exponent
       stosw                                    ;store exponent
       mov ah, bl
       mov al, byte ptr [b + 5]                 ;load mantissa of the real
       mov si, word ptr [b + 3]                 ;number in registers
       mov dx, word ptr [b + 1]
       mov bl, byte ptr [b]
       shl bl, 1                                ;1st shift left
       rcl dx, 1
       rcl si, 1
       rcl ax, 1
       shl bl, 1                                ;2nd shift left
       rcl dx, 1
       rcl si, 1
       rcl ax, 1
       shl bl, 1                                ;3rd shift left
       rcl dx, 1
       rcl si, 1
       rcl ax, 1
       shl ah, 1                                ;set sign bit
       shl bh, 1
       rcr ah, 1
       stosw                                    ;store 1st mantissaword
       mov cx, [mantwords]
       dec cx
       jle doub2
       mov ax, si
       stosw                                    ;store 2nd mantissaword
       dec cx
       jle doub2
       mov ax, dx
       stosw                                    ;store 3rd mantissaword
       dec cx
       jle doub2
       mov ah, bl
       xor al, al
       stosw                                    ;store 4th mantissaword
       dec cx
       jle doub2
       xor ax, ax
       rep stosw                                ;zeros to rest of mantissa
doub2: pop di                                   ;restore registers
       pop si
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------- Conversion extended to mfloat using 8086 instructions only ------

LDTOMF PROC PASCAL FAR
ARG    a:DWORD, b:TBYTE
LOCAL  mantwords:WORD

       push di                                  ;save register
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       les di, [a]
       mov ax, word ptr [b + 8]                 ;load exponent
       mov bh, ah                               ;store sign
       and ah, 7FH                              ;delete sign
       or ax, ax                                ;set flags
       jnz ex1
       mov word ptr es:[di], 0                  ;a <- 0
       jmp short ex2
ex1:   add ax, 4002H                            ;offset of exponent
       cld
       stosw                                    ;store exponent
       mov ax, word ptr [b + 6]                 ;load 1st mantissaword
       shl ah, 1
       shl bh, 1
       rcr ah, 1                                ;set sign bit
       stosw                                    ;store 1st mantissaword
       mov cx, [mantwords]
       dec cx
       jle ex2
       mov ax, word ptr [b + 4]                 ;load 2nd mantissaword
       stosw                                    ;store 2nd mantissaword
       dec cx
       jle ex2
       mov ax, word ptr [b + 2]                 ;load 3rd mantissaword
       stosw                                    ;store 3rd mantissaword
       dec cx
       jle ex2
       mov ax, word ptr [b]                     ;load 4th mantissaword
       stosw                                    ;store 4th mantissaword
       dec cx
       jle ex2
       xor ax, ax                               ;delete ax
       rep stosw                                ;zeros to rest of mantissa
ex2:   pop di                                   ;restore register
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;----- Conversion mfloat to double using 8086 instructions only ----------

MFTOD_ PROC PASCAL FAR
ARG    a:DWORD, b:DWORD
LOCAL  mantwords:WORD

       push si                                  ;save registers
       push di
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       les di, [b]
       mov ax, word ptr es:[di]
       sub ax, 7C02H                            ;calculate exponent
       ja todo1
       les di, [a]
       xor ax, ax
       cld
       mov cx, 4
       rep stosw                                ;store zero
       jmp short todo5
todo1: cmp ax, 7FFH
       jb todo2
       mov ax, 28
       push ax                                  ;conversion error Nr.: 28
       call far ptr MFLOATERROR
       jmp short todo5
todo2: mov bx, word ptr es:[di + 2]             ;1st mantissaword of b
       xor cx, cx
       xor si, si
       mov dx, [mantwords]
       dec dx
       jle todo3
       mov cx, word ptr es:[di + 4]             ;load 2nd mantissaword
       dec dx
       jle todo3
       mov si, word ptr es:[di + 6]             ;load 3rd mantissaword
       dec dx
       jle todo3
       mov dh, byte ptr es:[di + 9]             ;half of 4th mantissaword
       jmp short todo4
todo3: xor dh, dh
todo4: shl bx, 1
       rcr dl, 1                                ;store sing bit
       shr ax, 1                                ;shift right exponent
       rcr bx, 1                                ;LSBE in bx
       shr ax, 1                                ;1st bit shift right
       rcr bx, 1
       rcr cx, 1
       rcr si, 1
       rcr dh, 1
       shr ax, 1                                ;2nd bit shift right
       rcr bx, 1
       rcr cx, 1
       rcr si, 1
       rcr dh, 1
       shl dl, 1                                ;sign bit -> carry
       rcr al, 1                                ;carry -> al
       rcr bx, 1                                ;3rd bit shift right
       rcr cx, 1
       rcr si, 1
       rcr dh, 1
       les di, [a]
       mov byte ptr es:[di], dh                 ;store result
       mov word ptr es:[di + 1], si
       mov word ptr es:[di + 3], cx
       mov word ptr es:[di + 5], bx
       mov byte ptr es:[di + 7], al
todo5: pop di                                   ;restore registers
       pop si
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ Conversion mfloat to extended using 8086 instructions only -------

MFTOLD_ PROC PASCAL FAR
ARG    a:DWORD, b:DWORD
LOCAL  mantwords:WORD

       push si                                  ;save registers
       push di
       push ds
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       les di, [a]
       lds si, [b]
       cld
       lodsw                                    ;load exponent
       sub ax, 4002H
       ja  toex1
       mov cx, 5
       xor ax, ax
       rep stosw                                ;a <- 0
       jmp short toex6
toex1: cmp ax, 7FFFH
       jb toex2
       mov ax, 28
       pop ds                                   ;restore ds
       push ax                                  ;conversion error Nr.: 28
       call far ptr MFLOATERROR
       push ds                                  ;store ds
       jmp short toex6
toex2: mov bx, word ptr ds:[si]                 ;load 1st mantissaword
       shl ah, 1
       shl bh, 1
       rcr ah, 1                                ;set sign bit
       stc
       rcr bh, 1                                ;1st mantissabit <- 1
       mov es:[di + 8], ax                      ;store exponent
       mov es:[di + 6], bx                      ;store 1st mantissaword
       xor ax, ax
       mov cx, [mantwords]
       dec cx
       jle toex3
       add si, 2                                ;si points 2nd mantissaword
       lodsw                                    ;load 2nd mantissaword
toex3: mov es:[di + 4], ax                      ;store 2nd mantissaword
       xor ax, ax
       dec cx
       jle toex4
       lodsw                                    ;load 3rd mantissaword
toex4: mov es:[di + 2], ax                      ;store 3rd mantissaword
       xor ax, ax
       dec cx
       jle toex5
       lodsw                                    ;load 4th mantissaword
toex5: mov es:[di], ax                          ;store 4th mantissaword
toex6: pop ds                                   ;restore registers
       pop di
       pop si
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------- Conversion integer to mfloat using 8086 instructions only ------

INTTOMF PROC PASCAL FAR
ARG    a:DWORD, b:WORD
LOCAL  mantwords:WORD

       push di
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov cx, es:[MANTISSAWORDS]               ;copy number of mantissawords
       les di, [a]
       mov bx, [b]
       mov ax, bx                               ;store b
       cmp bx, 0
       jg  into2
       jl  into1
       mov es:word ptr [di], 0                  ;a <- 0
       jmp short into4
into1: neg bx                                   ;absolute value of bx
into2: mov dx, 8011H                            ;exponent
into3: dec dx
       shl bx, 1                                ;shift left bx, 1st mantw.
       jnc into3                                ;no carry
       shl ax, 1                                ;sign bit -> carry
       rcr bx, 1                                ;shift 1 bit back, sign bit
       mov es:[di], dx                          ;store exponent
       mov es:[di+2], bx                        ;store 1st. mantw.
       add di,4                                 ;points to second mant.
       dec cx
       jle into4
       xor ax, ax
       cld
       rep stosw                                ;store zeros
into4: pop di
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat integer part using 8086 instructions only ------------------

TRUNCM   PROC PASCAL FAR
ARG    a:DWORD
LOCAL  mantwords:WORD

       push di                                  ;save register
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       ;
       ;
       ;
       pop di                                   ;restore register
       cld                                      ;for code optimizer
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;-- special conversion mfloat to integer using 8086 instructions only ----

MFTOINT PROC PASCAL FAR
ARG    a:DWORD
LOCAL  mantwords:WORD

       push di                                  ;save register
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       les di, [a]
       mov dx, es:[di]
       sub dx, 8000H                            ;dx = exponent 2's complement
       ja intd1
       jmp intd11                               ;return zero
intd1: mov cl, 4
       mov bx, dx
       shr bx, cl                               ;bx = number of int. words
       mov cx, [mantwords]
       mov ax, cx
       sub cx, bx
       jae intd2
       jmp intd11                               ;return zero, |a| too large
intd2: shl ax, 1
       add di, ax                               ;di at last word
       xor ax, ax
       dec cx
       jl  intd10                               ;there's no fractional part
       jz  intd3
       std
       repz scasw                               ;compare last words
       jnz intd12                               ;return zero
intd3: mov cl, dl
       mov bx, 0FFFFH
       and cl, 0FH
       shr bx, cl
       and bx, word ptr es:[di]                 ;compare last bits
       jnz intd12                               ;return zero, ax = 0
       mov cx, 10H                              ;|a| has no fractional
       sub cx, dx
       jg  intd9                                ;|a| has less than 16 bits
       mov cl, dl
       and cl, 0FH
       jz intd4
       mov bx, word ptr es:[di]
       shl bx, cl
       rcl ax, 1                                ;LSB=1 => odd number
       jmp short intd5
intd4: mov ax, word ptr es:[di - 2]             ;LSB=1 => odd number
intd5: mov di, word ptr [a]
       mov bx, es:[di + 2]
       cmp bh, 80H                              ;test sign bit
       jb  intd7
       shr ax, 1
       jb  intd6                                ;test LSB, even-odd
       mov ax, 8000H                            ;minimum even integer
       jmp short intd12
intd6: mov ax, 8001H                            ;minimum odd integer
       jmp short intd12
intd7: shr ax, 1
       jb  intd8                                ;test LSB, even-odd
       mov ax, 7FFEH                            ;maximum even integer
       jmp short intd12
intd8: mov ax, 7FFFH                            ;maximum odd integer
       jmp short intd12
intd9: mov di, word ptr [a]
       mov ax, word ptr es:[di + 2]
       mov bx, ax
       or  ah, 80H
       shr ax, cl                               ;convert to integer format
       cmp bh, 80H
       jb  intd12
       neg ax                                   ;set sign
       jmp short intd12
intd10:test dl, 0FH                             ;8000H <= |a| <= FFFFH
       jnz intd11                               ;IF number has fractional
       mov ax, es:[di]                          ;LSB=1 => odd number
       jmp intd5
intd11:xor ax, ax
intd12:pop di
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;---------- mfloat comparision using 8086 instructions only --------------

GTM PROC PASCAL FAR
ARG    a:DWORD, b:DWORD
LOCAL  mantwords:WORD

       push si                                  ;save registers
       push di
       push ds
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;---------- mfloat comparision using 8086 instructions only --------------

EQM PROC PASCAL FAR
ARG    a:DWORD, b:DWORD

       push si                                  ;save registers
       push di
       push ds
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov cx, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;--- multiplication by a power of two using 8086 instructions only ------

LDEXPM PROC PASCAL FAR
ARG    a:DWORD, b:WORD

       les bx, [a]
       mov ax, [b]
       cmp word ptr es:[bx], 0                  ;test if a = 0
       jz ldexp2
       add word ptr es:[bx], ax
       rcr cx, 1                                ;shift carry into register
       xor cx, ax
       shl cx, 1
       jnc ldexp2
       shl ax, 1
       jc  ldexp1
       mov ax, 2
       push ax
       CALL far ptr MFLOATERROR                 ;overflow error
       jmp short ldexp2
ldexp1:mov word ptr es:[bx], 0                  ;underflow
ldexp2:cld                                      ;for code optimizer
       mov dx, word ptr [a + 2]                 ;store pointer of result
       mov ax, word ptr [a]                     ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat sine-base function using 8086 instructions only -----------

SINBASIS8086 PROC PASCAL
ARG    a:DWORD
       ; a further argument is the register ax: number of mantissawords
LOCAL  afix:WORD:mfloatwordsP1, sqra:WORD:mfloatwordsP1, \\
       mult:WORD:mfloatwordsP1, help:WORD:mfloatwordsP1, sign:BYTE, \\
       aend:WORD, afixend:WORD, sqraend:WORD, multend:WORD, helpend:WORD, \\
       bpointer:WORD, wordnumber:WORD, multnumber:WORD, aextra:WORD, \\
       n:BYTE, mantwords:WORD

; remark: no stack check necessary, since made in procedure "SINBASIS"
;         no far call necessary!

       push si                                  ;save registers
       push di
       push ds
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat sine-base function using 80386 or 8086 instructions -------

SINBASIS PROC PASCAL FAR
ARG    a:DWORD
LOCAL  afix:DWORD:lmfloatwordsP1, sqra:DWORD:lmfloatwordsP1, \\
       mult:DWORD:lmfloatwordsP1, help:DWORD:lmfloatwordsP1, sign:BYTE, \\
       aend:WORD, afixend:WORD, sqraend:WORD, multend:WORD, helpend:WORD, \\
       bpointer:WORD, wordnumber:WORD, multnumber:WORD, aextra:DWORD, \\
       n:WORD, lmantwords:WORD

       jb sin37                                 ;carry from "sub sp,arglen"
       cmp sp, 250H                             ;minimum stack remains
       ja sin38
sin37: mov sp, bp                               ;release local variables
       call far ptr MFLOATSTACKOVERFLOW
sin38: mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov bx, seg USE386INSTRUCTIONS
       mov es, bx
       cmp byte ptr es:[USE386INSTRUCTIONS], 0  ;80386 instructions allowed?
       jz sin39
       test ax, 1
       jz sin40                                 ;jump at even matissawords
sin39: mov sp, bp                               ;remove local variables
       push word ptr [a+2]                      ;segment of a
       push word ptr [a]                        ;offset of a
       call SINBASIS8086
       RET
.386
sin40: push si                                  ;save registers
       push di
       push ds
       shr ax, 1
       mov [lmantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;---- mfloat sine-hyperblic-base function using 8086 instructions only ---

SINHBASIS8086 PROC PASCAL
ARG    a:DWORD
       ; a further argument is the register ax: number of mantissawords
LOCAL  afix:WORD:mfloatwordsP1, sqra:WORD:mfloatwordsP1, \\
       mult:WORD:mfloatwordsP1, help:WORD:mfloatwordsP1, sign:BYTE, \\
       aend:WORD, afixend:WORD, sqraend:WORD, multend:WORD, helpend:WORD, \\
       bpointer:WORD, wordnumber:WORD, multnumber:WORD, aextra:WORD, \\
       n:BYTE, overfl:BYTE, mantwords:WORD

; remark: no stack check necessary, since made in procedure "SINHBASIS"
;         no far call necessary!

       push si                                  ;save registers
       push di
       push ds
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;- mfloat sine-hyperblic-base function using 80386 or 8086 instructions --

SINHBASIS PROC PASCAL FAR
ARG    a:DWORD
LOCAL  afix:DWORD:lmfloatwordsP1, sqra:DWORD:lmfloatwordsP1, \\
       mult:DWORD:lmfloatwordsP1, help:DWORD:lmfloatwordsP1, sign:BYTE, \\
       aend:WORD, afixend:WORD, sqraend:WORD, multend:WORD, helpend:WORD, \\
       bpointer:WORD, wordnumber:WORD, multnumber:WORD, aextra:DWORD, \\
       n:WORD, overfl:BYTE, lmantwords:WORD

       jb sinh34                                ;carry from "sub sp,arglen"
       cmp sp, 250H                             ;minimum stack remains
       ja sinh35
sinh34:mov sp, bp                               ;release local variables
       call far ptr MFLOATSTACKOVERFLOW
sinh35:mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov bx, seg USE386INSTRUCTIONS
       mov es, bx
       cmp byte ptr es:[USE386INSTRUCTIONS], 0  ;80386 instructions allowed?
       jz sinh36
       test ax, 1
       jz sinh37                                ;jump at even matissawords
sinh36:mov sp, bp                               ;remove local variables
       push word ptr [a+2]                      ;segment of a
       push word ptr [a]                        ;offset of a
       call SINHBASIS8086
       RET
.386
sinh37:push si                                  ;save registers
       push di
       push ds
       shr ax, 1
       mov [lmantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat atan-base function using 8086 instructions only -----------

ATANBASIS8086 PROC PASCAL
ARG    a:DWORD
       ;a further argument is the register ax: number of mantissawords
LOCAL  afix:WORD:mfloatwordsP1, sqra:WORD:mfloatwordsP1, \\
       mult:WORD:mfloatwordsP1, help:WORD:mfloatwordsP1, sign:BYTE, \\
       aend:WORD, afixend:WORD, sqraend:WORD, multend:WORD, helpend:WORD, \\
       bpointer:WORD, wordnumber:WORD, multnumber:WORD, aextra:WORD, \\
       n:WORD, mantwords:WORD

; remark: no stack check necessary, since made in procedure "ATANBASIS"
;         no far call necessary!

       push si                                  ;save registers
       push di
       push ds
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------ mfloat atan-base function using 80386 or 8086 instructions -------

ATANBASIS PROC PASCAL FAR
ARG    a:DWORD
LOCAL  afix:DWORD:lmfloatwordsP1, sqra:DWORD:lmfloatwordsP1, \\
       mult:DWORD:lmfloatwordsP1, help:DWORD:lmfloatwordsP1, sign:BYTE, \\
       aend:WORD, afixend:WORD, sqraend:WORD, multend:WORD, helpend:WORD, \\
       bpointer:WORD, wordnumber:WORD, multnumber:WORD, aextra:DWORD, \\
       n:DWORD, lmantwords:WORD

       jb atg37                                 ;carry from "sub sp,arglen"
       cmp sp, 250H                             ;minimum stack remains
       ja atg38
atg37: mov sp, bp                               ;release local variables
       call far ptr MFLOATSTACKOVERFLOW
atg38: mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov bx, seg USE386INSTRUCTIONS
       mov es, bx
       cmp byte ptr es:[USE386INSTRUCTIONS], 0  ;80386 instructions allowed?
       jz atg39
       test ax, 1
       jz atg40                                 ;jump at even matissawords
atg39: mov sp, bp                               ;remove local variables
       push word ptr [a+2]                      ;segment of a
       push word ptr [a]                        ;offset of a
       call ATANBASIS8086
       RET
.386
atg40: push si                                  ;save registers
       push di
       push ds
       shr ax, 1
       mov [lmantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;-------- mfloat artanh-base function using 8086 instructions only -------

ARTANHBASIS8086 PROC PASCAL
ARG    a:DWORD
       ;a further argument is the register ax: number of mantissawords
LOCAL  afix:WORD:mfloatwordsP1, sqra:WORD:mfloatwordsP1, \\
       mult:WORD:mfloatwordsP1, help:WORD:mfloatwordsP1, sign:BYTE, \\
       aend:WORD, afixend:WORD, sqraend:WORD, multend:WORD, helpend:WORD, \\
       bpointer:WORD, wordnumber:WORD, multnumber:WORD, aextra:WORD, \\
       n:WORD, overfl:BYTE, mantwords:WORD

; remark: no stack check necessary, since made in procedure "ARTANHBASIS"
;         no far call necessary!

       push si                                  ;save registers
       push di
       push ds
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;----- mfloat artanh-base function using 80386 or 8086 instructions ------

ARTANHBASIS PROC PASCAL FAR
ARG    a:DWORD
LOCAL  afix:DWORD:lmfloatwordsP1, sqra:DWORD:lmfloatwordsP1, \\
       mult:DWORD:lmfloatwordsP1, help:DWORD:lmfloatwordsP1, sign:BYTE, \\
       aend:WORD, afixend:WORD, sqraend:WORD, multend:WORD, helpend:WORD, \\
       bpointer:WORD, wordnumber:WORD, multnumber:WORD, aextra:DWORD, \\
       n:DWORD, overfl:BYTE, lmantwords:WORD

       jb atgh35                                ;carry from "sub sp,arglen"
       cmp sp, 250H                             ;minimum stack remains
       ja atgh36
atgh35:mov sp, bp                               ;release local variables
       call far ptr MFLOATSTACKOVERFLOW
atgh36:mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov bx, seg USE386INSTRUCTIONS
       mov es, bx
       cmp byte ptr es:[USE386INSTRUCTIONS], 0  ;80386 instructions allowed?
       jz atgh37
       test ax, 1
       jz atgh38                                ;jump at even matissawords
atgh37:mov sp, bp                               ;remove local variables
       push word ptr [a+2]                      ;segment of a
       push word ptr [a]                        ;offset of a
       call ARTANHBASIS8086
       RET
.386
atgh38:push si                                  ;save registers
       push di
       push ds
       shr ax, 1
       mov [lmantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;--------- mfloat modulo function using 8086 instructions only -----------

MODULOM PROC PASCAL FAR
ARG    a:DWORD, modul:DWORD, round:DWORD, overflow:DWORD
LOCAL  modulloc:WORD:mfloatwords, mult:WORD:mfloatwords, \\
       rest:WORD:mfloatwords, sign:BYTE, modulend:WORD, multend:WORD, \\
       restend:WORD, wordnumber:WORD, m1word:WORD, mantwords:WORD

       jb rd1                                   ;carry from "sub sp,arglen"
       cmp sp, 250H                             ;minimum stack remains
       ja rd2
rd1:   mov sp, bp                               ;release local variables
       call far ptr MFLOATSTACKOVERFLOW
rd2:   push si                                  ;save registers
       push di
       push ds
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov ax, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       mov [mantwords], ax
       ;
       ;
       ;
       pop ds                                   ;restore registers
       pop di
       pop si
       cld                                      ;for code optimizer
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

.8086
;------------ mfloat copy function using 8086 instructions only ----------

EQUM PROC PASCAL FAR
ARG    dest:DWORD, src:DWORD

       push si                                  ;save registers
       push di
       push ds
       mov ax, seg MANTISSAWORDS
       mov es, ax
       mov cx, word ptr es:[MANTISSAWORDS]      ;copy number of mantissawords
       inc cx
       lds si, [src]
       les di, [dest]
       cld
       rep movsw                                ;copy
       pop ds                                   ;restore registers
       pop di
       pop si
       mov dx, word ptr [dest + 2]              ;store pointer of result
       mov ax, word ptr [dest]                  ;for further use
       RET
       ENDP

;-------------------------------------------------------------------------
;*************************************************************************
;-------------------------------------------------------------------------

END