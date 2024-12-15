comment |
 asm code for the Data Encryption Standard  (DES) algorithm.
THIS CODE REQUIRES A 486 TO RUN. Uses BSWAP, a 486-specific instruction.
 Assembles with Turbo Assembler (I use v.2.5).
 By Steve Allen (CIS 73277,620)
|

model small
locals

d32seg segment para public 'DATA'  ;ensure 32-bit alignment of 32-bit data
temp    dq ?
block db 8 dup (?)
block1 dq ?
r_sched dw 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
loopcount  dw ?

ifdef TRIPLE_DES
en_keyptr dw en_keytbl
de_keyptr dw de_keytbl
else
keyptr dw keytbl
endif

d32seg ends

DGROUP group d32seg     ;tell tasm d32seg is same seg as _DATA seg


;                  ROUTINES CONTAINED HEREIN:

public c ascii_adjust_key       ;rotate parity bits to bit 0 in password
public c test_adjust_key        ;bogus, but DDJ and DESDOS like it
public c schedule               ;set up 16 rounds of key schedule
public c encrypt_block          ;void encrypt_block(void);
public c decrypt_block          ;void decrypt_block(void);
public c mov64                  ;char *mov64(char *dest, char *source);
public c xor64                  ;char *xor64(char *dest, char *source);
public c killkey                ;void killkey(void);

dataseg

public c block        ;the input/output for encrypt/decrypt
extrn PC1tbl:byte     ;Permuted Choice 1 for key schedule
extrn PC2tbl:byte     ;Permuted Choice 2 for key schedule
extrn Pmask:byte      ;setting bits for permute function
extrn sp_table:dword  ;the combined S and P boxes for f() function
extrn p1:dword        ;1st permute table
extrn p2:dword        ;inverse_permute table
ifdef TRIPLE_DES
extrn c en_keytbl:byte  ;encryption keytable
extrn c de_keytbl:byte  ;decryption keytable
public c en_keyptr      ;assignable pointer to key schedule used by encrypt
public c de_keyptr      ;assignable pointer to key schedule used by decrypt
else
extrn c keytbl:byte
public c keyptr         ;assignable pointer to key schedule
endif

c32seg segment para public 'CODE'   ;ensure best alignment for cache

c32group group _TEXT, c32seg        ;set up group with our seg & C codeseg
assume cs:c32group                  ;tell TASM it's OK

.486

;----------------------------------------------------------------------
;       void ascii_adjust_key(char password[8]);
; This routine takes an 8-byte ascii password and rotates each byte
; left one position, moving the nominal parity bit (bit 7) 
; to bit 0 (DES's bit 8). This has the effect of causing the 7 LSBs of
; each password byte to be used in key scheduling, and the parity bits
; to be ignored, as per DES.

proc ascii_adjust_key near
   arg output:dataptr   ;output also is input

        push bp
        mov bp,sp
        push di
        push ds
        pop es

        mov di,[output]
        mov si,di
        mov cx,8
@@1:
        lodsb
        call odd_parity ;pro forma parity adjust, but result is ignored...
        rol al,1        ;shift parity bit to bit 0 (DES calls it bit 8)
        stosb
        loop @@1

        pop di
        mov sp,bp
        pop bp
        ret
ascii_adjust_key endp

;----------------------------------------------------------------------
;               test_adjust_key(char password[8]);
;this code exists only for testing. It produces a key compatible with
;Phil Karn's DES, and is used when validating changes to my DES program.
;In the key produced here parity bits are significant,
;which is contrary to the DES specification.
align 16
proc test_adjust_key near
   arg output:dataptr

        push bp
        mov bp,sp
        push di
        push si
        push ds
        pop es

        mov di,[output]
        mov si,di
        mov cx,8

@@1:    lodsb
        call odd_parity
        stosb
        loop @@1

        pop si
        pop di
        pop bp
        ret
test_adjust_key endp

odd_parity:
        push cx
        push bx
        mov cx,7
        xor bx,bx
        or al,80h

@@1:    bt ax,bx
        jnc @@2         ;bit is 0
        xor al,80h      ;else flip parity bit
@@2:    inc bx
        loop @@1

        pop bx
        pop cx
        ret


;----------------------------------------------------------------------
;                void schedule(char *password long keytbl[]);
;        Expand a password into 16 rounds of Key Schedule.
;        Output goes into extern keytbl[16][8]
;         (This is not a time-critical function, and I simply
;          rewrote the c source into assembler, to make 
;          DES programming a 2-step process-- schedule the key,
;          encrypt/decrypt the block.)
align 16
proc schedule
 arg input:dataptr, tbl:dataptr
        push bp
        mov bp,sp
        push di

        push 64
        push offset PC1tbl
        mov bx,[input]
        push bx
        push offset temp
        call permute            ;select 56 bits
        add sp,8

        mov cx,16
        mov [loopcount],cx
        xor di,di

@@1:
        mov bx,di
        shl bx,1
        add bx,offset r_sched
        mov ax,[bx]
        push ax                 ;save for 2nd rotate
        push ax                 ;arg for rotate func.
        push offset temp
        call rotate
        add sp,4
        push offset temp+4
        call rotate
        add sp,4

        push 48
        push offset PC2tbl      ;perm table
        push offset temp        ;input
        push offset block1      ;output
        call permute
        add sp,8

        mov ax,[tbl]
        push ax
        push di                 ;table number
        push offset block1      ;input
        call set_table
        add sp,6

        inc di
        dec [loopcount]
        jnz @@1

        mov bx,[input]          ;erase the password space
        mov cx,4
        xor ax,ax
@@2:    mov [bx],ax
        inc bx
        inc bx
        loop @@2

        pop di
        pop bp
        ret
schedule endp

;----------------------------------------------------------------------
;                void set_table(char*input,int n,long*keytbl);
;       Take a 48-bit input, and make 8 bytes of 6 bits each.
;       Output goes into tbl[].
align 16
proc set_table near
 arg input:dataptr,num:word, tbl:dataptr

        push bp
        mov bp,sp
        push di

        mov di, [tbl]
        mov ax,[num]
        mov bl,8
        mul bl
        add di,ax       ;now di points to proper offset in keytbl
        mov bx,[input]
        mov dx,word ptr[bx+4]
        bswap edx       ;edx highbits now has highbits of key sched.
        mov ebx,[bx]
        bswap ebx       ;ebx gets 48 bits of key sched.
        mov cx,8        ;loop count

@@1:    shld eax,ebx,6  ;get next 6 bits
        shld ebx,edx,6
        shl edx,6
        shl ax,2        ;pre-shift for sizeof(long) in f() function
        and al,11111100b
        mov [di],al
        inc di
        loop @@1

        pop di
        mov sp,bp
        pop bp
        ret
set_table endp
;----------------------------------------------------------------------
;                              Rotate 28 bits (for KS function)
;                            void rotate(long *input,int count);
rotate proc near
 arg input:dataptr,count:word

push bp
mov bp,sp

mov cx,[count]
mov bx, [input]
mov eax,[bx]
bswap eax               ;get bits in good order
xor ebx,ebx
shld ebx, eax,cl        ;shift high bits to bl
shl eax,cl
shl ebx,4               ;move from bit 32 to bit 28
add eax,ebx

mov bx,[input]
bswap eax               ;reverse bytes...
mov [bx],eax            ;...so they store correctly

pop bp
ret
rotate endp

;----------------------------------------------------------------------
proc killkey near            ;zero out the key schedule when we're done
ifdef TRIPLE_DES
       mov di,offset en_keytbl
else
       mov di,offset keytbl
endif
       push ds
       pop es
       mov cx,8*16
       xor al,al
       rep stosb
ifdef TRIPLE_DES
        mov di,offset de_keytbl
        mov cx,8*16
        rep stosb
endif
       ret
killkey endp

;----------------------------------------------------------------------
;                  void permute(long *op,long *ip, char *tbl,int n);
align 16
permute proc near
 arg op:dataptr, ip:dataptr, tbl:dataptr, n:word

        push bp
        mov bp,sp
        push si
        push di

        mov bx,[op]
        mov si,tbl                     ;permutation table to si
        mov di,offset Pmask            ;setting-bits in di
        mov dword ptr[bx],0
        mov dword ptr[bx+4],0                  ;output=0
        mov bx,[ip]
        mov eax,[bx]
        mov edx,[bx+4]                  ;input to edx:eax
        mov cx,[n]                      ;count to cx

@@1:    mov ebx,[si]                    ;get high dword perm bit
        test eax,ebx                    ;is this bit set?
        jnz @@is_hit
        mov ebx,[si+4]                  ;get low dword perm bit
        test edx,ebx                    ;any action?
        jz @@no_hit

@@is_hit:
        push si
        mov si,[op]                     ;point to output
        mov ebx,[di]                    ;get setting-bit
        or [si],ebx                     ;set the bit
        mov ebx,[di+4]                  ;do for both halves
        or [si+4],ebx                   ;set bit
        pop si

@@no_hit:
        add si,8                        ;next permutation entry
        add di,8                        ;next setting-bits entry
        loop @@1


        pop di
        pop si
        mov sp,bp
        pop bp
        ret
permute endp

;----------------------------------------------------------------------
;                       Local PERMUTE for encrypt_block & decrypt_block
;input conditions:
;  si:perm. table ptr
;  edx:eax input high and low
;  ebp:edi output high and low
;  destroys all registers except cx!
align 16
l_permute:

; do high dword
        mov bx,dx               ;nybble 1
        shl bx,3
        and bx,1111000b
        shr edx,1
        mov ebp,[si+bx]         ;output high
        mov edi,[si+4+bx]       ;output low
        add si,8*16

@@2:
        mov bx,dx               ;nybble 2
        and bx,1111000b
        shr edx,4
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx,dx               ;nybble 3
        and bx,1111000b
        shr edx,4
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx,dx               ;nybble 4
        and bx,1111000b
        shr edx,4
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx,dx               ;nybble 5
        and bx,1111000b
        shr edx,4
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx,dx               ;nybble 6
        and bx,1111000b
        shr edx,4
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx,dx               ;nybble 7
        and bx,1111000b
        shr edx,4
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx,dx               ;nybble 8
        and bx,1111000b
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

; do low dword
        mov bx,ax               ;nybble 1
        shl bx,3                ;index * 8
        and bx,1111000b
        shr eax,1
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

@@3:
        mov bx, ax              ;nybble 2
        and bx,1111000b
        shr eax,4               ;next nybble
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx, ax              ;nybble 3
        and bx,1111000b
        shr eax,4               ;next nybble
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx, ax              ;nybble 4
        and bx,1111000b
        shr eax,4               ;next nybble
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx, ax              ;nybble 5
        and bx,1111000b
        shr eax,4               ;next nybble
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx, ax              ;nybble 6
        and bx,1111000b
        shr eax,4               ;next nybble
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx, ax              ;nybble 7
        and bx,1111000b
        shr eax,4               ;next nybble
        or ebp,[si+bx]
        or edi,[si+4+bx]
        add si,8*16

        mov bx, ax              ;nybble 8
        and bx,1111000b
        or ebp,[si+bx]
        or edi,[si+4+bx]

        ret


;----------------------------------------------------------------------
;               void encrypt_block(void);
;            encrypts data in (unsigned char block[8] ),
;            returning it in the same block[].
align 16
proc encrypt_block near
        push si
        push di
        push bp

        mov si, offset p1
        mov edx,dword ptr[block]
        mov eax,dword ptr[block+4]
        call l_permute                  ;returns with output in ebp:edi

        mov eax,edi                     ;f() input is ebp:eax
ifdef TRIPLE_DES
        mov si,[en_keyptr]
else
        mov si,[keyptr]
endif
        add si,7
        mov [loopcount],16              ;yes, this is the full 16 round DES
        jmp short @@1
align 16
@@1:
        mov edx,eax                     ;prepare to operate on right half
        mov ecx,eax                     ;and save it: it becomes left half
        bswap edx                       ;edx has f() input
        sub eax,eax                     ;output gets zero
        mov di,offset sp_table+7*4*64   ;point to combined s & p boxes
        rol edx,3                       ;prepare for E() function

                 ;THE F() FUNCTION (encrypt)
        mov bx,dx               ;nybble 1
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ; (E function)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 2
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ; (E function)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 3
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ; (E function)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 4
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ; (E function)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 5
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ; (E function)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 6
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ; (E function)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 7
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ; (E function)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 8
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble

        xor eax,ebp             ;left half^= f(right half,keytbl[n]);
        add si,16               ;point to next keytable
        mov ebp,ecx             ;old right half becomes new left half
        dec [loopcount]
        jnz @@1

        mov si, offset p2
        mov edx,eax
        mov eax,ebp
        call l_permute
        mov dword ptr[block],ebp
        mov dword ptr[block+4],edi

        pop bp
        pop di
        pop si
        ret
encrypt_block endp
;----------------------------------------------------------------------
align 16
proc decrypt_block near
        push si
        push di
        push bp

        mov si, offset p1
        mov edx,dword ptr[block]        ;get input for l_permute
        mov eax,dword ptr[block+4]
        call l_permute

        mov [loopcount],16              ;yes, this is the full 16 round DES
ifdef TRIPLE_DES
        mov si,[de_keyptr]
else
        mov si,[keyptr]
endif
        add si,15*8+7
        mov eax, edi                    ;ip.L=op.R

align 4
@@1:
        mov edx,eax                     ;f() input is ip.L
        mov ecx,edx                     ;op.R=ip.L
        bswap edx
        sub eax,eax                     ;output gets zero
        mov di,offset sp_table + 7*4*64 ;point to s&p boxes
        rol edx,3                       ;set up bits 32,1,2,3,4,5 shl 2

            ;THE F() FUNCTION (decrypt)
        mov bx,dx               ;nybble 1
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ;(see E Bit-Selection Table)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 2
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ;(see E Bit-Selection Table)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 3
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ;(see E Bit-Selection Table)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 4
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ;(see E Bit-Selection Table)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 5
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ;(see E Bit-Selection Table)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 6
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ;(see E Bit-Selection Table)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 7
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble
        ror edx,4               ;(see E Bit-Selection Table)
        sub di,4*64             ;sizeof(long)*sizeof(table)

        mov bx,dx               ;nybble 8
        xor bl,[si]             ;XOR with key bits
        and bx,11111100b
        dec si
        or eax, [bx+di]         ;set bits for this nybble

        xor eax,ebp                  ;ip.R ^ f(ip.L,keytbl[n]);
        mov ebp,ecx                  ;ip.R=op.R;
        dec [loopcount]
        jnz @@1

        mov si,offset p2                ;set up for inverse permutation
        mov edx,eax                     ;get input
        mov eax,ebp
        call l_permute
        mov dword ptr[block],ebp
        mov dword ptr[block+4],edi

        pop bp
        pop di
        pop si
        ret
decrypt_block endp

align 16
proc mov64 near
 arg dest:dataptr,source:dataptr

        push bp
        mov bp,sp
        push si
        mov si,[source]
        mov bx,[dest]

        mov eax,[si]
        mov [bx],eax
        mov eax,[si+4]
        mov [bx+4],eax
        
        mov ax,bx       ;return with pointer to dest
        pop si
        pop bp
        ret
mov64 endp

align 16
proc xor64 near
; arg dest:dataptr,source:dataptr

        push bp
        mov bp,sp
        push si
        mov si,[source]
        mov bx,[dest]

        mov eax,[si]
        xor [bx],eax
        mov eax,[si+4]
        xor [bx+4],eax

        mov ax,bx       ;return with pointer to dest
        pop si
        pop bp
        ret
xor64 endp

c32seg ends
end
