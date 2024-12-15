#include <stdio.h>
#include <sys/time.h>
#include "des.h"

unsigned long ks[16][2];
unsigned char buf[8];

main()
{
	unsigned long i;
	struct timeval start,stop;
	long elapsed;
	double s;

	deskey(ks,(unsigned char *)"12345678",0);
	printf("starting DES time trial, 1,000,000 encryptions\n");
	gettimeofday(&start,NULL);

	for(i=0;i<1000000;i++)
		des(ks,buf);

	gettimeofday(&stop,NULL);
	elapsed = 1000*(stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec)/1000.;
	printf("execution time = %ld ms\n",elapsed);
	s = 1000.*1000000./elapsed;
	printf("%.1lf crypts/sec %.1lf bytes/sec %.1lf bits/sec",s,s*8,s*64);
	return 0;
}
