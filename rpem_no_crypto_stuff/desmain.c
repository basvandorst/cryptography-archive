/*-- desmain.c -- Program to perform Data Encryption Standard (DES)
 *  encryption/decryption.
 *
 *  Implements ECB mode only (simple block-by-block independent
 *  encryption).  The output file is 1-8 bytes longer than input
 *  on encryption, and vice versa on decryption.
 *  This is because the last byte of the last block is reserved
 *  for storing the number of real data bytes in the last block.
 *  This idea is taken from SunOS's DES implementation.
 *
 *  Note that much of the code in this main program deals with
 *  handling the last block.  What a pain.
 *
 *  Mark Riordan  5 March 1991
 *  This code is hereby placed in the public domain.
 */

#include <stdio.h>
#ifdef MSDOS
#include <io.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "des.h"
#include "desproto.h"
#include "usagepro.h"
#include "getoptpr.h"

static char *author =
 "Mark Riordan  1100 Parker  Lansing, MI  48912  riordanmr@clvax1.cl.msu.edu  7 March 1991";
static char *author2= "DES code by Phil Karn and Jim Gillogly";

static char *usage_msg[] = {
   "Usage:  des {-e|-d} {-k key|-h hexkey} <in >out",
   "  where:",
   "  -e     means encipher; -d means enciper. ",
   "  key    is an 8-character ASCII key.  It will be modified so",
   "         that its high bit is an odd-parity bit, and the low order",
   "         bit will be stripped, to make a total of 56 bits.",
   "  hexkey is a 16 digit hex key, which will be modified as above.",
   " ",
   " Performs DES encryption/decryption in Electronic Code Book mode only",
   " (i.e., basic independent block-by-block encipherment).",
   " The last byte of the last block of an encrypted file contains",
   " the number of good bytes in that block (can be zero).",
   NULL
};

int
main(argc,argv)
int argc;
char *argv[];
{
#define MAXKEY 200
#define TRUE 1
#define FALSE 0

#ifdef DEBUG
#define BUFSIZE1 16
#define BUFSIZE2 16
#else
#define BUFSIZE1 24576
#define BUFSIZE2 16384
#endif
   unsigned char key[MAXKEY];
   char line[3];
   DES desbuf;
   unsigned char *keyarg;
   int inhand, outhand;
   FILE *instream=stdin,*outstream=stdout;
   unsigned char *buf, *blockptr, *begbptr, *endbptr, *buf1, *buf2;

   extern unsigned char *optarg;
   char c;
   int gotcip = FALSE, gotkey=FALSE, hexkey=FALSE;
   int encipher, j, parity, bit, ich, keylen, bytesinbuf, extra=0;
   int bufgood=0, buf2bytes;
   unsigned int bytesinline;

   /* Crack the command line. */

   while((c = getopt(argc,argv,"dek:h:")) != -1) {
      switch (c) {
         case 'd':
            encipher = FALSE;
            gotcip = TRUE;
            break;
         case 'e':
            encipher = TRUE;
            gotcip = TRUE;
            break;
         case 'k':
            keyarg = optarg;
            gotkey = TRUE;
            break;
         case 'h':
            keyarg = optarg;
            hexkey = TRUE;
            break;

      }
   }

   if(!(gotkey && gotcip)) {
      usage(NULL,usage_msg);
      exit(1);
   }


   /* Process the key.  For both hex and ASCII keys, zero-pad
    * the key if the one provided is less than 8 bytes.
    */
   keylen = strlen(keyarg);
   if(hexkey) {
      /* Process a hex key.  I don't know the standard for processing
       * hex keys, so I don't know whether I ought to do the
       * odd parity bit stuff for hex keys.
       */
      if(keylen&1) {
         fputs("Hex key must have even number of hex digits.\n",stderr);
         return 1;
      }
      keylen /= 2;
      line[2] = '\0';
      for(j=0; j<keylen; j++) {
         line[0] = keyarg[2*j];
         line[1] = keyarg[2*j+1];
         sscanf(line,"%2x",&ich);
         key[j] = ich;
      }
   } else {
      /* Process an ASCII key.  As is common with DES keys,
       * set the top bit of each byte to reflect odd parity.
       * Karn's DES code ignores the bottom bit of each byte.
       */
      for(j=0; j<keylen; j++) {
         c = keyarg[j];
         for(parity=0x80,bit=0; bit<7; bit++) {
            c <<= 1;
            parity ^= (c&0x80);
         }
         key[j] = (keyarg[j]&0x7f) | parity;
      }
   }

   /* Zero-pad the key (with odd parity)
    */
   for(; j<8; j++) key[j] = 0x80;

#if 0
   printf("Key in hex=");
   for(j=0; j<8; j++) printf("%2x",key[j]);
   putchar('\n');
   fflush(stdout);
#endif

   des_init(&desbuf,0);
   des_setkey(&desbuf,key);
   inhand = fileno(instream);
   outhand = fileno(outstream);

#ifdef M_I86
   setmode(inhand,O_BINARY);
   setmode(outhand,O_BINARY);
#endif

   /* Do the encipherment/decipherment.
    * This code may be difficult to follow, because I go out of my
    * way to do I/O in large chunks that are nice round numbers
    * so as to make use of the operating system's I/O facilities
    * as efficiently as possible.
    *
    * Note that I also try to allow for input that comes in chunks
    * of non-multiples of 8 bytes (as might happen when reading from
    * a socket over the net, for instance).
    *
    * "extra" is the number of bytes in the buffer mod 8 (i.e.,
    * those that won't be processed on this pass).
    */
   if(encipher) {
      buf = malloc(BUFSIZE1+8);

      while((bytesinbuf=read(inhand,buf+extra,BUFSIZE1)) > 0) {
         bytesinbuf += extra;
         blockptr = begbptr = buf;
         extra = 7&bytesinbuf;
         endbptr = blockptr - extra + bytesinbuf;
         for(; blockptr != endbptr; blockptr += 8) {
            des_encrypt(&desbuf,blockptr);
         }
         write(outhand,begbptr,bytesinbuf-extra);
         /* If the number of bytes wasn't a multiple of 8,
          * copy the extra bytes back to the beginning of the buffer
          * and next time, start the read just after these bytes.
          * This will be no more than 7 bytes.
          */
         if(extra) {
            for(j=0; j<extra; j++) buf[j] = blockptr[j];
         }
      }
      buf[7] = extra;
      des_encrypt(&desbuf,buf);
      write(outhand,buf,8);
   } else {
      /* Decipher the input.
       * This is more complicated than above, because we have to
       * detect when we have the last block of the file and
       * special-case it.
       * In order to do this efficiently, I use double-buffering.
       * buf1 is the buffer currently being processed.
       * buf2 is the "dirty" buffer that is waiting to be flushed.
       * We don't flush buf2 as soon as it's decrypted because
       * it may have the last block, which needs to be treated
       * differently.
       *
       * The below code may answer the question:  "Should I try
       * to write programs while watching a James Bond movie?"
       */
      buf1 = malloc(BUFSIZE2);
      buf2 = malloc(BUFSIZE2);
      if(!buf1 || !buf2) {
         fputs("Cannot allocate memory!",stderr);
         exit(1);
      }

      while((bytesinbuf=read(inhand,buf1+extra,BUFSIZE2)) > 0) {
         bytesinbuf += extra;
         blockptr = begbptr = buf1;
         extra = 7&bytesinbuf;
         endbptr = blockptr - extra + bytesinbuf;
         for(; blockptr != endbptr; blockptr += 8) {
            des_decrypt(&desbuf,blockptr);
         }
         if(bufgood) {
            /* This isn't the first buffer, so we can flush it. */
            write(outhand,buf2,buf2bytes);
         }
         if(extra) {
            for(j=0; j<extra; j++) buf2[j] = blockptr[j];
         }
         buf = buf1; buf1 = buf2; buf2 = buf;
         buf2bytes = bytesinbuf-extra;
         bufgood = 1;
      }
      /* OK--this buffer contains the last block.
       */
      buf2bytes = buf2bytes - 8 + buf2[buf2bytes-1];
      write(outhand,buf2,buf2bytes);
   }
   return 0;
}
