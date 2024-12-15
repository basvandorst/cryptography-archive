; Last updated: Sat Jul 20 1996

; Example IDEA Encryption Program
; Copyright (C) Fauzan Mirza 1995

; Link with IDEA/CFB object files

        .model  tiny
        .code

        extrn   Expandkey:near
        extrn   Encrypt:near
        extrn   Decrypt:near
        extrn   CFBBuffer:near

        org     100h

BufLen  equ     16384                   ; Size of buffer (16KB)

Start:
        mov     si,80h
        lodsb
        or      al,al                   ; Check if parameters given
        jnz     Begin

PrintUsage:
        mov     ah,9
        mov     dx,offset Usage
        int     21h                     ; Print usage message

        mov     ax,4c01h
        int     21h                     ; Exit

SkipSpaces:
        lodsb
        cmp     al,0dh
        jz      PrintUsage
        cmp     al,20h
        jz      SkipSpaces
        dec     si
        ret

AddZero:
        lodsb
        cmp     al,0dh
        jz      AddZ2
        cmp     al,20h
        jnz     AddZero
AddZ2:
        mov     byte ptr [si-1],0
        ret

Begin:
        call    SkipSpaces
        lodsb                           ; Get mode switch
        cmp     al,'-'
        jz      DeMode
        cmp     al,'+'
        jz      EnMode
        jmp     PrintUsage
DeMode:
        mov     byte ptr [Mode],255
EnMode:
        call    SkipSpaces
        mov     word ptr [Infile],si    ; Get input filename
        call    AddZero

        call    SkipSpaces
        mov     word ptr [Outfile],si   ; Get output filename
        call    AddZero
   
        mov     ah,9
        mov     dx,offset Enterkey
        int     21h                     ; Print message requesting key

        mov     ah,0ah
        mov     dx,offset Password
        int     21h                     ; Request key from user

        mov     si,offset Userkey
        mov     di,offset Key
        call    Expandkey               ; Expand user key to IDEA key

        mov     si,offset IV
        mov     di,offset CFBBuffer
        mov     cx,4
        repnz   movsw                   ; Set initialization vector

        mov     ax,3d00h
        mov     dx,word ptr [Infile]
        int     21h                     ; Open input file
        jc      Error
        mov     word ptr [Infile],ax

        mov     ah,3ch
        xor     cx,cx
        mov     dx,word ptr [Outfile]
        int     21h                     ; Create output file
        jc      Error
        mov     word ptr [Outfile],ax

Again:
        mov     ah,3fh
        mov     bx,word ptr [Infile]
        mov     cx,BufLen
        mov     dx,offset Buffer
        int     21h                     ; Read file data into buffer
        jc      Error

        or      ax,ax                   ; Check if done
        jz      Done

        push    ax
        call    DoCrypt                 ; Process buffer
        pop     cx

        mov     ah,40h
        mov     bx,word ptr [Outfile]
        mov     dx,offset Buffer
        int     21h                     ; Write buffer to output file
        jc      Error

        cmp     cx,BufLen
        jz      Again                   ; Continue until no more input

Done:

        mov     ah,3eh
        mov     bx,word ptr [Outfile]
        int     21h                     ; Close output file

        mov     ah,3eh
        mov     bx,word ptr [Infile]
        int     21h                     ; Close input file

        mov     ax,4c00h
        int     21h                     ; Exit

Error:
        mov     ah,09
        mov     dx,offset Message
        int     21h                     ; Display error message

        mov     ax,4c01h
        int     21h                     ; Exit

DoCrypt:
        mov     si,offset Key
        mov     di,offset Buffer
        add     ax,7
        mov     cl,3
        shr     ax,cl
        mov     cx,ax                   ; Calculate number of blocks
Block:
        push    cx
        push    si di
        cmp     byte ptr [Mode],0       ; Check mode
        jnz     DoDecrypt
        call    Encrypt                 ; Encrypt block using IDEA/CFB
        jmp     DoNextBlock
DoDecrypt:
        call    Decrypt                 ; Decrypt block using IDEA/CFB
DoNextBlock:
        pop     di si
        add     di,8                    ; Update pointer to next block
        pop     cx
        loop    Block                   ; Loop until all blocks processed
        ret

Usage:
        db      "IDEA Encryption Program",13,10
        db      "(C) Fauzan Mirza 1995",13,10,13,10
        db      "Usage: CRYPT [+|-] <InFile> <OutFile>",10,13,36

Enterkey:
        db      "Enter Key: ",36

Message:
        db      "File Access Error",10,13,36

Mode:
        db      0

Infile:
        dw      0

Outfile:
        dw      0

Password:
        db      17,0

Userkey:
        db      17 dup (?)

IV:
        dw      0102h,0304h,0506h,0708h

Key:
        dw      52 dup (?)

Buffer:
        db      BufLen dup (?)

        end     Start

