#include <stdio.h>
#include "beale.h"

#define	DEFLIM	3000	/* throw out elements above this value */
int	Limit = DEFLIM;
#define MAXMOD  200
#define	SCALE	30
int     m[MAXMOD];

main(argc,argv)
int argc;
char **argv;
{
	int     i;
	int     modulo;
	int     start,end;

	if( argc < 2)
		exit();
	if( *argv[1] == '-'){	/* use non-standard limit */
		Limit = atoi(*++argv);
		Limit = -Limit;
		argc--;
		}
	start = atoi(*++argv);  /* starting modulo */
	if( argc > 2 )
		end = atoi(*++argv);
	else
		end = start;
	if(end>=MAXMOD)
		end = MAXMOD-1;

	for(modulo=start;modulo<=end;modulo++){
		display("B1",msg1,SIZE1,modulo);
		display("B2",msg2,SIZE2,modulo);
		display("B3",msg3,SIZE3,modulo);
		}
}



display(s,msg,msglen,modulo)
char *s;
int *msg;
int msglen;
int modulo;
{
	register int i;
	register int j;
	double weight;
	int     counted = 0;
	double  mu,sigma,p,q;      /* standard deviation test */
	double  sqrt();

	for(i=0;i<modulo;i++)
		m[i]=0;

	for(i=0;i<msglen;i++)
		if( msg[i] <= Limit )
			{
			m[ msg[i]%modulo]++;
			counted++;
			}
	p = 1./modulo;
	q = 1 - p;
	mu = p * counted;
	sigma = sqrt( mu * q);
	printf("%s %% %d, mean:%6.2f, sigma:%6.2f\n",s,modulo,mu,sigma);

	for(i=0;i<modulo;i++)
		{
		printf("%s %%%-2d=%2d:",s,modulo,i);
		printf("%3d",m[i]);
		weight = (m[i]/mu)* SCALE;
		for(j=0;j<weight;j++)
			printf(" ");
		printf("%d",modulo);
		while( m[i] < mu-sigma ){
			printf("-");
			m[i] += sigma;
			}
		while( m[i] > mu+sigma ){
			printf("+");
			m[i] -= sigma;
			}
		printf("\n");
		}
	printf("\n");
}
