// idea.cpp - modified by Wei Dai from:

/*      idea.c - C source code for IDEA block cipher.
 *      IDEA (International Data Encryption Algorithm), formerly known as
 *      IPES (Improved Proposed Encryption Standard).
 *      Algorithm developed by Xuejia Lai and James L. Massey, of ETH Zurich.
 *      This implementation modified and derived from original C code
 *      developed by Xuejia Lai.
 *      Zero-based indexing added, names changed from IPES to IDEA.
 *
 *  Optimized for speed 21 Oct 92 by Colin Plumb.
 *
 *      The IDEA(tm) block cipher is covered by a patent held by ETH and a
 *      Swiss company called Ascom-Tech AG.  The Swiss patent number is
 *      PCT/CH91/00117.  International patents are pending. IDEA(tm) is a
 *      trademark of Ascom-Tech AG.  There is no license fee required for
 *      noncommercial use.  Commercial users may obtain licensing details
 *      from Dieter Profos, Ascom Tech AG, Solothurn Lab, Postfach 151, 4502
 *      Solothurn, Switzerland, Tel +41 65 242885, Fax +41 65 235761.
 *
 *      The IDEA block cipher uses a 64-bit block size, and a 128-bit key
 *      size.  It breaks the 64-bit cipher block into four 16-bit words
 *      because all of the primitive inner operations are done with 16-bit
 *      arithmetic.  It likewise breaks the 128-bit cipher key into eight
 *      16-bit words.
 *
 *      For further information on the IDEA cipher, see these papers:
 *      1) Xuejia Lai, "Detailed Description and a Software Implementation of
 *         the IPES Cipher", Institute for Signal and Information
 *         Processing, ETH-Zentrum, Zurich, Switzerland, 1991
 *      2) Xuejia Lai, James L. Massey, Sean Murphy, "Markov Ciphers and
 *         Differential Cryptanalysis", Advances in Cryptology- EUROCRYPT'91
 *
 *      This code assumes that each pair of 8-bit bytes comprising a 16-bit
 *      word in the key and in the cipher block are externally represented
 *      with the Most Significant Byte (MSB) first, regardless of the
 *      internal native byte order of the target CPU.
 */

#include <memory.h>
#include "misc.h"
#include "idea.h"

static const int IDEA_ROUNDS=8;
static const int IDEA_KEYLEN=(6*IDEA_ROUNDS+4);  // in # of word16s

#ifdef IDEA32                   /* Use >16-bit temporaries */
#define low16(x) ((x) & 0xFFFF)
typedef unsigned int uint16;    /* at LEAST 16 bits, maybe more */
#else
#define low16(x) (x)            /* this is only ever applied to uint16's */
typedef word16 uint16;
#endif

IDEA::IDEA (const byte * userKey, CipherDir direction)
    : key(IDEA_KEYLEN)
{
    EnKey(userKey);

    if (direction==DECRYPTION)
        DeKey();
}

void IDEA::EnKey (const byte *userKey)
{
   word16 *Z=key;
   for (int j=0;j<8;j++)
       Z[j] = (userKey[2*j]<<8) + userKey[2*j+1];
   for (int i=0;j<IDEA_KEYLEN;j++)
   {
      i++;
      Z[i+7]=((Z[i&7] << 9) | (Z[i+1 & 7] >> 7));
      Z+=i&8;
      i&=7;
   }
}

static uint16 inv(uint16 x)
{
   uint16 t0,t1;
   uint16 q,y;
   if (x<=1)
      return x;
   t1=(uint16)(0x10001l/x);
   y=(uint16)(0x10001l%x);
   if (y==1)
        return low16(1-t1);
   t0=1;
   do
   {
      q=x/y;
      x=x%y;
      t0+=q*t1;
      if (x==1)
            return t0;
      q=y/x;
      y=y%x;
      t1+=q*t0;
   } while (y!=1);
   return low16(1-t1);
}

void IDEA::DeKey()
{
   word16 *Z=key;
   int j;
   uint16 t1,t2,t3;
   SecBlock<word16> tempKey(IDEA_KEYLEN);
   word16 *p=(word16 *)tempKey+IDEA_KEYLEN;
   t1=inv(*Z++);
   t2=-*Z++;
   t3=-*Z++;
   *--p=inv(*Z++);
   *--p=t3;
   *--p=t2;
   *--p=t1;
   for (j=1;j<IDEA_ROUNDS;j++)
   {
      t1=*Z++;
      *--p=*Z++;
      *--p=t1;
      t1=inv(*Z++);
      t2=-*Z++;
      t3=-*Z++;
      *--p=inv(*Z++);
      *--p=t2;
      *--p=t3;
      *--p=t1;
   }
   t1=*Z++;
   *--p=*Z++;
   *--p=t1;
   t1=inv(*Z++);
   t2=-*Z++;
   t3=-*Z++;
   *--p=inv(*Z++);
   *--p=t3;
   *--p=t2;
   *--p=t1;
   /*copy and destroy temp copy*/
   memcpy(key, tempKey, IDEA_KEYLEN*sizeof(word16));
}

#ifdef IDEA_AVOID_JUMPS     // this version is slower on Wei's 486/50
static inline void MUL(uint16 &x, const uint16 y)
{
    word32 t32;
    uint16 t16;

    x = low16(x-1);
    t16 = low16(y-1);
    t32 = (word32)x*t16+x+t16+1;
    x = low16(t32);
    t16 = t32>>16;
    x = x-t16+(x<t16);
}
#else
static inline void MUL(uint16 &a, uint16 b)
{
    word32 p;

    if ((a=low16(a))!=0)
    {
        if (b)
        {
            p=(word32)a*b;
            b=(uint16)low16(p);
            a=(uint16)(p>>16);
            a=low16(b-a+(b<a));
        }
        else
        {
            a=low16(1-a);
        }
    }
    else
        a=low16(1-b);
}
#endif

void IDEA::ProcessBlock(const byte *in, byte *out)
{
   uint16 x1,x2,x3,x4,t1,t2;
#ifdef LITTLE_ENDIAN
   x1=Invert(((word16 *)in)[0]); x2=Invert(((word16 *)in)[1]);
   x3=Invert(((word16 *)in)[2]); x4=Invert(((word16 *)in)[3]);
#else
   x1=((word16 *)in)[0]; x2=((word16 *)in)[1];
   x3=((word16 *)in)[2]; x4=((word16 *)in)[3];
#endif

   const word16* Z=key;
   int r=IDEA_ROUNDS;
   do
   {
      MUL(x1,*Z++);
      x2+=*Z++;
      x3+=*Z++;
      MUL(x4,*Z++);
      t2=x1^x3;
      MUL(t2,*Z++);
      t1=t2+(x2^x4);
      MUL(t1,*Z++);
      t2=t1+t2;
      x1^=t1;
      x4^=t2;
      t2^=x2;
      x2=x3^t1;
      x3=t2;
   } while (--r);
   MUL(x1,*Z++);
   x3+=*Z++;
   x2+=*Z++;
   MUL(x4,*Z);

#ifdef LITTLE_ENDIAN
   ((word16 *)out)[0]=Invert(word16(x1));
   ((word16 *)out)[1]=Invert(word16(x3));
   ((word16 *)out)[2]=Invert(word16(x2));
   ((word16 *)out)[3]=Invert(word16(x4));
#else
   ((word16 *)out)[0]=x1;
   ((word16 *)out)[1]=x3;
   ((word16 *)out)[2]=x2;
   ((word16 *)out)[3]=x4;
#endif
}

