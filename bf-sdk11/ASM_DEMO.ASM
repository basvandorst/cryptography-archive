
;       ASM_DEMO.ASM
;       Assembler Test- and Demoprogram of the Compatibility 
;       (c)1996 Cedric Reinartz

; 	Simple programm which encrypts testdata and compares them with the
;	values provided by Bruce Schneier



	nopub	 equ 1			; we want to include BFENG386.ASM
	useSmall equ 1			; large model not needed
	noLoop	 equ 0			; we don't need much speed
	rnds	 equ 1			; 16 Rounds

	include bfe_asm.inc

	.data

key_0	dd 1058 dup(0)

;   official Testvektors from DDJ 10/95... 
PW_1		db "abcdefghijklmnopqrstuvwxyz"
data1_p 	dd 0424c4f57h, 046495348h
data1_c 	dd 0324ed0feh, 0f413a203h
len_PW_1 	equ offset data1_p - offset pw_1

PW_2		db "Who is John Galt?"
data2_p 	dd 0fedcba98h, 076543210h
data2_c 	dd 0cc91732bh, 08022f684h
len_PW_2	equ offset data2_p - PW_2

noerror		db 13,10,10,"Test completed successfully !",13,10,"$"
error		db 13,10,10,"Test failed !",13,10,"$"

	.code	

start:	_GetBoxPointer				; mov dx,seg key_0 would also do
	mov	ds,dx				; load the right Datasegment

	_GetBoxes ds, <offset key_0>		; Save original Boxes

	_InitCrypt ds,<offset PW_1>,len_PW_1	; Set the first Password
	_Encrypt ds,<offset data1_p>,8		; Encrypt 8 Byte of Testdata
	mov	si,offset data1_p		; Compare with
	call	comp				; official values
	jne	ko				; Not equal -> Test Failed

	_SetBoxes ds, <offset key_0>		; Restore original Boxes

	_InitCrypt ds,<offset PW_2>,len_PW_2	; Set the second Password
	_Encrypt ds,<offset data2_p>,8		; Encrypt 8 Byte of Testdata
	mov	si,offset data2_p		; Compare with
	call	comp				; official values
	jne	ko				; Not equal -> Test Failed

OK:	mov	dx,offset noerror		; Else: Test completed
	jmp	finish

KO:	mov	dx,offset error

finish:	mov	ax,0900h			; Show final message
	int	21h
	mov	ax,4c00h			; Terminate programm
	int	21h

comp:	mov	eax,[si]			; Compares two DWords
	cmp	eax,[si+8]			; with the following
	jne	c_end				; two DWords
	mov	eax,[si+4]
	cmp	eax,[si+12]
c_end:	ret

	end start

