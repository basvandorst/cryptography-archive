/*               -------------- des.h ---------------- 
        Header file for Data Encryption Standard algorithms
*/

/* -------------- prototypes ------------------- */

void ascii_adjust_key(char *output);    /* move parity bit to right end */
void test_adjust_key(char *output);     /* compatibility mode           */
void schedule(char *password,char *tbl); /* compute 16 rounds of Keys */
void encrypt_block(void);               /* encrypt 8 bytes using DES */
void decrypt_block(void);               /* decrypt 8 bytes using DES */
char *mov64(char *dest, char *source);  /* quick 8-byte move (returns dest) */
char *xor64(char *dest, char *source);  /* quick 8-byte xor (returns dest)  */
void killkey(void);                     /* zeros out key schedule       */

/* ----------- tables ------------ */
extern unsigned char block[8];          /* holds the i/o data           */

#ifdef TRIPLE_DES
extern char en_keytbl[];
extern char de_keytbl[];
#else
extern char keytbl[];
#endif
