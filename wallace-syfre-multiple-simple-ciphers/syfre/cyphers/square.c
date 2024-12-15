// square.c -- "Square Mod 251" (cypher).
//
// written by Marc Wallace (Merle), 26 May 93.
// first included in version 1.1.


#include "../include.h"


static unsigned char s[256], r[256];


static void
init()
    {
    int i;

    // initialize s[i], the array of squares mod 251.
    // for half of the entries we need to multiply by 2 for uniqueness.
    for (i = 0; i < 251; i++)
        s[i] = (i * i) % 251;
    for (i = 126; i < 251; i++)
        s[i] = (2 * s[i]) % 251;

    // initialize r[i], the array of square roots mod 251.
    // we'll need to adjust for the doubling done above.
    for (i = 0; i < 251; i++)
        r[s[i]] = i;  // tricky, eh?

    // fill in s[i] and r[i] for i > 250 (special cases).
    for (i = 251; i < 256; i++)
        s[i] = r[i] = i;
    }


static Data&
encode(Data& in, Data& out)
    {
    unsigned char*  dti = in.data;   // data pointer (input stream)
    unsigned char*  dto = out.data;  // data pointer (output stream)
    int             ri;              // temp variable (input stream)
    int             ro;              // temp variable (output stream)
    int             size = in.size;  // size (input stream)

    if (size + 4 > out.maxsize)
        out.reinit(size + 4);
    out.size = size + 4;

    // insert two random characters at the beginning
    for (ro = 0; ro < 2; ro++)
        dto[ro] = randc();

    // take the square of everything (using s[])
    for (ri = 0; ri < size; ri++, ro++)
        dto[ro] = s[dti[ri]];

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

    size -= 4;
    if (size  > out.maxsize)
        out.reinit(size);
    out.size = size;

    // ignore two random characters at the beginning
    ri = 2;

    // take the square root of everything (using r[])
    for (ro = 0; ro < size; ri++, ro++)
        dto[ro] = r[dti[ri]];

    // ignore two random characters at the end

    return out;
    };


#ifndef SECURE
Cypher square("Square Mod 251", encode, decode, init);
#else // SECURE
Cypher square("(unknown)", encode, decode, init);
#endif // SECURE

