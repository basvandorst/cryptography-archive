/*--- makerkey.c --  Create two primes and their product.
 *
 *   For use by "rpem", a privacy-enhanced mail public key cryptosystem.
 *
 *   Mark Riordan   14 March 1991
 *   This code is hereby placed in the public domain.
 */
#include <stdio.h>
#include <fcntl.h>
#ifdef MSDOS
#include <io.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "zbigint.h"
#include "zproto.h"
#include "cippkgpr.h"
#include "newdes.h"
#include "newdespr.h"
#include "getsyspr.h"
#include "prcodepr.h"
#include "usagepro.h"
#include "blockcip.h"
#include "blockcpr.h"
#include "p.h"

char *author = "Mark Riordan  1100 Parker  Lansing MI  48912";
char *author2= "riordanmr@clvax1.cl.msu.edu  March 1991";

int getopt P((int argc,char **argv, char *opts));

#define TRUE 1
#define FALSE 0

#define PREFERRED_UUENCODE_LEN  45

char *usage_msg[] = {
  "makerkey 1.0 -- Make Rabin key for \"rpem\"",
  "Creates two primes and their product (the modulus), for use",
  "   by the Rabin cryptosystem.",
  " ",
  "Usage: makerkey [-s privfile] [-b numbits] [-b numbits2] [-g] \\",
  "  [-u username] >>modulusout",
  " ",
  " where:",
  "-s   (\"secret\") specifies the filename into which the two primes",
  "     should be placed.  This is the private component of the key.",
  "     If -s is not specified, the value of the environment variable",
  "     RABIN_PQ_FILE is used.  If this variable is not defined,",
  "     the file ~/.rabinkey is used for Unix, or \\RABINKEY for MS-DOS.",
  " ",
  "-b   (\"bits\") specifies the number of bits desired in the primes.",
  "     May be specified twice--once for p and once for q.",
  "     If specified only once, that number of bits will be used",
  "     for p and a slightly larger (random) number of bits will",
  "     be used for q.  If not specified, p defaults to 100 bits",
  "     for performance reasons; if you're serious about security,",
  "     pick a number > 330.",
  " ",
  "     Warning:  RFC 1114 specifies a minimum modulus size (sum of",
  "     bit sizes of the two primes) of 512 bits.  Similar guidelines",
  "     should probably apply to the Rabin system.  Because the Rabin",
  "     system is rather slow, a smaller default has been chosen.",
  " ",
  "-g   (\"garble\") means that the primes in the output file are encrypted.",
  "     If specified, the program will prompt for the encryption key.",
  "     The NEWDES algorithm (Robert Scott, Cryptologia, Jan 1985)",
  "     is used.",
  " ",
  "-n   (\"no prompt\") means do not prompt the user for a pseudo-random seed,",
  "     but instead rely solely on system-dependent information gathered",
  "     by the program (such as the time of day) to generate the primes.",
  "     The default is to prompt the user for a string as well as",
  "     use system-dependent information.",
  " ",
  "-u   (\"username\") specifies the username to use in the modulus output file.",
  "     Default under Unix is your userid@hostname.",
  "     Default under MSDOS is the value of the environment variable",
  "     USER_NAME; if that does not exist, \"me\" is used.",
  NULL
};

FILE *userstream;

int
main(argc,argv)
int argc;
char *argv[];
{
#define RANDOMBUFSIZE   200
#define DEFAULTBITS     100
#define MINDIFF         7
#define BIGINT_BYTES    ((ZSIZEP*MYINT_BITS/BITS_PER_BYTE)+8)
#define LINESIZE        128

#define RABIN_PQ_FILE "RABIN_PQ_FILE"

#ifdef UNIX
#define DEFPQFILENAME ".rabinkey"
#endif

#ifdef MSDOS
#define DEFPQFILENAME "\\RABINKEY"
#endif

#define SYS_RAN   0
#define USER_RAN  1
#define KEY_RAN   2

   unsigned char allranbuf[3*RANDOMBUFSIZE], sysranbuf[RANDOMBUFSIZE];
   unsigned char userinbuf[RANDOMBUFSIZE], keytimebuf[RANDOMBUFSIZE];
   unsigned char userinbuf2[RANDOMBUFSIZE];
   int rancnt[3], allrancnt, usercnt2, rancnt2;

   unsigned char *ranptrs[3];

   unsigned char key[NEWDES_USER_KEY_BYTES];
   BIGINT primes[2], modulus;
#ifdef DEBUG
   BIGINT m1, mymod, m2;
#endif
   unsigned char bytes[BIGINT_BYTES];
   TypCipherInfo cipinfo;

   FILE *pqstream, *modstream=stdout;
   extern char *optarg;
   char *cptr, username[LINESIZE], pqfilename[LINESIZE];
   unsigned char byte, *bptr;
   char ch;
   int encipher = FALSE, prompt = TRUE, argerror = FALSE;
   int bitarg = 0;
   MYINT bitcount[2];
   int primecnt, lastgood, pwok, writepqfile;
   int j, ipr, c, loopct, needanswer=TRUE;
   long int ibit0, ibit1;

#ifdef UNIX
   userstream = fopen("/dev/tty","r");
#endif

   zstart();
   zrstart((MYINT)44444447);
#ifdef DEBUG
   {
     int j;   MYINT result;
      fputs("After zrstart, ready to zrandomint ",stderr);
      for(j=0; j<12; j++)  {
         result = zrandomint((MYINT)423468854+j);
         fputs(".",stderr);
      }
      fputs("  done.\n",stderr);
   }
#endif
   username[0] = pqfilename[0] = '\0';

   while((c = getopt(argc,argv,"b:s:gnu:")) != -1) {
      switch (c) {
         case 'g':
            encipher = TRUE;
            break;

         case 'n':
            prompt = FALSE;
            break;

         case 'b':
            if(bitarg > 1) {
               argerror = TRUE;
            } else if(0 == (bitcount[bitarg] = (MYINT)atoi(optarg))) {
               argerror = TRUE;
            }
            bitarg++;
            break;

         case 'u':
            strncpy(username,optarg,LINESIZE);
            break;

         case 's':
            strncpy(pqfilename,optarg,LINESIZE);
            break;

         default:
            argerror = TRUE;

      }
   }

   /* Clear the parameters so that users typing "ps" or "w" can't
    * see the password.
    */

   for(j=1; j<argc; j++) {
      cptr = argv[j];
      while(*cptr) *(cptr++) = '\0';
   }

   /* Check for syntax error. */
   if(argerror) {
      usage(NULL,usage_msg);
      exit(1);
   }

   /* Open the output file.
    * If -s wasn't specified, try the environment variable.
    * If that doesn't exist, use the default file name.
    */

   if(!pqfilename[0]) {
      if((cptr=getenv(RABIN_PQ_FILE))) {
         strncpy(pqfilename,cptr,LINESIZE);
      } else {
#ifdef UNIX
         GetUserHome(pqfilename,LINESIZE);

         if(pqfilename[strlen(pqfilename)-1] != '/') {
            strncat(pqfilename,"/",LINESIZE-strlen(pqfilename));
         }
         strncat(pqfilename,DEFPQFILENAME,LINESIZE-strlen(pqfilename));
#else
         strcpy(pqfilename,DEFPQFILENAME);
#endif
      }
   }

   /* Check for an existing secret key file.
    * We do this by first opening for read-only access.
    * Not quite the right way, but it's portable and I'm getting
    * tired of #ifdef's.
    */

   writepqfile = TRUE;
   pqstream = fopen(pqfilename,"r");
   while(pqstream && needanswer) {
      fprintf(stderr,"%s already exists.  Overwrite? ",pqfilename);
      usercnt2 = 1; rancnt2 = 0;
      GetUserInput(userinbuf,&usercnt2,sysranbuf,&rancnt2,TRUE);
      ch = (char) userinbuf[0];
      if(usercnt2 > 0) {
         if(ch == 'y' || ch == 'Y') {
            needanswer = FALSE;
            fclose(pqstream);
         } else if(ch == 'n' || ch == 'N') {
            needanswer = FALSE;
            writepqfile = FALSE;
         }
      }
   }

   if(!writepqfile) {
      fprintf(stderr,"No key created.\n");
      return 1;
   }

   pqstream = fopen(pqfilename,"w");
   if(!pqstream) {
      fprintf(stderr,"Error: can't open output file %s\n",pqfilename);
      exit(1);
   }

   /* Find out the username */

   if(!username[0]) {
      GetUserAddress(username,LINESIZE);
   }

   /* Obtain random bytes from the system.   */

   rancnt[SYS_RAN] = GetRandomBytes(sysranbuf,RANDOMBUFSIZE);
#ifdef DEBUG
   {
     int j;   MYINT result;
      fputs("After GetRandomBytes, ready to zrandomint",stderr);
      for(j=0; j<12; j++)  {
         result = zrandomint((MYINT)423468854+j);
         fputs(".",stderr);
      }
      fputs("  done.\n",stderr);
   }
#endif
   /* Check to see how many bit counts were given.  If none, fill
    * in the default.
    */
   if(bitarg == 0) {
      bitcount[0] = DEFAULTBITS;
   }

   /* If less than 2 bit counts were given, compute the number of
    * bits in the second prime as a random number slightly larger
    * than the number of bits in the first prime.
    */
   if(bitarg < 2) {
      byte = 0;
      for(j=0; j<rancnt[SYS_RAN]; j++) byte ^= sysranbuf[j];
      bitcount[1] = bitcount[0] + MINDIFF + (int)(0xf & byte);
   }


   /* Prompt the user for pseudo-random input if necessary.  */

   if(prompt) {
      fputs("Enter a random string: ",stderr);
      rancnt[USER_RAN] = rancnt[KEY_RAN] = RANDOMBUFSIZE;
      GetUserInput(userinbuf,&rancnt[USER_RAN],keytimebuf,&rancnt[KEY_RAN],TRUE);
   }
#if 1
   ranptrs[SYS_RAN] = sysranbuf;
   ranptrs[USER_RAN] = userinbuf;
   ranptrs[KEY_RAN] = keytimebuf;
   allrancnt = ConcatArrays(ranptrs,rancnt,3,allranbuf,3*RANDOMBUFSIZE);
#else
   memcpy(randombuf+randomcnt,userinbuf,usercnt);
   randomcnt += usercnt;
   memcpy(randombuf+randomcnt,keytimebuf,timecnt);
   randomcnt += timecnt;
#endif
   /* Choose the first prime */
   ChoosePrime(bitcount[0],allranbuf,allrancnt,&primes[0]);

   /* Choose the second prime.  We use the same random data as for
    * the first prime, but we mash it around quite a bit before
    * using it.
    */

   cipinfo.algorithm = newdes;
   cipinfo.blockmode = ecb;
   BlockInit(&cipinfo);
   for(loopct=0; loopct<18; loopct++) {
      BlockSetKey(allranbuf,NEWDES_USER_KEY_BYTES,1,&cipinfo);
      bptr = allranbuf;
      j=0;
      do {
         BlockEncipherBlock(bptr++,&cipinfo);
      } while(++j < (3*RANDOMBUFSIZE-2*NEWDES_BLOCK_BYTES));

      bptr = allranbuf + 3*RANDOMBUFSIZE-2*NEWDES_BLOCK_BYTES;
      for(j=0; bptr!=allranbuf; j++) {
         BlockEncipherBlock(bptr--,&cipinfo);
         *bptr += j;
      }
   }

   ChoosePrime(bitcount[1],allranbuf,allrancnt,&primes[1]);

   /* If we are going to encipher the primes, prepare the key. */

   if(encipher) {
      do {
         fputs("Enter a key: ",stderr);
         rancnt[USER_RAN] = RANDOMBUFSIZE;
         rancnt[KEY_RAN] = 0;
         for(j=0; j<NEWDES_USER_KEY_BYTES; j++) key[j] = 0;
         GetUserInput(userinbuf,&rancnt[USER_RAN],keytimebuf,&rancnt[KEY_RAN],FALSE);

         fputs("Enter again: ",stderr);
         usercnt2 = RANDOMBUFSIZE;
         rancnt2 = 0;
         GetUserInput(userinbuf2,&usercnt2,keytimebuf,&rancnt2,FALSE);
         pwok = (usercnt2==rancnt[USER_RAN]) &&
           strncmp((char *)userinbuf,(char *)userinbuf2,usercnt2)==0;
         if(!pwok) {
            fputs("Keys do not match.  Please try again.\n",stderr);
         }
      } while(!pwok);

      /* Use all of the key bytes typed by the user, even if we
       * have to take several passes through userinbuf.
       * Fill all of the key bytes, even if we have to reuse
       * some of the user input.
       */
      ExpandBytes(userinbuf,rancnt[USER_RAN],key,NEWDES_USER_KEY_BYTES);
      BlockSetKey(key,NEWDES_USER_KEY_BYTES,1,&cipinfo);
   }

   /* Now convert the primes to bytes, encipher if necessary,
    * and output.
    */

   for(ipr=0; ipr<2; ipr++) {
      primecnt = BignumToBytes(&primes[ipr],0,bytes);
      if(encipher) {
         lastgood = primecnt % NEWDES_BLOCK_BYTES;
         primecnt = ((primecnt + NEWDES_BLOCK_BYTES) / NEWDES_BLOCK_BYTES) *
          NEWDES_BLOCK_BYTES;
         bytes[primecnt-1] = (unsigned char)lastgood;
#if 1
         BlockEncipherBuf(bytes,primecnt,&cipinfo);
#else
         for(j=0; j<primecnt; j+=NEWDES_BLOCK_BYTES) {
            newdes_block(bytes+j);
         }
#endif
      }

      if(ipr == 0) {
         fputs("p\n",pqstream);
      } else {
         fputs("q\n",pqstream);
      }

      PutEncodedBytes(pqstream,bytes,primecnt);
   }

   /* Compute the modulus and output it. */

   zmul(&primes[0],&primes[1],&modulus);

   primecnt = BignumToBytes(&modulus,0,bytes);
   fputs("Rabin-Modulus: \n",pqstream);
   PutEncodedBytes(pqstream,bytes,primecnt);

   fputs("User: ",modstream);
   fputs(username,modstream);
   fputc('\n',modstream);

   fputs("Rabin-Modulus: \n",modstream);
   PutEncodedBytes(modstream,bytes,primecnt);
   fputc('\n',modstream);

   ibit0 = zlog(&primes[0]);  ibit1 = zlog(&primes[1]);
   fprintf(stderr,"Wrote primes of %ld and %ld bits to %s\n",
     ibit0, ibit1, pqfilename);

   return 0;
}
