// recurxor.c -- "Recursive XOR" (cypher).
//
// written by Marc Wallace (Merle), 30 Apr 93.
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

    char   mask = 0;

    if (size + 4 > out.maxsize)
        out.reinit(size + 4);
    out.size = size + 4;

    // insert two random characters at the beginning
    for (ro = 0; ro < 2; ro++)
        dto[ro] = randc();

    // start up the xor machine
    for (ri = 0; ri < size; ri++, ro++)
        {
        mask ^= dti[ri];
        dto[ro] = mask;
        };

    // insert two random characters at the end
    dto[ro++] = randc();
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

    char   mask = 0;

    size -= 4;
    if (size  > out.maxsize)
        out.reinit(size);
    out.size = size;

    // ignore two random characters at the beginning
    ri = 2;

    // start up the xor machine
    for (ro = 0; ro < size; ri++, ro++)
        {
        dto[ro] = mask ^ dti[ri];
        mask = dti[ri];
        };

    // ignore two random characters at the end
    // this part is pretty easy ... ;-)

    return out;
    };


#ifndef SECURE
Cypher recurxor("Recursive XOR", encode, decode);
#else // SECURE
Cypher recurxor("(unknown)", encode, decode);
#endif // SECURE

