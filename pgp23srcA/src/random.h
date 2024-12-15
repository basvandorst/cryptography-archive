/*	random.h - Header include file for random.c
	Last revised 15 Dec 90
	(c) 1989 Philip Zimmermann.  All rights reserved.
*/


#include "usuals.h"  /* typedefs for byte, word16, boolean, etc. */


/*	Don't define PSEUDORANDOM unless you want only pseudorandom numbers.
	If you do want PSEDORANDOM defined, it's better to define it right 
	here in this include file because then you can be sure that all the 
	files that include random.h will be properly affected. */
/* #define PSEUDORANDOM */

#ifdef PSEUDORANDOM		/* use pseudorandom numbers */
#define randombyte()  ((byte) pseudorand())	/* pseudorandom generator */
#define randaccum_later(bitcount)		/* null function */
#define randaccum(bitcount)		/* null function */
#define randload(bitcount)	/* null function */
#define randflush()		/* null function */
#define capturecounter()	/* null function */
#define keypress() kbhit()	/* TRUE iff keyboard input ready */
#define getkey() getch()	/* returns data from keyboard (no echo). */
#endif	/* ifdef PSEUDORANDOM */

#ifndef PSEUDORANDOM		/* use truly random numbers */

short try_randombyte(void);	/* returns truly random byte, or -1 */
short randombyte(void);	/* returns truly random byte from pool */

int getstring(char *strbuf,int maxlen,boolean echo);

void randaccum_later(short bitcount);	/* get random bits later */
void randaccum(short bitcount);	/* get this many raw random bits ready */

short randload(short bitcount);
/* Get fresh load of raw random bits into recyclepool for key generation */

void randflush(void);	/* flush recycled random bytes */

#endif			/* ifndef PSEUDORANDOM */

/*	DJGPP includes a getkey() which works, but PGP also uses getkey() to
	accumulate random bytes.  To avoid a conflict, getkey is #defined as
	pgp_getkey.  You don't really need to know this. */
#ifdef __GO32__
    #define getkey pgp_getkey
#endif

extern void flush_input(void);
