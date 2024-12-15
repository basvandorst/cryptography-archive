/* Investigate cycles in DES output feedback mode (experimental)
 * Phil Karn
 */
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "des.h"

void
get8(unsigned char *cp)
{
	int i,t;

	for(i=0;i<8;i++){
		scanf("%2x",&t);
		*cp++ = t;
	}
}
void
put8(unsigned char *cp)
{
	int i;

	for(i=0;i<8;i++){
		printf("%2x ",*cp++ & 0xff);
	}
}

unsigned char key[8] = {
	0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef };
unsigned char start[8] = {
	0,0,0,0,0,0,0,0 };
unsigned char work[8];
unsigned long ks[16][2];

main()
{
	long update;
	long iter;
	long i;

	printf("Setting key: "); put8(key); printf("\n");
	deskey(ks,key,0);
	printf("Starting value: "); put8(start); printf("\n");
	update = 10000;
	printf("Update interval: %ld\n",update);
	iter = 0;

	memcpy(work,start,8);

	for(iter = 0;; iter++){
		for(i=0;i<update;i++)
			des(ks,work);
		printf("%ld ",iter);
		put8(work); printf("\n");
	}
}
