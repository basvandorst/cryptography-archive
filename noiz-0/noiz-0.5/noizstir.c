/*      noizstir.c   -- stir the noise file with stdin

        Anticopyright (A) 1995 Henry Strickland <strick@yak.net>

        This package is placed this package in the public domain.

        Because this package is free, there is no warranty
        for it whatsoever.  Caveat hacker.
*/


/*
	This file noizstir.c is dedicated to Jacques Atali,
	in honor of his book "Noise, The Political Economy
	of Music."
*/


#include <stdio.h>
#include "noiz.h"
#include "md5.h"

char noiz[256];
char pad[256];

unsigned char in_hash[16];
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

	FILE* f= fopen( NOIZ_FILE, "r+" );
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
	MD5Final( in_hash, &context );

	for (row=0; row<16; row++ ) {
		MD5Init( &context );
		MD5Update( &context, noiz+16*row, 16 );
		MD5Update( &context, in_hash, 16 );
		MD5Update( &context, in_hash, row+1 );
		MD5Final( out_hash, &context );
		for ( i=0; i<16; i++ ) {
			noiz[16*row+i] ^= out_hash[i];
		}
	}

	fflush(f);
	rewind(f);
	
	for ( i=0; i<256; i++ ) {
		e= putc(noiz[i],f);
		if (e==EOF) goto bad;
	}

	e= fclose(f);
	if (e==EOF) goto bad;

	return 0;

bad:
	perror( NOIZ_FILE );
	exit(255);
}

/* END $Header: /mvp/fjord/strick/yaxen/noiz-0.5/RCS/noizstir.c,v 1.3 95/02/07 15:29:01 strick Exp Locker: strick $ */
