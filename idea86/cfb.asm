; Last updated: Sun Oct 22 1995

; IDEA/CFB assembly implementation
;       By Fauzan Mirza

        .model  tiny
        .code

; Public symbols for external linking (near)

        public  CFBBuffer
        public  Encrypt
        public  Decrypt

        extrn   Expandkey : near
        extrn   IDEA : near

; Cipher Feedback
; Entry:  si -> key, di-> data block
; Exit:   di -> processed data block, other registers trashed
; Key assumed to be little-endian, data assumed to be big-endian

CFBBuffer:
        db      8 dup (?)

; CFB encrypt routine

Encrypt:
        push    di
        mov     di,offset CFBBuffer     ; Encrypt 8 byte CFB buffer (DI)
        call    IDEA
        pop     si                      ; SI -> data, DI -> CFB buffer
        mov     cx,4                    ; Process 4 words
EnFeedback:
        lodsw                           ; Get a word from file buffer
        xchg    ah,al                   ;   and convert it to little-endian
        xor     ax,word ptr [di]        ; XOR data with CFB buffer
        stosw                           ; Replace ciphertext in CFB buffer
        xchg    ah,al                   ; Convert back to big-endian
        mov     word ptr [si-2],ax      ;   and store in file buffer
        loop    EnFeedback
        ret

; CFB decrypt routine

Decrypt:
        push    di
        mov     di,offset CFBBuffer     ; Encrypt 8 byte CFB buffer (DI)
        call    IDEA
        mov     si,di                   ; SI -> CFB buffer
        pop     di                      ; DI -> data
        mov     cx,4                    ; Process 4 words
DeFeedback:
        lodsw                           ; Get word from CFB buffer
        mov     dx,word ptr [di]        ; Get word from file buffer
        xchg    dh,dl                   ;   convert it to little-endian
        mov     word ptr [si-2],dx      ; Update CFB buffer
        xor     ax,dx                   ; XOR data with CFB buffer
        xchg    ah,al                   ; Convert back to big-endian
        stosw                           ;   and store in file buffer
        loop    DeFeedback
        ret

        end

