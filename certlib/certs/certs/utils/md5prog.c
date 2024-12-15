
#pragma ident "@(#)md5prog.c	1.2 95/11/18"
/* Main program to do md5 encryption using Colin Plumbs MD5 routines */

#include <sys/types.h>
#include <stdio.h>
typedef unsigned char byte;
#include "md5.h"


md5it(FILE *fp, byte *digest)
{
	u_char buf[1024];
	size_t n;
	struct xMD5Context c;
	xMD5Init(&c);
	do {
		n=fread((void *)buf, 1, 1024, fp);
		if (n > 0)
			xMD5Update(&c, (byte *)buf, n);
	} while (n);
	xMD5Final((byte *)digest, &c);
}
		
m_print(byte digest[]) {
	int i;
	for (i=0; i< 16; i++)
		printf("%02x", digest[i]);
}

main(int argc,const char *argv[])
{
	byte digest[16];
	int i;
	FILE *fp;
	if (argc == 1){
		md5it(stdin, digest);
		m_print(digest);
		printf("\n");
		exit(0);
	}
	for (i=1; i<argc; i++) {
		fp=fopen(argv[i],"r");
		if (fp == NULL) {
			printf("error opening %s\n",argv[i]);
			continue;
		}
		md5it(fp,digest);
		printf("MD5 (%s) = ",argv[i]);
		m_print(digest);
		printf("\n");
		fclose(fp);
	}

}
