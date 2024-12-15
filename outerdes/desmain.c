/*-- desmain.c -- Program to perform Data Encryption Standard (DES)
 *  encryption/decryption.
 *
 *  Implements ECB mode (Electronic Code Book: simple block-by-block 
 *  independent encryption) and CBC mode (Cipher Block Chaining;
 *  in enciphering, each block is XORed with the output of the 
 *  previous block before encryption).  
 *
 *  The output file is 1-8 bytes longer than input
 *  on encryption, and vice versa on decryption.
 *  This is because the last byte of the last block is reserved
 *  for storing the number of real data bytes in the last block.
 *  This idea is taken from SunOS's DES implementation.
 *
 *  Note that much of the code in this main program deals with
 *  handling the last block.
 *
 *  Mark Riordan  5 March 1991
 */

#include <stdio.h>
#ifdef MSDOS
#include <io.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "d3des.h"
#include "usagepro.h"
#include "getoptpr.h"

static char *Author =
 "Mark Riordan  1100 Parker  Lansing, MI  48912  mrr@ripem.msu.edu";
static char Author_email[] = 
  {'M','R','R',':',' ','m','r','r','@',
   'r','i','p','e','m','.','m','s','u','.','e','d','u','\0'};
static char *date_written= " 7 March 1991, 25 Feb 94 & 7 July 1994";
static char *Author2= "DES code by Richard Outerbridge";
static char Author2_email[] = 
  {'R','W','O',':',' ','7','1','7','5','5','.','2','0','4','@',
    'C','o','m','p','u','S','e','r','v','e','.','c','o','m','\0'};

static char *usage_msg[] = {
   "DES encryption program by Richard Outerbridge and Mark Riordan",
   "Usage:  des {-e|-d} {-k key|-h hexkey} [-b] ",
   "            {-i infile|<in} {-o outfile|>out}",
   "  where:",
   "  -e      means encipher; -d means enciper. ",
   "  key     is an 8-character ASCII key.  It will be modified so",
   "          that its high bit is an odd-parity bit, and the low order",
   "          bit will be stripped, to make a total of 56 bits.",
   "          This makes it compatible with Sun's des program.",
   "  hexkey  is a 16 digit hex key, which will be modified as above.",
   "  -b      selects encryption/decryption in Electronic Code Book mode,",
   "          (i.e., basic independent block-by-block encipherment).",
   "          Default without -b is Cipher Block Chaining mode, in",
   "          which each block is XORed with the output of the previous",
   "          block before being encrypted.  The first block",
   "          (the initializing vector) is assumed to be zero.",
   "          CBC mode is more secure than ECB mode.",
   "  infile  is the input file, else standard input is used.",
   "  outfile is the output file, else standard output is used.",
   " ",
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
#define UINT32 unsigned long int

#ifdef DEBUG
#define BUFSIZE1 16
#define BUFSIZE2 16
#else
#define BUFSIZE1 8192 
#define BUFSIZE2 8192
#endif
   unsigned char key[MAXKEY];
   char line[3];
   unsigned char *keyarg;
   int inhand, outhand;
   FILE *instream=stdin,*outstream=stdout;
   unsigned char *buf, *blockptr, *begbptr, *endbptr, *buf1, *buf2;
   UINT32 iv[2] = {0, 0};
   UINT32 ivnext[2];

   extern unsigned char *myoptarg;
   int c;
   int gotcip = FALSE, gotkey=FALSE, hexkey=FALSE;
   int encipher, j, parity, bit, ich, keylen, bytesinbuf, extra=0;
   int bufgood=FALSE, buf2bytes, badarg=FALSE;
   enum enum_desmodes {ECB, CBC} blockmode = CBC;

   /* Crack the command line. */

   while((c = mygetopt(argc,argv,"bdek:h:i:o:")) != -1) {
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
            keyarg = myoptarg;
            gotkey = TRUE;
            break;
         case 'h':
            keyarg = myoptarg;
            hexkey = TRUE;
            break;
         case 'b':
            blockmode = ECB;
            break;
         case 'i':
            instream = fopen(myoptarg,"r");
            if(!instream) {
               fputs("Can't open input file.\n",stderr);
               return 1;
            }
            break;
         case 'o':
            outstream = fopen(myoptarg,"w");
            if(!outstream) {
               fputs("Can't open output file.\n",stderr);
               return 2;
            }
            break;
         case '?':
            badarg = TRUE;
            break;

      }
   }

   if(badarg || !(gotkey && gotcip)) {
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
       * odd parity bit stuff for hex keys.  I chose not to.
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

   /* Wipe out the command-line arguments */
   for(j=1; j<argc; j++) {
      char *wptr = argv[j];
      while(*wptr) *(wptr++) = ' ';
      argv[j] = NULL;
   }

   deskey(key,1-encipher);
   inhand = fileno(instream);
   outhand = fileno(outstream);

#ifdef MSDOS
   /* Flush streams to work around compiler bug. */
   fflush(instream);
   fflush(outstream);
   
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
            if(blockmode == CBC) {
               /* In Cipher Block Chaining mode, first XOR the input
                * with the output from the last block.
                */
               *((UINT32 *)blockptr)     ^= iv[0];
               *((UINT32 *)(blockptr+4)) ^= iv[1];
            }
            des(blockptr,blockptr);
            /* Save the encrypted output of this block for next
             * time.  Needed in CBC mode only, but an explicit
             * test for CBC here would probably only slow things down.
             */
            iv[0] = *((UINT32 *) blockptr);
            iv[1] = *((UINT32 *) (blockptr+4));
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
      if(blockmode == CBC) {
         *((UINT32 *)buf)     ^= iv[0];
         *((UINT32 *)(buf+4)) ^= iv[1];
      }
      des(buf,buf);
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
            /* Save the ciphertext of this block for CBC decryption
             * of the next block.  Not needed in ECB mode, but
             * we'll do it anyway.
             */
            ivnext[0] = *((UINT32 *)blockptr);
            ivnext[1] = *((UINT32 *)(blockptr+4));

            des(blockptr,blockptr);

            if(blockmode == CBC) {
               /* In Cipher Block Chaining mode, XOR the result
                * with the output from the last block.
                */
               *((UINT32 *)blockptr)     ^= iv[0];
               *((UINT32 *)(blockptr+4)) ^= iv[1];
               iv[0] = ivnext[0];
               iv[1] = ivnext[1];
            }
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
         bufgood = TRUE;
      }
      /* OK--this buffer contains the last block.
       */
      buf2bytes = buf2bytes - 8 + buf2[buf2bytes-1];
      write(outhand,buf2,buf2bytes);
   }
#ifdef MSDOS
   /* Flush streams to work around compiler bug. */
   fflush(instream);
   fflush(outstream);
#endif
   return 0;
}
