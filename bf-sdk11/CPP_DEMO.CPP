

//      CPP_DEMO.CPP
//      test program for BFENG.OBJ
//      programming language: MS Visual C++ 1.0, QuickWin
//      memory model: large
//      last update: 05/25/96
//      (c)1996 Markus Hahn



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "blowfish.h"



// official testvectors from DDJ 10/95...

unsigned char key1[] = "abcdefghijklmnopqrstuvwxyz";
unsigned long testdata1_p[2] = {0x424c4f57, 0x46495348};
unsigned long testdata1_c[2] = {0x324ed0fe, 0xf413a203};

unsigned char key2[] = "Who is John Galt?";
unsigned long testdata2_p[2] = {0xfedcba98, 0x76543210};
unsigned long testdata2_c[2] = {0xcc91732b, 0x8022f684};


#define  BIGBUFSIZE 32000
#define  TESTLOOPS  400UL




//  random32() - creates 32bit random value, don't use this in real crypt apps
unsigned long random32()
{
  return ((unsigned long)rand() << 16) | (unsigned long)rand();
};


  

// global data... 

  unsigned long ulCBCIVL;  // IV == initalisation vector 
  unsigned long ulCBCIVR;   
  unsigned long ulCBCLeft;  
  unsigned long ulCBCRight; 
  unsigned char sMyKey[256];
  unsigned long testbuf[6];
  unsigned int 	unI;
  unsigned long ulStart;
  unsigned long ulTime;
  unsigned long ulRate;
  unsigned long *pBoxes;
  unsigned long BF_key[1058];
  unsigned char biggy[BIGBUFSIZE];
  clock_t       needed_time;

      

int main()
{

     // init. the random generator
     srand((unsigned)time(NULL));
           
      
     // save original boxes
     Blowfish_GetBoxes(BF_key);   
     Blowfish_SetRounds(16);
     
     
     // en- and decrypt the test vectors... 

     // #1 
     puts("test vector #1...");
     Blowfish_Init(key1, sizeof(key1)-1);
     // show the first 6 p-boxes
     pBoxes=(unsigned long*)Blowfish_GetBoxPointer;
     puts("pboxes:");
     for (unI=0; unI<5; unI++) printf("%08lx ", pBoxes[unI]);
     puts("");
     // now encrypt, decrypt and compare with the official values
     printf("plaintext : %08lx %08lx\n", testdata1_p[0], testdata1_p[1]);
     Blowfish_ECBEncrypt(testdata1_p, 8);                             
     printf("ciphertext: %08lx %08lx\n", testdata1_p[0], testdata1_p[1]);
     printf("DDJ 10/95 : %08lx %08lx\n", testdata1_c[0], testdata1_c[1]);
     Blowfish_ECBDecrypt(testdata1_p, 8);
     printf("decrypted : %08lx %08lx\n", testdata1_p[0], testdata1_p[1]);
     puts("");
     
     // #2 
     puts("test vector #2...");
     Blowfish_SetBoxes(BF_key);       // reload original boxes
     Blowfish_Init(key2, sizeof(key2)-1);
     pBoxes=(unsigned long*)Blowfish_GetBoxPointer;
     puts("pboxes:");
     for (unI=0; unI<5; unI++) printf("%08lx ", pBoxes[unI]);
     puts("");
     printf("plaintext : %08lx %08lx\n", testdata2_p[0], testdata2_p[1]);
     Blowfish_ECBEncrypt(testdata2_p, 8);                             
     printf("ciphertext: %08lx %08lx\n", testdata2_p[0], testdata2_p[1]);
     printf("DDJ 10/95 : %08lx %08lx\n", testdata2_c[0], testdata2_c[1]);
     Blowfish_ECBDecrypt(testdata2_p, 8);
     printf("decrypted : %08lx %08lx\n", testdata2_p[0], testdata2_p[1]);
     puts("press ENTER...\n");
     getchar();
     
     
     // start ECB tests with a small buffer... 
     
     // 16 rounds... 
     Blowfish_SetBoxes(BF_key); // reload original boxes 
     strcpy((char*) sMyKey, "I think I climb aboard!");
     Blowfish_Init((unsigned char*) sMyKey, strlen((char*) sMyKey));
     puts("ECB test 16 round...");
     puts("original data:");
     // generate and show random numbers...
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]=random32());
     puts("\nencrypted data:");
     Blowfish_ECBEncrypt(testbuf, 6*4);
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\ndecrypted data:");
     Blowfish_ECBDecrypt(testbuf, 6*4);
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\n");
     
     // 32 rounds... 
     Blowfish_SetRounds(32);	// change number of encryption rounds		
     Blowfish_SetBoxes(BF_key);	
     strcpy((char*) sMyKey, "I think I climb aboard!");
     Blowfish_Init((unsigned char*) sMyKey, strlen((char*) sMyKey));
     puts("ECB test 32 round...");
     puts("original data:");
     // (to show the difference we do NOT create new test data)
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\nencrypted data:");
     Blowfish_ECBEncrypt(testbuf, 6*4);
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\ndecrypted data:");
     Blowfish_ECBDecrypt(testbuf, 6*4);
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\npress ENTER...\n");
     getchar();
     

     // start CBC tests with a small buffer... 
     
     // 16 rounds... 
     ulCBCLeft=random32();	// init. and save the CBC IV
     ulCBCRight=random32();
     ulCBCIVL=ulCBCLeft;	
     ulCBCIVR=ulCBCRight;
     Blowfish_SetRounds(16);	// reset to 16 encryption rounds
     Blowfish_SetBoxes(BF_key); // reload original boxes 
     strcpy((char*) sMyKey, "Who wants some?");
     Blowfish_Init((unsigned char*) sMyKey, strlen((char*) sMyKey));
     puts("CBC test 16 round...");
     printf("IV: %08lx %08lx\n", ulCBCLeft, ulCBCRight);
     puts("original data:");
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\nencrypted data:");
     Blowfish_CBCEncrypt(testbuf, 6*4, &ulCBCIVL, &ulCBCIVR);
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\ndecrypted data:");                          
     // restore the original CBC IV for decryption
     ulCBCIVL=ulCBCLeft;	
     ulCBCIVR=ulCBCRight;
     Blowfish_CBCDecrypt(testbuf, 6*4, &ulCBCIVL, &ulCBCIVR);
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\n");
     
     // 32 rounds... 
     ulCBCIVL=ulCBCLeft;	// use the same CBC IV as above
     ulCBCIVR=ulCBCRight;
     Blowfish_SetRounds(32);	// set to 32 encryption rounds
     Blowfish_SetBoxes(BF_key); // reload original boxes 
     strcpy((char*) sMyKey, "Who wants some?");
     Blowfish_Init((unsigned char*) sMyKey, strlen((char*) sMyKey));
     puts("CBC test 32 round...");
     printf("IV: %08lx %08lx\n", ulCBCLeft, ulCBCRight);
     puts("original data:");
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\nencrypted data:");
     Blowfish_CBCEncrypt(testbuf, 6*4, &ulCBCIVL, &ulCBCIVR);
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\ndecrypted data:");                          
     ulCBCIVL=ulCBCLeft;	
     ulCBCIVR=ulCBCRight;
     Blowfish_CBCDecrypt(testbuf, 6*4, &ulCBCIVL, &ulCBCIVR);
     for (unI=0; unI<6; unI++) printf("%08lx  ", testbuf[unI]);
     puts("\npress ENTER...\n");
     getchar();


     // benchmark tests, done in CBC mode with a 32kB buffer
     puts("benchmark test\n16 rounds running...");
     Blowfish_SetRounds(16);
     needed_time=clock();
     for (unI=0; unI<TESTLOOPS; unI++) 
       Blowfish_CBCEncrypt((unsigned long*) biggy, BIGBUFSIZE, &ulCBCIVL, &ulCBCIVL);
     needed_time=clock()-needed_time;
     ulRate=( (TESTLOOPS * BIGBUFSIZE) / needed_time) * CLK_TCK;
     printf("%ld bytes per second\n", ulRate);
     puts("32 rounds running...");
     Blowfish_SetRounds(32);
     needed_time=clock();
     for (unI=0; unI<TESTLOOPS; unI++) 
       Blowfish_CBCEncrypt((unsigned long*) biggy, BIGBUFSIZE, &ulCBCIVL, &ulCBCIVL);
     needed_time=clock()-needed_time;
     ulRate=( (TESTLOOPS * BIGBUFSIZE) / needed_time) * CLK_TCK;
     printf("%ld bytes per second\n", ulRate);

                     
     return getchar();
};






