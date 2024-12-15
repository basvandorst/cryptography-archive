	text
	align	 16
	global	 _mpn_mul_1
_mpn_mul_1:
	subu	 r31,r31,16
	st	 r25,r31,4
	st	 r26,r31,8
	subu	 r4,r0,r4
	mak	 r13,r4,0<2>
	subu	 r3,r3,r13
	subu	 r6,r2,r13
	or	 r2,r0,0
	mask	 r7,r5,0xffff		; r7 = lo(s2_limb)
	extu	 r11,r5,0<16>		; r11 = hi(s2_limb)
	ld	 r25,r3[r4]
	br.n	 L1
	 subu	 r6,r6,4

Loop:
	ld	 r25,r3[r4]
	st	 r26,r6[r4]
L1:	addu	 r4,r4,1
	mul	 r26,r25,r5		;			mul_1	WB ld
	mask	 r12,r25,0xffff		; r12 = lo(s1_limb)	mask_1	
	mul	 r8,r12,r7		; r8 =  prod_0		mul_2	WB mask_1
	mul	 r10,r12,r11		; r10 = prod_1a		mul_3	
	extu	 r13,r25,0<16>		; r13 = hi(s1_limb)	extu_1	WB mul_1
	mul	 r12,r13,r7		; r12 = prod_1b		mul_4	WB extu_1
	mul	 r9,r13,r11		; r9  = prod_2		mul_5	WB mul_2
	extu	 r8,r8,0<16>		; r8 = hi(prod_0)	extu_2	WB mul_3
	addu	 r10,r10,r8		;			addu_1	WB extu_2
; bcnd	ne0,r0,0			;				WB addu_1
	addu.co	 r10,r10,r12		;				WB mul_5
	mask.u	 r10,r10,0xffff
	addu.ci	 r10,r10,r0
	rot	 r10,r10,16
	addu.co	 r26,r26,r2
	bcnd.n	 ne0,r4,Loop
	addu.ci	 r2,r9,r10

	st	 r26,r6[r4]
	ld	 r25,r31,4
	ld	 r26,r31,8
	jmp.n	 r1
	addu	 r31,r31,16
