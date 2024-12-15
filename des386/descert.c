#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "des.h"

static void get8 (unsigned char *);
static void put8 (unsigned char *);

main()
{
	unsigned char key[8],plain[8],cipher[8],work[8];
	int test;
	int fail;
	static unsigned long kne[16][2],knd[16][2];

	for(test=0;!feof(stdin);test++){
		fail = 0;

		get8(key);
		printf(" K: "); put8(key);
		deskey(kne,key,0);
		deskey(knd,key,1);

		get8(plain);
		printf(" P: "); put8(plain);

		get8(cipher);
		printf(" C: "); put8(cipher);

		memcpy(work,plain,8);
		des(kne,work);

		if(memcmp(work,cipher,8) !=0){
			printf(" Encrypt FAIL");
			printf(" c: "); put8(work);
			fail++;
		}
		des(knd,work);
		if(memcmp(work,plain,8) != 0){
			printf(" Decrypt FAIL");
			printf(" p: "); put8(work);
			fail++;
		}
		if(fail == 0)
			printf(" OK");
		printf("\n");
	}
	return 0;
}
static void
get8(cp)
unsigned char *cp;
{
	int i,t;

	for(i=0;i<8;i++){
		scanf("%2x",&t);
		if(feof(stdin))
			exit(0);
		*cp++ = t;
	}
}
static void
put8(cp)
unsigned char *cp;
{
	int i;

	for(i=0;i<8;i++){
		printf("%02x",*cp++ & 0xff);
	}
}
