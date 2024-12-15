/* Investigate cycles in DES output feedback mode (experimental)
 * Phil Karn
 */
#include <stdio.h>
main()
{
	char key[8],start[8],work[8];
	long update;
	register int i;
	long iter;
	char ks[16][8];

	desinit(0);
	printf("Enter key: ");
	get8(key);
	printf("Setting key: "); put8(key); printf("\n");
	setkey(ks,key);
	printf("Enter starting value: ");
	get8(start);
	printf("Starting value: "); put8(start); printf("\n");
	printf("Update interval: ");
	scanf("%ld",&update);

	memcpy(work,start,8);

	for(iter = 0;; iter++){

		endes(ks,work);
		if((iter % update) == 0){
			printf("%ld ",iter);
			put8(work); printf("\n");
		}
		if(memcmp(work,start,8) == 0){
			printf("CYCLE FOUND after %ld iterations\n",iter);
			exit(0);
		}
	}
}
get8(cp)
char *cp;
{
	int i,t;

	for(i=0;i<8;i++){
		scanf("%2x",&t);
		*cp++ = t;
	}
}
put8(cp)
char *cp;
{
	int i;

	for(i=0;i<8;i++){
		printf("%2x ",*cp++ & 0xff);
	}
}
