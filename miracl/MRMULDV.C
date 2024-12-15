;
;  Watcom C/386 32-bit compiler V7.0. Use with mirdef.h32
;  Most parameters passed in registers
;  Written for Phar Lap 386ASM macro-assembler
;

        .386
        ASSUME CS:_TEXT
_TEXT   SEGMENT BYTE PUBLIC 'CODE'

        PUBLIC muldiv_
muldiv_ PROC NEAR

        mul     edx                 ;multiply a*b
        add     eax,ebx             ;add in c
        adc     edx,0               ;carry
        div     ecx                 ;divide by m
        mov     ebx,[esp+4]
        mov     [ebx],edx           ;remainder
        ret     4                   ;quotient in eax

muldiv_ endP

        PUBLIC muldvm_
muldvm_ PROC NEAR

        xchg    eax,edx       ;a*base+c
        div     ebx           ;divide by m
        mov     [ecx],edx     ;store remainder
        ret                   ;quotient in eax

muldvm_ endP

        PUBLIC muldvd_
muldvd_ PROC NEAR

        mul     edx           ;multiply a*b
        add     eax,ebx       ;add in c
        adc     edx,0
        mov     [ecx],eax     ;store remainder
        mov     eax,edx       ;get quotient
        ret                   ;quotient in eax

muldvd_ endP

_TEXT   ENDS
END
