// quadadd.c -- "Quadratic Addition" (cypher).
//
// written by Marc Wallace (Merle), 12 May 93.
// first included in version 1.1.


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
    for (ro = 0; ro < 3; ro++)
        dto[ro] = randc();
    // making sure the fourth one is not too small
    // and not too big (has to be positive > 2)
    d = (dto[3] = randc(2,127));
    ro++;

    // set up the quadratic array
    for (x = 0; x < d; x++)
        arr[x] = (dto[0] * x * x + dto[1] * x + dto[2]) & 0xff;

    // process and output the input, byte by byte
    for (ri = 0; ri < size; ri++, ro++)
        dto[ro] = (dti[ri] + arr[ri % d]) & 0xff;

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
    // set up the quadratic array backwards (negative)
    // by xor'ing it with 0xff ...
    d = dti[3];
    for (x = 0; x < d; x++)
        arr[x] = -((dti[0] * x * x + dti[1] * x + dti[2]) & 0xff);
    ri = 4;

    // process and decode the input, byte by byte
    for (ro = 0; ro < size; ri++, ro++)
        dto[ro] = (dti[ri] + arr[ro % d]) & 0xff;

    return out;
    };


#ifndef SECURE
Cypher quadadd("Quadratic Addition", encode, decode);
#else // SECURE
Cypher quadadd("(unknown)", encode, decode);
#endif // SECURE

