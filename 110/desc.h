/* desc.h */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "des.h"

#define MAX_BLOCKS 32
#define MAX_RUNS 128
#define OUTPUT stdout
#define NUMKEYS 268435456

int checkkey(des_key_schedule, unsigned char *cipher, unsigned char *iv,
				unsigned char *known, int len);
float testkey(unsigned char key[8], unsigned char *cipher, unsigned char *iv,
				unsigned char *known, int len);
void dofullmessage(unsigned char key[8], unsigned char *cipher,unsigned
char *iv,int len);
void dofinalmessage(des_key_schedule ks, unsigned char *cipher,unsigned
char *iv,int len);

void des_cbc_encrypt8(des_cblock (*input), des_cblock
(*output),des_key_schedule schedule, des_cblock (*ivec));

void crack_key0(des_cblock (*input), des_cblock (*output), des_cblock (*ivec), DES_LONG tin[2], DES_LONG tcmp[2]);
void crack_key15(des_key_schedule schedule,DES_LONG tin[2], DES_LONG tou[2]);
void crack_key16(des_key_schedule schedule,DES_LONG tin[2], DES_LONG tou[2], DES_LONG load[2]);
int crack_key25(DES_LONG tin[2], DES_LONG tcmp, des_key_schedule schedule);
void crack_key1(des_key_schedule schedule,DES_LONG tin[2], DES_LONG tou[2]);
int crack_key2(DES_LONG tin[2], DES_LONG tcmp[2], des_key_schedule schedule);

int des_search(char *a_key);
int crypt_perf(void);

