// dualxor.c -- "Dual XOR" (cypher).
//
// written by Marc Wallace (Merle), 28 May 93.
// first included in version 1.1.


#include "../include.h"


static Data&
encode(Data& in, Data& out)
    {
    unsigned char*  dti = in.data;   // data pointer (input stream)
    unsigned char*  dto = out.data;  // data pointer (output stream)
    int             ri;              // temp variable (input stream)
    int             ro;              // temp variable (output stream)
    int             size = in.size;  // size (input stream)

    int i, j, k;
    int a[4], b[4];

    if (size + 4 > out.maxsize)
        out.reinit(size + 4);
    out.size = size + 4;

    // insert four random (masking) characters at the beginning
    for (ro = 0; ro < 4; ro++)
        {
        dto[ro] = a[ro] = randc();
        b[ro] = 0xff ^ a[ro];
        }

    // for each 2-byte block, perform the weird swap
    i = ri = 0;
    while (ri < size)
        {
        j = dti[ri++];
        k = dti[ri++];
        dto[ro++] = (j & a[i]) + (k & b[i]);
        dto[ro++] = (k & a[i]) + (j & b[i]);
        i = (i + 1) % 4;
        }

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

    int i, j, k;
    int a[4], b[4];

    size -= 4;
    if (size  > out.maxsize)
        out.reinit(size);
    out.size = size;

    // get the four masking characters from the beginning
    for (ri = 0; ri < 4; ri++)
        {
        a[ri] = dti[ri];
        b[ri] = 0xff ^ a[ri];
        }

    // for each 2-byte block, perform the weird swap
    i = ro = 0;
    while (ro < size)
        {
        j = dti[ri++];
        k = dti[ri++];
        dto[ro++] = (j & a[i]) + (k & b[i]);
        dto[ro++] = (k & a[i]) + (j & b[i]);
        i = (i + 1) % 4;
        }

    return out;
    };


#ifndef SECURE
Cypher dualxor("Dual XOR", encode, decode);
#else // SECURE
Cypher dualxor("(unknown)", encode, decode);
#endif // SECURE

