/* Written by Dr. S N Henson and released into the public domain */


#ifndef EVP_MAX_MD_BLOCK
#define EVP_MAX_MD_BLOCK 64
#endif
#ifndef EVP_MD_blocksize
#define EVP_MD_blocksize(a)	64
#endif

/* HMAC context structure */

typedef struct {
unsigned char key[EVP_MAX_MD_BLOCK];	/* Storage for HMAC key */
int keylen;		/* Length of key */
EVP_MD_CTX	ctx;	/* Standard digest context */
EVP_MD		*type;	/* Type of digest */
} HMAC_CTX;

/* hmac.c */
void HMAC_Init(HMAC_CTX *hctx, unsigned char *key, int keylen, EVP_MD *md_type);
void HMAC_Update(HMAC_CTX *hctx, unsigned char *data, unsigned int len);
void HMAC_Final(HMAC_CTX *hctx, unsigned char *md, unsigned int *len);
