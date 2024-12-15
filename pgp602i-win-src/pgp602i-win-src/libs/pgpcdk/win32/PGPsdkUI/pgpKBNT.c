/*
 * MS-DOS non-echoing keyboard routines.
 *
 * $Id: pgpKBNT.c,v 1.1 1998/06/16 21:34:21 elowe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>	/* For getch() and kbhit() */
#include <signal.h>	/* For raise() */

#include "pgpBase.h"

#ifdef UNITTEST
# define  PGPGlobalRandomPoolAddKeystroke(c) (void)c
#else
# include "pgpRandomPool.h"	/* For PGPGlobalRandomPoolAddKeystroke() */
#endif

#include "pgpKB.h"

/* These are pretty boring */
void kbCbreak(int x) { return; }
void kbNorm(void) { return; }

int kbGet(void)
{
	int c;

	while (!_kbhit()) ;
	c = _getch();
	if (c == 0)
		c = 0x100 + _getch();

	/*
	 * Borland C's getch() uses int 0x21 function 0x7,
	 * which does not detect break.  So we do it explicitly.
	 */
	if (c == 3)
		raise(SIGINT);

	 PGPGlobalRandomPoolAddKeystroke(c);

	return c;
}

void kbFlush(int thorough)
{
	do {
		while(_kbhit())
			(void)_getch();
		if (!thorough)
			break;
		/* Extra thorough: wait for one second of quiet */
		_sleep(1000);
	} while (_kbhit());
}

#if UNITTEST    /* Self-contained test driver */

#include <ctype.h>

int
main(void)
{
    int c;

    puts("Going to cbreak mode...");
    kbCbreak(0);
    puts("In cbreak mode.  Please type.");
    for (;;) {
        c = kbGet();
        if (c == '\n' || c == '\r')
            break;
        printf("c = %d = '%c'\n", c, c);
        kbFlush(isupper(c));
    }
    puts("Returning to normal mode...");
    kbNorm();
    puts("Done.");
    return 0;
}

#endif /* UNITTEST */

