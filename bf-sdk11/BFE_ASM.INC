

; Include file to use the Blowfish-Engine BFENG386 from Assembler
; (c) 1996 Cedric Reinartz

; !!!       All restrictions in BF-SDK.TXT apply        !!!

; The variable noPUB decides if the sourcecode from BFENG386 will be
; included, or if the Objekt-File BFENG386.OBJ will be used.

ifndef noPUB
        extrn Blowfish_GetBoxPointer: far
        extrn Blowfish_GetBoxes: far
        extrn Blowfish_SetBoxes: far
        extrn Blowfish_Init: far
        extrn Blowfish_ECBEncrypt: far
        extrn Blowfish_ECBDecrypt: far
        extrn Blowfish_CBCEncrypt: far
        extrn Blowfish_CBCDecrypt: far
        extrn Blowfish_Done: far
        extrn Blowfish_SetRounds: far
	extrn Blowfish_WeakKey: far
	.model large
	.386
else
	include bfeng386.asm
endif


callf	macro p1
;	call	far ptr p1		; normaly you don't need it
	call	p1			; The Ass. detects it from .model 
endm

_GetBoxPointer macro			; returns a Pointer to PBox
	callf	Blowfish_GetBoxPointer	; (DX:AX)
endm

_GetBoxes macro seg,off			; Saves the aktual P- and SBoxes
	push	seg			; at adress  seg:off
	push	off
	callf	Blowfish_GetBoxes
endm

_SetBoxes macro seg,off			; loads the P- and SBoxes with Data
	push	seg			; from adress   seg:off
	push	off
	callf	Blowfish_SetBoxes
endm

_ClearBoxes macro			; Wipes all Boxes clear
	callf	Blowfish_Done
endm

_SetRounds macro rnds			; Set the number of Rounds
	push	rnds
	callf	Blowfish_SetRounds
endm

_InitCrypt macro seg,off,len		; Initialises the Key with the Passw.
	push	seg
	push	off
	push	len
	callf	Blowfish_Init
endm

_WeakKey macro
	callf	Blowfish_WeakKey	; Test if (initialised) Key is weak
endm

_Encrypt macro seg,off,len		; Encrypts a String (ECB Method)
	push	seg			; len must be x*8
	push	off			
	push	len
	callf	Blowfish_ECBEncrypt
endm

_Decrypt macro seg,off,len		; Decrypts a String (ECB Method)
	push	seg			; len must be x*8
	push	off		
	push	len
	callf	Blowfish_ECBDecrypt
endm

_CBCEncrypt macro seg,off,len,segcl,offcl,segch,offch  ; encrypts (CBC Method)
	push	seg		; Adress of the Data
	push	off
	push	len		; len of data
	push	segcl		; Adress of CBC low
	push	offcl
	push	segch		; and CBC high
	push	offch
	callf	Blowfish_CBCEncrypt
endm

_CBCDecrypt macro seg,off,len,segcl,offcl,segch,offch ; decrypts (CBC Method)
	push	seg		; Adress of the Data
	push	off
	push	len		; and their length 
	push	segcl		; Adresse of CBC low
	push	offcl
	push	segch		; and CBC high
	push	offch
	callf	Blowfish_CBCDecrypt
endm

