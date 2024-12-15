/*      noizspin.c

        Anticopyright (A) 1995 Henry Strickland <strick@yak.net>

        This package is placed this package in the public domain.

        Because this package is free, there is no warranty
        for it whatsoever.  Caveat hacker.
*/

/*
	This file noizspin.c is dedicated to Matt Blaze,
	because I learned the trick from his code.
*/


#include <stdio.h>
#include <sys/types.h>
#include <signal.h>

unsigned long x;

enough()
{
	printf("%lu\n", x );
	fflush(stdout);
	exit(0);
}

main()
{
	signal( SIGALRM, enough );
	alarm(1);

	while (1) {
		++x;
	}

	/*NOTREACHED*/
	return 255;
}

/* END $Header: /mvp/fjord/strick/yaxen/noiz-0.5/RCS/noizspin.c,v 1.1 95/02/07 15:35:39 strick Exp Locker: strick $ */
