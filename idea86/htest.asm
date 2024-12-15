; Last updated: Sat Oct 21 1995

; Test driver for Tandem Davies-Meyer hash routine
;       By Fauzan Mirza

        .model  tiny
        .code

        extrn   TandemDM : near
        extrn   hashBurn : near
        extrn   Hash : near

BufLen  equ     128                     ; Length of message

        org     100h

start:
        mov     ah,09
        mov     dx,offset prompt
        int     21h

        mov     ah,0ah
        mov     dx,offset input
        int     21h

        mov     cx,BufLen               ; number of blocks (CX)
        shr     cx,1                    ;   = message length / block size
        shr     cx,1
        shr     cx,1

        mov     si,offset data
        call    TandemDM

; Hash printing routine by Bill Couture <bcouture@cris.com>

        mov     dl,0dh
        call    charout
        mov     dl,0ah
        call    charout                 ;CR/LF, be neat!
                                        ;Unless, of course, you want the
                                        ;hash to over-write the phrase!
        mov     si,offset Hash
        mov     bp,8                    ;CX is used for shiftcount,
                                        ;use another reg for loop count
Print:
        lodsw                           ; read words, not bytes
        push    ax
        mov     al,ah
        call    byteout
        pop     ax
        call    byteout
        mov     dl,20h
        call    charout                 ; seperate words, be neat!
        dec     bp
        jnz     print

        call    hashBurn                ; destroy evidence

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

prompt:
        db      "Enter message: ",24h

input:
        db      BufLen,0

data:
        db      BufLen dup (0)

        end     start

