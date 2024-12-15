CODE            segment byte public 
                assume  CS: CODE
                assume  DS: nothing, ES: nothing, SS: nothing

; =========================================================================
; function GetCpu: Word;
; =========================================================================
; Based on code from Intel's 486 Data Books, originally part of IdentifyCpu
; procedure which comes from the Pascal FastMem routines (PFU-32.ZIP).
        
; Does not detect Pentiums or higher CPUs though, nor does it differentiate
; between 8080/8086/8088/80186 machines.

; An interesting bit of trivia(?): some 486s have the Pentium's "cpuid"
; instruction, and some CPU detection routines may call a 486 a Pentium.


                public  GetCpu
GetCpu          proc    far                    ; Identify Cpu
;               push    BP                     ; save stack frame
;               mov     BP,     SP
;               push    DS                     ; save DS
                mov     DX,     0    
                pushf
                pushf
                pop     BX
                mov     AX,     0fffh
                and     AX,     BX
                push    AX
                popf
                pushf
                pop     AX
                and     AX,     0f000h
                cmp     AX,     0f000h
                mov     DX,     0    
                je              short Done
                or      BX,     0f000h
                push    BX
                popf
                pushf
                pop     AX
                and     AX,     0f000h
                mov     DX,     2      
                jz              short Done
                mov     BX,     SP
                and     SP,     not 3
                db      66h
                pushf
                db      66h
                pop     AX
                db      66h
                mov     CX,     AX
                db      66h
                xor     AX,     0
                dw      4
                db      66h
                push    AX
                db      66h
                popf
                db      66h
                pushf
                db      66h
                pop     AX
                db      66h
                xor     AX,     CX
                mov     DX,     3     
                je              short Done
                mov     DX,     4      
Done:           mov     AX,     DX
                popf
;               pop     DS
;               pop     BP
                retf
        
GetCpu          endp

CODE            ends
                end

