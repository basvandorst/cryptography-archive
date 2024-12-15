/*
 * timing program for des486.asm
 */

#define NUMBLOCKS 50000
#include <stdio.h>
#include <time.h>
#include "des.h"

char password[8]=
 {
   1,2,3,4,5,6,7,8
  };
int main()
 {
   int i,j;
   unsigned long l;
   unsigned long m;
   clock_t start,end;
   float f;

   if(cpu()<486)
    {
      printf("Not a 486 cpu\n");
      return(1);
     }
   schedule(password,keytbl);
   for(i=0;i<8;++i)
   block[i]=0x55;

   printf("DES encrypting %ld blocks: ",NUMBLOCKS);
   start=clock();
   ++start;
   while(start!=clock())        /* sync the timer */
        ;
   for(l=0;l<NUMBLOCKS;++l)
    encrypt_block();
   end=clock()-start;
   l=end*1000. /CLOCKS_PER_SEC+1;
   i=l/1000;
   j=l%1000;
   printf("(%d.%03d sec.)    ",i,j);
   m=(long)NUMBLOCKS*1000;
   f=m/l;
   printf("%g blocks/sec  %g bytes/sec\n",f,f*8);

   printf("DES decrypting %ld blocks: ",NUMBLOCKS);
   start=clock();
   ++start;
   while(start!=clock())        /* sync the timer */
        ;
   for(l=0;l<NUMBLOCKS;++l)
    decrypt_block();
   end=clock()-start;
   l=end*1000. /CLOCKS_PER_SEC+1;
   i=l/1000;
   j=l%1000;
   printf("(%d.%03d sec.)    ",i,j);
   m=(long)NUMBLOCKS*1000;
   f=m/l;
   printf("%g blocks/sec  %g bytes/sec\n",f,f*8);
   for(i=0;i<8;++i)
    if(block[i]!=0x55)
     {
       printf("Expected 0x55 in block[%d]-- got %02Xd\n",i,block[i]);
      }
   return(0);
  }