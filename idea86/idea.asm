; Last updated: Sat Jul 20 1996

; PUBLIC DOMAIN 8086 assembly implementation of the
; International Data Encryption Algorithm (IDEA)
;       By Fauzan Mirza

; Speed optimizations:
;       Bill Couture <bcouture@cris.com>        (IDEA)
;       Mark Andreas <voyageur@sky.net>         (Expandkey,IDEA,MulMod)

        .model  tiny
        .code

; Public symbols for external linking (near)

        public  Expandkey
        public  IDEA

; Expand user key to IDEA encryption key
; Entry:  si -> userkey, di -> buffer for IDEA key (can equal si)
; Exit:   di -> IDEA key
; All data must be/will be in little-endian format

Expandkey:
        mov     bx,8
        mov     cx,bx
        repnz   movsw                   ; Copy the key into the first 8 words
CompleteKey:
        mov     ax,bx                   ; Determine which two of the previous
        and     al,7                    ;  eight words are needed for this
        cmp     al,6                    ;  key expansion round

        mov     ax,word ptr [di-14]
        mov     dx,word ptr [di-12]
        jb      Update
        mov     dx,word ptr [di-28]
        jz      Update
        mov     ax,word ptr [di-30]
Update:
        mov     cl,9
        shl     ax,cl
        mov     cl,7
        shr     dx,cl                   ; Calculate the rotated value
        or      ax,dx
        stosw                           ;   and save it
        inc     bx
        cmp     bl,52
        jnz     CompleteKey             ; Continue until 52 words updated
        ret

; IDEA subroutine
; Entry:  si -> key, di -> input data
; Exit:   di -> output data, all other registers trashed
; All data must be/will be in little-endian format

; Refer to the PGP IDEA source for a better explanation
; of the algorithm and the optimisations

x0      equ     bx
x1      equ     cx
x2      equ     bp
x3      equ     di

IDEA:
        mov     byte ptr [Rounds],8     ; Eight rounds
        push    di
        mov     dx,word ptr [di]
        mov     x1,word ptr [di+2]
        mov     x2,word ptr [di+4]
        mov     x3,word ptr [di+6]      ; note that DI is over-written last
Round:
        call    MulMod
        xchg    x0,ax                   ; x0 *= *key++

        lodsw
        add     x1,ax                   ; x1 += *key++
        lodsw
        add     x2,ax                   ; x2 += *key++
        mov     dx,x3
        call    MulMod
        xchg    x3,ax                   ; x3 *= *key++

        push    x1                      ; s0 = x1
        push    x2                      ; s1 = x2
        xor     x2,x0                   ; x2 ^= x0
        xor     x1,x3                   ; x1 ^= x3

        mov     dx,x2
        call    MulMod
        add     x1,ax                   ; x2 *= *key++
        xchg    x2,ax                   ; x1 += x2
        mov     dx,x1
        call    MulMod
        add     x2,ax                   ; x1 *= *key++
        xchg    x1,ax                   ; x2 += x1

        xor     x0,x1                   ; x0 ^= x1
        xor     x3,x2                   ; x3 ^= x2
        pop     dx
        pop     ax
        xor     x1,dx                   ; x1 ^= s1
        xor     x2,ax                   ; x2 ^= s0

        mov     dx,x0
        dec     byte ptr [Rounds]       ; Continue until no more rounds
        jnz     Round

        call    MulMod
        xchg    x0,ax                   ; x0 *= *key++
        lodsw
        add     x2,ax                   ; x2 += *key++
        lodsw
        add     x1,ax                   ; x1 += *key++
        mov     dx,x3
        call    MulMod                  ; x3 *= *key++

        pop     di
        push    di

        xchg    x0,ax
        stosw
        xchg    x2,ax                   ; unswap x1, x2
        stosw
        xchg    x1,ax
        stosw
        xchg    x0,ax
        stosw

        pop     di
        ret

Rounds:
        db      0

; Multiplication modulo 65537
; ax = [si] * dx

MulMod:
        push    dx
        lodsw
        mul     dx
        sub     ax,dx
        pop     dx
        jnz     NotZero
        inc     ax
        sub     ax,word ptr [si-2]
        sub     ax,dx
        ret
NotZero:
        adc     ax,0
        ret

        end

