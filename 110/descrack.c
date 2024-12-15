/* start of descrack.cpp */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "arch.h"
#include "des.h"
#include "desc.h"

#ifdef TEST
#define x_asciitext t_asciitext
#define x_iv t_iv
#define x_knowntext t_knowntext
#define x_ciphertext t_ciphertext
#define x_knowntextc t_knowntextc
#define x_ciphertextc t_ciphertextc

unsigned char t_keyc[256] ={0xa1, 0x26, 0xdf, 0xb0, 0x13, 0x1f, 0x46, 0x98};
#else

unsigned char x_contest_id[] = "DES";
unsigned char x_cipher_name[] = "DES";
unsigned char x_con_state[] = "ongoing";
unsigned char x_asciitext[] = "The unknown message is:";
unsigned char x_key[8];
unsigned char x_iv[8] =	{0x99 ,0xe9 ,0x7c ,0xbf ,0x4f ,0x7a ,0x6e ,0x8f};
unsigned char x_knowntext[256]="The unknown mess";
unsigned char x_ciphertext[256] ={
 0x79 ,0x45 ,0x81 ,0xc0 ,0xa0 ,0x6e ,0x40 ,0xa2 ,0x0b ,0xe3 ,0x33 ,0xc6, 0x5c ,0x93 ,0xb7 ,0x22,
 0xaa ,0xc2 ,0x61 ,0x27 ,0xff ,0x72 ,0xac ,0xc6 ,0x03 ,0x68 ,0x60 ,0x2b, 0x43 ,0x99 ,0x3c ,0x31,
 0x9f ,0x36 ,0xe6 ,0xae ,0x82 ,0x0c ,0x5d ,0x3f ,0xa2 ,0x3b ,0xb4 ,0x91, 0xe5 ,0xef ,0xee ,0xa3,
 0xb9 ,0x92 ,0x78 ,0xd4 ,0xaf ,0x0f ,0x66 ,0xaa ,0xf5 ,0x17 ,0xe8 ,0x64, 0x08 ,0x88 ,0x82 ,0x8d,
 0x67 ,0xba ,0xfe ,0xbf ,0x78 ,0x95 ,0x60 ,0x95 ,0x31 ,0x6a ,0xe2 ,0x98, 0x78 ,0x1c ,0x6a ,0xf7
};

unsigned char x_ciphertextc[256] ={
 0x86, 0xba, 0x7e, 0x3f, 0x5f, 0x91, 0xbf, 0x5d, 0xf4, 0x1c, 0xcc, 0x39, 0xa3, 0x6c, 0x48, 0xdd, 
 0x55, 0x3d, 0x9e, 0xd8, 0x00, 0x8d, 0x53, 0x39, 0xfc, 0x97, 0x9f, 0xd4, 0xbc, 0x66, 0xc3, 0xce, 
 0x60, 0xc9, 0x19, 0x51, 0x7d, 0xf3, 0xa2, 0xc0, 0x5d, 0xc4, 0x4b, 0x6e, 0x1a, 0x10, 0x11, 0x5c, 
 0x46, 0x6d, 0x87, 0x2b, 0x50, 0xf0, 0x99, 0x55, 0x0a, 0xe8, 0x17, 0x9b, 0xf7, 0x77, 0x7d, 0x72, 
 0x98, 0x45, 0x01, 0x40, 0x87, 0x6a, 0x9f, 0x6a, 0xce, 0x95, 0x1d, 0x67, 0x87, 0xe3, 0x95, 0x08};

unsigned char x_knowntextc[256] ={0xab, 0x97, 0x9a, 0xdf, 0x8a, 0x91, 0x94, 0x91};
#endif

const int x_knowntext_len = 8;
const int x_ciphertext_len = 80;

unsigned char t_contest_id[] = "DES";
unsigned char t_cipher_name[] = "DES";
unsigned char t_con_state[] = "practice";
unsigned char t_asciitext[] = {
 0x54, 0x68, 0x65, 0x20, 0x75, 0x6e, 0x6b, 0x6e, 0x6f, 0x77, 0x6e, 0x20, 0x6d, 0x65, 0x73, 0x73,
 0x61, 0x67, 0x65, 0x20, 0x69, 0x73, 0x3a, 0x20, 0x54, 0x68, 0x65, 0x20, 0x44, 0x45, 0x53, 0x2d,
 0x74, 0x65, 0x73, 0x74, 0x20, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x73, 0x74, 0x27, 0x73, 0x20, 0x70,
 0x6c, 0x61, 0x69, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};
unsigned char t_key[8] = {0x5e, 0xd9, 0x20, 0x4f, 0xec, 0xe0, 0xb9, 0x67};
unsigned char t_iv[8] =	{0xa2, 0x18, 0x5a, 0xbf, 0x45, 0x96, 0x60, 0xbf};

unsigned char t_knowntext[256]={
 0x54, 0x68, 0x65, 0x20, 0x75, 0x6e, 0x6b, 0x6e,
 0x6f, 0x77, 0x6e, 0x20, 0x6d, 0x65, 0x73, 0x73
};

unsigned char t_ciphertext[256] = {   
 0x3e, 0xa7, 0x86, 0xf9, 0x1d, 0x76, 0xbb, 0xd3, 0x66, 0xc6, 0x3f, 0x54, 0xeb, 0x3f, 0xe3, 0x3f,
 0x39, 0x88, 0x81, 0x4c, 0x8b, 0xa1, 0x97, 0xf7, 0xbe, 0x1b, 0xdd, 0x7e, 0xfb, 0x39, 0x96, 0x31,
 0x3c, 0x3d, 0x3b, 0x65, 0xc8, 0xb8, 0x3e, 0x31, 0x89, 0xf9, 0x04, 0x14, 0xfb, 0xcd, 0xc3, 0x70,
 0xc1, 0x11, 0xa5, 0x2f, 0x3a, 0xef, 0x80, 0xf4, 0xcf, 0xf5, 0x43, 0xa4, 0xb1, 0x65, 0x5b, 0xae
};

unsigned char t_knowntextc[256] ={
 0xab, 0x97, 0x9a, 0xdf, 0x8a, 0x91, 0x94, 0x91, 
 0x90, 0x88, 0x91, 0xdf, 0x92, 0x9a, 0x8c, 0x8c
};

unsigned char t_ciphertextc[256] = {
 0xc1, 0x58, 0x79, 0x06, 0xe2, 0x89, 0x44, 0x2c, 0x99, 0x39, 0xc0, 0xab, 0x14, 0xc0, 0x1c, 0xc0, 
 0xc6, 0x77, 0x7e, 0xb3, 0x74, 0x5e, 0x68, 0x08, 0x41, 0xe4, 0x22, 0x81, 0x04, 0xc6, 0x69, 0xce, 
 0xc3, 0xc2, 0xc4, 0x9a, 0x37, 0x47, 0xc1, 0xce, 0x76, 0x06, 0xfb, 0xeb, 0x04, 0x32, 0x3c, 0x8f, 
 0x3e, 0xee, 0x5a, 0xd0, 0xc5, 0x10, 0x7f, 0x0b, 0x30, 0x0a, 0xbc, 0x5b, 0x4e, 0x9a, 0xa4, 0x51, 
 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const int t_knowntext_len = 8;
const int t_ciphertext_len = 64;
int countones[256];

extern int verboselevel;

int keytwiddles1[40]= {
	42,45,49,51,
	1,3,4,5,6,
	8,9,10,11,12,
	16,17,20,
	24,25,28,29,30,
	32,33,34,35,36,37,38,
	40,41,43,44,46,
	48,52,54,
	56,60,62
};

int firstbits[8]={ 13,18,19,50,53,59,14,57 };
int secondbits[8]={ 0,2,21,22,26,27,58,61 };

int gray8[256];
unsigned int bit8[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

#ifdef USE_ASM
void sched_update(unsigned long *, unsigned long *, unsigned long);
#define UPDATE_KEYSCHEDULE_1    sched_update(lp,lp2,gray8[jj]);
#else
#define UPDATE_KEYSCHEDULE_1	if (!gray8[jj]) { \
	  /* We're on bit 0. This one flips often, so unroll it */ \
	  lp[1]^=lp2[1];\
	  lp[2]^=lp2[2];\
	  lp[4]^=lp2[4];\
	  lp[6]^=lp2[6];\
	  lp[11]^=lp2[11];\
	  lp[12]^=lp2[12];\
	  lp[15]^=lp2[15];\
	  lp[16]^=lp2[16];\
	  lp[20]^=lp2[20];\
	  lp[23]^=lp2[23];\
	  lp[25]^=lp2[25];\
	  lp[27]^=lp2[27];\
	  lp[28]^=lp2[28];\
	} \
	else if (gray8[jj] == 1) {\
	  lp[1]^=lp2[1];\
	  lp[3]^=lp2[3];\
	  lp[7]^=lp2[7];\
	  lp[8]^=lp2[8];\
	  lp[10]^=lp2[10];\
	  lp[13]^=lp2[13];\
	  lp[14]^=lp2[14];\
	  lp[17]^=lp2[17];\
	  lp[18]^=lp2[18];\
	  lp[21]^=lp2[21];\
	  lp[22]^=lp2[22];\
	  lp[25]^=lp2[25];\
	  lp[28]^=lp2[28];\
	}\
	else if (gray8[jj] == 2) {\
	  lp[0]^=lp2[0];\
	  lp[2]^=lp2[2];\
	  lp[5]^=lp2[5];\
	  lp[7]^=lp2[7];\
	  lp[9]^=lp2[9];\
	  lp[10]^=lp2[10];\
	  lp[13]^=lp2[13];\
	  lp[15]^=lp2[15];\
	  lp[16]^=lp2[16];\
	  lp[18]^=lp2[18];\
	  lp[23]^=lp2[23];\
	  lp[24]^=lp2[24];\
	  lp[27]^=lp2[27];\
	  lp[28]^=lp2[28];\
	}\
	else /* 3,4,5 */\
	  for(xx=28;xx>=0;xx--)\
	    lp[xx]^=lp2[xx];
#endif

void set_gray8() {
	unsigned int x,y,g,gd,go;
	go = 0;
	for (x =0; x<256; x++) {
		g = x ^ (x >>1);
		gd = g ^ go;
		go = g;
		for (y = 0; y<8; y++)
			if ((1<<y) == gd) gray8[x] = y;
	}
	gray8[0]=7;
}

#define ACCURACY 256*256*256

int test_crypt(void) {

  int test1=0;
  int test2=0;
  int test;
  int jj,kk,ii,xx,nn;
  unsigned char wild[8];
  des_key_schedule ks,xks[64];
  DES_LONG tin[2],tint[2],ttst[2],tcmp[2];
  UINT4 *lp,*lp2;

  set_gray8();
  for (ii=0;ii<256;ii++) {
      test=ii;
      countones[ii]=0;
      for(jj=0;jj<8;jj++) {
	if (test & 0x1) countones[ii]++;
	test >>= 1;
      }
      /* printf("%d ",gray8[ii]); */
    }
  des_set_odd_parity((des_cblock *)t_key);
  des_set_key((des_cblock *)t_key,ks);
  crack_key0((des_cblock *)t_ciphertext,(des_cblock *)t_knowntext,(des_cblock *)t_iv,tin,tcmp);
  /*
  crack_key1(ks,tin,tint);
  crack_key15(ks,tint,ttst);
  */
  crack_key16(ks,tin,ttst,tint);
  if (crack_key25(ttst,tcmp[0],ks))
    test1 = 1;
  else {
    printf("not working\n");
    return -1;
  }
  /* Now test with a known-wrong key */
  t_key[0] = 0x00;
  des_set_odd_parity((des_cblock *)t_key);
  des_set_key((des_cblock *)t_key,ks);
  crack_key0((des_cblock *)t_ciphertext,(des_cblock *)t_knowntext,(des_cblock *)t_iv,tin,tcmp);
  crack_key16(ks,tin,ttst,tint);
  if (crack_key25(ttst,tcmp[0],ks)) {
    printf("not working 2!\n");
    return -1;
  }

  t_key[0] = 0x5e;

  for (jj=0;jj<8;jj++)
    wild[jj]=0;
  for(jj=0;jj<64;jj++) {
    wild[jj/8]= 1<< (7-(jj % 8));
    des_set_odd_parity((des_cblock *)wild);
    des_set_key((des_cblock *)wild,xks[jj]);
    lp2= (UINT4 *) xks[jj];
    wild[jj/8]=0;
  }
  
  lp= (UINT4 *) ks;

  /* load correct test key */
  for(jj=0;jj<32;jj++)
    lp[jj]=0;
  for (jj=0;jj<64;jj++) {
    if (t_key[jj/8] & (1 << (7-(jj%8)))) {
      lp2=(UINT4 *) xks[jj];
      for (kk=0;kk<32;kk++)
	lp[kk] |= lp2[kk];
    }
  }
#if 1
  for(nn=256;nn>=0;nn--) {  /* scrable correct key */
    test=rand() % 8;
    lp2=(UINT4 *) xks[firstbits[test]];
    for (kk=0;kk<32;kk++)
      lp[kk] |= lp2[kk];
    test=rand() % 8;
    lp2=(UINT4 *) xks[secondbits[test]];
    for (kk=0;kk<32;kk++)
      lp[kk] |= lp2[kk];
  }
  for(kk=255;kk>=0;kk--) {
    /*
      crack_key1(ks,tin,tint);
      crack_key15(ks,tint,ttst);
      */
    crack_key16(ks,tin,ttst,tint);
    for(jj=255;jj>=0;jj--) {
      if (crack_key25(ttst,tcmp[0],ks)) {
	test2 = 1;
	jj = 0;
	kk = 0;
	break;
      }
      test=firstbits[gray8[jj]];
      lp2=(UINT4 *) xks[test];
      if (gray8[jj]>=6) {
	for (xx=29;xx>=0;xx--) /* can skip key schedule update for first round */
	  lp[xx] ^= lp2[xx];
	crack_key15(ks,tint,ttst);
      }
      else {
	UPDATE_KEYSCHEDULE_1
      }
    }
    test=secondbits[gray8[kk]];
    lp2=(UINT4 *) xks[test];
    lp[31] ^= lp2[31];
    lp[30] ^= lp2[30];
    for (xx=27;xx>=0;xx--)  /* second bits aren't used in second round */
      lp[xx] ^= lp2[xx];
  }
  if(test1 == 1 && test2 == 1)
    return 0;
  else
    return -1;
}

int des_search(char *a_key) {
  int test;
  int jj,kk,ii,xx,mm,nn;
  unsigned char wild[8];
  des_key_schedule ks,xks[64];
  DES_LONG tin[2],tint[2],ttst[2],tcmp[2];
  DES_LONG tinc[2],tintc[2],ttstc[2],tcmpc[2];
  UINT4 *lp,*lp2;

  set_gray8();
  for (ii=0;ii<256;ii++) {
      test=ii;
      countones[ii]=0;
      for(jj=0;jj<8;jj++) {
	if (test & 0x1) countones[ii]++;
	test >>= 1;
      }
  }

  des_set_odd_parity((des_cblock *)a_key);;
  des_set_key((des_cblock *)a_key,ks);

  for (jj=0;jj<8;jj++)
    wild[jj]=0;
  for(jj=0;jj<64;jj++) {
    wild[jj/8]= 1<< (7-(jj % 8));
    des_set_odd_parity((des_cblock *)wild);
    des_set_key((des_cblock *)wild,xks[jj]);
    lp2= (UINT4 *) xks[jj];
    wild[jj/8]=0;
  }

  lp= (UINT4 *) ks;
  for(jj=0;jj<32;jj++)
    lp[jj]=0;
  for (jj=0;jj<64;jj++)
    {
      if (a_key[jj/8] & (1 << (7-(jj%8))))
  	{
	  lp2=(UINT4 *) xks[jj];
	  for (kk=0;kk<32;kk++)
	    lp[kk] |= lp2[kk];
  	}
    }

#ifdef DEBUG
  printf("\ntesting %02x %02x %02x %02x %02x %02x %02x %02x\n",a_key[0],a_key[1],a_key[2],a_key[3],a_key[4],a_key[5],a_key[6],a_key[7]);
#endif

  crack_key0((des_cblock *)x_ciphertext,(des_cblock *)x_knowntext,(des_cblock *)x_iv,tin,tcmp);
  /* and once again for the complement */
  crack_key0((des_cblock *)x_ciphertextc,(des_cblock *)x_knowntextc,(des_cblock *)x_iv,tinc,tcmpc);
  for(mm=15;mm>=0;mm--) {
    for(nn=255;nn>=0;nn--) {
      for(kk=255;kk>=0;kk--) {
	crack_key16(ks,tin,ttst,tint);
	/* complement */ 
	crack_key16(ks,tinc,ttstc,tintc);
	
	for(jj=255;jj>=0;jj--) {
	  if (crack_key25(ttst,tcmp[0],ks)) {
	    unsigned char result[256];
	    des_cbc_encrypt((des_cblock *)x_ciphertext,(des_cblock *)result,x_ciphertext_len,ks,(des_cblock *)x_iv,0);
	    if (!memcmp(result,x_knowntext,8))
	      return 1;
	  }
	  
	  /* do the complement too... */
	  if (crack_key25(ttstc,tcmpc[0],ks)) {
	    unsigned char result[256];
	    des_cbc_encrypt((des_cblock *)x_ciphertextc,(des_cblock *)result,x_ciphertext_len,ks,(des_cblock *)x_iv,0);
	    if (!memcmp(result,x_knowntextc,8))
	      return 1;
	  }
	  
	  test=firstbits[gray8[jj]];
	  lp2=(UINT4 *) xks[test];
	  if (gray8[jj]>=6) {
	    for (xx=29;xx>=0;xx--) /* can skip key schedule update for first round */
	      lp[xx] ^= lp2[xx];
	    crack_key15(ks,tint,ttst);
	    crack_key15(ks,tintc,ttstc); /* Complement */
	  }
	  else {
	    UPDATE_KEYSCHEDULE_1
	  }	  
	} /* for jj */
	test=secondbits[gray8[kk]];
	lp2=(UINT4 *) xks[test];
	lp[31] ^= lp2[31];
	lp[30] ^= lp2[30];
	for (xx=27;xx>=0;xx--)  /* second bits aren't used in second round */
	  lp[xx] ^= lp2[xx];
      } /* for kk */
      test=keytwiddles1[gray8[nn]];
      lp2=(UINT4 *) xks[test];
      for (xx=31;xx>=0;xx--)
	lp[xx] ^= lp2[xx];
    } /* for nn , 16M keys */
    test=keytwiddles1[gray8[mm]+8];
    lp2=(UINT4 *) xks[test];
    for (xx=31;xx>=0;xx--)
      lp[xx] ^= lp2[xx];
    if ( verboselevel > 2 ) {
      fprintf(OUTPUT,"%.2f%%..",(float)(16-mm)*6.25);
      fflush(OUTPUT);
    }
  } /* for mm */
  return 0;
#endif
}

void dofinalmessage(des_key_schedule ks, unsigned char *cipher,unsigned
char *iv,int len)
{
  int i;
  unsigned char result[256];

   des_cbc_encrypt((des_cblock *)cipher,(des_cblock *) result,len,ks,(des_cblock *)iv,0);
   for(i=0;i<len;i++)
   {
    	/* result[i] &= 0x5F; */
    	if (!result[i]) result[i]=' ';
    	if (result[i] < ' ') result[i]=' ';
    }
    result[len]=0;
    printf("Possible answer:%s\n",result);

}

void dofullmessage(unsigned char key[8], unsigned char *cipher,unsigned
char *iv,int len)
{
  int i;
  unsigned char result[256];
  des_key_schedule ks;

  des_set_odd_parity((des_cblock *)key);
  des_set_key((des_cblock *)key,ks);
  des_cbc_encrypt((des_cblock *)cipher,(des_cblock *) result,len,ks,(des_cblock *)iv,0);
  for(i=0;i<len;i++)
    {
      /* result[i] &= 0x5F; */
      if (!result[i]) result[i]=' ';
      if (result[i] < ' ') result[i]=' ';
    }
  result[len]=0;
  printf("Possible answer:%s\n",result);
}

int crypt_perf(void) {
  int test;
  int jj,kk,ii,nn,xx;
  unsigned char wild[8];
  long ticks;
  des_key_schedule ks,xks[64];
  DES_LONG tin[2],tint[2],ttst[2],tcmp[2];
  DES_LONG tinc[2],tintc[2],ttstc[2],tcmpc[2];
  UINT4 *lp,*lp2;
  char a_key[8];

  /* Zero out the key used for test */
  for (ii=0;ii<8;ii++)
    a_key[ii] = 0x00;

  set_gray8();
  for (ii=0;ii<256;ii++) {
      test=ii;
      countones[ii]=0;
      for(jj=0;jj<8;jj++) {
	if (test & 0x1) countones[ii]++;
	test >>= 1;
      }
  }

  for (jj=0;jj<8;jj++)
    wild[jj]=0;
  for(jj=0;jj<64;jj++) {
    wild[jj/8]= 1<< (7-(jj % 8));
    des_set_odd_parity((des_cblock *)wild);
    des_set_key((des_cblock *)wild,xks[jj]);
    lp2= (UINT4 *) xks[jj];
    wild[jj/8]=0;
  }
  for (kk = 0; kk < 8; kk++) {
    lp2 = (UINT4*)xks[firstbits[kk]];
    nn = 0;
    for (jj = 0; jj < 32; jj++)
       if(lp2[jj])
	 nn++;
    /*    printf("%02u (%02u): %u\n",kk,firstbits[kk],nn);*/
  }

  lp= (UINT4 *) ks;
  for(jj=0;jj<32;jj++)
    lp[jj]=0;
  for (jj=0;jj<64;jj++)
    {
      if (a_key[jj/8] & (1 << (7-(jj%8))))
  	{
	  lp2=(UINT4 *) xks[jj];
	  for (kk=0;kk<32;kk++)
	    lp[kk] |= lp2[kk];
  	}
    }

  des_set_odd_parity((des_cblock *)a_key);;
  des_set_key((des_cblock *)a_key,ks);

  crack_key0((des_cblock *)x_ciphertext,(des_cblock *)x_knowntext,(des_cblock *)x_iv,tin,tcmp);
  /* and once again for the complement */
  crack_key0((des_cblock *)x_ciphertextc,(des_cblock *)x_knowntextc,(des_cblock *)x_iv,tinc,tcmpc);
  ticks=clock();
  for(nn=16;nn>=0;nn--) {
    for(kk=255;kk>=0;kk--) {
      crack_key16(ks,tin,ttst,tint);
      /* complement */ 
      crack_key16(ks,tinc,ttstc,tintc);
      for(jj=255;jj>=0;jj--) {
	if (crack_key25(ttst,tcmp[0],ks)) {
	}
	  
	/* do the complement too... */
	if (crack_key25(ttstc,tcmpc[0],ks)) {
	}
	
	test=firstbits[gray8[jj]];
	lp2=(UINT4 *) xks[test];
	if (gray8[jj]>=6) {
	  for (xx=29;xx>=0;xx--) /* can skip key schedule update for first round */
	    lp[xx] ^= lp2[xx];
	  crack_key15(ks,tint,ttst);
	  crack_key15(ks,tintc,ttstc); /* Complement */
	}
	else {
	  UPDATE_KEYSCHEDULE_1
	}
      } /* for jj */
      test=secondbits[gray8[kk]];
      lp2=(UINT4 *) xks[test];
      lp[31] ^= lp2[31];
      lp[30] ^= lp2[30];
      for (xx=27;xx>=0;xx--)  /* second bits aren't used in second round */
	lp[xx] ^= lp2[xx];
    } /* for kk */
    test=keytwiddles1[gray8[nn]];
    lp2=(UINT4 *) xks[test];
    for (xx=31;xx>=0;xx--)
      lp[xx] ^= lp2[xx];
  } /* for nn */
  ticks-=clock();
  ticks= -ticks;
  
  return (float)2*(((float)1048576*(float)CLOCKS_PER_SEC)/(float)ticks);
}  
  
