/*
 * des - {en,de}crypt text using the DES CBC procedures
 */

#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <des.h>

#define	STREQ(a, b)	(*(a) == *(b) && strcmp((a), (b)) == 0)

#define	READBUFSIZE	(16 * 1024)
unsigned char bufpair[2][READBUFSIZE];	/* buffer space for processing */

char *progname;
int debug;

#define	MODE_CBC	0
#define	MODE_ECB	1
#define	MODE_PCBC	2

int mode = MODE_CBC;	/* default is CBC mode */
int encrypt = -1;	/* don't know whether we're encrypting or decrypting */
int nowarning = 0;	/* for Sun des(1) compatibility */
int software = 0;	/* Ibid. */
int hexkey = 0;		/* key specified in hex */
char *akey = NULL;	/* ascii form of key */

#define TOHEX(c) \
	(((c)<='9') ? ((c)-'0') : (((c)<='F') ? ((c)+10-'A') : ((c)+10-'a')))

void key_set_parity();
void process();

extern char *getpass();

/*
 * main - parse arguments and handle options
 */
main(argc, argv)
int argc;
char *argv[];
{
	int c;
	int errflg = 0;
	FILE *in, *out;
	unsigned char key[sizeof(des_cblock)];
	extern int optind;
	extern char *optarg;

	progname = argv[0];
	while ((c = getopt(argc, argv, "bdefhk:ps")) != EOF)
		switch (c) {
		case 'b':
			mode = MODE_ECB;
			break;
		case 'd':
		case 'e':
			if (encrypt != -1) {
				(void) fprintf(stderr,
	"%s: -d and -e flags are exclusive and may only be specified once\n",
				    progname);
				errflg++;
			} else if (c == 'd') {
				encrypt = 0;
			} else {
				encrypt = 1;
			}
			break;
		case 'f':
			nowarning = 1;
			break;
		case 'h':
			hexkey = 1;
			break;
		case 'k':
			if (akey != NULL) {
				(void) fprintf(stderr,
				    "%s: only one key may be specified\n",
				    progname);
				errflg++;
			} else {
				akey = optarg;
			}
			break;
		case 'p':
			mode = MODE_PCBC;
			break;
		case 's':
			software = 1;
			break;
		default:
			errflg++;
			break;
		}
	
	/*
	 * A hex key must be specified on the command line
	 */
	if (hexkey && akey == NULL) {
		(void) fprintf(stderr,
	"%s: hex keys must be specified on the command line using -k\n",
		    progname);
		errflg++;
	}
	if (encrypt == -1) {
		(void) fprintf(stderr,
		    "%s: specify -e for encryption, -d for decryption\n",
		    progname);
		errflg++;
	}

	if (errflg) {
		(void) fprintf(stderr,
		 "usage: %s [-bfhps] -d|-e [-k key] [ in_file [ outfile ] ]\n",
		     progname);
		exit(2);
	}

	/*
	 * If we didn't get a key, prompt for one
	 */
	bzero((char *)key, sizeof(key));
	if (akey == NULL) {
		unsigned char samekey[sizeof(des_cblock)];

		for (;;) {
			bzero((char *)key, sizeof(key));
			akey = getpass("Enter key: ");
			(void) strncpy((char *)key, akey, sizeof(key));
			bzero((char *)samekey, sizeof(samekey));
			akey = getpass("Enter key again: ");
			(void) strncpy((char *)samekey, akey, sizeof(samekey));
			if (!bcmp((char *)key, (char *)samekey,
			    sizeof(des_cblock)))
				break;
			(void) fprintf(stderr, "Key mistyped, try again\n");
		}
		key_set_parity(key, sizeof(key));
	} else if (hexkey) {
		if (!gethexkey(akey, key))
			exit(1);
	} else {
		bzero((char *)key, sizeof(key));
		(void) strncpy((char *)key, akey, sizeof(key));
		key_set_parity(key, sizeof(key));
	}

	while (*akey != '\0')
		*akey++ = '\0';

	if (optind == argc) {
		process(key, encrypt, mode, stdin, "stdin",
		    stdout, "stdout");
	} else {
		if ((in = fopen(argv[optind], "r")) == NULL) {
			(void) fprintf(stderr, "%s: can't open %s: ",
			    progname, argv[optind]);
			perror("");
			exit(1);
		}
		if (++optind == argc) {
			process(key, encrypt, mode, in, argv[optind-1],
			    stdout, "stdout");
			(void) fclose(in);
		} else {
			if ((out = fopen(argv[optind], "w")) == NULL) {
				(void) fprintf(stderr, "%s: can't open %s: ",
				    progname, argv[optind]);
				perror("");
				exit(1);
			}
			process(key, encrypt, mode, in, argv[optind-1],
			    out, argv[optind]);
			(void) fclose(in);
			(void) fclose(out);
		}
	}
	exit(0);
}

/*
 * do_cbc_encrypt - do our stuff
 */
static void
do_cbc_encrypt(schedule, in, inname, out, outname)
	des_key_schedule schedule;
	FILE *in;
	char *inname;
	FILE *out;
	char *outname;
{
	int i, j;

	bzero((char *)&bufpair[1][READBUFSIZE-8], 8);
	while ((i = fread(&bufpair[0][0], sizeof(char), READBUFSIZE, in))
	    == READBUFSIZE) {
		des_cbc_encrypt((des_cblock *)&bufpair[0][0],
		    (des_cblock *)&bufpair[1][0], READBUFSIZE, schedule,
		    (des_cblock *)&bufpair[1][READBUFSIZE-8], 1);
		(void) fwrite(&bufpair[1][0], sizeof(char), READBUFSIZE, out);
	}

	/*
	 * Check for errors.
	 */
	if (i < 0) {
		(void) fprintf(stderr, "%s: read %s: ", progname, inname);
		perror("");
		exit(1);
	}

	/*
	 * No error.  Pad length out to multiple of 8 and put number
	 * of nonuseless characters out of the last 8 in last byte.
	 */
	j = ((i + 8) & ~(0x7));
	bufpair[0][j-1] = (unsigned char)(8 - (j-i));
	des_cbc_encrypt((des_cblock *)&bufpair[0][0],
	    (des_cblock *)&bufpair[1][0], j, schedule,
	    (des_cblock *)&bufpair[1][READBUFSIZE-8], 1);
	(void) fwrite(&bufpair[1][0], sizeof(char), j, out);
}


/*
 * do_cbc_decrypt - decrypt the input
 */
static void
do_cbc_decrypt(schedule, in, inname, out, outname)
	des_key_schedule schedule;
	FILE *in;
	char *inname;
	FILE *out;
	char *outname;
{
	int i, j;
	unsigned char ivec[8];

	bzero((char *)ivec, sizeof(ivec));
	while ((i = fread(&bufpair[0][0], sizeof(char), READBUFSIZE, in))
	    == READBUFSIZE) {
		des_cbc_encrypt((des_cblock *)&bufpair[0][0],
		    (des_cblock *)&bufpair[1][0], READBUFSIZE, schedule,
		    (des_cblock *)ivec, 0);
		(void) fwrite(&bufpair[1][0], sizeof(char), READBUFSIZE, out);
		bcopy((char *)&bufpair[0][READBUFSIZE-8], (char *)ivec, 8);
	}

	/*
	 * Check for errors.
	 */
	if (i < 0) {
		(void) fprintf(stderr, "%s: read %s: ", progname, inname);
		perror("");
		exit(1);
	}

	/*
	 * No error.  Make sure we've got a multiple of 8 characters
	 * left or we're in trouble.
	 */
	if (i & 0x7) {
		(void) fprintf(stderr,
"%s: warning: file size not multiple of 8 (may not have been encrypted)\n",
		    progname);
		/*
		 * Corrective action.  Force everything to match.
		 */
		j = ((i + 8) & ~(0x7));
		bufpair[0][j-1] = (unsigned char)(8 - (j-i));
	} else {
		j = i;
	}
	des_cbc_encrypt((des_cblock *)&bufpair[0][0],
	    (des_cblock *)&bufpair[1][0], i, schedule,
	    (des_cblock *)ivec, 0);
	if ((bufpair[1][j-1] & ~(0x7)) != 0) {
		(void) fprintf(stderr,
	"%s: warning: file format incorrect (may not have been encrypted)\n",
		    progname);
	} else {
		i = (j-8) + (int)bufpair[1][j-1];
	}
	(void) fwrite(&bufpair[1][0], sizeof(char), i, out);
}


/*
 * do_pcbc_encrypt - do our stuff
 */
static void
do_pcbc_encrypt(schedule, in, inname, out, outname)
	des_key_schedule schedule;
	FILE *in;
	char *inname;
	FILE *out;
	char *outname;
{
	int i, j;

	bzero((char *)&bufpair[1][READBUFSIZE-8], 8);
	while ((i = fread(&bufpair[0][0], sizeof(char), READBUFSIZE, in))
	    == READBUFSIZE) {
		des_pcbc_encrypt((des_cblock *)&bufpair[0][0],
		    (des_cblock *)&bufpair[1][0], READBUFSIZE, schedule,
		    (des_cblock *)&bufpair[1][READBUFSIZE-8], 1);
		(void) fwrite(&bufpair[1][0], sizeof(char), READBUFSIZE, out);
	}

	/*
	 * Check for errors.
	 */
	if (i < 0) {
		(void) fprintf(stderr, "%s: read %s: ", progname, inname);
		perror("");
		exit(1);
	}

	/*
	 * No error.  Pad length out to multiple of 8 and put number
	 * of nonuseless characters out of the last 8 in last byte.
	 */
	j = ((i + 8) & ~(0x7));
	bufpair[0][j-1] = (unsigned char)(8 - (j-i));
	des_pcbc_encrypt((des_cblock *)&bufpair[0][0],
	    (des_cblock *)&bufpair[1][0], j, schedule,
	    (des_cblock *)&bufpair[1][READBUFSIZE-8], 1);
	(void) fwrite(&bufpair[1][0], sizeof(char), j, out);
}


/*
 * do_pcbc_decrypt - decrypt the input
 */
static void
do_pcbc_decrypt(schedule, in, inname, out, outname)
	des_key_schedule schedule;
	FILE *in;
	char *inname;
	FILE *out;
	char *outname;
{
	int i, j;
	unsigned char ivec[8];

	bzero((char *)ivec, sizeof(ivec));
	while ((i = fread(&bufpair[0][0], sizeof(char), READBUFSIZE, in))
	    == READBUFSIZE) {
		des_pcbc_encrypt((des_cblock *)&bufpair[0][0],
		    (des_cblock *)&bufpair[1][0], READBUFSIZE, schedule,
		    (des_cblock *)ivec, 0);
		(void) fwrite(&bufpair[1][0], sizeof(char), READBUFSIZE, out);
		bcopy((char *)&bufpair[0][READBUFSIZE-8], (char *)ivec, 8);
	}

	/*
	 * Check for errors.
	 */
	if (i < 0) {
		(void) fprintf(stderr, "%s: read %s: ", progname, inname);
		perror("");
		exit(1);
	}

	/*
	 * No error.  Make sure we've got a multiple of 8 characters
	 * left or we're in trouble.
	 */
	if (i & 0x7) {
		(void) fprintf(stderr,
"%s: warning: file size not multiple of 8 (may not have been encrypted)\n",
		    progname);
		/*
		 * Corrective action.  Force everything to match.
		 */
		j = ((i + 8) & ~(0x7));
		bufpair[0][j-1] = (unsigned char)(8 - (j-i));
	} else {
		j = i;
	}
	des_pcbc_encrypt((des_cblock *)&bufpair[0][0],
	    (des_cblock *)&bufpair[1][0], i, schedule,
	    (des_cblock *)ivec, 0);
	if ((bufpair[1][j-1] & ~(0x7)) != 0) {
		(void) fprintf(stderr,
	"%s: warning: file format incorrect (may not have been encrypted)\n",
		    progname);
	} else {
		i = (j-8) + (int)bufpair[1][j-1];
	}
	(void) fwrite(&bufpair[1][0], sizeof(char), i, out);
}


/*
 * Hack at it if we don't have the buffer routine
 */
#ifndef	HAVE_ECB_MODE
#define	des_ecb_buffer(a, b, c, d, e) \
	do { \
		(void) fprintf(stderr, \
		    "%s: ECB mode not supported by library\n", progname); \
		exit(1); \
	} while (0)
#endif


/*
 * do_ecb_encrypt - encrypt the stream in ECB mode
 */
static void
do_ecb_encrypt(schedule, in, inname, out, outname)
	des_key_schedule schedule;
	FILE *in;
	char *inname;
	FILE *out;
	char *outname;
{
	int i, j;

	while ((i = fread(&bufpair[0][0], sizeof(char), READBUFSIZE, in))
	    == READBUFSIZE) {
		des_ecb_buffer((des_cblock *)&bufpair[0][0],
		    (des_cblock *)&bufpair[1][0], READBUFSIZE, schedule, 1);
		(void) fwrite(&bufpair[1][0], sizeof(char), READBUFSIZE, out);
	}

	/*
	 * Check for errors.
	 */
	if (i < 0) {
		(void) fprintf(stderr, "%s: read %s: ", progname, inname);
		perror("");
		exit(1);
	}

	/*
	 * No error.  Pad length out to multiple of 8 and put number
	 * of nonuseless characters out of the last 8 in last byte.
	 */
	j = ((i + 8) & ~(0x7));
	bufpair[0][j-1] = (unsigned char)(8 - (j-i));
	des_ecb_buffer((des_cblock *)&bufpair[0][0],
	    (des_cblock *)&bufpair[1][0], j, schedule, 1);
	(void) fwrite(&bufpair[1][0], sizeof(char), j, out);
}


/*
 * do_ecb_decrypt - we don't do this either
 */
static void
do_ecb_decrypt(schedule, in, inname, out, outname)
	des_key_schedule schedule;
	FILE *in;
	char *inname;
	FILE *out;
	char *outname;
{
	int i, j;

	while ((i = fread(&bufpair[0][0], sizeof(char), READBUFSIZE, in))
	    == READBUFSIZE) {
		des_ecb_buffer((des_cblock *)&bufpair[0][0],
		    (des_cblock *)&bufpair[1][0], READBUFSIZE, schedule, 0);
		(void) fwrite(&bufpair[1][0], sizeof(char), READBUFSIZE, out);
	}

	/*
	 * Check for errors.
	 */
	if (i < 0) {
		(void) fprintf(stderr, "%s: read %s: ", progname, inname);
		perror("");
		exit(1);
	}

	/*
	 * No error.  Make sure we've got a multiple of 8 characters
	 * left or we're in trouble.
	 */
	if (i & 0x7) {
		(void) fprintf(stderr,
"%s: warning: file size not multiple of 8 (may not have been encrypted)\n",
		    progname);
		/*
		 * Corrective action.  Force everything to match.
		 */
		j = ((i + 8) & ~(0x7));
		bufpair[0][j-1] = (unsigned char)(8 - (j-i));
	} else {
		j = i;
	}
	des_ecb_buffer((des_cblock *)&bufpair[0][0],
	    (des_cblock *)&bufpair[1][0], i, schedule, 0);
	if ((bufpair[1][j-1] & ~(0x7)) != 0) {
		(void) fprintf(stderr,
	"%s: warning: file format incorrect (may not have been encrypted)\n",
		    progname);
	} else {
		i = (j-8) + (int)bufpair[1][j-1];
	}
	(void) fwrite(&bufpair[1][0], sizeof(char), i, out);
}


/*
 * process - do what we've been told
 */
static void
process(key, do_encrypt, use_mode, in, inname, out, outname)
	unsigned char *key;
	int do_encrypt;
	int use_mode;
	FILE *in;
	char *inname;
	FILE *out;
	char *outname;
{
	des_key_schedule schedule;

	if (des_set_key((des_cblock *)key, schedule) < 0) {
		(void) fprintf(stderr,
		    "%s: hex key parity incorrect or key is weak\n",
		    progname);
		exit(1);
	}

	if (do_encrypt) {
		switch (use_mode) {
		case MODE_CBC:
			do_cbc_encrypt(schedule, in, inname, out, outname);
			break;
		case MODE_ECB:
			do_ecb_encrypt(schedule, in, inname, out, outname);
			break;
		case MODE_PCBC:
			do_pcbc_encrypt(schedule, in, inname, out, outname);
			break;
		}
	} else {
		switch (use_mode) {
		case MODE_CBC:
			do_cbc_decrypt(schedule, in, inname, out, outname);
			break;
		case MODE_ECB:
			do_ecb_decrypt(schedule, in, inname, out, outname);
			break;
		case MODE_PCBC:
			do_pcbc_decrypt(schedule, in, inname, out, outname);
			break;
		}
	}
}


/*
 * gethexkey - turn an ascii string into a hex key
 */
static int
gethexkey(str, key)
	char *str;
	unsigned char *key;
{
	register char *cp;
	register unsigned char *kp;
	register int i;

	cp = str + strlen(str);
	kp = key;
	while (kp < key+8)
		*kp++ = 0x1;

	while (cp > str && kp > key) {
		i = ((int)*(--cp)) & 0x7f;
		if (!isxdigit(i))
			goto badchar;
		*(--kp) = (unsigned char)TOHEX(i);
		if (cp == str)
			break;
		i = ((int)*(--cp)) & 0x7f;
		if (!isxdigit(i))
			goto badchar;
		*kp |= (unsigned char)(TOHEX(i) << 4);
	}

	if (kp == key && cp != str) {
		(void) fprintf(stderr, "%s: excess hex digits specified\n",
		    progname);
		return 0;
	}

	return 1;

badchar:
	(void) fprintf(stderr, "%s: illegal hex character in key\n", progname);
	return 0;
}


/*
 * Odd parity table
 */
static unsigned char odd_parity_table[256] = {
	0x80, 0x01, 0x02, 0x83, 0x04, 0x85, 0x86, 0x07,
	0x08, 0x89, 0x8a, 0x0b, 0x8c, 0x0d, 0x0e, 0x8f,
	0x10, 0x91, 0x92, 0x13, 0x94, 0x15, 0x16, 0x97,
	0x98, 0x19, 0x1a, 0x9b, 0x1c, 0x9d, 0x9e, 0x1f,
	0x20, 0xa1, 0xa2, 0x23, 0xa4, 0x25, 0x26, 0xa7,
	0xa8, 0x29, 0x2a, 0xab, 0x2c, 0xad, 0xae, 0x2f,
	0xb0, 0x31, 0x32, 0xb3, 0x34, 0xb5, 0xb6, 0x37,
	0x38, 0xb9, 0xba, 0x3b, 0xbc, 0x3d, 0x3e, 0xbf,
	0x40, 0xc1, 0xc2, 0x43, 0xc4, 0x45, 0x46, 0xc7,
	0xc8, 0x49, 0x4a, 0xcb, 0x4c, 0xcd, 0xce, 0x4f,
	0xd0, 0x51, 0x52, 0xd3, 0x54, 0xd5, 0xd6, 0x57,
	0x58, 0xd9, 0xda, 0x5b, 0xdc, 0x5d, 0x5e, 0xdf,
	0xe0, 0x61, 0x62, 0xe3, 0x64, 0xe5, 0xe6, 0x67,
	0x68, 0xe9, 0xea, 0x6b, 0xec, 0x6d, 0x6e, 0xef,
	0x70, 0xf1, 0xf2, 0x73, 0xf4, 0x75, 0x76, 0xf7,
	0xf8, 0x79, 0x7a, 0xfb, 0x7c, 0xfd, 0xfe, 0x7f,
	0x80, 0x01, 0x02, 0x83, 0x04, 0x85, 0x86, 0x07,
	0x08, 0x89, 0x8a, 0x0b, 0x8c, 0x0d, 0x0e, 0x8f,
	0x10, 0x91, 0x92, 0x13, 0x94, 0x15, 0x16, 0x97,
	0x98, 0x19, 0x1a, 0x9b, 0x1c, 0x9d, 0x9e, 0x1f,
	0x20, 0xa1, 0xa2, 0x23, 0xa4, 0x25, 0x26, 0xa7,
	0xa8, 0x29, 0x2a, 0xab, 0x2c, 0xad, 0xae, 0x2f,
	0xb0, 0x31, 0x32, 0xb3, 0x34, 0xb5, 0xb6, 0x37,
	0x38, 0xb9, 0xba, 0x3b, 0xbc, 0x3d, 0x3e, 0xbf,
	0x40, 0xc1, 0xc2, 0x43, 0xc4, 0x45, 0x46, 0xc7,
	0xc8, 0x49, 0x4a, 0xcb, 0x4c, 0xcd, 0xce, 0x4f,
	0xd0, 0x51, 0x52, 0xd3, 0x54, 0xd5, 0xd6, 0x57,
	0x58, 0xd9, 0xda, 0x5b, 0xdc, 0x5d, 0x5e, 0xdf,
	0xe0, 0x61, 0x62, 0xe3, 0x64, 0xe5, 0xe6, 0x67,
	0x68, 0xe9, 0xea, 0x6b, 0xec, 0x6d, 0x6e, 0xef,
	0x70, 0xf1, 0xf2, 0x73, 0xf4, 0x75, 0x76, 0xf7,
	0xf8, 0x79, 0x7a, 0xfb, 0x7c, 0xfd, 0xfe, 0x7f
};


/*
 * key_set_parity - make an ascii key odd parity
 */
static void
key_set_parity(key, len)
	unsigned char *key;
	int len;
{
	register unsigned char *kp, *kpo;

	kp = kpo = key;
	while (len-- > 0)
		*kp++ = odd_parity_table[*kpo++];
}
