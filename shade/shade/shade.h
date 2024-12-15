
/* SMALL_ENDIAN
 *
 * Define this if the most significant byte of a
 * a word is in the high address.
 */
/* #define SMALL_ENDIAN /**/

/* PRIME_FILE
 * shade needs to read from a list of prime polynomials.
 * this file holds all 4080 degree 16 primes.
 */
#define PRIME_FILE "S:poly_prime"

typedef unsigned char  UBYTE;   /* ==  8 bits */
typedef unsigned short UWORD;   /* == 16 bits */
typedef unsigned long  ULONG;   /* == 32 bits */


/* degree 32 polynomial -- DO NOT CHANGE */
#define CRC_POLY 0x04C11DB7


/* The map_t structure is used to hold some precalculated values.
 *
 * The structure is entirely defined by the n, k and b fields.
 * The others are precalculated data filled in by map_init().
 */
typedef struct {
    int n; /* number of input sections */
    int k; /* number of output shadows */
    UWORD *b; /* base of n+k polynomials */
    UWORD *c; /* c[i] * B[i] == 1 (mod b[i]) */
    UWORD *d; /* d[i*k+j] == B[i] (mod b[j]) */
    void *data; /* memory area to hold b, c and d */
} map_t;

#ifndef SMALL_ENDIAN
#define MSB 0
#define LSB 1
#define fix_endian(x) (x)
#else
#define MSB 1
#define LSB 0
extern UWORD fix_endian( UWORD);
#endif

extern UBYTE mul4_tab[];
extern UBYTE shft_tab[];
extern UBYTE inv8_tab[];
extern UBYTE inb8_tab[];
extern UWORD lpn;
extern UWORD prime[];

extern UWORD mul8(UBYTE,UBYTE);
extern ULONG mul16(UWORD,UWORD);
extern UWORD mod16x(ULONG,UWORD);
extern UWORD mod16(ULONG,UWORD);
#ifndef CHECK
extern UWORD inv16(UWORD);
extern UWORD div16x(ULONG,UWORD);
extern UWORD div16(ULONG,UWORD);
extern UWORD gcd(UWORD,UWORD,UWORD*,UWORD*);
extern UWORD pinv(UWORD,UWORD);
#else
#define inv16(x)     c_inv16(x)
#define div16x(x,y)  c_div16x(x,y)
#define div16(x,y)   c_div16(x,y)
#define gcd(a,b,c,d) c_gcd(a,b,c,d)
#define pinv(x,y)    c_pinv(x,y)
extern UWORD c_inv16(UWORD);
extern UWORD c_div16x(ULONG,UWORD);
extern UWORD c_div16(ULONG,UWORD);
extern UWORD c_gcd(UWORD,UWORD,UWORD*,UWORD*);
extern UWORD c_pinv(UWORD,UWORD);
#endif

extern int  map_init(map_t*,int,int,UWORD*);
extern void map_free(map_t*);
extern void map_poly(map_t*,UWORD**,int,UWORD);
extern void map_block(map_t*,UWORD**,int,UWORD*,int);

extern void  uue_block(FILE*,UBYTE*,ULONG);
extern int   uud_pending();
extern ULONG uud_block(UBYTE*,ULONG,FILE*);
#define uu_size(n) (((((n)+2)/3)<<2)+2)

