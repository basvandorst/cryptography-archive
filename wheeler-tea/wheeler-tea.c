/*
The following block cipher is in the public domain and is/will be 
published in: B.Preneel, ed., Proceedings of the 1994 K.U. Leuven 
Workshop on Cryptographic Algorithms, Lecture Notes in Computer 
Science, Springer-Verlag, 1995.  This posting has been agreed by 
David Wheeler.

It uses little set up time and does a weak non-linear iteration 
of enough rounds to make it secure.  There are no preset tables 
or long set up times.  It assumes 32 bit words.

Encode Routine 

The routine, written in the C language, is used for encoding with 
a key, k[0] - k[3].  Data is in v[0] and v[1]. 
*/

void code(long* v, long* k)  
{   unsigned long y = v[0],
                  z = v[1], 
                  sum = 0,                           /* set up */
                  delta = 0x9e3779b9, 
                  n = 32;           /* a key schedule constant */

    while(n-- > 0)                        /* basic cycle start */
    {   sum += delta;
        y += (z << 4) + k[0] ^ z + sum ^ (z >> 5) + k[1];
        z += (y << 4) + k[2] ^ y + sum ^ (y >> 5) + k[3];
    }                                             /* end cycle */

    v[0] = y;
    v[1] = z;
}

/*
The number of rounds before a single bit change of the data or 
key has spread very close to 32 is at most six, so that sixteen 
cycles may suffice and we suggest 32. 

The key is set at 128 bits as this is enough to prevent simple 
search techniques being effective. 

Although speed is not a strong objective with 32 cycles (64 
rounds) on one implementation it is three times as fast as a good 
software implementation of DES which has 16 rounds. 

The modes of use of DES are all applicable.  The cycle count can 
readily be varied, or even made part of the key.  It is expected 
that security can be enhanced by increasing the number of 
iterations. 
*/

/* Decode Routine */

void decode( long* v, long* k )  
{   unsigned long n = 32, 
                  sum, 
                  y = v[0], 
                  z=v[1],
                  delta = 0x9e3779b9;

    sum = delta << 5;
                                                /* start cycle */
    while(n-- > 0) 
    {   z -= (y << 4) + k[2] ^ y + sum ^ (y >> 5) + k[3]; 
        y -= (z << 4) + k[0] ^ z + sum ^ (z >> 5) + k[1];
        sum -= delta;
    }
                                                  /* end cycle */
    v[0] = y;
    v[1] = z;
}

/*
A simple improvement is to copy k[0-3] into a,b,c,d before the 
iteration so that the indexing is taken out of the loop.  In one 
implementation it reduced the time by about 1/6th. 

It can be implemented as a couple of macros, which would remove 
the calling overheads. 
*/

