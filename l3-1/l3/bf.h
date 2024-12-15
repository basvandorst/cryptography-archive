typedef unsigned int IntU32 ;
typedef unsigned char IntU8 ;
#define NROUNDS         16
#define MAXKEYBYTES     56


/* Here's the object type that will hold all the
   key and sbox data */
typedef struct
{
  IntU32 p[2][NROUNDS+2],
         sbox[4][256] ;
} BFkey_type ; 


typedef unsigned long IntP ;

void crypt_8bytes(IntU8 *source, IntU8 *dest, BFkey_type *bfkey, short direction);
BFkey_type* make_bfkey(char key_string[], short keylength);
void destroy_bfkey(BFkey_type *bfkey);
