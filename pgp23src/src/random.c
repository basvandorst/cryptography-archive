/**********************************************************************
	random.c - C source code for random number generation - 19 Nov 86
	(c) Copyright 1986 by Philip Zimmermann.  All rights reserved.  

	Revised Jul 88 by PRZ and again Dec 88 by Allan Hoeltje
		to use IBM PC 8253 timer0 for a faster counter.
	Revised Apr 89 by PRZ to recycle random bytes.
	Revised 29 Jul 91 by PRZ for use in more limited environments.
	Later revised by several other folks to run in difficult environments
	such as Unix, VAX/VMS and others.
	Revised 2 Sep 92 by Peter Gutmann and PRZ to use MD5 to distill 
	high quality randomness down from low-grade random noise.
	Revised 25 Feb 93 by Colin Plumb to use MD5 much more for
	more secure random numbers.  (randstir hacked)
	Revised 4 Mar 93 by Colin Plumb to remove the CRUDE and non-USE_MD5
	options, as no platform currently uses them, they make the code
	hard to read, and they were suffering bit rot.  Also added
	NO_ITIMER for the isc port and randaccum_later.

	This code generates truly random numbers derived from a counter that is 
	incremented continuously while the keyboard is scanned for user input.
	Every time the user touches a key, the least significant bits of the 
	counter are pushed on a stack.  Later, this supply of random bytes can
	be popped off the stack by applications requiring stochastic numbers.
	Cryptographic applications require this kind of randomness.

	The only requirement to make this work is that keypress must be called 
	frequently, and/or getkey must be called to read the keyboard.  

	Note that you can only get as many random bytes as the number of
	bytes accumulated by the user touching the keyboard.
**********************************************************************/

#ifdef UNIX
#include <sys/types.h>
#include <sys/time.h>

#if defined(linux)
/* linux >= 0.99.9 has microsecond resolution gettimeofday() */
#define	USE_GETTIMEOFDAY
#endif

#ifdef NO_ITIMER	/* Some systems lie about itimer */
#undef ITIMER_REAL
#endif

#ifndef ITIMER_REAL
#include <sys/times.h>
#endif
#endif /* UNIX */

#if defined(UNIX) || defined(VMS)
#include <signal.h>
#endif

#include	<stdio.h>	/* for putchar() and printf() */
#include 	<string.h>
#ifndef _BSD
#include 	<time.h>
#endif
#include	"system.h"
#include	"random.h"
#include	"language.h"

static long pseudorand(void);	/* 31-bit LCG pseudorandom generator */

#ifdef  M_XENIX
long time();
#endif
#ifdef UNIX
#define	clock	Clock
#endif

/*	pseudorand() is used mainly for debugging while porting to a new
	machine-- it makes reproducible sequences, which makes debugging
	easier.  To use it as the source of random numbers, define
	PSEUDORANDOM.  Warning:  Don't do this for actual applications.
*/
/**
 ** Minimal Standard Pseudo-Random Number Generator
 **
 ** Author: Fuat C. Baran, Columbia University, 1988
 **
 ** Based on code in "Random Number Generators: Good Ones are Hard to Find",
 ** by Stephen K. Park and Keith W. Miller in Communications of the ACM,
 ** 31, 10 (Oct. 1988) pp. 1192-1201.
 **
 **/

/* some constants we need */
#define A 16807L
#define M 2147483647L		/* Mersenne prime 2^31 -1 */
#define Q 127773L			/* M div A (M / A) */
#define R 2836L				/* M mod A (M % A) */

static long pseudorand(void)
{
	long hi, lo;
#ifdef DEBUG
	static long seed = 1;
#else
	static long seed = 0;
#endif
	if (!seed || seed == M)
#ifdef UNIX
		seed = ((long) getpid() << 16) ^ time(NULL);
#else
		seed = ((long) clock() << 16) ^ time(NULL);
#endif
	hi = seed / Q;
	lo = seed % Q;
	if ((seed = A * lo - R * hi) <= 0)
		seed += M;
	return seed;
}


#ifndef PSEUDORANDOM		/* use truly random numbers */

/* #define USEPCTIMER */ /* use fast hardware timer on IBM PC or AT or clone */
/* #define DEBUG */

/* Prototypes for kbhit() (whether the keyboard has been hit) and getch()
   (like getchar() but no echo, no buffering).  Not available under some
   implementations */
#if defined(MSDOS) && !defined(__GO32__)
#include <conio.h>
typedef word16 fastcnt_t;
#else
typedef word32 fastcnt_t;
#endif /* !MSDOS || __GO32__ */

#ifdef VMS
int putch(int);
#else
#define	putch(c)	putc(c, stderr)
#endif

#ifdef DEBUG
#define DEBUGprintf1(x) fprintf(stderr,x)
#define DEBUGprintf2(x,y) fprintf(stderr,x,y)
#else
#define DEBUGprintf1(x)
#define DEBUGprintf2(x,y)
#endif

#define POOLSIZE 256
static int randcount = 0;		/* # of random bytes accumulated in pool */
static byte randpool[POOLSIZE] = {0} ;	/* pool of truly random bytes */
static int recyclecount = 0 ;	/* # of recycled random bytes accumulated */
static byte recyclepool[POOLSIZE] = {0} ; /* pool of recycled random bytes */

static int accumpending = 0;	/* # of random bits to add to next request */

static void randstir(byte *pool);

/* fastcounter is a free-running counter incremented in main event loop. */
static fastcnt_t fastcounter = 0;	/* not needed if we can use the PC timer. */
static fastcnt_t lastcounter = 0;
static int cbits;
static char toofast = 0;
/* toofast indicates keystroke rejected because it was typed too fast */

#ifdef AMIGA
int aecho;
#endif

/* Information needed by the MD5 code for distilling large quantities of
   semi-random information into a small amount of highly-random information.
   This works as follows:

   Initially we have no random information.  Whenever we add more slightly
   random data, we put it in the MD5 data field and run MD5 over the
   random byte pool.  This propagates the randomness in the data evenly
   throughout the pool due to the avalanche effect of the MD5 transformation.
   The randomness transformation is carried out inside capturecounter() when
   the data is entered, and is transparent to the operation of the rest of
   the code.
*/

#include "md5.h"
static byte seedBuffer[ 64 ];			/* Buffer for MD5 seed value */
static boolean isInitialised = FALSE;	/* Whether buffer has been inited */
static byte iv[ 16 ];					/* IV for MD5 transformation */

#ifdef USEPCTIMER	/* we will use fast hardware timer on IBM PC */
/* #include <conio.h> */	/* function definitions for inp() and outp() */
/* outp() and inp() works only for Microsoft C for IBM PC or AT */
/* timer0 on 8253-5 on IBM PC or AT tics every .84 usec. */
#define timer0		0x40	/* 8253 timer 0 port */
#define timercntl	0x43	/* 8253 control register */
#define timer0rwl	0x00	/* read lo/hi bytes of cntr 2 with latch */
#define timer0rnl	0x30	/* read lo/hi bytes of cntr 2 w/o latch */

static byte latched_hitimer = 0; /* captured by keyboard ISR */
static byte latched_lotimer = 0; /* captured by keyboard ISR */
/* when kbisr captures timer, timer_latched is set. */
static boolean timer_latched = FALSE;

static void kbisr(void)	/* Keyboard Interrupt Service Routine (ISR) */
/*
	kbisr should be called on the way into, or on the way out of,
	or from within the DOS keyboard ISR, as long as it gets called
	at the time of a keyboard interrupt.  Assumes that the real
	DOS keyboard ISR captures the keystroke in the normal way.
	Only the hardware timer counter is captured by the kbisr routine,
	leaving the actual keystroke capture to the normal DOS keyboard ISR.
	We indicate that a timer capture has taken place by setting 
	timer_latched.

	NOTE: WE STILL NEED TO FIND A WAY to connect this subroutine with the 
	normal keyboard ISR, so that kbisr gets called when there's a keyboard
	interrupt.
*/
{	outp(timercntl,timer0rwl);
	latched_lotimer = inp(timer0);
	latched_hitimer = inp(timer0);
	timer_latched = TRUE;
}	/* kbisr */

static unsigned short pctimer0(void)
{
/*	Reads and returns the hardware 8253 timer0 on the PC or AT
**	or clone, shifted right 1 bit.
**
**	DO NOT SET THE HARDWARE COUNTER TO ZERO. It is already initialized
**	by the system to be used by the clock.  It is set up in mode 3
**	(square wave rate generator) and counts down by 2 from 0 (0xFFFF+1)
**	to produce an 18.2 Hz square wave.  We may, however, READ the
**	lo and hi bytes without causing any problems.  BUT just
**	remember that the lo byte will always be even (since it is
**	counting by two).
**
**	Note that we can not use counter 1 since it is tied to the
**	dynamic RAM refresh hardware.  Counter 2 is tied to the 8255
**	PPI chip to do things like sound.  Though it would be safe to
**	use counter 2 it is not desirable since we would have to turn
**	the speaker on in order to make the timer count!  Normally one
**	sets counter 2 to mode 3 (square wave generator) to sound the
**	speaker.  You can set mode 2 (pulse generator) and the speaker
**	hardly makes any sound at all, a click when you turn it on and
**	a click when you turn it off.  Counter 0 should be safe if
**	we only read the counter bytes.
**
**	WARNING:  To use the hardware timer the way it really should be
**	used, we ought to capture it via a keyboard interrupt service
**	routine (ISR).	Otherwise, we may experience weaknesses in randomness
**	due to harmonic relationships between the hardware counter frequency
**	and the keyboard software polling frequency.  Unfortunately, this
**	implementation does not currently use keyboard interrupts to
**	capture the counter.  This is not a problem if we don't use the
**	hardware counter, but instead use the software counter fastcounter.
**	Thus, the hardware counter should not be used at all, unless we
**	support it with an ISR.
*/
	unsigned short t ;
	/* See if timer has been latched by kbisr(). */
	if (!timer_latched) /* The timer was not already latched. */
		kbisr();	/* latch timer */
	/* return latched timer and clear latch */
	t = ( 	(((unsigned short) latched_hitimer) << 8) |
		 ((unsigned short) latched_lotimer)
		) >> 1 ;
	timer_latched = FALSE;
	return (t) ;
}	/* pctimer0 */

#endif	/* ifdef USEPCTIMER */


/* keybuf is used only by keypress(), getkey(), and capturecounter(). */
static short keybuf = 0;


#ifdef VMS

extern unsigned long	vms_clock_bits[2];	/* VMS Hardware Clock */
extern const long	vms_ticks_per_update;	/* Clock update int. */

#endif /* VMS */

static void capturecounter(void)
/*	Push a fast counter on the random stack.  Should be called when
**	the user touches a key or clicks the mouse.
*/
{
	fastcnt_t dt;
#ifndef MSDOS
	byte *random_xtra;	/* extra timer info */
#endif
	static byte abits = 0;	/* number of accumulated bits in accum */
	unsigned int accum1;

#define cbitsmask ((1 << cbits)-1)

	/*	fastcounter only contains timing information, in the form of a 
		free-running timer, either hardware or software.
		accum1 contains stuff from fastcounter and other sources,
		like the actual key the user hit.
	*/

#if defined(USEPCTIMER) /* we will use fast hardware timer on IBM PC */
	fastcounter = pctimer0();	/* capture hardware timer */
#endif	/* not USEPCTIMER */

#ifdef VMS
#define	HW_TIMER	1		/* using hardware timer */
	/* Capture fast system timer: */
	SYS$GETTIM(vms_clock_bits);
#ifdef TEST_COUNTER
	fprintf(stderr," time %10ol %10ol\n",vms_clock_bits[0],vms_clock_bits[1]);
#endif
	/* vms_clock_bits[0] and vms_ticks_per_update are 32-bits each. */
	fastcounter = vms_clock_bits[0] / vms_ticks_per_update;
#endif /* VMS */

#ifdef UNIX
#define	HW_TIMER	1		/* using hardware timer */
#ifdef USE_GETTIMEOFDAY
	{	static struct timeval tv;
		gettimeofday(&tv, NULL);
		/* divide by 64 (we can at least expect a 60Hz clock) */
		fastcounter = tv.tv_usec / (1000000/64) + tv.tv_sec * 64;
		random_xtra = (byte *) &tv;
#define	XTRA_BYTES	(sizeof(struct timeval))
	}
#else /* !USE_GETTIMEOFDAY */
#ifdef ITIMER_REAL
	{	static struct itimerval it;
		getitimer(ITIMER_REAL, &it);
		if (it.it_value.tv_sec == 0 && it.it_value.tv_usec == 0)
		{	/* start the timer */
			it.it_value.tv_sec = 100000;
			it.it_value.tv_usec = 0;
			it.it_interval.tv_sec = 100000;
			it.it_interval.tv_usec = 0;
			signal(SIGALRM, SIG_IGN);	/* just in case.. */
			setitimer(ITIMER_REAL, &it, NULL);
			return;
		}
		/* divide by 64 (we can at least expect a 60Hz clock) */
		fastcounter = 6400000-(it.it_value.tv_usec / (1000000/64)
				+ it.it_value.tv_sec * 64);
		random_xtra = (byte *) &it.it_value;
#define	XTRA_BYTES	(sizeof(struct timeval))
	}
#else /* !ITIMER_REAL */
	{	static struct tms tms;
		fastcounter = times(&tms);
		random_xtra = (byte *) &tms;
#define	XTRA_BYTES	(sizeof(struct tms))
	}
#endif /* !ITIMER_REAL */
#endif /* !USE_GETTIMEOFDAY */
#endif /* UNIX */

	dt = fastcounter - lastcounter;
#ifdef TEST_COUNTER
	fprintf(stderr,"%6lu", dt);
#endif
#ifndef NO_CBIT_STRIP
	dt /= 6;	/* use 2.5 bits less than the actual count */
#endif
	for (cbits = 0; dt; ++cbits)
		dt >>= 1;
#if 0	/* I don't think this is necessary - CP */
	if (cbits > 8)
		cbits = 8;
#endif

#ifdef TEST_COUNTER
	fprintf(stderr,"%6u\n", cbits);
#endif
	if (cbits <= 0)
	{
		toofast++;	/* indicate a too-fast keystroke */
		return;	/* only captures if enough time has passed */
	}

	lastcounter = fastcounter;	/* latch timer info */

	/* Initialise the MD5 info if necessary */
	if( !isInitialised )	/* we probably shouldn't bother */
	{	memset(seedBuffer, 0, 64);
		memset(iv, 0, 16);
		isInitialised = TRUE;
	}

	/*	Add the slightly-random data to the MD5 input buffer.  Currently we
		just add a few bytes of environmental noise, but we could mix in 
		up to 512 bits worth.  This should be extended in a system-specific
		manner.
	*/
	seedBuffer[ 0 ] = keybuf;	/* actual character user typed */
	seedBuffer[ 1 ] = lastcounter & 0xFF;
	seedBuffer[ 2 ] = lastcounter >> 8;
#ifdef MSDOS
	accum1 = ( int ) clock();			/* clock ticks */
	seedBuffer[ 3 ] = accum1 & 0xFF;
	seedBuffer[ 4 ] = accum1 >> 8;
#endif
#ifndef USE_GETTIMEOFDAY
	accum1 = ( int ) time(NULL);		/* seconds */
	seedBuffer[ 5 ] = accum1 & 0xFF;
	seedBuffer[ 6 ] = accum1 >> 8;
#endif /* !USE_GETTIMEOFDAY */
#ifdef XTRA_BYTES	/* add some extra "random" stuff */
	memcpy(seedBuffer+16, random_xtra, XTRA_BYTES);
#endif
	/* The preceding "noise quantum" has only cbits worth of randomness. */
	/* Spread it uniformly across the entire pool */

	randstir(randpool);

	/*
		Now the somewhat dubious bit:  In order to make this fit in with
		the existing code, we use the existing mechanism of keeping track
		of a high-water mark in the buffer.  This is actually reasonably
		realistic, since it keeps a count of what percentage of the full
		buffer is random enough-- measuring the "molarity" of solution. 
	*/

	abits += cbits;		/* Update count of bits of randomness */
	while (abits >= 8)
	{	/* We've got another byte's worth, increment the buffer pointer */
		if (randcount < sizeof(randpool))
			randcount++; /* not a byte counter-- now a molarity of randomness */
		abits -= 8;
	}
}	/* capturecounter */


/* Because these truly random bytes are so unwieldy to accumulate,
   they can be regarded as a precious resource.  Unfortunately,
   cryptographic key generation algorithms may require a great many
   random bytes while searching about for large random prime numbers.
   Fortunately, they need not all be truly random.  We only need as
   many truly random bytes as there are bytes in the large prime we
   are searching for.  These random bytes can be recycled and modified
   via pseudorandom numbers until the key is generated, without losing
   any of the integrity of randomness of the final key.
*/


static void randstir(byte *pool)
/* Stir up the recycled random number bin, via a pseudorandom generator
   Any truly random bytes stored into the seedBuffer are hashed into the
   pool, and then destroyed for security reasons. */
{	int i;
	int j;
	/* Now use MD5 to diffuse the randomness in these bits over the random
	   byte pool, raising the salinity of randomness in the pool.  The
	   transformation is carried out by "encrypting" the data in CFB mode
	   with MD5 as the block cipher */
	for(i = 0; i < POOLSIZE; i += 16)
	{
		Transform((UINT4 *) iv, (UINT4 *) seedBuffer);
		for(j = 0; j < 16; j++)
			pool[i+j] ^= iv[j];
		memcpy(iv, pool+i, 16);
	}
/* CFB encryption is reversible; if we want the stirring operation to
   be strictly one-way, we must destroy the key.  The operation
   accomplishes that, and increases the end-to-beginning feedback
   in randstir(), in an attempt to make all bytes depend on all
   other bytes. */
	memcpy(seedBuffer, pool+POOLSIZE-sizeof(seedBuffer),
	       sizeof(seedBuffer));
}	/* randstir */


short randload(short bitcount)
/*	Flushes stale recycled random bits and copies a fresh supply of raw 
	random bits from randpool to recyclepool.  Returns actual number of 
	bits transferred. */
{	int bytecount;
	bytecount = (bitcount+7)/8;
	bytecount = min(bytecount,randcount);
	randflush();	/* reset recyclecount, discarding recyclepool */
	while (bytecount--)
		recyclepool[recyclecount++] = randpool[--randcount];
	DEBUGprintf2("\nAllocating %d recycleable random bytes. ",recyclecount);
	randstir(recyclepool);
	recyclecount = sizeof(recyclepool);
	return(recyclecount*8);
}	/* randload */


void randflush(void)	/* destroys pool of recycled random numbers */
/* Ensures no sensitive data remains in memory that can be recovered later. */
{	
/* We do not actually destroy the random bytes (which would be somewhat
   wasteful); we just stir them so that they are irretrievable.  This
   assumes that MD5 is truly one-way. */
	randstir(recyclepool);
	recyclecount = 0;
}	/* randflush */

short try_randombyte(void)
/* Returns a truly random byte if any are available, or -1 if none.
 * it is not an error to call this if none are available.  For example,
 * make_random_ideakey calls it to add to its other sources of pseudo-random
 * noise.
 */
{
	/* First try to get a cheap recycled random byte, if there are any. */
	if (recyclecount)	/* nonempty recycled pool */
	{
		byte b;
		b = recyclepool[--recyclecount];
		if (!recyclecount)	/* Used them all up? */
			randstir(recyclepool);	/* Refresh recycled pool */
		return b;
	}

	/* Empty recycled pool.  Try a more expensive fresh random byte. */

	if (randcount)	/* nonempty random pool--return a very random number */
		return (randpool[--randcount]);

	/* No, all is for naught.  Return -1. */
	return -1;
}

short randombyte(void)
/*	Returns truly random byte from pool, or a pseudorandom value
**	if pool is empty.  It is recommended that the caller check
**	the value of randcount before calling randombyte.
*/
{	
	short r;

	r = try_randombyte();
	if (r >= 0)
		return r;

	/* Oops!  Random pool empty.  Were there some unsatisifed deferred
	   accumulation requests? */
	randaccum(0);
	r = try_randombyte();
	if (r >= 0)
		return r;
	
	/* Alas, fresh random pool is truly empty.  Complain, and return
	   a pseudorandom byte.  Pseudorandom numbers are bad for
	   cryptographic applications.  Although we will return a
	   pseudorandom byte in the low order byte, indicate error by
	   making the result negative in the high byte.
	*/
	fprintf(stderr, " \007WARNING: Random pool empty! ");
	return ( (pseudorand() & 0xFF) ^ (-1) );
}	/* randombyte */


static boolean keypress(void)	/* TRUE iff keyboard input ready */
{	/* Accumulates random numbers by timing user keystrokes. */
	static short lastkey = 0; /* used to detect autorepeat key sequences */
	static short next_to_lastkey = 0; /* allows a single repeated key */

#ifndef HW_TIMER	/* no fast hardware timer available */
	fastcounter++;	/* used in lieu of fast hardware timer counter */
#endif	/* ifndef HW_TIMER */

	if (keybuf & 0x100)	/* bit 8 means keybuf contains valid data */
		return( TRUE );	/* key was hit the last time thru */

	/*
	 * If HW_TIMER is defined we don't need a busy loop, so keypress
	 * waits for a key and will always return TRUE.
	 */
#ifndef HW_TIMER
	if (kbhit() == 0)	/* keyboard was not hit */
		return( FALSE );
#endif

	keybuf = getch() | 0x100; /* set data latch bit */

	/* Keyboard was hit.  Decide whether to call capturecounter... */

	/*  Guard against typahead buffer defeating fastcounter's randomness.
	**  This could be a problem for multicharacter sequences generated
	**  by a function key expansion or by the user generating keystrokes
	**  faster than our event loop can handle them.  Only the last
	**  character of a multicharacter sequence will trigger the counter
	**  capture.  Also, don't let the keyboard's autorepeat feature
	**  produce nonrandom counter capture.  However, we do allow a 
	**  single repeated character to trigger counter capture, because
	**  many english words have double letter combinations, and it's 
	**  unlikely a typist would exploit the autorepeat feature to
	**  type a simple double letter sequence.
	**  We have some separate checks in capturecounter() to guard
	**  against other reasons for characters coming in too fast.
	*/

#ifndef HW_TIMER
	if (kbhit() == 0)	/* nothing in typahead buffer */
#endif
	{	/* don't capture counter if key repeated */
		if (keybuf != lastkey)
			capturecounter(); /* read random noise from environment */
		else if (keybuf != next_to_lastkey) /* allow single repeat */
			capturecounter();
		next_to_lastkey = lastkey;
		lastkey = keybuf;
	}
	return( TRUE );
}	/* keypress */


static short getkey(void)	/* Returns data from keyboard (no echo). */
{	/* Also accumulates random numbers via keypress(). */
	while(! keypress() )		/* loop until key is pressed. */
		;
	return( keybuf &= 0xff);	/* clear latch bit 8 */
}	/* getkey */


#define BS	8		/* ASCII backspace */
#define CR	13		/* ASCII carriage return */
#define LF	10		/* ASCII linefeed */
#define DEL 0177    /* ASCII delete */

/* Since getting random bits from the keyboard requires user attention,
   we buffer up requests for them until we can do one big request. */

void randaccum_later(short bitcount)
{
	accumpending += bitcount;	/* Wow, that was easy! :-) */
}

/* We will need a series of truly random bits for key generation.
   In most implementations, our random number supply is derived from
   random keyboard delays rather than a hardware random number
   chip.  So we will have to ensure we have a large enough pool of
   accumulated random numbers from the keyboard.  Later, randombyte
   will return bytes one at a time from the accumulated pool of
   random numbers.  For ergonomic reasons, we may want to prefill
   this random pool all at once initially.  The randaccum function
   prefills a pool of random bits.
*/

void randaccum(short bitcount)	/* Get this many random bits ready */
{	short nbytes;
#ifndef HW_TIMER
	unsigned long timer;
#endif
	int fasts_rejected = 0;			/* number of too-fast keystrokes */

	bitcount += accumpending;	/* Do deferred accumulation now */
	accumpending = 0;
	nbytes = min((bitcount+7)/8,sizeof(randpool));
	fasts_rejected = 0;	/* number of too-fast keystrokes */
	toofast = 0;	/* clear too-fast latch */

	if (randcount < nbytes)	/* if we don't have enough already */
	{	fprintf(stderr,
PSTR("\nWe need to generate %d random bytes.  This is done by measuring the\
\ntime intervals between your keystrokes.  Please enter some random text\
\non your keyboard until you hear the beep:\n"), nbytes-randcount);
#ifdef NEEDBREAK
		ttycbreak();
#endif
		/* display counter to show progress */

		while (randcount < nbytes)
		{
#ifndef TEST_COUNTER
			fprintf(stderr,"\r%3d ",nbytes-randcount);
#endif
			fflush(stderr);
			getkey();
			if (toofast)
			{	fasts_rejected++;	/* keep score */
				toofast = 0;		/* reset latch */
				putc('?', stderr);	/* indicate trouble */
			}
			else 
				putc('.', stderr);
		}

		fprintf(stderr,PSTR("\007*\n-Enough, thank you.\n"));

		if (fasts_rejected > 2)	/* Did user type too fast? */
			fprintf(stderr,PSTR("(Ignored %d keystrokes that were typed too fast.)\n"), 
				fasts_rejected);

#ifdef HW_TIMER	/* keypress() would block if using hardware timer */
		sleep(1);
#else
		/* Clean up typeahead for at least 1 full second... */
		timer = time(NULL) + 1;	/* need at least 1 second of quiet */
		while ((unsigned long) time(NULL) <= timer)
		{	if (keypress())	/* user touched a key, reset timer */
			{	getkey();
				timer = time(NULL) + 1;
			}
		}
#endif

#ifdef NEEDBREAK
		ttynorm();
#endif /* !NEEDBREAK */
	}
}	/* randaccum */


int getstring(char *strbuf, int maxlen, boolean echo)
/*	Gets string from user, with no control characters allowed.
	Also accumulates random numbers by calling getkey().
	maxlen is max length allowed for string.
	echo is TRUE iff we should echo keyboard to screen.
	Returns null-terminated string in strbuf.
*/
{	short i;
	char c;
#ifdef NEEDBREAK
	ttycbreak();
#endif
#ifdef AMIGA
    aecho = (int)echo;
    echo = FALSE;   /* echo is done in getch */
#endif  /* AMIGA */
	fflush(stdout);
	i=0;
	while (TRUE)
	{	
#ifndef VMS
		fflush(stderr);
#endif /* VMS */
		c = getkey();
#ifdef VMS
		if (c == 25) {  /*  Control-Y detected */
		    ttynorm();
		    breakHandler(SIGINT);
		}
#endif /* VMS */
		if (c==BS || c==DEL)
		{	if (i)
			{	if (echo)
				{	putch(BS);
					putch(' ');
					putch(BS);
				}
				i--;
			}
			continue;
		}
		if (c < ' ' && c != LF && c != CR)
		{	putch('\007');
			continue;
		}
		if (echo) putch(c);
		if (c==CR)
		{	if (echo) putch(LF);
			break;
		}
		if (c==LF)
			break;
		if (c=='\n')
			break;
		strbuf[i++] = c;
		if (i>=maxlen)
		{	fprintf(stderr,"\007*\n");	/* -Enough! */
			while (keypress())
				getkey();	/* clean up any typeahead */
			break;
		}
	}
	strbuf[i] = '\0';	/* null termination of string */
#ifdef NEEDBREAK
	ttynorm();
#endif
	return(i);		/* returns string length */
}	/* getstring */


#endif			/* ifndef PSEUDORANDOM */

void flush_input(void)
{	/* on unix ttycbreak() will flush the input queue */
#ifndef HW_TIMER
	while (keypress())	/* flush typahead buffer */
		getkey();
#endif
}

