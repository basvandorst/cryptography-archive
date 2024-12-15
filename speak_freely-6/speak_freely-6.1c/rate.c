
/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained. 
 * Lance Norskog And Sundry Contributors are not responsible for 
 * the consequences of using this software.
 */

/*

 * Modified for use in Speak Freely by John Walker

 * Sound Tools rate change effect file.
 */

#include "speakfree.h"

/*
 * Least Common Multiple Linear Interpolation 
 *
 * Find least common multiple of the two sample rates.
 * Construct the signal at the LCM by interpolating successive
 * input samples as straight lines.  Pull output samples from
 * this line at output rate.
 *
 * Of course, actually calculate only the output samples.
 *
 * LCM must be 32 bits or less.  Two prime number sample rates
 * between 32768 and 65535 will yield a 32-bit LCM, so this is 
 * stretching it.
 */

/*
 * Algorithm:
 *  
 *  Generate a master sample clock from the LCM of the two rates.
 *  Interpolate linearly along it.  Count up input and output skips.
 *
 *  Input:   |inskip |	     |	     |	     |	     |
 *									
 *									
 *									
 *  LCM:     |	 |   |	 |   |	 |   |	 |   |	 |   |
 *									
 *									
 *									
 *  Output:  |	outskip  |	     |		 | 
 *
 *									
 */

typedef unsigned int u_i;
typedef unsigned long u_l;
typedef unsigned short u_s;

/* Private data for Lerp via LCM file */
typedef struct ratestuff {
	u_l	lcmrate;		/* least common multiple of rates */
	u_l	inskip, outskip;	/* LCM increments for I & O rates */
	u_l	total;
	u_l	intot, outtot;		/* total samples in terms of LCM rate */
	long	lastsamp;
} rate_t;

static rate_t rt;
static rate_t *rate = &rt;

/* here for linear interp.  might be useful for other things */

static long gcd(a, b) 
long a, b;
{
	if (b == 0)
		return a;
	else
		return gcd(b, a % b);
}

static long lcm(a, b) 
long a, b;
{
	return (a * b) / gcd(a, b);
}

/*
 * Prepare processing.
 */
void rate_start(inrate, outrate)
  int inrate, outrate;
{
	rate->lcmrate = lcm((long) inrate, (long) outrate);
	/* Cursory check for LCM overflow.  
	 * If both rate are below 65k, there should be no problem.
	 * 16 bits x 16 bits = 32 bits, which we can handle.
	 */
	rate->inskip = rate->lcmrate / inrate;
	rate->outskip = rate->lcmrate / outrate; 
	rate->total = rate->intot = rate->outtot = 0;
	rate->lastsamp = 0;
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */

void rate_flow(ibuf, obuf, isamp, osamp)
unsigned char *ibuf, *obuf;
int *isamp, *osamp;
{
	int len, done;
	int skip;
	unsigned char *istart = ibuf;
	long last;
	
	done = 0;

	if (rate->total == 0) {
		/* Emit first sample.  We know the fence posts meet. */
		*obuf = *ibuf++;
		/* Count up until have right input samples */
		rate->lastsamp = audio_u2s(*obuf++);
		done = 1;
		rate->total = 1;
		/* advance to second output */
		rate->outtot += rate->outskip;
		/* advance input range to span next output */
		while ((rate->intot + rate->inskip) <= rate->outtot){
/*			last = audio_u2s(*ibuf++); */
			ibuf++;
			rate->intot += rate->inskip;
		}
	}

	/* number of output samples the input can feed */

	len = (*isamp * rate->inskip) / rate->outskip;
	if (len > *osamp)
		len = *osamp;
	last = rate->lastsamp;
	for(; done < len; done++) {
		int osl;

/*		osl = last;*/
		osl = last + ((long) (audio_u2s(*ibuf) - last) *
			((long) rate->outtot - rate->intot)) / rate->inskip;
		*obuf++ = audio_s2u(osl);
		/* advance to next output */
		rate->outtot += rate->outskip;
		/* advance input range to span next output */
		while ((rate->intot + rate->inskip) <= rate->outtot){
			last = audio_u2s(*ibuf++);
			rate->intot += rate->inskip;
			if (ibuf - istart == *isamp)
				goto out;
		}
                /* long samples with high LCM's overrun counters! */
		if (rate->outtot == rate->intot)
			rate->outtot = rate->intot = 0;
	}
out:
	*isamp = ibuf - istart;
	*osamp = len;
	rate->lastsamp = last;
}
