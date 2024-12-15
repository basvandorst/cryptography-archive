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

#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>

#include	"nio.h"

/*
 *	NUMBER io
 */

/*
 *		Funktionen
 *
 * int	num_sput( n, s, l)
 *		NUMBER *n;
 *		char s[l];
 *			schreibt *n als Hex-Zahl in s
 *
 * int	num_fput( n, f )
 *		NUMBER *n;
 *		FILE *f;
 *			schreibt *n als Hex-Zahl in File f
 *
 * int	num_sget( n, s )
 *		NUMBER *n;
 *		char *s;
 *			liest Hex-Zahl s in *n ein
 *
 * int	num_fget( n, f )
 *		NUMBER *n;
 *		FILE *f;
 *			liest eine Hex-Zahl von f in *n ein
 *
 */


static char *HEX="0123456789ABCDEF";
static char *hex="0123456789abcdef";

static NUMBER bits[9];
static NUMBER int16[16];

static int init = 0;

static num_init()
{
	int i;
	
	a_assign( &bits[0], &a_one );
	for ( i=1; i<9; i++)
		a_add( &bits[i-1], &bits[i-1], &bits[i] );

	a_assign( &int16[0], &a_one );
	for ( i=1; i<16; i++)
		a_add( &int16[i-1], &a_one, &int16[i] );
	
	init = 1;
}


int num_sput( n, s, l)
NUMBER *n;
char *s;
{
#if MAXINT == ( (1 << MAXBIT) - 1 )
	INT *p;
	int bi,ab,i;
	long b;
	int first = 1;
	
	bi = MAXBIT * n->n_len;
	ab = 4 - (bi + 3) % 4 -1;
	p  = &n->n_part[n->n_len -1];

	if ( (bi+3) / 4 >= l )
		return(EOF);
	
	b  = 0;
	while (bi) {
		b <<= (MAXBIT);
		b |= (unsigned long)*p--;
		bi -= MAXBIT;
		ab += MAXBIT;
		while (ab >= 4) {
			i = (b >> (ab - 4));
			b &= ( 1L << (ab - 4) ) -1L;
			ab -= 4;

			if (first && !i)
				continue;
			first = 0;
			*s++ = HEX[ i ];
		}
	}
	if (b)
		abort();
	*s = '\0';

	return (0);
#else			
	NUMBER r,q;
	int i,b,p,len,low,high;
	char *np;
	
	if (! init)
		num_init();
	
	a_assign( &q, n);
	len = l;
	np = s + l;

	for (; q.n_len && len > 1; len --) {
		a_div( &q, &bits[4], &q, &r );
		for (p=8, b=0, i=3; i >= 0; i--, p /= 2 ) {
			if ( a_cmp( &r, &bits[i] ) >= 0 ) {
				a_sub( &r, &bits[i], &r );
				b += p;
			}
		}
		*--np = HEX[ b ];
	}
	if (q.n_len)
		return(EOF);

	l -= len;
	len = l;
	for (; l--; )
		*s++ = *np++;
	
	*s = '\0';

	return (0);
#endif
}


int num_fput( n, f )
NUMBER *n;
FILE *f;
{
	int j;
	char *np;
	unsigned char n_print[ STRLEN + 1 ];
	
	if ( num_sput( n, n_print, sizeof( n_print ) ) == EOF )
		return(EOF);
	
	for (j=0, np=(char *)n_print; *np ; np++, j++ ) {
		if (j==64) {
			fputs("\n",f);
			j = 0;
		}
		putc((int)*np,f);
	}

	if (j)
		putc('\n',f);

	return(0);
}


int num_sget( n, s )
NUMBER *n;
char *s;
{
#if MAXINT == ( (1 << MAXBIT) - 1 )
	INT *p;
	char *hp;
	int bi,ab,i;
	long b;
	int first = 1;
	
	bi = 4 * strlen(s);
	ab = MAXBIT - (bi + MAXBIT -1) % MAXBIT -1;
	i  =  (bi + MAXBIT-1) / MAXBIT;
	p  = &n->n_part[ i -1 ];
	n->n_len = i;

	if ( i > MAXLEN )
		return(EOF);
	
	b  = 0;
	while (bi > 0) {
		if ( hp= strchr( HEX, *s ) )
			i = hp - HEX;
		else if ( hp= strchr( hex, *s ) )
			i = hp - hex;
		else
			return(EOF);
		s++;
		
		b <<= 4;
		b |= (unsigned long)i;
		bi -= 4;
		ab += 4;
		while (ab >= MAXBIT) {
			i = (b >> (ab - MAXBIT));
			b &= ( 1L << (ab - MAXBIT) ) -1L;
			ab -= MAXBIT;
			if (first && !i) {
				p--;
				n->n_len--;
			}
			else {
				first = 0;
				*p-- = i;
			}
		}
	}
	if (b)
		abort();
	*s = '\0';

	return (0);
#else			
	char *p;
	int i,c;

	if (! init)
		num_init();
	
	n->n_len = 0;
	while ( (c = *s++ & 0xFF) ) {
		if ( p= strchr( HEX, c ) )
			i = p - HEX;
		else if ( p= strchr( hex, c ) )
			i = p - hex;
		else
			return(EOF);
		
		a_mult( n, &bits[4], n );
		if (i)
			a_add( n, &int16[i-1], n );
	}
	
	return(0);
#endif
}

int num_fget( n, f )
NUMBER *n;
FILE *f;
{
	int j,c;
	char *np;
	char n_print[ STRLEN + 1 ];

	np = n_print;
	j = sizeof(n_print);
	while ( (c=getc(f)) != EOF && ( isxdigit(c) || isspace(c) ) ) {
		if (isspace(c))
			continue;
		if (! --j)
			return(EOF);
		*np++ = (char)c;
	}
	*np = '\0';

	if (c != EOF)
		ungetc(c,f);
	
	if ( num_sget( n, n_print ) == EOF )
		return( EOF );
	
	return(0);
}

