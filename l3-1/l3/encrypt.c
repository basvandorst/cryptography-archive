
/* encrypt.c - Mark Henderson (markh@wimsey.bc.ca) */
#include "l3.h"
main(int argc, char *argv[])
{
	char pubfile[128];
	unsigned int i, k;
	int alg = getalg();
	unsigned char key[LKEYLEN], cipher[MAXC];
	FILE *public;
	MP_INT *e, *n, *c;
	init(e);
	init(n);
	init(c);

	if (argc < 2) {
		fprintf(stderr, "usage: encrypt pubfile [randomdata...] \n");
		exit(1);
	}

	strncpy(pubfile, argv[1], 120);
	strcat(pubfile, ".pub");

	if (!(public = mfopen(pubfile, "rb"))) {
		fprintf(stderr, "could not open %s for read\n", pubfile);
		exit(1);
	}

	if (readpk(public, n, e) < 0) {
		fprintf(stderr, "execution aborted: readpk\n");
		exit(1);
	}

	fclose(public);

	initrandom();
	for (i = 2; i < argc; i++)
		if (argv[i] && (k = strlen(argv[i]))) {
			seedrandom((unsigned char *) argv[i], k);
			zap((unsigned char *) argv[i], k);
		}
#ifndef BATCH
	fprintf(stderr, "please enter a random string -- end with <CR>\n");
	(void) getstring(0);
#endif
	for (i = 0; i < LKEYLEN; i++)
		key[i] = randombyte();

	encrypt_session_key(c, key, n, e, alg);
	fputc((char) alg, stdout);
	write_number(stdout, c);
	scramble(stdin, stdout, &key[0], &key[KEYBYTES], EN0);
	zap(key, LKEYLEN);
	closerandom();
	exit(0);
}
