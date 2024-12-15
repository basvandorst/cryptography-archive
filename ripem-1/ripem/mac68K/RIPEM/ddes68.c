/* DDES (V5.0B) - 
 *
 * A portable, public domain, version of the Data Encryption Standard.
 *
 * Written with Symantec's THINK (Lightspeed) C by Richard Outerbridge.
 * Thanks to: Dan Hoey for his excellent Initial and Inverse permutation
 * code;  Jim Gillogly & Phil Karn for the DES key schedule code; Dennis
 * Ferguson, Eric Young and Dana How for comparing notes; and Ray Lau,
 * for humouring me on. 
 *
 * Copyright (c) 1988,1989,1990,1991,1992 by Richard Outerbridge.
 * (GEnie : OUTER; CIS : [71755,204]) Graven Imagery, 1992.
 */

#include "ddes.h"

static void copy8(unsigned char *, unsigned char *);
static void desfunc(unsigned long *, unsigned long *);

#define scrunch(a,b) copy8((a), (unsigned char *)(b))
#define unscrun(a,b) copy8((unsigned char *)(a), (b))

static unsigned long KnL[32] = { 0L };
static unsigned long KnR[32] = { 0L };
static unsigned char Df_Key[16] = {
	0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
	0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10 };
	
void deskey(key, edf)			/* Thanks to Mike Duvos! */
unsigned char *key;
short edf;
{
asm {
	movem.l d3-d5/a2-a3,-(a7)	;save registers
	lea	@kpp,a2			;table of permutation adjusts
	lea	@kp0,a0			;tables of byte/bit offsets
	movea.l key,a1			;load address of nominal key
	moveq	#0,d4			;clear this register
	moveq	#15,d0			;do all 16 rounds
	lea	KnL,a3			;load address of key register
	cmpi.w	#EN0,edf		;are we encrypting
	beq	@cks1			;yes - start now
	adda.w	#120,a3			;no - adjust backfill
	bra	@cks1			;now start

@kpp:	dc.w	0,0,0,112,112,112,112,112,224,0,112,112,112,112,112,224

@kp0:	dc.b	6,6,1,6,6,5,4,6,7,4,6,7,2,7,1,5,0,5,4,5,3,6,3,7,5,4,7,6
	dc.b	0,1,2,2,3,4,4,1,6,2,4,3,0,4,0,2,7,3,2,3,4,2,7,1,1,1,2,4
	dc.b	5,5,4,7,7,7,0,6,1,7,2,5,5,6,6,4,7,5,0,7,4,4,3,5,2,6,5,7
	dc.b	5,2,5,1,3,2,0,3,6,3,2,1,3,3,1,4,5,3,1,2,1,3,7,2,6,1,3,1
	
	dc.b	4,0,4,3,4,6,1,3,5,6,5,0,4,2,1,5,0,2,5,2,5,1,0,0,5,3,0,6
	dc.b	7,2,7,4,3,0,7,0,6,6,6,0,3,2,3,5,6,2,7,3,6,4,7,1,2,6,7,6
	dc.b	1,4,1,2,0,1,5,5,5,4,1,6,0,3,1,1,0,4,1,0,4,4,4,1,0,5,4,5
	dc.b	3,3,2,3,2,5,2,4,7,5,3,1,3,4,2,0,2,1,3,6,6,3,2,2,6,5,6,1
	
	dc.b	0,3,5,5,1,4,5,1,1,1,4,5,5,4,0,2,5,6,4,1,0,5,4,2,4,4,4,0
	dc.b	2,2,3,6,7,6,6,1,3,3,3,4,7,1,6,2,7,5,6,3,2,5,6,5,7,2,2,0
	dc.b	5,2,0,0,5,0,0,4,1,0,1,5,1,3,5,3,4,6,0,6,0,1,1,6,4,3,1,2
	dc.b	6,4,7,0,7,4,3,0,2,1,2,6,7,3,3,2,6,0,3,5,2,4,6,6,2,3,3,1

@cks1:	suba.w	(a2)+,a0			;correct permutation
	moveq	#7,d1			;do 8 bytes worth
@cks2:	moveq	#6,d2			;do 7 bits per byte
	moveq	#0,d3			;clear a byte to fill
@cks3:	move.b	(a0)+,d4		;get a byte number
	move.b	(a0)+,d5		;get a bit number
	btst	d5,0(a1,d4.w)		;test bit in key byte
	beq.s	@cks4			;if no bit to set
	bset	d2,d3			;else set a bit
@cks4:	dbf	d2,@cks3		;loop until all bits done
	move.b	d3,(a3)+		;set byte
	dbf	d1,@cks2		;loop through all bytes
	lea	-8(a3),a1		;reset a1 to last subkey
	cmpi.w	#EN0,edf		;check mode
	beq.s	@cks5			;encryption
	suba.w	#16,a3			;adjust filling
@cks5:	dbf	d0,@cks1		;for all rounds
	move.l	#0x3f3f3f3f,d1		;load bit mask
	lea	KnL,a0			;load key register
	moveq	#15,d3			;load loop index
@cks6:	move.l	(a0),d0			;next raw target
	and.l	d1,d0			;clear bits
	rol.l	#4,d0			;preallign
	move.l	d0,(a0)+		;set cooked key - even
	move.l	(a0),d0			;next raw target
	and.l	d1,d0			;clear bits
	move.l	d0,(a0)+		;set cooked key - odd
	dbf	d3,@cks6		;all 32 longs
	movem.l (a7)+,d3-d5/a2-a3	;restore registers
	}
return;
}

void usekey(premix)
unsigned long *premix;
{
asm {	movea.l premix,a1
	lea	KnL,a0
	moveq	#31,d0
@usek1:	move.l	(a1)+,(a0)+
	dbf	d0,@usek1
	}
return;
}

void cpkey(where)
unsigned long *where;
{
asm {	movea.l	where,a1
	lea	KnL,a0
	moveq	#31,d0
@cpk1:	move.l	(a0)+,(a1)+
	dbf	d0,@cpk1
	}
return;
}
	
void des(inblock, outblock)
unsigned char *inblock, *outblock;
{
	unsigned long work[2];

	scrunch(inblock, work);
	desfunc(work, KnL);
	unscrun(work, outblock);
	return;
	}

static void copy8(outof, into)
unsigned char *outof;
unsigned char *into;
{
asm {
	movea.l	outof,a0
	movea.l	into,a1
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0),(a1)
	}
return;
}

static void desfunc(inblock, keysched)
unsigned long *inblock, *keysched;
{	
asm {	movem.l d3-d6/a2,-(a7)
	movea.l	inblock,a0
	move.l	(a0),d3		/* D3 = L */
	move.l	4(a0),d5	/* D5 = R */
	
	move.l	d3,d2		/* EXSHMSK(R,0x0f0f0f0f,L,4,tmp) */
	lsr.l	#4,d2
	eor.l	d5,d2
	andi.l	#0x0f0f0f0f,d2
	eor.l	d2,d5
	lsl.l	#4,d2
	eor.l	d2,d3
	swap	d3		/* EXSHMSK(R,0x0000ffff,L,16,tmp) */
	move.w	d3,d2
	move.w	d5,d3
	move.w	d2,d5
	swap	d3
	move.l	d5,d2		/* EXSHMSK(L,0x33333333,R,2,tmp) */
	lsr.l	#2,d2
	eor.l	d3,d2
	andi.l	#0x33333333,d2
	eor.l	d2,d3
	lsl.l	#2,d2
	eor.l	d2,d5
	move.l	d5,d2		/* EXSHMSK(L,0x00ff00ff,R,8,tmp) */
	lsr.l	#8,d2
	eor.l	d3,d2
	andi.l	#0x00ff00ff,d2
	eor.l	d2,d3
	lsl.l	#8,d2
	eor.l	d2,d5
	rol.l	#1,d5		/* EXSHMSK(R,0x55555555,L,1,tmp) */
	move.l	d3,d2
	eor.l	d5,d2
	andi.l	#0xaaaaaaaa,d2
	eor.l	d2,d5
	eor.l	d2,d3
	rol.l	#1,d3

	lea	@SP0,a0
	lea	@SP1,a1
	movea.l	keysched,a2
	move.l	#0x03f003f0,d2
	moveq	#7,d1
	bra	@des1
	
DC.W	0x0123	/* to force long allignment, if needed */
@SP0:	DC.L	0x01010400,0x00000208,0x00000100,0x00200000
	DC.L	0x00000000,0x08020200,0x02080100,0x04200002
	DC.L	0x00010000,0x00000000,0x02080000,0x04000802
	DC.L	0x01010404,0x08020008,0x42000100,0x00000000
	DC.L	0x01010004,0x08000200,0x00080000,0x00000800
	DC.L	0x00010404,0x00000000,0x00000100,0x04000802
	DC.L	0x00000004,0x00020208,0x40000000,0x00200802
	DC.L	0x00010000,0x08000200,0x02080000,0x04200800
	DC.L	0x00000400,0x00020008,0x40080100,0x04200802
	DC.L	0x01010400,0x08000008,0x00080000,0x00200000
	DC.L	0x01010404,0x08000008,0x02000100,0x00000000
	DC.L	0x00000400,0x00020000,0x40080100,0x04000002
	DC.L	0x01000404,0x08020208,0x42000100,0x00000002
	DC.L	0x01010004,0x00020008,0x42080000,0x04000000
	DC.L	0x01000000,0x08020000,0x00080100,0x04200002
	DC.L	0x00000004,0x00000208,0x40000000,0x00000802
	DC.L	0x00000404,0x08000000,0x02000000,0x04000800
	DC.L	0x01000400,0x00000008,0x40080000,0x00200802
	DC.L	0x01000400,0x08020200,0x40080000,0x00200002
	DC.L	0x00010400,0x00000200,0x00000000,0x04000800
	DC.L	0x00010400,0x00020200,0x40000100,0x04000002
	DC.L	0x01010000,0x08020000,0x42080100,0x04200000
	DC.L	0x01010000,0x08020008,0x42080100,0x04200800
	DC.L	0x01000404,0x00020208,0x02000100,0x00200002
	DC.L	0x00010004,0x08000208,0x42080000,0x04200000
	DC.L	0x01000004,0x00020200,0x40000100,0x00000800
	DC.L	0x01000004,0x00020000,0x00000000,0x00000802
	DC.L	0x00010004,0x08000208,0x42000000,0x04200802
	DC.L	0x00000000,0x00000008,0x02080100,0x00200800
	DC.L	0x00000404,0x08020208,0x02000000,0x00000002
	DC.L	0x00010404,0x00000200,0x42000000,0x04000000
	DC.L	0x01000000,0x08000000,0x00080100,0x00200800
	DC.L	0x00010000,0x08020200,0x00080000,0x04000000
	DC.L	0x01010404,0x08000000,0x42000100,0x00200800
	DC.L	0x00000004,0x00020008,0x00000100,0x00200000
	DC.L	0x01010000,0x00000208,0x02000000,0x04000802
	DC.L	0x01010400,0x00020000,0x40000000,0x04000802
	DC.L	0x01000000,0x08020200,0x02080000,0x04200002
	DC.L	0x01000000,0x08000200,0x42000100,0x04200002
	DC.L	0x00000400,0x00000000,0x40080100,0x00000002
	DC.L	0x01010004,0x00000200,0x02000100,0x00200002
	DC.L	0x00010000,0x00020008,0x40000000,0x04000000
	DC.L	0x00010400,0x08020208,0x42080000,0x04000800
	DC.L	0x01000004,0x08000200,0x02080100,0x00200000
	DC.L	0x00000400,0x08000008,0x40080100,0x04200800
	DC.L	0x00000004,0x00000200,0x00000100,0x00000802
	DC.L	0x01000404,0x00000000,0x02000000,0x00200802
	DC.L	0x00010404,0x08020008,0x42080000,0x04200800
	DC.L	0x01010404,0x08000208,0x42080100,0x00000802
	DC.L	0x00010004,0x00020000,0x00080100,0x04000002
	DC.L	0x01010000,0x08000000,0x42000000,0x04200802
	DC.L	0x01000404,0x08020208,0x42080100,0x04200000
	DC.L	0x01000004,0x00000008,0x02080000,0x00200800
	DC.L	0x00000404,0x00020208,0x00000000,0x00000000
	DC.L	0x00010404,0x00020200,0x40080000,0x00000002
	DC.L	0x01010400,0x08000008,0x42000000,0x04200802
	DC.L	0x00000404,0x08020000,0x00080100,0x00000000
	DC.L	0x01000400,0x08000208,0x02000100,0x00200802
	DC.L	0x01000400,0x00000208,0x40000100,0x04200000
	DC.L	0x00000000,0x08020000,0x00080000,0x00000800
	DC.L	0x00010004,0x00020208,0x00000000,0x04000002
	DC.L	0x00010400,0x00000008,0x40080000,0x04000800
	DC.L	0x00000000,0x08020008,0x02080100,0x00000800
	DC.L	0x01010004,0x00020200,0x40000100,0x00200002

@SP1:	DC.L	0x80108020,0x00802001,0x20000010,0x10001040
	DC.L	0x80008000,0x00002081,0x20400000,0x00001000
	DC.L	0x00008000,0x00002081,0x00004000,0x00040000
	DC.L	0x00108020,0x00000080,0x20404010,0x10041040
	DC.L	0x00100000,0x00802080,0x20400000,0x10000000
	DC.L	0x00000020,0x00800081,0x00000010,0x10001040
	DC.L	0x80100020,0x00800001,0x20404010,0x00000040
	DC.L	0x80008020,0x00002001,0x00400000,0x10000000
	DC.L	0x80000020,0x00000000,0x20004000,0x00040040
	DC.L	0x80108020,0x00802000,0x00404010,0x10040000
	DC.L	0x80108000,0x00802000,0x00400000,0x10041040
	DC.L	0x80000000,0x00802081,0x20000010,0x00041000
	DC.L	0x80008000,0x00000081,0x00400010,0x10041000
	DC.L	0x00100000,0x00000000,0x20004000,0x00041040
	DC.L	0x00000020,0x00800080,0x20000000,0x00001000
	DC.L	0x80100020,0x00800001,0x00004010,0x00000040
	DC.L	0x00108000,0x00000001,0x00000000,0x10040000
	DC.L	0x00100020,0x00002000,0x00400010,0x10000040
	DC.L	0x80008020,0x00800000,0x20004010,0x10001000
	DC.L	0x00000000,0x00802001,0x00004000,0x00001040
	DC.L	0x80000000,0x00000080,0x00404000,0x00041000
	DC.L	0x00008000,0x00800000,0x20004010,0x00040040
	DC.L	0x00108020,0x00002001,0x00000010,0x10040040
	DC.L	0x80100000,0x00002080,0x20400010,0x10041000
	DC.L	0x00100020,0x00800081,0x20400010,0x00001040
	DC.L	0x80000020,0x00000001,0x00000000,0x00000000
	DC.L	0x00000000,0x00002080,0x00404010,0x00000000
	DC.L	0x00108000,0x00800080,0x20404000,0x10040040
	DC.L	0x00008020,0x00002000,0x00004010,0x10000040
	DC.L	0x80108000,0x00802080,0x00404000,0x10001000
	DC.L	0x80100000,0x00802081,0x20404000,0x00041040
	DC.L	0x00008020,0x00000081,0x20000000,0x00040000
	DC.L	0x00000000,0x00800080,0x20004000,0x00041040
	DC.L	0x00108020,0x00800001,0x00000010,0x00040000
	DC.L	0x80100020,0x00802000,0x20400010,0x10041000
	DC.L	0x00100000,0x00802081,0x00404000,0x00001000
	DC.L	0x80008020,0x00000081,0x20404010,0x00000040
	DC.L	0x80100000,0x00000000,0x00400000,0x10040040
	DC.L	0x80108000,0x00000000,0x00004010,0x00001000
	DC.L	0x00008000,0x00802000,0x20000010,0x00041040
	DC.L	0x80100000,0x00002080,0x00400000,0x10001000
	DC.L	0x80008000,0x00800080,0x20004000,0x00000040
	DC.L	0x00000020,0x00800081,0x20000000,0x10000040
	DC.L	0x80108020,0x00000001,0x00004010,0x10040000
	DC.L	0x00108020,0x00802001,0x20000010,0x10040040
	DC.L	0x00000020,0x00002081,0x20404010,0x10000000
	DC.L	0x00008000,0x00002081,0x00404000,0x00040000
	DC.L	0x80000000,0x00000080,0x20400000,0x10001040
	DC.L	0x00008020,0x00802081,0x00404010,0x00000000
	DC.L	0x80108000,0x00000081,0x20404000,0x10041040
	DC.L	0x00100000,0x00000001,0x00000000,0x00040040
	DC.L	0x80000020,0x00002000,0x20400010,0x10000040
	DC.L	0x00100020,0x00800001,0x00000010,0x10040000
	DC.L	0x80008020,0x00002001,0x00004000,0x10001000
	DC.L	0x80000020,0x00802080,0x20400000,0x10001040
	DC.L	0x00100020,0x00800081,0x00404010,0x00000000
	DC.L	0x00108000,0x00002001,0x00004000,0x10041040
	DC.L	0x00000000,0x00002080,0x00400010,0x00041000
	DC.L	0x80008000,0x00800000,0x20004010,0x00041000
	DC.L	0x00008020,0x00802001,0x00000000,0x00001040
	DC.L	0x80000000,0x00000080,0x20404000,0x00001040
	DC.L	0x80100020,0x00800000,0x20000000,0x00040040
	DC.L	0x80108020,0x00002000,0x00400010,0x10000000
	DC.L	0x00108000,0x00802080,0x20004010,0x10041000

@des1:	move.l	(a2)+,d0
	eor.l	d5,d0
	move.l	d0,d6
	and.l	d2,d0
	move.l	12(a0,d0.w),d4	/* S6 */
	swap	d0
	or.l	4(a0,d0.w),d4	/* S2 */
	ror.l	#8,d6
	and.l	d2,d6
	or.l	8(a0,d6.w),d4	/* S4 */
	swap	d6
	or.l	0(a0,d6.w),d4	/* S0 */
	
	move.l	(a2)+,d0
	eor.l	d5,d0
	move.l	d0,d6
	lsl.l	#4,d0
	and.l	d2,d0
	or.l	12(a1,d0.w),d4	/* S7 */
	swap	d0
	or.l	4(a1,d0.w),d4	/* S3 */
	lsr.l	#4,d6
	and.l	d2,d6
	or.l	8(a1,d6.w),d4	/* S5 */
	swap	d6
	or.l	0(a1,d6.w),d4	/* S1 */
	eor.l	d4,d3
	
	move.l	(a2)+,d0
	eor.l	d3,d0
	move.l	d0,d6
	and.l	d2,d0
	move.l	12(a0,d0.w),d4	/* S6 */
	swap	d0
	or.l	4(a0,d0.w),d4	/* S2 */
	ror.l	#8,d6
	and.l	d2,d6
	or.l	8(a0,d6.w),d4	/* S4 */
	swap	d6
	or.l	0(a0,d6.w),d4	/* S0 */
	
	move.l	(a2)+,d0
	eor.l	d3,d0
	move.l	d0,d6
	lsl.l	#4,d0
	and.l	d2,d0
	or.l	12(a1,d0.w),d4	/* S7 */
	swap	d0
	or.l	4(a1,d0.w),d4	/* S3 */
	lsr.l	#4,d6
	and.l	d2,d6
	or.l	8(a1,d6.w),d4	/* S5 */
	swap	d6
	or.l	0(a1,d6.w),d4	/* S1 */
	eor.l	d4,d5
	dbf	d1,@des1	/* 53 */
	
	ror.l	#1,d5		/* EXSHMSK(R,0x55555555,L,1,tmp) */
	move.l	d5,d2
	eor.l	d3,d2
	andi.l	#0xaaaaaaaa,d2
	eor.l	d2,d3
	eor.l	d2,d5
	ror.l	#1,d3
	move.l	d3,d2		/* EXSHMSK(L,0x00ff00ff,R,8,tmp) */
	lsr.l	#8,d2
	eor.l	d5,d2
	andi.l	#0x00ff00ff,d2
	eor.l	d2,d5
	lsl.l	#8,d2
	eor.l	d2,d3
	move.l	d3,d2		/* EXSHMSK(L,0x33333333,R,2,tmp) */
	lsr.l	#2,d2
	eor.l	d5,d2
	andi.l	#0x33333333,d2
	eor.l	d2,d5
	lsl.l	#2,d2
	eor.l	d2,d3
	swap	d5		/* EXSHMSK(R,0x0000ffff,L,16,tmp) */
	move.w	d5,d2
	move.w	d3,d5
	move.w	d2,d3
	swap	d5
	move.l	d5,d2		/* EXSHMSK(R,0x0f0f0f0f,L,4,tmp) */
	lsr.l	#4,d2
	eor.l	d3,d2
	andi.l	#0x0f0f0f0f,d2
	eor.l	d2,d3
	lsl.l	#4,d2
	eor.l	d2,d5
	
	movea.l	inblock,a0
	move.l	d5,(a0)+
	move.l	d3,(a0)
	movem.l	(a7)+,a2/d3-d6	
	}			/* end asm	*/
return;
}

#ifdef D2_DES

void desDkey(hexkey, mode)
unsigned char *hexkey;			/* unsigned char[16] */
short mode;
{
	short revmod;

	revmod = (mode == EN0) ? DE1 : EN0;
	deskey(&hexkey[8], revmod);
	cpkey(KnR);
	deskey(hexkey, mode);
	return;
	}

void cpDkey(into)
unsigned long *into;		/* unsigned long[64] */
{
	cpkey(into);
asm {	movea.l	into,a1
	adda.w	#128,a1
	lea	KnR,a0
	moveq	#31,d0
@cpdk1:	move.l	(a0)+,(a1)+
	dbf	d0,@cpdk1
	}
return;
}

void useDkey(from)
unsigned long *from;		/* unsigned long[64] */
{
	usekey(from);
asm {	movea.l from,a1
	adda.w	#128,a1
	lea	KnR,a0
	moveq	#31,d0
@usdk1:	move.l	(a1)+,(a0)+
	dbf	d0,@usdk1
	}
return;
}

void Ddes(from, into)
unsigned char *from, *into;		/* unsigned char[8] */
{
	unsigned long work[2];

	scrunch(from, work);
	desfunc(work, KnL);
	desfunc(work, KnR);
	desfunc(work, KnL);
	unscrun(work, into);
	return;
	}

void D2des(from, into)
unsigned char *from;			/* unsigned char[16] */
unsigned char *into;			/* unsigned char[16] */
{
	unsigned long *right, *l1, swap;
	unsigned long leftt[2], bufR[2];
	
	right = bufR;
	l1 = &leftt[1];
	scrunch(from, leftt);
	scrunch(&from[8], right);
	desfunc(leftt, KnL);
	desfunc(right, KnL);
	swap = *l1;
	*l1 = *right;
	*right = swap;
	desfunc(leftt, KnR);
	desfunc(right, KnR);
	swap = *l1;
	*l1 = *right;
	*right = swap;
	desfunc(leftt, KnL);
	desfunc(right, KnL);
	unscrun(leftt, into);
	unscrun(right, &into[8]);
	return;
	}

void makekey(aptr, kptr)
char *aptr;				/* NULL-terminated  */
unsigned char *kptr;		/* unsigned char[8] */
{
	unsigned char *store;
	int first, i;
	unsigned long savek[64];

	cpDkey(savek);
	desDkey(Df_Key, EN0);
	for( i = 0; i < 8; i++ ) kptr[i] = Df_Key[i];
	first = 1;
	while( (*aptr != '\0')	|| first ) {
		store = kptr;
		for( i = 0; i < 8 && (*aptr != '\0'); i++ ) {
			*store++ ^= *aptr & 0x7f;
			*aptr++ = '\0';
			}
		Ddes(kptr, kptr);
		first = 0;
		}
	useDkey(savek);
	return;
	}

void makeDkey(aptr, kptr)
char *aptr;				/* NULL-terminated   */
unsigned char *kptr;		/* unsigned char[16] */
{
	unsigned char *store;
	int first, i;
	unsigned long savek[64];

	cpDkey(savek);
	desDkey(Df_Key, EN0);
	for( i = 0; i < 16; i++ ) kptr[i] = Df_Key[i];
	first = 1;
	while( (*aptr != '\0')	|| first ) {
		store = kptr;
		for( i = 0; i < 16 && (*aptr != '\0'); i++ ) {
			*store++ ^= *aptr & 0x7f;
			*aptr++ = '\0';
			}
		D2des(kptr, kptr);
		first = 0;
		}
	useDkey(savek);
	return;
	}

#endif	/* D2_DES */

/* Validation sets:
 *
 * Single-length key, single-length plaintext -
 * Key    : 01 23 45 67 89 ab cd ef
 * Plain  : 01 23 45 67 89 ab cd e7
 * Cipher : c9 57 44 25 6a 5e d3 1d
 *
 * Double-length key, single-length plaintext -
 * Key    : 01 23 45 67 89 ab cd ef fe dc ba 98 76 54 32 10
 * Plain  : 01 23 45 67 89 ab cd e7
 * Cipher : 7f 1d 0a 77 82 6b 8a ff
 *
 * Double-length key, double-length plaintext -
 * Key    : 01 23 45 67 89 ab cd ef fe dc ba 98 76 54 32 10
 * Plain  : 01 23 45 67 89 ab cd ef 01 23 45 67 89 ab cd ff
 * Cipher : 27 a0 84 40 40 6a df 60 27 8f 47 cf 42 d6 15 d7
 *
 * ddes V5.0B rwo 921121 10:44 Graven Imagery
 **********************************************************/
