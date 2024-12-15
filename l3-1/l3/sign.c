
/* sign.c - Mark Henderson (markh@wimsey.bc.ca) */
#include "l3.h"
main(int argc, char *argv[])
{
	MP_INT *e, *p, *q, *c;
	int alg = getalg();
	char prvfile[128];
	int i;
	unsigned char k[16];
	unsigned char iv[8];
	unsigned char hash[HASHLEN];
	unsigned char *ss;
	FILE *private;
	init(e);
	init(p);
	init(q);
	init(c);
	if (argc < 2) {
		fprintf(stderr, "usage: sign prvfile\n");
		exit(1);
	}
	for (i = 0; i < 8; i++)
		iv[i] = 0;

	strncpy(prvfile, argv[1], 120);
	strcat(prvfile, ".prv");

	if (!(private = mfopen(prvfile, "rb"))) {
		fprintf(stderr, "could not open %s for read\n", prvfile);
		exit(1);
	}

	fprintf(stderr, "Enter password to private key: ");
	ss = getstring(1);
	mdstring(ss, strlen(ss), k);
	for (i = 0; i < 8; i++)
		iv[i] = 0;

	if (!read_numberx(private, p, k, iv) || !read_numberx(private, q, k, iv)) {
		fprintf(stderr, "wrong key to key entered or key corrupt\n");
		zap(k, 16);
		zap(hash, sizeof(hash));
		zap(ss, strlen(ss));
		exit(1);
	}

	read_number(private, e);

	haval_file_2(stdin,hash);
	sign(c, hash, p, q, e, alg);
	fputc((char) alg, stdout);
	write_number(stdout, c);
	zap(k, 16);
	zap(hash, 16);
	zap(ss, strlen(ss));
	exit(0);
}
