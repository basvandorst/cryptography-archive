
/* ssk.c (self-sign key) - Mark Henderson (markh@wimsey.bc.ca) */

/* this adds a key identifier (string of <127 characters to a key)
   and signs the (public key, identifier pair). This can then be
   checked by encrypt and vsign. This prevents certain kinds of tampering
   with the identifier information.

   typically the identifier info should consist of the key holder's
   name and email address. I suggest something like:

   John H. Smith <jsmith@foo.bar.ca>

*/

#include "l3.h"
main(int argc, char *argv[])
{
	MP_INT *e, *p, *q, *c, *n;
	char prvfile[127];
	int i;
	char pubfile[127];
	unsigned int idlen;
	unsigned char k[16];
	unsigned char iv[8];
	unsigned char idstring[127];
	unsigned char hash[HASHLEN];
	int alg = getalg();
	unsigned char *ss;
	FILE *private;
	FILE *public;
	init(e);
	init(p);
	init(q);
	init(c);
	init(n);
	if (argc < 2) {
		fprintf(stderr, "usage: ssk prvfile\n");
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
		zap(hash, 16);
		zap(ss, strlen(ss));
		exit(1);
	}

	read_number(private, e);
	mpz_mul(n, p, q);
	strncpy(pubfile, argv[1], 120);
	strcat(pubfile, ".pub");

	if (!(public = fopen(pubfile, "wb"))) {
		fprintf(stderr, "could not open %s for write\n", pubfile);
		zap(k, 16);
		zap(hash, HASHLEN);
		zap(ss, strlen(ss));
		exit(1);
	}

	fprintf(stderr, "please enter one line id, e.g. John Smith <john@foo.ca>\n");
	if (!fgets(idstring, 127, stdin)) {
		fprintf(stderr, "could not read id from stdin\n");
		zap(k, 16);
		zap(hash, HASHLEN);
		zap(ss, strlen(ss));
		exit(1);
	}
	idlen = strlen(idstring);
	if (idstring[idlen - 1] == '\n') {
		idstring[idlen - 1] = '\0';
		idlen--;
	}
	havalpkid(n, e, idstring, hash);

#ifdef DEBUG
	fprintf(stderr, "hash of (%s,n,e) = ", idstring);
	for (i = 0; i < HASHLEN; i++)
		fprintf(stderr, "%02x", hash[i]);
	fprintf(stderr, "\n");
#endif

	sign(c, hash, p, q, e, alg);
	write_number(public, n);
	write_number(public, e);
	write_string(public, idstring);
	fputc((char) alg, public);
	write_number(public, c);
	fclose(public);
	zap(k, 16);
	zap(hash, sizeof(hash));
	zap(ss, strlen(ss));
}
