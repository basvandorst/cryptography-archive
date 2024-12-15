/*
 * - BETA - BETA - BETA - BETA - BETA - BETA - BETA - BETA - BETA - BETA -
 *
 * Don't use in earnest, until the 2.01 additions are debugged more thoroughly
 *
 * - BETA - BETA - BETA - BETA - BETA - BETA - BETA - BETA - BETA - BETA -
 *
 *   - Adam
 */

/* 
 * Stealth 2.01 beta (by Henry Hastur see v1.1 comments below)
 *
 * Stealth 2.0 mods by Adam Back:
 * 
 * Improved security
 *
 *   Added code to make RSA encrypted IDEA session key have a uniform
 *   distribution.  This is important as v1.1 and below leaked information,
 *   a knowledgeable opponent could determine with a high degree of certainty
 *   that the message was RSA encrypted given many stealthed files.
 *
 *   The new code makes this much more secure.  I believe this to
 *   be very secure against analysis, the whole output should not be 
 *   distinguishable from white noise now.
 *
 * Random No generation
 *
 *   The improved security code described above needs to generate random
 *   numbers.  It is important that we have a decent random number generator.
 *   What I have done about this is to use the MD5 digest of PGPs ranseed.bin
 *   with a few words of pseudo random stuff from rand() stirred in.
 *
 *   This is the weak point at the moment as in the event an opponent
 *   captures your system and the randseed.bin before you run PGP again
 *   (PGP stirs randseed.bin after each use) proof of authorship is given.
 *
 *   If we could get this folded into PGP 3.0 could use PGP's ran no 
 *   facility, and stir randseed.bin itself.
 *
 *   Of course if you're using this program in a country where encryption
 *   is outlawed, then being found with a randseed.bin, together with
 *   stealth and pgp executables would be enough to get you in trouble
 *   to start with.
 *
 *   The moral of the story:
 *
 *   If this is critical keep your software secure, on a floppy disk 
 *   or something, and if in doubt burn the disk.
 *
 *   To avoid the proof of authorship problem:
 *
 *   Always run PGP once more after using stealth to stir randseed.bin.
 *
 * Also updated keyids for PGP version 2.6.x & Viacrypt 2.7
 *
 *   Now also understands longer 32 bit keyids for searching public keyring
 *   (the move from 24 to 32 was made at PGP 2.5)
 *
 * Updated output format for PGP >= 2.5
 *
 *   Can still get old pgp < 2.5 output format with -2 flag, in case you 
 *   only have pgp2.3 (true on some platforms still)
 *
 * Changed to avoid temporary file
 *
 *   Now read IDEA packet into memory to avoid temp file.  
 *
 *   Will fail if you don't have enough memory (or file size is over 1 meg),
 *   unless you enable temp file usage with -t.
 *
 *   1 meg limit is to avoid swapping.  A wiped temp file is probably
 *   better than swapping.
 *
 * Program by Henry Hastur, v2.0 updates by me (Adam Back)
 *
 * Adam Back <aba@dcs.ex.ac.uk> pgp-key: 0x556A4A67 on keyservers
 *
 */

/*
 * Stealth V1.1 by Henry Hastur
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

/* 
 *  Comment this out to get stealth v1.1, all stealth2 mods over stealth1.1
 *  are ifdefed STEALTH2
 *
 *  - Adam
 *
 */

#define STEALTH2

#ifdef STEALTH2
#include <sys/time.h>
#include "mpilib.h"
#include "global.h"
#include "md5.h"
#endif

/* Few definitions from PGP for its header/algorithm versions */

#ifdef STEALTH2

/*
 *  number of bits to use for random number 
 *
 *  This should be sufficient security as to exploit this 64 bit key
 *  space to gain a statistical proof of ownership the opponent would
 *  need > 2**64 messages.
 *
 */

#define MARGIN_BITS 64

#define MAX_PREC ((MAX_BIT_PRECISION+MARGIN_BITS+UNITSIZE - 1)/UNITSIZE*2)
typedef unit bigint[MAX_PREC];
typedef unsigned char bigmpi[MAX_PREC * UNITSIZE / 8];

#define OLD_VERSION 0x02
#define CURRENT_VERSION 0x03

#else

#define CURRENT_VERSION 0x03

#endif

#define RSA_VERSION		0x01
#define ID_SIZE			8

/* already get these from MPILIB header "usuals.h" with STEALTH2 */

#ifndef STEALTH2

/* define TRUE and FALSE */

#define	TRUE	1
#define FALSE	0

/* A byte */

typedef	unsigned char	byte;

#endif

/* Few global variables */

static	int	verbose = FALSE;
static	int	conventional = FALSE;
static	int	adding = FALSE;
static	char	file_name [1024];
static	int	file_open = FALSE;
static	FILE	*afp;

#ifdef STEALTH2

/* 
 * need to keep the modulus around so that we can scale the RSA
 * encrypted IDEA session key accordingly
 *
 */

static bigint	n;
static bigmpi	n_mpi;
static int	pgp_version = CURRENT_VERSION;
static int	allow_temp  = FALSE;
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

#ifdef STEALTH2
#ifdef DEBUG
static  char*	to_hex(num)
unitptr num;
{
	static	char result[(MAX_BIT_PRECISION*8+MARGIN_BITS+7)/4+1+100];
	static  bigmpi mpi;

	int	i, len, lo, hi;

	len = reg2mpi(mpi,num);
	fprintf(stderr,"[len = %d]\n",len);
	for (i=0; i<len;i++)
	{
		lo = mpi[i+2] & 0xF;
		hi = (mpi[i+2] & 0xF0) >> 4;
		result[i*2] = (hi < 10) ? hi + '0' : hi - 10 + 'A';
		result[i*2+1] = (lo < 10) ? lo + '0' : lo - 10 + 'A';
	}
	result[i*2] = '\0';
	return result;
}
#endif
#endif

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
#ifdef STEALTH2
	int     old_key;     /* boolean used to keep compatibility with */
	                     /* 24 bit keyids for PGP < 2.5 */
#endif

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

	/* Also allow for use of hex key id */

	if (type == ID_FROM_NAME && !strncmp(s,"0x",2)) {
		char	*i;
		int	c;

		hex_id = 0l;
		i =  s + 2;
		
#ifdef STEALTH2
		c = strlen( i );
		if ( c != 6 && c != 8 ) {
		  fprintf(stderr, 
"Hex key must be 8 hex digits (6 as used in PGP < 2.5 also accepted)\n");
		  exit (1);
		}
		old_key = c == 6;
#endif

		while (*i) {
			hex_id <<= 4;
			c = tolower (*i);

			if (c >= '0' && c <= '9') {
				hex_id += (c - '0');
			}
			else if (c >= 'a' && c <= 'f') {
				hex_id += (c + 10 - 'a');
			}
			else {
				fprintf (stderr, "Hex key id given with invalid hex digits !\n");
				exit (1);
			}

			i++;
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
#ifdef STEALTH2
			n_mpi[0] = getc(pub);
			n_mpi[1] = getc(pub);
			klen = n_mpi[0] * 256 + n_mpi[1];
#else
			klen = getc(pub) * 256;
			klen += getc (pub);
#endif


			/* Return the length for the caller to use */

			*length = klen;

			/* We only need the last 64 bits */

			len -= ((klen + 7) / 8);

#ifdef STEALTH2
			/* read the mpi */
			fread( n_mpi + 2, (klen + 7) / 8, 1, pub );

/* dont bother doing mpi2reg(n,n_mpi) here as it might be the wrong key */

#else
			i = (((klen + 7) / 8) - ID_SIZE);

			/* Skip unneccesary bytes */

			for (; i > 0; i--) {
				(void) getc (pub);
			}
#endif
			if (type == ID_FROM_NAME) {
				for (i = 0; i < ID_SIZE; i++) {
#ifdef STEALTH2
				        id[i] = n_mpi[2+(klen+7)/8-ID_SIZE+i];
#else
					id [i] = getc(pub);
#endif
				}
			}
			else {

				/* Looking for length from ID */

				int	found_id = TRUE;

				for (i = 0; i < ID_SIZE; i++) {
#ifdef STEALTH2
				        if (id[i]!=n_mpi[2+(klen+7)/8-ID_SIZE+i])
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

#ifdef STEALTH2
					for (i = old_key ? 5 : 4; i < 8; i++) {
#else
					for (i = 5; i < 8; i++) {
#endif
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

#ifdef STEALTH2

/* re-create MPI length header */

static  void    fix_length(num,num_bytes)

unsigned char*  num;
int      num_bytes;
{
        int    i, val, first_bit, num_bits;

        first_bit = 0;
	for (i=0; i < num_bytes; i++)
	{
	  if ( num[i+2] != 0 )
	  {
	    val = num[i+2];
	    for (; val != 0; first_bit++)
	    {
	      val >>= 1;
	    }
	    break;
	  }
	  first_bit += 8;
	}
	num_bits = num_bytes * 8 - (8 - first_bit);
	num[0] = (num_bits & 0xFF00) >> 8;
	num[1] = num_bits & 0xFF;
}

#endif

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

#ifdef STEALTH2
	/* static to save stack space for DOS */
	static bigint m, top, scale, scale2, ran_no, max_ran, temp, ran_scale;
	static bigmpi msg, ran;
	static unsigned char seed_data[256];
	static char seed_name[512];

	int ran_bytes, mpi_bytes, msg_bytes, top_bits;
	MD5_CTX context;
	FILE* seed;
	unsigned long int ran_words[4];
	struct timeval tv;
	char* path;
#endif
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

#ifdef STEALTH2
			if (c1 != CURRENT_VERSION && c1 != OLD_VERSION) {
#else
			if (c1 != CURRENT_VERSION) {
#endif
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

#ifdef STEALTH2
			msg[0] = getc(fp);
			msg[1] = getc(fp);
			mpi_length = msg[0] * 256 + msg[1];
			mpi_bytes = (mpi_length + 7) / 8;
			fread(msg + 2, mpi_bytes, 1, fp);

/* convert modulus and msg to internal mpi format */

			mpi2reg(m,msg);
			mpi2reg(n,n_mpi);

#ifdef DEBUG
			fprintf(stderr,"m = 0x%s\n",to_hex(m));
			fprintf(stderr,"n = 0x%s\n",to_hex(n));
#endif

/* generate ran no, store in ran_no */

/*
 *  Use the MD5 digest of PGP's randseed.bin for ran no seed
 *
 */

			/* Find pubring.pgp */

			if (path = getenv("PGPPATH")) {
				sprintf (seed_name, "%s/randseed.bin", path);
				seed = fopen(seed_name, "rb");
			}

			if (!seed && (path = getenv("RANDSEED")))
			{
				sprintf (seed_name, "%s/randseed.bin", path);
				seed = fopen(seed_name, "rb");
			}

			if (!seed)
			{
				seed = fopen ("randseed.bin", "rb");
			}

			if (!seed) 
			{
				fprintf (stderr,
"Can't find randseed.bin in $PGPPATH or $RANDSEED . : exiting !\n");
				exit (1);
			}

			MD5Init( &context );
			while (fread(seed_data, 256, 1, seed))
			{
	                        MD5Update( &context, seed_data, 256);
			}

			fclose( seed );
/* 
 *  Okay, here's the manipulation of our number msg in the range [0,n)
 *  into a number in the range [0..top) where 
 *
 *      top = 2**(((mpi_length + MARGIN_BITS +7)/8)*8)
 *
 *  ie 2 raised to the power of (ceil(log2(n)) + MARGIN_BITS) rounded up to
 *  the next multiple of 8 bits
 *
 */

/* top = 2**(mpi_length + MARGIN_BITS rounded up to nearest 8 bits) */

			top_bits = ((mpi_length + MARGIN_BITS + 7)/8)*8;
			mp_init(top,1);
			for (i=0; i<top_bits; i++)
			{
				mp_shift_left(top);
			}
#ifdef DEBUG
			fprintf(stderr,"top = 0x%s\n",to_hex(top));
			fprintf(stderr,"topbits = %d\n",top_bits);
#endif

/* scale = top / n */

			mp_div(temp,scale,top,n);

#ifdef DEBUG
			fprintf(stderr,"scale = 0x%s\n",to_hex(scale));
#endif

/*
 *  use of scale2 described below, precomputed here so the calculation
 *  isn't repeated
 *
 *   a)  scale2 = 2**128 / scale
 *
 */
			mp_init(max_ran,1);
			for (i=0; i<128; i++)
			{
				mp_shift_left(max_ran);
			}
			mp_div(temp,scale2,max_ran,scale);
#ifdef DEBUG
			fprintf(stderr,"scale2 = 0x%s\n",to_hex(scale2));
#endif

/*
 *  two possible failures, when we try again:
 *
 *    i)  the random generation of a number in the range [0..scale) 
 *        as described immediately below
 *
 *    ii) very, very occasionally the whole thing could fail to generate
 *        a number in the range [0..top)
 *
 *  in either of these cases we will jump back here, and try again.
 *
 */

try_again:

/*
 *  used to do
 *
 *    temp = ran_no % scale
 *
 *  but Bodo Moeller pointed out this biases the distribution of the
 *  random number in [0..scale) - this due to wrapping effect of 
 *  modulus operation, the interval [ 0..(2**128 - 1) mod scale ] would
 *  have higher probability of being chosen than the remainder of the
 *  interval ( (2**128 - 1) mod scale .. scale)
 *
 *  Now we do:
 *
 *   a)  scale2 = 2**128 / scale     (code for this above)
 *   b)  ran_no = gen random
 *   c)  ran_scale = ran_no / scale2
 *   d)  if ran_scale >= scale goto b)
 *
 *  this should ensure that if gen random is producing evenly distributed
 *  random digits that temp will be evenly distributed in [0..scale)
 *
 */

/*
 *  Stir in a few random words so that we shouldn't use the same
 *  random no twice if PGP is not called between invocations of stealth
 *
 *  b)  ran_no = gen random
 *
 */
			gettimeofday( &tv, 0 );
			srand( tv.tv_usec );
			for (i=0; i<4; i++)
			{
				ran_words[i] = rand();
			}
			MD5Update( &context, (char*)ran_words, 16 );

	                MD5Final( ran + 2, &context );
        	        fix_length( ran, 16 );
	                mpi2reg( ran_no, ran );
#ifdef DEBUG
			fprintf(stderr,"ran_no = 0x%s\n",to_hex(ran_no));
#endif

/*
 *  c)  ran_scale = ran_no / scale2
 *
 */
			mp_div(temp,ran_scale,ran_no,scale2);

#ifdef DEBUG
			fprintf(stderr,"ran_scale = 0x%s\n",to_hex(ran_scale));
#endif

/*
 *  d)  if ran_scale >= scale goto b)
 *
 */

			if ( mp_compare(ran_scale,scale) >= 0 ) {
#ifdef DEBUG
				fprintf(stderr,
					"ran_scale >= scale, trying again\n");
#endif
				goto try_again;
			}

/* m = m + ran_scale * n */

			mp_mult(temp,ran_scale,n);
			mp_add(m,temp);

#ifdef DEBUG
			fprintf(stderr,"M = 0x%s\n",to_hex(m));
#endif

/* check for m >= top -- very, very unlikely, prob < 1 / 2**MARGIN_BITS */

			if ( mp_compare(m,top) >= 0 ) {
#ifdef DEBUG
				fprintf(stderr,"trying again\n");
#endif
				goto try_again;
			}

/* convert back and output */

			msg_bytes = reg2mpi(msg,m);

/* pad with leading 0's */
			for (i = 0; i < top_bits / 8 - msg_bytes; i++)
			{
			  putchar( 0 );
			}
			fwrite(msg+2, msg_bytes, 1, stdout);
#else
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

			/* Copy remaining data from packet */
			
			len -= 12;
			for (; len > 0; len--)
				putchar (getc(fp));
#endif

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
#ifdef STEALTH2
	int     no_read, msg_bytes, read_all, bytes_read;
	bigint  m, temp;
	bigmpi	msg;

/* allocate buffers in 64 k chunks */

#define BUFFER_SIZE (64 * 1024)

/* upto 16 x 64k = 1meg will be read */

	char*   buffer[16];

/*
 * allocate 4k first so that we have some left if we run out of memory, 
 * a safety precaution in case the C library functions rely on having
 * some heap memory left.
 *
 * If we run out of memory we will free this.
 *
 */
#define CONTINGENCY (4 * 1024)
	char*   contingency;

	int     buffers_used = 0;
#endif

#ifdef USE_PGPPATH
	char	*pgp_path;
#endif

/*
 * with STEALTH2 only create temporary file if insufficient memory
 * to read in
 *
 */

#ifndef STEALTH2
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
#endif

	if (!conventional) {

		/* First output the PK header */

		len = 4 + ID_SIZE + (length + 7)/8;

#ifdef STEALTH2
		write_length (0x84, len, stdout);
		putchar( pgp_version );
#else
		write_length (0x84, len, afp);
		putc (CURRENT_VERSION, afp);
#endif
		/* Store the key ID */

		for (i = 0; i < 8; i++) {
#ifdef STEALTH2
		        putchar( id[i] );
#else
			putc (id [i], afp);
#endif
		}

		/* RSA version */

#ifdef STEALTH2
		putchar( RSA_VERSION );
#else
		putc (RSA_VERSION, afp);
#endif

#ifdef STEALTH2
#ifdef DEBUG		
		fprintf(stderr,"topbits = %d\n",(length + MARGIN_BITS +7)/8*8);
#endif
		msg_bytes = ((length + MARGIN_BITS +7)/8);

/* eat leading zeros */

		no_read = 0;
		do
		{
		  msg[2] = getchar();
		  no_read++;
		} 
		while (msg[2] == 0);

		fread(msg + 3, msg_bytes - no_read, 1, stdin);
		fix_length(msg,msg_bytes);

/* convert modulus and msg to internal mpi format */

		mpi2reg(n,n_mpi);
		mpi2reg(m,msg);

#ifdef DEBUG
		fprintf(stderr,"M = 0x%s\n",to_hex(m));
		fprintf(stderr,"n = 0x%s\n",to_hex(n));
#endif


/*
 *   Convert back to original message org = m mod n
 *
 */

		mp_mod(temp,m,n);      /* temp = m % n */

#ifdef DEBUG
		fprintf(stderr,"m = 0x%s\n",to_hex(temp));
#endif

/* convert to external format and write */

		msg_bytes = reg2mpi(msg,temp);
		fwrite(msg, msg_bytes + 2, 1, stdout);

#else
		/* MPI header */

		c = (length & 0xFF00) >> 8;
		putc (c, afp);
		putc (length & 0xFF, afp);

		/* Copy the MPI over */

		i = (length + 7) / 8;
		while (i-- > 0) {
			c = getchar();
			putc (c, afp);
		}
#endif
	}

	/* Now the IDEA bits */

#ifdef STEALTH2
	len = 0;
	read_all = FALSE;
	contingency = (char*)malloc( CONTINGENCY );
	if ( contingency == 0 )
	{
		goto outofmem_use_file;
	}
	buffer[ 0 ] = (char*)malloc( BUFFER_SIZE );
	if ( buffer[ 0 ] == 0 )
	{
		goto outofmem_use_file;
	}
	buffers_used++;
	while ( !feof(stdin) )
	{
		bytes_read = fread(buffer[buffers_used-1],1,BUFFER_SIZE,stdin);
		len += bytes_read;
		if (bytes_read == BUFFER_SIZE)
		{
			if (buffers_used == 16)
			{
				goto outofmem_use_file;
			}
			buffer[buffers_used] = (char*)malloc( BUFFER_SIZE );
			if ( buffer[buffers_used] == 0 )
			{
				goto outofmem_use_file;
			}
			buffers_used++;
		}
	}
	read_all = TRUE;

outofmem_use_file:

	if ( !read_all )
	{
/*
 *  Maybe less secure to use a temporary file, so have to enable use of 
 *  temp file with -t, this should only be necessary for large files
 *  unless you have little memory
 *
 */

		if ( !allow_temp )
		{
			fprintf( stderr, 
"out of memory, give -t option if you are willing to use a temporary file\n" );
			exit( 3 );
		}

/*
 *  Free up contingency (if we have it) in case further C library calls
 *  rely on having some memory left.
 *
 */
		if ( contingency != 0 )
		{
			free( contingency );
		}
/*
 *  create a temp file
 *
 */

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

/*
 * Copy rest of stdin to temp file as we're out of memory
 *
 */
		while ((c = getchar()) != EOF) {
			putc(c, afp);
			len++;
		}
		fflush( afp );
	}

/*
 *  PGP versions >= 2.5 seem to always use 32 bit size fields for the 
 *  length - either should work, its just nice to keep the input
 *  and output byte identical
 *
 */

	if ( pgp_version == 2 )
	{
		write_length( 0xA4, len, stdout );
	}
	else	/* default to 32 bit field for >= 2.5 & future versions */
	{
		/* manually do a 32 bit length field */

		putchar( 0xA6 );
		putchar( (len & 0xFF000000) >> 24 );
		putchar( (len & 0xFF0000) >> 16 );
		putchar( (len & 0xFF00) >> 8 );
		putchar( len & 0xFF );
	}
/*
 *  Write all the buffers (if any used)
 *
 */
	if (buffers_used)
	{
		for (i = 0; i <buffers_used-1; i++)
		{
			fwrite(buffer[i],1,BUFFER_SIZE,stdout);
		}

		if ( file_open )
		{
			fwrite(buffer[i],1,BUFFER_SIZE,stdout);
		}
		else
		{
			fwrite(buffer[i],1,len - (buffers_used-1)*BUFFER_SIZE,
				stdout);
		}
	}
	
/*
 *  If we ran out of memory, dump the temp file too
 *
 */

	if ( file_open )
	{
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

#else
	len = 0xFFFFFFFF;

	fpos = ftell (afp) + 1;
	write_length (0xA4, len, afp);

	len = 0;

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
#endif
}

static	char	looking_for [256];

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
#ifdef STEALTH2
				case '2':
				pgp_version = 2;
				break;

				case 't':
				allow_temp = TRUE;
				break;
#endif
				case 'a':
				adding = TRUE;
				break;

			}
		}

		arg++;
	}

#ifdef STEALTH2
	set_precision( MAX_UNIT_PRECISION );
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
}
