; Last updated: Sat Oct 21 1995

; Tandem Davies-Meyer hashing algorithm using IDEA
;       By Fauzan Mirza

        .model  tiny
        .code

; Public symbols for external linking (near)

        public  TandemDM
        public  hashBurn
        public  Hash

        extrn   Expandkey : near
        extrn   IDEA : near

; Tandem DM hashing algorithm
; Entry:  si -> data, cx=number of eight-byte blocks
; Exit:   all registers trashed
; Input must be in big-endian, but hash will be in little-endian

h       equ     Hash
g       equ     Hash+8

;  tandem DM-scheme
;     w = IDEA(g[i-1] * 2**64 + x[i], h[i-1])
;     h[i] = h[i-1] ^ w
;     g[i] = g[i-1] ^ IDEA(x[i] * 2**64 + w, g[i-1])

TandemDM:
        push    cx
        call    hashBurn        ; -- Zero data area
        pop     cx
hashBlock:
        push    cx              ; push block counter    <0
        push    si              ; push data             <1

        mov     cx,4
        mov     si,offset g     ; -- Let first half of key = second half of hash (G)
        mov     di,offset key
        repnz   movsw           ; after: di->key+8

        pop     si              ; si->data              1>
        push    si              ; push data             <1
        mov     cl,4            ; -- Let second half of key = data
copy1:
        lodsw
        xchg    ah,al           ; -- (Convert string to little-endian words)
        stosw
        loop    copy1

        mov     si,offset key
        mov     di,si
        push    si              ; push key              <2
        call    Expandkey       ; -- Expand key : Key = {G, data}

        mov     cx,4
        mov     si,offset h
        mov     di,offset w
        push    di              ; push w                <3
        repnz   movsw           ; -- Let W = first half of hash (H)

        pop     di              ; di->w                 3>
        pop     si              ; si->key               2>
        push    si              ; push key              <2

        call    IDEA            ; -- W = IDEA encrypt (W, {G,data})

        mov     si,di           ; si->w
        mov     di,offset h
        mov     cx,4
doH:
        lodsw
        xor     ax,[di]
        stosw
        loop    doH             ; -- Update H with (H xor W)

        pop     di              ; di->key               2>
        pop     si              ; si->data              1>
        push    si              ; push data             <1
        push    di              ; push key              <2
        mov     cl,4            ; -- Let first half of key = data
copy2:
        lodsw
        xchg    ah,al           ; -- (Convert string to little-endian words)
        stosw
        loop    copy2

        mov     cl,4
        mov     si,offset w     ; di->key+8
        repnz   movsw           ; -- Let second half of key = W

        pop     si              ; si->key               2>
        mov     di,si
        call    Expandkey       ; -- Expand key : Key = {data, W}

        mov     cx,4
        mov     si,offset g
        mov     di,offset g0
        push    si              ; push g                <2
        push    di              ; push g0               <3
        repnz   movsw           ; -- Let G0 = G

        pop     di              ; di->g0                3>
        push    di              ; push g0               <3
        mov     si,offset key
        call    IDEA            ; -- G0 = IDEA encrypt (G0, {data, W})

        pop     si              ; si->g0                3>
        pop     di              ; di->g                 2>
        mov     cx,4
doG:
        lodsw
        xor     ax,[di]
        stosw
        loop    doG             ; -- Update G with (G xor G0)

        pop     si              ; si->data              1>
        add     si,8            ; data+=8
        pop     cx              ; cx=block counter      0>

        loop    hashBlock       ; -- Continue hashing until no more blocks

        ret

; Burn the hash and associated buffers
; Entry:  nothing
; Exit:   {ax,cx,di} trashed

hashBurn:
        mov     di,offset Hash
        mov     cx,worksp
        xor     ax,ax
        repnz   stosb
        ret

Hash:
        dw      8 dup (0)
w:
        dw      4 dup (0)
g0:
        dw      4 dup (0)
key:
        dw      52 dup (0)

worksp  equ     $-Hash

        end

