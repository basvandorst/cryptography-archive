#include "asmmath.h"

#define	NO_ASM	1

void	mul32( unsigned long * W, unsigned long U, unsigned long V )
{
#if	! NO_ASM
	asm 68020
	{
		move.l	U, d0
		mulu.l	V, d1:d0
		move.l	W, a0
		move.l	d1, (a0)+
		move.l	d0, (a0)
	}
#else
	unsigned long uh, ul, vh, vl;
	
	ul = U & 0xFFFF;
	uh = (U >> 16) & 0xFFFF;
	vl = V & 0xFFFF;
	vh = (V >> 16) & 0xFFFF;
	
	W[1] = ul * vl;
	W[0] = uh * vh;
	ul *= vh;
	vl *= uh;
	W[0] += (ul >> 16) & 0xFFFF;
	W[0] += (vl >> 16) & 0xFFFF;
	ul &= 0xFFFF;
	vl &= 0xFFFF;
	ul += vl;
	W[0] += (ul >> 16) & 0xFFFF;
	ul <<= 16;
	if ( ul > 0xFFFFFFFF - W[1] )
		W[0]++;
	W[1] += ul;
#endif
}

void	mulstep( unsigned long * W, unsigned long * K, unsigned long U, unsigned long V )
{
#if ! NO_ASM
	asm 68020
	{
		moveq.l	#0, d2		; 0 for ADDX instruction
		move.l	K, a0
		move.l	W, a1
		move.l	U, d0
		mulu.l	V, d1:d0	; D1:D0 = UV
		add.l	(a0), d0
		addx.l	d2, d1		; D1:D0 = UV + *K
		add.l	(a1), d0
		addx.l	d2, d1		; D1:D0 = UV + *K + *W
		move.l	d0, (a1)	; *W = D0	
		move.l	K, a0
		move.l	d1, (a0)	; *K = D1
	}
#else
	unsigned long	sum[2];
	
	mul32( sum, U, V );
	if ( *K > 0xFFFFFFFF - sum[1] )
		sum[0]++;
	sum[1] += *K;
	if ( *W > 0xFFFFFFFF - sum[1] )
		sum[0]++;
	sum[1] += *W;
	
	*K = sum[0];
	*W = sum[1];
#endif
}

void	addstep( unsigned long * W, unsigned long * K, unsigned long U, unsigned long V )
{
#if ! NO_ASM
	asm 68020
	{
		move.l	K, a0
		moveq.l	#0, d1
		move.l	d1, d2
		move.l	U, d0
		add.l	V, d0
		addx.l	d2, d1		; D1:D0 = U + V
		add.l	(a0), d0
		addx.l	d2, d1		; D1:D0 = U + V + *K
		move.l	d1, (a0)	; *K = D1
		move.l	W, a0
		move.l	d0, (a0)	; W = D0
	}
#else
	unsigned long sum = U, carry = 0;
	
	if ( sum > 0xFFFFFFFF - V )
		carry = 1;
	sum += V;
	if ( sum > 0xFFFFFFFF - *K )
		carry++;
	sum += *K;
	*W = sum;
	*K = carry;
#endif
}

void	div32( unsigned long * Q, unsigned long * R, unsigned long U0, unsigned long U1, unsigned long V )
{
#if ! NO_ASM
	asm 68020
	{
		move.l	U0, d1
		move.l	U1, d0
		divu.l	V, d1:d0
		bvc		@1
		move.l	#0xFFFFFFFF, d0
	@1:
		move.l	Q, a0
		move.l	d0, (a0)
		move.l	R, a0
		move.l	d1, (a0)
	}
#else
	unsigned long	q = 0, r = 0;
	unsigned long	mask;
	unsigned long	bit;
	
	for ( mask = 0x80000000; mask != 0; mask >>= 1 )
	{
		bit = (U0 & mask) ? 1 : 0;
		if ( q > 0xFFFFFFFF - q )
		{
			*Q = 0xFFFFFFFF;
			return;
		}
		q += q;
		if ( r + bit >= V - r )
		{
			q++;
			r -= (V-r)-bit;
		}
		else
			r += r+bit;
	}
	for ( mask = 0x80000000; mask != 0; mask >>= 1 )
	{
		bit = (U1 & mask) ? 1 : 0;
		if ( q > 0xFFFFFFFF - q )
		{
			*Q = 0xFFFFFFFF;
			return;
		}
		q += q;
		if ( r + bit >= V - r )
		{
			q++;
			r -= (V-r)-bit;
		}
		else
			r += r+bit;
	}
	*Q = q;
	*R = r;
#endif
}

void	qstep( unsigned long * Q, unsigned long V1, unsigned long V2, unsigned long U0, unsigned long U1, unsigned long U2 )
{
#if ! NO_ASM
	asm 68020
	{
		;
		; while Q * V1:V2 > U0:U1:U2
		;	decrement Q
		;
		move.l	Q, a0
@TryAgain:
		move.l	V1, d1
		mulu.l	(a0), d2:d1
		move.l	V2, d0
		move.l	d3, -(a7)
		move.l	d4, -(a7)
		move.l	#0, d4
		mulu.l	(a0), d3:d0
		add.l	d3, d1
		addx.l	d4, d2		; D2:D1:D0 = Q * V1:V2
		move.l	(a7)+, d4
		move.l	(a7)+, d3
		
		cmp.l	U0, d2
		bhi		@FixQ
		bcs		@Done
		
		cmp.l	U1, d1
		bhi		@FixQ
		bcs		@Done
		
		cmp.l	U2, d0
		bhi		@FixQ
@Done:
		return
@FixQ:
		subq.l	#1, (a0)
		bra		@TryAgain		
	}
#else
	unsigned long	v0, v1, v2;
	unsigned long	k;
	
loop:
	v0 = 0;
	v1 = 0;
	v2 = 0;
	k = 0;
	mulstep( &v2, &k, V2, *Q );
	mulstep( &v1, &k, V1, *Q );
	mulstep( &v0, &k,  0, *Q );
	
	if ( v0 > U0 )
		goto again;
	if ( v0 < U0 )
		return;
	if ( v1 > U1 )
		goto again;
	if ( v1 < U1 )
		return;
	if ( v2 > U2 )
		goto again;
	return;
again:
	(*Q)--;
	goto loop;
#endif
}

