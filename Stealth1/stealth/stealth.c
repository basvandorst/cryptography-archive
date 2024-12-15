
/*
 * Stealth V1.2 by Henry Hastur
 *
 * May or may not be covered by US ITAR encryption export regulations, if
 * in doubt, don't export it. It would be pretty stupid if it was, but
 * hey, governments have done some pretty stupid things before now....
 *
 * This program is copyright Henry Hastur 1994, but may be freely distributed,
 * modified, incorporated into other programs and used, as long as the
 * copyright stays attached and you obey any relevant import or export
 * restrictions on the code. No warranty is offered, and no responsibility
 * is taken for any damage use of this program may cause. In other words,
 * do what you want with it, but don't expect me to pay up if anything
 * unexpected goes wrong - you're using it at your own risk...
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#ifndef DOS
#include <unistd.h>
#else
#include <fcntl.h>
#include <io.h>
#endif
#include <signal.h>

/* Few definitions from PGP for its header/algorithm versions */

#define CURRENT_VERSION		0x02
#define RSA_VERSION		0x01
#define ID_SIZE			8

#include "ext.h"
#ifdef FRIG
#include "frig.h"
#endif

/* Few global variables */

static	int	verbose = FALSE;
static	int	conventional = FALSE;
static	int	adding = FALSE;
static	char	file_name [1024];
static	int	file_open = FALSE;
static	FILE	*afp;
#ifdef FRIG
static	byte	frig = FALSE;
static	byte	frigkey [KEY_SIZE];
static	byte	line_key [KEY_SIZE];
static	int	frig_factor = 0;

/* We need one byte of slack for both of these */

static	byte	frig_block [BLOCK_SIZE+1];
static	byte	buf [BLOCK_SIZE+1];
static	int	frig_pointer = 0;
static	byte	insert_zeros = FALSE;
static	int	insert_number = 0;
static	int	bits_to_use = BLOCK_SIZE;

static	int	rot_left = 0;
static	int	bits_read = 0;
static	byte	last_byte;
static	byte	key_read = FALSE;


/* Take a character, block it up, and output the result after passing it
   through the frig code */

static	void	frig_put (c, fp)

byte	c;
FILE	*fp;

{
	byte	b;
	int	i;
	int	mask;

	/* First the easy bit, no frig_factor to deal with */

	if (!frig_factor || frig_factor == 50) {
		frig_block [frig_pointer++] = c;
		if (frig_pointer == BLOCK_SIZE) {
			frigblock (frig_block);
			fwrite (frig_block, BLOCK_SIZE, 1, fp);
			frig_pointer = 0;
		}
		return;
	}

	/* Ah, now we actually need to create a suitable block.. */

	frig_block [frig_pointer++] = c;
	bits_read += 8;

	/* No need to output */

	if (bits_read < bits_to_use)
		return;

	/* Fill up - twiddle last byte appropriately, and fill the rest */

	mask = (0xFF << (bits_read - bits_to_use)) & 0xFF;

	if (insert_zeros) {
		b = 0;
		frig_block [frig_pointer-1] &= mask;
	}
	else {
		b = 0xFF;
		frig_block [frig_pointer-1] |= ~mask;
	}

	while (frig_pointer <= BLOCK_SIZE)
		frig_block [frig_pointer++] = b;

	if (rot_left) {
		buf[0] = (last_byte << (8-rot_left));

		for (i = 0; i < BLOCK_SIZE; i++) {
			buf [i] |= (frig_block[i] >> rot_left);
			buf [i+1] = (frig_block [i] << (8-rot_left));
		}
	}
	else
		for (i = 0; i < BLOCK_SIZE; i++)
			buf [i] = frig_block [i];

	/* Finally, store away some useful information, and write it out */

	last_byte = c;
	rot_left = bits_read - bits_to_use;
	bits_read = rot_left;

	frigblock (buf);
	fwrite (buf, BLOCK_SIZE, 1, fp);
	frig_pointer = 0;
}

/* Read a block, decrypt with frig, and return each character */

static	int	frig_get (fp)

{
	int	c;
	byte	b;
	int	i;

	/* First the easy bit, no frig_factor to deal with */

	if (!frig_factor || frig_factor == 50) {
		if (!frig_pointer) {
			if (!fread (frig_block, BLOCK_SIZE, 1, fp))
				return EOF;
			frigblock (frig_block);
			return frig_block[frig_pointer++];
		}

		c = frig_block [frig_pointer++];
		frig_pointer &= (BLOCK_SIZE-1);

		return c;
	}

	/* Now do the nasty code */

	if (bits_read < 8) {
		if (!fread (buf, BLOCK_SIZE, 1, fp))
			return EOF;

		frigblock (buf);

		if (bits_read) {
			frig_block [0] = frig_block[frig_pointer] & (0xFF << (8-bits_read));
			for (i = 0; i < BLOCK_SIZE; i++) {
				frig_block [i] |= (buf[i] >> bits_read);
				frig_block [i+1] = (buf [i] << (8-bits_read));
			}

			bits_read += bits_to_use;
		}
		else {
			for (i = 0; i < BLOCK_SIZE; i++) {
				frig_block [i] = buf [i];
			}
			bits_read = bits_to_use;
		}

		frig_pointer = 0;
	}

	bits_read -= 8;
	return frig_block [frig_pointer++];
}

/* Initialise the variables that we need */

static	init_frig_factor (n)

int	n;

{

	/* Fast case, don't insert anything */

	if (n == 50 || !n)
		return;

	/* Set frig_factor */

	frig_factor = n;

	/* n is fraction of ones to insert */

	if (n < 50) {
		insert_zeros = TRUE;
		n = (100 - n);
	}

	/* Calculate number of bits to insert */

	insert_number = ((2*n - 100) * BLOCK_SIZE_BITS) / 100;

	/* Out of range ? */

	if (insert_number < 0 || insert_number >= BLOCK_SIZE_BITS) {
		frig_factor = 0;
		return;
	}

	bits_to_use = BLOCK_SIZE_BITS - insert_number;
}

/* Flush remaining bytes to file, passing through frig */

static	void	frig_flush (fp)

{
	if (frig_pointer) {
		byte	m, b;
		int	i;

		/* Append garbage */

		if (!frig_factor || frig_factor == 50) {

			/* Easy, use random() for 50:50 distribution */

			while (frig_pointer < BLOCK_SIZE)
				frig_block[frig_pointer++] = 
					(random () & 0xFF);
		}
		else {

			while (frig_pointer < (bits_to_use / 8)) {
				frig_block [frig_pointer++] =
					(random () & 0xFF);
			}

			/* Generate correct-ish frequency distribution */

			while (frig_pointer < BLOCK_SIZE) {
				m = 0x80;
				b = 0;
				while (m) {
					if ((random () % 100) < frig_factor)
						b |= m;
					m >>= 1;
				}

				frig_block[frig_pointer++] = b;
			}
		}

		if (rot_left) {
			buf[0] = (last_byte << (8-rot_left));

			for (i = 0; i < BLOCK_SIZE; i++) {
				buf [i] |= (frig_block[i] >> rot_left);
				buf [i+1] = (frig_block [i] << (8-rot_left));
			}
		}
		else
			for (i = 0; i < BLOCK_SIZE; i++)
				buf [i] = frig_block [i];

		/* Write it out */

		frigblock (buf);
		fwrite (buf, BLOCK_SIZE, 1, fp);
		frig_pointer = 0;
	}
}
#endif

/* int_handler() - tidy up and exit */

static	void	int_handler(unused)

int	unused;

{
	long	fpos;

	/* If we've still got a file open */

	if (file_open) {

		/* If we can still write to it, erase it */ 

		if (afp) {
			fseek (afp, 0l, 2);
			fpos = ftell (afp);
			fseek (afp, 0l, 0);

			while (fpos --) 
				putc (0, afp);
		}

#ifndef UNIX
		/* Finally unlink it */

		unlink (file_name);
#endif
	}

	exit (1);
}

/* Read a PGP ctb-lengh */

static	long	read_length (c,fp)

FILE	*fp;
int	c;

{
	static	int	bytes [] = { 1, 2, 4, 0 };
	long	len = 0;
	int	n;

	n = bytes [c & 0x03];

	if (!n)
		return 0x7FFFFFFF;

	for (; n > 0 ; n--) {
		len *= 256;
		len += getc(fp);
	}

	return len;
}

/* Write a PGP ctb-length */

static	void	write_length (ctb,length,fp)

int	ctb;
unsigned long	length;
FILE	*fp;

{
	unsigned long	mask;
	int	bytes, shift,c;

	ctb &= 0xFC;

	if (length < 256) {
		mask = 0xFF;
		bytes = 1;
		shift = 0;
	}
	else if (length < 65536) {
		mask = 0xFF00;
		bytes = 2;
		shift = 8;
		ctb |= 1;
	}
	else {
		mask = 0xFF000000;
		shift = 24;
		bytes = 4;
		ctb |= 2;
	}

	putc (ctb, fp);

	while (bytes-- > 0) {
		c = ((length & mask) >> shift);
		mask >>= 8;
		shift -= 8;

		putc (c, fp);
	}
}

/* Convert char to hex value */

static	int	hex_digit (c)

char	c;

{
	c = tolower (c);
	if (c >= '0' && c <= '9') {
		return (c - '0');
	}
	else if (c >= 'a' && c <= 'f') {
		return (c + 10 - 'a');
	}
	else {
		fprintf (stderr, "Hex value given with invalid hex digits !\n");
		exit (1);
	}
}

/* Hunt through pubring.pgp for the appropriate secret key */

#define	ID_FROM_NAME	0
#define LENGTH_FROM_ID	1

static	int	find_key_id(id,length,s,type)

char	*s;
byte	*id;
int	*length;
int	type;

{
	char	*path;
	FILE	*pub = NULL;
	int	c1, c2, len, i, klen, c;
	long	hex_id = (-1);
	int	id_size;

	/* Following are static only to reduce DOS stack requirements */

	static	char	pub_name [1024];
	static	char	userid [256];

	/* Find pubring.pgp */

	if (path = getenv("PGPPATH")) {
		sprintf (pub_name, "%s/pubring.pgp", path);
		pub = fopen(pub_name, "rb");
	}

	if (!pub) 
		pub = fopen ("pubring.pgp", "rb");

	if (!pub) {
		fprintf (stderr,"Can't find pubring.pgp in $PGPPATH or . : exiting !\n");
		exit (1);
	}

	id_size = ID_SIZE;
#ifdef FRIG
	if (frig)
		id_size = KEY_SIZE;
#endif

	/* Also allow for use of hex key id */

	if (type == ID_FROM_NAME && !strncmp(s,"0x",2)) {
		char	*i;
		int	c;

		hex_id = 0l;
		i =  s + 2;
		
		while (*i) {
			hex_id <<= 4;
			hex_id += hex_digit (*i++);
		}
	}

	/* Read the contents till we find what we're looking for */

	while ((c1 = getc(pub)) != EOF) {
		c2 = (c1 & 0xBC);

		switch (c2) {

			/* Secret key, probably revocation cert.  */

			case 0x94:
			len = read_length (c1, pub);

			for (; len > 0; len--)
				 (void) getc (pub);

			break;

			/* Public key - grab id and length */

			case 0x98:
			len = read_length (c1, pub);

			for (i = 0; i < 8; i++)
				(void) getc (pub);

			len -= 10;

			/* OK, here we are at the public modulus, get the
			   size from the MPI header */

			klen = getc(pub) * 256;
			klen += getc (pub);

			/* Return the length for the caller to use */

			*length = klen;

			/* We only need the last 64 bits */

			len -= ((klen + 7) / 8);
			i = (((klen + 7) / 8) - id_size);

			/* Skip unneccesary bytes */

			for (; i > 0; i--) {
				(void) getc (pub);
			}

			if (type == ID_FROM_NAME) {
#ifdef FRIG
				if (frig) {
					for (i = ID_SIZE; i < id_size; i++)
						frigkey [i] = getc (pub);
				}
#endif
				for (i = 0; i < ID_SIZE; i++) {
					id [i] = getc(pub);
#ifdef FRIG
					frigkey [i] = id [i];
#endif
				}
			}
			else {

				/* Looking for length from ID */

				int	found_id = TRUE;
#ifdef FRIG
				if (frig) {
					for (i = ID_SIZE; i < id_size; i++)
						frigkey [i] = getc (pub);
				}
#endif

				for (i = 0; i < ID_SIZE; i++) {
#ifdef FRIG
					if (id[i] != (frigkey [i] = getc(pub)))
#else
					if (id[i] != getc(pub))
#endif
						found_id = FALSE;
				}

				if (found_id) {
					fclose (pub);
					return TRUE;
				}
			}

			for (; len > 0 ; len--) {
				(void) getc (pub);
			}

			break;

			/* Keyring trust, comment */

			case 0xB0:
			case 0xB8:

			len = getc (pub);

			for (; len > 0; len--)
				(void) getc (pub);

			break;

			/* USER ID ! */

			case 0xB4:

			len = getc (pub);

			if (type == ID_FROM_NAME) {
				for (i = 0; i < len; i++) {
					c = getc (pub);
					userid [i] = tolower (c);
				}

				userid [i] = 0;

				if (strstr(userid, s)) {
					if (verbose)
						fprintf (stderr, "Found user: %s\n",userid);
					return TRUE;
				}

				/* Ok, check for hex id */

				if (hex_id >= 0) {
					long	id_read = 0l;

					for (i = 5; i < 8; i++) {
						id_read <<= 8;
						id_read += id[i];
					}

					if (hex_id == id_read) {
						if (verbose)
							fprintf (stderr, "Found hex id : 0x%06X\n", hex_id);
						return TRUE;
					}
				}
			}
			else
				for (i = 0; i < len; i++)
					(void) getc (pub);
			break;

			/* Anything we don't care about */

			default:
			len = read_length (c1, pub);

			for (; len > 0; len--)
				(void) getc (pub);
			break;

		}
	}

	fclose (pub);

	/* Uh-oh, failed to find it ! */

	return FALSE;
}

/* Strip_headers() : Should be obvious what this does, really ! */

static	void	strip_headers(fp)

FILE	*fp;

{
	int	c1,c2;
	long	len;
	int	i;
	byte	id [ID_SIZE];
	int	key_length;
	byte	key_length_found = FALSE;
	byte	rsa_written = FALSE;
	int	mpi_length;

	/* Run through the whole message checking each packet */

	while ((c1 = getc(fp)) != EOF) {
		c2 = (c1 & 0xBC);

		switch (c2) {

			/* Public key encoded packet */

			case 0x84:

			/* Read length */

			len = read_length(c1, fp);
			if (verbose)
				fprintf (stderr, "Found %d byte RSA packet.\n",
					len);

			/* 
			   We only support ONE RSA block ! This is because
			   we have no idea of the file format when we start
			   adding headers, so we have to assume that this
			   is the case. Warn the user, then abort...
			*/

			if (rsa_written) {
				fprintf (stderr, "WARNING: More than one RSA block found... stripping extra block !\n");

				/* Throw away the block */

ohno_abort_abort:
				while (len-- > 0)
					(void) getc (fp);

				break;
			}

			/* Check for conventional encryption specified */

			if (conventional) {
				fprintf (stderr, "WARNING: You specified conventional encryption with an RSA-encrypted file !\nI hope you know what you're doing... stripping RSA header....\n");

				goto ohno_abort_abort;
			}

			/* Check public key version byte */

			c1 = getc (fp);

			if (c1 != CURRENT_VERSION) {
				fprintf(stderr, "Hmm, PK version %d not %d, may not decrypt at recipient\n", c1, CURRENT_VERSION);
			}

			/* Strip key ID */

			for (i = 0; i < ID_SIZE; i++)
				id[i] =  getc (fp);

			if (find_key_id (id,&key_length,NULL,LENGTH_FROM_ID)) {
				key_length_found = TRUE;
			}

			/* Check RSA version byte */

			c1 = getc(fp);

			if (c1 != RSA_VERSION) {
				fprintf (stderr, "Hmm, RSA version %d not %d, may not decrypt at recipient\n", c1, RSA_VERSION);
			}

			/* Strip MPI prefix */

			mpi_length = getc(fp);
			mpi_length = mpi_length * 256 + getc(fp);

			/* Now, we have a problem in that PGP may generate
			   an RSA block shorter than your key, in which
			   case decryption is likely to fail. Check for
			   this and warn the user ! */

			if (!key_length_found) {
				fprintf (stderr, "Hmm, couldn't get the length of this key, so can't verify that decryption\nwill be successful.\n");
			}
			else {
				if (((mpi_length + 7) / 8) != 
					((key_length + 7) / 8)) {
					fprintf (stderr, "WARNING : Short RSA block output, decryption will probably fail if used !\n");
				}
			}

#ifdef FRIG
			/* Ok, we now have the frig key, encrypt the output */

			if (frig) {
				if (key_read) 
					initfrig (line_key, FALSE);
				else
					initfrig (frigkey, FALSE);
			}
#endif

			/* Copy remaining data from packet */
			
			len -= 12;
#ifdef FRIG
			if (frig) {
			 	for (; len > 0; len--) 
					frig_put (getc (fp), stdout);
			}
			else
#endif
			for (; len > 0; len--)
				putchar (getc(fp));

			rsa_written = TRUE;
			break;

			/* IDEA packet */

			case 0xA4:

			/* Read length */

			len = read_length(c1, fp);
			if (verbose)
				fprintf (stderr, "Found %d byte IDEA packet.\n",
					len);

			/* Copy data from packet */
			
#ifdef FRIG
			if (frig) {
			 	for (; len > 0; len--) 
					frig_put (getc (fp), stdout);
			}
			else
#endif
			for (; len > 0; len--)
				putchar (getc (fp));

			break;

			default:

			/* Oh no ! Don't know what this is - just skip it ! */

			if (verbose)
				fprintf (stderr, "Oops ! Unexpected packet type, skipping !\n");

			len = read_length (c1, fp);

			for (; len > 0; len --)
				(void) getc (fp);

			break;
		}
	}
}

/* Now we put the headers back in again */

static	void	add_headers(id, length)

byte	*id;
int	length;

{
	unsigned long	len, mask;
	int	shift;
	int	i, c;
	long	fpos;
	long	flen;
	int	s;
#ifdef USE_PGPPATH
	char	*pgp_path;
#endif

	/* Foo ! We have to use a temporary file, because we need to be
	   able to output the length after reading it in ! */

#ifdef USE_TMP
	strcpy (file_name, "/tmp/stealth.t");
#else
#ifdef USE_PGPPATH
	pgp_path = getenv ("PGPPATH");

	if (!pgp_path) {
		fprintf (stderr, "PGPPATH not set !\n");
		exit (1);
	}

	sprintf(file_name,"%s/stealth.t",pgp_path);
#else
	strcpy (file_name, "stealth.t");
#endif
#endif

	s = strlen (file_name);

	i = 0;

#ifdef DOS
#define	F_OK	0
#endif

	while (!access (file_name, F_OK) && i < 100) {
		sprintf (file_name + s, "%d", i++);
	}

	afp = fopen (file_name,"w+b");

	if (!afp) {
		fprintf (stderr, "Can't open '%s' !\n", file_name);
		exit (2);
	}

	/* On unix, unlink the file immediately, to improve security */

#ifdef UNIX
	unlink (file_name);
#endif

	file_open = TRUE;

#ifdef FRIG
	if (frig) {
		if (key_read) 
			initfrig (line_key, TRUE);
		else
			initfrig (frigkey, TRUE);
	}
#endif

	if (!conventional) {

		/* First output the PK header */

		len = 4 + ID_SIZE + (length + 7)/8;

		write_length (0x84, len, afp);
		putc (CURRENT_VERSION, afp);

		/* Store the key ID */

		for (i = 0; i < 8; i++) {
			putc (id [i], afp);
		}

		/* RSA version */

		putc (RSA_VERSION, afp);
	
		/* MPI header */

		c = (length & 0xFF00) >> 8;
		putc (c, afp);
		putc (length & 0xFF, afp);

		/* Copy the MPI over */

		i = (length + 7) / 8;
#ifdef FRIG
		if (frig) {
			while (i-- > 0) {
				c = frig_get (stdin);
				putc (c, afp);
			}
		}
		else
#endif
		while (i-- > 0) {
			c = getchar();
			putc (c, afp);
		}

	}

	/* Now the IDEA bits */

	len = 0xFFFFFFFF;

	fpos = ftell (afp) + 1;
	write_length (0xA4, len, afp);

	len = 0;

#ifdef FRIG
	if (frig) {
		while ((c = frig_get (stdin)) != EOF) {
			len ++;
			putc (c, afp);
		}
	}
	else
#endif
	while ((c = getchar ()) != EOF) {
		len ++;
		putc (c, afp);
	}

	fseek (afp, fpos, 0);

	/* Set up mask for length writing */

	mask = 0xFF000000;
	shift = 24;

	/* Write the length back */

	for (i = 0; i < 4; i++) {
		c = (len & mask) >> shift;
		shift -= 8;
		mask >>= 8;

		putc (c, afp);
	}

	/* OK, now let's output the data ! */

	fseek (afp, 0l, 0);

	while ((c = getc (afp)) != EOF) {
		putchar (c);
	}

	/* Erase the file */

	flen = ftell (afp);
	
	fseek (afp, 0l,0);

	while (flen --)
		putc (0, afp);

	fclose (afp);
	afp = NULL;

#ifndef UNIX
	/* Finally, delete the temporary file */

	unlink (file_name);
#endif

	file_open = FALSE;
}

static	char	looking_for [256];

#ifdef FRIG
/* Read key from specified string */

static	void	read_key (s)

char	*s;

{
	int	j;

	memset (line_key, 0, KEY_SIZE);
	j = KEY_SIZE * 2 - strlen (s);

	if (j < 0)
		j = 0;

	while (j < KEY_SIZE * 2 && *s) {
		line_key [j/2] <<= 4;
		line_key [j++/2] += hex_digit (*s++);
	}
}
#endif

/* Do the stuff */

main(argc,argv)

char	*argv[];
int	argc;

{
	int	length;
	byte	id [ID_SIZE];
	char	*s, *d;
	int	arg = 1,i;

	/* Following needed for binary stdin/stdout on DOS */

#ifdef DOS
	_fmode = O_BINARY;
	setmode (fileno(stdin), O_BINARY);
	setmode (fileno(stdout), O_BINARY);
#endif

	/* Set the umask for any files we may create */

	umask (077);

	signal (SIGINT, int_handler);

	/* Check command line parameters */

	while (arg != argc && argv [arg][0] == '-') {

		for (i = 1; argv[arg][i]; i++) {
			switch (argv[arg][i]) {

				case 'v':
				verbose = TRUE;
				break;

				case 'c':
				conventional = TRUE;
				break;

				case 'a':
				adding = TRUE;
				break;

#ifdef FRIG
				case 'f':
				frig = TRUE;
				while (argv[arg+1] && *argv[arg+1] != '-') {
					arg++;
					if (!strncmp (argv[arg], "0x", 2)) {

						/* Got a key */

						read_key (argv [arg] + 2);
						key_read = TRUE;
					}
					else {
						init_frig_factor (atoi (argv[arg]));
					}
				}
				goto	end_loop;
#endif

			}
		}
#ifdef FRIG
	end_loop:
#endif

		arg++;
	}

#ifdef FRIG
	if (frig && conventional && !key_read) {
		printf ("Can't frig conventional input without key !\n");
		exit (1);
	}
#endif

	if (!adding)
		strip_headers (stdin);
	else {
		if (!conventional) {

			if (arg == argc) {
				fprintf (stderr, "You specified -a, but gave no user id !\n");
				exit (1);
			}

			s = argv[arg];
			d = looking_for;

			while (*s) {
				*d++ = tolower (*s);
				s++;
			}
			*d = 0;
		}

		if (conventional || 
			find_key_id (id,&length,looking_for,ID_FROM_NAME)) {
			add_headers (id, length);
		}
		else {
			fprintf (stderr, "Can't find key for user %s\n",argv[arg]);
		}
	}

#ifdef FRIG
	/* Tidy up */

	if (frig) {
		if (!adding)
			frig_flush (stdout);
		closefrig();
	}

	memset (frigkey, 0, KEY_SIZE);
	memset (line_key, 0, KEY_SIZE);
	memset (frig_block, 0, BLOCK_SIZE+1);
	memset (buf, 0, BLOCK_SIZE+1);
#endif
}

