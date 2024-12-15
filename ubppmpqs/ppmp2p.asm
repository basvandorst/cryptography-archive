;PPMP2P.ASM 2.0
;	MACHINE LANGUAGE SUBROUTINES
;	FOR PPMPQS
;	PROTECT MODE part
;	1991/92 by YUJI KIDA
;
.386P
pmode	segment para public use32
	assume	cs:pmode,ds:pmode

	include	ppmp.h

	org	100h
start:

pmodesetR1R2:
	mov	ax,10h
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax

	; loop for primes

	mov	esi,primeadr2+2*primeunitbytes	;skip sign & 2
	mov	ecx,ds:[_primes]
	sub	ecx,2			;skip sign & 2
	sub	ecx,ds:[_primes3B]
	sub	ecx,ds:[_primes4]
	mov	ebx,2
align 4
R1R2loop:
	push	ecx
	movzx	eax,word ptr [esi]
	add	ebx,eax

	call	setparam
	add	esi,primeunitbytes
	pop	ecx
	loop	R1R2loop

	mov	ecx,ds:[_primes3B]
	add	ecx,ds:[_primes4]
	jecxz	short R1R2out
	mov	esi,primeadr
align 4
R1R2loopB:
	push	ecx
	movzx	eax,word ptr [esi]
	add	ebx,eax

	call	setparam
	add	esi,primeunitbytes
	pop	ecx
	loop	R1R2loopB

R1R2out:
	; return to real mode

	db	0eah
	dd	104h		;32 bit offset
	dw	18h		;gdt_code16 - gdttbl

	;data structure
	; prime diff	2bytes
	; log(P) 	2bytes(higher=0)
	; sqrt(WN) @ P	4bytes
	; start1	4 bytes
	; start2	4 bytes
	; ( total	10h bytes=primeunitbytes)

	; calc parameters for each prime
setparam:
	push	esi
	mov	esi,_D+12

	xor	edx,edx
	std
	rept	3
	  lodsd
	  div	ebx
	endm
	cld

	mov	eax,edx
	shl	eax,1
	mul	edx
	div	ebx
	mov	eax,edx

	call	ax_modinv_bx

	mov	ds:[_MIA2P],eax	;set 1/(2A) @ P

	mov	esi,_B+20

	xor	edx,edx
	std
	rept	5
	  lodsd
	  div	ebx
	endm
	cld
;//
	pop	esi
	mov	eax,[esi+4]
	add	edx,eax
	cmp	edx,ebx
	jbe	short setparamj1
	sub	edx,ebx
setparamj1:
	mov	eax,ebx
	sub	eax,edx			;(-B-R)@P

	mul	dword ptr ds:[_MIA2P]
	div	ebx			;edx = (-B-R)@P/2A@P

	mov	eax,edx
	add	eax,ds:[_sievewidth]
	xor	edx,edx
	div	ebx
	mov	[esi+8],edx		;((-B-R)@P/2A+M)@P

	mov	eax,[esi+4]
	shl	eax,1
	mul	dword ptr ds:[_MIA2P]	;2R/2A
	div	ebx			;edx = 2R/2A@P
	add	edx,[esi+8]
	cmp	edx,ebx
	jb	short setparamj3
	sub	edx,ebx
setparamj3:
	add	edx,ds:[_sievetop]
	mov	[esi+12],edx		;((-B+R)@P/2A+M)@P
	mov	eax,[esi+8]
	add	eax,ds:[_sievetop]
	mov	[esi+8],eax
	ret


	org	280h

	; * sieve process

pmodesieve:
	mov	ax,10h
	mov	ds,ax
	mov	es,ax

	; set initial value

	push	ebp
	mov	ebp,ds:[_sieveBsize]

	mov	edi,ds:[_sievetop]
	mov	ecx,ebp
	shr	ecx,2
	inc	ecx
	mov	eax,ds:[_inilog]
	rep	stosd

	mov	edx,ds:[_sieveover]

	; sieve main1 start

	mov	esi,primeadr2+2*primeunitbytes	;skip sign & 2
	mov	ecx,ds:[_primes]
	sub	ecx,ds:[_primes1]	;cut primes > sieveXsize
	sub	ecx,ds:[_primes2]	;cut primes > sieveDsize
	sub	ecx,ds:[_primes3A]	;cut primes > sieveWsize
	sub	ecx,ds:[_primes3B]	;cut primes > sieveWsize
	sub	ecx,ds:[_primes4]	;cut primes > sieveBsize
	sub	ecx,2			;skip sign & 2
	mov	ebx,2
align 4
primeloop:
	push	ecx
	movzx	eax,word ptr [esi]
	add	ebx,eax

	mov	eax,ebx
	shl	eax,sieveRepLog
	sub	eax,ebx
	mov	ecx,edx
	sub	ecx,eax		;ecx = sieveover - (sieveRep-1)*prime

	mov	eax,esi			;push	esi
	shl	eax,8			;

	mov	al,[esi+2]	;log

	mov	edi,[esi+8]
	mov	esi,[esi+12]
	cmp	esi,edi
	jbe	short setlogp10
	xchg	esi,edi
setlogp10:
	sub	esi,edi

align 4
setlogplp1:
  rept	sieveRep
	sub	[edi],al
	sub	[edi+esi],al
	add	edi,ebx
  endm
	cmp	edi,ecx
	jb	setlogplp1

  rept	sieveRep-1
	cmp	edi,edx
	jae	short setlogp30
	sub	[edi],al
	sub	[edi+esi],al
	add	edi,ebx
  endm
setlogp30:
	add	esi,edi
	cmp	esi,edx
	jae	short setlogp40
	sub	[esi],al
	add	esi,ebx
setlogp40:
	xchg	eax,esi			;pop	esi
	shr	esi,8			;

	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+8],edi
	sub	eax,ebp
	mov	[esi+12],eax

	add	esi,primeunitbytes
	pop	ecx
	dec	ecx
	jnz	primeloop

	; sieve main1 end

	; sieve main2 start
	; 4 or 8 times

	mov	ecx,ds:[_primes1]
	or	ecx,ecx
	jz	sievemain3
align 4
primeloop2:
	movzx	eax,word ptr [esi]
	add	ebx,eax

	mov	eax,esi			;push	esi
	shl	eax,8			;

	mov	al,[esi+2]		;log

	mov	edi,[esi+8]
	mov	esi,[esi+12]
	cmp	esi,edi
	jbe	short setlogp100
	xchg	esi,edi
setlogp100:
	sub	esi,edi
  rept	4
	sub	[edi],al
	sub	[edi+esi],al
	add	edi,ebx
  endm
  rept	4
	cmp	edi,edx
	jae	short setlogp130
	sub	[edi],al
	sub	[edi+esi],al
	add	edi,ebx
  endm

setlogp130:
	add	esi,edi
	cmp	esi,edx
	jae	short setlogp140
	sub	[esi],al
	add	esi,ebx

setlogp140:
	xchg	eax,esi			;pop	esi
	shr	esi,8			;

	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+8],edi
	sub	eax,ebp
	mov	[esi+12],eax

	add	esi,primeunitbytes
	dec	ecx
	jnz	primeloop2

	; sieve main2 end

	; sieve main3 start
	; 2 or 4 times

align	4
sievemain3:
	mov	ecx,ds:[_primes2]
	jecxz	short sievemain4
align 4
primeloop3:
	movzx	eax,word ptr [esi]
	add	ebx,eax

	mov	al,[esi+2]		;log
	mov	edi,[esi+8]

  rept	2
	sub	[edi],al
	add	edi,ebx
	endm
  rept	2
	cmp	edi,edx
	jae	short setlogp310
	sub	[edi],al
	add	edi,ebx
  endm

setlogp310:
	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+8],edi

	mov	edi,[esi+12]
  rept	2
	sub	[edi],al
	add	edi,ebx
  endm
  rept	2
	cmp	edi,edx
	jae	short setlogp320
	sub	[edi],al
	add	edi,ebx
  endm

setlogp320:
	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+12],edi

	add	esi,primeunitbytes
	loop	primeloop3

	; sieve main3 end

	; sieve main4 start
	; 1 or 2 times

align 4
sievemain4:
	mov	ecx,ds:[_primes3A]
	jecxz	short sievemain4B
align 4
primeloop4:
	movzx	eax,word ptr [esi]
	add	ebx,eax

	mov	al,[esi+2]		;log
	mov	edi,[esi+8]
	sub	[edi],al
	add	edi,ebx
	cmp	edi,edx
	jae	short setlogp410
	sub	[edi],al
	add	edi,ebx
align	4
setlogp410:
	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+8],edi

	mov	edi,[esi+12]
	sub	[edi],al
	add	edi,ebx
	cmp	edi,edx
	jae	short setlogp420
	sub	[edi],al
	add	edi,ebx
align	4
setlogp420:
	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+12],edi

	add	esi,primeunitbytes
	loop	primeloop4

align 4
sievemain4B:
	mov	ecx,ds:[_primes3B]
	jecxz	short sievemain5
	mov	esi,primeadr
align 4
primeloop4B:
	movzx	eax,word ptr [esi]
	add	ebx,eax

	mov	al,[esi+2]		;log
	mov	edi,[esi+8]
	sub	[edi],al
	add	edi,ebx
	cmp	edi,edx
	jae	short setlogp410B
	sub	[edi],al
	add	edi,ebx
align	4
setlogp410B:
	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+8],edi

	mov	edi,[esi+12]
	sub	[edi],al
	add	edi,ebx
	cmp	edi,edx
	jae	short setlogp420B
	sub	[edi],al
	add	edi,ebx
align	4
setlogp420B:
	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+12],edi

	add	esi,primeunitbytes
	loop	primeloop4B

	; sieve main4 end

	; sieve main5 start
	; 0 or 1 time

align	4
sievemain5:
	mov	ecx,ds:[_primes4]
	jecxz	short sievedone
	cmp	esi,primeadr
	jae	short primeloop5
	mov	esi,primeadr
align 4
primeloop5:
	movzx	eax,word ptr [esi]
	add	ebx,eax

	mov	al,[esi+2]		;log
	mov	edi,[esi+8]
	cmp	edi,edx
	jae	short setlogp510
	sub	[edi],al
	add	edi,ebx
align	4
setlogp510:
	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+8],edi

	mov	edi,[esi+12]
	cmp	edi,edx
	jae	short setlogp520
	sub	[edi],al
	add	edi,ebx
align	4
setlogp520:
	sub	edi,ebp			;ds:[_sieveBsize]
	mov	[esi+12],edi

	add	esi,primeunitbytes
	loop	primeloop5

	; sieve main5 end

align	4
sievedone:
	mov	al,ds:[_cutlog]
	mov	edx,ds:[_sievetop]

	mov	ebx,sieveansarea
	mov	word ptr [ebx],0
	mov	esi,sieveansarea+4
	mov	edi,edx			;ds:[_sievetop]
	mov	ecx,ebp			;ds:[_sieveBsize]
align 4
sieveanslp:
	scasb
	ja	short sieveansfind
sieveansnext:
	loop	sieveanslp

	; return to real mode
sieveansret:
	pop	ebp

	db	0eah
	dd	104h		;32 bit offset
	dw	18h		;gdt_code16 - gdttbl


sieveansfind:
	sub	edi,edx		;ds:[_sievetop]
	dec	edi
	mov	[esi],edi	;result
	add	edi,edx		;ds:[_sievetop]
	inc	edi
	add	esi,4
	inc	word ptr [ebx]	;# of results
	jmp	sieveansnext



	org	700h
decompose:
	mov	ax,10h
	mov	ds,ax
	mov	es,ax

	push	ebp

	movzx	eax,word ptr ds:[_absQ+2]
	shl	eax,4
	movzx	esi,word ptr ds:[_absQ]
	add	esi,eax		;absolute adr of W#
	push	esi		;*

	mov	ebx,_W
	mov	edi,ebx		;set in _W by dword format
	xor	eax,eax
	lodsw
	mov	ecx,eax
	inc	eax
	shr	eax,1
	stosd
	rep	movsw
	xor	eax,eax
	stosw

	; divide by p=2
decomp2:
	cmp	dword ptr [ebx+4],0
	jne	short decomp4
	mov	ecx,[ebx]
	dec	ecx
	mov	[ebx],ecx
	lea	edi,[ebx+4]
	lea	esi,[edi+4]
	rep	movsd
	jmp	decomp2
decomp4:
	std

	mov	eax,[ebx+4]
	mov	ebp,1
decomp5:
	shl	ebp,1
	shr	eax,1
	jnc	decomp5
	shr	ebp,1
	cmp	ebp,1
	je	short decomp8

	mov	esi,ebx
	mov	eax,[esi]
	mov	ecx,eax
	shl	eax,2
	add	esi,eax		;highest dword adr
	xor	edx,edx
	lodsd
	div	ebp
	push	eax		;new highest value
	jmp	short decomp7
align 4
decomp6:
	lodsd
	div	ebp
decomp7:
	mov	[esi+4],eax
	loop	decomp6
	pop	eax
	or	eax,eax
	jnz	short decomp75
	dec	dword ptr [ebx]
decomp75:
	mov	eax,[ebx]
	or	eax,[ebx+4]
	dec	eax
	jz	decompout	;if result=1

	; divide by odd primes
decomp8:
	mov	eax,ds:[_sieveConst]
	sub	eax,ds:[_result]
	mov	ds:[_result],ebx
	mov	ebx,eax

	mov	edi,primeadr2+2*primeunitbytes	;skip sign & 2
	mov	ecx,ds:[_primes]
	sub	ecx,2			;skip sign & 2
	sub	ecx,ds:[_primes3B]
	sub	ecx,ds:[_primes4]
  	mov	ebp,2
decomp10:
	cmp	edi,primeadr
	movzx	eax,word ptr [edi]
	add	ebp,eax

	xor	edx,edx
	mov	eax,[edi+8]
	add	eax,ebx
	div	ebp
	or	edx,edx
	jz	godecomp	;divide exactly

	xor	edx,edx
	mov	eax,[edi+12]
	add	eax,ebx
	div	ebp
	or	edx,edx
	jz	godecomp	;divide exactly
decomp40:
	add	edi,primeunitbytes
	loop	decomp10

	;
	
	mov	ecx,ds:[_primes3B]
	add	ecx,ds:[_primes4]
	jecxz	short decomp100
	mov	edi,primeadr
decomp10B:
	cmp	edi,primeadr
	movzx	eax,word ptr [edi]
	add	ebp,eax

	xor	edx,edx
	mov	eax,[edi+8]
	add	eax,ebx
	div	ebp
	or	edx,edx
	jz	godecompB	;divide exactly

	xor	edx,edx
	mov	eax,[edi+12]
	add	eax,ebx
	div	ebp
	or	edx,edx
	jz	godecompB	;divide exactly
decomp40B:
	add	edi,primeunitbytes
	loop	decomp10B
	
decomp100:
	mov	ebx,ds:[_result]

decompout:
	cld

	mov	esi,ebx
	pop	ebx		;*transfer to 16bit format
	mov	edi,ebx
	lodsd
	shl	eax,1
	mov	ecx,eax		;word length
	stosw
	rep	movsw
	cmp	word ptr [edi-2],0
	jne	short decomp50		;if highest not 0
	dec	word ptr [ebx]
decomp50:
	pop	ebp

	; return to real mode

	db	0eah
	dd	104h		;32 bit offset
	dw	18h		;gdt_code16 - gdttbl


decomp90:
	mov	esi,ebx
	mov	eax,[esi]
	mov	ecx,eax
	shl	eax,2
	add	esi,eax		;highest dword adr
	xor	edx,edx

  rept	ansDword
	lodsd
	div	ebp
	dec	ecx
	jz	short decomp96
  endm

decomp96:
	or	edx,edx
	jz	short decomp200
	pop	ecx
	pop	ebx
	jmp	decomp40
align 4
godecomp:
	push	ebx
	push	ecx
	mov	ebx,ds:[_result]
decomp200:
	mov	esi,ebx
	mov	eax,[esi]
	mov	ecx,eax
	shl	eax,2
	add	esi,eax		;highest dword adr
	xor	edx,edx
	lodsd
	div	ebp
	push	eax		;new highest value

	mov	[esi+4],eax
	dec	ecx
	jz	short decomp125

  rept	ansDword
	lodsd
	div	ebp
	mov	[esi+4],eax
	dec	ecx
	jz	short decomp125
  endm

decomp125:
	pop	eax
	or	eax,eax
	jnz	short decomp130	;if same length
	dec	dword ptr [ebx]
decomp130:
	mov	eax,[ebx]
	or	eax,[ebx+4]
	dec	eax
	jz	short decomp140	;if result=1
	cmp	ebp,4096
	jbe	decomp90	;check divisible ^2,^3 ?
	pop	ecx
	pop	ebx
	jmp	decomp40
decomp140:
	pop	eax		;dummy
	pop	eax		;dummy
	jmp	decompout	;if result=1

decomp90B:
	mov	esi,ebx
	mov	eax,[esi]
	mov	ecx,eax
	shl	eax,2
	add	esi,eax		;highest dword adr
	xor	edx,edx

  rept	ansDword
	lodsd
	div	ebp
	dec	ecx
	jz	short decomp96B
  endm

decomp96B:
	or	edx,edx
	jz	short decomp200B
	pop	ecx
	pop	ebx
	jmp	decomp40B
align 4
godecompB:
	push	ebx
	push	ecx
	mov	ebx,ds:[_result]
decomp200B:
	mov	esi,ebx
	mov	eax,[esi]
	mov	ecx,eax
	shl	eax,2
	add	esi,eax		;highest dword adr
	xor	edx,edx
	lodsd
	div	ebp
	push	eax		;new highest value

	mov	[esi+4],eax
	dec	ecx
	jz	short decomp125B

  rept	ansDword
	lodsd
	div	ebp
	mov	[esi+4],eax
	dec	ecx
	jz	short decomp125B
  endm

decomp125B:
	pop	eax
	or	eax,eax
	jnz	short decomp130B	;if same length
	dec	dword ptr [ebx]
decomp130B:
	mov	eax,[ebx]
	or	eax,[ebx+4]
	dec	eax
	jz	short decomp140B	;if result=1
	cmp	ebp,4096
	jbe	decomp90B	;check divisible ^2,^3 ?
	pop	ecx
	pop	ebx
	jmp	decomp40B
decomp140B:
	pop	eax		;dummy
	pop	eax		;dummy
	jmp	decompout	;if result=1

;
;  long integer @ 32bit integer
;  edx = [esi] @ ebx
;  [esi] > 0 must not be 0
; destroy : eax,ecx,edx

longmod_bx:
	mov	eax,[esi]
	mov	ecx,eax
	shl	eax,2
	add	esi,eax
	xor	edx,edx
	std
	cmp	ecx,5
	je	short longmod50
	cmp	ecx,6
	je	short longmod60
	cmp	ecx,4
	jz	short longmod40
	dec	ecx
	jz	short longmod10
	dec	ecx
	jz	short longmod20
	dec	ecx
	jz	short longmod30
	jmp	short longmodret
longmod60:
	lodsd
	div	ebx
longmod50:
	lodsd
	div	ebx
longmod40:
	lodsd
	div	ebx
longmod30:
	lodsd
	div	ebx
longmod20:
	lodsd
	div	ebx
longmod10:
	lodsd
	div	ebx
longmodret:
	cld
	ret

;
; inverse modulo prime
; 
; inp : eax
; out : eax = 1/eax mod ebx
; destroy : ecx,esi,edi

ax_modinv_bx:
	cmp	eax,1
	je	short modinvret
	mov	ecx,ebx
	cmp	eax,ebx
	jb	short modinv10
	xor	edx,edx
	div	ebx
	mov	eax,edx
	cmp	eax,1
	je	short modinvret
modinv10:
	xchg	eax,ecx
	xor	esi,esi		;coef1
	mov	edi,1		;coef2
modinv20:
	xor	edx,edx
	div	ecx
	shl	edx,1
	cmp	edx,ecx
	jbe	short modinv60

	shr	edx,1
	sub	edx,ecx
	neg	edx
	push	edx		;new remainder
	inc	eax		;inc quotient

	mul	edi
	div	ebx
	mov	eax,edx
	sub	eax,esi
	jae	short modinv30
	add	eax,ebx
modinv30:
	mov	esi,edi		;new coef1=old coef2
	mov	edi,eax		;new coef2=quotient*coef2-coef1

	mov	eax,ecx
	pop	ecx
	cmp	ecx,1
	jne	modinv20	;GCD must 1 otherwise endlessloop
	mov	eax,edi
modinvret:
	ret
	
modinv60:
	shr	edx,1
	push	edx		;remainder

	mul	edi
	div	ebx
	mov	eax,esi
	sub	eax,edx
	jae	short modinv70
	add	eax,ebx
modinv70:
	mov	esi,edi		;new coef1=old coef2
	mov	edi,eax		;new coef2=coef1-quotient*coef2

	mov	eax,ecx
	pop	ecx
	cmp	ecx,1
	jne	modinv20	;GCD must 1 otherwise endlessloop
	mov	eax,edi
	ret

pmode	ends


end	start
