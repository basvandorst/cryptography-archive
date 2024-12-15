/* Copyright (C) 1998-9 Network Associates, Inc.
   Author: Michael_Elkins@NAI.com
   Last Edit: April 9, 1999 */

/* PKCS#12 PBE (Password Based Encryption) Algorithm */

#include "pbe12.h"

#undef LOUD

#ifdef LOUD
static void pbe12DumpBytes (const char *comment, PGPByte *b, PGPSize bsize)
{
    PGPSize i;

    if(comment)
    {
	fputc('\n',stdout);
	puts(comment);
    }
    fputc('\n',stdout);
    for(i=0;i<bsize;i++)
    {
	if(i%14==0)
	    puts("");
	printf("0x%02X,", b[i]);
    }
    puts("");
}
#endif

/* concatenate copies of `pad' to create a string which is a multiple of
   `v' bytes */
static PGPError
pbe12PadString (
    PGPMemoryMgrRef	mem,		/* [IN] */
    const unsigned char	*pad,		/* [IN] */
    size_t		padSize,	/* [IN] */
    size_t		v,		/* [IN] hash input block size */
    unsigned char	**str,		/* [OUT] */
    size_t		*strSize)	/* [OUT] */
{
    size_t bytes = 0;
    size_t count;

    *strSize = ((padSize + v - 1) / v) * v;
    if(*strSize)
    {
	*str = PGPNewSecureData(mem, *strSize, 0);
	while(bytes < *strSize)
	{
	    count = *strSize - bytes;
	    if(count > padSize)
		count = padSize;
	    memcpy(*str + bytes, pad, count);
	    bytes += count;
	}
    }
    else
	*str=NULL;
    return kPGPError_NoErr;
}

/* compute a = (a + b + c) % 2^v */
static void pbe12AddBytesModV (PGPByte *a, PGPByte *b, PGPByte c, PGPSize v)
{
    int i;
    unsigned int j;
    
    for (i = v - 1; i >= 0; i--)
    {
	j = a[i] + b[i] + c;
	a[i] = j & 0xff;
	c = (j >> 8) & 0xff;
    }
}

PGPError
pbe12GenerateRandomBits (
    PGPContextRef	context,
    PGPHashAlgorithm	alg,
    unsigned char	id,
    size_t		iterationCount,
    const unsigned char	*salt,
    size_t		saltSize,
    const unsigned char	*password,
    size_t		passwordSize,
    size_t		n,		/* [IN] n, num of random bytes */
    unsigned char	**A)		/* [OUT] random bit string */
{
    unsigned char	*S;
    unsigned char	*P;
    unsigned char	*I;
    unsigned char	*B;
    size_t		Sbytes;
    size_t		Pbytes;
    size_t		Ibytes;
    size_t		Bbytes;
    size_t		c;
    size_t 		i;
    size_t		j;
    size_t		u;
    size_t		v = 64;
    size_t		Aoffset = 0;
    size_t		Ioff;

    PGPHashContextRef	hashref;
    PGPByte				*hashout;
    PGPByte				*hashinput;
    PGPSize				hashinputsize;
    PGPMemoryMgrRef		mem = PGPGetContextMemoryMgr(context);

    switch (alg)
    {
	case kPGPHashAlgorithm_SHA:
	    u = 20;
	    break;
	case kPGPHashAlgorithm_MD5:
	    u = 16;
	    break;
	default:
	    return kPGPError_BadHashNumber;
    }

    pbe12PadString (mem, salt, saltSize, v, &S, &Sbytes);
    pbe12PadString (mem, password, passwordSize, v, &P, &Pbytes);
    Ibytes = Sbytes + Pbytes;
    I = PGPNewSecureData (mem, Ibytes, 0);
    memcpy (I, S, Sbytes);
    memcpy (I + Sbytes, P, Pbytes);
#ifdef LOUD
    pbe12DumpBytes("Stretched Salt:",S,Sbytes);
    pbe12DumpBytes("Stretched Password:",P,Pbytes);
#endif
    if (S)
	PGPFreeData (S);
    if (P)
	PGPFreeData (P);

    PGPNewHashContext (mem, alg, &hashref);
    hashout = PGPNewSecureData (mem, u, 0);
    hashinputsize = v + Ibytes;
    hashinput = PGPNewSecureData (mem, hashinputsize, 0);

    c = (n + u - 1) / u; /* calculate the number of iterations needed to
			    get `n' bytes worth of data */
#ifdef LOUD
    printf(">>> Number of digests required: %d\n", c);
#endif

    *A = PGPNewSecureData (mem, c * u, 0);	/* allocate enough storage for
						   the output, which may be
						   more than `n' */
    for (i = 1; i <= c; i++)
    {
#ifdef LOUD
	printf("\n>>> Computing A%d\n", i - 1);
	printf("I value for A%d",i-1);
	pbe12DumpBytes(NULL, I, Ibytes);
#endif

	/* A = H^c(D||I) */
	memset (hashinput, id, v);	/* D, which is constant */
	memcpy (hashinput + v, I, Ibytes);
	hashinputsize = v + Ibytes;	/* reset */
	for (j = 0; j < iterationCount; j++)
	{
		PGPResetHash (hashref);
	    PGPContinueHash (hashref, hashinput, hashinputsize);
	    PGPFinalizeHash (hashref, hashout);	/* ASSUMES hashinputsize
						   is larger than u */
#ifdef LOUD
	    pbe12DumpBytes("Hash of D||I:",hashout,u);
#endif
	    memcpy (hashinput, hashout, u);
	    hashinputsize = u;
	}

	memcpy (*A + Aoffset, hashout, u);
	Aoffset += u;

	/* concatenate the hash val to make a string of 'v' bytes */
	pbe12PadString (mem, hashout, u, v, &B, &Bbytes);
#ifdef LOUD
	printf("\nB value as stretching A%d", i - 1);
	pbe12DumpBytes(NULL,B,Bbytes);
#endif

	/* I[j] = (I[j] + B + 1) % 2^v */
	for (Ioff = 0; Ioff < Ibytes; Ioff += v)
	{
	    pbe12AddBytesModV (I + Ioff, B, 1, v);
#ifdef LOUD
	    printf("\nB addition for %d-k-th piece:", Ioff/v);
	    pbe12DumpBytes(NULL,I+Ioff,v);
#endif
	}

#ifdef LOUD
	printf("\n>>>> Output A%d", i - 1);
	pbe12DumpBytes(NULL,*A +Aoffset-u,u);
#endif
	PGPFreeData (B);
    }
    PGPFreeHashContext (hashref);
    PGPFreeData (hashinput);
    PGPFreeData (hashout);
    PGPFreeData (I);

    return kPGPError_NoErr;
}

/* converts from ASCII password to BMPString */
PGPError
pbe12ASCIIToBMPString (
    PKICONTEXT		*context,
    const unsigned char	*ascii,
    size_t		asciilen,
    unsigned char	**bmp,
    size_t		*bmplen)
{
    size_t n;

    *bmplen = (asciilen + 1) * 2;
    *bmp = PKIAlloc(context->memMgr, *bmplen);
    if(!*bmp)
	return kPGPError_OutOfMemory;
    for(n=0;n<*bmplen-2;n+=2)
    {
	(*bmp)[n] = 0;
	(*bmp)[n+1] = *ascii++;
    }
    (*bmp)[n]=0;
    (*bmp)[n+1]=0;
    return kPGPError_NoErr;
}
