// cubicxor.c -- "Cubic XOR" (cypher).
//
// written by Marc Wallace (Merle), 06 May 93.
// first included in version 1.0.


#include "../include.h"


static char arr[256];


static Data&
encode(Data& in, Data& out)
    {
    unsigned char*  dti = in.data;   // data pointer (input stream)
    unsigned char*  dto = out.data;  // data pointer (output stream)
    int             ri;              // temp variable (input stream)
    int             ro;              // temp variable (output stream)
    int             size = in.size;  // size (input stream)

    char   mask = 0;
    int    d, x;

    if (size + 4 > out.maxsize)
        out.reinit(size + 4);
    out.size = size + 4;

    // insert four random characters at the beginning
    for (ro = 0; ro < 4; ro++)
        dto[ro] = randc();

    // set up the cubic array
    for (x = 0; x < 256; x++)
        arr[x] = (dto[0] * x*x*x + dto[1] * x*x + dto[2] * x + dto[3]) & 0xff;

    // for each byte, xor it with the corresponding arr[x]
    for (ri = 0; ri < size; ri++, ro++)
        dto[ro] = dti[ri] ^ arr[ri % 256];

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

    char   mask = 0;
    int    d, x;

    size -= 4;
    if (size  > out.maxsize)
        out.reinit(size);
    out.size = size;

    // parse four random characters from the beginning
    // use them to set up the cubic array
    for (x = 0; x < 256; x++)
        arr[x] = (dti[0] * x*x*x + dti[1] * x*x + dti[2] * x + dti[3]) & 0xff;
    ri = 4;

    // for each byte, xor it with the corresponding arr[x]
    for (ro = 0; ro < size; ri++, ro++)
        dto[ro] = dti[ri] ^ arr[ro % 256];

    return out;
    };


#ifndef SECURE
Cypher cubicxor("Cubic XOR", encode, decode);
#else // SECURE
Cypher cubicxor("(unknown)", encode, decode);
#endif // SECURE

