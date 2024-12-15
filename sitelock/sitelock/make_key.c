/* make_key for sitelock V 1.01 
 */
/* The following copyright notices apply
 */
/* Copyright (C) 1994 by me.
 */
/* Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
rights reserved.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/* The following makes MD default to MD5 if it has not already been
  defined with C compiler flags.
 */
#ifndef MD
#define MD MD5
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "global.h"
#if MD == 2
#include "md2.h"
#endif
#if MD == 4
#include "md4.h"
#endif
#if MD == 5
#include "md5.h"
#endif


static void MDPrint PROTO_LIST ((unsigned char [16]));

#if MD == 2
#define MD_CTX MD2_CTX
#define MDInit MD2Init
#define MDUpdate MD2Update
#define MDFinal MD2Final
#endif
#if MD == 4
#define MD_CTX MD4_CTX
#define MDInit MD4Init
#define MDUpdate MD4Update
#define MDFinal MD4Final
#endif
#if MD == 5
#define MD_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final
#endif


/* Prints a message digest in hexadecimal.
 */
static void MDPrint (digest)
unsigned char digest[16];
{
  unsigned int i;

  for (i = 0; i < 16; i++)
 printf ("%02x", digest[i]);
}

void generate_key() {
	MD_CTX context;
	unsigned char digest[16];
	unsigned char buf[5];
	int i;
	sprintf(buf,"%x",gethostid());
	
	MDInit(&context);
	MDUpdate(&context,buf,4);
	MDFinal(digest,&context);
	printf("Key for hostid: %x\n",gethostid());
	printf("unsigned char key[] = {\t");
	for(i=0;i<8;i++) 
		printf(" 0x%02x,",digest[i]);
	printf("\n\t\t\t");
	for(;i<15;i++)
		printf(" 0x%02x,",digest[i]);
	printf(" 0x%02x",digest[15]);
	printf("\n");
	printf("};\n");
}
/*
 * What we do when we discover we are running on a non authorized host
 */

void nadda(argv)
char **argv;
{
	unlink(argv[0]);
	exit(1);
}

main() {
	generate_key();
}
