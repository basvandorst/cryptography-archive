; Last updated: Sat Oct 21 1995

; Test Driver for IDEA/CFB assembly implementation
;       By Fauzan Mirza

        .model  tiny
        .code

        extrn   IDEA : near
        extrn   Expandkey : near

        org     100h

start:
        mov     ah,09
        mov     dx,offset keyprompt
        int     21h                     ; Display prompt for key

        mov     ah,0ah
        mov     dx,offset keyinput
        int     21h                     ; Read user key

        mov     si,offset key
        mov     di,si
        call    Expandkey               ; Expand user key to IDEA key

        mov     ah,09
        mov     dx,offset dataprompt
        int     21h                     ; Display prompt for data

        mov     ah,0ah
        mov     dx,offset datainput
        int     21h                     ; Read data

        mov     si,offset key
        mov     di,offset data
        call    IDEA                    ; Encrypt data

        mov     ah,09
        mov     dx,offset output
        int     21h                     ; Display label for output

; Print routine by Bill Couture <bcouture@cris.com>

        mov     si,offset data
        mov     bp,4                    ;CX is used for shiftcount,
                                        ;use another reg for loop count
Print:
        lodsw                           ; read word (little-endian)
        push    ax
        mov     al,ah
        call    byteout
        pop     ax
        call    byteout
        mov     dl,20h
        call    charout                 ; seperate words, be neat!
        dec     bp
        jnz     print

        mov     ah,0
        int     21h

byteout:
        push    ax                              ;save number
        mov     cl,4                            ;for 4-bit shift
        shr     al,cl                           ;high nibble to low nibble
        call    digitout                        ;show this digit
        pop     ax                              ;get low digit back
digitout:
        and     al,0fh                          ;isolate low nibble
        add     al,90h                          ;trick 6-byte hex conversion
        daa
        adc     al,40h
        daa
        mov     dl,al                           ;char to dl
charout:
        mov     ah,2                            ;fn = char out
        int     21h                             ;call DOS
        ret                                     ;and done

output:
        db      0dh,0ah,"Encrypted: ",24h

keyprompt:
        db      "Enter key: ",24h

dataprompt:
        db      0dh,0ah,"Enter data: ",24h

datainput:
        db      9,0

data:
        db      9 dup (0)

keyinput:
        db      17,0

key:
        dw      52 dup (0)


        end     start

