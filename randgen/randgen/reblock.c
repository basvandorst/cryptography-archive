/* take the output of the  bc  program and make it more readable */
/* by converting to binary, blocking in groups of ten, etc.      */


#include <stdio.h>

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	int	nbits, i, j, new, buf, blen, llen, gave, enuf ;

	int	atoi() ;

	if(argc != 2) { printf("Usage: %s no_of_bits\n", argv[0]); exit(1); }
	enuf=atoi(argv[1]);

	i = scanf("%d", &nbits);  if(i == EOF) exit(1);

	gave = 0;
	buf = 0;
	blen = 0;
	llen = 0;
	while(gave < enuf)
	{
		i = scanf("%d", &new) ;  if(i==EOF) new=0;
		buf = (buf << nbits) | new ;
		blen += nbits ;
		if(blen >= 10)
		{
			for(j=(blen-1); j>=(blen-10); j--)
			{
				i = (buf & (0x001 << j)) ? 1 : 0 ;
				buf &= (~ (0x001 << j) ) ;
				if(gave < enuf) { printf("%01d",i); gave++; }
			}
			blen -= 10 ;
			llen += 11 ;
			if(llen > 75) { printf("\n"); llen = 0; }
			else printf(" ");
		}
	}
	if(llen != 0) printf("\n");
}
