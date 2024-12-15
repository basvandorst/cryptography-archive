#include <stdio.h>
#include "beale.h"
#include "getwrd.c"

/*
 *	use --- print use counts for all 3 Beale ciphers
 */

int use1[MAXKEY] = {0};
int use2[MAXKEY] = {0};
int use3[MAXKEY] = {0};

char    ustr[] = ".123456789ABCDEF************************";

main(argc,argv)
int argc;
char **argv;
{
	register int i = 1;
	char	buf[MAXBUF];	/* hold a keytext word */

	collect(msg1,SIZE1,use1);
	collect(msg2,SIZE2,use2);
	collect(msg3,SIZE3,use3);

	while( getwrd(buf) != EOF )
		usage(i++,buf);
	for( ;i<MAXKEY;i++)
		if(use1[i] || use2[i] || use3[i] )
			usage(i,"_");   /* tag on non-zero elements */
}

usage(i,s)
int i;
char *s;
{
	printf("%4d %c%c%c",i,ustr[use1[i]],ustr[use2[i]],ustr[use3[i]]);
	printf(" %s\n",s);
}

collect(ip,size,a)
int *ip;        /* pointer to code message */
int size;       /* length of message */
int *a;		/* accumulated array counts */
{
	while(size--)
		a[ *ip++ ] += 1;
}
