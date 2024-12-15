
/* vfk.c (verify signature on public key)
  - Mark Henderson (markh@wimsey.bc.ca) */
#include "l3.h"
main(int argc, char *argv[])
{
	MP_INT *e, *n;
	char pubfile[128];
	int i;
	FILE *public;
	int alg = getalg();

	init(e);
	init(n);
	if (argc < 2) {
		fprintf(stderr, "usage: vfk pubfile\n");
		exit(1);
	}

	strncpy(pubfile, argv[1], 120);
	strcat(pubfile, ".pub");

	if (!(public = mfopen(pubfile, "rb"))) {
		fprintf(stderr, "could not open %s for read\n", pubfile);
		exit(1);
	}

	if (readpk(public, n, e) <= 0)
		exit(1);
	fclose(public);
	clear(e);
	clear(n);

	exit(0);
}
