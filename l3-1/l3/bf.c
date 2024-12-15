/*************************************************************************/
/* File: bf.c
   Blowfish cipher by Bruce Schneier,
   Code by Bryan Olson, based partly on Schneier's.
   Modified for L3
*/

#include <stdio.h>
#include <time.h>
#include "bf.h"
#include "bfinit.h"
/* sub(s,n) allows us to subsript s by byte offsets, which
   allows us to avoid a subscript scaling.
*/
#define sub(s,n) *((IntU32 *)((IntP)s+(n)))

/* Below is one BlowFish round including the F function 
*/
#define round(l,r,n) \
       l ^= P[n]; \
       r ^= ( sub(S[0],l>>22 & 0x3fc) + sub(S[1],l>>14 & 0x3fc) \
	      ^ sub(S[2],l>>6 & 0x3fc) ) +S[3][l & 0xff] 



/* This function requires the block to be two 32 bit integers, in 
whatever endian form the machine uses.  On little endian machines 
use crypt_8bytes() on user data.  make_bfkey should call crypt_block
on either endian machine.  Pass direction 0 to encrypt, 1 to decrypt.
*/
void crypt_block(IntU32 block[2], BFkey_type *bfkey, short direction ) 
{
  register IntU32 left, right, 
                  (*S)[256], 
                  *P ; 
                  
  left = block[0] ; right = block[1] ; 

  S = bfkey->sbox ; 
  P = bfkey->p[direction] ;

  round( left, right,  0 ) ;   round( right, left,  1 ) ;  
  round( left, right,  2 ) ;   round( right, left,  3 ) ;
  round( left, right,  4 ) ;   round( right, left,  5 ) ;
  round( left, right,  6 ) ;   round( right, left,  7 ) ;
  round( left, right,  8 ) ;   round( right, left,  9 ) ;
  round( left, right, 10 ) ;   round( right, left, 11 ) ;
  round( left, right, 12 ) ;   round( right, left, 13 ) ;
  round( left, right, 14 ) ;   round( right, left, 15 ) ;

  left = left ^ P[NROUNDS] ;
  right = right ^ P[NROUNDS+1] ;
  block[0] = right ;
  block[1] = left ;
}

/* The following should be allignment and endian independent.
   I have not tested it on a little-endian machine.
   It takes the input block from source, and puts the output
   in dest.  They can be the same.  It takes the same direction
   parameter as crypt_block().
*/
void crypt_8bytes(IntU8 *source, IntU8 *dest,
		 BFkey_type *bfkey, short direction)
{
   IntU32  block[2] ;

   block[0] = source[3] | source[2]<<8 | source[1]<<16 | source[0]<<24 ;
   block[1] = source[7] | source[6]<<8 | source[5]<<16 | source[4]<<24 ;
#ifdef TEST
   fprintf(stderr, "input to crypt_block %08x:%08x\n", block[0], block[1]);
#endif

   crypt_block( block, bfkey, direction ) ;
#ifdef TEST
   fprintf(stderr, "output from crypt_block %08x:%08x\n", block[0], block[1]);
#endif

   dest[0]= block[0]>>24 ;
   dest[1]= block[0]>>16 & 0xff ;
   dest[2]= block[0]>>8  & 0xff ;
   dest[3]= block[0]     & 0xff ;
   dest[4]= block[1]>>24 ;
   dest[5]= block[1]>>16 & 0xff ;
   dest[6]= block[1]>> 8 & 0xff ;
   dest[7]= block[1]     & 0xff ;
}

/* make_bfkey() takes the address of the key data as a char*,
   and the length of the key in bytes.  It generates and returns
   a pointer to an object of BFkey_type, which can be passed
   to the crypt functions.  It does some simple testing of the
   init data and crypt routine, and returns 0 on error.
*/
BFkey_type* make_bfkey(char key_string[], short keylength)
{
   short       i, j, k ;
   IntU32      data,
               dspace[2],
               checksum=0 ;

   BFkey_type     *bfkey ;

   bfkey = (BFkey_type *) malloc( sizeof( BFkey_type )) ;
   if( bfkey==0 ) return 0 ;

   /* Copy constant initial data to P vector */
   for( i=0 ; i<NROUNDS+2 ; ++i )
     {
       bfkey->p[0][i] = p_init[i] ;
       bfkey->p[1][NROUNDS+1-i] = p_init[i] ;
       checksum = (checksum<<1 | checksum>>31)+p_init[i] ;
     }

   /* Copy constant initial data to sboxes */
   for( i=0 ; i<4 ; ++i )
     for( j=0 ; j<256 ; ++j )
       {
	 bfkey->sbox[i][j] = s_init[i][j] ;
         checksum = ((checksum*13)<<11 | (checksum*13)>>21)
	   + s_init[i][j] ;	 
       }

   /* Test init data. */
   if( checksum != 0x55861a61 )
     {
       printf("**ERROR bad init data.\n" ) ;
       free( bfkey ) ;
       return 0 ;
     }

   dspace[0] = 0 ;
   dspace[1] = 0 ;

   /* Test the crypt_block() routine. */
   for( i=0 ; i<10 ; ++i )
     crypt_block( dspace, bfkey, 0 ) ;
   checksum = dspace[0] ;
   for( i=0 ; i<10 ; ++i )
     crypt_block( dspace, bfkey, 1 ) ;
   if( (checksum!=0xaafe4ebd) || dspace[0] || dspace[1] )
     {
       printf("***ERROR in crypt_block() routine.\n") ;
       free( bfkey ) ;
       return 0 ;
     }

   /* Xor key string into encryption key vector */
   j = 0 ;
   for (i=0 ; i<NROUNDS+2 ; ++i) 
     {
       data = 0 ;
       for (k=0 ; k<4 ; ++k ) 
	   data = (data << 8) | key_string[j++ % keylength];
       bfkey->p[0][i] ^= data;
     }

   for (i = 0 ; i<NROUNDS+2 ; i+=2) 
     {
       crypt_block( dspace, bfkey, 0 ) ;
       bfkey->p[0][i] = dspace[0] ;
       bfkey->p[1][NROUNDS+1-i] = dspace[0] ;
       bfkey->p[0][i+1] = dspace[1] ;
       bfkey->p[1][NROUNDS-i] = dspace[1] ;
   }
   
   for ( i=0 ; i<4 ; ++i )
     for ( j=0 ; j<256 ; j+=2 ) 
       {
	 crypt_block( dspace, bfkey, 0 ) ;
	 bfkey->sbox[i][j] = dspace[0] ;
	 bfkey->sbox[i][j+1] = dspace[1] ;
       }

   return bfkey ;
}

/* Zero-out the key and free the storage. */
void destroy_bfkey( BFkey_type *bfkey )
{
  short i, j ;
  for( i=0 ; i<NROUNDS+2 ; ++i )
    {
      bfkey->p[1][i] = 0 ;
      bfkey->p[2][i] = 0 ;
    }
  for( i=0 ; i<4 ; ++i )
    for( j=0 ; j<256 ; ++j )
      bfkey->sbox[i][j] = 0 ;
  free( bfkey ) ;
}

#ifdef TEST
main()
{
	BFkey_type *bfkey;
	int i;
	unsigned char key[] = "this is a longer key";
	unsigned char c[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
	bfkey = make_bfkey(key,(short)strlen(key));
	crypt_8bytes(c,c,bfkey,(short)1);
	for (i=0; i<8; i++)
		fprintf(stderr, "%02x\n", c[i]);
}
#endif
