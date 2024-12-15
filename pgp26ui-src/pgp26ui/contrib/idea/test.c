/* Copyright (c) 1993 Colin Plumb.  This code may be freely
   distributed under the terms of the GNU General Public Licence. */

   /* This was developed using Borland C for the IBM PC */

#include "idea.h"
#include <stdio.h>
#include <time.h>

#define RAND16 (rand() ^ rand() << 1)

uint16 huge bigbuf1[32764];
uint16 huge bigbuf2[32764];

void
timetest(uint16 iv[4], uint16 *key)
{
	clock_t start, finish;
	uint16 iv2[4];
	uint16 i;

	iv2[0]=iv[0]; iv2[1]=iv[1]; iv2[2]=iv[2]; iv2[3]=iv[3];

	printf("Starting time test.\n");

	start = clock();
#if 0
	IdeaCFB(iv, key, bigbuf1, bigbuf2, 8192);
	IdeaCFB(iv, key, bigbuf2, bigbuf1, 8192);
	IdeaCFBx(iv2, key, bigbuf1, bigbuf2, 8192);
	IdeaCFBx(iv2, key, bigbuf2, bigbuf1, 8192);
#else
	for (i = 0; i < 32; i++) {
		IdeaCFB(iv, key, bigbuf1, bigbuf2, 8192);
		IdeaCFBx(iv, key, bigbuf2, bigbuf2, 8192);
	}
#endif
	finish = clock();
	printf("%4x / %4x  %4x / %4x  %4x / %4x  %4x / %4x\n",
		iv[0], iv2[0], iv[1], iv2[1], iv[2], iv2[2], iv[3], iv2[3]);
	finish -= start;
	printf("Time taken for 64 64K buffers (4096 K): %lu clocks, %f seconds\n",
		finish, (float)finish/CLK_TCK);
}

int
main(int argc, char **argv)
{
	ulong j;
	uint16 a, b, c, d, i;
	uint16 key[KEYSIZE], in[4], out1[4], out2[4], out3[4];
	uint16 buf1[8], buf2[8], buf3[8];

	if (argc < 2)
		srand((unsigned)time(0));
	else {
		argv++;	/* suppress silly warning */
		srand(argc);
	}

	printf("Starting to test...\n");
for (j = 0; j < 10000; j++) {
	if (j % 100 == 0)
		printf("%5lu\r", j);
	a = RAND16;
	b = RAND16;
	c = Mul(a,b);
	d = Mul2(a,b);
	if (c != d)
		printf("%u * %u = %u / %u\n", a, b, c, d);

	for (i = 0; i < 4; i++)
		in[i] = RAND16;
	for (i = 0; i < KEYSIZE; i++)
		key[i] = RAND16;
	Idea(in, out1, key);
#if 0
	Idea2(in, out2, key);
	Idea3(in, out3, key);
#else
	for (i = 0; i < 4; i++)
		out3[i] = out2[i] = in[i];
	IdeaCFB(out2, key, buf1, buf1, 1);	/* buf1 args are dummies */
	IdeaCFBx(out3, key, buf1, buf1, 1);
#endif
	Idea3(in, out3, key);
	for (i = 0; i < 4; i++)
		if (out1[i] != out2[i] || out2[i] != out3[i]) {
			printf("Unequal for j = %lu\n", j);
			for (i = 0; i < 4; i++)
				printf("%4x - %4x / %4x / %4x    %4x\n",
				       in[i], out1[i], out2[i], out3[i], key[i]);
			break;
		}
	for (i = 0; i < 8; i++)
		buf1[i] = RAND16;
	IdeaCFB(out2, key, buf1, buf2, 3);
	IdeaCFBx(out3, key, buf2, buf3, 3);
	a = 0;
	for (i = 0; i < 4; i++)
		if (out2[i] != out3[i])
			a = 1;
	for (i = 0; i < 8; i++)
		if (buf1[i] != buf3[i])
			a = 1;
	if (a) {
		printf("CFB problem for j = %lu\n", j);
		for (i = 0; i < 4; i++)
			printf("%4x / %4x = %4x     %4x\n",
			     out1[i], out2[i], out3[i], key[i]);
		for (i = 0; i < 8; i++)
			printf("%4x -> %4x -> %4x   %4x\n",
			     buf1[i], buf2[i], buf3[i], key[i+4]);
	}
} /* for (j) */
	timetest(in, key);
	return 0;
}