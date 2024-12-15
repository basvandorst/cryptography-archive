/*	crypto.c  - Cryptographic routines for PGP.
	PGP: Pretty Good(tm) Privacy - public key cryptography for the masses.

	(c) Copyright 1990-1992 by Philip Zimmermann.  All rights reserved.
	The author assumes no liability for damages resulting from the use
	of this software, even if the damage results from defects in this
	software.  No warranty is expressed or implied.

	All the source code Philip Zimmermann wrote for PGP is available for
	free under the "Copyleft" General Public License from the Free
	Software Foundation.  A copy of that license agreement is included in
	the source release package of PGP.  Code developed by others for PGP
	is also freely available.  Other code that has been incorporated into
	PGP from other sources was either originally published in the public
	domain or was used with permission from the various authors.  See the
	PGP User's Guide for more complete information about licensing,
	patent restrictions on certain algorithms, trademarks, copyrights,
	and export controls.  

 	Modified: 12-Nov-92 HAJK
 	Add FDL stuff for VAX/VMS local mode. 
	Reopen temporary files rather than create new version.

*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef __mips
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#ifndef	__NeXT__
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "random.h"
#include "crypto.h"
#include "pgp.h"
#include "mpilib.h"

#ifndef S_IRUSR
#define S_IRUSR S_IREAD
#define S_IWUSR S_IWRITE
#endif

/* The kbhit() function: Determines if a key has been hit.  May not be
   available in some implementations */

int kbhit( void );
int zipup(FILE *, FILE *);
#ifdef  M_XENIX
long time();
#endif

word32 get_timestamp(word32 *timestamp)
/*      Return current timestamp as a byte array in internal byteorder,
        and as a 32-bit word */
{       word32 t;
        t = time(NULL);    /* returns seconds since GMT 00:00 1 Jan 1970 */

#ifdef _MSC_VER
#if (_MSC_VER == 700)
        /*  Under MSDOS and MSC 7.0, time() returns elapsed time since
         *  GMT 00:00 31 Dec 1899, instead of Unix's base date of 1 Jan 1970.
         *  So we must subtract 70 years worth of seconds to fix this.
         *  6/19/92  rgb
        */
#define LEAP_DAYS       (((unsigned long)70L/4)+1)
#define CALENDAR_KLUDGE ((unsigned long)86400L * (((unsigned long)365L * 70L) +
LEAP_DAYS))
        t -= CALENDAR_KLUDGE;
#endif
#endif

        t += timeshift; /* timeshift derived from TZFIX in config.pgp */

        if (timestamp != NULL)
        {       /* first, fill array in external byte order: */
		*timestamp = t;
        }

        return(t);      /* return 32-bit timestamp integer */
}       /* get_timestamp */

/*--------------------------------------------------------------------------*/

#define IDEA_RANDOM
#ifdef IDEA_RANDOM
int strong_pseudorandom(byte *buf, int bufsize)
/*	
	Reads IDEA random key and random number seed from file, cranks the
	the seed through the idearand strong pseudorandom number generator,
	and writes them back out.  This is used for generation of
	cryptographically strong pseudorandom numbers.  This is mainly to
	save the user the trouble of having to type in lengthy keyboard
	sequences for generation of truly random numbers every time we want
	to make a random session key.  This pseudorandom generator will only
	work if the file containing the random seed exists and is not empty.
	If it doesn't exist, it will be automatically created.  If it exists
	and is empty or nearly empty, it will not be used.
*/
{	char seedfile[MAX_PATH];	/* Random seed filename */
	FILE *f;
	byte key[IDEAKEYSIZE];
	byte seed[IDEABLOCKSIZE];
	int i;
	word32 tstamp;

	strcpy(seedfile, getenv("HOME"));
	strcat(seedfile, "/");
	strcat(seedfile, ".rand");

	if (access(seedfile, F_OK))	/* No seed file. Start one... */
	{
	    return -1;
	}
	else	/* seedfile DOES exist.  Open it and read it. */
	{	f = fopen(seedfile,"r");	/* open for reading binary */
		if (f==NULL)	/* did open fail? */
			return(-1);	/* error: no random number seed file available */
		/* read IDEA random generator key */
		if (fread(key,1,sizeof(key),f) < sizeof(key))	/* empty file? */
		{	/* Empty or nearly empty file means don't use it. */
			fclose(f);
			return(-1);	/* error: no random number seed file available */
		}
		else
			fread(seed,1,sizeof(seed),f); /* read pseudorandom seed */
		fclose(f);
	}	/* seedfile exists */


	/* Initialize, key, and seed the IDEA pseudorandom generator: */
	get_timestamp(&tstamp);	/* timestamp points to tstamp */
	init_idearand(key, seed, tstamp);

	/* Note that the seed will be cycled thru IDEA before use */

	/* Now fill the user's buffer with gobbledygook... */
	while (bufsize--)
		*buf++ = idearand() ^ randombyte();

	/* now cover up evidence of what user got */
	for (i=1; i<sizeof(key); i++)
		key[i] ^= idearand() ^ randombyte();
	for (i=0; i<sizeof(seed); i++)
		seed[i] = idearand() ^ randombyte();

	close_idearand();	/* close IDEA random number generator */

	chmod(seedfile, S_IRUSR|S_IWUSR);
	f = fopen(seedfile,"w");	/* open for writing binary */
	if (f==NULL)	/* did open fail? */
		return(-1);	/* error: no random number seed file available */
#ifdef VMS
	fseek (f, 0, SEEK_SET);
#endif
	/* Now at start of file again */
	fwrite(key,1,sizeof(key),f);
	fwrite(seed,1,sizeof(seed),f);
	fclose(f);
	burn(key);		/* burn sensitive data on stack */
	burn(seed);		/* burn sensitive data on stack */
	return(0);	/* normal return */
}	/* strong_pseudorandom */

extern int randcount;

int refresh_pseudorandom()
{	char seedfile[MAX_PATH];	/* Random seed filename */
	FILE *f;
	byte key[IDEAKEYSIZE];
	byte seed[IDEABLOCKSIZE];
	int i;
	word32 tstamp;

	capturecounter();
	if (randcount < 2*(sizeof(key) + sizeof(seed)))
		return;

	strcpy(seedfile, getenv("HOME"));
	strcat(seedfile, "/");
	strcat(seedfile, ".rand");

	f = fopen(seedfile,"w");	/* open for writing binary */
	chmod(seedfile, S_IRUSR|S_IWUSR);
	if (f==NULL)	/* failed to create seedfile */
		return(-1);	/* error: no random number seed file available */
	fclose(f);	/* close new empty seed file */
	/* kickstart the generator with true random numbers */
	for (i=1; i<sizeof(key); i++)
		key[i] ^= randombyte();
	for (i=0; i<sizeof(seed); i++)
		seed[i] ^= randombyte();

	/* Initialize, key, and seed the IDEA pseudorandom generator: */
	get_timestamp(&tstamp);	/* timestamp points to tstamp */
	init_idearand(key, seed, tstamp);

	/* Note that the seed will be cycled thru IDEA before use */


	/* now cover up evidence of what user got */
	for (i=1; i<sizeof(key); i++)
		key[i] ^= idearand() ^ randombyte();
	for (i=0; i<sizeof(seed); i++)
		seed[i] = idearand() ^ randombyte();

	close_idearand();	/* close IDEA random number generator */

	f = fopen(seedfile,"w");	/* open for writing binary */
	if (f==NULL)	/* did open fail? */
		return(-1);	/* error: no random number seed file available */
#ifdef VMS
	fseek (f, 0, SEEK_SET);
#endif
	/* Now at start of file again */
	fwrite(key,1,sizeof(key),f);
	fwrite(seed,1,sizeof(seed),f);
	fclose(f);
	burn(key);		/* burn sensitive data on stack */
	burn(seed);		/* burn sensitive data on stack */
	return(0);	/* normal return */
}	/* strong_pseudorandom */

int make_random_ideakey(byte key[16])
/*	Make a random IDEA key.  Returns its length (a constant). */
{	int count;

	if (strong_pseudorandom(key, IDEAKEYSIZE) == 0)
		return(IDEAKEYSIZE);

	fprintf(stderr,PSTR("Preparing random session key..."));

	randaccum(IDEAKEYSIZE*8); /* get some random key bits */

	count=0;
	while (++count <= IDEAKEYSIZE)
		key[count] = randombyte();

	return(IDEAKEYSIZE);

}	/* make_random_ideakey */
#endif


int init_idea_stream(byte* buf, byte *ideakey, boolean decryp, IDEAkey Z, word16* iv)
/*	Use IDEA in cipher feedback (CFB) mode to encrypt or decrypt a file. 
	The encrypted material starts out with a 64-bit random prefix, which
	serves as an encrypted random CFB initialization vector, and
	following that is 16 bits of "key check" material, which is a
	duplicate of the last 2 bytes of the random prefix.  Encrypted key
	check bytes detect if correct IDEA key was used to decrypt ciphertext.
*/
{	int status = 0;
	FILE *f;

	/* init CFB key */
	fill0(iv,sizeof(*iv) * 4); /* define initialization vector IV as 0 */
	initcfb_idea(Z,iv,ideakey,decryp);

	if (!decryp)	/* encrypt-- insert key check bytes */
	{	/* There is a random prefix followed by 2 key check bytes */
		int	i;

		for (i=0; i<RAND_PREFIX_LENGTH; ++i)
				buf[i] = randombyte();
		/* key check bytes are simply duplicates of final 2 random bytes */
		buf[i] = buf[i-2];	/* key check bytes for redundancy */
		buf[i+1] = buf[i-1];

		ideacfb(buf,RAND_PREFIX_LENGTH+2, Z, iv, decryp);
		return 0;
	}
	else	/* decrypt-- check for key check bytes */
	{	/* See if the redundancy is present after the random prefix */
		ideacfb(buf,RAND_PREFIX_LENGTH+2, Z, iv, decryp);
		if ((buf[RAND_PREFIX_LENGTH] != buf[RAND_PREFIX_LENGTH-2])
			|| (buf[RAND_PREFIX_LENGTH+1] != buf[RAND_PREFIX_LENGTH-1]))
			return(-2);		/* bad key error */
		return 0;
	}
}

#if 0
int test()
{
	byte ideakey[16];
	byte passphrase[256];
	extern char password[];
	FILE*	f;
	FILE*	f1;
	int i;

	if (GetHashedPassPhrase((char *)passphrase,(char *)ideakey,NOECHO2) <= 0)
	{
		return(-1);
	}

	f = fopen("t", "r");
	f1 = fopen("t1", "w");
	/* Now compress the plaintext and encrypt it with IDEA... */
	init_idea_stream( ideakey, ENCRYPT_IT );
	for (i = 0 ; i < RAND_PREFIX_LENGTH + 2 ; i++)
		putc(buf[i], f1);

	while(!feof(f))
	{
		int c;
		char c1;
		c = getc(f);
		c1 = c;
		if (c < 0) break;
		idea_stream(&c1);
		putc(c, f1);
	}

	burn(passphrase);

	close_idea_stream();
	return 0;
}	/* idea_encryptfile */

int test1()
{
	byte ideakey[16];
	byte passphrase[256];
	extern char password[];
	FILE*	f;
	FILE*	f1;
	int i;

	if (GetHashedPassPhrase((char *)passphrase,(char *)ideakey,NOECHO2) <= 0)
	{
		return(-1);
	}

	/* Now compress the plaintext and encrypt it with IDEA... */
	f = fopen("t1", "r");
	f1 = fopen("t2", "w");
	for (i = 0 ; i < RAND_PREFIX_LENGTH + 2 ; i++)
		buf[i] = getc(f);

	if (init_idea_stream( ideakey, DECRYPT_IT) < 0)
		abort();

	while(!feof(f))
	{
		int c;
		char c1;
		c = getc(f);
		c1 = c;
		if (c < 0) break;
		idea_stream(&c);
		putc(c, f1);
	}

	burn(passphrase);

	close_idea_stream();
	return 0;
}	/* idea_encryptfile */
#endif


idea_stream(byte* ch, IDEAkey Z, word16 iv[4], boolean decryp)
{
	ideacfb(ch, 1, Z, iv, decryp);
	return 0;
}

close_idea_stream(IDEAkey Z, word16 iv[4])
{
	close_idea(Z);	/* Clean up data structures */
	burn(iv);		/* burn sensitive data */
	return(0);	/* should always take normal return */
}	/* idea_file */

long timeshift = 0;

byte moduli_ext[] =
{0x2, 0x1, 0x1, 0xc1, 0xff, 0x9d, 0x1a, 0x49, 0x1c, 0xb4, 0xf8, 
 0x27, 0x29, 0xb8, 0xb4, 0x9d, 0xa9, 0xa, 0xd6, 0x59, 0xd5, 0xb8, 
 0x1a, 0xcc, 0x1c, 0x40, 0x9f, 0xad, 0x1a, 0xf5, 0xbe, 0x8c, 0xe6, 
 0x9d, 0x4e, 0x44, 0xe4, 0xb7, 0x6e, 0x95, 0x5d, 0x76, 0x61, 0x79, 
 0x6b, 0xae, 0x9c, 0xd5, 0x75, 0x2a, 0x80, 0xba, 0x3b, 0xe2, 0xb4, 
 0x5e, 0xca, 0x91, 0x54, 0xe9, 0x70, 0x5e, 0x8e, 0x8a, 0xf7, 0x5f, 
 0x5f};

unit moduli[MAX_UNIT_PRECISION];	/* q */
unit agreedkey[MAX_UNIT_PRECISION];	/* session key */
unit publickey[MAX_UNIT_PRECISION];	/* my public key */
unit privatekey[MAX_UNIT_PRECISION];	/* my private key */
byte buf[MAX_BYTE_PRECISION];		/* Temporary static area */

unit two[MAX_UNIT_PRECISION];		/* The number 2 */

static unit randomunit(void)
	/* Fills 1 unit with random bytes, and returns unit. */
{	unit u = 0;
	byte i;
	static int already_complained = 0;
	i = BYTES_PER_UNIT;
	if (strong_pseudorandom((byte*)&u, i) < 0)
	{
	    if (!already_complained)
	    {
	        panic("No random seed file (normal for first run).");
		already_complained = 1;
	    }
	    do
		u = (u << 8) + randombyte();
	    while (--i != 0);
        }
	return(u);
}	/* randomunit */

static void randombits(unitptr p, short nbits)
/*	Make a random unit array p with nbits of precision.  Used mainly to 
	generate large random numbers to search for primes.
*/
{	/* Fill a unit array with exactly nbits of random bits... */
	short nunits;	/* units of precision */
	mp_init(p,0);
	nunits = bits2units(nbits);	/* round up to units */
	make_lsbptr(p,global_precision);
	*p = randomunit();
	while (--nunits)
	{	*pre_higherunit(p) = randomunit();
		nbits -= UNITSIZE;
	}
	*p &= (power_of_2(nbits)-1); /* clear the top unused bits remaining */
}	/* randombits */

void
init_crypto()
{
    set_precision(MAX_UNIT_PRECISION);
    mp_init(two, 2);
    mpi2reg(moduli, moduli_ext);
    /*mp_display("moduli", moduli);*/
}

static int have_key = 0;


int
createDHkey()
{
    if (!have_key)
    {
	randombits(privatekey, 510);
	mp_modexp(publickey, two, privatekey, moduli);
    }
}

int
create_getDHpublic(byte* ptr)
{
    createDHkey();
    have_key = 0;
    
    /*mp_display("privatekey", privatekey);*/
    /*mp_display("publickey", publickey);*/
    /*mp_display("two", two);*/
    
    return reg2mpi(ptr, publickey) + 2;
}

int
getDHpublic(byte* ptr)
{
    have_key = 0;
    return reg2mpi(ptr, publickey) + 2;
}

void
DHprepare()
{
    if (!have_key)
    {
	randombits(privatekey, 510);
	mp_modexp(publickey, two, privatekey, moduli);
	have_key = 1;
    }
}

int
computeDHagreed_key(unit* key, byte** ptr)
{
    int len;
    
    /*mp_display("theirkey", key);*/
    mp_modexp(agreedkey, key, privatekey, moduli);
    /*mp_display("agreedkey", agreedkey);*/
    len = reg2mpi(buf, agreedkey);
    *ptr = buf + 2;
    return len;
}

void
DHburn()
{
    mp_burn(privatekey);
    burn(buf);
    mp_burn(agreedkey);
}
