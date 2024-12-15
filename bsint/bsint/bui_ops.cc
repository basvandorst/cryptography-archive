#include "bsint.h"
#include "asmmath.h"
#include <iostream.h>
#include <stdlib.h>

void	bui_add( bsint & W, const bsint & Ua, const bsint & Va )
{
	bsint		U;
	bsint		V;
	
	if ( Ua.getSize() > Va.getSize() )
	{
		U = Ua;
		V = Va;
	}
	else
	{
		U = Va;
		V = Ua;
	}
	
	W.needOverwrite();
	
	W.setSize( U.getSize() );
	
	unsigned long carry = 0;
	for ( int i = 1; i <= V.getSize(); i++ )
		addstep( &W[U.getSize()-i], &carry, U[U.getSize()-i], V[V.getSize()-i] );
	for ( i = V.getSize() + 1; i <= U.getSize(); i++ )
		addstep( &W[U.getSize()-i], &carry, U[U.getSize()-i], 0 );
		
	if ( carry )
		W.insertDigit(carry);
}

void	bui_sub( bsint & W, const bsint & Ua, const bsint & Va )
{
	bsint		U = Ua;
	bsint		V = Va;
	
	W.needOverwrite();
	
	int	n = U.getSize();
	int m = V.getSize();
	
	if ( n < m )
	{
		W.setSize(1);
		W[0] = 0;
		return;
	}
	
	W.setSize(n);
	
	unsigned long	borrow = 0;
	
	for ( int i = 1; i <= m; i++ )
	{
		W[n-i] = U[n-i];
		if ( borrow )
		{
			if ( W[n-i] )
			{
				W[n-i]--;
				borrow = 0;
			}
			else
				W[n-i] = 0xFFFFFFFF;
		}
		if ( W[n-i] >= V[m-i] )
			W[n-i] -= V[m-i];
		else
		{
			W[n-i] += 0xFFFFFFFF - V[m-i];
			W[n-i] += (borrow = 1);
		}
	}
	for ( i = m + 1; i <= n; i++ )
	{
		W[n-i] = U[n-i];
		if ( borrow )
		{
			if ( W[n-i] )
			{
				W[n-i]--;
				borrow = 0;
			}
			else
				W[n-i] = 0xFFFFFFFF;
		}
	}
	W.removeLeadingZeros();
}

void	bui_mul0( bsint & W, const bsint & Ua, const bsint & Va )
{
	int		n, m;
	int		i, j, t;
	unsigned long k;
	
	bsint U = Ua;
	bsint V = Va;
	
	n = U.getSize();
	m = V.getSize();
	
	W.needOverwrite();
	
	W.setSize(n+m);
	
M1:
	for ( i = m+1; i <= m+n; i++ )
		W(i) = 0;
	j = m;
	
M2:
	if ( V(j) == 0 )
	{
		W(j) = 0;
		goto M6;
	}
	
M3:
	i = n;
	k = 0;
	
M4:
	mulstep( &W(i+j), &k, U(i), V(j) );
	
M5:
	if ( --i > 0 )
		goto M4;
	W(j) = k;
	
M6:
	if ( --j > 0 )
		goto M2;
		
	W.removeLeadingZeros();
}

void	bui_mul1( bsint & W, const bsint & Ua, const bsint & Va, int cut )
{
	int digits = (Ua.getSize() + Va.getSize() + 1)/4;
	
	if ( digits <= cut || Ua.getSize() <= digits || Va.getSize() <= digits
		|| W.bsintBad() )
	{
		bui_mul0( W, Ua, Va );
		return;
	}
	
	bsint	A, B, C, D, T;
	bsint	U;
	bsint 	V;
	
	
	if ( A.bsintBad() || B.bsintBad() || C.bsintBad() ||
		D.bsintBad() || T.bsintBad() || U.bsintBad() || V.bsintBad() )
	{
		bui_mul0( W, Ua, Va );
		return;
	}
	
	U = Ua;
	V = Va;
	W.needOverwrite();
	
	bui_split(A,B,U,digits);
	bui_split(C,D,V,digits);
	bui_mul1(W,B,D,cut);
	bui_mul1(T,A,C,cut);
	bui_add(A,A,B);
	bui_add(C,C,D);
	bui_mul1(A,A,C,cut);
	bui_sub(A,A,T);
	bui_sub(A,A,W);
		
	T.appendDigit(0,2*digits);
	A.appendDigit(0,digits);

	bui_add(W,W,A);
	bui_add(W,W,T);
}

void	bui_div( bsint & Q, bsint & R, const bsint & Ua, const bsint & Va )
{
	bsint	V = Va;
	
	if ( Va.getSize() == 1 )
	{
		//
		// Divisor is a single digit.  Handle this as a special case
		//
		unsigned long	r;
		Q = Ua;
		Q.needModify();
		R.needOverwrite();
		r = bui_div_digit( Q, V[0] );
		Q.removeLeadingZeros();
		R.setSize(1);
		R[0] = r;
		return;
	}
	
	if ( Va.getSize() > Ua.getSize() )
	{
		Q.needOverwrite();
		Q.setSize(1);
		Q[0] = 0;
		R = Ua;
		return;
	}
	
	R = Ua;
				
	bsint & U = R;
				
	U.needModify();
	V.needModify();
	Q.needOverwrite();
	
	int	n = V.getSize();
	int m = U.getSize() - n;
	
	Q.setSize(m+1);
	
	unsigned long d;	// normalization factor
	unsigned long k;	// carry from addition or multiplication
	unsigned long b;	// borrow for subtraction
	unsigned long qh;	// q-hat
	int	i;				// generic index
	int j;				// main index used by Knuth
	
D1:
	if ( V(1) == 0xFFFFFFFF )
	{
		d = 1;
		k = 0;
	}
	else
	{
		div32( &d, &k, 1, 0, V(1)+1 );
		
		k = 0;
		for ( i = n; i >= 1; i-- )
			mulstep( &V(i), &k, V(i), d-1 );
			
		k = 0;
		for ( i = m + n; i >= 1; i-- )
			mulstep( &U(i), &k, U(i), d-1 );
	}
	U.insertDigit(k);		// U now zero-based
	

D2:
	j = 0;
	
D3:
	if ( U[j] == V(1) )
		qh = 0xFFFFFFFF;
	else
		div32( &qh, &k, U[j], U[j+1], V(1) );
		
	qstep( &qh, V(1), V(2), U[j], U[j+1], U[j+2] );
	


D4:
	b = 0;
	k = 0;
	
	for ( i = n; i >= 1; i-- )
	{
		unsigned long t = 0;
		
		mulstep( &t, &k, qh, V(i) );
		if ( b )
		{
			if ( U[j+i] )
			{
				U[j+i]--;
				b = 0;
			}
			else
				U[j+i] = 0xFFFFFFFF;
		}
		if ( t <= U[j+i] )
			U[j+i] -= t;
		else
		{
			U[j+i] += (0xFFFFFFFF - t) + 1;
			b = 1;
		}
	}
	
	if ( b )
	{
		if ( U[j] )
		{
			U[j]--;
			b = 0;
		}
		else
			U[j] = 0xFFFFFFFF;
	}
	if ( k <= U[j] )
		U[j] -= k;
	else
	{
		U[j] += (0xFFFFFFFF - k) + 1;
		b = 1;
	}

D5:
	Q[j] = qh;
	if ( ! b )
		goto D7;
		
D6:
	Q[j]--;
	for ( k = 0, i = n; i >= 1; i-- )
	{
		addstep( &U[j+i], &k, U[j+i], V(i) );
	}
	U[j] += k;
	
D7:
	if ( ++j <= m )
		goto D3;
		
D8:
	k = 0;
	for ( i = 1; i <= n; i++ )
	{
		div32( &U[m+i], &k, k, U[m+i], d );
	}
	
	R.removeLeadingZeros();
	Q.removeLeadingZeros();
}

int		bui_equal( const bsint & Ua, const bsint & Va )
{
	bsint U = Ua;
	bsint V = Va;
	
	if ( U.getSize() != V.getSize() )
		return 0;
	for ( int i = 0; i < U.getSize(); i++ )
		if ( U[i] != V[i] )
			return 0;
	return 1;
}

int		bui_less( const bsint & Ua, const bsint & Va )
{
	bsint U = Ua;
	bsint V = Va;
	
	if ( U.getSize() > V.getSize() )
		return 0;
	else if ( U.getSize() < V.getSize() )
		return 1;
	for ( int i = 0; i < U.getSize(); i++ )
		if ( U[i] < V[i] )
			return 1;
		else if ( U[i] > V[i] )
			return 0;
	return 0;
}

int		bui_less_or_equal( const bsint & Ua, const bsint & Va )
{
	bsint U = Ua;
	bsint V = Va;
	
	if ( U.getSize() > V.getSize() )
		return 0;
	else if ( U.getSize() < V.getSize() )
		return 1;
	for ( int i = 0; i < U.getSize(); i++ )
		if ( U[i] < V[i] )
			return 1;
		else if ( U[i] > V[i] )
			return 0;
	return 1;
}

void	bui_add_digit( bsint & W, unsigned long change )
{
	if ( change == 0 )
		return;
		
	W.needModify();
	
	unsigned long k = change;
	
	for ( int i = W.getSize()-1; i >= 0 && k; i-- )
	{
		addstep( &W[i], &k, W[i], 0 );
	}
	if ( i < 0 && k )
		W.insertDigit( k );
}

void	bui_sub_digit( bsint & W, unsigned long change )
{
	if ( change == 0 )
		return;
		
	W.needModify();
	
	unsigned long k = change;
	
	for ( int i = W.getSize()-1; i >= 0 && k; i-- )
	{
		if ( k <= W[i] )
		{
			W[i] -= k;
			k = 0;
		}
		else
		{
			W[i] += (0xFFFFFFFF-k) + 1;
			k = 1;
		}
	}
	W.removeLeadingZeros();
}


void	bui_mul_digit( bsint & W, unsigned long multiplier )
{
	if ( multiplier == 1 )
		return;
		
	W.needModify();
	
	if ( multiplier == 0 )
	{
		W.setSize(1);
		W[0] = 0;
		return;
	}
	
	unsigned long	k = 0;
	
	for ( int i = W.getSize()-1; i >= 0; i-- )
		mulstep( &W[i], &k, W[i], multiplier-1 );
	if ( k )
		W.insertDigit(k);
}


unsigned long	bui_div_digit( bsint & Q, unsigned long divisor )
{
	if ( divisor < 2 )
		return 0;

	Q.needModify();
	
	unsigned long k = 0;
	
	for ( int i = 0; i < Q.getSize(); i++ )
		div32( &Q[i], &k, k, Q[i], divisor );
		
	Q.removeLeadingZeros();
	
	return k;
}

void	bui_dump( const bsint & Qa )
{
	bsint	Q = Qa;
	for ( int index = 0; index < Q.getSize(); index++ )
	{
		if ( index )
			cout << ' ';
		for ( int i = 7; i >= 0; i-- )
		{
			cout << "0123456789abcdef"[(Q[index] >> (4*i)) & 15];
		}
	}
}

void	bui_setrandom( bsint & W, int ndigits )
{
	int		i;
	
	W.needOverwrite();
	W.setSize( ndigits );
	for ( i = 0; i < W.getSize(); i++ )
	{
		W[i] = (rand() >> 8) & 0xFF;
		W[i] <<= 8;
		W[i] |= (rand() >> 8) & 0xFF;
		W[i] <<= 8;
		W[i] |= (rand() >> 8) & 0xFF;
		W[i] <<= 8;
		W[i] |= (rand() >> 8) & 0xFF;
	}
}

void	bui_split( bsint & H, bsint & L, bsint & U, int nDigits )
{
	H.needOverwrite();
	L.needOverwrite();
	
	int		i;
	int		ui = 0;
	
	if ( nDigits < U.getSize() )
	{
		H.setSize( U.getSize() - nDigits );
		for ( i = 0; i < H.getSize(); i++,ui++ )
			H[i] = U[ui];
	}
	else
	{
		H.setSize(1);
		H[0] = 0;
	}
	L.setSize(U.getSize()-ui);
	for ( i = 0; ui < U.getSize(); i++, ui++ )
		L[i] = U[ui];
}

void	bui_pow( bsint & W, const bsint & Ua, const bsint & Va )
{
	unsigned long	r;
	bsint		U = Ua;
	bsint		V = Va;
	
	W.needOverwrite();
	U.needModify();
	V.needModify();
	
	W.setSize(1);
	W[0] = 1;
	
	while ( V.getSize() > 1 || V[0] != 0 )
	{
		r = bui_div_digit(V,2);
		if ( r )
			bui_mul1(W,W,U);
		bui_mul1(U,U,U);
	}
}

void	bui_powmod( bsint & W, const bsint & Ua, const bsint & Va, const bsint & M )
{
	unsigned long	r;
	bsint		U = Ua;
	bsint		V = Va;
	bsint		discard;
	
	W.needOverwrite();
	U.needModify();
	V.needModify();
	
	W.setSize(1);
	W[0] = 1;
	
	while ( V.getSize() > 1 || V[0] != 0 )
	{
		r = bui_div_digit(V,2);
		if ( r )
		{
			bui_mul1(W,W,U);
			bui_div(discard,W,W,M);
		}
		bui_mul1(U,U,U);
		bui_div(discard,U,U,M);
	}
}

void	bui_mod_mp( bsint & W, int p )
{
	int				i, j;
	unsigned long	k;
	
	if ( 32 * W.getSize() <= p )
		return;
	
	W.needModify();
	
	bsint	H, L;
	
doagain:
	H.setSize( (32 * W.getSize() - p + 31)/32 );
	for ( i = 0; i < H.getSize(); i++ )
		H[i] = W[i];
	L.setSize( (p+31)/32 );
	for ( i = 0; i < L.getSize(); i++ )
		L[i] = W[W.getSize() - L.getSize() + i];

	//
	// If p is not a multiple of 32, then we have extra bits in H and L
	//
	if ( (i = p%32) != 0 )
	{
		//
		// Extra bits in L are at left, so we can just mask.
		//
		L[0] &= ~(0xFFFFFFFF << i);
		
		//
		// Extra bits in H are at the right, so we have to shift
		//
		unsigned int	d = (unsigned int)1 << i;
		
		for ( j = 0, k = 0; j < H.getSize(); j++ )
			div32( &H[j], &k, k, H[j], d );
		H.removeLeadingZeros();
	}
	
	bui_add( W, H, L );
	
	i = (p+31)/32;
	if ( W.getSize() > i )
		goto again;
	if ( W.getSize() == i )
	{
		i = p % 32;
		if ( i )
			k = ~(0xFFFFFFFF << i);
		else
			k = 0xFFFFFFFF;
		for ( i = 0; i < W.getSize(); i++ )
		{
			if ( W[i] < k )
				goto done;
			if ( W[i] > k )
				goto again;
			k = 0xFFFFFFFF;
		}
		// W == 2^p-1
		W.setSize(1);
		W[0] = 0;
	}
done:
	return;
	
again:
	if ( 32 * W.getSize() <= p )
		goto done;
	goto doagain;
}
