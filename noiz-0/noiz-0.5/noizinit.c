/*      noizinit.c

        Anticopyright (A) 1995 Henry Strickland <strick@yak.net>

        This package is placed this package in the public domain.

        Because this package is free, there is no warranty
        for it whatsoever.  Caveat hacker.
*/


/*
	This file noizinit.c is dedicated to Richard Stallman,
	who taught me the joy of free software.
*/


#include <stdio.h>
#include "noiz.h"

main()
{
	int e;
	int i;

	FILE* f= fopen( NOIZ_FILE, "w" );
	if (!f) goto bad;

	for ( i=0; i<256; i++ ) {
		e= putc(i,f);
		if (e==EOF) goto bad;
	}

	e= fclose(f);
	if ( e!=0 ) goto bad;

	return 0;

bad:
	perror( NOIZ_FILE );
	exit(255);
}

/* END $Header: /mvp/fjord/strick/yaxen/noiz-0.5/RCS/noizinit.c,v 1.3 95/02/07 15:28:59 strick Exp Locker: strick $ */
