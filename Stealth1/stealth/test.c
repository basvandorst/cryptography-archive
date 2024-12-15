
/*
 * Frigtest V1.0 by Henry Hastur
 *
 * This code is copyright Henry Hastur 1994, but may be freely distributed,
 * modified, incorporated into other programs and used, as long as the
 * copyright stays attached and you obey any relevant import or export
 * restrictions on the code. No warranty is offered, and no responsibility
 * is taken for any damage use of this program may cause. In other words,
 * do what you want with it, but don't expect me to pay up if anything
 * unexpected goes wrong - you're using it at your own risk...
 *
 * All this program does is to allow you to run a few statistical tests
 * on the output of the frigblock () function. It may be useful to you if
 * you change the arrays in conv.c or want to cryptanalyse the frigblock ()
 * function.
 *
 */

#ifdef FRIG

#include "ext.h"
#include "frig.h"

static	byte	block [BLOCK_SIZE];
static	byte	old [BLOCK_SIZE];
static	byte	key [KEY_SIZE];
static	byte	key_entered [KEY_SIZE];
static	int	random_size = BLOCK_SIZE;

static	setkey ()

{
	int	i;

	for (i = 0; i < KEY_SIZE; i++)
		key [i] = key_entered [i];
}

static	fillblock (b)

byte	*b;

{
	int	i;

	for (i = 0; i < random_size; i++)
		*b++ = random () & 0xFF;

	for (;i < BLOCK_SIZE; i++)
		*b++ = 0;
}

static	dumpkey()

{
	int	i;

	printf("Key: ");
	for (i = 0; i < KEY_SIZE; i++)
		printf("%02X", key [i]);

	putchar ('\n');
}

static	countbits (b, d)

byte	b, d;

{
	byte	m;
	int	c, i;

	c = 0;
	m = 0x80;
	for (i = 0; i < 8; i++) {
		if ((b & m) == (d & m))
			c++;
		m >> = 1;
	}

	return c;
}

static	dumpblock ()

{
	byte	*s;
	int	i, bits = 0;

	s = (byte *) block;

	printf ("Block is :\n");
	for (i = 0; i < BLOCK_SIZE; s++,i++) {
		printf ("%02X",*s);
		bits += countbits (*s, old [i]);
		if ((i&0x1f) == 0x1f)
			putchar ('\n');
	}
	putchar ('\n');
	printf("Matching bits in entire block   : %d, %d percent\n", bits, 
		(100 * bits)/BLOCK_SIZE_BITS);

	s = block;
	bits = 0;

	for (i = 0; i < 16; s++,i++) {
		bits += countbits (*s, old [i]);
	}

	printf ("Matching bits in first 16 bytes : %d, %d percent\n",
		bits, (100 * bits) / (16*8));
	putchar ('\n');
}

big_test ()

{
	int	i, j, l;
	byte	*s, m;
	long	t, t1;

printf ("Big Test\n");
	setkey ();
	s = (byte *) block;
	for (i = 0; i < 40; i++) {
		s [i] = old[i] = random () & 0xFF;
	}

	l = 0;
	m = 0x80;

#define LOOPS 16384

initfrig (key, FALSE);
t = time (0);
for (i = 0; i < LOOPS; i++) {
	frigblock (block);
}
t1 = time (0);

printf("Frigged %dk in %ld seconds - %dk per second.\n", (LOOPS * 64)/1024,
	 t1 - t, (LOOPS*64)/ ((1024)*(int)(t1 - t)));

	dumpblock ();
for (i = 0; i < KEY_SIZE_BITS; i++) {

	/* Copy original over */

	s = (byte *) block;
	for (j = 0; j < BLOCK_SIZE; j++)
		*s++ = old [j];

	/* Encrypt */

	initfrig (key, FALSE);

	frigblock (block);
	dumpkey ();
	dumpblock ();

	/* Decrypt test */

	initfrig (key, TRUE);
	frigblock (block);
	dumpblock ();

	/* Update key */

	setkey ();
	if (!m) {
		m = 0x80;
		l++;
	}

	key [l] ^= m;
	m >>= 1;
}
}

static	random_key_test ()

{
	int	i, l;
	byte	encrypted [BLOCK_SIZE];

	printf ("Decryption with random key test\n");

	setkey ();

	fillblock (old);

	while (i < BLOCK_SIZE)
		old [i++] = 0;

	for (i = 0; i < BLOCK_SIZE; i++)
		encrypted [i] = old [i];

	printf ("\nFirst create random block\n");

	for (i = 0; i < BLOCK_SIZE; i++)
		block [i] = old [i];

	dumpblock ();

	printf ("\nSecond encrypt it\n");

	initfrig (key, FALSE);
	frigblock (encrypted);

	for (i = 0; i < BLOCK_SIZE; i++)
		block [i] = encrypted [i];

	dumpkey ();
	dumpblock ();

	printf ("\nNow decrypt with 256 random keys and compare the output \n");

	for (l=0; l < 256; l++) {
		for (i = 0; i < BLOCK_SIZE; i++)
			block [i] = encrypted [i];

		for (i = 0; i < KEY_SIZE; i++)
			key [i] = random ();

		initfrig (key, TRUE);

		frigblock (block);
		dumpkey ();
		dumpblock ();
	}
}

static	key_test ()

{
	int	i, l;
	byte	m;
	byte	encrypted [BLOCK_SIZE];

	printf ("Decryption with single bit incorrect key test\n");

	setkey ();

	fillblock (old);

	while (i < BLOCK_SIZE)
		old [i++] = 0;

	for (i = 0; i < BLOCK_SIZE; i++)
		encrypted [i] = old [i];

	initfrig (key, FALSE);
	frigblock (encrypted);

	l = 0;
	m = 0x80;
	while (l < KEY_SIZE) {
		for (i = 0; i < BLOCK_SIZE; i++)
			block [i] = encrypted [i];

		setkey ();
		key [l] ^= m;

		m >>= 1;
		if (!m) {
			m = 0x80;
			l++;
		}

		initfrig (key, TRUE);

		frigblock (block);
		dumpkey ();
		dumpblock ();
	}
}

static	diff_text_test ()

{
	int	i, l;
	byte	m;
	byte	plain [BLOCK_SIZE];

	printf ("Here we change a single bit of the plaintext and encrpyt, comparing\n");
	printf ("the output to the output of the last encryption.\n\n");

	fillblock (old);
	for (i = 0; i < BLOCK_SIZE; i++) 
		plain [i] = old [i];

	m = 0x80;
	l = 0;

	setkey ();

	old [l] ^= m;
	m >>= 1;

	initfrig (key, FALSE);
	frigblock (old);

	while (l < BLOCK_SIZE) {
		for (i = 0; i < BLOCK_SIZE; i++)
			block [i] = plain [i];

		block [l] ^= m;
		m >>= 1;

		if (!m) {
			l++;
			m = 0x80;
		}

		frigblock (block);

		dumpkey ();
		dumpblock ();

		for (i = 0; i < BLOCK_SIZE; i++)
			old [i] = block [i];
	}
}
static	diff_key_test ()

{
	int	i, l;
	byte	m;
	byte	plain [BLOCK_SIZE];

	printf ("Here we change a single bit of the key and encrpyt, comparing\n");
	printf ("the output to the output of the last encryption.\n\n");

	fillblock (old);
	for (i = 0; i < BLOCK_SIZE; i++) 
		plain [i] = old [i];

	m = 0x80;
	l = 0;

	setkey ();

	key [l] ^= m;
	m >>= 1;

	initfrig (key, FALSE);
	frigblock (old);

	while (l < KEY_SIZE) {
		setkey ();

		key [l] ^= m;
		m >>= 1;

		if (!m) {
			l++;
			m = 0x80;
		}

		for (i = 0; i < BLOCK_SIZE; i++)
			block [i] = plain [i];

		initfrig (key, FALSE);
		frigblock (block);

		dumpkey ();
		dumpblock ();

		for (i = 0; i < BLOCK_SIZE; i++)
			old [i] = block [i];
	}
}

static	encrypt_test ()

{
	int	i, l;
	byte	m;

	fillblock (old);

	m = 0x80;
	l = 0;

	while (l < KEY_SIZE) {
		setkey ();

		key [l] ^= m;
		m >>= 1;

		if (!m) {
			l++;
			m = 0x80;
		}

		for (i = 0; i < BLOCK_SIZE; i++)
			block [i] = old [i];

		initfrig (key, FALSE);
		frigblock (block);

		dumpkey ();
		dumpblock ();
	}
}

static	usage ()

{
	printf ("Usage : frigtest [insert] [0xkey] -[arguments]\n\n");
	printf ("\tValid arguments are :\n");
	printf ("\tb\tBig Test\n");
	printf ("\te\tEncrypt with single bit key change\n");
	printf ("\tf\tEncrypt and compare encrypted output\n");
	printf ("\th\tThis message\n");
	printf ("\tk\tDecryption with single bit key change\n");
	printf ("\tp\tEncrpyt with single bit plaintext change\n");
	printf ("\tr\tDecryption with random key\n");
	printf ("\n\nTo specify the key to use, enter it in hex, prefixed\n");
	printf ("with 0x. 'insert' specified as a decimal number < %d\n",
		BLOCK_SIZE);
	printf ("will specify the number of zero bytes to pad the test\n");
	printf ("blocks with.\n");
}

static	int	hex_digit (c)

char	c;

{
	if (c < '0')
		return 0;
	if (c > 'Z')
		c = c - ('a' - 'A');

	if (c > '9')
		return (c - 'A' + 10);
	else
		return (c - '0');
}

main (argc, argv)

int	argc;
char	*argv [];

{
	int	arg = 1;
	int	i, j;

	srandom (time (0));
	memset (key_entered, 0xAA, KEY_SIZE);

	while (arg < argc) {
		if (*argv [arg] == '-') {
			while (*++argv[arg]) {
				switch (*argv[arg]) {

					case 'b':
					big_test ();
					break;

					case 'e':
					encrypt_test ();
					break;

					case 'f':
					diff_key_test ();
					break;

					case 'h':
					usage ();
					break;
	
					case 'k':
					key_test ();
					break;

					case 'p':
					diff_text_test ();
					break;

					case 'r':
					random_key_test ();
					break;

				}
			}
		}
		else if (!strncmp (argv[arg], "0x", 2)) {
			memset (key_entered, 0, KEY_SIZE);

			argv [arg] ++;
			i = 0;
			j = (KEY_SIZE*2+1) - strlen (argv[arg]);
			while (*++argv [arg] && j < KEY_SIZE * 2) {
				i <<= 4;
				i += hex_digit (*argv [arg]);
				if (j & 1) {
					key_entered [j / 2] = i;
				}
				j++;
			}
		}
		else {
			random_size = BLOCK_SIZE - atoi (argv [arg]);
			if (random_size < 1)
				random_size = 1;
			if (random_size > BLOCK_SIZE)
				random_size = BLOCK_SIZE;
		}

		arg++;
	}

	closefrig ();
}

#endif /* FRIG */

