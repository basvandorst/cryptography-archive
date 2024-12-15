// md5.cpp - modified by Wei Dai from Eric Young's md5_dgst.c

#include "md5.h"
#include "misc.h"

static const int MD5_LAST_BLOCK = 56;

MD5::MD5()
    : buffer(DIGESTSIZE/4 + DATASIZE/4),
      digest(buffer), data(digest+DIGESTSIZE/4)
{
    Init();
}

void MD5::Init ()
{
    countLo = countHi = 0;

    digest[0] = 0x67452301L;
    digest[1] = 0xefcdab89L;
    digest[2] = 0x98badcfeL;
    digest[3] = 0x10325476L;
}

inline void MD5::do_block (const word32 *input)
{
#ifdef LITTLE_ENDIAN
    Transform(digest, input);
#else
    byteReverse(data, input, DATASIZE);
    Transform(digest, data);
#endif
}

void MD5::Update (const byte *input, unsigned int len)
{
    word32 tmp = countLo;
    if ((countLo = tmp + ((word32)len << 3)) < tmp)
        countHi++;             /* Carry from low to high */
    countHi += len >> 29;

    int num = ((int)tmp >> 3) & 0x3F;

    if (num != 0)
    {
        if ((num+len) >= DATASIZE)
        {
            memcpy((byte *)data+num, input, DATASIZE-num);
            do_block(data);
            input += (DATASIZE-num);
            len-=(DATASIZE - num);
            num=0;
            // drop through and do the rest
        }
        else
        {
            memcpy((byte *)data+num, input, len);
            return;
        }
    }

    // we now can process the input data in blocks of DATASIZE
    // chars and save the leftovers to this->data.
    if (len >= DATASIZE)
    {
        if ((int)input&3)   // test for 32-bit word alignment
            do
            {   // copy input first if it's not aligned correctly
                memcpy(data, input, DATASIZE);
                do_block(data);
                input+=DATASIZE;
                len-=DATASIZE;
            } while (len >= DATASIZE);
        else
            do
            {
                do_block((word32 *)input);
                input+=DATASIZE;
                len-=DATASIZE;
            } while (len >= DATASIZE);
    }

    memcpy(data, input, len);
}

void MD5::Final (byte *hash)
{
    int num = ((int)countLo >> 3) & 0x3F;
    // this->data should definitly have room for at least one more byte.
    ((byte *)data)[num++]=0x80;
    if (num > MD5_LAST_BLOCK)
    {
        memset((byte *)data+num, 0, DATASIZE-num);
        do_block(data);
        memset(data, 0, MD5_LAST_BLOCK);
    }
    else
    {
        memset((byte *)data+num, 0, MD5_LAST_BLOCK-num);
        CorrectEndianess(data, data, MD5_LAST_BLOCK);
    }

    data[14] = countLo;
    data[15] = countHi;

    Transform(digest, data);
    CorrectEndianess(digest, digest, DIGESTSIZE);
    memcpy(hash, digest, DIGESTSIZE);

    // reinit for next use
    Init();
}

void MD5::Transform (word32 *digest, const word32 *X)
{
// #define	F(x,y,z)	((x & y)  |  (~x & z))
#define F(x,y,z)    (z ^ (x & (y^z)))
// #define	G(x,y,z)	((x & z)  |  (y & ~z))
#define G(x,y,z)    (y ^ (z & (x^y)))
#define	H(x,y,z)	(x ^ y ^ z)
#define	I(x,y,z)	(y  ^  (x | ~z))

#define ROTATE(a,n)	a=(((a)<<(n))|((a)>>(32-(n))))

#define R0(a,b,c,d,k,s,t) { \
    a+=(k+t+ F((b),(c),(d))); \
    a&=0xffffffff;\
    ROTATE(a,s); \
    a+=b; };\

#define R1(a,b,c,d,k,s,t) { \
    a+=(k+t+ G((b),(c),(d))); \
    a&=0xffffffff;\
    ROTATE(a,s); \
    a+=b; };

#define R2(a,b,c,d,k,s,t) { \
    a+=(k+t+ H((b),(c),(d))); \
    a&=0xffffffff;\
    ROTATE(a,s); \
    a+=b; };

#define R3(a,b,c,d,k,s,t) { \
    a+=(k+t+ I((b),(c),(d))); \
    a&=0xffffffff;\
    ROTATE(a,s); \
    a+=b; };

    register unsigned long A,B,C,D;

    A=digest[0];
    B=digest[1];
    C=digest[2];
    D=digest[3];

    /* Round 0 */
    R0(A,B,C,D,X[ 0], 7,0xd76aa478);
    R0(D,A,B,C,X[ 1],12,0xe8c7b756);
    R0(C,D,A,B,X[ 2],17,0x242070db);
    R0(B,C,D,A,X[ 3],22,0xc1bdceee);
    R0(A,B,C,D,X[ 4], 7,0xf57c0faf);
    R0(D,A,B,C,X[ 5],12,0x4787c62a);
    R0(C,D,A,B,X[ 6],17,0xa8304613);
    R0(B,C,D,A,X[ 7],22,0xfd469501);
    R0(A,B,C,D,X[ 8], 7,0x698098d8);
    R0(D,A,B,C,X[ 9],12,0x8b44f7af);
    R0(C,D,A,B,X[10],17,0xffff5bb1);
    R0(B,C,D,A,X[11],22,0x895cd7be);
    R0(A,B,C,D,X[12], 7,0x6b901122);
    R0(D,A,B,C,X[13],12,0xfd987193);
    R0(C,D,A,B,X[14],17,0xa679438e);
    R0(B,C,D,A,X[15],22,0x49b40821);
    /* Round 1 */
    R1(A,B,C,D,X[ 1], 5,0xf61e2562);
    R1(D,A,B,C,X[ 6], 9,0xc040b340);
    R1(C,D,A,B,X[11],14,0x265e5a51);
    R1(B,C,D,A,X[ 0],20,0xe9b6c7aa);
    R1(A,B,C,D,X[ 5], 5,0xd62f105d);
    R1(D,A,B,C,X[10], 9,0x02441453);
    R1(C,D,A,B,X[15],14,0xd8a1e681);
    R1(B,C,D,A,X[ 4],20,0xe7d3fbc8);
    R1(A,B,C,D,X[ 9], 5,0x21e1cde6);
    R1(D,A,B,C,X[14], 9,0xc33707d6);
    R1(C,D,A,B,X[ 3],14,0xf4d50d87);
    R1(B,C,D,A,X[ 8],20,0x455a14ed);
    R1(A,B,C,D,X[13], 5,0xa9e3e905);
    R1(D,A,B,C,X[ 2], 9,0xfcefa3f8);
    R1(C,D,A,B,X[ 7],14,0x676f02d9);
    R1(B,C,D,A,X[12],20,0x8d2a4c8a);
    /* Round 2 */
    R2(A,B,C,D,X[ 5], 4,0xfffa3942);
    R2(D,A,B,C,X[ 8],11,0x8771f681);
    R2(C,D,A,B,X[11],16,0x6d9d6122);
    R2(B,C,D,A,X[14],23,0xfde5380c);
    R2(A,B,C,D,X[ 1], 4,0xa4beea44);
    R2(D,A,B,C,X[ 4],11,0x4bdecfa9);
    R2(C,D,A,B,X[ 7],16,0xf6bb4b60);
    R2(B,C,D,A,X[10],23,0xbebfbc70);
    R2(A,B,C,D,X[13], 4,0x289b7ec6);
    R2(D,A,B,C,X[ 0],11,0xeaa127fa);
    R2(C,D,A,B,X[ 3],16,0xd4ef3085);
    R2(B,C,D,A,X[ 6],23,0x04881d05);
    R2(A,B,C,D,X[ 9], 4,0xd9d4d039);
    R2(D,A,B,C,X[12],11,0xe6db99e5);
    R2(C,D,A,B,X[15],16,0x1fa27cf8);
    R2(B,C,D,A,X[ 2],23,0xc4ac5665);
    /* Round 3 */
    R3(A,B,C,D,X[ 0], 6,0xf4292244);
    R3(D,A,B,C,X[ 7],10,0x432aff97);
    R3(C,D,A,B,X[14],15,0xab9423a7);
    R3(B,C,D,A,X[ 5],21,0xfc93a039);
    R3(A,B,C,D,X[12], 6,0x655b59c3);
    R3(D,A,B,C,X[ 3],10,0x8f0ccc92);
    R3(C,D,A,B,X[10],15,0xffeff47d);
    R3(B,C,D,A,X[ 1],21,0x85845dd1);
    R3(A,B,C,D,X[ 8], 6,0x6fa87e4f);
    R3(D,A,B,C,X[15],10,0xfe2ce6e0);
    R3(C,D,A,B,X[ 6],15,0xa3014314);
    R3(B,C,D,A,X[13],21,0x4e0811a1);
    R3(A,B,C,D,X[ 4], 6,0xf7537e82);
    R3(D,A,B,C,X[11],10,0xbd3af235);
    R3(C,D,A,B,X[ 2],15,0x2ad7d2bb);
    R3(B,C,D,A,X[ 9],21,0xeb86d391);

    digest[0]+=A;
    digest[1]+=B;
    digest[2]+=C;
    digest[3]+=D;
}

