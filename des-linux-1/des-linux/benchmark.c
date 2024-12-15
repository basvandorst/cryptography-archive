/* Just run DES in a loop consuming CPU time; good for benchmarking
 * Phil Karn
 */
#include <stdio.h>
main()
{
	static char key[8] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
	static char work[8];
	long iter,count;
	char ks[16][8];

	desinit(0);
	printf("Setting key: "); put8(key); printf("\n");
	setkey(ks,key);
	printf("Starting value: "); put8(work); printf("\n");
	printf("Number of iterations: ");
	scanf("%ld",&count);

	for(iter = 0;iter < count; iter++)
		endes(ks,work);
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
