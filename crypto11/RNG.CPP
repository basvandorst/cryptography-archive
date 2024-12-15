#include <memory.h>
#include "misc.h"
#include "rng.h"
#include <time.h>

// linear congruential generator
// originally by William S. England

// do not use for cryptographic purposes

/*
** Original_numbers are the original published m and q in the
** ACM article above.  John Burton has furnished numbers for
** a reportedly better generator.  The new numbers are now
** used in this program by default.
*/

#ifndef LCRNG_ORIGINAL_NUMBERS
const word32 LC_RNG::m=2147483647L;
const word32 LC_RNG::q=44488L;

const word16 LC_RNG::a=(unsigned int)48271L;
const word16 LC_RNG::r=3399;
#else
const word32 LC_RNG::m=2147483647L;
const word32 LC_RNG::q=127773L;

const word16 LC_RNG::a=16807;
const word16 LC_RNG::r=2836;
#endif

byte LC_RNG::GetByte()
{
    word32 hi    = seed/q;
    word32 lo    = seed%q;

    long test = a*lo - r*hi;

    if (test > 0)
	    seed = test;
    else
	    seed = test+ m;

    return (seedptr[0] ^ seedptr[1] ^ seedptr[2] ^ seedptr[3]);
}

X917RNG::X917RNG(BlockTransformation *c, const byte *seed)
    : cipher(c),
      S(cipher->BlockSize()),
      dtbuf(SecAlloc(byte, 3*S)),
      randseed(dtbuf+S),
      randbuf(dtbuf+2*S),
      randbuf_counter(0)
{
    memcpy(randseed, seed, S);
    time_t tstamp1 = time(0);
    xorbuf(dtbuf, (byte *)&tstamp1, min((int)sizeof(tstamp1), S));
    cipher->ProcessBlock(dtbuf);
    clock_t tstamp2 = clock();
    xorbuf(dtbuf, (byte *)&tstamp2, min((int)sizeof(tstamp2), S));
    cipher->ProcessBlock(dtbuf);
}

X917RNG::~X917RNG()
{
    delete cipher;
    SecFree(dtbuf, 3*S);
}

byte X917RNG::GetByte()
{
    if (randbuf_counter==0)
    {
        // calculate new enciphered timestamp
        clock_t tstamp = clock();
        xorbuf(dtbuf, (byte *)&tstamp, min((int)sizeof(tstamp), S));
        cipher->ProcessBlock(dtbuf);

    	// combine enciphered timestamp with seed
    	xorbuf(randseed, dtbuf, S);

    	// generate a new block of random bytes
    	cipher->ProcessBlock(randseed, randbuf);

    	// compute new seed vector
    	for (int i=0; i<S; i++)
    	    randseed[i] = randbuf[i] ^ dtbuf[i];
    	cipher->ProcessBlock(randseed);

    	randbuf_counter=S;
    }
    return(randbuf[--randbuf_counter]);
}

