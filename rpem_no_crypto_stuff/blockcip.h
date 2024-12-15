/*--- blockcip.h -- Header file for general interface to block ciphers.
 *  Mark Riordan   23 March 1991
 *  Placed into the public domain.
 */

#define MAXKEYSIZE 16
typedef enum enum_block_mode {cbc, ecb} TypBlockMode;
typedef enum enum_algorithm {des, newdes, none} TypAlgorithm;
typedef enum enum_file_mode {FMascii, FMbinary, FMlast} TypFileMode;

typedef struct struct_block {
   TypAlgorithm  algorithm;
   TypBlockMode  blockmode;
   int           blocksize;
   int           keysize;
   unsigned char *key;
} TypCipherInfo;
