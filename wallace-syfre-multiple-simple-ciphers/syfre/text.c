// text.c -- binary data <--> text data conversion functions.
//
// written by Marc Wallace, 14 May 93.
// last change: version 1.1 (14 May 93).


#include "include.h"


static unsigned char chars[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,";

static unsigned char bbuf[1024];
static unsigned char tbuf[1024];


unsigned char
undo(char c)
    {
    if (c >= 'A')
        return c - 'A' - ((c > 'Z') ? 6 : 0);
    else if (c == '.')
        return 62;
    else if (c == ',')
        return 63;
    else
        return c + 4;
    }


void
bin2txt(Data& d)
    {
    int size = d.size;
    int newsize = 0;
    int rt = 0, rb = 0;

    // copy the data out
    memcpy(bbuf, d.data, size);

    // find the values (0..63) for each output character
    while (rb < size)
        {
        tbuf[rt++]  = bbuf[rb]    & 0x3f;        // first  text byte
        tbuf[rt]    = bbuf[rb++] >> 6;
        tbuf[rt++] += (bbuf[rb]   & 0x0f) << 2;  // second text byte
        tbuf[rt]    = bbuf[rb++] >> 4;
        tbuf[rt++] += (bbuf[rb]   & 0x03) << 4;  // third  text byte
        tbuf[rt++]  = bbuf[rb++] >> 2;           // fourth text byte
        };
    size = (size << 2) / 3;

    // apply the chars[] array and insert carriage returns
    for (rt = 0, newsize = 0; rt < size; rt++)
        {
        d.data[newsize++] = chars[tbuf[rt]];
        if ((rt % 50) == 49)
            d.data[newsize++] = '\n';
        };
    d.size = newsize;
    }


void
txt2bin(Data& d)
    {
    int size = d.size;
    int newsize = 0;
    int rt, rb;

    // copy the data out
    memcpy(tbuf, d.data, size);

    // remove carriage returns
    for (newsize = rt = 0; rt < size; newsize++)
        {
        tbuf[newsize] = tbuf[rt++];
        if (tbuf[rt] == '\n')
            rt++;
        };
    size = newsize;

    // undo the chars[] processing
    for (rt = 0; rt < size; rt++)
        tbuf[rt] = undo(tbuf[rt]);

    // unprocess the data
    for (rb = rt = 0; rt < size;)
        {
        bbuf[rb]    = tbuf[rt++];
        bbuf[rb++] += (tbuf[rt]   & 0x03) << 6;  // first  binary byte
        bbuf[rb]    = tbuf[rt++] >> 2;
        bbuf[rb++] += (tbuf[rt]   & 0x0f) << 4;  // second binary byte
        bbuf[rb]    = tbuf[rt++] >> 4;
        bbuf[rb++] += (tbuf[rt++] & 0x3f) << 2;  // third  binary byte
        };

    // unapply the chars[] array and fix the size
    memcpy(d.data, bbuf, rb);
    d.size = rb;
    }

