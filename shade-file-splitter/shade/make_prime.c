#include <stdio.h>
#include "shade.h"

/* lpn: the number of primes in prime[]
 * prime[]: lpn prime poly of degree 16.
 */
UWORD lpn; UWORD prime[5000];

/* We start by forming a series of large poly containing the product
 * of all 16-bit primes.  The first poly contains as many primes
 * as will fit in 1 word, the second poly contains the next few primes
 * to form a 2 word product, and the series goes on with products
 * of sizes 4, 8, 16, etc.  The last product in the series may
 * be smaller.
 *
 * To check wether a poly is prime, we only have to check that
 * it is relatively prime to every poly in this series.
 *
 * buff holds the series,
 * b is a pointer to the last poly of the series.
 * n is the size of the last poly in the series
 * s is the number of free bits in this last poly
 * x is the poly whose primality is being tested
 * i is the size in bits of x
 * c, j, k, r and rr are loop indices and temporary variables.
 */
int init_prime()
{
    UWORD *buff, *b, *c, x;
    UWORD n, s, i, j, k;
    ULONG r, rr;
    FILE *fd;

    fd = fopen( PRIME_FILE, "w");
    if (!fd) {
        fprintf( stderr, "%s: can't open for output.\n", PRIME_FILE);
        exit( 10);
    }

    /* allocate a buffer and initialize it */
    b = buff = (UWORD*) malloc( 5000 * sizeof(UWORD));
    n = 1;
    *b = 2;
    s = 14;

    /* for each size for x */
    for (i=1;i<16;i++) {
        /* for each x of that size */
        for (x=1<<i;x!=(UWORD)(1<<(i+1));x++) {
            if (!(x&1)) continue;
            if (!((x+1)&0xFF)) printf( "%d\n", 512 - ((x+1)>>8));
            /* is x prime relative to the series */
            for (rr=1,c=buff,k=1;c<b;c+=k,k<<=1) {
                for (r=0,j=k;j--;) {
                    r = (r<<16) | c[j];
                    r ^= mul16( div16( r, x), x);
                }
                rr = x;
                while (r) {
                    ULONG t = rr ^ mul16( div16( rr, r), r);
                    rr = r; r = t;
                }
                if (rr != 1) break;
            }
            if (rr != 1) continue;
            /* is x prime relative to the last poly of the series */
            for (r=0,j=n;j--;) {
                r = (r<<16) | b[j];
                r ^= mul16( div16( r, x), x);
            }
            rr = x;
            while (r) {
                ULONG t = rr ^ mul16( div16( rr, r), r);
                rr = r; r = t;
            }
            if (rr != 1) continue;
            /* x is prime */
            if (s < i) {
                /* no more room, add a new poly at the end of the series */
                s = 32 * n - 1;
                b += n;
                n = 1;
                *b = 1;
            }
            /* multiply x into the last poly of the series */
            s -= i;
            for (r=0,j=0;j<n;j++) {
                r ^= mul16( b[j], x);
                b[j] = r;
                r >>= 16;
            }
            /* extend the last poly of the series */
            if (r) {
                b[n] = r;
                n++;
            }
        }
    }

    /* for each poly x of degree 16 */
    for (x=1;x;x++) {
        if (!(x&1)) continue;
        if (!((x+1)&0xFF)) printf( "%d\n", 256 - ((x+1)>>8));
        /* is x relatively prime to the series? */
        for (c=buff,k=1;c<b;c+=k,k*=2) {
            for (r=0,j=k;j--;) {
                r = (r<<16) | c[j];
                rr = div16x( r, x);
                r ^= mul16( rr, x) ^ (rr << 16);
            }
            if (r==1) continue;
            if (!r) break;
            rr = 0x10000 ^ x ^ mul16( div16( (1<<16)|x, r), r);
            while (rr) {
                ULONG t = r ^ mul16( div16( r, rr), rr);
                r = rr; rr = t;
            }
            if (r != 1) break;
        }
        if (r != 1) continue;
        /* is x relatively prime to the last poly of the series */
        for (r=0,j=n;j--;) {
            r = (r<<16) | b[j];
            rr = div16x( r, x);
            r ^= mul16( rr, x) ^ (rr << 16);
        }
        if (!r) continue;
        rr = (r==1) ? 0 : (0x10000 ^ x ^ mul16( div16( (1<<16)|x, r), r));
        while (rr) {
            ULONG t = r ^ mul16( div16( r, rr), rr);
            r = rr; rr = t;
        }
        if (r != 1) continue;
        /* x is prime */
        fprintf( fd, "0x1%04X\n", x);
        prime[lpn] = x;
        lpn += 1;
    }
    free( buff);
    fclose( fd);
    return 1;
}


main()
{
    puts( "Be very patient.  This will take a few hours,");
    puts( "maybe even days on slower processors.");
    init_prime();
    return 0;
}

