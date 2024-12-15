#include <stdio.h>
#include "beale.h"
#include "loadkey.c"
#include "getwrd.c"

/*
 *      code --- read key text from standard input and use
 *               it to decrypt Beale ciphers
 */

#define WIDTH    75     /* default number of letters per line */
#define VWIDTH   25     /* default number of letters per line (verbose mode)*/
int verbose = 0;        /* display more crud when set */
int mflag = 0;		/* vertical matrix format when set */
int rflag = 0;          /* reverse output */
int lflag = 0;          /* use last letter in each keyword */


main(argc,argv)
int argc;
char **argv;
{
	register int i,j;
	register int *ip;       /* pointer to selected cipher */
	int msglen;             /* length of selected cipher */
	int width = WIDTH;      /* display width */
	char *p;

	/* default is #1, non-verbose */
	ip = msg1;
	msglen = SIZE1;
	if( argc >= 2){
		p = argv[1];    /* first argument is which cipher & verbose */
		while(*p){
			switch(*p++){
				case '1':
					ip = msg1;
					msglen = SIZE1;
					break;
				case '2':
					ip = msg2;
					msglen = SIZE2;
					break;
				case '3':
					ip = msg3;
					msglen = SIZE3;
					break;
				case 'v':
					verbose++;
					width = VWIDTH;
					break;
				case 'm':	/* vertical matrix */
					mflag++;
					break;
				case 'r':
					rflag++;
					break;
				default:
					printf("Usage: code [123vmr] [display-width]  <document\n");
					exit();
				}
			}
		}

	if( argc >= 3 )
		width = atoi(argv[2]);  /* use specified width */

	loadkey();

	if(mflag){	/* vert. matrix flag overrides other options */
		vdump(ip,msglen,width);
		exit(0);
		}
	/* convert cipher to text */
	if(verbose && !rflag){    /* include key numbers over text */
		for(i=0; i<msglen; i += width){
			for(j=i; j<i+width && j<msglen; j++)
				printf("%-5d",*(ip+j));
			printf("\n");
			for(j=i; j<i+width && j<msglen; j++)
				printf("%c    ",key[*(ip+j)]);
			printf("\n\n");
			}
		}
	else if (!rflag){   /* short form */
		for(i=0;i<msglen;i+=width){
			for(j=i;j<i+width && j<msglen; j++)
				printf("%c",key[*(ip+j)]);
			printf("\n");
			}
		}
	else{
		for(i=msglen-1; i>=0; i-=width){
			for(j=i;j> i-width && j>=0; j--)
				printf("%c",key[*(ip+j)]);
			printf("\n");
			}
		}
}

/*
 *	vdump --- vertical matrix dump of a cipher
 */
vdump(ip,l,width)
int *ip;	/* which cipher */
int l;		/* cipher length */
int width;      /* elements per line */
{
	while(l--){
		printf("%d\t",*ip);
		outline(*ip,10);
		printf("\n");
		ip++;
		}
}

/* 
 *	outline --- output elements around a specific key element
 */
outline(ell,eps)
int ell;	/* key element # */
int eps;	/* plus/minus this much */
{
	register int i;

	for(i=ell-eps; i<= ell+eps; i++){
		if( i<=0 )
			printf(" ");
		else if( i==ell )
			printf(" %c ",key[i]);
		else
			printf("%c",key[i]);
		}
}
