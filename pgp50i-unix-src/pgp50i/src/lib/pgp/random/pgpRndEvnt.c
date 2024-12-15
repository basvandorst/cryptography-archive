/*
* pgpRndEvnt.c -- Take random events and add it to the random pool.
* This also does entropy estimation.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Colin Plumb
*
* $Id: pgpRndEvnt.c,v 1.5.2.1 1997/06/07 09:51:33 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpRnd.h"
#include "pgpRndPool.h"	/* Our functions are declared here */

/* Number of elements in the given array */
#define elemsof(x) ((unsigned)(sizeof(x)/sizeof(*x)))

/*
* Estimate the amount of entropy in an input value x by seeing how
* different it is from what has come before.
*
* This is based on computing the delta from the previous value, and the
* second-order delta from the previous delta, and so on, for h orders.
* The minimum of any of those deltas is returned as the entropy estimate.
* (Which must, of course, have a logarithm taken to produce a "number
* of bits" output.)
*
* This requires a pointer to a h-word history table of previous deltas.
* A value of h = 3 is generally good, but some things (like keystroke
* timings) feed deltas and not input values into this, so that removes
* the first level.
*/
static word32
randEstimate(word32 x, word32 *history, unsigned h)
	{
			word32 t, min = x;

while (h--) {
			t = history[h];		 	/* Last delta */
			t = (x > t) ? x - t : t - x;	 /* |x - history[h]| */
			history[h] = x;
			x = t;
			if (min > x)
					min = x;
	}
return min;
}

/*
* Gather and estimate entropy from keyboard timings. Double letters
* are allowed, but triples and more are considered suspiscious and
* entropy is not counted. (The actual criterion is that the current
* letter has appeared more than once in the previous four letters,
* which rejects aaaa... and ababa...)
*
* The "letter" can be generalized to mouse-clicks, button-pushes, menu
* selections, or anything else that can be categorized into a finite
* number of events.
*
* Question: is there a way to achieve this effect without remembering
* the recent keystrokes so explicitly? It seems like a possible
* security hole.
*
* We incorporate entropy from the first 3 samples, but don't count them
* since only after that many do we get reliable per-sample entropy estimates.
* (This is time for the two entries in teh hist array to get initialized,
* plus the one level of delta history implicitly included in the
* ranGetEntropy timing. It has to be there unless we want to export
* knowledge about the modulus at which the timer it uses wraps.)
*/
unsigned
pgpRandPoolKeystroke(int event)
	{
			static int pastevent[4];		/* Last 4 events */
			static word32 hist[2];
			static unsigned histcount=elemsof(hist)+1;
				 		 	/* # invalid entries in hist array */
			word32 delta;
			unsigned n = 0;
			int i;

			delta = ranGetEntropy(&pgpRandomPool);
			delta = randEstimate(delta, hist, elemsof(hist));

			pgpRandomAddBytes(&pgpRandomPool, (byte *)&event, sizeof(event));

			/* Check for repetitive keystroke patterns */
			i = elemsof(pastevent) - 1;
			n = (event == pastevent[i]);
			do {
				 n += (event == (pastevent[i] = pastevent[i-1]));
			} while (--i);
			pastevent[0] = event;

			if (histcount > 0) {
				 /* Not yet filled hist array */
				 --histcount;
				 return 0;
			}

			return (n > 1) ? 0 : pgpRandPoolAddEntropy(delta);
	}

/*
* Generate entropy from mouse motion. This simply measures entropy
* in both directions independently. Also includes entropy of timing,
* although that may be pretty low due to OS synchrony.
*
* In essence, we try to predict the mouse position from past positions,
* using three predictors: constant position, constant velocity, and
* constant acceleration. The minimum misprediction distance is used
* to extimate entropy from.
*
* We incorporate entropy from the first 3 samples, but don't count them
* since only after that many do the predictors start working.
* (Note that the timing history array is one shorter, because one extra
* level of delta computation is buried in ranGetEntropy().)
*/
unsigned
pgpRandPoolMouse(word32 x, word32 y)
	{
			static word32 histx[3], histy[3]; /* X and Y position history */
			static word32 hist[2];		 /* Timing history */
			static unsigned histxcount=elemsof(histx);
				 		 	/* # invalid entries in histx array */
			word32 delta;			/* Timing delta */
			word32 deltax;		 	/* X delta */
			word32 deltay;		 	/* Y delta */

			delta = ranGetEntropy(&pgpRandomPool);
			delta = randEstimate(delta, hist, elemsof(hist));
			deltax = randEstimate(x, histx, elemsof(histx));
			deltay = randEstimate(y, histy, elemsof(histy));

			pgpRandomAddBytes(&pgpRandomPool, (byte *)&x, sizeof(x));
			pgpRandomAddBytes(&pgpRandomPool, (byte *)&y, sizeof(y));

			/* Wait until we have filled our arrays to start counting entropy */
			if (histxcount > 0) {
				 --histxcount;
				 return 0;
			}
			return pgpRandPoolAddEntropy(delta)
				+ pgpRandPoolAddEntropy(deltax)
				+ pgpRandPoolAddEntropy(deltay);
	}
