/* program to generate and evaluate candidate mars sboxes
 *
 * Last modified: June-2-1998
 * 
 * to compile:
 *    gcc -Wall -pedantic -O6 -o sbox sbox.c sha.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "sha.h"

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/* Since we define the sha input in words, we have to deal with
 * endianness, which we arbitrarily pick as little endian. 
 * On a big endian machine, we define BSWAP()
 * to do the conversions. BSWAP() is about the best you can do in C.
 * Real implementations will undoubtedly use inline ASM, as most risc
 * machines can do this in one instruction.
 *
 * Make a best guess on platform endianness; This works on linux, AIX,
 * and W95. For other platforms, you may have to manually define SWAP_BYTES
 * on a big endian machine if this guessing doesn't work.
 */    
#ifdef _AIX
#define SWAP_BYTES
#else
#ifdef __linux__
#include <endian.h>
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
#define SWAP_BYTES
#endif
#endif
#endif
#endif
#ifdef SWAP_BYTES
#define BSWAP(x) \
    ( (( x          ) << 24) | \
      (((x)&0xff00  ) << 8 ) | \
      (((x)&0xff0000) >> 8 ) | \
      (( x          ) >> 24)  )
#else
#define BSWAP(x) (x)
#endif                        

typedef unsigned long WORD;

/* this program can run a looonggg time - let SIGHUP display status */
static int dumpme = 0;
void dump(int sig)
{
    dumpme = 1;	
    signal(1,dump);
}

enum SboxTests { Correlation, OneBitBias, TwoBitBias, Parity, Avalanche };
static char *names[] = { "single bit correlation", 
                         "single bit bias", 
                         "consecutive bit bias",
                         "parity bias", "avalanche"};


static int fixSbox(WORD *S);
static int fail(WORD *sbox, float *tests);
static float correlation(WORD *S,float *tests);
static float avalanche(WORD *S, float *tests);
static float oneBitBias(WORD *S, float *tests);
static float twoBitBias(WORD *S, float *tests);
static float parity(WORD *S, float *tests);


void showbest(int j, int f, int bestj, WORD *best, float *besttests)
{
    int i,k;
    printf("After %d, (%d fail) new min j = %d (0x%x)\n",j,f,bestj,bestj);
    avalanche(best,besttests);  /* just for information purposes */
    for (k=0; k<5; k++)
        printf("test %d eval %f (%s)\n",k,besttests[k],names[k]);
    printf("WORD S[] = {\n");
    for (i = 0; i < 512; i+=4) {
        printf("  0x%8.8lx, 0x%8.8lx, 0x%8.8lx, 0x%8.8lx, \n",
                best[i], best[i+1], best[i+2], best[i+3]);
    }
    printf("};\n");
    fflush(stdout);
}

int main()
{
    struct SHAContext s;
    /* sbox is 512 words, but leave room for overlap of last sha tile */
    WORD sbox[515];
    WORD best[512];
    /* buf[0] = sha tiling counter
     * buf[1] = pi
     * buf[2] = e
     * buf[3] = varied parameter for picking best sbox
     */
    WORD buf[4] = { 0, 0xb7e15162, 0x243f6a88, 0 }; 
    float min, new, tests[10], besttests[10]; 
    int i,j,bestj,failures=0;

    /* make sure the endian conversion is right */
    WORD t = 0x12345678;
#ifdef LITTLE_ENDIAN
    	if (*(word8 *)&t != 0x78) {
	        puts("Error: Define BIG_ENDIAN in SHA.H and recompile");
        	exit(-1);
        }
#else
#ifdef BIG_ENDIAN 
    	if (*(word8 *)&t != 0x12) {
	        puts("Error: Define LITTLE_ENDIAN in SHA.H and recompile");
	        exit(-1);
        }
#endif
#endif

    /* install output signal handler */
    signal(1,dump);

    min = 200.0;  /* a very large number */
    bestj = 0;
    memset((char *)best, 0, sizeof(best));
    memset((char *)besttests, 0, sizeof(besttests));

    buf[1] = BSWAP(buf[1]);
    buf[2] = BSWAP(buf[2]);

    /* try all values of minimization word */
    for (j=0; j!=-1; j++) {
        buf[3] = BSWAP(j);
        /* sha generates 5 words */
        for (i = 0; i < 512; i+=5) {
            shaInit(&s);
            buf[0] = BSWAP(i);
            shaUpdate(&s, (unsigned char *)buf, sizeof(buf));
            shaFinal(&s, (unsigned char *)(&sbox[i]));
            sbox[i] = BSWAP(sbox[i]);
            sbox[i+1] = BSWAP(sbox[i+1]);
            sbox[i+2] = BSWAP(sbox[i+2]);
            sbox[i+3] = BSWAP(sbox[i+3]);
            sbox[i+4] = BSWAP(sbox[i+4]);
        }

	/* modify the S-box to eliminate diffs with >= 2 zero bytes */
	fixSbox(sbox); 

        /* the fail() test is lengthy -- do it only for candidates */
        new = correlation(sbox,tests);
        if (new < min) {
            if (!fail(sbox,tests)) { 
                memcpy((char *)best, (char *)sbox, sizeof(best));
                memcpy((char *)besttests, (char *)tests, sizeof(besttests));

		if (min > 100.0) /* the first accepted Sbox */
		    showbest(j+1,failures,j,best,besttests);

                min = new;
                bestj = j;
            }
            else
                failures++;
        }
        if (dumpme) {
            dumpme = 0;
            showbest(j+1,failures,bestj,best,besttests);
        }
    }
    /* not that I think we will ever get here :-) */
    showbest(j,failures,bestj,best,besttests);
    return(0);
}

/* comparison function for qsorting the (512 2) differences
 */
static int mycmp(const void *a, const void *b)
{
    WORD x = *(WORD*)a; 
    WORD y = *(WORD*)b; 
    return (x < y) ? 1: -1;
}

static WORD xorDiffs[256*512];
static WORD subDiffs[512*512];

/* modify the S-box to eliminate diffs with 2 or more zero bytes */
static int fixSbox(WORD *S)
{
    int i, j;
    WORD diff;

    /* first pass - modify the S-box */
    for (i = 0; i < 256; i++)  for(j = i+1; j < 256; j++) {
        diff = S[i] ^ S[j];

	/* if 'diff' has two or more zero bytes, 
	 * multiply S[i] by 3 and continue to the next i
	 */
	if ( ((diff & 0x0000ffff) == 0) || ((diff & 0x00ff00ff) == 0) ||
	     ((diff & 0xff0000ff) == 0) || ((diff & 0x00ffff00) == 0) ||
	     ((diff & 0xff00ff00) == 0) || ((diff & 0xffff0000) == 0) )
	{
	    S[i] *= 3;
	    break;
	}
    }
    for (i = 256; i < 512; i++)  for(j = i+1; j < 512; j++) {
        diff = S[i] ^ S[j];

	/* if 'diff' has two or more zero bytes, 
	 * multiply S[i] by 3 and continue to the next i
	 */
	if ( ((diff & 0x0000ffff) == 0) || ((diff & 0x00ff00ff) == 0) ||
	     ((diff & 0xff0000ff) == 0) || ((diff & 0x00ffff00) == 0) ||
	     ((diff & 0xff00ff00) == 0) || ((diff & 0xffff0000) == 0) )
	{
	    S[i] *= 3;
	    break;
	}
    }

    return 0;
}


/* check a candidate min sbox for rejection criteria */
static int fail(WORD *S, float *tests)
{
    int i,j;
    WORD diff;
    WORD *xor_ptr, *sub_ptr;

    /* requirement 0: no diffs in S0, S1 with >= 2 zero bytes */
    for (i = 0; i < 256; i++)  for(j = i+1; j < 256; j++) {
        diff = S[i] ^ S[j];
	if ( ((diff & 0x0000ffff) == 0) || ((diff & 0x00ff00ff) == 0) ||
	     ((diff & 0xff0000ff) == 0) || ((diff & 0x00ffff00) == 0) ||
	     ((diff & 0xff00ff00) == 0) || ((diff & 0xffff0000) == 0)  )
	    return 1;
    }
    for (i = 256; i < 512; i++)  for(j = i+1; j < 512; j++) {
        diff = S[i] ^ S[j];
	if ( ((diff & 0x0000ffff) == 0) || ((diff & 0x00ff00ff) == 0) ||
	     ((diff & 0xff0000ff) == 0) || ((diff & 0x00ffff00) == 0) ||
	     ((diff & 0xff00ff00) == 0) || ((diff & 0xffff0000) == 0)  )
	    return 1;
    }

    /* requirement 1: S does not contain all 0 or 1 words */
    for (i=0; i<512; i++) {
        if ((S[i] == 0)||(S[i] == 0xffffffff))
	    return(1);
    }

    /* requirement 2: S does not contain equal, complement, negative pairs.
     * While we're here, we also initialize the xorDiff, subDiffs arrays. 
     */
    xor_ptr = xorDiffs;
    sub_ptr = subDiffs;
    for (i=0;i<512;i++)   for(j=i+1;j<512;j++) {
        if ((S[j] == S[i]) || (S[j] == -S[i]) || (S[j] == ~S[i]))
	    return 1;

	*(xor_ptr++) = S[i]^S[j];
	sub_ptr[0] = S[j]-S[i];
	sub_ptr[1] = S[i]-S[j];
	sub_ptr += 2;
    }

    /* requirement 3: all xor and sub differences are distinct */
    qsort(xorDiffs, 4, 511*256, mycmp);
    for(i=0; i<(511*256)-1; i++) {
        if (xorDiffs[i] == xorDiffs[i+1])
            return(1);
    } 
    qsort(subDiffs, 4, 511*512, mycmp);
    for(i=0; i<(511*512)-1; i++) {
        if (subDiffs[i] == subDiffs[i+1])
            return(1);
    }
 
    /* requirement 4: all xor differences have at least 4 1's */
    for (i=0; i<(511*256); i++){
        diff = xorDiffs[i];

	/* each iteration reduces by one the number of 1's in diff */
	diff &= (diff -1);
	diff &= (diff -1);
	diff &= (diff -1);
	if (diff == 0)
	    return(1);
    }

    /* requierment 5-7:
     * parity-bias <= 1/32, 1-bit-bias <= 1/30 and 2-bit-bias <= 1/30
     */
    if ( (parity(S,tests) > (1.0/32.0)) ||
	 (oneBitBias(S,tests) > (1.0/30.0)) ||
	 (twoBitBias(S,tests) > (1.0/30.0)) )
        return 1;

    return(0);
}

/* compute | Pr(x_i = S[x]_j) - 1/2 | (single bit correlation) */
static float correlation(WORD *S,float *tests)
{
    int i, j, k;
    int results[32];
    int deviation=0;
    WORD mask, res;

    for (i = 0; i < 9; i++) {
        for (j = 0; j < 32; j++)
            results[j] = 0;
        for (k = 0; k < 512; k++) {
            mask = -((k>>i)&1);  /* all 0 if k_i=0, or all 1 if k_i=1 */
            res = S[k] ^ mask;
            for (j = 0; j < 32; j++) {
	        results[j] += res & 1;
	        res >>= 1;
            }
        }
        for (j = 0; j < 32; j++) {
	    int curDev = abs(results[j] - 256);
            deviation = MAX(deviation, curDev);
        }
    }
    tests[Correlation] = (float)deviation/(512.0);
    return(tests[Correlation]);
}

/* compute | Pr(S[x+2^i]_j = S[x]_j) - 1/2 | (avalanche) */
/* this is the only test done separately for each sbox */
static float avalanche(WORD *S, float *tests)
{
    int i, j, k;
    int curDev, results0[32], results1[32], deviation0=0, deviation1=0;
    WORD res;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 32; j++){
            results0[j] = 0;
            results1[j] = 0;
        }
        /* first for sbox0 */
        for (k = 0; k < 256; k++) {
            res = S[k] ^ S[k^(1<<i)];
            for (j = 0; j < 32; j++) {
	        results0[j] += res & 1;
	        res >>= 1;
            }
        }
        for (j = 0; j < 32; j++) {
            curDev = abs(results0[j] - 128);
	    deviation0 = MAX(deviation0, curDev);
        }
        /* second, for sbox1 */
        for (k = 0; k < 256; k++) {
            res = S[k+256] ^ S[(k^(1<<i))+256];
            for (j = 0; j < 32; j++) {
	        results1[j] += res & 1;
	        res >>= 1;
            }
        }
        for (j = 0; j < 32; j++) {
            curDev = abs(results1[j] - 128);
	    deviation1 = MAX(deviation1, curDev);
        }
    }
    tests[Avalanche] = ((float)MAX(deviation0,deviation1))/(256.0);
    return(tests[Avalanche]);
}


/* compute | Pr(S[x]_j = 0) - 1/2 | (single bit bias) */
static float oneBitBias(WORD *S, float *tests)
{
    int j, k, results[32], deviation=0;
    WORD res;

    for (j = 0; j < 32; j++)
        results[j] = 0;
    for (k = 0; k < 512; k++) {
        res = S[k];
        for (j = 0; j < 32; j++) {
            results[j] += res & 1;
            res >>= 1;
        }
    }
    for (j = 0; j < 32; j++) {
        int curDev = abs(results[j] - 256);
        deviation = MAX(deviation, curDev);
    }

    tests[OneBitBias] = (float)deviation/(512.0);
    return(tests[OneBitBias]);
}

/* compute | Pr(Parity(S[x]) = 0) - 1/2 | (parity bias) */
static float parity(WORD *S, float *tests)
{
    int j, k, parity, deviation=0;
    WORD res;

    for (k = 0; k < 512; k++) {
        res = S[k];
        parity = 0;
        for (j = 0; j < 32; j++) {
            parity += res & 1;
            res >>= 1;
        }
        parity &= 1;
        deviation += parity;
    }
    deviation = abs(deviation - 256);
    tests[Parity] = (float)deviation/(512.0);
    return(tests[Parity]);
}

/* compute | Pr(S[x]_j ^ S[x]_j+1 = 0) - 1/2 | (consecutive bit bias) */
static float twoBitBias(WORD *S, float *tests)
{
    int j, k, results[32], deviation=0;
    WORD res;

    for (j = 0; j < 31; j++)
        results[j] = 0;
    for (k = 0; k < 512; k++) {
        res = S[k];
        for (j = 0; j < 31; j++) {
            if (((res & 3) == 0)||((res&3) == 3))
                results[j]++;
            res >>= 1;
        }
    }
    for (j = 0; j < 31; j++) {
        int curDev = abs(results[j] - 256);
        deviation = MAX(deviation, curDev);
    }
    tests[TwoBitBias] = (float)deviation/(512.0);
    return(tests[TwoBitBias]);
}
