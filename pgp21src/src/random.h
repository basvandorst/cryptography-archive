/*	random.h - Header include file for random.c
	Last revised 15 Dec 90
	(c) 1989 Philip Zimmermann.  All rights reserved.
*/


#include "usuals.h"  /* typedefs for byte, word16, boolean, etc. */

int pseudorand(void);	/* 16-bit LCG pseudorandom generator */

/*	Don't define PSEUDORANDOM unless you want only pseudorandom numbers.
	If you do want PSEDORANDOM defined, it's better to define it right 
	here in this include file because then you can be sure that all the 
	files that include random.h will be properly affected. */
/* #define PSEUDORANDOM */

#ifdef PSEUDORANDOM		/* use pseudorandom numbers */
#define randombyte()  ((byte) pseudorand())	/* pseudorandom generator */
#define randaccum(bitcount)		/* null function */
#define randload(bitcount)	/* null function */
#define randflush()		/* null function */
#define capturecounter()	/* null function */
#define keypress() kbhit()	/* TRUE iff keyboard input ready */
#define getkey() getch()	/* returns data from keyboard (no echo). */
#endif	/* ifdef PSEUDORANDOM */

#ifndef PSEUDORANDOM		/* use truly random numbers */

extern int randcount;	/* number of random bytes accumulated in pool */

void capturecounter(void); /* capture a fast counter into the random pool. */
/* Should be called when the user clicks the mouse, or from getkey(). */

short randombyte(void);	/* returns truly random byte from pool */

int getstring(char *strbuf,int maxlen,boolean echo);

void randaccum(short bitcount);	/* get this many raw random bits ready */

short randload(short bitcount);
/* Get fresh load of raw random bits into recyclepool for key generation */

void randflush(void);	/* flush recycled random bytes */

boolean keypress(void);	/* TRUE iff keyboard input ready */
#ifndef AMIGA
short getkey(void);		/* returns data from keyboard (no echo). */
#endif /* !AMIGA */

#endif			/* ifndef PSEUDORANDOM */

