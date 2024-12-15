/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates. 
	All rights reserved.
	
	

	$Id: pgpDiskWiper.c,v 1.4.8.1 1998/11/12 03:11:07 heller Exp $
____________________________________________________________________________*/

/* System Headers */
#if PGP_DEBUG
#include <stdio.h>
#endif
#include <string.h> /* For memcpy(), etc. */


/* PGPsdk Headers */
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpUtilities.h" 
#include "pgpPubTypes.h"
#include "pgpUtilities.h"
#include "pgpMemoryMgr.h"

/* Project Headers */
#include "pgpDiskWiper.h"


/*
 * These are the disk-wiping patterns.  They are repeating patterns of
 * 1, 2, 3 and 4 bits, along with some random passes.  See Peter Gutmann's
 * paper, "Secure Deletion of Data from Magnetic and Solid-State Memory",
 * first published in the Sixth USENIX Security Symposium Proceedings,
 * San Jose, California, July 22-25, 1996 for the gory details why.
 * (http://www.cs.auckland.ac.nz/~pgut001/pubs/secure_del.html)
 */
PGPInt32 patterns[] = {
	-1, /* Random */
	-1, /* Random */
	0x555, 0xAAA,
	0x249, 0x492, 0x924,
	0x000, 0xFFF,
	0x111, 0x222, 0x333, 0x444, 0x666, 0x777, 
	0x888, 0x999, 0xBBB, 0xCCC, 0xDDD, 0xEEE,
	0xDB6, 0xB6D, 0x6DB,
	-1, /* Random */
	-1 /* Random */
};

#define NUMPATTERNS (sizeof(patterns)/sizeof(patterns[0]))

/* Random number code from Colin */

/*
 * This is Bob Jenkins' ISAAC generator
 * (http://ourworld.compuserve.com/homepages/bob_jenkins/isaacafa.htm)
 * which is designed to be cryptographically strong and very fast.
 * I'm not sure if I'd trust it for key material, but it should be
 * strong enough for this application, and the speed is nice when
 * you need megabytes of output.
 */

#define ISAAC_LOG 8
#define ISAAC_SIZE 256
#define ISAAC_BYTES (ISAAC_SIZE * sizeof(PGPInt32))

typedef struct isaac_state isaac_state;

struct isaac_state
{
	PGPInt32 mm[ISAAC_SIZE]; /* State */
	PGPInt32 aa, bb, cc;
};

typedef struct PGPDiskWipe	 PGPDiskWipe;

struct PGPDiskWipe
{
	PGPContextRef		context;
	PGPMemoryMgrRef		manager;
	PGPInt32			index;
	PGPInt32			passes;
	isaac_state 		state;
	PGPInt32 			patterns[NUMPATTERNS];
};


/* This index operation is more efficient on many processors */
#define ind(mm,x) *(unsigned *)\
((char *)(mm) + ( (x) & (ISAAC_SIZE-1)<<2 ))

/* The central step. This uses two temporaries, x and y. */
#define isaac_step(mix,a,b,mm,m,off,r) \
( \
a = (a^(mix)) + m[off], \
x = *m, \
*(m++) = y = ind(mm,x) + a + b, \
*(r++) = b = ind(mm,y>>ISAAC_LOG) + x \
)

/*
* Refill the entire r array with one call.
*/
static void
isaac_refill(struct isaac_state *s, 
			 PGPInt32 r[ISAAC_SIZE])
{
	register PGPInt32 a, b; /* Caches of a and b */
	register PGPInt32 x, y; /* Temps needed by isaac_step() macro */
	register PGPInt32 *m = s->mm; /* Pointer into state array */

	a = s->aa;
	b = s->bb + (++s->cc);

	do 
	{
		isaac_step(a << 13, a, b, s->mm, m, ISAAC_SIZE/2, r);
		isaac_step(a >> 6, a, b, s->mm, m, ISAAC_SIZE/2, r);
		isaac_step(a << 2, a, b, s->mm, m, ISAAC_SIZE/2, r);
		isaac_step(a >> 16, a, b, s->mm, m, ISAAC_SIZE/2, r);
	} while (m < s->mm+ISAAC_SIZE/2);
	
	do 
	{
		isaac_step(a << 13, a, b, s->mm, m, -ISAAC_SIZE/2, r);
		isaac_step(a >> 6, a, b, s->mm, m, -ISAAC_SIZE/2, r);
		isaac_step(a << 2, a, b, s->mm, m, -ISAAC_SIZE/2, r);
		isaac_step(a >> 16, a, b, s->mm, m, -ISAAC_SIZE/2, r);
	} while (m < s->mm+ISAAC_SIZE);
	
	s->bb = b;
	s->aa = a;
}

/* The basic seed-scrambling step */
#define mix(a,b,c,d,e,f,g,h) \
(a ^= b << 11, d += a, \
b += c, b ^= c >> 2, e += b, \
c += d, c ^= d << 8, f += c, \
d += e, d ^= e >> 16, g += d, \
e += f, e ^= f << 10, h += e, \
f += g, f ^= g >> 4, a += f, \
g += h, g ^= h << 8, b += g, \
h += a, h ^= a >> 9, c += h, \
a += b)


/*
* Initialize the ISAAC RNG with the given 256-entry seed array.
* That array may be the mm[] array.
*/
static void
isaac_init(struct isaac_state *s, PGPInt32 const *seed)
{
	PGPInt32 i;
	PGPInt32 passes = (seed != 0);
	PGPInt32 a, b, c, d, e, f, g, h;
	
#if 0
	/* The original presentation */
	a = b = c = d = e = f = g = h = 0x9e3779b9; /* the golden ratio */
	
	for (i = 0; i < 4; ++i) /* scramble it */
	{
		mix(a, b, c, d, e, f, g, h);
	}
	
#else
	/* A smaller and faster, although more opaque version. */
	a = 0x1367df5a;
	b = 0x95d90059;
	c = 0xc3163e4b;
	d = 0x0f421ad8;
	e = 0xd92a4a78;
	f = 0xa51a3c49;
	g = 0xc4efea1b;
	h = 0x30609119;
#endif
	
	do 
	{
		/* Fill up mm[] with messy stuff */
		for (i = 0; i < ISAAC_SIZE; i += 8) 
		{
			if (seed) 
			{
				/* use all the information in the seed */
				a += seed[i];
				b += seed[i+1];
				c += seed[i+2];
				d += seed[i+3];
				e += seed[i+4];
				f += seed[i+5];
				g += seed[i+6];
				h += seed[i+7];
			}

			mix(a, b, c, d, e, f, g, h);

			s->mm[i] = a;
			s->mm[i+1] = b;
			s->mm[i+2] = c;
			s->mm[i+3] = d;
			s->mm[i+4] = e;
			s->mm[i+5] = f;
			s->mm[i+6] = g;
			s->mm[i+7] = h;
		}
		
		seed = s->mm;
		
		/*
		 * If we were given a seed, do a second pass to make
		 * all of the seed affect all of mm
		 */
	} while (passes--);

	s->aa = s->bb = s->cc = 0;
}

/*
* Seed the RNG, using 160 bits of seed material, which should be *plenty*.
* This cheats and uses the s->mm[] array to hold the seed material,
* which is allowed by isac_init().
*/
static PGPError
isaac_seed(PGPDiskWipeRef ref)
{
	PGPError		error	= kPGPError_NoErr;
	struct isaac_state* s	= &(ref->state);
	
	/* Get some good seed material. */
	error = PGPContextGetRandomBytes(ref->context, s->mm, 20);
	isaac_init(s, s->mm);
	
	return error;
}

/* End RNG code */



/*
* Shuffle an array of ints into random order.
* This is done by swapping the last element in the array
* with a random element of the array and then repeating
* for the array without the last element.
*/
static void
shuffle(PGPInt32 *array, 
		PGPUInt32 num, 
		struct isaac_state *s)
{
	PGPInt32 i, t;
	PGPInt32 r[ISAAC_SIZE];
	
	isaac_refill(s, r);
	
	while (--num) 
	{
		i = (int)(r[num] % (num+1));
		t = array[i];
		array[i] = array[num];
		array[num] = t;
	}
}

/* Move the random pass into the middle of the shuffle */
static void
tweakshuffle(PGPInt32 *array, 
			 PGPUInt32 num)
{
	PGPUInt32 i;

	for (i = 0; i < num; i++) 
	{
		if (array[i] < 0) 
		{
			num /= 2;
			array[i] = array[num];
			array[num] = -1;
			break;
		}
	}
}

/* Pass types:
* 0xXXX - Repeat XXX to fill file
* -1 - Random
*/
static void
filltable(struct isaac_state *s, 
		  PGPInt32 buf[ISAAC_SIZE], 
		  PGPInt32 type)
{
	PGPInt32 i;

	if (type < 0) 
	{
		isaac_refill(s, (PGPInt32 *)buf);
	} 
	else
	{
		((unsigned char *)buf)[0] = 255 & (type >> 4);
		((unsigned char *)buf)[1] = 255 & (type << 4 | type >> 8);
		((unsigned char *)buf)[2] = 255 & type;
		
		for (i = 3; i < ISAAC_BYTES/2; i *= 2)
		{
			memcpy((char *)buf+i, (char *)buf, i);
		}
		
		memcpy((char *)buf+i, (char *)buf, ISAAC_BYTES-i);
	}
}


PGPError 
PGPCreateDiskWiper( PGPContextRef context, 
					PGPDiskWipeRef *wipeRef, 
					PGPInt32 numPasses)
{
	PGPError error = kPGPError_NoErr;
	PGPMemoryMgrRef manager = PGPGetContextMemoryMgr (context);

	if (IsntNull(wipeRef))
	{
		*wipeRef = NULL;
	}
	
	
	*wipeRef = (PGPDiskWipeRef) PGPNewData(
								manager,
								sizeof(PGPDiskWipe), 
								kPGPMemoryMgrFlags_Clear);
	
	
	if(IsntNull(*wipeRef))
	{
		PGPDiskWipeRef ref = *wipeRef;
		
		/* we need this guy for random data */
		ref->context = context;

		/* we need this guy for deleting ourselves */
		ref->manager = manager;

		/* keep track of which pass we are on */
		ref->index = 0; 
		
		/* remember total # of passes */
		ref->passes = numPasses; 
		
		/* initialize random # generator */
		error = isaac_seed( ref ); 

		/* did it initialize ok? */
		if( IsPGPError(error) )
		{
			/* if not clean up */
			PGPFreeData(ref);
			
			return error;
		}
		
		/* set up the pattern array */
		if(numPasses < NUMPATTERNS)
		{
			PGPInt32 i;
			
			for(i = 0; i < numPasses - 1; i++)
			{
				ref->patterns[i] =  patterns[i];
			}
			
			ref->patterns[i] = -1;
		}
		else 
		{
			memcpy(ref->patterns, patterns, sizeof(patterns));				
		}
		
		if(numPasses > 3)
		{
			if(numPasses > NUMPATTERNS)
			{
				/* only wanna shake up the array*/
				numPasses = NUMPATTERNS;
			}

			/* Shuffle, leaving first & last pass alone */
			shuffle(ref->patterns + 1, 
				(unsigned)numPasses - 2, 
				&(ref->state));
	
			/* Force a random pass to the middle */
			tweakshuffle(ref->patterns + 1, 
					(unsigned)numPasses - 2);
		}
						
	}
	else
	{
		error = kPGPError_OutOfMemory;
		*wipeRef = NULL;
	}
	
	return error;
}
						
PGPError 
PGPGetDiskWipeBuffer(PGPDiskWipeRef wipeRef, 
					 PGPInt32 buffer[256])
{
	PGPError error = kPGPError_NoErr;
	
	if(	IsntNull(wipeRef) && IsntNull(buffer))
	{
		if(wipeRef->index < wipeRef->passes)
		{
			if(wipeRef->index < NUMPATTERNS - 1)
			{
#if PGP_DEBUG
	#if ! PGP_MACINTOSH
				printf("pattern is: 0x%x\r\n",
					wipeRef->patterns[wipeRef->index]);
	#endif
#endif				
				filltable(	&(wipeRef->state),
		 					buffer,
		 					wipeRef->patterns[wipeRef->index++]);
			}
			else
			{
#if PGP_DEBUG
	#if ! PGP_MACINTOSH
				printf("pattern is: 0xffffffff\r\n");
	#endif
#endif				
				filltable(	&(wipeRef->state),
		 					buffer,
		 					-1);
		 				
		 		wipeRef->index++;
			}
		
		}
		else
		{
			error = kPGPError_NoMorePatterns;
		}
	}
	else
	{
		error = kPGPError_BadParams;
	}

	return error;
}
											
PGPError 
PGPDestroyDiskWiper( PGPDiskWipeRef wipeRef )
{
	PGPError error = kPGPError_NoErr;
	
	if( IsntNull(wipeRef) )
	{		
		PGPFreeData( wipeRef );
	}
	else
	{
		error = kPGPError_BadParams;
	}
	
	return error;
}
