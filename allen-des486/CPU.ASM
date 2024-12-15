;
; From the book "Using Assembly Language"
;               Allen L. Wyatt, Sr.
;               3rd Edition
;               QUE Corporation
;               Libraray of Congress Number: 92-80083
;               ISBN Number:                 0-88022-884-9
;               $29.95, US
;               $37.95, CAN
;               From Page 358-363
;
;
; Callable from C as:    int cpu()
;
; Return Values:   86  =    8086 processor
;                  88  =    8088 processor
;                 186  =   80186 processor
;                 188  =   80188 processor
;                 286  =   80286 processor
;                 386  =   80386 processor
;                 486  =   80486 processor
;
;
;

          PUBLIC _cpu

          .MODEL small, C

          .DATA

CPUType   dw   0000h
TestInst  db     43h

          .CODE

_cpu      proc  near

          pushf                         ; original flags
          pop   ax                      ; into a workable variable
          and   ax, 3fffh               ; turn-off bits 14, 15
          push  ax
          popf                          ; put in proper register
          pushf                         ; store flags
          pop   ax                      ; get back flags to examine
          and   ax, 0c000h              ; only interested in bits 15, 16
          cmp   ax, 0                   ; have they been cleared
          je    I286_1                  ; if so, it's at least a 286

          mov   cl, 21h
          mov   al, 0ffh
          shr   al, cl                  ; if AL not clear, it's an 80186 or 80188
          cmp   al, 0
          mov   ax, 188                 ; assume it's an 80188

          jne   DoBuss                  ; flag still set from previous CMP

          mov   ax, 88                  ; assume it's an 8088

DoBuss:   CALL  TestQueue               ; check the Buss width

          sub   ax, bx                  ; BX is 0 if 188 else BX is 2
          mov   CPUType, ax
          jmp   Exit

; at this point the processor is a 286, 386 or 486

I286_1:   pushf
          pop    ax
          or     ax, 4000h               ; set bit 4 (NT flag)
          push   ax
          popf                           ; flag bit 14 is now set
          pushf                          ; store flags
          pop    ax                      ; get flags back to examine
          and    ax, 4000h               ; only care about bit 14
          cmp    ax, 4000h               ; is it still set?
          je     I386_1                  ; if so, it's at least a 386
          mov    CPUType, 286            ; else it's a 286
          jmp    Exit

; at this point we know it is a 32-bit processor

          .386                           ; enable 386 assembly
I386_1:   pushfd                         ; push extended flags
          pop    eax                     ; get back to work with
          mov    ebx, eax                ; store a copy
          and    ebx, 40000h             ; only care about bit 18 (AC flag)
          xor    eax, 40000h             ; toggle AC flag (bit 18)
          push   eax                     ; stuff adjusted flag register
          popfd                          ; put it in the proper place
          pushfd                         ; push it again
          pop    eax                     ; get it back to examine
          and    eax, 40000h             ; want to look at bit 18 (AC flag)
          mov    CPUType, 386            ; assume it's a 386
          cmp    eax, ebx                ; if same, it's a 386
          je     Exit

          pushfd                         ; must be a 486
          pop    eax                     ; move to work on
          xor    eax, 40000h             ; toggle AC flag
          push   eax
          popfd
          mov    CPUType, 486

          .8086
Exit:     mov    ax, CPUType

          ret

_cpu      endp




TestQueue proc

          USES   ax

          mov    bx, 1
          mov    al, TestInst            ; Load opcode for INC BX
          xor    al, 8                   ; change INC BX to DEC BX
          mov    TestInst, al            ; move it back to proper place
          nop                            ; If using an 8-bit data bus,
          nop                            ;    the following 4 NOPs will
          nop                            ;    fill the queue, allowing the
          nop                            ;    next bytes to be changed

          ret

TestQueue endp

          end

