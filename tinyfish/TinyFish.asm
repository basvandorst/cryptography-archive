; TINYFISH (BlowFish) encryption program
; This code is based on TINYIDEA, by Fauzan Mirza.
; Author: Anonimous! (This code is therefor PUBLIC DOMAIN)
; Speed Optimization (12 to 50%) and other small modifications
; by Dutra de Lacerda <dulac@ip.pt>

TITLE Blowfish for DOS
PAGE 60
    .model tiny
    .radix 16
    .386
    _code segment use16
    assume cs:_code, ds:_code, es:_code

READSIZE    equ 8000
SBOX        equ 1800
PBOX        equ SBOX-48
DATABUF     equ SBOX+1000



    org 100
START:
    cld
    mov dx, offset Usage
    mov si, 80
    lodsw
    or  al, al                  ; check for parameters
    jz  short Exit
    lodsb                       ; get mode switch
    cmp al, '-'
    jz  short DeMode
    cmp al, '+'
    jz  short EnMode
Exit:
    mov ah, 9
    int 21
Done:
    int 20

DeMode:
    inc byte ptr [mode]
EnMode:
    lodsw
AddZero:
    lodsb
    cmp al, 20
    ja  AddZero
    mov [si-1], bl              ; make ASCIIZ

; Main Program.
; First open the file PI.
; file must be in current or root dir.
BLOWFISH:
    mov ax, 3D00                ; read only...
    mov di, offset CFBbuf
    mov cx, 8
    rep stosb
    mov dx, offset PiNameBuf
    int 21                      ; ...at current directory!
    jnc short PiOpenErr
    mov ax, 3D00                ; read only...
    mov dx, offset RootDir
    int 21                      ; ... at root!
PiOpenErr:
    mov dx, offset PiErrMsg
    jc  exit                    ; If NC -> File Opened OK!
    xchg bx, ax                 ; Handle
    mov ah, 3F                  ; READ (bytes from PI)
    mov cx, DataBuf-PBox        ; =1048
    mov dx, PBox
    int 21
    jc PiOpenErr                ; If NC -> PI readed OK!

GETKEY:
    mov ah, 9                   ; Ask for the Key
    mov dx, offset SolicitKey
    int 21
    mov ah, 0A
    mov dx, offset KeyBuf
    int 21
    mov si, dx
    lodsw                       ; ah contains key lenght
ExpandKey:                      ; Cicling through key
    movzx di, ah
    add di, si
    push si
    cmp ah,0
    jnz short notnul
    dec si			; si reverts to the #BytesRead byte (=0)
NOTNUL:
    mov cx, 48			; 48h = (18 PBoxes x 4 Bytes)
    rep movsb                   ; Overflowing up to 2x, but it's OK!
    pop si

; Init P and S Arrays
; XOR P array with key
INIT:
    mov cl, 12
    mov di, pbox
InitLoop:
    lodsd
      xchg ah, al		; Exchanging Endian Format
      rol eax, 10		; using 386 Code instead of one 486 instruction
      xchg ah, al		; this way beeing able to run in 386 machines!
    xor [di], eax
    scasd
    loop InitLoop

; Encrypt P-Boxes using key
; operating on 0's
BOXCRYPT:
    xor eax, eax
    mov ebp, eax
    mov di, PBox
    mov cx, 209                 ; 8 bytes per encrypt
CryptLoop:                      ; Input is ebp:eax
    push di
    xchg edi, eax               ; Now ebp:edi
    call BfEnc                  ; Use BF to Encrypt
    pop di                      ; BfEnc output is eax:ebp
    stosd
    xchg ebp, eax               ; Now in ebp:eax
    stosd
    loop CryptLoop

    mov dx, 84
    mov ax, 3D02                ; Open Data File
    int 21
    jc short DataErr
    xchg ax, bx

BF1:                            ; Continue until read=0 bytes
    mov cx, ReadSize
    mov dx, DataBuf
    mov ah, 3F
    int 21
    jnc short GoodRead
DataErr:
    mov dx, offset DataErrMsg
    jmp Exit
GoodRead:
    or ax, ax
    jz Done
    push dx                     ; DataBuf
    mov si, dx
    push ax                     ; Bytes Read
    push bx                     ; Handle
    add ax, 7
    shr ax, 3
    xchg cx, ax                 ; # Blocks in Buffer

BLOCK:
    push si                     ; DataBuf
    mov si, offset CFBbuf
    push si                     ; CFBbuf
    lodsd
      xchg ah, al
      rol eax, 10
      xchg ah, al
    xchg ebp, eax
    lodsd
      xchg ah, al
      rol eax, 10
      xchg ah, al
    xchg edi, eax               ; Input  in ebp:edi
    call BfEnc                  ; Output in ebp:eax
    pop di                      ; CFBbuf
    push di
      xchg ah, al
      rol eax, 10
      xchg ah, al
    stosd
    xchg eax, ebp
      xchg ah, al
      rol eax, 10
      xchg ah, al
    stosd
    pop di                      ; CFBbuf
    pop si                      ; DataBuf
    push cx                     ; BlockCount
    mov cx, 2

Mode:                           ; CFB Mode
    clc
    jc short DeCrypt
EnCrypt:
    lodsd
    xor eax, [di]
    stosd
    jmp short Ed1
Decrypt:
    mov eax, [di]
    xor eax, [si]
    movsd
Ed1:
    mov [si-4], eax
    loop mode
    pop cx                      ; Block Count
    loop BLOCK

PtrSet:
    pop bx                      ; Handle
    pop dx                      ; Bytes Read
    push dx
    neg dx
    dec cx                      ; Here CX:DX is dword negative of bytes read
    mov ax, 4201                ; Offsets pointer [cx:dx] from current pos.
    int 21                      ; BackFile ptr to read point
WriteData:                      ; And then write the cipher bytes to file
    pop cx                      ; Bytes Read
    pop dx                      ; DataBuf
    mov ah, 40
    int 21
    jnc BF1
    mov  dx, offset WriteErrMsg
    jmp Exit



; Encrypt a 64-bit value
; Input  in ebp:edi
; Output in ebp:eax
BFENC:
    push cx
    mov cx, 10                 ; 16 Rounds
RRound:
; One Feistel Round
; Input  in ebp:edi
; Output in ebp:edi (?)
ROUND:
    mov bx, 10
    sub bx, cx                  ; here bx= 0->15
    shl bx, 2
    mov si, PBox
    add si, bx
    lodsd
    xor ebp, eax
    mov ebx, ebp
FUNC:                           ; Blowfish S-Box Function
    xor edx, edx                ; Zero Output reg
    push cx
    mov cx, 4
FuncLoop:
    rol ebx, 8                  ; Start with MSByte
    mov al, bl
    mov ah, 4
    sub ah, cl                  ; S-Box #= 0->3
    shl ax, 2                   ; 4 bytes per box
    mov si, SBox
    add si, ax
    lodsd
    cmp cl, 2
    jnz short AddIt
    xor edx, eax
    loop FuncLoop
AddIt:
    add edx, eax
    loop FuncLoop
    pop cx
    xor edi, edx                ; Round Continues
    xchg ebp, edi
RoundDone:
    loop Round
    mov si, PBox+40
    lodsd
    xor ebp, eax
    lodsd
    xor eax, edi
    pop cx
    ret


PiErrMsg:
RootDir:	db 'C:\'
PiNameBuf:	db 'PI', 0               
DataErrMsg:	db 'Read Error$'
WriteErrMsg:	db 'WriteError$'
CFBbuf:		dw 0, 0, 0, 0   ; 4x16bit words makes a 64 bits block
Usage:		db ' DosFish +/- File$'
KeyBuf:		db 49, 0        ; Maximum Size and Read Bytes
				; Overlaping the folowing data
SolicitKey:	db 'Key: $'     

end START
