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

/* files that we use if we are on autopilot */
#ifdef AUTOKEY
#define PUBLIC "public"
#define PRIVATE "secret"
#include <fcntl.h>
#endif AUTOKEY

main()
{
	NUMBER p1,p2,n,d,e,phi,*max_p;
	int len;
#ifdef AUTOKEY
        FILE *fdpub,*fdpriv;
        FILE *fd;

	if((fdpub=fopen(PUBLIC,"w"))==0) {
		perror(PUBLIC);
		exit(1);
	}
	if((fdpriv=fopen(PRIVATE,"w"))==0) {
		perror(PRIVATE);
		exit(1);
	}
	system("genprim 9 20 >ptemp");
        system("echo '#' >>ptemp");
	system("genprim 10 20 >>ptemp");

	fd=fopen("ptemp","r");
        if(fd==NULL) {
		printf("Cant open temp file\n");
		exit(1);
	}
	num_fget( &p1, fd ); getc(fd);
	num_fget( &p2, fd );
 	fclose(fd);
	system("rm -f ptemp\n");
#else
	num_fget( &p1, stdin ); getchar();
	num_fget( &p2, stdin );
#endif AUTOKEY
	
	if ( !a_cmp( &p1, &p2 ) ) {
		fprintf(stderr,"the prime numbers must not be identical!\n");
		exit(1);
	}
	
	if (a_cmp( &p1, &p2 ) > 0)
		max_p = &p1;
	else
		max_p = &p2;

	a_mult( &p1, &p2, &n );
	
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
	
	
	inv( &d, &phi, &e );
	
#ifndef AUTOKEY
	num_fput( &n, stdout ); puts("#"); fflush(stdout);
	num_fput( &d, stdout ); puts("#"); fflush(stdout);
	num_fput( &e, stdout );
#else AUTOKEY
        /* priv is first and second of above,  */
        /* public is first and third of above  */
	num_fput( &n, fdpub ); fprintf(fdpub,"#\n");
	num_fput( &n, fdpriv ); fprintf(fdpriv,"#\n");
	num_fput( &d, fdpriv );
	num_fput( &e, fdpub );

	fclose(fdpub);
	fclose(fdpriv);
#endif AUTOKEY
}

