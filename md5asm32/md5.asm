; ==========================================================================
; MD5.ASM 1.01 by Robert Rothenburg Walking-Owl (26 September 1994)
; ==========================================================================
; Note: source is targeted for MASM 5.0 and 386 machines.

; The sources include only the low-level MD5-Transformation rounds.

; This module does not include routines for message finalization (that is,
; padding the message and appending the message-size) since that can be
; done more easily in high-level languages.

; See the MD5-A Internet Draft for more information on this procedure.

; The MD5 Message Digest Algorithm is by RSA Data Security, Inc.

; --------------------------------------------------------------------------
; This source code is (c) Copyright 1994, Robert Rothenburg Walking-Owl.  
;                                         <rrothenb@ic.sunysb.edu>

; 1.00  23-Sep-94       First version of this software released locally
; 1.01  26-Sep-94       Added a few more comments to source code (RRWO) 

; These sources are "original", based entirely on the MD5-A Internet Draft
; (10 July 1991) descriptions of the algorithm.

; License: this code, as well as any derivative code, may be used only for
; personal and/or non-commercial purposes.  Modified versions of this code
; may not be distributed without the author's permission.

; Any software (including compiled executables) containing this code may
; only be distributed as free software--that is, no fees of any kind are
; to be asked of users of that software.

; This source code must be distributed as-is, with no modifications.

; The author makes no guarantees or warrantees of any kind about this code.
; Use at your own risk.  The author will not be held accountable for any
; damages resulting from the use or mis-use of this software, etc.
; --------------------------------------------------------------------------

                        .386

CODE                    segment byte public use16        
                        assume  CS:CODE
                        assume  DS:nothing, ES:nothing, SS:nothing

; ==========================================================================
; Defines (miscellaneous)
; ==========================================================================

A                       equ     <dword ptr ES:[DI]>
B                       equ     <dword ptr ES:[DI+4]>
C                       equ     <dword ptr ES:[DI+8]>
D                       equ     <dword ptr ES:[DI+12]>

X                       equ     <dword ptr DS:[SI]>

stor                    macro   dest,src
                        mov     EAX,            src
                        mov     CS: dest,       EAX
                        endm

addd                    macro   i,j
                        mov     EAX,            j
                        add     i,              EAX
                        endm

; ==========================================================================
; Defines: Round 1.
; --------------------------------------------------------------------------
; FF(a,b,c,d,X[k],s,t) denotes a = b + ((a + F(b,c,d) + X[k] + t) <<< s)
; ==========================================================================
FF                      macro   a,b,c,d,X,s,t
                        mov     EBX,            b
                        mov     ECX,            c
                        mov     EDX,            d
                        call    F
                        add     EAX,            a
                        add     EAX,            X
                        add     EAX,            t
                        rol     EAX,            s
                        add     EAX,            b
                        mov     a,              EAX
                        endm

S11                     equ     7
S12                     equ     12
S13                     equ     17
S14                     equ     22

; ==========================================================================
; Defines: Round 2.
; --------------------------------------------------------------------------
; GG(a,b,c,d,X[k],s,t) denotes a = b + ((a + G(b,c,d) + X[k] + t) <<< s)
; ==========================================================================
GG                      macro   a,b,c,d,X,s,t
                        mov     EBX,            b
                        mov     ECX,            c
                        mov     EDX,            d
                        call    G
                        add     EAX,            a
                        add     EAX,            X
                        add     EAX,            t
                        rol     EAX,            s
                        add     EAX,            b
                        mov     a,              EAX
                        endm

S21                     equ     5
S22                     equ     9
S23                     equ     14
S24                     equ     20

; ==========================================================================
; Defines: Round 3.
; --------------------------------------------------------------------------
; HH(a,b,c,d,X[k],s,t) denotes a = b + ((a + H(b,c,d) + X[k] + t) <<< s)
; ==========================================================================
HH                      macro   a,b,c,d,X,s,t
                        mov     EBX,            b
                        mov     ECX,            c
                        mov     EDX,            d
                        call    H
                        add     EAX,            a
                        add     EAX,            X
                        add     EAX,            t
                        rol     EAX,            s
                        add     EAX,            b
                        mov     a,              EAX
                        endm

S31                     equ     4
S32                     equ     11
S33                     equ     16
S34                     equ     23

; ==========================================================================
; Defines: Round 4.
; --------------------------------------------------------------------------
; II(a,b,c,d,X[k],s,t) denotes a = b + ((a + I(b,c,d) + X[k] + t) <<< s)
; ==========================================================================
II                      macro   a,b,c,d,X,s,t
                        mov     EBX,            b
                        mov     ECX,            c
                        mov     EDX,            d
                        call    I
                        add     EAX,            a
                        add     EAX,            X
                        add     EAX,            t
                        rol     EAX,            s
                        add     EAX,            b
                        mov     a,              EAX
                        endm

S41                     equ     6
S42                     equ     10
S43                     equ     15
S44                     equ     21

; ==========================================================================
; Auxiliary function F(X,Y,Z)=XY v not(X) Z
; --------------------------------------------------------------------------
; X = EBX, Y = ECX, Z = EDX, result = EAX
; ==========================================================================
F                       proc    near
                        mov     EAX,            EBX
                        and     EAX,            ECX
                        push    EBX
                        not     EBX
                        and     EBX,            EDX
                        or      EAX,            EBX
                        pop     EBX
                        ret
F                       endp
; ==========================================================================
; Auxiliary function G(X,Y,Z)=XZ v Y not(Z)
; ==========================================================================
G                       proc    near
                        mov     EAX,            EBX
                        and     EAX,            EDX
                        push    EDX
                        not     EDX
                        and     EDX,            ECX
                        or      EAX,            EDX
                        pop     EDX
                        ret
G                       endp
; ==========================================================================
; Auxiliary function H(X,Y,Z)=X xor Y xor Z
; ==========================================================================
H                       proc    near
                        mov     EAX,            EBX
                        xor     EAX,            ECX
                        xor     EAX,            EDX
                        ret
H                       endp
; ==========================================================================
; Auxiliary function I(X,Y,Z)=Y xor (X v not(Z))
; ==========================================================================
I                       proc    near
                        mov     EAX,            EDX
                        not     EAX
                        or      EAX,            EBX
                        xor     EAX,            ECX
                        ret
I                       endp

; ==========================================================================
; Internal dataspaces used for calculations
; ==========================================================================
AA                      label   dword
                        dd      (?)
BB                      label   dword
                        dd      (?)
CC                      label   dword
                        dd      (?)
DD0                     label   dword
                        dd      (?)

; ==========================================================================
; Routine to Initialize Message Digest Buffer
; --------------------------------------------------------------------------
; procedure MD5Init(var Digest);
; ==========================================================================
                        public  MD5Init
MD5Init                 proc    far
                        push    BP
                        mov     BP,             SP
                        les     di,             [ BP+6 ]    ; hash[16]
                        mov     A,    67452301h
                        mov     B,   0efcdab89h
                        mov     C,    98badcfeh 
                        mov     D,    10325476h
                        pop     BP
                        ret     4
MD5Init                 endp

; ==========================================================================
;
; --------------------------------------------------------------------------
; procedure MD5Hash(var Data; var Digest);
; ==========================================================================
; This uses a pascal model: do the proper fiddling to use with C                        

                        public  MD5Hash
MD5Hash                 proc    far
                        push    BP                          ; stack frame
                        mov     BP,             SP
                        push    DS
                        lds     SI,             [ BP+10 ]   ; data[64]
                        les     DI,             [ BP+6 ]    ; hash[16]
                        stor    AA,             A           ; backup hash
                        stor    BB,             B
                        stor    CC,             C
                        stor    DD0,            D
; --------------------------------------------------------------------------
; In step i, the additive constant is the integer part of
; 4294967296 times abs(sin(i)), where i is in radians

; --------------------------------------------------------------------------
; Round 1.
; --------------------------------------------------------------------------
                        FF      a,b,c,d, X.[4*0], S11, 3614090360  ; Step 1
                        FF      d,a,b,c, X.[4*1], S12, 3905402710  ; Step 2
                        FF      c,d,a,b, X.[4*2], S13,  606105819  ; Step 3
                        FF      b,c,d,a, X.[4*3], S14, 3250441966  ; Step 4
                        FF      a,b,c,d, X.[4*4], S11, 4118548399  ; Step 5
                        FF      d,a,b,c, X.[4*5], S12, 1200080426  ; Step 6
                        FF      c,d,a,b, X.[4*6], S13, 2821735955  ; Step 7
                        FF      b,c,d,a, X.[4*7], S14, 4249261313  ; Step 8
                        FF      a,b,c,d, X.[4*8], S11, 1770035416  ; Step 9
                        FF      d,a,b,c, X.[4*9], S12, 2336552879  ; Step 10
                        FF      c,d,a,b, X.[4*10],S13, 4294925233  ; Step 11
                        FF      b,c,d,a, X.[4*11],S14, 2304563134  ; Step 12
                        FF      a,b,c,d, X.[4*12],S11, 1804603682  ; Step 13
                        FF      d,a,b,c, X.[4*13],S12, 4254626195  ; Step 14
                        FF      c,d,a,b, X.[4*14],S13, 2792965006  ; Step 15
                        FF      b,c,d,a, X.[4*15],S14, 1236535329  ; Step 16
; --------------------------------------------------------------------------
; Round 2.
; --------------------------------------------------------------------------
                        GG      a,b,c,d, X.[4*1], S21, 4129170786  ; Step 17
                        GG      d,a,b,c, X.[4*6], S22, 3225465664  ; Step 18
                        GG      c,d,a,b, X.[4*11],S23,  643717713  ; Step 19
                        GG      b,c,d,a, X.[4*0], S24, 3921069994  ; Step 20
                        GG      a,b,c,d, X.[4*5], S21, 3593408605  ; Step 21
                        GG      d,a,b,c, X.[4*10],S22,   38016083  ; Step 22 
                        GG      c,d,a,b, X.[4*15],S23, 3634488961  ; Step 23
                        GG      b,c,d,a, X.[4*4], S24, 3889429448  ; Step 24
                        GG      a,b,c,d, X.[4*9], S21,  568446438  ; Step 25
                        GG      d,a,b,c, X.[4*14],S22, 3275163606  ; Step 26
                        GG      c,d,a,b, X.[4*3], S23, 4107603335  ; Step 27
                        GG      b,c,d,a, X.[4*8], S24, 1163531501  ; Step 28
                        GG      a,b,c,d, X.[4*13],S21, 2850285829  ; Step 29
                        GG      d,a,b,c, X.[4*2], S22, 4243563512  ; Step 30
                        GG      c,d,a,b, X.[4*7], S23, 1735328473  ; Step 31
                        GG      b,c,d,a, X.[4*12],S24, 2368359562  ; Step 32
; --------------------------------------------------------------------------
; Round 3.
; --------------------------------------------------------------------------
                        HH      a,b,c,d, X.[4*5], S31, 4294588738  ; Step 33
                        HH      d,a,b,c, X.[4*8], S32, 2272392833  ; Step 34
                        HH      c,d,a,b, X.[4*11],S33, 1839030562  ; Step 35
                        HH      b,c,d,a, X.[4*14],S34, 4259657740  ; Step 36
                        HH      a,b,c,d, X.[4*1], S31, 2763975236  ; Step 37
                        HH      d,a,b,c, X.[4*4], S32, 1272893353  ; Step 38
                        HH      c,d,a,b, X.[4*7], S33, 4139469664  ; Step 39
                        HH      b,c,d,a, X.[4*10],S34, 3200236656  ; Step 40
                        HH      a,b,c,d, X.[4*13],S31,  681279174  ; Step 41
                        HH      d,a,b,c, X.[4*0], S32, 3936430074  ; Step 42
                        HH      c,d,a,b, X.[4*3], S33, 3572445317  ; Step 43
                        HH      b,c,d,a, X.[4*6], S34,   76029189  ; Step 44
                        HH      a,b,c,d, X.[4*9], S31, 3654602809  ; Step 45
                        HH      d,a,b,c, X.[4*12],S32, 3873151461  ; Step 46
                        HH      c,d,a,b, X.[4*15],S33,  530742520  ; Step 47
                        HH      b,c,d,a, X.[4*2], S34, 3299628645  ; Step 48
; --------------------------------------------------------------------------
; Round 4.
; --------------------------------------------------------------------------
                        II      a,b,c,d, X.[4*0], S41, 4096336452  ; Step 49
                        II      d,a,b,c, X.[4*7], S42, 1126891415  ; Step 50
                        II      c,d,a,b, X.[4*14],S43, 2878612391  ; Step 51
                        II      b,c,d,a, X.[4*5], S44, 4237533241  ; Step 52
                        II      a,b,c,d, X.[4*12],S41, 1700485571  ; Step 53
                        II      d,a,b,c, X.[4*3], S42, 2399980690  ; Step 54
                        II      c,d,a,b, X.[4*10],S43, 4293915773  ; Step 55
                        II      b,c,d,a, X.[4*1], S44, 2240044497  ; Step 56
                        II      a,b,c,d, X.[4*8], S41, 1873313359  ; Step 57
                        II      d,a,b,c, X.[4*15],S42, 4264355552  ; Step 58
                        II      c,d,a,b, X.[4*6], S43, 2734768916  ; Step 59
                        II      b,c,d,a, X.[4*13],S44, 1309151649  ; Step 60
                        II      a,b,c,d, X.[4*4], S41, 4149444226  ; Step 61
                        II      d,a,b,c, X.[4*11],S42, 3174756917  ; Step 62
                        II      c,d,a,b, X.[4*2], S43,  718787259  ; Step 63
                        II      b,c,d,a, X.[4*9], S44, 3951481745  ; Step 64
; --------------------------------------------------------------------------
                        addd    A,              AA          ; add hash
                        addd    B,              BB
                        addd    C,              CC
                        addd    D,              DD0

                        pop     DS                          ; restore stack
                        pop     BP                          ;   frame
                        ret     8

MD5Hash                 endp

; No finalization routines are included: an MD5Final() procedure could be
; added as a project to the user, or as part of the HLL interface (Pascal
; or C etc.), as with the Turbo Pascal demo included.


CODE                    ends
                        
                        end


