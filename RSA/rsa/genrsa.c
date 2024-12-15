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

#include	"arith.h"
#include	"nio.h"
#include	"prim.h"
#include	"rnd.h"

main()
{
	NUMBER p1,p2,n,d,e,phi,*max_p;
	int len;
	
	num_fget( &p1, stdin ); getchar();
	num_fget( &p2, stdin );
	
	if ( !a_cmp( &p1, &p2 ) ) {
		fprintf(stderr,"the prime numbers must not be identical!\n");
		exit(1);
	}
	
	if (a_cmp( &p1, &p2 ) > 0)
		max_p = &p1;
	else
		max_p = &p2;

	a_mult( &p1, &p2, &n );
	num_fput( &n, stdout ); puts("#"); fflush(stdout);
	
	a_sub( &p1, &a_one, &phi );
	a_sub( &p2, &a_one, &e );
	a_mult( &phi, &e, &phi );
	
	len = n_bitlen( &phi );
	len = ( len + 3 ) / 4;
	
	a_assign( &p1, &phi );
	a_sub( &p1, &a_one, &p1 );
	init_rnd();
	do {
		do {
			gen_number( len, &d );
		} while (a_cmp( &d, max_p ) <= 0 || a_cmp( &d, &p1 ) >= 0);
				
		a_ggt( &d, &phi, &e );
	} while ( a_cmp( &e, &a_one ) );
	
	num_fput( &d, stdout ); puts("#"); fflush(stdout);
	
	inv( &d, &phi, &e );
	
	num_fput( &e, stdout );
}

