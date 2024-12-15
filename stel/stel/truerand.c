/*
 *	Physically random numbers (very nearly uniform)
 *	D. P. Mitchell 
 * 		hacked by mab
 */
/*
 * The author of this software is Don Mitchell.
 *              Copyright (c) 1995 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#include <math.h>

static jmp_buf env;
static unsigned count;
static unsigned ocount;
static unsigned buffer;

static
tick()
{
	struct itimerval it, oit;

	timerclear(&it.it_interval);
	it.it_value.tv_sec = 0;
	it.it_value.tv_usec = 16665;
	if (setitimer(ITIMER_REAL, &it, &oit) < 0)
		perror("tick");
}

static void
interrupt()
{
	if (count)
		longjmp(env, 1);
	(void) signal(SIGALRM, interrupt);
	tick();
}

static unsigned
roulette()
{
	if (setjmp(env)) {
		count ^= (count>>3) ^ (count>>6) ^ ocount;
		count &= 0x7;
		ocount=count;
		buffer = (buffer<<3) ^ count;
		return buffer;
	}
	(void) signal(SIGALRM, interrupt);

	count = 0;
	tick();
	for (;;)
		count++;	/* about 1 MHz on VAX 11/780 */
}

unsigned
truerand()
{
	count=0;
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	return roulette();
}

int
n_truerand(n)
int n;
{
	int slop, v;

	slop = 0x7FFFFFFF % n;
	do {
		v = truerand() >> 1;
	} while (v <= slop);
	return v % n;
}

#ifdef TEST
main()
{
	printf("%u\n", truerand());
}
#endif
