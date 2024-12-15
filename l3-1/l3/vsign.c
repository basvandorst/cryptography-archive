
/* vsign.c (verify signatures) - Mark Henderson (markh@wimsey.bc.ca) */
#include "l3.h"
main(int argc, char *argv[])
{
	MP_INT *e, *c, *n;
	char pubfile[128];
	int i;
	int alg = getalg();
	unsigned char hash[HASHLEN];
	unsigned char sig[MAXC];
	FILE *public;
	FILE *sigfile;
	init(e);
	init(n);
	init(c);
	if (argc < 3) {
		fprintf(stderr, "usage: vsign pubfile sigfile\n");
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


	if (!(sigfile = fopen(argv[2], "rb"))) {
		fprintf(stderr, "could not open %s for read\n", argv[2]);
		exit(1);
	}

	if (!readalg(sigfile, &alg) || !read_number(sigfile, c)) {
		printf("corrupt file signature\n");
		exit(1);
	}

	haval_file_2(stdin, hash);
	if (vsign(c, hash, n, e, alg)) {
		printf("file signature verified\n");
	}
	else {
		printf("file signature did not verify\n");
		exit(1);
	}
	zap(hash,sizeof(hash));
	exit(0);
}
