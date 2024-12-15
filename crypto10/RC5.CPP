#include <memory.h>
#include "misc.h"
#include "rc5.h"

static const unsigned int MASK=sizeof(RC5_WORD)*8-1;     // mask for rotation
#define ROTL(x,y) (rotl((x), (unsigned int)(y) & MASK))
#define ROTR(x,y) (rotr((x), (unsigned int)(y) & MASK))

RC5Base::RC5Base(const byte *k, unsigned int keylen, unsigned int rounds)
    : r(rounds), sTable(2*(r+1))
{
    static const RC5_WORD MAGIC_P = 0xb7e15163L;    // magic constant P for wordsize
    static const RC5_WORD MAGIC_Q = 0x9e3779b9L;    // magic constant Q for wordsize
    static const int U=sizeof(RC5_WORD);

    const unsigned int c=(keylen-1)/U + 1;
    SecBlock<RC5_WORD> l(c);

    for (int i = (keylen-1) ; i >= 0; i--)
        l[i/U] = (l[i/U] << 8) + k[i];

    for (sTable[0] = MAGIC_P, i=1; i<sTable.size;i++)
        sTable[i] = sTable[i-1] + MAGIC_Q;

    RC5_WORD a=0, b=0;
    const int n = 3*max(sTable.size,c);

    for (int h = 0; h < n; h++)
    {
        a = sTable[h % sTable.size] = ROTL((sTable[h % sTable.size] + a + b), 3);
        b = l[h % c] = ROTL((l[h % c] + a + b), (a+b));
    }
}

// Fetch 8 bytes from user's buffer into "left" and "right"
// in LITTLE-endian order
inline void RC5Base::GETBLOCK(const byte *block, word32 &left, word32 &right)
{
#ifdef LITTLE_ENDIAN
    left = *(word32 *)block;
    right = *(word32 *)(block+4);
#else
    left = Invert(*(word32 *)block);
    right = Invert(*(word32 *)(block+4));
#endif
}

// Put 8 bytes back into user's buffer in LITTLE-endian order */
inline void RC5Base::PUTBLOCK(byte *block, word32 left, word32 right)
{
#ifdef LITTLE_ENDIAN
    *(word32 *)block = left;
    *(word32 *)(block+4) = right;
#else
    *(word32 *)block = Invert(left);
    *(word32 *)(block+4) = Invert(right);
#endif
}

void RC5Encryption::ProcessBlock(const byte *in, byte *out)
{
    const RC5_WORD *sptr = sTable;
    RC5_WORD a, b;

    GETBLOCK(in, a, b);
    a += *sptr++;
    b += *sptr++;

    for (int i=r; i; i--)
    {
        a = ROTL(a^b,b) + *sptr++;
        b = ROTL(a^b,a) + *sptr++;
    }

    PUTBLOCK(out, a, b);
}

void RC5Decryption::ProcessBlock(const byte *in, byte *out)
{
    const RC5_WORD *sptr = sTable+sTable.size;
    RC5_WORD a, b;

    GETBLOCK(in, a, b);

    int i=r;
    do
    {
        b -= *--sptr; b = ROTR(b, a) ^ a;
        a -= *--sptr; a = ROTR(a, b) ^ b;
    } while (--i);

    b -= *--sptr;
    a -= *--sptr;
    PUTBLOCK(out, a, b);
}

