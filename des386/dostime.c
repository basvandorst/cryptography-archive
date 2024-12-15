/* DES code timing driver for Borland C++ under DOS */
#include <stdio.h>
#include <sys/timeb.h>
#include "des.h"

unsigned long ks[16][2];
unsigned char buf[8];

main()
{
	unsigned long i;
	struct timeb start,stop;
	long elapsed;
	double s;

	deskey(ks,(unsigned char *)"12345678",0);
	printf("starting DES time trial, 1,000,000 encryptions\n");
	ftime(&start);

	for(i=0;i<1000000;i++)
		des(ks,buf);

	ftime(&stop);
	elapsed = 1000*(stop.time - start.time) + (stop.millitm - start.millitm);
	printf("execution time = %ld ms\n",elapsed);
	s = 1000.*1000000./elapsed;
	printf("%.1lf crypts/sec %.1lf bytes/sec %.1lf bits/sec",s,s*8,s*64);
	return 0;
}
