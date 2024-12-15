/*
 * ESM DH parameter generator
 */

/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1995 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include "global.h"
#include "rsaref.h"
#include "esm.h"

main()
{
	R_DH_PARAMS dhparams[3];
	unsigned char p0[512],p1[512],p2[512],g0[512],g1[512],g2[512];
	int x;
	
	randinit();

	fprintf(stderr,"Generating 512\n");
	dhparams[0].prime=p0;
	dhparams[0].generator=g0;
	if (x=R_GenerateDHParams(&dhparams[0],512,256,&rs))
		fprintf(stderr,"failed %x\n",x);
	fprintf(stderr,"Generating 768\n");
	dhparams[1].prime=p1;
	dhparams[1].generator=g1;
	if (x=R_GenerateDHParams(&dhparams[1],768,384,&rs))
		fprintf(stderr,"failed %x\n",x);
	fprintf(stderr,"Generating 1024\n");
	dhparams[2].prime=p2;
	dhparams[2].generator=g2;
	if (x=R_GenerateDHParams(&dhparams[2],1024,512,&rs))
		fprintf(stderr,"failed %x\n",x);
	fprintf(stderr,"Printing\n");
	printparams(dhparams);
	exit(0);
}

printparams(dh)
     R_DH_PARAMS *dh;
{
	int i,j;
	
	printf("#include \"global.h\"\n");
	printf("#include \"rsaref.h\"\n");
	for (j=0; j<3; j++) {
		printf("static unsigned char p%d[] = {\n",j);
		for (i=0; i < (dh[j].primeLen-1); i++) {
			printf(" 0x%02x,",dh[j].prime[i]);
			if (i%6 == 5)
				printf("\n");
		}
		printf(" 0x%02x};\n",dh[j].prime[i]);
		printf("static unsigned char g%d[] = {\n",j);
		for (i=0; i < (dh[j].generatorLen-1); i++) {
			printf(" 0x%02x,",dh[j].generator[i]);
			if (i%6 == 5)
				printf("\n");
		}
		printf(" 0x%02x};\n",dh[j].generator[i]);
	}
	printf("\nR_DH_PARAMS dhparams[3] = {\n");
	for (j=0; j<3; j++) {
		printf(" {p%d, %d, g%d, %d}%s\n",j,dh[j].primeLen,
		       j, dh[j].generatorLen, j==2?"};":",");
	}
}
	
