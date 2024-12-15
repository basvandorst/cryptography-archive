/*******************************************************************************
*									       *
*	Copyright (c) Martin Nicolay,  22. Nov. 1988			       *
*									       *
*	Wenn diese (oder sinngemaess uebersetzte) Copyright-Angabe enthalten   *
*	bleibt, darf diese Source fuer jeden nichtkomerziellen Zweck weiter    *
*	verwendet werden.						       *
*									       *
*	martin@trillian.megalon.de					       *
*									       *
*******************************************************************************/

#include	"prim.h"

/*
 *		RSA
 *
 *	p,q prim
 *	p != q
 *	n = p*q
 *	phi = (p -1)*(q -1)
 *	e,d aus 0...n-1
 *	e*d == 1 mod phi
 *
 *	m aus 0...n-1 sei eine Nachricht
 *
 *	Verschluesseln:
 *		E(x) = x^e mod n		( n,e oeffendlich )
 *
 *	Entschluesseln:
 *		D(x) = x^d mod n		( d geheim )
 *
 *
 *	Sicherheit:
 *
 *		p,q sollten bei mind. 10^100 liegen.
 *		(d,phi) == 1, das gilt fuer alle Primzahlen > max(p,q).
 *		Allerdings sollte d moeglichst gross sein ( d < phi-1 )
 *		um direktes Suchen zu verhindern.
 */


/*
 *		FUNKTIONEN um RSA Schluessel zu generieren.
 *
 * int	p_prim( n, m )
 *		NUMBER *n;
 *		int m;
 *			0 : n ist nicht prim
 *			1 : n ist mit Wahrscheinlichkeit (1-1/2^m) prim
 *		ACHTUNG !!!!
 *		p_prim benutzt m_init
 *
 *	inv( d, phi, e )
 *		NUMBER *d,*phi,*e;
 *			*e = *d^-1 (mod phi)
 *		ACHTUNG !!!!
 *		p_prim benutzt m_init
 */

/*
 * Prototypes
 */
static int	jak_f( NUMBER* );	
static int	jak_g( NUMBER*, NUMBER* );	
static int	jakobi( NUMBER*, NUMBER* );

/*
 * Hilfs-Funktion fuer jakobi
 */
static int jak_f( n )
NUMBER *n;
{
	int f,ret;
	
	f = n_bits( n, 3 );
	
	ret = ((f == 1) || (f == 7)) ? 1 : -1;
	
	return(ret);
}

/*
 * Hilfs-Funktuion fuer jakobi
 */	
static int jak_g( a, n )
NUMBER *a,*n;
{
	int ret;
	
	if ( n_bits( n, 2 ) == 1 ||
			n_bits( a, 2 ) == 1 )
		ret = 1;
	else
		ret = -1;
	
	return(ret);
}
		
/*
 * Jakobi-Symbol
 */
static int jakobi( a, n )
NUMBER *a,*n;
{
	NUMBER t[2];
	int at,nt, ret;
	
	a_assign( &t[0], a ); at = 0;
	a_assign( &t[1], n ); nt = 1;

	/*
	 * b > 1
	 *
	 * J( a, b) =
	 * a == 1	:	1
	 * a == 2	:	f(n)
	 * a == 2*b	:	J(b,n)*J(2,n) ( == J(b,n)*f(n) )
	 * a == 2*b -1	:	J(n % a,a)*g(a,n)
	 *
	 */
	 
	ret = 1;
	while (1) {
		if (! a_cmp(&t[at],&a_one) ) {
			break;
		}
		if (! a_cmp(&t[at],&a_two) ) {
			ret *= jak_f( &t[nt] );
			break;
		}
		if ( ! t[at].n_len )		/* Fehler :-)	*/
			abort();
		if ( t[at].n_part[0] & 1 ) {	/* a == 2*b -1	*/
			int tmp;
			
			ret *= jak_g( &t[at], &t[nt] );
			a_div( &t[nt], &t[at], NUM0P, &t[nt] );
			tmp = at; at = nt; nt = tmp;
		}
		else {				/* a == 2*b	*/
			ret *= jak_f( &t[nt] );
			a_div2( &t[at] );
		}

	}
	
	return(ret);
}
		
/*
 * Probabilistischer Primzahltest
 *
 *  0 -> n nicht prim
 *  1 -> n prim mit  (1-1/2^m) Wahrscheinlichkeit.
 *
 *	ACHTUNG !!!!!!
 *	p_prim benutzt m_init !!
 *
 */
int p_prim( n, m )
NUMBER *n;
{
	NUMBER gt,n1,n2,a;
	INT *p;
	int i,w,j;
	long lrand48();

	if (a_cmp(n,&a_two) <= 0 || m <= 0)
		abort();
		
	a_sub( n, &a_one, &n1 );	/* n1 = -1    mod n		*/
	a_assign( &n2, &n1 );
	a_div2( &n2 );			/* n2 = ( n -1 ) / 2		*/
	
	m_init( n, NUM0P );
	
	w = 1;
	for (; w && m; m--) {
				/* ziehe zufaellig a aus 2..n-1		*/
		do {
			for (i=n->n_len-1, p=a.n_part; i; i--)
				*p++ = (INT)lrand48();
			if ( i=n->n_len )
				*p = (INT)( lrand48() % ((unsigned long)n->n_part[i-1] +1) );
			while ( i && ! *p )
				p--,i--;
			a.n_len = i;
		} while ( a_cmp( &a, n ) >= 0 || a_cmp( &a, &a_two ) < 0 );

				/* jetzt ist a fertig			*/

		/*
		 * n ist nicht prim wenn gilt:
		 *	(a,n) != 1
		 * oder
		 *	a^( (n-1)/2 ) != J(a,n)   mod n
		 *
		 */
		 
		a_ggt( &a, n, &gt );
		if ( a_cmp( &gt, &a_one ) == 0 ) {

			j= jakobi( &a, n );
			m_exp( &a, &n2, &a );

			if  (   ( a_cmp( &a, &a_one ) == 0 && j ==  1 )
			     || ( a_cmp( &a, &n1    ) == 0 && j == -1 ) )
				w = 1;
			else
				w = 0;
		}
		else
			w = 0;
	}

	return( w );
}

/*
 * Berechne mulitiplikatives Inverses zu d (mod phi)
 *	d relativ prim zu phi ( d < phi )
 *	d.h. (d,phi) == 1
 *
 *	ACHTUNG !!!!
 *	inv benutzt m_init
 */
void inv( d, phi, e )
NUMBER *d,*phi,*e;
{
	int k, i0, i1, i2;
	NUMBER r[3],p[3],c;
	
	/*
	 * Berlekamp-Algorithmus
	 *	( fuer diesen Spezialfall vereinfacht )
	 */

	if (a_cmp(phi,d) <= 0)
		abort();
	
	m_init( phi, NUM0P );
	
	p[1].n_len = 0;
	a_assign( &p[2], &a_one );
	a_assign( &r[1], phi );
	a_assign( &r[2], d );
	
	k = -1;
	do {
		k++;
		i0=k%3; i1=(k+2)%3; i2=(k+1)%3;
		a_div( &r[i2], &r[i1], &c, &r[i0] );
		m_mult( &c, &p[i1], &p[i0] );
		m_add( &p[i0], &p[i2], &p[i0] );
	} while (r[i0].n_len);

	if ( a_cmp( &r[i1], &a_one ) )	/* r[i1] == (d,phi) muss 1 sein	*/
		abort();
		
	if ( k & 1 )	/* falsches ''Vorzeichen''	*/
		a_sub( phi, &p[i1], e );
	else
		a_assign( e, &p[i1] );
}
