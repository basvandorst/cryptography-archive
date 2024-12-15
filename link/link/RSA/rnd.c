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

#include	"rnd.h"

#include	"nio.h"

void gen_number( len, n )
NUMBER *n;
{
	char *hex = "0123456789ABCDEF" ;
	char num[ MAXLEN*MAXBIT/4 +1 ];
	char *p;
	int i,l;
	
	p=&num[ sizeof(num) -1];
	*p-- = '\0';
	
	for (l=len; l--; p-- ) {
		i = lrand48() % 16;
		*p = hex[ i ];
	}
	p++;
	
	while (len-- && *p == '0')
		p++;
	
	num_sget( n, p );
}

#ifdef AMIGA
int getpid()        /* we got no getpid on amiga */
{
	return(time((long *)0));
}
#endif

void init_rnd()
{
	long time();
	short seed[3];

	seed[0] = time((long *)0) & 0xFFFF;
	seed[1] = getpid() & 0xFFFF;
	seed[2] = (time((long *)0) >> 16) & 0xFFFF;
	(void)seed48( seed );
}	


