// blockxor.c -- "Block XOR" (cypher).
//
// written by Marc Wallace (Merle), 07 May 93.
// first included in version 1.0.


#include "../include.h"


static Data&
encode(Data& in, Data& out)
    {
    unsigned char*  dti = in.data;   // data pointer (input stream)
    unsigned char*  dto = out.data;  // data pointer (output stream)
    int             ri;              // temp variable (input stream)
    int             ro;              // temp variable (output stream)
    int             size = in.size;  // size (input stream)

    int    s1, s2, s3;

    if (size + 4 > out.maxsize)
        out.reinit(size + 4);
    out.size = size + 4;

    // insert four random characters at the beginning
    for (ro = 0; ro < 4; ro++)
        dto[ro] = randc();

    // set up checkpoints at size/4's
    s1 = size >> 2;
    s2 = size >> 1;
    s3 = s1 + s2;

    // for the first half, xor with the second half
    for (ri = 0; ri < s2; ri++, ro++)
        dto[ro] = dti[ri] ^ dti[s2+ri];

    // for the third fourth(!), xor with the last fourth
    for (; ri < s3; ri++, ro++)
        dto[ro] = dti[ri] ^ dti[s1+ri];

    // for the last fourth, xor with our four random bytes
    for (; ri < size; ri++, ro++)
        dto[ro] = dti[ri] ^ dto[ro % 4];

    return out;
    };


static Data&
decode(Data& in, Data& out)
    {
    unsigned char*  dti = in.data;   // data pointer (input stream)
    unsigned char*  dto = out.data;  // data pointer (output stream)
    int             ri;              // temp variable (input stream)
    int             ro;              // temp variable (output stream)
    int             size = in.size;  // size (input stream)

    int    s1, s2, s3;

    size -= 4;
    if (size  > out.maxsize)
        out.reinit(size);
    out.size = size;

    // set up checkpoints at size/4's
    s1 = size >> 2;
    s2 = size >> 1;
    s3 = s1 + s2;

    // for the last fourth, xor with our four random bytes
    for (ro = s3, ri = s3 + 4; ro < size; ri++, ro++)
        dto[ro] = dti[ri] ^ dti[ri % 4];

    // for the third fourth, xor with the last fourth
    for (ro = s2, ri = s2 + 4; ro < s3; ri++, ro++)
        dto[ro] = dti[ri] ^ dto[s1+ro];

    // for the first half, xor with the second half
    for (ro = 0, ri = 4; ro < s2; ri++, ro++)
        dto[ro] = dti[ri] ^ dto[s2+ro];

    return out;
    };


#ifndef SECURE
Cypher blockxor("Block XOR", encode, decode);
#else // SECURE
Cypher blockxor("(unknown)", encode, decode);
#endif // SECURE

