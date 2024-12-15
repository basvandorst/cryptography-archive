
#pragma ident "@(#)md2.c	1.2 95/11/16 Sun Microsystems"

/* 
 * An implementation of the MD2 message-digest algorithm.
 */
#include <string.h>     /* For memcpy and memset */
#include "md2.h"

/*
 * The temporary buffer is permanently allocated here because it
 * makes the implementation easier, it's not a lot of overhead
 * to pre-allocate, and it avoids having to wipe a separate temporary
 * buffer in the transform routine each time.
 */

static void
md2Transform(struct xMD2Context *md2)
{
        unsigned i, j;
        unsigned char t;
        /* Non-linear permutation table, derived from the digits of Pi. */
        static unsigned char const permutation[256] = {
             41, 46, 67,201,162,216,124,  1, 61, 54, 84,161,236,240,  6, 19,
             98,167,  5,243,192,199,115,140,152,147, 43,217,188, 76,130,202,
             30,155, 87, 60,253,212,224, 22,103, 66,111, 24,138, 23,229, 18,
            190, 78,196,214,218,158,222, 73,160,251,245,142,187, 47,238,122,
            169,104,121,145, 21,178,  7, 63,148,194, 16,137, 11, 34, 95, 33,
            128,127, 93,154, 90,144, 50, 39, 53, 62,204,231,191,247,151,  3,
            255, 25, 48,179, 72,165,181,209,215, 94,146, 42,172, 86,170,198,
             79,184, 56,210,150,164,125,182,118,252,107,226,156,116,  4,241,
             69,157,112, 89,100,113,135, 32,134, 91,207,101,230, 45,168,  2,
             27, 96, 37,173,174,176,185,246, 28, 70, 97,105, 52, 64,126, 15,
             85, 71,163, 35,221, 81,175, 58,195, 92,249,206,186,197,234, 38,
             44, 83, 13,110,133, 40,132,  9,211,223,205,244, 65,129, 77, 82,
            106,220, 55,200,108,193,171,250, 36,225,123,  8, 12,189,177, 74,
            120,136,149,139,227, 99,232,109,233,203,213,254, 59,  0, 29, 57,
            242,239,183, 14,102, 88,208,228,166,119,114,248,235,117, 75, 10,
             49, 68, 80,180,143,237, 31, 26,219,153,141, 51,159, 17,131, 20
        };

        /* Fill in the temp buf */
        for (i = 0; i < 16; i++)
                md2->buf[i+32] = md2->buf[i+16] ^ md2->buf[i];

        /* Update the checksum in the last 16 bytes of the buf */
        t = md2->buf[63];
        for (i = 0; i < 16; i++)
            t = md2->buf[48+i] ^= permutation[md2->buf[16+i] ^ t];

        /* 18 passes of encryption over the first 48 bytes of the buf */
        t = 0;
        for (i = 0; i < 18; i++) {
                for (j = 0; j < 48; j++)
                        t = md2->buf[j] ^= permutation[t];
                t += i;
        }
}

void
xMD2Init(struct xMD2Context *md2)
{
        memset(md2->buf, 0, sizeof(md2->buf));
        md2->len = 0;
}

/*
 * Take an arbitrary number of bytes, break it into 16-byte chunks,
 * and apply the core transformation routine.  The transformation is
 * done eagerly, so there is always at least one byte of free space in
 * the input buffer.
 */
void
xMD2Update(struct xMD2Context *md2, unsigned char const *buf, unsigned len)
{
        unsigned space = 16 - md2->len; /* Amount of free space */

        /* No need to transform at all */
        if (len < space) {
                memcpy(md2->buf+16+md2->len, buf, len);
                md2->len += len;
                return;
        }

        /* Copy in and transform the leading partial block (1 <= len <= 16) */
        memcpy(md2->buf+16+md2->len, buf, space);
        len -= space;
        buf += space;
        md2Transform(md2);

        /* Copy in and transform intermediate partial blocks */
        while (len >= 16) {
                memcpy(md2->buf+16, buf, 16);
                buf += 16;
                len -= 16;
                md2Transform(md2);
        }

        /* Copy in the trailing partial block (0 <= len <= 15) */
        memcpy(md2->buf+16, buf, len);
        md2->len = len;
}

/*
 * Wrap up an MD2 computation.  Pad the last block and then append a
 * running checksum.
 */
void
xMD2Final(struct xMD2Context *md2, unsigned char hash[16])
{
        unsigned space = 16 - md2->len; /* Amount of free space */

        /* Pad with "space" bytes of value "space" */
        memset(md2->buf+16+md2->len, space, space);
        md2Transform(md2);

        /* Append checksum */
        memcpy(md2->buf+16, md2->buf+48, 16);
        md2Transform(md2);
        /*
         * The transform redundantly updates the checksum, but it's not worth
         * optimizing away.
         */

        /* Copy hash out */
        memcpy(hash, md2->buf, 16);

        /* Burn the md2 context */
        memset(md2, 0, sizeof(*md2));
}

#ifdef TEST
#include <stdio.h>

void
printhash(char const *prompt, unsigned char const hash[16])
{
        int i;

        fputs(prompt, stdout);
        for (i = 0; i < 16; i++)
                printf(" %02X", hash[i]);
        putchar('\n');
}

int
main(void)
{
        struct xMD2Context md2;
        unsigned char hash[16];
        unsigned i;

        md2Init(&md2);
        md2Final(&md2, hash);
        printhash("Hash of \"\" is ", hash);

        md2Init(&md2);
        md2Update(&md2, (unsigned char const *)"a", 1);
        md2Final(&md2, hash);
        printhash("Hash of \"a\" is ", hash);

        md2Init(&md2);
        md2Update(&md2, (unsigned char const *)"abc", 3);
        md2Final(&md2, hash);
        printhash("Hash of \"abc\" is ", hash);

        md2Init(&md2);
        for (i = 0; i < 1000/5; i++)
                md2Update(&md2, (unsigned char const *)"aaaaa", 5);
        md2Final(&md2, hash);
        printhash("Hash of 1000 a's is ", hash);

        md2Init(&md2);
        for (i = 0; i < 1000/25; i++)
                md2Update(&md2, (unsigned char const *)"aaaaaaaaaaaaaaaaaaaaaaaaa", 25);
        md2Final(&md2, hash);
        printhash("Hash of 1000 a's is ", hash);

        md2Init(&md2);
        for (i = 0; i < 1000/125; i++)
                md2Update(&md2, (unsigned char const *)"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 125);
        md2Final(&md2, hash);
        printhash("Hash of 1000 a's is ", hash);

        return 0;
}

#endif
