/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
/*
 * Magic number defining supported algorithms
 */
#define CRYPTALG_UNKNWON   0
#define CRYPTALG_DES       1 /* DES-CBC block cipher (64/56 bit key)    */
#define CRYPTALG_RC2_40    2 /* RC2-CBC block cipher (40 bit key)       */
#define CRYPTALG_RC4_40    3 /* RC4' stream cipher (40 bit key)         */
#define CRYPTALG_RC4_128   4 /* RC4' stream cipher (128 bit key)        */
#define CRYPTALG_3DES_2    5 /* 2 key Triple-DES-CBC block cipher       */
#define CRYPTALG_3DES_3    6 /* 3 key Triple-DES-CBC block cipher       */
#define CRYPTALG_IDEA      7 /* IDEA-CBC block cipher (128 bit key)     */
#define CRYPTALG_SIMPLE   10 /* Sun Simple Crypt (64 bit key)           */
#define CRYPTALG_MAX      11 /* Used to query maximum lengths           */
#define CRYPTALG_MAXBLOCK 12 /* Query maximum lengths for block ciphers */

#define MEMEOR(dst,src,len) { int _i = len; while (_i--) dst[_i] ^= src[_i]; }
#define MAXBLOCKLEN 16  /* Block len is assumed to be <= 128 bit :-) */

/*
 * Public interface to algorithms
 */
extern int crypt_init(void);  /* Initialize all crypt algorithms */
extern int crypt_exit(void);  /* Cleanup all crypt algorithms */

extern int crypt_encryptstatelen(int alg); /* Get algo encrypt state len   */
extern int crypt_decryptstatelen(int alg); /* Get algo encrypt state len   */
extern int crypt_blocklen(int alg);        /* Get algorithm data block len */
extern int crypt_keylen(int alg);          /* Get algorithm key len        */
extern int crypt_MIlen(int alg);           /* Get algorithm MI len         */

extern int crypt_encrypt(int alg, u_char *state, u_char *key,
			 u_char *oldMI, u_char *newMI, struct memblk *data);
extern int crypt_decrypt(int alg, u_char *state, u_char *key, u_char *MI,
                         struct memblk *data);

/*
 * Internal definition of an algorithm
 */
struct crypt_algorithm
{
  int (*init)(void);             /* Initialization */
  int (*exit)(void);             /* Cleanup */
  int (*encryptstatelen)(void);  /* Len of encrypt state */
  int (*decryptstatelen)(void);  /* Len of decrypt state */
  int (*blocklen)(void);         /* Len of data block for cipher (stream: 1) */
  int (*keylen)(void);           /* Len of key */
  int (*MIlen)(void);            /* Len of Initialization Vector IV */
  int (*setencryptkey)(u_char *key, u_char *state);
  int (*setdecryptkey)(u_char *key, u_char *state);
  int (*encrypt)(u_char *data, int len, u_char *state);
  int (*decrypt)(u_char *data, int len, u_char *state);
  int (*streamfastforward)(u_char *state, u_int amount);
};
