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

#include 	<string.h>
#include 	<time.h>
#include	"random.h"
#include	"language.h"
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
static int randseed=0; /* used only by pseudorand() function. */
int pseudorand(void)
/*	Home-grown 16-bit LCG pseudorandom generator. */
{	randseed = (randseed*31421 + 6927) & 0xffff;
	return (randseed);
}	/* pseudorand */


#ifndef PSEUDORANDOM		/* use truly random numbers */

/* #define USEPCTIMER */ /* use fast hardware timer on IBM PC or AT or clone */
/* #define DEBUG */

/*	If operating system shields keyboard event timings from user, then 
	we can't use any hardware or software timer logic to time events.
	This is a messy situation, so we have special logic to handle
	it, defining the symbol CRUDE to compile special code. 
	This may be needed in some multiuser Unix systems, for example.
*/
#ifdef CRUDE /* No environmental support to capture keyboard event times */
#undef USEPCTIMER
#endif	/* CRUDE */

#include	<stdio.h>	/* for putchar() and printf() */

/* Prototypes for kbhit() (whether the keyboard has been hit) and getch()
   (like getchar() but no echo, no buffering).  Not available under some
   implementations */

int kbhit( void );
int getch( void );

#ifdef DEBUG
#define DEBUGprintf1(x) fprintf(stderr,x)
#define DEBUGprintf2(x,y) fprintf(stderr,x,y)
#else
#define DEBUGprintf1(x)
#define DEBUGprintf2(x,y)
#endif


int randcount = 0 ;		/* # of random bytes accumulated in pool */
static byte randpool[256] = {0} ;	/* pool of truly random bytes */
static int recyclecount = 0 ;	/* # of recycled random bytes accumulated */
static byte recyclepool[256] = {0} ; /* pool of recycled random bytes */
static int recycleptr = 0;	/* points to next byte to grab in recyclepool */

/* fastcounter is a free-running counter incremented in main event loop. */
static unsigned fastcounter = 0;	/* not needed if we can use the PC timer. */
static unsigned lastcounter = 0;
static char toofast = 0;
/* toofast indicates keystroke rejected because it was typed too fast */

#ifdef CRUDE 
static boolean capturemode = FALSE;
#endif	/* CRUDE */

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

   The use of this randomness-distillation can be turned off by toggling the
   following define in case the randomness code needs to be run on a
   washing-machine controller or somesuch.
*/

#define USE_MD5		/* Turn on MD5 randomization */

#ifdef USE_MD5
void Transform( unsigned long *seedBuffer, unsigned long *data );
byte seedBuffer[ 64 ];			/* Buffer for MD5 seed value */
boolean isInitialised = FALSE;	/* Whether buffer has been inited */
byte iv[ 16 ];					/* IV for MD5 transformation */
#endif	/* USE_MD5 */

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

void capturecounter(void)
/*	Push a fast counter on the random stack.  Should be called when
**	the user touches a key or clicks the mouse.
*/
{
	int dt, i, j;
#ifndef USE_MD5
	static unsigned int accum = 0;
#endif /* USE_MD5 */
	static byte abits = 0;	/* number of accumulated bits in accum */
	unsigned int accum1;

#define cbitsmask ((1 << cbits)-1)

#ifdef CRUDE /* No environmental support to capture keyboard event times. */
	if (!capturemode)
		return;	/* only captures during randaccum function */
#endif	/* CRUDE */

	/*	fastcounter only contains timing information, in the form of a 
		free-running timer, either hardware or software.
		accum1 contains stuff from fastcounter and other sources,
		like the actual key the user hit.
	*/

#if defined(USEPCTIMER) /* we will use fast hardware timer on IBM PC */
#define cbits 8		/* number of bits of counter to capture each time */
	fastcounter = pctimer0();	/* capture hardware timer */
#else
#if defined(CRUDE)			/* no fast hardware timer available */
#define cbits 2
#else	/* not CRUDE */
#ifndef cbits
#define cbits 4		/* number of bits of counter to capture each time */
#endif
#ifdef VMS
#define cbits 2	/* fewer bits because VMS timer is so slow (100 Hz) */
	/* Capture fast system timer: */
	SYS$GETTIM(vms_clock_bits);
#ifdef TEST_COUNTER
	fprintf(stderr," time %10ol %10ol\n",vms_clock_bits[0],vms_clock_bits[1]);
#endif
	/* vms_clock_bits[0] and vms_ticks_per_update are 32-bits each. */
	fastcounter = vms_clock_bits[0] / vms_ticks_per_update;
#endif /* VMS */
#endif /* not CRUDE */
#endif	/* not USEPCTIMER */

#ifndef CRUDE	/* keyboard timings are available */
	dt = fastcounter - lastcounter;
	if (dt < 0)		/* timer rolled over past zero */
		dt = -dt;	/* absolute value of time elapsed */
#ifdef TEST_COUNTER
	fprintf(stderr,"<dt=%6u>\n", dt);
#endif
	if ( ((unsigned) dt) < (unsigned)(1 << (cbits + 2)))
	{
		toofast++;	/* indicate a too-fast keystroke */
		return;	/* only captures if enough time has passed */
	}
#endif /* not CRUDE */

	lastcounter = fastcounter;	/* latch timer info */

#ifdef USE_MD5
	/* Initialise the MD5 info if necessary */
	if( !isInitialised )	/* we probably shouldn't bother */
	{	memset(seedBuffer, 0, 64);
		memset(iv, 0, 16);
		isInitialised = TRUE;
	}

	/*	Add the slightly-random data to the MD5 input buffer.  Currently we
		just add a few bytes of environmental noise, but we could mix in 
		up to 512 bits worth.
	*/
	seedBuffer[ 0 ] = keybuf;	/* actual character user typed */
	seedBuffer[ 1 ] = lastcounter & 0xFF;
	seedBuffer[ 2 ] = lastcounter >> 8;
	accum1 = ( int ) clock();			/* clock ticks */
	seedBuffer[ 3 ] = accum1 & 0xFF;
	seedBuffer[ 4 ] = accum1 >> 8;
	accum1 = ( int ) time(NULL);		/* seconds */
	seedBuffer[ 5 ] = accum1 & 0xFF;
	seedBuffer[ 6 ] = accum1 >> 8;
	/* The preceding "noise quantum" has only cbits worth of randomness. */

	/* Now use MD5 to diffuse the randomness in these bits over the random
	   byte pool, raising the salinity of randomness in the pool.  The
	   transformation is carried out by "encrypting" the data in CFB mode
	   with MD5 as the block cipher */
	for(i = 0; i < sizeof(randpool); i += 16)
	{
		Transform((unsigned long *) iv, (unsigned long *) seedBuffer);
		for(j = 0; j < 16; j++)
			randpool[i+j] ^= iv[j];
		memcpy(iv, randpool+i, 16);
	}

	/*
		Now the somewhat dubious bit:  In order to make this fit in with
		the existing code, we use the existing mechanism of keeping track
		of a high-water mark in the buffer.  This is actually reasonably
		realistic, since it keeps a count of what percentage of the full
		buffer is random enough-- measuring the "molarity" of solution. 
		However when it comes time to re-randomize the buffer in
		randombyte(), it would probably be better to use the above
		transformation than the existing randombyte() code.  I've left it 
		as is to keep it as compatible as possible.
	*/

	abits += cbits;		/* Update count of bits of randomness */
	while (abits >= 8)
	{	/* We've got another byte's worth, increment the buffer pointer */
		if (randcount < sizeof(randpool))
			randcount++; /* not a byte counter-- now a molarity of randomness */
		abits -= 8;
	}
#else	/* not USE_MD5 */
	/* combine several sources to attempt maximum randomness... */
	accum1 = keybuf;		/* actual keystroke */
	accum1 += lastcounter;	/* add latched timer info */
	accum1 += clock();		/* clock ticks */
	accum1 += time(NULL);	/* seconds */

	accum1 ^= (accum1 >> cbits);	/* Fold upper and lower bits */

	/* slide some new bits into the bit accumulator: */
	accum = (accum << cbits) | (unsigned int) (accum1 & cbitsmask);
	abits += cbits;
	while (abits >= 8)	/* another byte's worth of bits accumulated */
	{	if (randcount < sizeof(randpool))
			/* take lower byte of accum */
			randpool[randcount++] = accum;
		abits -= 8;
		accum >>= 8;
	}
#endif /* not USE_MD5 */
#undef cbitsmask
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


static void randstir(void)
/* Stir up the recycled random number bin, via a pseudorandom generator */
{	int i;
	i = recyclecount;
	while (i--)
		recyclepool[i] ^= (byte) pseudorand();
	DEBUGprintf2(" Stirring %d recycled bytes. ",recyclecount);
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
	return(recyclecount*8);
}	/* randload */


void randflush(void)	/* destroys pool of recycled random numbers */
/* Ensures no sensitive data remains in memory that can be recovered later. */
{	recyclecount = sizeof (recyclepool);
	while (recyclecount)
		recyclepool[--recyclecount]=0;
	/* recyclecount is left at 0 */
	recycleptr = 0;
	memset(seedBuffer, 0, 64);
	memset(iv, 0, 16);
}	/* randflush */


short randombyte(void)
/*	Returns truly random byte from pool, or a pseudorandom value
**	if pool is empty.  It is recommended that the caller check
**	the value of randcount before calling randombyte.
*/
{	
	/* First try to get a cheap recycled random byte, if there are any. */
	if (recyclecount)	/* nonempty recycled pool */
	{	if (++recycleptr >= recyclecount)	/* ran out? */
		{	recycleptr = 0;	/* ran out of recycled random numbers */
			randstir();	/* stir up recycled bits */
		}
		return (recyclepool[recycleptr]);
	}

	/* Empty recycled pool.  Try a more expensive fresh random byte. */
	if (randcount)	/* nonempty random pool--return a very random number */
		return (randpool[--randcount]);

	/* Alas, fresh random pool is empty.  Get a pseudorandom byte.
	   Pseudorandom numbers are bad for cryptographic applications.
	   Although we will return a pseudorandom byte in the low order byte,
	   indicate error by making the result negative in the high byte.
	*/
	/* DEBUGprintf1("\007Warning: random pool empty! "); */
	return ( (pseudorand() & 0xFF) ^ (-1) );
}	/* randombyte */


boolean keypress(void)	/* TRUE iff keyboard input ready */
{	/* Accumulates random numbers by timing user keystrokes. */
#ifndef CRUDE
	static short lastkey = 0; /* used to detect autorepeat key sequences */
	static short next_to_lastkey = 0; /* allows a single repeated key */

#ifndef USEPCTIMER	/* no fast hardware timer available */
	fastcounter++;	/* used in lieu of fast hardware timer counter */
#endif	/* ifndef USEPCTIMER */

	if (keybuf & 0x100)	/* bit 8 means keybuf contains valid data */
		return( TRUE );	/* key was hit the last time thru */

	if (kbhit() == 0)	/* keyboard was not hit */
		return( FALSE );

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

	if (kbhit() == 0)	/* nothing in typahead buffer */
	{	/* don't capture counter if key repeated */
		if (keybuf != lastkey)
			capturecounter(); /* read random noise from environment */
		else if (keybuf != next_to_lastkey) /* allow single repeat */
			capturecounter();
		next_to_lastkey = lastkey;
		lastkey = keybuf;
	}
	return( TRUE );
#else	/* CRUDE */
	return(FALSE);
#endif
}	/* keypress */


short getkey(void)	/* Returns data from keyboard (no echo). */
{	/* Also accumulates random numbers via keypress(). */
#ifndef CRUDE
	while(! keypress() );		/* loop until key is pressed. */
	return( keybuf &= 0xff);	/* clear latch bit 8 */
#else
	return(keybuf = getch() & 0xff);
#endif
}	/* getkey */


#define BS	8		/* ASCII backspace */
#define CR	13		/* ASCII carriage return */
#define LF	10		/* ASCII linefeed */
#define DEL 0177    /* ASCII delete */

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

#if defined(UNIX) || defined(AMIGA) || defined(VMS)
#define NEEDBREAK
void ttycbreak();
void ttynorm();
#endif

#ifndef CRUDE	/* We can measure keyboard event times accurately. */

void randaccum(short bitcount)	/* Get this many random bits ready */
{	short nbytes;
	char c;
	int col = 0;
	unsigned long timer;
	int fasts_rejected = 0;			/* number of too-fast keystrokes */

	nbytes = min((bitcount+7)/8,sizeof(randpool));
	fasts_rejected = 0;	/* number of too-fast keystrokes */
	toofast = 0;	/* clear too-fast latch */

	if (randcount < nbytes)	/* if we don't have enough already */
	{	fprintf(stderr,
PSTR("\nWe need to generate %d random bytes.  This is done by measuring the\
\ntime intervals between your keystrokes.  Please enter some text on your\
\nkeyboard, at least %d nonrepeating keystrokes, until you hear the beep:\n"),
			nbytes-randcount, (8*(nbytes-randcount)+cbits-1)/cbits);
#ifdef NEEDBREAK
		ttycbreak();
#endif
		/* display counter to show progress */

		while (randcount < nbytes)
		{
			fprintf(stderr,"\r%3d ",nbytes-randcount);
			fflush(stderr);
			c=getkey();
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

		/* Clean up typeahead for at least 1 full second... */
		timer = time(NULL) + 1;	/* need at least 1 second of quiet */
		while ((unsigned long) time(NULL) <= timer)
		{	if (keypress())	/* user touched a key, reset timer */
			{	getkey();
				timer = time(NULL) + 1;
			}
		}

#ifdef NEEDBREAK
		ttynorm();
#endif /* !NEEDBREAK */
	}
}	/* randaccum */

#endif	/* not CRUDE */


#ifdef CRUDE	/* We cannot measure keyboard event times accurately. */

/*	If we can't measure keyboard event timings, use the actual keys 
	typed as the source of randomness.

	This is a really lousy way to make random numbers.  We will only use
	this method as a last resort, only if this machine does not allow
	keyboard events to be timed accurately.  It's better than nothing.
	The other randaccum function listed above is much better, if the
	CRUDE symbol is undefined and the environment will support it.

	We have improved this scheme somewhat by using a good hashing
	function (MD5) on the user input before capturing it in the 
	random pool.  This is done in function capturecounter().
*/

void randaccum(short bitcount)	/* Get this many random bits ready */
{	short nbytes;
	char c, flushbuf[80];
	nbytes = min((bitcount+7)/8,sizeof(randpool));

	randcount = 0;	/* start with nothing in pool. */
	capturemode = TRUE;	/* enable capture of random keys */
	fastcounter += time(0);	/* start with time of day, if available */

	fprintf(stderr,PSTR("\nWe need to generate %d random bytes.\
\nPlease enter at least %d random keystrokes, as random\
\nas you can possibly make them.\n"),
		nbytes-randcount, (8*(nbytes-randcount)+cbits-1)/cbits);
	while (randcount < nbytes)
	{	c=getkey();
		capturecounter();
	}
	/*
	 * CRUDE mode must work in normal tty mode, so we can't
	 * use keypress() to flush input.
	 */
#ifdef AMIGA
    while(keypress()) getch();
#else
	fgets(flushbuf, sizeof(flushbuf), stdin);
#endif
	fprintf(stderr,PSTR("\007 -Enough, thank you.\n"));

	capturemode = FALSE;	/* don't capture any more keys */

#ifdef NEEDBREAK
	ttynorm();
#endif
}	/* randaccum */

#endif	/* CRUDE */


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
	{	fflush(stderr);
		c = getkey();
		if (c==BS || c==DEL)
		{	if (i)
			{	if (echo)
				{	fputc(BS,stderr);
					fputc(' ',stderr);
					fputc(BS,stderr);
				}
				i--;
			}
			continue;
		}
		if (c < ' ' && c != LF && c != CR)
		{	putc('\007', stderr);
			continue;
		}
		if (echo) fputc(c,stderr);
		if (c==CR)
		{	if (echo) fputc(LF,stderr);
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

void flush_input()
{
#ifdef NEEDBREAK
	ttycbreak();
#endif
	while (keypress())	/* flush typahead buffer */
		getkey();
#ifdef NEEDBREAK
	ttynorm();
#endif
}

