// sixteen.c -- "Sixteen Square Rotation" (cypher).
//
// written by Marc Wallace (Merle), 26 May 93.
// first included in version 1.1.


#include "../include.h"


static unsigned char s[256];


static void
init()
    {
    int i;

    // s[] is a rotation matrix, sending the byte [aaaabbbb]->[bbbbaaaa].
    // in other words:
    for (i = 0; i < 256; i++)
        s[i] = 16 * (i % 16) + (i / 16);
    // luckily, s[] is its own inverse.. ;-)
    }


static Data&
encode(Data& in, Data& out)
    {
    unsigned char*  dti = in.data;   // data pointer (input stream)
    unsigned char*  dto = out.data;  // data pointer (output stream)
    int             ri;              // temp variable (input stream)
    int             ro;              // temp variable (output stream)
    int             size = in.size;  // size (input stream)

    int i;

    if (size + 4 > out.maxsize)
        out.reinit(size + 4);
    out.size = size + 4;

    // insert three random characters at the beginning
    for (ro = 0; ro < 3; ro++)
        dto[ro] = randc();

    // for each 256-byte block, perform the rotation on it
    ri = 0;
    while (size - ri > 255)
        {
        for (i = 0; i < 256; i++, ro++)
            dto[ro] = dti[ri + s[i]];
        ri += 256;
        }

    // for the rest, just output the bloody stuff
    while (ri < size)
        dto[ro++] = dti[ri++];

    // insert one random character at the end
    dto[ro++] = randc();

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

    int i;

    size -= 4;
    if (size  > out.maxsize)
        out.reinit(size);
    out.size = size;

    // ignore three random characters at the beginning
    ri = 3;

    // for each 256-byte block, perform the rotation on it
    ro = 0;
    while (size - ro > 255)
        {
        for (i = 0; i < 256; i++, ro++)
            dto[ro] = dti[ri + s[i]];
        ri += 256;
        }

    // for the rest, just output the bloody stuff
    while (ro < size)
        dto[ro++] = dti[ri++];

    // ignore one random character at the end

    return out;
    };


#ifndef SECURE
Cypher sixteen("Sixteen Square Rotation", encode, decode, init);
#else // SECURE
Cypher sixteen("(unknown)", encode, decode, init);
#endif // SECURE

