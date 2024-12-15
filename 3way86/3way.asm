
; 80386 assembly implementation of the
;       3-Way Block Cipher

; Written by Fauzan Mirza <fauzan@dcs.rhbnc.ac.uk>

; Public functions are [encrypt] and [decrypt]
;  The data is in registers  eax:ecx:edx
;  The key  is in registers  ebp:esi:edi
;  Only the data registers are modified (all others preserved)

; Tested under MSDOS but suitable for porting to Linux

; Reference:
;  "A New Approach Towards Block Cipher Design"
;  Joan Daemen, Rene Govaerts, Joos Vandewalle
;  ftp://ftp.esat.kuleuven.ac.be/pub/COSIC/daemen/threeway.ps

        .model  tiny
        .code
        .386

; [main] test driver

        org     100h

main:
        mov     eax,00000003h
        mov     ecx,00000002h
        mov     edx,00000001h

        mov     ebp,00000006h
        mov     esi,00000005h
        mov     edi,00000004h

        pusha

        mov     dx,offset encrypt_msg
        mov     ah,9
        int     21h

        popa

        call    encrypt

        cmp     eax,0d2f05b5eh
        jnz     failed
        cmp     ecx,0d6144138h
        jnz     failed
        cmp     edx,0cab920cdh
        jnz     failed

        pusha

        mov     dx,offset passed_msg
        mov     ah,9
        int     21h

        mov     dx,offset decrypt_msg
        mov     ah,9
        int     21h

        popa

        call    decrypt

        cmp     eax,00000003h
        jnz     failed
        cmp     ecx,00000002h
        jnz     failed
        cmp     edx,00000001h
        jnz     failed

        mov     dx,offset passed_msg
        mov     ah,9
        int     21h

        mov     dx,offset timing_msg
        mov     ah,9
        int     21h

        mov     ebx,(3*1048576)/12
@timing:
        call    encrypt
        dec     ebx
        jnz     @timing

        mov     ax,4c00h
        int     21h

failed:
        mov     dx,offset failed_msg
        mov     ah,9
        int     21h

        mov     ax,4c01h
        int     21h

encrypt_msg:
        db      "encrypt: ",24h

decrypt_msg:
        db      "decrypt: ",24h

passed_msg:
        db      "Passed",0dh,0ah,24h

failed_msg:
        db      07,"Failed",0dh,0ah,24h

timing_msg:
        db      "Encrypting 3 MB (262144 data blocks)",0dh,0ah,24h

; 3-Way Block Cipher

STRT_E  equ     00b0bh
STRT_D  equ     0b1b1h
NMBR    equ     12

a0      equ     eax
a1      equ     ecx
a2      equ     edx
b0      equ     ebp
b1      equ     esi
b2      equ     edi
temp    equ     ebx

; [rndcon_gen]

rndcon_gen:
        push    eax edi
        xchg    ebx,eax
        mov     edi,offset rcon
        mov     bx,NMBR
@fill:
        stosd
        shl     eax,1
        test    eax,10000h
        jz      @loop
        xor     eax,11011h
@loop:
        dec     bx
        jnz     @fill
        pop     edi eax
        ret

; [decrypt]

decrypt:
        push    ebx
        push    a0 a1 a2
        mov     a0,b0
        mov     a1,b1
        mov     a2,b2
        push    b2 b1 b0
        call    theta
        pop     b2 b1 b0
        call    mu
        pop     a2 a1 a0
        mov     ebx,STRT_D
        call    rndcon_gen
        push    b0 b1 b2
        call    mu
        pop     b2 b1 b0
        call    cipher
        push    b0 b1 b2
        call    mu
        pop     b2 b1 b0
        pop     ebx
        ret

; [encrypt]

encrypt:
        push    ebx
        mov     ebx,STRT_E
        call    rndcon_gen
        call    cipher
        pop     ebx
        ret

; [cipher]

cipher:
        mov     bx,NMBR
@round:
        push    bx

        shl     bx,2
        neg     bx
        mov     temp,[bx+NMBR*4+offset rcon]
        push    temp

        shl     temp,16
        xor     a0,temp
        xor     a0,b0

        xor     a1,b1

        pop     temp
        xor     a2,temp
        xor     a2,b2

        pop     bx
        push    b0 b1 b2
        push    bx

        call    theta

        pop     bx
        dec     bx
        jz      @done

; [rho] the round function

; [pi_1]
        ror     a0,10
        rol     a2,1

; [gamma] the non-linear step

        mov     b0,a2
        not     b0
        or      b0,a1

        mov     b1,a0
        not     b1
        or      b1,a2

        mov     b2,a1
        not     b2
        or      b2,a0

        xor     a0,b0
        xor     a1,b1
        xor     a2,b2

        pop     b2 b1 b0

; [pi_2]
        rol     a0,1
        ror     a2,10

; rho ends

        jmp     @round
@done:
        pop     b2 b1 b0
        ret

; [theta] the linear step

theta:
        mov     b0,a0
        rol     b0,16
        mov     temp,a1
        rol     temp,16
        xor     b0,temp
        mov     temp,a2
        rol     temp,16
        xor     b0,temp

        mov     b1,b0
        mov     b2,b0

        ; b0
        ror     temp,8
        xor     b0,temp

        xor     b0,a0

        mov     temp,a0
        shl     temp,8
        xor     b0,temp
        shl     temp,16
        xor     b0,temp

        mov     temp,a1
        shr     temp,24
        xor     b0,temp

        mov     temp,a2
        shr     temp,8
        xor     b0,temp

        ; b1
        mov     temp,a0
        rol     temp,8
        xor     b1,temp

        xor     b1,a1

        mov     temp,a1
        shl     temp,8
        xor     b1,temp
        shl     temp,16
        xor     b1,temp

        mov     temp,a2
        shr     temp,24
        xor     b1,temp

        mov     temp,a0
        shr     temp,8
        xor     b1,temp

        ; b2
        rol     a1,8
        xor     b2,a1

        ror     a1,8
        shr     a1,8
        xor     b2,a1

        xor     b2,a2

        shl     a2,8
        xor     b2,a2
        shl     a2,16
        xor     b2,a2

        shr     a0,24
        xor     b2,a0

        mov     a0,b0
        mov     a1,b1
        mov     a2,b2

        ret

; [mu] inverts the order of the bits of a

mu:
        xor     b0,b0
        xor     b1,b1
        xor     b2,b2
        mov     bx,32
@inv:
        shl     b0,1
        shl     b1,1
        shl     b2,1

        bt      a0,0
        jnc     @a1
        inc     b2
@a1:
        bt      a1,0
        jnc     @a2
        inc     b1
@a2:
        bt      a2,0
        jnc     @a0
        inc     b0
@a0:
        shr     a0,1
        shr     a1,1
        shr     a2,1

        dec     bx
        jnz     @inv
        mov     a0,b0
        mov     a1,b1
        mov     a2,b2
        ret

        even

rcon:
        dd      NMBR dup (?)

        end     main

