/*--- mrrrrabts.c -- Test Rabin cryptosystem.
 *
 *  Mark Riordan   10 Feb 1991
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "zbigint.h"
#include "zproto.h"
#include "zmiscpro.h"
#include "cippkgpr.h"
#include "p.h"

#include "typtime.h"

void display_time P((TYPTIME begtime,TYPTIME endtime, char *msg));
void my_get_time P((TYPTIME *mytime));
void mberlekamp P((BIGINT *ain, BIGINT *p,/* out */ BIGINT *r));
int mcrt P((int num,BIGINT *resids, BIGINT *mods, /* out */ BIGINT *prodmod, BIGINT *cr));
void msqrt P((BIGINT *a,BIGINT *prime1,BIGINT *prime2,BIGINT sqrts[]));
void put_bignum(BIGINT *a);


int
main(argc,argv)
int argc;
char *argv[];
{
   BIGINT prime1, prime2;
   BIGINT temp, temp2, temp3;
   BIGINT planum, planum2, cipnum, cipnum2, decipnum[4];
   BIGINT msg, r, m2p1, e1, d1;
   int j, resp, jac, quit=0, mysize, result, nbytes, nbytescip, nbytesplain;
   TYPTIME begtime, endtime;
   FILE *instream;
   MYINT  nbits;
#define MAXBYTES 200
   unsigned char bytes[MAXBYTES], bytes2[MAXBYTES], bytes3[2*MAXBYTES];
   int nbytes1, nbytes2;
   char line[MAXBYTES];

   if(argc>1) {
      instream = fopen(argv[1],"r");
   } else {
      instream = stdin;
   }

   zstart();
   zrstart((MYINT)44444447);
   prime1.len = 0;

 do {
   printf(
   "Enter 1(decrypt); 2(encrypt) 3(Williams) 4(prime) 5(bytes) 0(stop): ");
   resp = -1;
   do {
      fgets(line,MAXBYTES,instream);
      sscanf(line,"%d",&resp);
   } while (resp == -1);
   if(resp == 0) {
      quit = 1;
      continue;
   }

  if(resp == 1) {
   /* Test the "decryption" of a large integer.
    */
   printf("Enter prime1: ");
   fzread(instream,&prime1);
   printf("Enter prime2: ");
   fzread(instream,&prime2);
   zmul(&prime1,&prime2,&r);
   printf("  Product contains %ld bits\n",zlog(&r));

   printf("Enter cipher number: ");
   fzread(instream,&cipnum);

   msqrt(&cipnum,&prime1,&prime2,decipnum);

   printf("The 4 square roots are:\n");
   for(j=0; j<4; j++) {
      zwrite(&decipnum[j]);
      putchar('\n');
   }
  } else if(resp == 2) {
      /* Test the "encryption" of a large integer.
       */
      printf("Enter prime1: ");
      fzread(instream,&prime1);
      printf("Enter prime2: ");
      fzread(instream,&prime2);
      zmul(&prime1,&prime2,&r);
      printf("  Product contains %ld bits\n",zlog(&r));

      printf("Enter numeric message: ");
      fzread(instream,&msg);
      zmulmod(&msg,&msg,&r,&cipnum);
      printf("Enciphered message is: \n");
      zwrite(&cipnum);
      putchar('\n');
  } else if(resp == 3) {
#if 1
      printf("Williams test disabled; see source.\n");
#else
      /* Test some thoughts I had about using Williams' ideas
       * to distinguish the correct square root.
       * I decided not to use it, but I'm leaving the code
       * here anyway.
       */
      printf("Enter prime1: ");
      fzread(instream,&prime1);
      printf("Enter prime2: ");
      fzread(instream,&prime2);
      zmul(&prime1,&prime2,&r);

      printf("Enter numeric message: ");
      fzread(instream,&planum);

      WilliamsE(&planum,&r,&e1);
      printf("E1(message) = "); zwrite(&e1); putchar('\n');

      RabinEncipherBignum(&e1,&r,&cipnum);
      printf("Rabin(E1) = "); zwrite(&cipnum); putchar('\n');

      RabinDecipherBignum(&cipnum,&prime1,&prime2,decipnum);
      printf("The 4 square roots are:\n");
      for(j=0; j<4; j++) {
         zwrite(&decipnum[j]);
         putchar('\n');
      }

      for(j=0; j<4; j++) {
         WilliamsD(&decipnum[j],&r,&d1);
         printf("D1[decipnum[%d] = ",j);
         zwrite(&d1);  putchar('\n');
      }
#endif
  } else if(resp == 4) {
      /* Choose a prime number.
       */
      printf("Enter nbits: ");
      fscanf(instream,"%ld",&nbits);

      nbytes1 = MAXBYTES; nbytes2 = MAXBYTES;
      printf("\nInput a random string: ");
      GetUserInput(bytes,&nbytes1,bytes2,&nbytes2);
      memcpy(bytes3,bytes,nbytes1);
      memcpy(bytes3+nbytes1,bytes2,nbytes2);
      ChoosePrime(nbits,bytes3,nbytes1+nbytes2,&prime1);
      zwrite(&prime1);  putchar('\n');
  } else if(resp == 5) {
      /* Do a test encryption and decryption of a byte string.
       * You must first do a 1, 2, or 3 to set the primes.
       */
      if(!prime1.len) {
         printf("Need to enter primes first!\n");
      } else {
      printf("Input bytes: ");
      fgets(bytes,MAXBYTES,instream);

      nbytes = strlen(bytes) - 1;

      nbytescip = RabinEncipherBuf(bytes,nbytes,&r,bytes2);
#if 1
      printf("Ciphertext (%d bytes): ",nbytescip);
      for(j=0; j<nbytescip; j++) putchar(bytes2[j]);
      putchar('\n');
#endif

      nbytesplain = RabinDecipherBuf(bytes2,nbytescip,&prime1,&prime2,bytes3);
      printf("Deciphered %d bytes: ",nbytesplain);
      for(j=0; j<nbytesplain; j++) putchar(bytes3[j]);
      putchar('\n');

#if 0
      BytesToBignum(bytes,nbytes,&planum);
      printf("planum = ");
      zwrite(&planum);  putchar('\n');
      printf("planum ASCII = "); put_bignum(&planum); putchar('\n');

      BignumToBytes(&planum,nbytes+9,bytes2);
      printf("Decoded: \"");
      for(j=0; j<nbytes; j++) putchar(bytes2[j]);
      putchar('"');
      for(; j<nbytes+9; j++) putchar(bytes2[j]);
      result = WhetherGoodMsg(bytes2,nbytes+9);
      printf("\nWhetherGoodMsg returns %d\n",result);
#endif
      }
  }
 } while (!quit);
   putchar('\n');
   return(0);
}


void
put_bignum(a)
BIGINT *a;
{
   MYINT j, this;
   int k;
   unsigned char ch;

   for(j=0; j<a->len; j++) {
      this = a->val[j];
      for(k=0; k<4; k++) {
         ch = (unsigned char)(this%256);
         this /= 256;
         putchar(ch);
      }
   }
}

void
display_time(begtime,endtime,msg)
TYPTIME begtime,endtime;
char *msg;
{
#ifdef USEFLOAT
   double elapsed;
#else
#ifdef MSDOS
   double elapsed;
#else
   long int elapsed;
#endif
#endif

#ifdef MSDOS
   elapsed = (double) (endtime - begtime) / (double) CLOCKS_PER_SEC;
#else
   long int begmic, endmic;

   begmic = 1000000*begtime.ru_utime.tv_sec + begtime.ru_utime.tv_usec;
   endmic = 1000000*endtime.ru_utime.tv_sec + endtime.ru_utime.tv_usec;
#ifdef USEFLOAT
   elapsed = ((double)(endmic - begmic)) / 1000000.0;
#else
   elapsed = (endmic - begmic) / 1000;
#endif
#endif

#ifdef MSDOS
   printf("\nElapsed time for %s: %6.2lf sec\n",msg,elapsed);
#else
#ifdef USEFLOAT
   printf("\nElapsed time for %s: %6.2lf sec\n",msg,elapsed);
#else
   printf("\nElapsed time for %s: %6d ms\n",msg,elapsed);
#endif
#endif

}

void
my_get_time(mytime)
TYPTIME *mytime;
{
#ifdef MSDOS
   *mytime = clock();
#else
#define RUSAGE_SELF 0

   getrusage(RUSAGE_SELF,mytime);
#endif
}
