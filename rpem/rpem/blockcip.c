/*--- blockcip.c -- Routines to provide a common interface to
 *   various block cipher algorithms.
 *
 *   The block ciphers implemented so far are:
 *     DES
 *     NEWDES  (*not* a modified version of DES)
 *     none    (non-encryption; does nothing)
 *
 *   Mark Riordan   23 March 1991
 *   This code is hereby placed in the public domain.
 */

#include <stdio.h>
#include <stdlib.h>
#include "des.h"
#include "desproto.h"
#include "newdespr.h"
#include "newdes.h"
#include "blockcip.h"
#include "blockcpr.h"

/*--- function BlockInit ------------------------------------------------
 *
 *  Initialize a block cipher system.
 *
 *  Entry:  cipptr points to a structure whose "algorithm"
 *                 field is filled in.
 *
 *  Exit:   cipptr's blocksize, keysize, & key fields are filled in.
 */
void
BlockInit(cipptr)
TypCipherInfo *cipptr;
{
   switch(cipptr->algorithm) {
      case des:
#ifndef EXPORT
         cipptr->key = (unsigned char *) malloc(sizeof(DES));
         des_init((DES *)cipptr->key,0);
         cipptr->blocksize = 8;
         cipptr->keysize = 8;
#else
         fputs("DES not available in export version.\n",stderr);
         exit(1);
#endif
         break;

      case newdes:
         cipptr->key = (unsigned char *) malloc(NEWDES_SIZE_KEY_UNRAV);
         cipptr->blocksize = NEWDES_BLOCK_BYTES;
         cipptr->keysize = NEWDES_USER_KEY_BYTES;
         break;

      case none:
         cipptr->key = NULL;
         cipptr->blocksize = 8;  /* arbitrary */
         cipptr->keysize = 8;  /* arbitrary */
         break;
   }
}


/*--- function BlockSetKey ------------------------------------------------
 *
 *  Set the key to be used in subsequent encryption/decryption.
 *
 *  Entry:  key      is the key to use.
 *          nbytes   is the number of bytes in the key.
 *          encipher is TRUE if we are about to encipher, FALSE
 *                   if we are about to decipher.  (For some systems,
 *                   the process of setting the key depends on
 *                   whether it's to be used to encipher or decipher.)
 *          cipptr   tells which algorithm is being used.  Space
 *                   has been allocated for the key.
 *
 *  Exit:   The key has been set.
 */
void
BlockSetKey(key,nbytes,encipher,cipptr)
unsigned char *key;
int nbytes;
int encipher;
TypCipherInfo *cipptr;
{
   int j, bit;
   unsigned char c, parity, kbytes[64];

   switch(cipptr->algorithm) {
      case des:
#ifndef EXPORT
         /* Process an ASCII key.   As is common with DES keys,
          * set the top bit of each byte to reflect odd parity.
          * Karn's DES code ignores the bottom bit of each byte.
          */
         for(j=0; j<nbytes; j++) {
            c = key[j];
            for(parity=0x80,bit=0; bit<7; bit++) {
               c <<= 1;
               parity ^= (c&0x80);
             }
             kbytes[j] = (unsigned char) (key[j]&0x7f) | parity;
         }

         /* Zero-pad the key (with odd parity)
          */
         for(; j<8; j++) kbytes[j] = 0x80;

         des_setkey((DES *)cipptr->key,(char *)kbytes);
#else
         fputs("DES not available in export version.\n",stderr);
         exit(1);
#endif
         break;

      case newdes:
         if(encipher) {
            NewdesSetKeyEncipher(key,cipptr->key);
         } else {
            NewdesSetKeyDecipher(key,cipptr->key);
         }
         break;
   }
}

/*--- function BlockEncipherBlock -----------------------------------------
 *
 *  Encrypt a block.
 *
 *  Entry:  buf     points to a block of bytes.  Its length depends
 *                  on the algorithm being used.
 *          cipptr  describes the cipher being used.
 *
 *  Exit:   buf     has been encrypted in place.
 */
void
BlockEncipherBlock(buf,cipptr)
unsigned char *buf;
TypCipherInfo *cipptr;
{
   switch(cipptr->algorithm) {
      case des:
#ifndef EXPORT
         des_encrypt((DES *)cipptr->key,(char *)buf);
#else
         fputs("DES not available in export version.\n",stderr);
         exit(1);
#endif
         break;

      case newdes:
         NewdesBlock(buf,cipptr->key);
         break;
   }
}

/*--- function BlockDecipherBlock -----------------------------------------
 *
 *  Decrypt a block.
 *
 *  Entry:  buf     points to a block of bytes.  Its length depends
 *                  on the algorithm being used.
 *          cipptr  describes the cipher being used.
 *
 *  Exit:   buf     has been decrypted in place.
 */
void
BlockDecipherBlock(buf,cipptr)
unsigned char *buf;
TypCipherInfo *cipptr;
{
   switch(cipptr->algorithm) {
      case des:
#ifndef EXPORT
         des_decrypt((DES *)cipptr->key,(char *)buf);
#else
         fputs("DES not available in export version.\n",stderr);
         exit(1);
#endif
         break;

      case newdes:
         NewdesBlock(buf,cipptr->key);
         break;
   }
}

/*--- function BlockEncipherBuf -------------------------------------------
 *
 *  Encipher a buffer of bytes.
 *
 *  Entry:  buf     is a buffer of bytes.
 *          nbytes  is the number of bytes in the buffer.
 *          cipptr  describes the cipher being used.
 *
 *  Exit:   buf     has been encrypted in place.
 *          Returns the number of encrypted bytes.  This will be == nbytes
 *            unless nbytes wasn't a multiple of the block size, in
 *            which case it is rounded up to the next multiple of
 *            the block size.
 */
int
BlockEncipherBuf(buf,nbytes,cipptr)
unsigned char *buf;
int nbytes;
TypCipherInfo *cipptr;
{
   unsigned int mylen, mylen2;
   unsigned char *keyptr = cipptr->key;
   int blksize = cipptr->blocksize;

   mylen2 = mylen = (((nbytes - 1) / blksize) + 1) * blksize;

   for(;mylen; mylen -= blksize){
      BlockEncipherBlock(buf,cipptr);
      buf += blksize;
   }

   return(mylen2);
}

/*--- function BlockDecipherBuf --------------------------------------------
 *
 *  Decipher a buffer of bytes.
 *
 *  Entry:  buf     is a buffer of bytes.
 *          nbytes  is the number of bytes in the buffer.
 *          cipptr  describes the cipher being used.
 *
 *  Exit:   buf     has been decrypted in place.
 *          Returns the number of decrypted bytes.  This will be == nbytes
 *            unless nbytes wasn't a multiple of the block size, in
 *            which case it is rounded up to the next multiple of
 *            the block size.
 *
 */
int
BlockDecipherBuf(buf,nbytes,cipptr)
unsigned char *buf;
int nbytes;
TypCipherInfo *cipptr;
{
   unsigned int mylen, mylen2;
   int blksize = cipptr->blocksize;
   unsigned char *keyptr = cipptr->key;

   mylen2 = mylen = (((nbytes - 1) / blksize) + 1) * blksize;

   for(;mylen; mylen -= blksize){
      BlockDecipherBlock(buf,cipptr);
      buf += blksize;
   }

   return(mylen2);
}

/*--- function BlockFinish ----------------------------------------------
 *
 *  Clean up when finished encrypting/decrypting with a given key.
 *
 *  Entry:  cipptr   points to a structure describing the key and
 *                   cipher being used.
 *
 *  Exit:   Allocated memory has been freed.
 */
void
BlockFinish(cipptr)
TypCipherInfo *cipptr;
{
   if(cipptr->key) free(cipptr->key);
}
