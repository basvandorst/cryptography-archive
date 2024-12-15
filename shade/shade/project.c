#include <stdio.h>
#include "shade.h"

/*
 * It is based on the chinese remainder theorem for arithmetic
 * on polynomials with coeficients modulo 2.
 *
 * Let b[i] be an array of n polynomials of degree 16 that are
 * relatively prime to each other.  Let x[i] be n polynomials
 * where x[i] is a residue modulo b[i] for i=0...n-1.
 *
 * Define B as the product of the b[i].  Then there is a unique
 * residue X modulo B such that X = x[i] mod b[i] for
 * i=0...n-1.
 *
 * Let, B[i] = B/b[i], and X[i] such that x[i] = X[i]*B[i] mod b[i]
 * then X is the sum of the X[i]*B[i] mod B.
 *
 * This works because each X[i]*B[i] mod b[k] is x[i] when i==k
 * and 0 otherwise.  Therefore X mod b[i] is x[i] for all i.
 *
 * Now suppose I choose some large polynomial X and n mutually
 * prime small polynomials b[i].  I now compute x[i] = X mod
 * b[i] for i=0...n-1. Let B be the product of the b[i] like
 * before.  From the b[i] and x[i] I can reconstruct X mod B
 * using the chinese remainder theorem. If B has a larger
 * degree than X, I have recovered X.
 *
 * If you take a 4-word quantity, you could take five 1-word
 * residues such that the knowledge of any 4 allows you to
 * reconstruct the original. But instead, we will take the
 * 4-word quantity and interpret it as the 4 first residues and
 * construct the implied 4-word composite.  This composite is
 * used to calculate the fifth residue.  Now, if any one word of
 * the original four is lost, the artificial fifth can
 * be used to recover it.
 *
 * Thus we have two types of parts: sections of the original
 * and extra shadows.  If enough parts are available, the composite
 * is reconstructed and the missing sections of the original
 * are recovered.
 *
 * Multiplication and division of large polynomials require a
 * lot of computations.  Fortunately, the degree of the
 * polynomials can always be kept small (<32) using a few
 * tricks.
 *
 * We don't interpret the 45K block as a single large
 * polynomial but as a series of polynomials of degree < 16
 * (16 bits).  Therefore, the sections of the original do not
 * describe a single large composite, but a series of smaller
 * (but still possibly large) composites.
 *
 * Finally, we don't need to compute the composite at all, we
 * only need its redidue modulo polynomials of degree 16, as
 * many as there are shadows to compute.
 *
 * To recap:
 *
 *      1) Split the large file in blocks of equal length.
 *         Pad the last block with zeros as necessary.
 *         The block length (in bytes) has to be a multiple of 2.
 *
 *      2) Using the first 2 bytes from each section, compute
 *         the implied composite.
 *
 *      3) Compute as many extra shadows from the composite as
 *         needed.
 *
 *      4) Repeat steps 2) and 3) until the sections are exhausted.
 *
 * To compute the shadows, we never actually compute the composite.
 * Using the above notation, the composite X is the sum of the
 * X[i]*B[i] mod B.  The shadow s is X mod p, where p is some
 * polynomial of degree 16. Therefore, s is the sum of the
 * X[i]*B[i] mod p.  X[i] itself is (x[i]/B[i]) mod b[i].
 * So we precompute (1/B[i]) mod b[i] and B[i] mod p and store
 * these values.  The shadow is then
 *    ( ( (x[i]*(1/B[i])) mod b[i] ) * B[i] ) mod p
 *
 * In map_t:
 *      n                 : number of sections
 *      k                 : number of shadows
 *      b[0...n-1]        : b[i] corresponding to the sections
 *      b[n...n+k-1]      : b[i] corresponding to the shadows
 *      c[0...n-1]        : (1/B[i]) mod b[i]
 *      d[0...n-1,0..k-1] : B[i] mod b[j]
 *
 * Recovering missing sections and computing shadows are just two
 * sides of the same coin.
 *
 * To recover missing sections using the shadows, permute b[0..n+k-1]
 * so that available sections and shadows are all in the first n
 * elements and the missing sections are in b[n..n+k-1].
 * Then just do as if the missing sections were the shadows you
 * are looking for.  Reverse the permutation of sections and shadows
 * to bring the recovered sections at their proper places.
 *
 */

#ifdef SMALL_ENDIAN
UWORD fix_endian( UWORD x)
{
    return (x<<8) | (x>>8);
}
#endif

#define data_size(n,k) ((k)+2*(n)+(k)*(n))
int  map_init( map_t *map, int n, int k, UWORD *b)
{
    int i, j, l; ULONG t;

    map->n = n;
    map->k = k;
    /* allocate storage space */
    map->data = (UWORD*) malloc( sizeof(UWORD)*data_size(n,k));
    if (!map->data) return 0;
    map->b = (UWORD*) map->data;
    map->c = map->b + n + k;
    map->d = map->c + n;

    /* copy the base */
    memcpy( map->b, b, sizeof(UWORD)*(n+k));

    /* compute map->c and map->d */
    for (i=0;i<n;i++) {

        /* compute map->c[i] */
        map->c[i] = 1;
        for (j=0;j<n;j++) if (j!=i) {
            t = mul16( map->c[i], b[j]) ^ ((ULONG)map->c[i]<<16);
            map->c[i] = t ^ mul16( div16x( t, b[i]), b[i]);
        }

        map->c[i] = pinv( map->c[i], b[i]);
        /* compute map->d[i*k...(i+1)*k-1] */
        for (j=0;j<k;j++) {

            /* compute map->d[i*k+j] */
            map->d[i*k+j] = 1;
            for (l=0;l<n;l++) if (l!=i) {
                t = mul16( map->d[i*k+j], b[l]) ^ ((ULONG)map->d[i*k+j]<<16);
                map->d[i*k+j] = t ^ mul16( div16x( t, b[n+j]), b[n+j]);
            }
        }
    }
    return 1;
}

void map_free( map_t *map)
{
    if (map->data) free(map->data);
    map->data = 0;
    map->n = map->k = 0;
}

/*
 * We construct the shadows in k buffers. These buffers are
 * first cleared.  Each part is then then fed to map_poly().
 * When all n parts have been fed, the shadows are complete
 * and can be read from the buffers.  shadows points to an
 * array containing the pointers to the k buffers. i is the
 * part number of x.
 */

void map_poly( map_t *map, UWORD **shadows, int i, UWORD x)
{
    int j; ULONG t; UWORD xx;
    t = mul16( x, map->c[i]);
    xx = t ^ mul16( div16x( t, map->b[i]), map->b[i]);
    for (j=0;j<map->k;j++) {
        t = mul16( xx, map->d[i*map->k+j]);
        t ^= mul16( div16x( t, map->b[map->n+j]), map->b[map->n+j]);
        shadows[j][0] ^= fix_endian(t);
    }
}

void map_block( map_t *map, UWORD **shadows, int i, UWORD *x, int len)
{
    int j, l=(len>>1);
    while (l--) {
        map_poly( map, shadows, i, fix_endian(*x++));
        for (j=0;j<map->k;j++) shadows[j] += 1;
    }
    for (j=0;j<map->k;j++) shadows[j] -= (len>>1);
}



