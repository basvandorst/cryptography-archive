// diamond.cpp - modified by Wei Dai from:

/* diamond.c - Encryption designed to exceed DES in security.
   This file and the DiamondBase and DiamondBase Lite Encryption Algorithms
   described herein are hereby dedicated to the Public Domain by the
   author and inventor, Michael Paul Johnson.  Feel free to use these
   for any purpose that is legally and morally right.  The names
   "DiamondBase Encryption Algorithm" and "DiamondBase Lite Encryption
   Algorithm" should only be used to describe the algorithms described
   in this file, to avoid confusion.

   Disclaimers:  the following comes with no warranty, expressed or
   implied.  You, the user, must determine the suitability of this
   information to your own uses.  You must also find out what legal
   requirements exist with respect to this data and programs using
   it, and comply with whatever valid requirements exist.
*/
#include "misc.h"
#include "diamond.h"
#include <assert.h>

class DiamondSboxMaker
{
public:
    DiamondSboxMaker(const byte *external_key, unsigned int key_size,
                     unsigned int rounds, boolean lite);

    void MakeSbox(byte *sbox, CipherDir direction);

private:
    word16 keyrand(word16 max_value);
    void makeonebox(byte *s, unsigned int i, unsigned int j);

    CRC32 crc;
    const byte *const key;
    const unsigned int keysize;
    unsigned int keyindex;
    const int numrounds;
    const int roundsize; // Number of bytes in one round of substitution boxes
    const int blocksize;
};

DiamondSboxMaker::DiamondSboxMaker(const byte *external_key, unsigned int key_size, unsigned int rounds,
                                   boolean lite)
    : key(external_key),
      keysize(key_size),
      keyindex(0),
      numrounds(rounds),
      roundsize(lite ? 2048 : 4096),
      blocksize(lite ? 8 : 16)
{
    assert((rounds * blocksize) <= 255);
}

word16 DiamondSboxMaker::keyrand(word16 max_value)    /* Returns uniformly distributed pseudorandom */
    {                           /* value based on key[], sized keysize */
    word16 prandvalue, i;
    unsigned long mask;

    if (!max_value) return 0;
    mask = 0L;              /* Create a mask to get the minimum */
    for (i=max_value; i > 0; i = i >> 1)    /* number of bits to cover the */
        mask = (mask << 1) | 1L;        /* range 0 to max_value. */
    i=0;
    do
        {
        crc.UpdateByte(key[keyindex++]);
        if (keyindex >= keysize)
            {
            keyindex = 0;   /* Recycle thru the key */
            crc.UpdateByte(byte(keysize));
            crc.UpdateByte(byte(keysize >> 8));
            }
        prandvalue = (word16) (~crc.GetCrc() & mask);
        if ((++i>97) && (prandvalue > max_value))   /* Don't loop forever. */
            prandvalue -= max_value;                /* Introduce negligible bias. */
        }
    while (prandvalue > max_value); /* Discard out of range values. */
    return prandvalue;
    }

void DiamondSboxMaker::makeonebox(byte *s, unsigned int i, unsigned int j)
    {
    int n;
    word16 pos, m, p;
    boolean filled[256];

    for (m = 0; m < 256; m++)   /* The filled array is used to make sure that */
        filled[m] = FALSE;      /* each byte of the array is filled only once. */
    for (n = 255; n >= 0 ; n--) /* n counts the number of bytes left to fill */
        {
        pos = keyrand(n);   /* pos is the position among the UNFILLED */
                            /* components of the s array that the */
                            /* number n should be placed.  */
        p=0;
        while (filled[p]) p++;
        for (m=0; m<pos; m++)
            {
            p++;
            while (filled[p]) p++;
            }
        *(s + (roundsize*i) + (256*j) + p) = n;
        filled[p] = TRUE;
        }
    }

void DiamondSboxMaker::MakeSbox(byte *s, CipherDir direction)
{
    int i, j, k;


    for (i = 0; i < numrounds; i++)
        for (j = 0; j < blocksize; j++)
            makeonebox(s, i, j);

    if (direction==DECRYPTION)
    {
        SecByteBlock si(numrounds * roundsize);
        for (i = 0; i < numrounds; i++)
            for (j = 0; j < blocksize; j++)
                for (k = 0; k < 256; k++)
                    *(si + (roundsize * i) + (256 * j) + *(s + (roundsize * i) + (256 * j) + k)) = k;
        memcpy(s, si, numrounds * roundsize);
    }
}

DiamondBase::DiamondBase(const byte *key, unsigned int key_size,
                 unsigned int rounds, CipherDir direction)
    : numrounds(rounds),
      s(numrounds * ROUNDSIZE)
{
    DiamondSboxMaker m(key, key_size, rounds, FALSE);
    m.MakeSbox(s, direction);
}

inline void DiamondBase::substitute(int round, byte *y)
{
    const byte *sbox = s + (ROUNDSIZE*round);

    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y];
}

inline void DiamondBase::permute(byte *a)
{
#ifdef LITTLE_ENDIAN
    word32 temp0     = (a[0] | (word32(a[10])<<24)) & 0x80000001;
#else
    word32 temp0     = ((word32(a[0])<<24) | a[10]) & 0x01000080;
#endif
           temp0    |=                      permtable[0][a[1]] |
                       permtable[1][a[2]] | permtable[2][a[3]] |
                       permtable[3][a[4]] | permtable[4][a[5]] |
                       permtable[5][a[6]] | permtable[6][a[7]] |
                       permtable[7][a[8]] | permtable[8][a[9]];

#ifdef LITTLE_ENDIAN
    word32 temp1     = (a[4] | (word32(a[14])<<24)) & 0x80000001;
#else
    word32 temp1     = ((word32(a[4])<<24) | a[14]) & 0x01000080;
#endif
           temp1    |=                      permtable[0][a[5]] |
                       permtable[1][a[6]] | permtable[2][a[7]] |
                       permtable[3][a[8]] | permtable[4][a[9]] |
                       permtable[5][a[10]] | permtable[6][a[11]] |
                       permtable[7][a[12]] | permtable[8][a[13]];

#ifdef LITTLE_ENDIAN
    word32 temp2     = (a[8] | (word32(a[2])<<24)) & 0x80000001;
#else
    word32 temp2     = ((word32(a[8])<<24) | a[2]) & 0x01000080;
#endif
           temp2    |=                       permtable[0][a[9]] |
                       permtable[1][a[10]] | permtable[2][a[11]] |
                       permtable[3][a[12]] | permtable[4][a[13]] |
                       permtable[5][a[14]] | permtable[6][a[15]] |
                       permtable[7][a[0]] | permtable[8][a[1]];

#ifdef LITTLE_ENDIAN
    word32 temp3     = (a[12] | (word32(a[6])<<24)) & 0x80000001;
#else
    word32 temp3     = ((word32(a[12])<<24) | a[6]) & 0x01000080;
#endif
    ((word32 *)a)[3] = temp3 |               permtable[0][a[13]] |
                       permtable[1][a[14]] | permtable[2][a[15]] |
                       permtable[3][a[0]] | permtable[4][a[1]] |
                       permtable[5][a[2]] | permtable[6][a[3]] |
                       permtable[7][a[4]] | permtable[8][a[5]];

    ((word32 *)a)[0] = temp0;
    ((word32 *)a)[1] = temp1;
    ((word32 *)a)[2] = temp2;
}

inline void DiamondBase::ipermute(byte *a)
{
#ifdef LITTLE_ENDIAN
    word32 temp0     = (a[9] | (word32(a[3])<<24)) & 0x01000080;
#else
    word32 temp0     = ((word32(a[9])<<24) | a[3]) & 0x80000001;
#endif
           temp0    |=                      ipermtable[0][a[2]] |
                       ipermtable[1][a[1]] | ipermtable[2][a[0]] |
                       ipermtable[3][a[15]] | ipermtable[4][a[14]] |
                       ipermtable[5][a[13]] | ipermtable[6][a[12]] |
                       ipermtable[7][a[11]] | ipermtable[8][a[10]];

#ifdef LITTLE_ENDIAN
    word32 temp1     = (a[13] | (word32(a[7])<<24)) & 0x01000080;
#else
    word32 temp1     = ((word32(a[13])<<24) | a[7]) & 0x80000001;
#endif
           temp1    |=                      ipermtable[0][a[6]] |
                       ipermtable[1][a[5]] | ipermtable[2][a[4]] |
                       ipermtable[3][a[3]] | ipermtable[4][a[2]] |
                       ipermtable[5][a[1]] | ipermtable[6][a[0]] |
                       ipermtable[7][a[15]] | ipermtable[8][a[14]];

#ifdef LITTLE_ENDIAN
    word32 temp2     = (a[1] | (word32(a[11])<<24)) & 0x01000080;
#else
    word32 temp2     = ((word32(a[1])<<24) | a[11]) & 0x80000001;
#endif
           temp2    |=                      ipermtable[0][a[10]] |
                       ipermtable[1][a[9]] | ipermtable[2][a[8]] |
                       ipermtable[3][a[7]] | ipermtable[4][a[6]] |
                       ipermtable[5][a[5]] | ipermtable[6][a[4]] |
                       ipermtable[7][a[3]] | ipermtable[8][a[2]];

#ifdef LITTLE_ENDIAN
    word32 temp3     = (a[5] | (word32(a[15])<<24)) & 0x01000080;
#else
    word32 temp3     = ((word32(a[5])<<24) | a[15]) & 0x80000001;
#endif
    ((word32 *)a)[3] = temp3 |               ipermtable[0][a[14]] |
                       ipermtable[1][a[13]] | ipermtable[2][a[12]] |
                       ipermtable[3][a[11]] | ipermtable[4][a[10]] |
                       ipermtable[5][a[9]] | ipermtable[6][a[8]] |
                       ipermtable[7][a[7]] | ipermtable[8][a[6]];

    ((word32 *)a)[0] = temp0;
    ((word32 *)a)[1] = temp1;
    ((word32 *)a)[2] = temp2;
}

void DiamondEncryption::ProcessBlock(byte *y)
{
    substitute(0, y);
    for (int round=1; round < numrounds; round++)
    {
        permute(y);
        substitute(round, y);
    }
}

void DiamondEncryption::ProcessBlock(const byte *x, byte *y)
{
    memcpy(y, x, BLOCKSIZE);
    DiamondEncryption::ProcessBlock(y);
}

void DiamondDecryption::ProcessBlock(byte *y)
{
    substitute(numrounds-1, y);
    for (int round=numrounds-2; round >= 0; round--)
    {
        ipermute(y);
        substitute(round, y);
    }
}

void DiamondDecryption::ProcessBlock(const byte *x, byte *y)
{
    memcpy(y, x, BLOCKSIZE);
    DiamondDecryption::ProcessBlock(y);
}

DiamondLiteBase::DiamondLiteBase(const byte *key, unsigned int key_size,
                                 unsigned int rounds, CipherDir direction)
    : numrounds(rounds),
      s(numrounds * ROUNDSIZE)
{
    DiamondSboxMaker m(key, key_size, rounds, TRUE);
    m.MakeSbox(s, direction);
}

inline void DiamondLiteBase::substitute(int round, byte *y)
{
    const byte *sbox = s + (ROUNDSIZE*round);

    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y]; sbox += 256; y++;
    *y = sbox[*y];
}

inline void DiamondLiteBase::permute(byte *a)
{
    word32 temp      = permtable[0][a[0]] | permtable[1][a[1]] |
                       permtable[2][a[2]] | permtable[3][a[3]] |
                       permtable[4][a[4]] | permtable[5][a[5]] |
                       permtable[6][a[6]] | permtable[7][a[7]];

    ((word32 *)a)[1] = permtable[0][a[4]] | permtable[1][a[5]] |
                       permtable[2][a[6]] | permtable[3][a[7]] |
                       permtable[4][a[0]] | permtable[5][a[1]] |
                       permtable[6][a[2]] | permtable[7][a[3]];

    ((word32 *)a)[0] = temp;
}

inline void DiamondLiteBase::ipermute(byte *a)
{
    word32 temp      = ipermtable[0][a[0]] | ipermtable[1][a[1]] |
                       ipermtable[2][a[2]] | ipermtable[3][a[3]] |
                       ipermtable[4][a[4]] | ipermtable[5][a[5]] |
                       ipermtable[6][a[6]] | ipermtable[7][a[7]];

    ((word32 *)a)[1] = ipermtable[0][a[4]] | ipermtable[1][a[5]] |
                       ipermtable[2][a[6]] | ipermtable[3][a[7]] |
                       ipermtable[4][a[0]] | ipermtable[5][a[1]] |
                       ipermtable[6][a[2]] | ipermtable[7][a[3]];

    ((word32 *)a)[0] = temp;
}

void DiamondLiteEncryption::ProcessBlock(byte *y)
{
    substitute(0, y);
    for (int round=1; round < numrounds; round++)
    {
        permute(y);
        substitute(round, y);
    }
}

void DiamondLiteEncryption::ProcessBlock(const byte *x, byte *y)
{
    memcpy(y, x, BLOCKSIZE);
    DiamondLiteEncryption::ProcessBlock(y);
}

void DiamondLiteDecryption::ProcessBlock(byte *y)
{
    substitute(numrounds-1, y);
    for (int round=numrounds-2; round >= 0; round--)
    {
        ipermute(y);
        substitute(round, y);
    }
}

void DiamondLiteDecryption::ProcessBlock(const byte *x, byte *y)
{
    memcpy(y, x, BLOCKSIZE);
    DiamondLiteDecryption::ProcessBlock(y);
}

