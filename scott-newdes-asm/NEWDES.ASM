          page     81,132
;--- Program NEWDES
;
;   This program implements an encryption algorithm called "NEWDES".
;
;   See Robert Scott's article "Wide-open Encryption Design Offers
;   Flexible Implementations" in Volume 9, Number 1 (January 1985)
;   of Cryptologia.
;   This algorithm resembles the Data Encryption Standard, but is easier
;   to implement in software and is supposed to be more secure.
;
;   Called as:
;
;   Written by Mark Riordan, March 1988.
;   Adapted from MCOPY, written by Mark Riordan on 23 December 1984.


NEWDES_CD SEGMENT  PARA COMMON 'CODE'
          assume   cs:newdes_cd,ds:dataseg,ss:stkseg
MCOPY:    jmp      main
          int      20h

;-------- Symbols ------------------------------------------------------

as_cr     equ      13              ;ascii carriage return
as_lf     equ      0ah             ;ascii line feed

hnd_stin  equ      0               ;handle for standard input
hnd_stout equ      1               ;handle for standard output

acc_read  equ      0               ;access code for read
acc_write equ      1               ;access code for write

int_dos   equ      21h             ;interrupt number for DOS interrupt.

fnc_creat equ      3ch             ;create a file
fnc_rdfod equ      3fh             ;read from file or device
fnc_wtfod equ      40h             ;write to file or device
fnc_alloc equ      48h             ;allocate memory
fnc_setbl equ      4ah             ;SETBLOCK--Modify allocated memory
fnc_term  equ      4ch             ;Terminate a process (Exit)
fnc_open  equ      3dh             ;Open a file
fnc_close equ      3eh             ;Close a file handle.

lenkey    equ      15              ;Number of bytes in the key.
bufsize   equ      32768           ;Number of bytes in the buffer.

;-------- Start of Main Program ----------------------------------------
main:
          mov      ax,seg dataseg
          mov      ds,ax
          mov      savepsp,es

          call     crack           ;crack control statement
;;        call     getmem          ;obtain buffer space.
          call     openfiles       ;open files.
          call     readkey         ;read and process key.
          call     docopy          ;do the file copy.
          call     endit           ;terminate normally.

;--- Subroutine ABORT
;
;       entry   DS:DX point to an error message terminated by a $.
;
;       exit    None--program is terminated.

ABORT     PROC
          call     putmsg
          lea      dx,msgsyntax
          call     putmsg
          mov      ah,fnc_term
          mov      al,1
          int      int_dos
ABORT     ENDP

;--- Subroutine COPYFNAME
;
;         entry    ES:SI points to the beginning of a file name.
;                  DI points to the place to copy it.
;
;         exit     SI points to LWA+1 of file name source.
;                  The file name has been copied & converted to upper case.
;                  There's a trailing zero byte on the file name.

COPYFNAME PROC
copylp:
          cmp      si,ncmdch
          jae      endcopy         ;jump if reached end of string
          mov      al,es:[si]
          call     ifdelim
          jc       endcopy         ;jump if hit delimiter (end of file name)

          mov      al,es:[si]
          inc      si
          call     toupper         ;convert to upper case
          mov      [di],al         ;copy to destination
          inc      di
          jmp      copylp          ;loop
endcopy:
          mov      byte ptr [di],0 ;put in 0 to make it an ASCIIZ string.
          ret
COPYFNAME ENDP

;--- Subroutine CRACK
;
;       entry   uncracked statement resides at DS:80h
;
;       exit    encflag  is 1 for encryption, 0 for decryption.
;               keyfname contains key file name.
;               We have aborted with an error message if there
;                 was a syntax error.

CRACK     PROC
          mov      bx,80h
          mov      al,es:[bx]
          xor      ah,ah           ;ax=number of parameter characters
          add      ax,81h
          mov      ncmdch,ax       ;ncmdch=number of parameter characters
                                   ; including 80h bytes at beginning

          mov      si,81h          ;point to first param character

;         Crack the parameters.
;         SI points to the current position in the command line.

parmloop:
          call     skpdelim
          jc       endparms        ;jump if no more parameters.
          mov      ax,es:[si]

;         Look for -kkeyfile

          cmp      ax,'k-'
          jne      nokey
          lea      di,keyfname
          add      si,2
          call     copyfname       ;copy file name to holding place.
          mov      gotkey,1
          jmp      endparmlp

;         Look for -e

nokey:
          cmp      ax,'e-'
          jne      noencrypt
          mov      gotenc,1
          mov      encflag,1
          add      si,2
          jmp      endparmlp

;         Look for -d

noencrypt:
          cmp      ax,'d-'
          jne      nodecrypt
          mov      gotenc,1
          mov      encflag,0
          add      si,2
          jmp      endparmlp
nodecrypt:

;         Unrecognized parameter

          lea      dx,msgbadparm
          call     abort

endparmlp:
          jmp      parmloop
endparms:

          cmp      gotenc,0
          jne      gotencok
          lea      dx,msgnoenc
          call     abort
gotencok:
          cmp      gotkey,0
          jne      gotkeyok
          lea      dx,msgnokey
          call     abort
gotkeyok:
          ret

CRACK     ENDP

;--- Subroutine DOBLOCK
;
;    Encipher or decipher an 8-byte block.
;
;         entry    SI points to the first byte of the block
;                  BH is zero.
;                  key has been set up properly.  This is 60 bytes
;                      of key information arranged so that we can just
;                      increment through it 0,1,2, etc. rather than
;                      testing for modulus 15.
;                      key is also set up properly to do encryption
;                      or decryption upon entry.  Thus, DOBLOCK serves
;                      both to encrypt or decrypt, the only difference
;                      being how "key" is set up.
;
;         exit     The block has been transformed.
;                  SI still points to its FWA.
;
;         Method:  The eight bytes of the block, designated as B0 - B7,
;                  are loaded into registers.  Then we go through
;                  eight rounds of encryption, followed by another
;                  half-round of encryption.
;                  Note that my use of the word "round" differs from
;                  Robert Scott's:  he has 17 rounds, alternating
;                  dissimilar odd and even rounds.  I have 8.5 identical
;                  rounds.  Just different ways of looking at things.
;
;                  AX holds B1 B0   (B0 in AL)
;
;                  AX = B1 B0
;                  BX used for implementation of f function & scratch usage
;                  CX = B7 B6
;                  DX = B5 B4
;                  SI holds pointer to block upon entry; round count in loop
;                  DI holds pointer to next byte of key ("i" in Scott's article)
;                  BP = B3 B2
;
;                  Note:  BP is not byte-addressable, so it is exchanged
;                         with AX when we need to access it.


DOBLOCK   PROC

;         Load up the registers.

          mov      ax,[si]         ;AX = B1 B0
          mov      bp,[si+2]       ;BP = B3 B2
          mov      dx,[si+4]       ;DX = B5 B4
          mov      cx,[si+6]       ;CX = B7 B6
          push     si
          lea      di,key

;         Do the encryption.

          mov      si,8

encloop:

;         B4 = B4 ^ f[B0 ^ key[i++]]

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,al           ;bl=b0 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      dl,bl           ;B4 = B4 ^ bl

;         B5 = B5 ^ f[B1 ^ key[i++]]

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,ah           ;bl=B1 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      dh,bl           ;B5 = B5 ^ bl

;         B6 = B6 ^ f[B2 ^ key[i++]]

          xchg     ax,bp           ;ax=B3 B2; bp=B1 B0

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,al           ;bl = B2 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      cl,bl           ;B6 = B6 ^ bl

;         B7 = B7 ^ f[B3 ^ key[i++]]

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,ah           ;bl = B3 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      ch,bl           ;B7 = B7 ^ bl

          xchg     ax,bp           ;ax=B1 B0; bp=B3 B2

;    ---  End of half-round  -------

;         B1 = B1 ^ f[B4 ^ key[i++]]

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,dl           ;bl = B4 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      ah,bl           ;B1 = B1 ^ bl

;         B2 = B2 ^ f[B4 ^ B5]

          xchg     ax,bp           ;ax=B3 B2; bp=B1 B0

          mov      bl,dl           ;bl=B4
          xor      bl,dh           ;bl = B4 ^ B5
          mov      bl,[bx]         ;bl = f(bl)
          xor      al,bl           ;B2 = B2 ^ bl

;         B3 = B3 ^ f[B6 ^ key[i++]]

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,cl           ;bl = B6 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      ah,bl           ;B3 = B3 ^ bl

;         B0 = B0 ^ f[B7 ^ key[i++]]

          xchg     ax,bp           ;ax=B1 B0; bp=B3 B2

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,ch           ;bl = B7 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      al,bl           ;B0 = B0 ^ bl

;---      End of odd & even round loop.
;         Decrement loop counter and loop if necessary.

          dec      si
          jnz      encloop         ;jump if not done with 8 rounds

;         Now do one last half-round.  -------------------------------------

;         B4 = B4 ^ f[B0 ^ key[i++]]

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,al           ;bl=b0 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      dl,bl           ;B4 = B4 ^ bl

;         B5 = B5 ^ f[B1 ^ key[i++]]

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,ah           ;bl=B1 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      dh,bl           ;B5 = B5 ^ bl

;         B6 = B6 ^ f[B2 ^ key[i++]]

          xchg     ax,bp           ;ax=B3 B2; bp=B1 B0

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,al           ;bl = B2 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      cl,bl           ;B6 = B6 ^ bl

;         B7 = B7 ^ f[B3 ^ key[i++]]

          mov      bl,[di]         ;bl=key[i]
          inc      di              ;i++
          xor      bl,ah           ;bl = B3 ^ key[i]
          mov      bl,[bx]         ;bl = f(bl)
          xor      ch,bl           ;B7 = B7 ^ bl

          xchg     ax,bp           ;ax=B1 B0; bp=B3 B2

;    ---  End of last half-round  -------

;         Save the registers back in the block.

          pop      si
          mov      [si],ax
          mov      [si+2],bp
          mov      [si+4],dx
          mov      [si+6],cx

          ret
DOBLOCK   ENDP

;--- Subroutine DOCOPY
;
;         entry    inhand is the input handle.
;                  outhand is the output handle.
;                  buffwa is the FWA of the buffer, in absolute paragraphs
;                  buflen is the length of the buffer in bytes.
;
;         exit     The file has been copied.

DOCOPY    PROC
docopylp:

;         Do the DOS read.

          mov      bx,inhand
          mov      cx,buflen
          lea      dx,buffer
          mov      ah,fnc_rdfod
          int      int_dos         ;do the read.
          jc       doreader        ;jump if read error

          test     ax,ax
          jz       enddocopy       ;jump if end of file encountered.

;         First, determine whether the number of bytes read in is
;         a multiple of eight.  It will be except possibly on the last
;         buffer.
;         If it is not, pad out the block with zeros to make it a multiple
;         of 8.
;         AX = # of bytes read in.

          test     ax,7            ;test bottom 3 bits
          jz       mult8           ;jump if multiple of eight
          mov      di,ax           ;di points to first byte after buffer
          add      ax,8
          and      ax,0fff8h       ;ax is next greater mult of 8
          mov      bh,0
zerolp:
          mov      [buffer+di],bh         ;zero out a byte in the buffer
          inc      di
          cmp      di,ax
          jne      zerolp          ;jump if haven't reached end of block

mult8:
          mov      nbytes,ax
          mov      bh,0

;         Now encipher all the blocks in the buffer.

          mov      bytesrem,ax     ;store number of bytes left
          lea      si,buffer

;         Loop to encipher/decipher one block per pass.
;         SI       points to block to process.
;         bytesrem is the number of bytes remaining to be processed.

doblklp:
          call     doblock
          add      si,8            ;move pointer to next block
          sub      bytesrem,8      ;decrement bytes left to process
          jnz      doblklp         ;jump if more bytes remain


;         Flush the output buffer.
;         nbytes = number of bytes in output buffer.

          mov      cx,nbytes       ;number of bytes to write.
          mov      bx,outhand
          mov      ah,fnc_wtfod
          lea      dx,buffer
          int      int_dos         ;do the write.
          jc       dowriteer       ;jump if write error.

          jmp      docopylp        ;loop

doreader:
          lea      dx,rdmsg1
          call     abort
dowriteer:
          lea      dx,wtmsg1
          call     abort

;         Close the files.

enddocopy:
          mov      bx,inhand
          mov      ah,fnc_close
          int      int_dos
          jc       closeerr

          mov      bx,outhand
          mov      ah,fnc_close
          int      int_dos
          jc       closeerr
          ret

closeerr:
          lea      dx,clmsg1
          call     abort

DOCOPY    ENDP

;--- Subroutine ENDIT
;
;   Terminate the program.

ENDIT     PROC
          mov      ah,fnc_term
          mov      al,0
          int      int_dos
          jmp      dword ptr stopcode
ENDIT     ENDP

;--- Subroutine GETMEM
;
;         entry    We want to allocate memory for a buffer
;
;         exit     buffwa is the absolute address of the buffer,
;                    in paragraph form.

GETMEM    PROC
;;        lea      bx,[endmem+16]
          mov      cl,4
          shr      bx,cl           ;bx=LWA+n of program, in paragraph form
                                   ; (with only offset accounted for)
          mov      si,bx
          mov      ax,ds
          add      ax,bx           ;ax=LWA+n of program, absolute, paragraph
;;        mov      buffwa,ax       ;store address for later

          mov      bx,3840         ;get buffer space desired.
          add      bx,si           ;add in size of program for request.
          mov      ah,fnc_setbl
          int      int_dos         ;get blocks
          jnc      getok           ;jump if went all right

          lea      dx,geterr
          call     abort
getok:
          ret

GETMEM    ENDP

;--- Subroutine IFDELIM
;
;       entry   AL has a character
;
;       exit    AL still has the character.
;               carry is set if it's a delimiter (space or comma).

IFDELIM PROC
          cmp      al,' '
          je       yesdelim
          cmp      al,','
          je       yesdelim
          clc
          ret
yesdelim:
          stc
          ret
IFDELIM   ENDP

;--- Subroutine OPENFILES
;
;         entry    keyfname contains the key file name.
;
;         exit     The files have been opened.
;                  If an error occurred, we have issued a message and
;                    aborted.

OPENFILES PROC
          lea      dx,keyfname
          mov      al,acc_read
          mov      ah,fnc_open
          int      int_dos
          mov      keyhand,ax
          jnc      openinok        ;jump if open went OK.

          lea      dx,msgkeyopen
          call     abort
openinok:
          ret
OPENFILES ENDP

;--- Subroutine PUTMSG
;
;         entry    DS:DX point to a message terminated by a $.

PUTMSG    PROC
          mov      ah,9
          int      int_dos
          ret
PUTMSG    ENDP

;--- Subroutine READKEY
;
;         entry    encflag  is 1 for encrypt, 0 for decrypt.
;
;         exit     key      has a copy of the key bytes in the
;                           order in which they will be used.
;                           This is dependent upon whether we are
;                           enciphering or deciphering.

READKEY   PROC

;         Try to read "lenkey" bytes from the key file into "inikey".

          mov      ah,fnc_rdfod    ;read from file or device
          mov      bx,keyhand
          mov      cx,lenkey
          lea      dx,inikey
          int      int_dos

;         If we got anything other than exactly 15 bytes, signal error.

          cmp      ax,lenkey
          je       nkeyok          ;jump if byte count OK.
          lea      dx,msgkeyct
          call     abort
nkeyok:

          cmp      encflag,0
          je       dodecip         ;jump if decipher

;         Set up the key for enciphering.
;         We got the 15 bytes OK--now copy these bytes into the "key"
;         of the array.  Simply make 4 consecutive copies of the byte string,
;         totalling 60 bytes.

          mov      cx,4
          lea      di,key
kystrlp:
          push     cx
          mov      cx,lenkey
          lea      si,inikey
kybytlp:
          mov      al,[si]
          inc      si
          mov      [di],al
          inc      di
          loop     kybytlp
          pop      cx
          loop     kystrlp

          jmp      short endkey

;--       Come here to set up the key for deciphering.
;
dodecip:
          mov      si,11
          mov      di,0
enckeylp:
          mov      al,inikey[si]
          inc      si
          cmp      si,15
          jne      iok1
          mov      si,0
iok1:
          mov      key[di],al
          inc      di

          mov      al,inikey[si]
          inc      si
          cmp      si,15
          jne      iok2
          mov      si,0
iok2:
          mov      key[di],al
          inc      di

          mov      al,inikey[si]
          inc      si
          cmp      si,15
          jne      iok3
          mov      si,0
iok3:
          mov      key[di],al
          inc      di

          mov      al,inikey[si]
          mov      key[di],al
          inc      di

          add      si,9
          cmp      si,15
          jl       nokeyover       ;jump if we did not overflow 15 byte
          sub      si,15
nokeyover:
          cmp      si,12
          je       endkey

          mov      al,inikey[si]
          inc      si
          mov      key[di],al
          inc      di

          mov      al,inikey[si]
          inc      si
          mov      key[di],al
          inc      di

          mov      al,inikey[si]
          mov      key[di],al
          inc      di

          add      si,9
          cmp      si,15
          jl       nokeyover2      ;jump if we did not overflow 15 byte
          sub      si,15
nokeyover2:

          jmp      enckeylp        ;jump if more key bytes to write

endkey:
          ret
READKEY   ENDP

;--- Subroutine SKPDELIM
;
;       entry      SI points to part of a parameter string.
;
;       exit       SI points to the next character that is
;                    not a delimiter (space or comma)
;                  Carry is set if we ran off the end of the
;                    command line.

SKPDELIM  PROC
skploop:
          cmp      si,ncmdch
          jge      toofar          ;jump if we have gone too far
          mov      al,es:[si]
          call     ifdelim
          jc       skpone          ;jump if it's a delimiter
          ret
skpone:
          inc      si
          cmp      si,ncmdch
          jl       skploop         ;jump if we have not gone too far

;         We have gone too far.

toofar:
          stc
          ret
SKPDELIM  ENDP

;--- Subroutine TOUPPER
;
;         entry    AL has a character
;
;         exit     AL has the upper case version of that character

TOUPPER   PROC
          cmp      al,'a'
          jb       endtoupper      ;jump if not a lower case letter
          cmp      al,'z'
          ja       endtoupper      ;jump if not a lower case letter
          sub      al,'a'-'A'      ;switch to upper case
endtoupper:
          ret
TOUPPER   ENDP
NEWDES_CD ENDS

stkseg    SEGMENT  STACK
          db       256 dup (?)
stkseg    ENDS

dataseg   SEGMENT  'DATA'

;-------- Data Area ----------------------------------------------------

f         db    32,137,239,188,102,125,221, 72,212, 68, 81, 37, 86,237,147,149
          db    70,229, 17,124,115,207, 33, 20,122,143, 25,215, 51,183,138,142
          db   146,211,110,173,  1,228,189, 14,103, 78,162, 36,253,167,116,255
          db   158, 45,185, 50, 98,168,250,235, 54,141,195,247,240, 63,148,  2
          db   224,169,214,180, 62, 22,117,108, 19,172,161,159,160, 47, 43,171
          db   194,175,178, 56,196,112, 23,220, 89, 21,164,130,157,  8, 85,251
          db   216, 44, 94,179,226, 38, 90,119, 40,202, 34,206, 35, 69,231,246
          db    29,109, 74, 71,176,  6, 60,145, 65, 13, 77,151, 12,127, 95,199
          db    57,101,  5,232,150,210,129, 24,181, 10,121,187, 48,193,139,252
          db   219, 64, 88,233, 96,128, 80, 53,191,144,218, 11,106,132,155,104
          db    91,136, 31, 42,243, 66,126,135, 30, 26, 87,186,182,154,242,123
          db    82,166,208, 39,152,190,113,205,114,105,225, 84, 73,163, 99,111
          db   204, 61,200,217,170, 15,198, 28,192,254,134,234,222,  7,236,248
          db   201, 41,177,156, 92,131, 67,249,245,184,203,  9,241,  0, 27, 46
          db   133,174, 75, 18, 93,209,100,120, 76,213, 16, 83,  4,107,140, 52
          db    58, 55,  3,244, 97,197,238,227,118, 49, 79,230,223,165,153, 59

cmdptr    dw       ?               ;pointer to character on command line
ncmdch    dw       ?               ;number of characters on command line.
keyfname  db       64 dup (?)
inhand    dw       0
outhand   dw       1
keyhand   dw       ?               ;handle for key file.
; buffwa    dw       ?               ;address of FWA of I/O buffer.
buflen    dw       bufsize         ;length of buffer.
stopcode  dw       103h            ;offset of code to terminate
savepsp   dw       ?               ;seg address of Program Segment Prefix

inikey    db       15 dup (?)
key       db       60 dup (?)
;;key       db       31,41,59,26,53,58,97,93,238,46,26,43,38,32,79
;;          db       31,41,59,26,53,58,97,93,238,46,26,43,38,32,79
;;          db       31,41,59,26,53,58,97,93,238,46,26,43,38,32,79
;;          db       31,41,59,26,53,58,97,93,238,46,26,43,38,32,79
begmsg    db       'Hi there.'
          db       as_cr,as_lf
          db       '$'
rdmsg1    db       'Error encountered while reading file.'
          db       '$'
wtmsg1    db       'Error encountered while writing file.'
          db       '$'
clmsg1    db       'Error encountered while closing file.'
          db       '$'
msgkeyopen  db       'Error in opening key file.'
          db       '$'
geterr    db       'Error:  Cannot allocate enough buffer space.'
          db       '$'
msgnoenc  db       'You must specify either -e or -d.'
          db       '$'
msgnokey  db       'You must specify a key file (-kkeyfile)'
          db       '$'
msgbadparm db      'Unrecognized parameter.'
          db       '$'
msgkeyct  db       'Error:  the key file contained less than 15 bytes.'
          db       '$'
msgsyntax db       as_cr,as_lf
          db       'Usage:  newdes <infile >outfile {-e | -d} -kkeyfile'
          db       as_cr,as_lf
          db       '        -e means encrypt; -d means decrypt.'
          db       as_cr,as_lf
          db       '        keyfile should have exactly 15 binary bytes.'
          db       as_cr,as_lf
          db       '(Program by Mark Riordan  1100 Parker  48912;  '
          db       'algorithm by Robert Scott.)'
          db       '$'
gotenc    db       0
encflag   db       ?               ;1=encipher, 0=decipher
gotkey    db       0
blockoff  dw       ?               ;offset of current block being enciphered
                                   ;(from beginning of I/O buffer)
nbytes    dw       ?               ;Number of bytes in buffer
bytesrem  dw       ?               ;Number of bytes left to encipher


          even
buffer    db       32 dup (?)
          org      $+bufsize
dataseg   ENDS
          end      MCOPY
