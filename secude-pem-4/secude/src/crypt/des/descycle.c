/* Investigate cycles in DES output feedback mode (experimental)
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
	char key[8],start[8],work[8];
	long update;
	register int i;
	long iter;

	desinit(0);
	printf("Enter key: ");
	get8(key);
	printf("Setting key: "); put8(key); printf("\n");
	setkey_(key);
	printf("Enter starting value: ");
	get8(start);
	printf("Starting value: "); put8(start); printf("\n");
	printf("Update interval: ");
	scanf("%ld",&update);

	for(i=0;i<8;i++)
		work[i] = start[i];

	for(iter = 0;; iter++){

		endes(work);
		if((iter % update) == 0){
			printf("%ld ",iter);
			put8(work); printf("\n");
		}
		for(i=0;i<8;i++){
			if(work[i] != start[i])
				break;
		}
		if(i == 8){
			printf("CYCLE FOUND after %ld iterations\n",iter);
			exit(0);
		}
	}
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
