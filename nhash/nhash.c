/* NHASH.C - Implementation of N-Hash.
Version of 93.02.15.
*/

#include <stdio.h>

static unsigned char BN = { 2 } ;/* Block number */
static unsigned char MESSAGE[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef } ;

static unsigned char INITIAL[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } ;

static unsigned char INITIAL2[16] = {
    0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52,
    0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25 } ;


static unsigned char OUT[16] ;

void main()
{
    register i, loop ;
    void HASH128( unsigned char BN, unsigned char *MSGIN,
            unsigned char *INIT, unsigned char *OUT ) ;

    printf( "\n\nInitial Value (No.1) \n" ) ;
    for ( i = 0 ; i < 16 ; i++ )
     printf( " %02X", INITIAL[i] ) ;
    printf( "\n\nMessage" ) ;
    for ( loop = 0 ; loop < BN ; loop++ )
    {
     printf( "\n" ) ;
     for ( i = 0 ; i < 16 ; i++ )
         printf( " %02X", MESSAGE[ 16 * loop + i ] ) ;
    }

    HASH128( BN, MESSAGE, INITIAL, OUT ) ;
    printf( "\n\nOutput of N-Hash\n" ) ;
    for ( i = 0 ; i < 16 ; i++ )
     printf( " %02X", OUT[i] ) ;


    printf( "\n\n\nInitial Value (No.2) \n" ) ;
    for ( i = 0 ; i < 16 ; i++ )
     INITIAL[i] = INITIAL2[i] ;

    for ( i = 0 ; i < 16 ; i++ )
     printf( " %02X", INITIAL[i] ) ;
    printf( "\n\nMessage" ) ;
    for ( loop = 0 ; loop < BN ; loop++ )
    {
     printf( "\n" ) ;
     for ( i = 0 ; i < 16 ; i++ )
         printf( " %02X", MESSAGE[ 16 * loop + i ] ) ;
    }

    HASH128( BN, MESSAGE, INITIAL, OUT ) ;
    printf( "\n\nOutput of N-Hash\n" ) ;
    for ( i = 0 ; i < 16 ; i++ )
     printf( " %02X", OUT[i] ) ;
}

void HASH128( unsigned char BN, unsigned char *MSGIN, unsigned char *INIT,
     unsigned char *OUT )
{
    register i, loop ;
    unsigned char X2[16], X1[16], Y[16] ;
    void setkey( unsigned char *key ) ;
    void encrypt( unsigned char *data, unsigned char *output ) ;

    for ( i = 0 ; i < 16 ; i++ )
     X2[i] = INIT[i] ;
    for ( loop = 0 ; loop < BN ; loop++ )
    {
     for ( i = 0 ; i < 16 ; i++ )
         X1[i] = MSGIN[16*loop + i ] ;
     printf( "\n\nH%X      ", loop ) ;
     for ( i = 0 ; i < 16 ; i++ )
         printf( " %02X", X2[i] ) ;
     printf( "\nM%x      ", loop + 1 ) ;
     for ( i = 0 ; i < 16 ; i++ )
         printf( " %02X", X1[i] ) ;

     setkey( X2 ) ;
     
     encrypt( X1, Y ) ;

     printf( "\nH%x      ", loop + 1 ) ;
     for ( i = 0 ; i < 16 ; i++ )
         printf( " %02X", Y[i] ) ;
     for ( i = 0 ; i < 16 ; i++ )
         X2[i] = Y[i] ;
    }
    for ( i = 0 ; i < 16 ; i++ )
     OUT[i] = Y[i] ;
}

static unsigned char X1[16], X2[16] ;

void setkey( unsigned char *key )
{
    register int i ;

    for ( i = 0 ; i < 16 ; i++ )
     X2[i] = key[i] ;
}

void encrypt( unsigned char *data, unsigned char *output )
{
    unsigned char block[16], work[4], V ;
    register int loop, i ;
    int sbox( unsigned char data ) ;

    for ( i = 0 ; i < 16 ; i++ )
     X1[i] = data[i] ;

    printf( "\nInput X1 of IP function\n" ) ;
    for ( i = 0 ; i < 16 ; i++ )
     printf( " %02X", X1[i] ) ;
    printf( "\nInput X2 of IP function\n" ) ;
    for ( i = 0 ; i < 16 ; i++ )
     printf( " %02X", X2[i] ) ;
    for ( i = 0 ; i < 8 ; i++ )
     block[i] = X2[i] ^ X1[i+8] ^ 0xaa ;
    for ( i = 0 ; i < 8 ; i++ )
     block[i+8] = X2[i+8] ^ X1[i] ^ 0xaa ;
    V = 0 ;
    for ( loop = 0 ; loop < 8 ; loop++ )
    {
     for ( i = 0 ; i < 4 ; i++ )
         work[i] = block[i] ^ X1[i] ;

     /* P1 */

     ++V ;
     work[3] ^= V ;
     work[1] ^= work[0] ;
     work[2] ^= work[3] ;
     work[1] = sbox( work[1] + work[2] + 1 ) ;
     work[2] = sbox( work[2] + work[1] ) ;
     work[0] = sbox( work[0] + work[1] ) ;
     work[3] = sbox( work[3] + work[2] + 1 ) ;

     for ( i = 0 ; i < 4 ; i++ )
         work[i] ^= block[i+4] ;
     for ( i = 0 ; i < 4 ; i++ )
         block[i+12] ^= work[i] ;
     for ( i = 0 ; i < 4 ; i++ )
         work[i] ^= X1[i+4] ;

     /* P2 */

     ++V ;
     work[3] ^= V ;
     work[1] ^= work[0] ;
     work[2] ^= work[3] ;
     work[1] = sbox( work[1] + work[2] + 1 ) ;
     work[2] = sbox( work[2] + work[1] ) ;
     work[0] = sbox( work[0] + work[1] ) ;
     work[3] = sbox( work[3] + work[2] + 1 ) ;

     for ( i = 0 ; i < 4 ; i++ )
         block[i+8] ^= work[i] ^ block[i] ;

     /******************************/

     for ( i = 0 ; i < 4 ; i++ )
         work[i] = block[i+8] ^ X1[i+8] ;

     /* P3 */

     ++V ;
     work[3] ^= V ;
     work[1] ^= work[0] ;
     work[2] ^= work[3] ;
     work[1] = sbox( work[1] + work[2] + 1 ) ;
     work[2] = sbox( work[2] + work[1] ) ;
     work[0] = sbox( work[0] + work[1] ) ;
     work[3] = sbox( work[3] + work[2] + 1 ) ;

     for ( i = 0 ; i < 4 ; i++ )
         work[i] ^= block[i+12] ;
     for ( i = 0 ; i < 4 ; i++ )
         block[i+4] ^= work[i] ;
     for ( i = 0 ; i < 4 ; i++ )
         work[i] ^= X1[i+12] ;

     /* P4 */

     ++V ;
     work[3] ^= V ;
     work[1] ^= work[0] ;
     work[2] ^= work[3] ;
     work[1] = sbox( work[1] + work[2] + 1 ) ;
     work[2] = sbox( work[2] + work[1] ) ;
     work[0] = sbox( work[0] + work[1] ) ;
     work[3] = sbox( work[3] + work[2] + 1 ) ;

     for ( i = 0 ; i < 4 ; i++ )
         block[i] ^= work[i] ^ block[i+8] ;
    }

    for ( i = 0 ; i < 16 ; i++ )
     output[i] = block[i] ^ X1[i] ^ X2[i] ;

    printf( "\nOutput Y of IP function\n" ) ;
    for ( i = 0 ; i < 16 ; i++ )
     printf( " %02X", output[i] ) ;
}

int sbox( unsigned char data )
{
    union {
     unsigned int DATA ;
     struct {
         unsigned char low ;
         unsigned char high ;
     } reg ;
    } s ;

    s.reg.high = 0 ;
    s.reg.low = data ;
    s.DATA <<= 2 ;
    return ( s.reg.high | s.reg.low ) ;
}
