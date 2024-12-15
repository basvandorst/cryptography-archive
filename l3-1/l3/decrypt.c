/* decrypt.c - Mark Henderson (markh@wimsey.bc.ca) */
#include "l3.h"
main(int argc, char *argv[])
{
	MP_INT *e, *p, *q, *c;
	char prvfile[128];
	int i;
	unsigned char key[LKEYLEN];
	int alg = getalg();
	unsigned char *ss;
	unsigned char iv[8];
	unsigned char k[16];
	unsigned char cipher[MAXC];
	FILE *private;
	init(e);
	init(p);
	init(c);
	init(q);
	if (argc < 2) {
		fprintf(stderr, "usage: decrypt prvfile\n");
		exit(1);
	}

	strncpy(prvfile, argv[1], 120);
	strcat(prvfile, ".prv");

	if (!(private = mfopen(prvfile, "rb"))) {
		fprintf(stderr, "could not open %s\n", prvfile);
		exit(1);
	}
	fprintf(stderr, "Enter password to private key: ");
	ss = getstring(1);
	mdstring(ss, strlen(ss), k);
	for (i = 0; i < 8; i++)
		iv[i] = 0;

	if (!read_numberx(private, p, k, iv) || !read_numberx(private, q, k, iv)) {
		fprintf(stderr, "wrong key to key entered or key corrupt\n");
		zap(key, 32), zap(k, 16);
		zap(cipher, MAXC);
		zap(ss, strlen(ss));
		exit(1);
	}
	read_number(private, e);

	if (!readalg(stdin, &alg) || !read_number(stdin, c)) {
		fprintf(stderr, "readalg failed -- file corruption detected\n");
		zap(key, LKEYLEN), zap(k, 16);
		zap(cipher, MAXC);
		zap(ss, strlen(ss));
		exit(1);
	}
	decrypt_session_key(key, c, p, q, e, alg);
#ifdef DEBUG
	fprintf(stderr, "decrypted session key is:");
	for (i = 0; i < LKEYLEN; i++)
		fprintf(stderr, "%02x", key[i]);
	fprintf(stderr, "\n");
#endif
	if (!scramble(stdin, stdout, &key[0], &key[KEYBYTES], DE1)) {
		fprintf(stderr, "possible file corruption detected\n");
		zap(key, LKEYLEN), zap(k, 16);
		zap(cipher, MAXC);
		zap(ss, strlen(ss));
		exit(1);
	}
	zap(key, LKEYLEN), zap(k, 16);
	zap(cipher, MAXC);
	zap(ss, strlen(ss));
	exit(0);
}
