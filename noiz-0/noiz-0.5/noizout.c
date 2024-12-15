/*      noizstir.c   -- stir the noise file with stdin

        Anticopyright (A) 1995 Henry Strickland <strick@yak.net>

        This package is placed this package in the public domain.

        Because this package is free, there is no warranty
        for it whatsoever.  Caveat hacker.
*/


/* 
	This file noizout.c is dedicated in memory of John Cage,
	who explained to me the point of his pointless noise. 
*/



#include <stdio.h>
#include "noiz.h"
#include "md5.h"

char noiz[256];
char pad[256];

unsigned char out_hash[16];

struct MD5Context context;

main()
{
	int e;
	int i;
	long seconds;
	long pid;
	long ppid;
	int row;

	FILE* f= fopen( NOIZ_FILE, "r" );
	if (!f) goto bad;

	for ( i=0; i<256; i++ ) {
		e= getc(f);
		if (e==EOF) goto bad;
		noiz[i]= e;
	}

	MD5Init( &context );
	MD5Update(&context, (unsigned char*) &noiz, 256);
	while ( fread(pad, 1, 256, stdin) > 0 ) {
		MD5Update(&context, (unsigned char*) &pad, 256);
	}
	time( &seconds );
	pid= getpid();
	ppid= getppid();
	MD5Update( &context, (unsigned char*) &seconds, sizeof seconds );
	MD5Update( &context, (unsigned char*) &pid, sizeof pid );
	MD5Update( &context, (unsigned char*) &ppid, sizeof ppid );
	MD5Final( out_hash, &context );

	for ( i=0; i<16; i++ ) {
		e= putchar(out_hash[i]);
		if (e==EOF) goto bad;
	}

	e= fflush(stdout);
	if (e==EOF) goto bad;
	e= fclose(stdout);
	if (e==EOF) goto bad;

	return 0;
bad:
	perror( NOIZ_FILE );
	exit(255);
}

/* END $Header: /mvp/fjord/strick/yaxen/noiz-0.5/RCS/noizout.c,v 1.3 95/02/07 15:29:00 strick Exp Locker: strick $ */
