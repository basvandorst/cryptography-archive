// rotate.c -- "Byte Rotation" (cypher).
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

    int i, curr, tmp;
    char a[8];

    if (size + 4 > out.maxsize)
        out.reinit(size + 4);
    out.size = size + 4;

    // insert four random characters at the beginning
    // these represent a[0..7], four random numbers between 2 and 15
    for (i = 0; i < 8; i++)
        a[i] = randc(1, 15);
    for (ro = 0; ro < 4; ro++)
        dto[ro] = 16 * a[2 * ro] + a[2 * ro + 1];

    // for each possible block of length a[curr], rotate it
    curr = 0;
    ri = 0;
    while (size - ri > a[curr])
        {
        // { a0 a1 a2 .. an } --> { a1 a2 .. an a0 }
        tmp = dti[ri++];
        for (i = 1; i < a[curr]; i++)
            dto[ro++] = dti[ri++];
        dto[ro++] = tmp;
        curr = (curr + 1) % 8;
        }

    // for the rest, just output the bloody stuff
    while (ri < size)
        dto[ro++] = dti[ri++];

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

    int i, curr, marker;
    int a[8];

    size -= 4;
    if (size  > out.maxsize)
        out.reinit(size);
    out.size = size;

    // get four random characters from the beginning
    // these represent a[0..7], four random numbers between 2 and 15
    for (ri = 0; ri < 4; ri++)
        {
        a[2 * ri]     = dti[ri] / 16;
        a[2 * ri + 1] = dti[ri] % 16;
        }

    // for each possible block of length a[curr], unrotate it
    curr = 0;
    ro = 0;
    while (size - ro > a[curr])
        {
        // { a0 a1 a2 .. an } <-- { a1 a2 .. an a0 }
        marker = ro++;
        for (i = 1; i < a[curr]; i++)
            dto[ro++] = dti[ri++];
        dto[marker] = dti[ri++];
        curr = (curr + 1) % 8;
        }

    // for the rest, just output the bloody stuff
    while (ro < size)
        dto[ro++] = dti[ri++];

    return out;
    };


#ifndef SECURE
Cypher rotate("Byte Rotation", encode, decode);
#else // SECURE
Cypher rotate("(unknown)", encode, decode);
#endif // SECURE

