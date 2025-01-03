/* idea.c - C source code for IDEA block cipher. IDEA (International Data 
 * Encryption Algorithm), formerly known as IPES (Improved Proposed Encryption
 * Standard). Algorithm developed by Xuejia Lai and James L. Massey, of ETH 
 * Zurich. This implementation modified and derived from original C code 
 * developed by Xuejia Lai. Zero-based indexing added, names changed from IPES
 * to IDEA. CFB functions added. Random number routines added. Optimized for 
 * speed 21 Oct 92 by Colin Plumb <colin@nsq.gts.org>. This code assumes that 
 * each pair of 8-bit bytes comprising a 16-bit word in the key and in the 
 * cipher block are externally represented with the Most Significant Byte 
 * (MSB) first, regardless of internal native byte order of the target CPU.  */

#include "idea.h"

#define min(x, y) (((x) < (y)) ? (x) : (y))

#ifdef TEST
#include <stdio.h>
#include <time.h>
#endif

#ifdef sgi
#define HIGHFIRST
#endif

#ifdef sun
#define HIGHFIRST
#define const
#endif

#define TRUE	1
#define FALSE	0

#define IDEABLOCKSIZE 8
#define ROUNDS      8               /* Don't change this value, should be 8 */
#define KEYLEN	    (6*ROUNDS+4)    /* length of key schedule */

#define byte	unsigned char
#define word16	unsigned short
#define boolean int
#define word32	unsigned long
#define byteptr unsigned char *

typedef word16 IDEAkey[KEYLEN];

#ifdef IDEA32 /* Use >16-bit temporaries */
#define low16(x) ((x) & 0xFFFF)
typedef unsigned int uint16;	    /* at LEAST 16 bits, maybe more */
#else
#define low16(x) (x)                /* this is only ever applied to uint16's */
typedef word16 uint16;
#endif

#ifdef _GNUC_
/* __const__ simply means there are no side effects for this function,
 * which is useful info for the gcc optimizer */
#define CONST __const__
#else
#define CONST
#endif

static void en_key_idea();
static void de_key_idea();
static void cipher_idea();

/* Multiplication, modulo (2**16)+1. Note that this code is structured like 
 * this on the assumption that untaken branches are cheaper than taken 
 * branches, and the compiler doesn't schedule branches. */

#ifdef SMALL_CACHE
CONST static uint16 mul(register uint16 a, register uint16 b)
{
       register word32 p;
       if (a)
       {     if (b)
	     {	    p = (word32)a * b;
		    b = low16(p);
		    a = p>>16;
		    return b - a + (b < a);
	     }
	     else
	     {	    return 1-a;
	     }
       }
       else
       {     return 1-b;
       }
}
#endif /* SMALL_CACHE */

/* Compute multiplicative inverse of x, modulo (2**16)+1, using Euclid's GCD 
 * algorithm. It is unrolled twice to avoid swapping the meaning of the 
 * registers each iteration; some subtracts of t have been changed to adds.  */

CONST static uint16 inv(x)     
    uint16 x;
{
       uint16 t0, t1;
       uint16 q, y;
       if (x <= 1)
	     return x;	  /* 0 and 1 are self-inverse */
       t1 = 0x10001 / x;  /* Since x >= 2, this fits into 16 bits */
       y = 0x10001 % x;
       if (y == 1)
	     return low16(1-t1);
       t0 = 1;
       do
       {     q = x / y;
	     x = x % y;
	     t0 += q * t1;
	     if (x == 1)
		    return t0;
	     q = y / x;
	     y = y % x;
	     t1 += q * t0;
       } while (y != 1);
       return low16(1-t1);
}

/*     Compute IDEA encryption subkeys Z */

static void en_key_idea(userkey, Z)
    word16 *userkey; word16 *Z;
{
       int i,j;
       /* shifts */
       for (j=0; j<8; j++)
	     Z[j] = *userkey++;
       for (i=0; j<KEYLEN; j++)
       {     i++;
	     Z[i+7] = Z[i & 7] << 9 | Z[i+1 & 7] >> 7;
	     Z += i & 8;
	     i &= 7;
       }
}

/*     Compute IDEA decryption subkeys DK from encryption subkeys Z */
/* Note: these buffers *may* overlap! */

static void de_key_idea(Z, DK)
    IDEAkey Z; IDEAkey DK;
{
       int j;
       uint16 t1, t2, t3;
       IDEAkey T;
       word16 *p = T + KEYLEN;
       t1 = inv(*Z++);
       t2 = -*Z++;
       t3 = -*Z++;
       *--p = inv(*Z++);
       *--p = t3;
       *--p = t2;
       *--p = t1;
       for (j = 1; j < ROUNDS; j++)
       {
	     t1 = *Z++;
	     *--p = *Z++;
	     *--p = t1;
	     t1 = inv(*Z++);
	     t2 = -*Z++;
	     t3 = -*Z++;
	     *--p = inv(*Z++);
	     *--p = t2;
	     *--p = t3;
	     *--p = t1;
       }
       t1 = *Z++;
       *--p = *Z++;
       *--p = t1;
       t1 = inv(*Z++);
       t2 = -*Z++;
       t3 = -*Z++;
       *--p = inv(*Z++);
       *--p = t3;
       *--p = t2;
       *--p = t1;
/* Copy and destroy temp copy */
       for (j = 0, p = T; j < KEYLEN; j++)
       {
	     *DK++ = *p;
	     *p++ = 0;
       }
}

/* MUL(x,y) computes x = x*y, modulo 0x10001. Requires two temps, t16 and t32.
 * x must me a side-effect-free lvalue. y may be anything, but unlike x, must 
 * be strictly 16 bits even if low16() is #defined. All of these are 
 * equivalent; see which is faster on your machine.  */

#ifdef SMALL_CACHE
#define MUL(x,y) (x = mul(low16(x),y))
#else
#ifdef AVOID_JUMPS
#define MUL(x,y) (x = low16(x-1), t16 = low16((y)-1), \
             t32 = (word32)x*t16+x+t16+1, x = low16(t32), \
	     t16 = t32>>16, x = x-t16+(x<t16) )
#else
#define MUL(x,y) ((t16 = (y)) ? (x=low16(x)) ? \
        t32 = (word32)x*t16, x = low16(t32), t16 = t32>>16, \
        x = x-t16+(x<t16) : \
	(x = 1-t16) : (x = 1-x))
#endif
#endif

/* IDEA encryption/decryption algorithm . In/out can be the same buffer */ 

static void cipher_idea(in, out, Z)
    word16 in[4]; word16 out[4]; register CONST IDEAkey Z;
{
       register uint16 x1, x2, x3, x4, t1, t2;
       register uint16 t16;
       register word32 t32;
       int r = ROUNDS;
       x1 = *in++;  x2 = *in++;
       x3 = *in++;  x4 = *in;
       do
       {
	     MUL(x1,*Z++);
	     x2 += *Z++;
	     x3 += *Z++;
	     MUL(x4, *Z++);
	     t2 = x1^x3;
	     MUL(t2, *Z++);
	     t1 = t2 + (x2^x4);
	     MUL(t1, *Z++);
	     t2 = t1+t2;
	     x1 ^= t1;
	     x4 ^= t2; 
	     t2 ^= x2;
	     x2 = x3^t1;
	     x3 = t2;
       } while (--r);
       MUL(x1, *Z++);
       *out++ = x1;
       *out++ = x3 + *Z++;
       *out++ = x2 + *Z++;
       MUL(x4, *Z);
       *out = x4;
}

#ifdef TEST

/* Number of Kbytes of test data to encrypt. Defaults to 1 MByte. */

#ifndef KBYTES
#define KBYTES 1024
#endif

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif

int main()
{      /* Test driver for IDEA cipher */ 
       int i, j, k; 
       IDEAkey Z, DK;
       word16 XX[4], TT[4], YY[4];     
       word16 userkey[8];
       clock_t start, end;
       long l;

       /* Make a sample user key for testing... */

       for(i=0; i<8; i++)
	     userkey[i] = i+1;

       /* Compute encryption subkeys from user key... */

       en_key_idea(userkey,Z);
       printf("\nEncryption key subblocks: ");
       for(j=0; j<ROUNDS+1; j++)
       {
             printf("\nround %d:   ", j+1);
	     if (j==ROUNDS)
		    for(i=0; i<4; i++)
                          printf(" %6u", Z[j*6+i]);
	     else
		    for(i=0; i<6; i++)
                          printf(" %6u", Z[j*6+i]);
       }

       /* Compute decryption subkeys from encryption subkeys... */

       de_key_idea(Z,DK);
       printf("\nDecryption key subblocks: ");
       for(j=0; j<ROUNDS+1; j++)
       {
             printf("\nround %d:   ", j+1);
	     if (j==ROUNDS)
		    for(i=0; i<4; i++)
                          printf(" %6u", DK[j*6+i]);
	     else
		    for(i=0; i<6; i++)
                          printf(" %6u", DK[j*6+i]);
       }

       /* Make a sample plaintext pattern for testing... */

       for (k=0; k<4; k++)
	     XX[k] = k;
       printf("\n Encrypting %d KBytes (%ld blocks)...", KBYTES, KBYTES*64l);
       fflush(stdout);
       start = clock();
       cipher_idea(XX,YY,Z);	     /* encrypt plaintext XX, making YY */ 
       for (l = 1; l < 64*KBYTES; l++)
	     cipher_idea(YY,YY,Z);   /* repeated encryption */
       cipher_idea(YY,TT,DK);	     /* decrypt ciphertext YY, making TT */ 
       for (l = 1; l < 64*KBYTES; l++)
	     cipher_idea(TT,TT,DK);  /* repeated decryption */
       end = clock() - start;
       l = end * 1000. / CLOCKS_PER_SEC + 1;
       i = l/1000;
       j = l%1000;
       l = KBYTES * 1024. * CLOCKS_PER_SEC / end;

       printf("%d.%03d seconds = %ld bytes per second\n", i, j, l);
       printf("\nX %6u   %6u  %6u  %6u \n",    
	 XX[0], XX[1],	XX[2], XX[3]);
       printf("Y %6u   %6u  %6u  %6u \n",
	 YY[0], YY[1],	YY[2], YY[3]);
       printf("T %6u   %6u  %6u  %6u \n",
	 TT[0], TT[1],	TT[2], TT[3]);
       /* Now decrypted TT should be same as original XX */
       for (k=0; k<4; k++)
	     if (TT[k] != XX[k])
	     {
                    printf("\n\07Error!  Noninvertable encryption.\n");
		    exit(-1);	   /* error exit */ 
	     }
       printf("\nNormal exit.\n");
       return 0;
}
#endif /* TEST */

/* xorbuf - change buffer via xor with random mask block. Used for Cipher 
 * Feedback (CFB) or Cipher Block Chaining (CBC) modes of encryption. Can be
 *  applied for any block encryption algorithm, with any block size, such as 
 * the DES or the IDEA cipher.	*/

static void xorbuf(buf, mask, count)
    register byteptr buf; register byteptr mask; register int count;
/*     count must be > 0 */
{
       if (count) 
	     do
		    *buf++ ^= *mask++;
	     while (--count);
}      /* xorbuf */

/* cfbshift - shift bytes into IV for CFB input. Used only for Cipher Feedback
 * (CFB) mode of encryption. Can be applied for any block encryption algorithm
 * with any block size, such as the DES or the IDEA cipher.  */

static void cfbshift(iv, buf,  count, blocksize)
    register byteptr iv; register byteptr buf;
    register int count; int blocksize;
/* iv is initialization vector. buf is buffer pointer. count is number of bytes
 * to shift in...must be > 0. blocksize is 8 bytes for DES or IDEA ciphers. */
{
       int retained;
       if (count)
       {
	     retained = blocksize-count;   /* number bytes in iv to retain */
	 /* left-shift retained bytes of IV over by count bytes to make room */
	     while (retained--)
	     {
		    *iv = *(iv+count);
		    iv++;
	     }
	     /* now copy count bytes from buf to shifted tail of IV */
	     do     *iv++ = *buf++;
	     while (--count);
       }
}      /* cfbshift */

/* Key schedules for IDEA encryption and decryption */

static IDEAkey Z, DK;
static word16 *iv_idea;        /* pointer to IV for CFB or CBC */
static boolean cfb_dc_idea;    /* TRUE iff CFB decrypting */

/* initkey_idea initializes IDEA for ECB mode operations */

void initkey_idea(key, decryp)
    byte key[16]; boolean decryp;
{
       word16 userkey[8]; /* IDEA key is 16 bytes long */
       int i;
       /* Assume each pair of bytes comprising a word is ordered MSB-first. */
       for (i=0; i<8; i++)
       {
	     userkey[i] = (key[0]<<8) + key[1];
	     key++; key++;
       }
       en_key_idea(userkey,Z);
       if (decryp)
       {
	     de_key_idea(Z,Z);	 /* compute inverse key schedule DK */
       }
       for (i=0; i<8; i++)/* Erase dangerous traces */
	     userkey[i] = 0;
} /* initkey_idea */

/* Run a 64-bit block thru IDEA in ECB (Electronic Code Book) mode, using the
 * currently selected key schedule. */

void idea_ecb(inbuf, outbuf)
    word16 *inbuf; word16 *outbuf;
{
       /* Assume each pair of bytes comprising a word is ordered MSB-first. */
#ifndef HIGHFIRST   /* If this is a least-significant-byte-first CPU */
       word16 x;
       /* Invert the byte order for each 16-bit word for internal use. */
       x = inbuf[0]; outbuf[0] = x >> 8 | x << 8;
       x = inbuf[1]; outbuf[1] = x >> 8 | x << 8;
       x = inbuf[2]; outbuf[2] = x >> 8 | x << 8;
       x = inbuf[3]; outbuf[3] = x >> 8 | x << 8;
       cipher_idea(outbuf, outbuf, Z);
       x = outbuf[0]; outbuf[0] = x >> 8 | x << 8;
       x = outbuf[1]; outbuf[1] = x >> 8 | x << 8;
       x = outbuf[2]; outbuf[2] = x >> 8 | x << 8;
       x = outbuf[3]; outbuf[3] = x >> 8 | x << 8;
#else  /* HIGHFIRST */
       /* Byte order for internal and external representations is the same. */
       cipher_idea(inbuf, outbuf, Z);
#endif /* HIGHFIRST */
} /* idea_ecb */

/* initcfb - Initializes IDEA key schedule tables via key; initializes Cipher
 * Feedback mode IV. References context variables cfb_dc_idea and iv_idea.  */

void initcfb_idea(iv0, key, decryp)
    word16 iv0[4]; byte key[16]; boolean decryp;
/* iv0 is copied to global iv_idea, buffer will be destroyed by ideacfb. key is
 * pointer to key buffer. decryp is TRUE if decrypting, FALSE if encrypting. */
{
       iv_idea = iv0;
       cfb_dc_idea = decryp;
       initkey_idea(key,FALSE);
}

/* ideacfb - encipher a buffer with IDEA enciphering algorithm, using Cipher
 *  Feedback (CFB) mode. Assumes initcfb_idea has already been called. 
 * References context variables cfb_dc_idea and iv_idea.  */

void ideacfb(buf, count)
    byteptr buf; int count;
/* buf is input, output buffer, may be more than 1 block. count is byte count
 * is byte count of buffer.  May be > IDEABLOCKSIZE. */
{
       int chunksize;		     /* smaller of count, IDEABLOCKSIZE */
       word16 temp[IDEABLOCKSIZE/2];

       while ((chunksize = min(count,IDEABLOCKSIZE)) > 0)
       {
	     idea_ecb(iv_idea,temp);  /* encrypt iv_idea, making temp. */ 
	     if (cfb_dc_idea)/* buf is ciphertext */
		    /* shift in ciphertext to IV... */
		    cfbshift((byte *)iv_idea,buf,chunksize,IDEABLOCKSIZE);
	     /* convert buf via xor */
	     xorbuf(buf,(byte *)temp,chunksize);/* buf has enciphered output */
	     if (!cfb_dc_idea)/* buf was plaintext, is now ciphertext */
		    /* shift in ciphertext to IV... */
		    cfbshift((byte *)iv_idea,buf,chunksize,IDEABLOCKSIZE);
	     count -= chunksize;
	     buf += chunksize;
       }
}

/* close_idea function erases all the key schedule information when we are 
 * done with a set of operations for a particular IDEA key context. This is to
 * prevent any sensitive data from being left around in memory. */

void close_idea()	  /* erase current key schedule tables */
{
       short i;
       for (i = 0; i < KEYLEN; i++)
	     Z[i] = 0;
}      /* close_idea() */

/* These buffers are used by init_idearand, idearand, and close_idearand. */
static word16 dtbuf_idea[4] = {0};     /* buffer for enciphered timestamp */
static word16 randseed_idea[4] = {0};  /* seed for IDEA random # generator */
static word16 randbuf_idea[4] = {0};   /* buffer for IDEA random # generator */
static byte randbuf_idea_counter = 0;  /* random bytes left in randbuf_idea */

/* init_idearand - initialize idearand, IDEA random number generator. Used for
 *  generating cryptographically strong random numbers. Much of design comes 
 * from Appendix C of ANSI X9.17. key is pointer to IDEA key buffer. seed is 
 * pointer to random number seed buffer. tstamp is a 32-bit timestamp */

void init_idearand(key, seed, tstamp)
    byte key[16]; byte seed[8]; word32 tstamp;
{
       int i;
       initkey_idea(key, FALSE);      /* initialize IDEA */
       for (i=0; i<4; i++)	      /* capture timestamp material */
       {     dtbuf_idea[i] = tstamp;  /* get bottom word */
	     tstamp = tstamp >> 16;   /* drop bottom word */
	     /* tstamp has only 4 bytes-- last 4 bytes will always be 0 */
       }
       /* Start with enciphered timestamp: */
       idea_ecb(dtbuf_idea,dtbuf_idea);
       /* initialize seed material */
       for (i=0; i<8; i++)
	     ((byte *)randseed_idea)[i] = seed[i];
       randbuf_idea_counter = 0; /* # of random bytes left in randbuf_idea */
}

/* idearand - IDEA pseudo-random number generator. Used for generating 
 * cryptographically strong random numbers. Much of design comes from Appendix
 * C of ANSI X9.17.  */

byte idearand()
{
       int i;
       if (randbuf_idea_counter==0)	    /* if random buffer is spent...*/
       {     /* Combine enciphered timestamp with seed material: */
	     for (i=0; i<4; i++)
		    randseed_idea[i] ^= dtbuf_idea[i];
	     idea_ecb(randseed_idea,randbuf_idea);   /* fill new block */
	     /* Compute new seed vector: */
	     for (i=0; i<4; i++)
		    randseed_idea[i] = randbuf_idea[i] ^ dtbuf_idea[i];
	     idea_ecb(randseed_idea,randseed_idea);    /* fill new seed */
	     randbuf_idea_counter = 8;	 /* reset counter for full buffer */
       }
       /* Take a byte from randbuf_idea: */
       return(((byte *)randbuf_idea)[--randbuf_idea_counter]);
}

void close_idearand()
{      /* Erase random IDEA buffers and wipe out IDEA key info */
       int i;
       for (i=0; i<4; i++)
       {     randbuf_idea[i] = 0;
	     randseed_idea[i] = 0;
	     dtbuf_idea[i] = 0;
       }
       close_idea();/* erase current key schedule tables */
}
