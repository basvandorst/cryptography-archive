/* Just run DES in a loop consuming CPU time; good for benchmarking
 * Phil Karn
 */
#include <stdio.h>
/***************************************************************
 *
 * Procedure main
 *
 ***************************************************************/
#ifdef __STDC__

int main(
)

#else

int main(
)

#endif

{
	char key[8],work[8];
	long iter,count;

	desinit(0);
	printf("Enter key: ");
	get8(key);
	printf("Setting key: "); put8(key); printf("\n");
	setkey_(key);
	printf("Enter starting value: ");
	get8(work);
	printf("Starting value: "); put8(work); printf("\n");
	printf("Number of iterations: ");
	scanf("%ld",&count);

	for(iter = 0;iter < count; iter++)
		endes(work);
}
/***************************************************************
 *
 * Procedure get8
 *
 ***************************************************************/
#ifdef __STDC__

int get8(
	char	 *cp
)

#else

int get8(
	cp
)
char	 *cp;

#endif

{
	int i,t;

	for(i=0;i<8;i++){
		scanf("%2x",&t);
		*cp++ = t;
	}
}
/***************************************************************
 *
 * Procedure put8
 *
 ***************************************************************/
#ifdef __STDC__

int put8(
	char	 *cp
)

#else

int put8(
	cp
)
char	 *cp;

#endif

{
	int i;

	for(i=0;i<8;i++){
		printf("%2x ",*cp++ & 0xff);
	}
}
