
typedef unsigned char bf_cblock[8];

/* Define IntU32 to be an unsigned in 32 bits long */
typedef unsigned int IntU32 ;
typedef unsigned char IntU8 ;

/* Define IntP to be an integer which
   is the same size as a pointer. */
typedef unsigned long IntP ;

#define NROUNDS         16
/* blowfish internal key size = 4 * ( ( 2 * ( NROUNDS + 2 ) ) + ( 4 * 256 ) ) */
#define BF_KEY_SIZE = 4240;

typedef struct
{
  IntU32 p[2][NROUNDS+2],
         sbox[4][256];
} BFkey_type; 

int blowfish_make_bfkey( unsigned char * key_string, int keylength, BFkey_type * bfkey );
void blowfish_crypt( bf_cblock source, bf_cblock dest, BFkey_type * bfkey, short direction );

