/* Fast 386 Borland C inline assembler version of the transform() function
 * from the RSA Data Security, Inc, MD5 Message Digest Algorithm.
 *
 * This version uses native 32 bit registers, so it needs a 386 or 486 CPU.
 * To compile: bcc md5trans.c
 *
 * Because this function does *lots* of 32-bit operations, this version is
 * MUCH faster than the reference C version compiled with a garden-
 * variety 16-bit MS-DOS C compiler.
 *
 * Written and placed into the public domain on
 * 22 February 1992 by Phil Karn, KA9Q
 */
#pragma inline
/* I really shouldn't have to do this explicitly... */
#ifdef	__COMPACT__
	asm	.MODEL COMPACT
#elif	__HUGE__
	asm	.MODEL HUGE
#elif	__LARGE__
	asm	.MODEL LARGE
#elif	__MEDIUM__
	asm	.MODEL MEDIUM
#elif	__SMALL__
	asm	.MODEL SMALL
#elif	__TINY__
	asm	.MODEL TINY
#endif

/* Code sequence common to all four rounds.
 * evaluates a = b + (a + edi + x + t) <<< s)
 */
#define	COM(a,b,x,s,t)\
	asm	add a,edi;\
	asm	add a,x;\
	asm	add a,t;\
	asm	rol a,s;\
	asm	add a,b;

/* Round 1 functions */
/* edi = F(x,y,z) = (x & y) | (~x & z) */
#define	F(x,y,z)\
	asm	mov edi,x;\
	asm	and edi,y;\
	asm	mov esi,x;\
	asm	not esi;\
	asm	and esi,z;\
	asm	or edi,esi

/* a = b + ((a + F(x,y,z) + x + t) <<< s); */
#define	FF(a,b,c,d,x,s,t)\
	F(b,c,d);\
	COM(a,b,x,s,t)

/* Round 2 functions */
/* edi = G(x,y,z) = F(z,x,y) = (x & z) | (y & ~z) */
#define	G(x,y,z) F(z,x,y)

/* a = b + ((a + G(b,c,d) + x + t) <<< s) */
#define	GG(a,b,c,d,x,s,t)\
	G(b,c,d);\
	COM(a,b,x,s,t)

/* Round 3 functions */
/* edi = H(x,y,z) = x ^ y ^ z */
#define	H(x,y,z)\
	asm	mov edi,x;\
	asm	xor edi,y;\
	asm	xor edi,z

/* a = b + ((a + H(b,c,d) + x + t) <<< s) */
#define	HH(a,b,c,d,x,s,t)\
	H(b,c,d);\
	COM(a,b,x,s,t)

/* Round 4 functions */
/* edi = I(x,y,z) = y ^ (x | ~Z) */
#define	I(x,y,z)\
	asm	mov edi,z;\
	asm	not edi;\
	asm	or edi,x;\
	asm	xor edi,y

/* a = b + ((a + I(b,c,d) + x + t) <<< s) */
#define	II(a,b,c,d,x,s,t)\
	I(b,c,d);\
	COM(a,b,x,s,t)

#define	A	eax
#define	B	ebx
#define	C	ecx
#define	D	edx

void
Transform(buf,input)
long *buf;
long *input;
{
	asm	.386;	/* Allow use of 32-bit general registers */

	/* Save caller's registers */
	asm	push si;
	asm	push di;
	asm	if @DataSize NE 0
	asm		push ds;
	asm	endif

	/* Get buf argument */
	asm	if @DataSize NE 0
	asm		lds si,buf;
	asm	else
	asm		mov si,buf;
	asm	endif
	asm	mov A,dword ptr si[0*4];	/* A = buf[0] */
	asm	mov B,dword ptr si[1*4];	/* B = buf[1] */
	asm	mov C,dword ptr si[2*4];	/* C = buf[2] */
	asm	mov D,dword ptr si[3*4];	/* D = buf[3] */

	/* Warning: This makes our other args inaccessible until bp
	 * is restored!
	*/
	asm	push bp;
	asm	if @DataSize NE 0
	asm		lds bp,input
	asm	else
	asm		mov bp,input
	asm	endif

/* Round 1. The *4 factors in the subscripts to bp account for the
 * byte offsets of each long element in the input array.
 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
	FF(A,B,C,D,bp[ 0*4],S11,3614090360); /* 1 */
	FF(D,A,B,C,bp[ 1*4],S12,3905402710); /* 2 */
	FF(C,D,A,B,bp[ 2*4],S13, 606105819); /* 3 */
	FF(B,C,D,A,bp[ 3*4],S14,3250441966); /* 4 */
	FF(A,B,C,D,bp[ 4*4],S11,4118548399); /* 5 */
	FF(D,A,B,C,bp[ 5*4],S12,1200080426); /* 6 */
	FF(C,D,A,B,bp[ 6*4],S13,2821735955); /* 7 */
	FF(B,C,D,A,bp[ 7*4],S14,4249261313); /* 8 */
	FF(A,B,C,D,bp[ 8*4],S11,1770035416); /* 9 */
	FF(D,A,B,C,bp[ 9*4],S12,2336552879); /* 10 */
	FF(C,D,A,B,bp[10*4],S13,4294925233); /* 11 */
	FF(B,C,D,A,bp[11*4],S14,2304563134); /* 12 */
	FF(A,B,C,D,bp[12*4],S11,1804603682); /* 13 */
	FF(D,A,B,C,bp[13*4],S12,4254626195); /* 14 */
	FF(C,D,A,B,bp[14*4],S13,2792965006); /* 15 */
	FF(B,C,D,A,bp[15*4],S14,1236535329); /* 16 */

/* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
	GG(A,B,C,D,bp[ 1*4],S21,4129170786); /* 17 */
	GG(D,A,B,C,bp[ 6*4],S22,3225465664); /* 18 */
	GG(C,D,A,B,bp[11*4],S23, 643717713); /* 19 */
	GG(B,C,D,A,bp[ 0*4],S24,3921069994); /* 20 */
	GG(A,B,C,D,bp[ 5*4],S21,3593408605); /* 21 */
	GG(D,A,B,C,bp[10*4],S22,  38016083); /* 22 */
	GG(C,D,A,B,bp[15*4],S23,3634488961); /* 23 */
	GG(B,C,D,A,bp[ 4*4],S24,3889429448); /* 24 */
	GG(A,B,C,D,bp[ 9*4],S21, 568446438); /* 25 */
	GG(D,A,B,C,bp[14*4],S22,3275163606); /* 26 */
	GG(C,D,A,B,bp[ 3*4],S23,4107603335); /* 27 */
	GG(B,C,D,A,bp[ 8*4],S24,1163531501); /* 28 */
	GG(A,B,C,D,bp[13*4],S21,2850285829); /* 29 */
	GG(D,A,B,C,bp[ 2*4],S22,4243563512); /* 30 */
	GG(C,D,A,B,bp[ 7*4],S23,1735328473); /* 31 */
	GG(B,C,D,A,bp[12*4],S24,2368359562); /* 32 */

/* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
	HH(A,B,C,D,bp[ 5*4],S31,4294588738); /* 33 */
	HH(D,A,B,C,bp[ 8*4],S32,2272392833); /* 34 */
	HH(C,D,A,B,bp[11*4],S33,1839030562); /* 35 */
	HH(B,C,D,A,bp[14*4],S34,4259657740); /* 36 */
	HH(A,B,C,D,bp[ 1*4],S31,2763975236); /* 37 */
	HH(D,A,B,C,bp[ 4*4],S32,1272893353); /* 38 */
	HH(C,D,A,B,bp[ 7*4],S33,4139469664); /* 39 */
	HH(B,C,D,A,bp[10*4],S34,3200236656); /* 40 */
	HH(A,B,C,D,bp[13*4],S31, 681279174); /* 41 */
	HH(D,A,B,C,bp[ 0*4],S32,3936430074); /* 42 */
	HH(C,D,A,B,bp[ 3*4],S33,3572445317); /* 43 */
	HH(B,C,D,A,bp[ 6*4],S34,  76029189); /* 44 */
	HH(A,B,C,D,bp[ 9*4],S31,3654602809); /* 45 */
	HH(D,A,B,C,bp[12*4],S32,3873151461); /* 46 */
	HH(C,D,A,B,bp[15*4],S33, 530742520); /* 47 */
	HH(B,C,D,A,bp[ 2*4],S34,3299628645); /* 48 */

/* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
	II(A,B,C,D,bp[ 0*4],S41,4096336452); /* 49 */
	II(D,A,B,C,bp[ 7*4],S42,1126891415); /* 50 */
	II(C,D,A,B,bp[14*4],S43,2878612391); /* 51 */
	II(B,C,D,A,bp[ 5*4],S44,4237533241); /* 52 */
	II(A,B,C,D,bp[12*4],S41,1700485571); /* 53 */
	II(D,A,B,C,bp[ 3*4],S42,2399980690); /* 54 */
	II(C,D,A,B,bp[10*4],S43,4293915773); /* 55 */
	II(B,C,D,A,bp[ 1*4],S44,2240044497); /* 56 */
	II(A,B,C,D,bp[ 8*4],S41,1873313359); /* 57 */
	II(D,A,B,C,bp[15*4],S42,4264355552); /* 58 */
	II(C,D,A,B,bp[ 6*4],S43,2734768916); /* 59 */
	II(B,C,D,A,bp[13*4],S44,1309151649); /* 60 */
	II(A,B,C,D,bp[ 4*4],S41,4149444226); /* 61 */
	II(D,A,B,C,bp[11*4],S42,3174756917); /* 62 */
	II(C,D,A,B,bp[ 2*4],S43, 718787259); /* 63 */
	II(B,C,D,A,bp[ 9*4],S44,3951481745); /* 64 */

	asm pop bp;		/* We can address our args again */
	asm	if @DataSize NE 0
	asm		lds si,buf
	asm	else
	asm		mov si,buf;
	asm	endif
	asm add	dword ptr si[0*4],A;	/* buf[0] += A */
	asm add dword ptr si[1*4],B;	/* buf[1] += B */
	asm add dword ptr si[2*4],C;	/* buf[2] += C */
	asm add dword ptr si[3*4],D;	/* buf[3] += D */

	/* Restore caller's registers */
	asm	if @DataSize NE 0
	asm		pop ds
	asm	endif

	asm	pop di;
	asm	pop si;
}
