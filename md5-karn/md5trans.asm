	ifndef	??version
?debug	macro
	endm
publicdll macro	name
	public	name
	endm
$comm	macro	name,dist,size,count
	comm	dist name:BYTE:count*size
	endm
	else
$comm	macro	name,dist,size,count
	comm	dist name[size]:BYTE:count
	endm
	endif
	?debug	S "mymd5.c"
	?debug	C E9CD355818076D796D64352E63
_TEXT	segment byte public 'CODE'
_TEXT	ends
DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
_BSS	ends
_DATA	segment word public 'DATA'
		.MODEL SMALL
_DATA	ends
_TEXT	segment byte public 'CODE'
   ;	
   ;	void
   ;	
	assume	cs:_TEXT
_Transform	proc	near
	push	bp
	mov	bp,sp
	push	si
	push	di
   ;	
   ;	Transform(buf,input)
   ;	long *buf;
   ;	long *input;
   ;	{
   ;	
		.386
   ;	
   ;		asm	.386;	/* Allow use of 32-bit general registers */
   ;	
   ;		/* Save caller's registers */
   ;	
		push	 si
   ;	
   ;		asm	push si;
   ;	
		push	 di
   ;	
   ;		asm	push di;
   ;	
		if	 @DataSize NE 0
   ;	
   ;		asm	if @DataSize NE 0
   ;	
			push	 ds
   ;	
   ;		asm		push ds;
   ;	
		endif	
   ;	
   ;		asm	endif
   ;	
   ;		/* Get buf argument */
   ;	
		if	 @DataSize NE 0
   ;	
   ;		asm	if @DataSize NE 0
   ;	
			lds	 si,[bp+4]
   ;	
   ;		asm		lds si,buf;
   ;	
		else	
   ;	
   ;		asm	else
   ;	
			mov	 si,[bp+4]
   ;	
   ;		asm		mov si,buf;
   ;	
		endif	
   ;	
   ;		asm	endif
   ;	
		mov	 eax,dword ptr si[0*4]
   ;	
   ;		asm	mov A,dword ptr si[0*4];	/* A = buf[0] */
   ;	
		mov	 ebx,dword ptr si[1*4]
   ;	
   ;		asm	mov B,dword ptr si[1*4];	/* B = buf[1] */
   ;	
		mov	 ecx,dword ptr si[2*4]
   ;	
   ;		asm	mov C,dword ptr si[2*4];	/* C = buf[2] */
   ;	
		mov	 edx,dword ptr si[3*4]
   ;	
   ;		asm	mov D,dword ptr si[3*4];	/* D = buf[3] */
   ;	
   ;		/* Warning: This makes our other args inaccessible until bp
   ;		 * is restored!
   ;		*/
   ;	
		push	 bp
   ;	
   ;		asm	push bp;
   ;	
		if	 @DataSize NE 0
   ;	
   ;		asm	if @DataSize NE 0
   ;	
			lds	 bp,[bp+6]
   ;	
   ;		asm		lds bp,input
   ;	
		else	
   ;	
   ;		asm	else
   ;	
			mov	 bp,[bp+6]
   ;	
   ;		asm		mov bp,input
   ;	
		endif	
   ;	
   ;		asm	endif
   ;	
   ;	/* Round 1. The *4 factors in the subscripts to bp account for the
   ;	 * byte offsets of each long element in the input array.
   ;	 */
   ;	#define S11 7
   ;	#define S12 12
   ;	#define S13 17
   ;	#define S14 22
   ;	
 	mov	 edi,ebx
 	and	 edi,ecx
 	mov	 esi,ebx
 	not	 esi
 	and	 esi,edx
 	or	 edi,esi
 	add	 eax,edi
 	add	 eax,bp[ 0*4]
 	add	 eax,3614090360
 	rol	 eax,7
 	add	 eax,ebx
   ;	
   ;		FF(A,B,C,D,bp[ 0*4],S11,3614090360); /* 1 */
   ;	
 	mov	 edi,eax
 	and	 edi,ebx
 	mov	 esi,eax
 	not	 esi
 	and	 esi,ecx
 	or	 edi,esi
 	add	 edx,edi
 	add	 edx,bp[ 1*4]
 	add	 edx,3905402710
 	rol	 edx,12
 	add	 edx,eax
   ;	
   ;		FF(D,A,B,C,bp[ 1*4],S12,3905402710); /* 2 */
   ;	
 	mov	 edi,edx
 	and	 edi,eax
 	mov	 esi,edx
 	not	 esi
 	and	 esi,ebx
 	or	 edi,esi
 	add	 ecx,edi
 	add	 ecx,bp[ 2*4]
 	add	 ecx,606105819
 	rol	 ecx,17
 	add	 ecx,edx
   ;	
   ;		FF(C,D,A,B,bp[ 2*4],S13, 606105819); /* 3 */
   ;	
 	mov	 edi,ecx
 	and	 edi,edx
 	mov	 esi,ecx
 	not	 esi
 	and	 esi,eax
 	or	 edi,esi
 	add	 ebx,edi
 	add	 ebx,bp[ 3*4]
 	add	 ebx,3250441966
 	rol	 ebx,22
 	add	 ebx,ecx
   ;	
   ;		FF(B,C,D,A,bp[ 3*4],S14,3250441966); /* 4 */
   ;	
 	mov	 edi,ebx
 	and	 edi,ecx
 	mov	 esi,ebx
 	not	 esi
 	and	 esi,edx
 	or	 edi,esi
 	add	 eax,edi
 	add	 eax,bp[ 4*4]
 	add	 eax,4118548399
 	rol	 eax,7
 	add	 eax,ebx
   ;	
   ;		FF(A,B,C,D,bp[ 4*4],S11,4118548399); /* 5 */
   ;	
 	mov	 edi,eax
 	and	 edi,ebx
 	mov	 esi,eax
 	not	 esi
 	and	 esi,ecx
 	or	 edi,esi
 	add	 edx,edi
 	add	 edx,bp[ 5*4]
 	add	 edx,1200080426
 	rol	 edx,12
 	add	 edx,eax
   ;	
   ;		FF(D,A,B,C,bp[ 5*4],S12,1200080426); /* 6 */
   ;	
 	mov	 edi,edx
 	and	 edi,eax
 	mov	 esi,edx
 	not	 esi
 	and	 esi,ebx
 	or	 edi,esi
 	add	 ecx,edi
 	add	 ecx,bp[ 6*4]
 	add	 ecx,2821735955
 	rol	 ecx,17
 	add	 ecx,edx
   ;	
   ;		FF(C,D,A,B,bp[ 6*4],S13,2821735955); /* 7 */
   ;	
 	mov	 edi,ecx
 	and	 edi,edx
 	mov	 esi,ecx
 	not	 esi
 	and	 esi,eax
 	or	 edi,esi
 	add	 ebx,edi
 	add	 ebx,bp[ 7*4]
 	add	 ebx,4249261313
 	rol	 ebx,22
 	add	 ebx,ecx
   ;	
   ;		FF(B,C,D,A,bp[ 7*4],S14,4249261313); /* 8 */
   ;	
 	mov	 edi,ebx
 	and	 edi,ecx
 	mov	 esi,ebx
 	not	 esi
 	and	 esi,edx
 	or	 edi,esi
 	add	 eax,edi
 	add	 eax,bp[ 8*4]
 	add	 eax,1770035416
 	rol	 eax,7
 	add	 eax,ebx
   ;	
   ;		FF(A,B,C,D,bp[ 8*4],S11,1770035416); /* 9 */
   ;	
 	mov	 edi,eax
 	and	 edi,ebx
 	mov	 esi,eax
 	not	 esi
 	and	 esi,ecx
 	or	 edi,esi
 	add	 edx,edi
 	add	 edx,bp[ 9*4]
 	add	 edx,2336552879
 	rol	 edx,12
 	add	 edx,eax
   ;	
   ;		FF(D,A,B,C,bp[ 9*4],S12,2336552879); /* 10 */
   ;	
 	mov	 edi,edx
 	and	 edi,eax
 	mov	 esi,edx
 	not	 esi
 	and	 esi,ebx
 	or	 edi,esi
 	add	 ecx,edi
 	add	 ecx,bp[10*4]
 	add	 ecx,4294925233
 	rol	 ecx,17
 	add	 ecx,edx
   ;	
   ;		FF(C,D,A,B,bp[10*4],S13,4294925233); /* 11 */
   ;	
 	mov	 edi,ecx
 	and	 edi,edx
 	mov	 esi,ecx
 	not	 esi
 	and	 esi,eax
 	or	 edi,esi
 	add	 ebx,edi
 	add	 ebx,bp[11*4]
 	add	 ebx,2304563134
 	rol	 ebx,22
 	add	 ebx,ecx
   ;	
   ;		FF(B,C,D,A,bp[11*4],S14,2304563134); /* 12 */
   ;	
 	mov	 edi,ebx
 	and	 edi,ecx
 	mov	 esi,ebx
 	not	 esi
 	and	 esi,edx
 	or	 edi,esi
 	add	 eax,edi
 	add	 eax,bp[12*4]
 	add	 eax,1804603682
 	rol	 eax,7
 	add	 eax,ebx
   ;	
   ;		FF(A,B,C,D,bp[12*4],S11,1804603682); /* 13 */
   ;	
 	mov	 edi,eax
 	and	 edi,ebx
 	mov	 esi,eax
 	not	 esi
 	and	 esi,ecx
 	or	 edi,esi
 	add	 edx,edi
 	add	 edx,bp[13*4]
 	add	 edx,4254626195
 	rol	 edx,12
 	add	 edx,eax
   ;	
   ;		FF(D,A,B,C,bp[13*4],S12,4254626195); /* 14 */
   ;	
 	mov	 edi,edx
 	and	 edi,eax
 	mov	 esi,edx
 	not	 esi
 	and	 esi,ebx
 	or	 edi,esi
 	add	 ecx,edi
 	add	 ecx,bp[14*4]
 	add	 ecx,2792965006
 	rol	 ecx,17
 	add	 ecx,edx
   ;	
   ;		FF(C,D,A,B,bp[14*4],S13,2792965006); /* 15 */
   ;	
 	mov	 edi,ecx
 	and	 edi,edx
 	mov	 esi,ecx
 	not	 esi
 	and	 esi,eax
 	or	 edi,esi
 	add	 ebx,edi
 	add	 ebx,bp[15*4]
 	add	 ebx,1236535329
 	rol	 ebx,22
 	add	 ebx,ecx
   ;	
   ;		FF(B,C,D,A,bp[15*4],S14,1236535329); /* 16 */
   ;	
   ;	/* Round 2 */
   ;	#define S21 5
   ;	#define S22 9
   ;	#define S23 14
   ;	#define S24 20
   ;	
 	mov	 edi,edx
 	and	 edi,ebx
 	mov	 esi,edx
 	not	 esi
 	and	 esi,ecx
 	or	 edi,esi
 	add	 eax,edi
 	add	 eax,bp[ 1*4]
 	add	 eax,4129170786
 	rol	 eax,5
 	add	 eax,ebx
   ;	
   ;		GG(A,B,C,D,bp[ 1*4],S21,4129170786); /* 17 */
   ;	
 	mov	 edi,ecx
 	and	 edi,eax
 	mov	 esi,ecx
 	not	 esi
 	and	 esi,ebx
 	or	 edi,esi
 	add	 edx,edi
 	add	 edx,bp[ 6*4]
 	add	 edx,3225465664
 	rol	 edx,9
 	add	 edx,eax
   ;	
   ;		GG(D,A,B,C,bp[ 6*4],S22,3225465664); /* 18 */
   ;	
 	mov	 edi,ebx
 	and	 edi,edx
 	mov	 esi,ebx
 	not	 esi
 	and	 esi,eax
 	or	 edi,esi
 	add	 ecx,edi
 	add	 ecx,bp[11*4]
 	add	 ecx,643717713
 	rol	 ecx,14
 	add	 ecx,edx
   ;	
   ;		GG(C,D,A,B,bp[11*4],S23, 643717713); /* 19 */
   ;	
 	mov	 edi,eax
 	and	 edi,ecx
 	mov	 esi,eax
 	not	 esi
 	and	 esi,edx
 	or	 edi,esi
 	add	 ebx,edi
 	add	 ebx,bp[ 0*4]
 	add	 ebx,3921069994
 	rol	 ebx,20
 	add	 ebx,ecx
   ;	
   ;		GG(B,C,D,A,bp[ 0*4],S24,3921069994); /* 20 */
   ;	
 	mov	 edi,edx
 	and	 edi,ebx
 	mov	 esi,edx
 	not	 esi
 	and	 esi,ecx
 	or	 edi,esi
 	add	 eax,edi
 	add	 eax,bp[ 5*4]
 	add	 eax,3593408605
 	rol	 eax,5
 	add	 eax,ebx
   ;	
   ;		GG(A,B,C,D,bp[ 5*4],S21,3593408605); /* 21 */
   ;	
 	mov	 edi,ecx
 	and	 edi,eax
 	mov	 esi,ecx
 	not	 esi
 	and	 esi,ebx
 	or	 edi,esi
 	add	 edx,edi
 	add	 edx,bp[10*4]
 	add	 edx,38016083
 	rol	 edx,9
 	add	 edx,eax
   ;	
   ;		GG(D,A,B,C,bp[10*4],S22,  38016083); /* 22 */
   ;	
 	mov	 edi,ebx
 	and	 edi,edx
 	mov	 esi,ebx
 	not	 esi
 	and	 esi,eax
 	or	 edi,esi
 	add	 ecx,edi
 	add	 ecx,bp[15*4]
 	add	 ecx,3634488961
 	rol	 ecx,14
 	add	 ecx,edx
   ;	
   ;		GG(C,D,A,B,bp[15*4],S23,3634488961); /* 23 */
   ;	
 	mov	 edi,eax
 	and	 edi,ecx
 	mov	 esi,eax
 	not	 esi
 	and	 esi,edx
 	or	 edi,esi
 	add	 ebx,edi
 	add	 ebx,bp[ 4*4]
 	add	 ebx,3889429448
 	rol	 ebx,20
 	add	 ebx,ecx
   ;	
   ;		GG(B,C,D,A,bp[ 4*4],S24,3889429448); /* 24 */
   ;	
 	mov	 edi,edx
 	and	 edi,ebx
 	mov	 esi,edx
 	not	 esi
 	and	 esi,ecx
 	or	 edi,esi
 	add	 eax,edi
 	add	 eax,bp[ 9*4]
 	add	 eax,568446438
 	rol	 eax,5
 	add	 eax,ebx
   ;	
   ;		GG(A,B,C,D,bp[ 9*4],S21, 568446438); /* 25 */
   ;	
 	mov	 edi,ecx
 	and	 edi,eax
 	mov	 esi,ecx
 	not	 esi
 	and	 esi,ebx
 	or	 edi,esi
 	add	 edx,edi
 	add	 edx,bp[14*4]
 	add	 edx,3275163606
 	rol	 edx,9
 	add	 edx,eax
   ;	
   ;		GG(D,A,B,C,bp[14*4],S22,3275163606); /* 26 */
   ;	
 	mov	 edi,ebx
 	and	 edi,edx
 	mov	 esi,ebx
 	not	 esi
 	and	 esi,eax
 	or	 edi,esi
 	add	 ecx,edi
 	add	 ecx,bp[ 3*4]
 	add	 ecx,4107603335
 	rol	 ecx,14
 	add	 ecx,edx
   ;	
   ;		GG(C,D,A,B,bp[ 3*4],S23,4107603335); /* 27 */
   ;	
 	mov	 edi,eax
 	and	 edi,ecx
 	mov	 esi,eax
 	not	 esi
 	and	 esi,edx
 	or	 edi,esi
 	add	 ebx,edi
 	add	 ebx,bp[ 8*4]
 	add	 ebx,1163531501
 	rol	 ebx,20
 	add	 ebx,ecx
   ;	
   ;		GG(B,C,D,A,bp[ 8*4],S24,1163531501); /* 28 */
   ;	
 	mov	 edi,edx
 	and	 edi,ebx
 	mov	 esi,edx
 	not	 esi
 	and	 esi,ecx
 	or	 edi,esi
 	add	 eax,edi
 	add	 eax,bp[13*4]
 	add	 eax,2850285829
 	rol	 eax,5
 	add	 eax,ebx
   ;	
   ;		GG(A,B,C,D,bp[13*4],S21,2850285829); /* 29 */
   ;	
 	mov	 edi,ecx
 	and	 edi,eax
 	mov	 esi,ecx
 	not	 esi
 	and	 esi,ebx
 	or	 edi,esi
 	add	 edx,edi
 	add	 edx,bp[ 2*4]
 	add	 edx,4243563512
 	rol	 edx,9
 	add	 edx,eax
   ;	
   ;		GG(D,A,B,C,bp[ 2*4],S22,4243563512); /* 30 */
   ;	
 	mov	 edi,ebx
 	and	 edi,edx
 	mov	 esi,ebx
 	not	 esi
 	and	 esi,eax
 	or	 edi,esi
 	add	 ecx,edi
 	add	 ecx,bp[ 7*4]
 	add	 ecx,1735328473
 	rol	 ecx,14
 	add	 ecx,edx
   ;	
   ;		GG(C,D,A,B,bp[ 7*4],S23,1735328473); /* 31 */
   ;	
 	mov	 edi,eax
 	and	 edi,ecx
 	mov	 esi,eax
 	not	 esi
 	and	 esi,edx
 	or	 edi,esi
 	add	 ebx,edi
 	add	 ebx,bp[12*4]
 	add	 ebx,2368359562
 	rol	 ebx,20
 	add	 ebx,ecx
   ;	
   ;		GG(B,C,D,A,bp[12*4],S24,2368359562); /* 32 */
   ;	
   ;	/* Round 3 */
   ;	#define S31 4
   ;	#define S32 11
   ;	#define S33 16
   ;	#define S34 23
   ;	
 	mov	 edi,ebx
 	xor	 edi,ecx
 	xor	 edi,edx
 	add	 eax,edi
 	add	 eax,bp[ 5*4]
 	add	 eax,4294588738
 	rol	 eax,4
 	add	 eax,ebx
   ;	
   ;		HH(A,B,C,D,bp[ 5*4],S31,4294588738); /* 33 */
   ;	
 	mov	 edi,eax
 	xor	 edi,ebx
 	xor	 edi,ecx
 	add	 edx,edi
 	add	 edx,bp[ 8*4]
 	add	 edx,2272392833
 	rol	 edx,11
 	add	 edx,eax
   ;	
   ;		HH(D,A,B,C,bp[ 8*4],S32,2272392833); /* 34 */
   ;	
 	mov	 edi,edx
 	xor	 edi,eax
 	xor	 edi,ebx
 	add	 ecx,edi
 	add	 ecx,bp[11*4]
 	add	 ecx,1839030562
 	rol	 ecx,16
 	add	 ecx,edx
   ;	
   ;		HH(C,D,A,B,bp[11*4],S33,1839030562); /* 35 */
   ;	
 	mov	 edi,ecx
 	xor	 edi,edx
 	xor	 edi,eax
 	add	 ebx,edi
 	add	 ebx,bp[14*4]
 	add	 ebx,4259657740
 	rol	 ebx,23
 	add	 ebx,ecx
   ;	
   ;		HH(B,C,D,A,bp[14*4],S34,4259657740); /* 36 */
   ;	
 	mov	 edi,ebx
 	xor	 edi,ecx
 	xor	 edi,edx
 	add	 eax,edi
 	add	 eax,bp[ 1*4]
 	add	 eax,2763975236
 	rol	 eax,4
 	add	 eax,ebx
   ;	
   ;		HH(A,B,C,D,bp[ 1*4],S31,2763975236); /* 37 */
   ;	
 	mov	 edi,eax
 	xor	 edi,ebx
 	xor	 edi,ecx
 	add	 edx,edi
 	add	 edx,bp[ 4*4]
 	add	 edx,1272893353
 	rol	 edx,11
 	add	 edx,eax
   ;	
   ;		HH(D,A,B,C,bp[ 4*4],S32,1272893353); /* 38 */
   ;	
 	mov	 edi,edx
 	xor	 edi,eax
 	xor	 edi,ebx
 	add	 ecx,edi
 	add	 ecx,bp[ 7*4]
 	add	 ecx,4139469664
 	rol	 ecx,16
 	add	 ecx,edx
   ;	
   ;		HH(C,D,A,B,bp[ 7*4],S33,4139469664); /* 39 */
   ;	
 	mov	 edi,ecx
 	xor	 edi,edx
 	xor	 edi,eax
 	add	 ebx,edi
 	add	 ebx,bp[10*4]
 	add	 ebx,3200236656
 	rol	 ebx,23
 	add	 ebx,ecx
   ;	
   ;		HH(B,C,D,A,bp[10*4],S34,3200236656); /* 40 */
   ;	
 	mov	 edi,ebx
 	xor	 edi,ecx
 	xor	 edi,edx
 	add	 eax,edi
 	add	 eax,bp[13*4]
 	add	 eax,681279174
 	rol	 eax,4
 	add	 eax,ebx
   ;	
   ;		HH(A,B,C,D,bp[13*4],S31, 681279174); /* 41 */
   ;	
 	mov	 edi,eax
 	xor	 edi,ebx
 	xor	 edi,ecx
 	add	 edx,edi
 	add	 edx,bp[ 0*4]
 	add	 edx,3936430074
 	rol	 edx,11
 	add	 edx,eax
   ;	
   ;		HH(D,A,B,C,bp[ 0*4],S32,3936430074); /* 42 */
   ;	
 	mov	 edi,edx
 	xor	 edi,eax
 	xor	 edi,ebx
 	add	 ecx,edi
 	add	 ecx,bp[ 3*4]
 	add	 ecx,3572445317
 	rol	 ecx,16
 	add	 ecx,edx
   ;	
   ;		HH(C,D,A,B,bp[ 3*4],S33,3572445317); /* 43 */
   ;	
 	mov	 edi,ecx
 	xor	 edi,edx
 	xor	 edi,eax
 	add	 ebx,edi
 	add	 ebx,bp[ 6*4]
 	add	 ebx,76029189
 	rol	 ebx,23
 	add	 ebx,ecx
   ;	
   ;		HH(B,C,D,A,bp[ 6*4],S34,  76029189); /* 44 */
   ;	
 	mov	 edi,ebx
 	xor	 edi,ecx
 	xor	 edi,edx
 	add	 eax,edi
 	add	 eax,bp[ 9*4]
 	add	 eax,3654602809
 	rol	 eax,4
 	add	 eax,ebx
   ;	
   ;		HH(A,B,C,D,bp[ 9*4],S31,3654602809); /* 45 */
   ;	
 	mov	 edi,eax
 	xor	 edi,ebx
 	xor	 edi,ecx
 	add	 edx,edi
 	add	 edx,bp[12*4]
 	add	 edx,3873151461
 	rol	 edx,11
 	add	 edx,eax
   ;	
   ;		HH(D,A,B,C,bp[12*4],S32,3873151461); /* 46 */
   ;	
 	mov	 edi,edx
 	xor	 edi,eax
 	xor	 edi,ebx
 	add	 ecx,edi
 	add	 ecx,bp[15*4]
 	add	 ecx,530742520
 	rol	 ecx,16
 	add	 ecx,edx
   ;	
   ;		HH(C,D,A,B,bp[15*4],S33, 530742520); /* 47 */
   ;	
 	mov	 edi,ecx
 	xor	 edi,edx
 	xor	 edi,eax
 	add	 ebx,edi
 	add	 ebx,bp[ 2*4]
 	add	 ebx,3299628645
 	rol	 ebx,23
 	add	 ebx,ecx
   ;	
   ;		HH(B,C,D,A,bp[ 2*4],S34,3299628645); /* 48 */
   ;	
   ;	/* Round 4 */
   ;	#define S41 6
   ;	#define S42 10
   ;	#define S43 15
   ;	#define S44 21
   ;	
 	mov	 edi,edx
 	not	 edi
 	or	 edi,ebx
 	xor	 edi,ecx
 	add	 eax,edi
 	add	 eax,bp[ 0*4]
 	add	 eax,4096336452
 	rol	 eax,6
 	add	 eax,ebx
   ;	
   ;		II(A,B,C,D,bp[ 0*4],S41,4096336452); /* 49 */
   ;	
 	mov	 edi,ecx
 	not	 edi
 	or	 edi,eax
 	xor	 edi,ebx
 	add	 edx,edi
 	add	 edx,bp[ 7*4]
 	add	 edx,1126891415
 	rol	 edx,10
 	add	 edx,eax
   ;	
   ;		II(D,A,B,C,bp[ 7*4],S42,1126891415); /* 50 */
   ;	
 	mov	 edi,ebx
 	not	 edi
 	or	 edi,edx
 	xor	 edi,eax
 	add	 ecx,edi
 	add	 ecx,bp[14*4]
 	add	 ecx,2878612391
 	rol	 ecx,15
 	add	 ecx,edx
   ;	
   ;		II(C,D,A,B,bp[14*4],S43,2878612391); /* 51 */
   ;	
 	mov	 edi,eax
 	not	 edi
 	or	 edi,ecx
 	xor	 edi,edx
 	add	 ebx,edi
 	add	 ebx,bp[ 5*4]
 	add	 ebx,4237533241
 	rol	 ebx,21
 	add	 ebx,ecx
   ;	
   ;		II(B,C,D,A,bp[ 5*4],S44,4237533241); /* 52 */
   ;	
 	mov	 edi,edx
 	not	 edi
 	or	 edi,ebx
 	xor	 edi,ecx
 	add	 eax,edi
 	add	 eax,bp[12*4]
 	add	 eax,1700485571
 	rol	 eax,6
 	add	 eax,ebx
   ;	
   ;		II(A,B,C,D,bp[12*4],S41,1700485571); /* 53 */
   ;	
 	mov	 edi,ecx
 	not	 edi
 	or	 edi,eax
 	xor	 edi,ebx
 	add	 edx,edi
 	add	 edx,bp[ 3*4]
 	add	 edx,2399980690
 	rol	 edx,10
 	add	 edx,eax
   ;	
   ;		II(D,A,B,C,bp[ 3*4],S42,2399980690); /* 54 */
   ;	
 	mov	 edi,ebx
 	not	 edi
 	or	 edi,edx
 	xor	 edi,eax
 	add	 ecx,edi
 	add	 ecx,bp[10*4]
 	add	 ecx,4293915773
 	rol	 ecx,15
 	add	 ecx,edx
   ;	
   ;		II(C,D,A,B,bp[10*4],S43,4293915773); /* 55 */
   ;	
 	mov	 edi,eax
 	not	 edi
 	or	 edi,ecx
 	xor	 edi,edx
 	add	 ebx,edi
 	add	 ebx,bp[ 1*4]
 	add	 ebx,2240044497
 	rol	 ebx,21
 	add	 ebx,ecx
   ;	
   ;		II(B,C,D,A,bp[ 1*4],S44,2240044497); /* 56 */
   ;	
 	mov	 edi,edx
 	not	 edi
 	or	 edi,ebx
 	xor	 edi,ecx
 	add	 eax,edi
 	add	 eax,bp[ 8*4]
 	add	 eax,1873313359
 	rol	 eax,6
 	add	 eax,ebx
   ;	
   ;		II(A,B,C,D,bp[ 8*4],S41,1873313359); /* 57 */
   ;	
 	mov	 edi,ecx
 	not	 edi
 	or	 edi,eax
 	xor	 edi,ebx
 	add	 edx,edi
 	add	 edx,bp[15*4]
 	add	 edx,4264355552
 	rol	 edx,10
 	add	 edx,eax
   ;	
   ;		II(D,A,B,C,bp[15*4],S42,4264355552); /* 58 */
   ;	
 	mov	 edi,ebx
 	not	 edi
 	or	 edi,edx
 	xor	 edi,eax
 	add	 ecx,edi
 	add	 ecx,bp[ 6*4]
 	add	 ecx,2734768916
 	rol	 ecx,15
 	add	 ecx,edx
   ;	
   ;		II(C,D,A,B,bp[ 6*4],S43,2734768916); /* 59 */
   ;	
 	mov	 edi,eax
 	not	 edi
 	or	 edi,ecx
 	xor	 edi,edx
 	add	 ebx,edi
 	add	 ebx,bp[13*4]
 	add	 ebx,1309151649
 	rol	 ebx,21
 	add	 ebx,ecx
   ;	
   ;		II(B,C,D,A,bp[13*4],S44,1309151649); /* 60 */
   ;	
 	mov	 edi,edx
 	not	 edi
 	or	 edi,ebx
 	xor	 edi,ecx
 	add	 eax,edi
 	add	 eax,bp[ 4*4]
 	add	 eax,4149444226
 	rol	 eax,6
 	add	 eax,ebx
   ;	
   ;		II(A,B,C,D,bp[ 4*4],S41,4149444226); /* 61 */
   ;	
 	mov	 edi,ecx
 	not	 edi
 	or	 edi,eax
 	xor	 edi,ebx
 	add	 edx,edi
 	add	 edx,bp[11*4]
 	add	 edx,3174756917
 	rol	 edx,10
 	add	 edx,eax
   ;	
   ;		II(D,A,B,C,bp[11*4],S42,3174756917); /* 62 */
   ;	
 	mov	 edi,ebx
 	not	 edi
 	or	 edi,edx
 	xor	 edi,eax
 	add	 ecx,edi
 	add	 ecx,bp[ 2*4]
 	add	 ecx,718787259
 	rol	 ecx,15
 	add	 ecx,edx
   ;	
   ;		II(C,D,A,B,bp[ 2*4],S43, 718787259); /* 63 */
   ;	
 	mov	 edi,eax
 	not	 edi
 	or	 edi,ecx
 	xor	 edi,edx
 	add	 ebx,edi
 	add	 ebx,bp[ 9*4]
 	add	 ebx,3951481745
 	rol	 ebx,21
 	add	 ebx,ecx
   ;	
   ;		II(B,C,D,A,bp[ 9*4],S44,3951481745); /* 64 */
   ;	
   ;	
 	pop	 bp
   ;	
   ;		asm pop bp;		/* We can address our args again */
   ;	
		if	 @DataSize NE 0
   ;	
   ;		asm	if @DataSize NE 0
   ;	
			lds	 si,[bp+4]
   ;	
   ;		asm		lds si,buf
   ;	
		else	
   ;	
   ;		asm	else
   ;	
			mov	 si,[bp+4]
   ;	
   ;		asm		mov si,buf;
   ;	
		endif	
   ;	
   ;		asm	endif
   ;	
 	add		dword ptr si[0*4],eax
   ;	
   ;		asm add	dword ptr si[0*4],A;	/* buf[0] += A */
   ;	
 	add	 dword ptr si[1*4],ebx
   ;	
   ;		asm add dword ptr si[1*4],B;	/* buf[1] += B */
   ;	
 	add	 dword ptr si[2*4],ecx
   ;	
   ;		asm add dword ptr si[2*4],C;	/* buf[2] += C */
   ;	
 	add	 dword ptr si[3*4],edx
   ;	
   ;		asm add dword ptr si[3*4],D;	/* buf[3] += D */
   ;	
   ;		/* Restore caller's registers */
   ;	
		if	 @DataSize NE 0
   ;	
   ;		asm	if @DataSize NE 0
   ;	
			pop	 ds
   ;	
   ;		asm		pop ds
   ;	
		endif	
   ;	
   ;		asm	endif
   ;	
   ;	
		pop	 di
   ;	
   ;		asm	pop di;
   ;	
		pop	 si
   ;	
   ;		asm	pop si;
   ;	
	pop	di
	pop	si
	pop	bp
	ret	
_Transform	endp
	?debug	C E9
_TEXT	ends
_DATA	segment word public 'DATA'
s@	label	byte
_DATA	ends
_TEXT	segment byte public 'CODE'
_TEXT	ends
	public	_Transform
	end
