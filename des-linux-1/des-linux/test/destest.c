#include <stdio.h>
main()
{
	char key[8],work[8];
	char ks[16][8];

	desinit(0);
	printf("Ready\n");
	get8(key);
	printf("Setting key: "); put8(key); printf("\n");
	setkey(ks,key);
	printf("Key set\n");

go4it:
		get8(work);
		printf("Plain: "); put8(work); printf("\n");
		endes(ks,work);
		printf("Cipher: "); put8(work); printf("\n");
		dedes(ks,work);
		printf("Decrypted: "); put8(work); printf("\n");
goto go4it;
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
		printf("%02x ",*cp++ & 0xff);
	}
}
