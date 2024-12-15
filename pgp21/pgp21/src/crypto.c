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

#include "mpilib.h"
#include "mpiio.h"
#include "random.h"
#include "idea.h"
#include "crypto.h"
#include "keymgmt.h"
#include "mdfile.h"
#include "fileio.h"
#include "language.h"
#include "pgp.h"

#define ENCRYPT_IT FALSE	/* to pass to idea_file */
#define DECRYPT_IT TRUE		/* to pass to idea_file */

/* The kbhit() function: Determines if a key has been hit.  May not be
   available in some implementations */

int kbhit( void );
int zipup(FILE *, FILE *);
#ifdef  M_XENIX
long time();
#endif

/*--------------------------------------------------------------------------*/


void CToPascal(char *s)
{	/* "xyz\0" --> "\3xyz" ... converts C string to Pascal string */
	int i,j;
	j = string_length(s);
	for (i=j; i!=0; i--)
		s[i] = s[i-1];	/* move everything 1 byte to the right */
	s[0] = j;		/* Pascal length byte at beginning */
}	/* CToPascal */


void PascalToC( char *s )
{	/* "\3xyz" --> "xyz\0" ... converts Pascal string to C string */
	int i,j;
	for (i=0,j=((byte *) s)[0]; i<j; i++)
		s[i] = s[i+1];	/* move everything 1 byte to the left */
	s[i] = '\0';		/* append C string terminator */
}	/* PascalToC */


/* 
	Note:  On MSDOS, the time() function calculates GMT as the local
	system time plus a built-in timezone correction, which defaults to
	adding 7 hours (PDT) in the summer, or 8 hours (PST) in the winter,
	assuming the center of the universe is on the US west coast. Really--
	I'm not making this up!  The only way to change this is by setting 
	the MSDOS environmental variable TZ to reflect your local time zone,
	for example "set TZ=MST7MDT".  This means add 7 hours during standard
	time season, or 6 hours during daylight time season, and use MST and 
	MDT for the two names of the time zone.  If you live in a place like 
	Arizona with no daylight savings time, use "set TZ=MST7".  See the
	Microsoft C function tzset().  Just in case your local software
	environment is too weird to predict how to set environmental
	variables for this, PGP also uses its own TZFIX variable in
	config.pgp to optionally correct this problem further.  For example,
	set TZFIX=-1 in config.pgp if you live in Colorado and the TZ
	variable is undefined.
*/

word32 get_timestamp(byte *timestamp)
/*	Return current timestamp as a byte array in internal byteorder,
	and as a 32-bit word */
{	word32 t;
	t = time(NULL);    /* returns seconds since GMT 00:00 1 Jan 1970 */

#ifdef _MSC_VER
#if (_MSC_VER == 700)
	/*  Under MSDOS and MSC 7.0, time() returns elapsed time since
	 *  GMT 00:00 31 Dec 1899, instead of Unix's base date of 1 Jan 1970.
	 *  So we must subtract 70 years worth of seconds to fix this.
	 *  6/19/92  rgb 
	*/
#define	LEAP_DAYS	(((unsigned long)70L/4)+1)
#define CALENDAR_KLUDGE ((unsigned long)86400L * (((unsigned long)365L * 70L) + LEAP_DAYS))
   	t -= CALENDAR_KLUDGE;
#endif
#endif

	t += timeshift; /* timeshift derived from TZFIX in config.pgp */

	if (timestamp != NULL)
	{	/* first, fill array in external byte order: */
		put_word32(t, timestamp);
		convert_byteorder(timestamp,4);	/* convert to internal byteorder */
	}

	return(t);	/* return 32-bit timestamp integer */
}	/* get_timestamp */


int date_ymd(word32 *tstamp, int *year, int *month, int *day)
/*	Given timestamp as seconds elapsed since 1970 Jan 1 00:00:00,
	returns year (1970-2106), month (1-12), day (1-31).
	Not valid for dates after 2100 Feb 28 (no leap day that year).
	Also returns day of week (0-6) as functional return.
*/
{	word32 days,y;
	int m,d,i;
	static short mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	days = (*tstamp)/(unsigned long)86400L;	/* day 0 is 1970/1/1 */
	days -= 730L;	/* align days relative to 1st leap year, 1972 */
	y = ((days*4)/(unsigned long)1461L);	/* 1972 is year 0 */
	/* reduce to days elapsed since 1/1 last leap year: */
	d = (int) (days - ((y/4)*1461L));
	*year = (int)(y+1972);
	for (i=0; i<48; i++)	/* count months 0-47 */
	{	m = i % 12;
		d -= mdays[m] + (i==1);	/* i==1 is the only leap month */
		if (d < 0)
		{	d += mdays[m] + (i==1);
			break;
		}
	}
	*month = m+1;
	*day = d+1;
	i = (int)((days-2) % (unsigned long)7L);	/* compute day of week 0-6 */
	return(i);	/* returns weekday 0-6; 0=Sunday, 6=Saturday */
}	/* date_ymd */


char *cdate(word32 *tstamp)
/*	Return date string, given pointer to 32-bit timestamp */
{	int month,day,year;
	static char datebuf[20];
	if (*tstamp == 0)
		return("          ");
	(void) date_ymd(tstamp,&year,&month,&day);
	sprintf(datebuf,"%4d/%02d/%02d", year, month, day);
	return (datebuf);
}	/* cdate */


char *ctdate(word32 *tstamp)
/*	Return date and time string, given pointer to 32-bit timestamp */
{	int hours,minutes;
	static char tdatebuf[40];
	long seconds;
	seconds = (*tstamp) % (unsigned long)86400L;	/* seconds past midnight today */
	minutes = (int)((seconds+30L) / 60L);	/* round off to minutes past midnight */
	hours = minutes / 60;			/* hours past midnight */
	minutes = minutes % 60;			/* minutes past the hour */
	sprintf(tdatebuf,"%s %02d:%02d GMT", cdate(tstamp), hours, minutes);
	return (tdatebuf);
}	/* ctdate */



/* Warn user he if key in keyfile at position fp of length pktlen, belonging
 * to userid, is untrusted.  Return -1 if the user doesn't want to proceed.
 */
int warn_signatures(char *keyfile, long fp, int pktlen, char *userid,
		boolean warn_only)
{	FILE		*f;
	long		fpusr;
	int			usrpktlen;
	byte		keyctrl;
	int			trust_status = -1;

	keyctrl = KC_LEGIT_UNKNOWN;	/* Assume the worst */
	PascalToC(userid);
	if (getpubuserid (keyfile, fp, (byte *) userid, &fpusr, &usrpktlen, FALSE) >= 0)
	{	f = fopen(keyfile, FOPRBIN);
		fseek (f, fpusr+usrpktlen, SEEK_SET);
		/* Read trust byte */
		trust_status = read_trust(f, &keyctrl);
		fseek(f, fp, SEEK_SET);
		if (is_compromised(f))
		{
			CToPascal(userid);
			fprintf(pgpout, "\n");
			show_key(f, fp, 0);
			fclose (f);
			fprintf(pgpout, PSTR("\007\nWARNING:  This key has been revoked by its owner,\n\
possibly because the secret key was compromised.\n"));
			if (warn_only)
			{	/* this is only for checking signatures */
				fprintf(pgpout, PSTR("This could mean that this signature is a forgery.\n"));
				return(1);
			}
			else
			{	/* don't use it for encryption */
				fprintf(pgpout, PSTR("You cannot use this revoked key.\n"));
				return(-1);
			}
		}
		fclose (f);
	}
	CToPascal(userid);
	if ((keyctrl & KC_LEGIT_MASK) != KC_LEGIT_COMPLETE)
	{	byte userid0[256];
		PascalToC(userid);
		strcpy ((char *) userid0, userid);
		CToPascal(userid);
		if ((keyctrl & KC_LEGIT_MASK) == KC_LEGIT_UNKNOWN)
			fprintf(pgpout,PSTR("\007\nWARNING:  Because this public key is not certified with a trusted\n\
signature, it is not known with high confidence that this public key\n\
actually belongs to: \"%s\".\n"), LOCAL_CHARSET((char *)userid0));
		if ((keyctrl & KC_LEGIT_MASK) == KC_LEGIT_UNTRUSTED)
			fprintf(pgpout, PSTR("\007\nWARNING:  This public key is not trusted to actually belong to:\n\
\"%s\".\n"), LOCAL_CHARSET((char *)userid0));
		if ((keyctrl & KC_LEGIT_MASK) == KC_LEGIT_MARGINAL)
			fprintf(pgpout,PSTR("\007\nWARNING:  Because this public key is not certified with enough trusted\n\
signatures, it is not known with high confidence that this public key\n\
actually belongs to: \"%s\".\n"), LOCAL_CHARSET((char *)userid0));
		if (!filter_mode && !warn_only && !(keyctrl & KC_WARNONLY))
		{	fprintf(pgpout,PSTR("\nAre you sure you want to use this public key (y/N)? "));
			if (!getyesno('n'))
				return(-1);
			if (trust_status == 0 && (f = fopen(keyfile, FOPRWBIN)) != NULL)
			{	fseek (f, fpusr+usrpktlen, SEEK_SET);
				keyctrl |= KC_WARNONLY;
				write_trust(f, keyctrl);
				fclose(f);
			}
		}
	}
	if( pktlen );	/* Get rid of unused parameter warning */

	return(0);
}	/* warn_signatures */


boolean legal_ctb(byte ctb)
{	/* Used to determine if nesting should be allowed. */
	boolean legal;
	byte ctbtype;
	if (!is_ctb(ctb))		/* not even a bonafide CTB */
		return(FALSE);
	/* Sure hope CTB internal bit definitions don't change... */
	ctbtype = (ctb & CTB_TYPE_MASK) >> 2;
	/* Only allow these CTB types to be nested... */
	legal = (
			(ctbtype==CTB_PKE_TYPE)
		||	(ctbtype==CTB_SKE_TYPE)
		||	(ctbtype==CTB_CERT_SECKEY_TYPE)
		||	(ctbtype==CTB_CERT_PUBKEY_TYPE)
		||	(ctbtype==CTB_LITERAL_TYPE)
		||	(ctbtype==CTB_LITERAL2_TYPE)
		||	(ctbtype==CTB_COMPRESSED_TYPE)
		||  (ctbtype==CTB_CKE_TYPE)
		 );
	return(legal);
}	/* legal_ctb */


/* Return nonzero if val doesn't match checkval, after printing a
 * warning.
 */
int
version_error(int val, int checkval)
{	if (val != checkval)
	{	fprintf (pgpout, PSTR(
"\n\007Unsupported packet format - you need a newer version of PGP for this file.\n"));
		return(1);
	}
	return(0);
}

/*-------------------------------------------------------------------------*/


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
	word32 tstamp; byte *timestamp = (byte *) &tstamp;

	buildfilename(seedfile,RANDSEED_FILENAME);

	if (!file_exists(seedfile))	/* No seed file. Start one... */
	{	f = fopen(seedfile,FOPWBIN);	/* open for writing binary */
		if (f==NULL)	/* failed to create seedfile */
			return(-1);	/* error: no random number seed file available */
		fclose(f);	/* close new empty seed file */
		/* kickstart the generator with true random numbers */
		fprintf(pgpout,PSTR("Initializing random seed file..."));
		randaccum(8*(sizeof(key)+sizeof(seed)));
		for (i=1; i<sizeof(key); i++)
			key[i] ^= randombyte();
		for (i=0; i<sizeof(seed); i++)
			seed[i] ^= randombyte();
	}	/* seedfile does not exist */

	else	/* seedfile DOES exist.  Open it and read it. */
	{	f = fopen(seedfile,FOPRBIN);	/* open for reading binary */
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
	get_timestamp(timestamp);	/* timestamp points to tstamp */
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

	f = fopen(seedfile,FOPWBIN);	/* open for writing binary */
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

	fprintf(pgpout,PSTR("Preparing random session key..."));

	randaccum(IDEAKEYSIZE*8); /* get some random key bits */

	count=0;
	while (++count <= IDEAKEYSIZE)
		key[count] = randombyte();

	return(IDEAKEYSIZE);

}	/* make_random_ideakey */


word32 getpastlength(byte ctb, FILE *f)
/*	Returns the length of a packet according to the CTB and
	the length field. */
{	word32 length;
	unsigned int llength;	/* length of length */
	byte buf[8];

	fill0(buf,sizeof(buf));
	length = 0L;
	/* Use ctb length-of-length field... */
	llength = ctb_llength(ctb);	/* either 1, 2, 4, or 8 */
	if (llength==8)		/* 8 means no length field, assume huge length */
		return(-1L);	/* return huge length */

	/* now read in the actual length field... */
	if (fread((byteptr) buf,1,llength,f) < llength)
		return (-2L); /* error -- read failure or premature eof */
	/* convert length from external byteorder... */
	if (llength==1)
		length = (word32) buf[0];
	if (llength==2)
		length = (word32) fetch_word16(buf);
	if (llength==4)
		length = fetch_word32(buf);
	return(length);
}	/* getpastlength */


/* Write a CTB with the appropriate length field.  If big is true,
 * always use a four-byte length field.
 */
void write_ctb_len (FILE *f, byte ctb_type, word32 length, boolean big)
{
	int		llength, llenb;
	byte	ctb;
	byte	buf[4];

	if (big || (length > 0xFFFFL))
	{	llength = 4;
		llenb = 2;
	}
	else if ((word16)length > 0xFF)
	{	llength = 2;
		llenb = 1;
	}
	else
	{	llength = 1;
		llenb = 0;
	}
	ctb = CTB_BYTE(ctb_type, llenb);
	fwrite( &ctb, 1, 1, f );
	/* convert length to external byteorder... */
	if (llength==1)
		buf[0] = length;
	if (llength==2)
		put_word16((word16) length, buf);
	if (llength==4)
		put_word32(length, buf);
	fwrite( buf, 1, llength, f );
} /* write_ctb_len */


int idea_file(byte *ideakey, boolean decryp, FILE *f, FILE *g, word32 lenfile)
/*	Use IDEA in cipher feedback (CFB) mode to encrypt or decrypt a file. 
	The encrypted material starts out with a 64-bit random prefix, which
	serves as an encrypted random CFB initialization vector, and
	following that is 16 bits of "key check" material, which is a
	duplicate of the last 2 bytes of the random prefix.  Encrypted key
	check bytes detect if correct IDEA key was used to decrypt ciphertext.
*/
{	int count, status = 0;
	word16 iv[4];
	extern byte textbuf[DISKBUFSIZE];
#define RAND_PREFIX_LENGTH 8

	/* init CFB key */
	fill0(iv,sizeof(iv));	/* define initialization vector IV as 0 */
	initcfb_idea(iv,ideakey,decryp);

	if (!decryp)	/* encrypt-- insert key check bytes */
	{	/* There is a random prefix followed by 2 key check bytes */
		int	i;

		for (i=0; i<RAND_PREFIX_LENGTH; ++i)
				textbuf[i] = randombyte();
		/* key check bytes are simply duplicates of final 2 random bytes */
		textbuf[i] = textbuf[i-2];	/* key check bytes for redundancy */
		textbuf[i+1] = textbuf[i-1];

		ideacfb(textbuf,RAND_PREFIX_LENGTH+2);
		fwrite(textbuf,1,RAND_PREFIX_LENGTH+2,g);
	}
	else	/* decrypt-- check for key check bytes */
	{	/* See if the redundancy is present after the random prefix */
		count = fread(textbuf,1,RAND_PREFIX_LENGTH+2,f);
		lenfile -= count;
		if (count==(RAND_PREFIX_LENGTH+2))
		{	ideacfb(textbuf,RAND_PREFIX_LENGTH+2);
			if ((textbuf[RAND_PREFIX_LENGTH] != textbuf[RAND_PREFIX_LENGTH-2])
				|| (textbuf[RAND_PREFIX_LENGTH+1] != textbuf[RAND_PREFIX_LENGTH-1]))
			{	return(-2);		/* bad key error */
			}
		}
		else	/* file too short for key check bytes */
			return(-3);		/* error of the weird kind */
	}


	do	/* read and write the whole file in CFB mode... */
	{	count = (lenfile < DISKBUFSIZE) ? (int)lenfile : DISKBUFSIZE;
		count = fread(textbuf,1,count,f);
		lenfile -= count;
		if (count>0)
		{	ideacfb(textbuf,count);
			if (fwrite(textbuf,1,count,g) != count)
			{	status = -3;
				break;
			}
		}
		/* if text block was short, exit loop */
	} while (count==DISKBUFSIZE);

	close_idea();	/* Clean up data structures */
	burn(iv);		/* burn sensitive data on stack */
	burn(textbuf);	/* burn sensitive data on stack */
	return(status);	/* should always take normal return */
}	/* idea_file */


/* Checksum maintained as a running sum by read_mpi and write_mpi.
 * The checksum is maintained based on the plaintext values being
 * read and written.  To use it, store a 0 to it before doing a set
 * of read_mpi's or write_mpi's.  Then read it aftwerwards.
 */
word16	mpi_checksum;

int read_mpi(unitptr r, FILE *f, boolean adjust_precision, boolean scrambled)
/*	Read a mutiprecision integer from a file.
	adjust_precision is TRUE iff we should call set_precision to the 
	size of the number read in.
	scrambled is TRUE iff field is encrypted (protects secret key fields).
	Returns the bitcount of the number read in, or returns a negative
	number if an error is detected.
*/
{	byte buf[MAX_BYTE_PRECISION+2];
	unsigned int count;
	word16 bytecount,bitcount;

	mp_init(r,0);

	if ((count = fread(buf,1,2,f)) < 2)
		return (-1); /* error -- read failure or premature eof */

	bitcount = fetch_word16(buf);
	if (bits2units(bitcount) > global_precision)
		return(-1);	/* error -- possible corrupted bitcount */

	bytecount = bits2bytes(bitcount);

	count = fread(buf+2,1,bytecount,f);
	if (count < bytecount)
		return(-1);	/* error -- premature eof */

	if (scrambled)	/* decrypt the field */
		ideacfb(buf+2,bytecount);

	/* Update running checksum, in case anyone cares... */
	mpi_checksum += checksum (buf, bytecount+2);

	/*	We assume that the bitcount prefix we read is an exact
		bitcount, not rounded up to the next byte boundary.
		Otherwise we would have to call mpi2reg, then call
		countbits, then call set_precision, then recall mpi2reg
		again.
	*/
	if (adjust_precision && bytecount)
	{	/* set the precision to that specified by the number read. */
		if (bitcount > MAX_BIT_PRECISION-SLOP_BITS)
			return(-1);
		set_precision(bits2units(bitcount+SLOP_BITS));
		/* Now that precision is optimally set, call mpi2reg */
	}

	if (mpi2reg(r,buf) == -1)	/* convert to internal format */
		return(-1);
	burn(buf);	/* burn sensitive data on stack */
	return (bitcount);
}	/* read_mpi */



void write_mpi(unitptr n, FILE *f, boolean scrambled)
/*	Write a multiprecision integer to a file.
	scrambled is TRUE iff we should scramble field on the way out,
	which is used to protect secret key fields.
*/
{	byte buf[MAX_BYTE_PRECISION+2];
	short bytecount;
	bytecount = reg2mpi(buf,n);
	mpi_checksum += checksum (buf, bytecount+2);
	if (scrambled)  /* encrypt the field, skipping over the bitcount */
		ideacfb(buf+2,bytecount);
	fwrite(buf,1,bytecount+2,f); 
	burn(buf);	/* burn sensitive data on stack */
}	/* write_mpi */


/*======================================================================*/


int get_header_info_from_file(char *infile,  byte *header, int count)
/*	Reads the first count bytes from infile into header. */
{	FILE *f;
	fill0(header,count);
	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL)
		return(-1);
	/* read Cipher Type Byte, and maybe more */
	count = fread(header,1,count,f);
	fclose(f);
	return(count);	/* normal return */
}	/* get_header_info_from_file */


/* System clock must be broken if it isn't past this date: */
#define REASONABLE_DATE ((unsigned long) 0x27804180L)  /* 91 Jan 01 00:00:00 */


int make_signature_certificate(byte *certificate, MD5_CTX *MD, byte class,
	unitptr n, unitptr d, unitptr p, unitptr q, unitptr u)
/*	Constructs a signed message digest in a signature certificate.
	Returns total certificate length in bytes, or returns negative
	error status.
*/
{
	byte inbuf[MAX_BYTE_PRECISION], outbuf[MAX_BYTE_PRECISION];
	byte mdpacket[17];
	byte *mdbufptr;
	int i,j,certificate_length,blocksize,bytecount;
	word16 ske_length;
	word32 tstamp; byte *timestamp = (byte *) &tstamp;
	byte keyID[KEYFRAGSIZE];
	byte val;
	int mdlen = 5;	/* length of class plus timestamp, for adding to MD */

	/*	Note that RSA key must be at least big enough to encipher a
		complete message digest packet in a single RSA block. */

	blocksize = countbytes(n)-1;	/* size of a plaintext block */
	if (blocksize < 31)
	{	fprintf(pgpout,"\n\007Error: RSA key length must be at least 256 bits.\n");
		return(-1);
	}

	get_timestamp(timestamp);	/* Timestamp when signature was made */
	if (tstamp < REASONABLE_DATE) /* complain about bad time/date setting */
	{	fprintf(pgpout,PSTR("\n\007Error: System clock/calendar is set wrong.\n"));
		return(-1);
	}
	convert_byteorder(timestamp,4); /* convert to external form */

	/* Finish off message digest calculation with this information */
	MD_addbuffer (MD, &class, 1, FALSE);
	MD_addbuffer (MD, timestamp, 4, TRUE);

#ifdef XLOWFIRST	/* Assumes LSB-first order */
	mdbufptr = MD->digest;	/* point at actual message digest */
	for (i=0; i<sizeof(mdpacket)-1; i++)
	{	mdpacket[i] = *mdbufptr++;
	}
	mdpacket[i] = MD5_ALGORITHM_BYTE;
#else
	/* Assumes MSB-first order */
	mdpacket[0] = MD5_ALGORITHM_BYTE;
	mdbufptr = MD->digest;	/* point at actual message digest */
	for (i=1; i<sizeof(mdpacket); i++)
	{	mdpacket[i] = *mdbufptr++;
	}
#endif
	/* Pre-block mdpacket, and convert to INTERNAL byte order: */
	preblock((unitptr)inbuf, mdpacket, sizeof(mdpacket), n, NULL);

	if (!filter_mode)
	{	fprintf(pgpout,PSTR("Just a moment..."));	/* RSA will take a while. */
		fflush(pgpout);
	}

	/* do RSA signature calculation: */
	rsa_decrypt((unitptr)outbuf,(unitptr)inbuf,d,p,q,u);

	/* bytecount does not include the 2 prefix bytes */
	bytecount = reg2mpi(outbuf,(unitptr)outbuf); /* convert to external format */
	/*	outbuf now contains a message digest in external byteorder 
		form.  Now make a complete signature certificate from this.
	*/

	certificate_length = 0;

	/* SKE is Secret Key Encryption (signed).  Append CTB for signed msg. */
	certificate[certificate_length++] = CTB_SKE;

	/* SKE packet length does not include itself or CTB prefix: */
	ske_length = 1 + 1 	/* version and mdlen byte */
				+ mdlen		/* class, timestamp and validation period */ 
				+ KEYFRAGSIZE + 1 + 1	/* Key ID and 2 algorithm bytes */
				+ 2 + bytecount+2;	/* 2 MD bytes and RSA MPI w/bitcount */
	put_word16((word16) ske_length, certificate+certificate_length);
	certificate_length+=2;	/* advance past word */

	certificate[certificate_length++] = VERSION_BYTE;

	/* Begin fields that are included in MD calculation... */

	certificate[certificate_length++] =  mdlen;	/* mdlen is length of MD-extras */

	certificate[certificate_length++] =  class & 0xff;
	mdlen--;	/* assume class byte always present */

	/* timestamp already in external format */
	if (mdlen>0)
	{	for (j=0; j<SIZEOF_TIMESTAMP; j++)
		{	certificate[certificate_length++] =  timestamp[j];
			mdlen--;
		}
	}

	/* if any bytes remain in mdlen, assume it's the validation period */
	if (mdlen>0)
	{	val = 0;	/* Validation period */
		put_word16(val, certificate+certificate_length);
		certificate_length+=2;	/* advance past word */
		mdlen-=2;
	}
	/* hopefully, mdlen is now zero.  */

	/* ...end of fields that are included in MD calculation */

	/* Now append keyID... */
	extract_keyID(keyID, n);	/* gets keyID */
	for (i=0; i<KEYFRAGSIZE; i++)
		certificate[certificate_length++] = keyID[i];

	certificate[certificate_length++] = RSA_ALGORITHM_BYTE;
	certificate[certificate_length++] = MD5_ALGORITHM_BYTE;

	/* Now append first two bytes of message digest */
	mdbufptr = MD->digest;
	certificate[certificate_length++] = *mdbufptr++;
	certificate[certificate_length++] = *mdbufptr++;

	/* Now append the RSA-signed message digest packet: */
	for (i=0; i<bytecount+2; i++)
		certificate[certificate_length++] = outbuf[i];

	if (!filter_mode)
		fputc('.',pgpout);	/* Signal RSA signature completion. */

	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */

	return(certificate_length);	/* return length of certificate in bytes */

}	/* make_signature_certificate */


#ifdef VMS
void write_litlocal(FILE *g, char *fdl, short fdl_len)
{
/*
 * Local mode VMS, we write out the word VMS to say who owns the data then we follow
 * that with the file's FDL generated earlier by fdl_generate(). This FDL is preceded
 * by a sixteen bit size. The file follows.
 */
    fputc('\0', g); /* Kludge for null literal file name (supplied by FDL) */
    fputs("VMS ", g);
    fwrite(&fdl_len, 2, 1, g); /* Byte order *not* important, only VMS reads this!*/
    fwrite(fdl, 1, fdl_len, g);
}
#endif /* VMS */

/*======================================================================*/


int signfile(boolean nested, boolean separate_signature,
		char *mcguffin, char *infile, char *outfile,
		char lit_mode, char *literalfile)
/*	Write an RSA-signed message digest of input file to specified
	output file, and append input file to output file.
	separate_signature is TRUE iff we should not append the 
	plaintext to the output signature certificate.
	If lit_mode is MODE_TEXT, we know the infile is in canonical form.
	We create a CTB_LITERAL packet for the plaintext data.
*/
{	
	FILE *f;
	FILE *g;
	int certificate_length;	/* signature certificate length */
	byte certificate[MAX_SIGCERT_LENGTH];
	char lfile[MAX_PATH];
	byte signature_class;
#ifdef VMS
	char *fdl;
	short fdl_len;
#endif /* VMS */


	{	/* temporary scope for some buffers */
		word32 tstamp; byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
		byte userid[256];
		char keyfile[MAX_PATH];
		long fp;
		int pktlen, status;
		MD5_CTX MD;
		byte keyID[KEYFRAGSIZE];
		unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION], d[MAX_UNIT_PRECISION];
		unit p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION], u[MAX_UNIT_PRECISION];

		set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

		if (verbose)
			fprintf(pgpout,"signfile: infile = '%s', outfile = '%s', mode = '%c', literalfile = '%s'\n",
			infile,outfile,lit_mode,literalfile);

		if (MDfile(&MD, infile) < 0)
			return(-1);	/* problem with input file.  error return */

		userid[0] = '\0';
		if (mcguffin)
			strcpy((char *) userid,mcguffin);	/* Who we are looking for */

		if (getsecretkey(FALSE, FALSE, NULL, NULL, timestamp, NULL, NULL,
						 userid, n, e, d, p, q, u) < 0)
			return(-1);	/* problem with secret key file. error return. */

		extract_keyID(keyID, n);
		buildfilename(keyfile,PUBLIC_KEYRING_FILENAME); /* use default pathname */
		if ((status = getpublickey(FALSE, TRUE, keyfile, &fp, &pktlen,
				keyID, timestamp, userid, n, e)) != 0)
		{	if (status == 1)
			{	PascalToC((char *) userid);
				fprintf(pgpout, PSTR("\nKey for user ID \"%s\"\n\
has been revoked.  You cannot use this key.\n"), 
					LOCAL_CHARSET((char *)userid));	/* same msg in signkey */
			}
			return(-1);	/* problem with public key file. error return. */
		}

		if (lit_mode==MODE_TEXT) signature_class = SM_SIGNATURE_BYTE;
		else signature_class = SB_SIGNATURE_BYTE;

		certificate_length = make_signature_certificate(certificate, &MD,
			signature_class, n, d, p, q, u);
		if (certificate_length < 0)
			return(-1);	/* error return from make_signature_certificate() */
	}	/* end of scope for some buffers */

	/* open file f for read, in binary (not text) mode...*/
#ifdef VMS
	if (lit_mode == MODE_LOCAL) {
	    if (!(fdl_generate(infile, &fdl, &fdl_len ) & 01)) {
		fprintf(pgpout,PSTR("\n\007Can't open input plaintext file '%s'\n"),infile);
		return(-1);
	    }
	}
#endif /* VMS */
	if ((f = fopen(infile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open plaintext file '%s'\n"),infile);
		return(-1);
	}

	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen(outfile,FOPWBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't create signature file '%s'\n"),outfile);
		fclose(f);
		return(-1);
	}

	/* write out certificate record to outfile ... */
	fwrite(certificate,1,certificate_length,g);

	if (literalfile == NULL)
	{	/* Put in a zero byte to indicate no filename */
		lfile[0] = '\0';
	}
	else
	{	strcpy( lfile, literalfile );
		file_to_canon( lfile );
		CToPascal( lfile );
	}

	if (!separate_signature)
	{	if (!nested)
		{	word32 flen = fsize(f);
			word32 dummystamp = 0;
			if (lit_mode == MODE_LOCAL)
#ifdef VMS
			    write_ctb_len (g, CTB_LITERAL2_TYPE,
						flen + fdl_len + sizeof(fdl_len) + 6, TRUE);
#else
				/* debug check: should never get here */
			    fprintf(pgpout, "signfile: invalid mode\n");
#endif
			else
#ifdef USE_LITERAL2
			    write_ctb_len (g, CTB_LITERAL2_TYPE, flen + (unsigned char) lfile[0] + 6, FALSE);
#else
			    write_ctb_len (g, CTB_LITERAL_TYPE, flen, FALSE);
#endif /* USE_LITERAL2 */
			fwrite ( &lit_mode, 1, 1, g );	/*	write lit_mode */
			if (lit_mode == MODE_LOCAL) {	
#ifdef VMS
			    write_litlocal( g, fdl, fdl_len);
			    free(fdl);
#endif /* VMS */
			} else {
			    /* write literalfile name */
				fwrite (lfile, 1, (unsigned char) lfile[0]+1, g);
			    /* Dummy file creation timestamp */
			    fwrite ( &dummystamp, 1, sizeof(dummystamp), g);
			}
		}
		copyfile(f,g,-1L);	/* copy rest of file from file f to g */
	}

	fclose(f);
	if (write_error(g))
	{	fclose(g);
		return(-1);
	}
	fclose(g);
	return(0);	/* normal return */

}	/* signfile */


/*======================================================================*/


int compromise(char *keyguffin, char *keyfile)
{	
	FILE *f, *g;
	byte ctb;	/* Cipher Type Byte */
	int certificate_length;	/* signature certificate length */
	byte certificate[MAX_SIGCERT_LENGTH];
	byte keyID[KEYFRAGSIZE];
	word32 tstamp; byte *timestamp = (byte *) &tstamp;
	byte userid[256];
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	MD5_CTX MD;
	unit d[MAX_UNIT_PRECISION];
	unit p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION], u[MAX_UNIT_PRECISION];
	long fp, insertpos;
	int pktlen;
	int prec;
	char *scratchf;

	setoutdir(keyfile);
	scratchf = tempfile(0);

	strcpy((char *)userid, keyguffin);
	if (getsecretkey(FALSE, FALSE, NULL, NULL, timestamp, NULL, NULL,
					 userid, n, e, d, p, q, u) < 0)
		return(-1);	/* problem with secret key file. error return. */

	extract_keyID(keyID, n);

	strcpy((char *)userid, keyguffin);

	if (getpublickey(FALSE, TRUE, keyfile, &fp, &pktlen,
			keyID, timestamp, userid, n, e) < 0)
		return(-1);

	PascalToC((char *) userid);
	fprintf(pgpout, 
PSTR("\nDo you want to permanently revoke your public key\n\
by issuing a secret key compromise certificate\n\
for \"%s\" (y/N)? "), LOCAL_CHARSET((char *)userid));
	if (!getyesno('n'))
		return(-1);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(keyfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),keyfile);
		return(-1);
	}

	fseek (f, fp+pktlen, SEEK_SET);
	nextkeypacket(f, &ctb);
	if (ctb == CTB_KEYCTRL)
	{	insertpos = ftell(f);
		nextkeypacket(f, &ctb);
	}
	else
		insertpos = fp + pktlen;

	if (is_ctb_type(ctb, CTB_SKE_TYPE))
	{
		fprintf(pgpout, PSTR("This key has already been revoked.\n"));
		fclose(f);
		return(-1);
	}

	prec = global_precision;
	set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

	fseek(f, fp, SEEK_SET);
	/* Calculate signature */
	if (MDfile0_len(&MD, f, pktlen) < 0)
	{	fclose(f);
		return(-1);	/* problem with input file.  error return */
	}
	set_precision(prec);

	certificate_length = make_signature_certificate(certificate, &MD,
		KC_SIGNATURE_BYTE, n, d, p, q, u);
	if (certificate_length < 0)
	{	fclose(f);
		return(-1);	/* error return from make_signature_certificate() */
	}


	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen(scratchf,FOPWBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't create output file to update key ring.\n"));
		fclose(f);
		return(-1);
	}

	/* Copy pre-key and key to file g */
	rewind(f);
	copyfile (f, g, insertpos);

	/* write out certificate record to outfile ... */
	fwrite(certificate,1,certificate_length,g);

	/* Copy the remainder from file f to file g */
	copyfile (f, g, -1L);
	
	if (write_error(g))
	{	fclose(g);
		return(-1);
	}
	fclose(g);

	savetempbak(scratchf,keyfile);

	fprintf(pgpout, PSTR("\nKey compromise certificate created.\n"));
	return(0);	/* normal return */
}	/* compromise */

/*======================================================================*/


int signkey(char *keyguffin, char *sigguffin, char *keyfile)
/*  Write an RSA-signed message digest of key for user keyguffin in
    keyfile, using signature from user sigguffin.  Append
	the signature right after the key.
*/
{	
	FILE *f;
	FILE *g;
	byte ctb;	/* Cipher Type Byte */
	int certificate_length;	/* signature certificate length */
	byte certificate[MAX_SIGCERT_LENGTH];
	byte keyID[KEYFRAGSIZE], keyID2[KEYFRAGSIZE];
	word32 tstamp; byte *timestamp = (byte *) &tstamp;
	byte userid[256];
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	char pubring[MAX_PATH];
	long fp, fpusr;
	int pktlen, usrpktlen, usrctrllen;
	char *tempring;
	int status;

	/* Get signature key ID */
	strcpy((char *)userid,sigguffin);	/* Who we are looking for */
	if (getsecretkey(FALSE, FALSE, NULL, NULL, timestamp, NULL, NULL,
					 userid, n, e, NULL, NULL, NULL, NULL) < 0)
	{
		return(-1);	/* problem with secret key file. error return. */
	}
	extract_keyID(keyID, n);	/* Remember signer key ID */
	buildfilename(pubring,PUBLIC_KEYRING_FILENAME); /* use default pathname */
	if ((status = getpublickey(FALSE, FALSE, pubring, &fp, &pktlen,
			keyID, timestamp, userid, n, e)) != 0)
	{	if (status == 1)
		{	PascalToC((char *) userid);
			fprintf(pgpout, PSTR("\nKey for user ID \"%s\"\n\
has been revoked.  You cannot use this key.\n"), 
				LOCAL_CHARSET((char *)userid));	/* same msg in signfile */
		}
		return(-1);	/* problem with public key file. error return. */
	}

	strcpy((char *)userid, keyguffin);
	fprintf(pgpout, PSTR("\nLooking for key for user '%s':\n"), 
		LOCAL_CHARSET((char *)userid));

	if (getpublickey(FALSE, TRUE, keyfile, &fp, &pktlen,
			NULL, timestamp, userid, n, e) < 0)
		return(-1);
	PascalToC((char *) userid);
	if (getpubuserid (keyfile, fp, (byte *)keyguffin, &fpusr, &usrpktlen, FALSE) < 0)
		return(-1);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(keyfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),keyfile);
		return(-1);
	}

	fseek(f, fp, SEEK_SET);
	if (is_compromised(f))
	{	fprintf(pgpout, PSTR("\n\007Sorry, this key has been revoked by its owner.\n"));
		fclose(f);
		return(-1);
	}
	/* See if there is another signature with this keyID already */
	fseek (f, fpusr+usrpktlen, SEEK_SET);
	nextkeypacket(f, &ctb);		/* Add key control packet to len */
	usrctrllen = 0;
	if (ctb != CTB_KEYCTRL)
		fseek(f,fpusr+usrpktlen,SEEK_SET);
	else
		usrctrllen = (int) (ftell(f) - (fpusr+usrpktlen));
	for ( ; ; )
	{	status = readkeypacket(f,FALSE,&ctb,NULL,NULL,NULL,NULL,
					NULL,NULL,NULL,NULL,keyID2,NULL);
		if (status < 0  ||  is_key_ctb (ctb)  ||  ctb==CTB_USERID)
			break;
		if (equal_buffers(keyID, keyID2, KEYFRAGSIZE))
		{	fprintf(pgpout,PSTR("\n\007Key is already signed by user '%s'.\n"),
				LOCAL_CHARSET(sigguffin));
			fclose(f);
			return(-1);
		}
	}
	rewind(f);

	fprintf(pgpout,
PSTR("\n\nREAD CAREFULLY:  Based on your own direct first-hand knowledge, are\n\
you absolutely certain that you are prepared to solemnly certify that\n\
the above public key actually belongs to the user specified by the\n\
above user ID (y/N)? "));

	if (!getyesno('n'))
	{	fclose(f);
		return(-1);
	}

	{	/* temporary scope for some buffers */
		MD5_CTX MD;
		unit d[MAX_UNIT_PRECISION];
		unit p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION], u[MAX_UNIT_PRECISION];

		set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

		if ((g = fopen(keyfile,FOPRBIN)) == NULL)
		{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),keyfile);
			return(-1);
		}
		fseek(g, fp, SEEK_SET);
		/* Calculate signature */
		if (MDfile0_len(&MD, g, pktlen) < 0)
		{	fclose(g);
			fclose(f);
			return(-1);	/* problem with input file.  error return */
		}
		fclose(g);

		/* Add data from user id */
		CToPascal((char *)userid);
		MD5Update(&MD, userid+1, (int)(unsigned char)userid[0]);

		strcpy((char *)userid,sigguffin);	/* Who we are looking for */

		if (getsecretkey(FALSE, FALSE, NULL, NULL, timestamp, NULL, NULL,
						 userid, n, e, d, p, q, u) < 0)
		{	fclose(f);
			return(-1);	/* problem with secret key file. error return. */
		}

		certificate_length = make_signature_certificate(certificate, &MD,
			K0_SIGNATURE_BYTE, n, d, p, q, u);
		if (certificate_length < 0)
			return(-1);	/* error return from make_signature_certificate() */

	}	/* end of scope for some buffers */

	/* open file g for write, in binary (not text) mode...*/
	tempring = tempfile(TMP_TMPDIR);
	if ((g = fopen(tempring,FOPWBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't create output file to update key ring.\n"));
		fclose(f);
		return(-1);
	}

	/* Copy pre-key and key to file g */
	copyfile (f, g, fpusr+usrpktlen+usrctrllen);

	/* write out certificate record to outfile ... */
	fwrite(certificate,1,certificate_length,g);

	/* Add "trusty" control packet */
	write_trust (g, KC_SIGTRUST_ULTIMATE | KC_CONTIG);

	/* Copy the remainder from file f to file g */
	copyfile (f, g, -1L);
	
	fclose(f);
	if (write_error(g))
	{	fclose(g);
		return(-1);
	}
	fclose(g);

	savetempbak(tempring,keyfile);

	fprintf(pgpout, PSTR("\nKey signature certificate added.\n"));
	return(0);	/* normal return */

}	/* signkey */


/*======================================================================*/

int check_signaturefile(char *infile, char *outfile, boolean strip_signature,
			char *preserved_name)
{	/* Check signature in infile for validity.  Strip off the signature
	   and write the remaining packet to outfile.  If strip_signature,
	   also write the signature to outfile.sig.
	   the original filename is stored in preserved_name
	*/
	byte ctb,ctb2;	/* Cipher Type Bytes */
	char keyfile[MAX_PATH];	/* for getpublickey */
	char sigfile[MAX_PATH]; /* .sig file if strip_signature */
	char plainfile[MAX_PATH]; /* buffer for getstring() */
#ifndef CANONICAL_TEXT
	char *tempFileName;		/* Name for temporary uncanonicalized file */
	FILE *tempFile;
#endif /* !CANONICAL_TEXT */
	long fp;	/* unused, just to satisfy getpublickey */
	int pktlen;	/* unused, just to satisfy getpublickey */
	FILE *f;
	FILE *g;
	long start_text;	/* marks file position */
	int i,count;
	word16 cert_length;
	byte certbuf[MAX_SIGCERT_LENGTH];
	byteptr certificate; /* for parsing certificate buffer */
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte inbuf[MAX_BYTE_PRECISION];
	byte outbuf[MAX_BYTE_PRECISION];
	byte keyID[KEYFRAGSIZE];
	word32 tstamp;
	byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	word32 dummystamp;
	byte userid[256];
	MD5_CTX MD;
	boolean separate_signature;
	boolean fixedLiteral = FALSE;	/* Whether it's a fixed literal2 packet */
	extern boolean moreflag;
	extern char *myArgv[];
	extern int myArgc;
	char lit_mode = MODE_BINARY;
	unsigned char litfile[MAX_PATH];
	word32 text_len = -1;
	int	status;
	byte	*mdextras;
	byte	mdlensave;
	byte	version;
	byte	mdlen;	/* length of material to be added to MD calculation */
	byte	class;
	byte	algorithm;
	byte	mdlow2[2];
	char	org_sys[5];	    /* Name of originating system */
#ifdef VMS
	char	*fdl;
	short	fdl_len;
#endif

	fill0( keyID, KEYFRAGSIZE );

	set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

	buildfilename(keyfile,PUBLIC_KEYRING_FILENAME); /* use default pathname */

	if (verbose)
		fprintf(pgpout,"check_signaturefile: infile = '%s', outfile = '%s'\n",
		infile,outfile);

	if (preserved_name)
		*preserved_name = '\0';

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open ciphertext file '%s'\n"),infile);
		return(-1);
	}

	/******************** Read header CTB and length field ******************/

	fread(&ctb,1,1,f);	/* read certificate CTB byte */
	certificate = certbuf;
	*certificate++ = ctb;	/* copy ctb into certificate */

	if (!is_ctb(ctb) || !is_ctb_type(ctb,CTB_SKE_TYPE))
		goto badcert;	/* complain and return bad status */

	cert_length = getpastlength(ctb, f); /* read certificate length */
	certificate += ctb_llength(ctb);	/* either 1, 2, 4, or 8 */
	if (cert_length > MAX_SIGCERT_LENGTH-3)	/* Huge packet length */
		goto badcert;	/* complain and return bad status */

	/* read whole certificate: */
	if (fread((byteptr) certificate, 1, cert_length, f) < cert_length)
		/* bad packet length field */
		goto badcert;	/* complain and return bad status */

	version = *certificate++;
	if (version_error(version, VERSION_BYTE))
		goto err1;

	mdlensave = mdlen = *certificate++;	/* length of material to be added to MD */
	mdextras = certificate;	/* pointer to extra material for MD calculation */

	class = *certificate++;
	if (class != SM_SIGNATURE_BYTE  &&  class != SB_SIGNATURE_BYTE)
	{	(void) version_error(class, SM_SIGNATURE_BYTE);
		goto err1;
	}
	mdlen--;

	if (mdlen>0)	/* if more MD material is included... */
	{	for (i=0; i<SIZEOF_TIMESTAMP; ++i)
		{	timestamp[i] = *certificate++;
			mdlen--;
		}
	}

	if (mdlen>0)	/* if more MD material is included... */
	{	certificate+=2;	/* skip past unused validity period field */
		mdlen-=2;
	}

	for (i=0; i<KEYFRAGSIZE; i++)
		keyID[i] = *certificate++; /* copy rest of key fragment */

	algorithm = *certificate++;
	if (version_error(algorithm, RSA_ALGORITHM_BYTE))
		goto err1;

	algorithm = *certificate++;
	if (version_error(algorithm, MD5_ALGORITHM_BYTE))
		goto err1;

	mdlow2[0] = *certificate++;
	mdlow2[1] = *certificate++;

	/* getpublickey() sets precision for mpi2reg, if key not found: use
	   maximum precision to avoid error return from mpi2reg() */
	if (getpublickey(FALSE, verbose, keyfile, &fp, &pktlen,
			keyID, (byte *)&dummystamp, userid, n, e) < 0)
		set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

	if (mpi2reg((unitptr)inbuf,certificate) == -1)	/* get signed message digest */
		goto err1;
	certificate += countbytes((unitptr)inbuf)+2;

	if ((certificate-certbuf) != cert_length+3)
		/*	Bad length in signature certificate.  Off by 
			((certificate-certbuf) - (cert_length+3)) */
		goto badcert;	/* complain and return bad status */

	start_text = ftell(f);	/* mark position of text for later */

	if (fread(outbuf,1,1,f) < 1)	/* see if any plaintext is there */
	{	/*	Signature certificate has no plaintext following it.
			Must be in another file.  Go look. */
		separate_signature = TRUE;
		if (preserved_name)	/* let caller know there is no output file */
			strcpy(preserved_name, "/dev/null");
		fclose(f);
		fprintf(pgpout,PSTR("\nFile '%s' has signature, but with no text."),infile);
		if (myArgc > 3 && file_exists(myArgv[3]))
		{	outfile = myArgv[3];
			fprintf(pgpout,PSTR("\nText is assumed to be in file '%s'.\n"),outfile);
		}
		else
		{	strcpy(plainfile, outfile);
			outfile = plainfile;
			drop_extension(outfile);

			if (file_exists(outfile))
				fprintf(pgpout,PSTR("\nText is assumed to be in file '%s'.\n"),outfile);
			else
			{	fprintf(pgpout,PSTR("\nPlease enter filename of text that signature applies to: "));
				getstring(outfile,59,TRUE);	/* echo keyboard */
				if ((int)strlen(outfile) == 0)
					return(-1);
			}
		}
		/* open file f for read, in binary (not text) mode...*/
		if ((f = fopen(outfile,FOPRBIN)) == NULL)
		{	fprintf(pgpout,PSTR("\n\007Can't open file '%s'\n"),outfile);
			return(-1);
		}
		start_text = ftell(f);	/* mark position of text for later */
 		text_len = fsize(f);	/* remember length of text */
	}	/* had to open new input file */
	else
	{	separate_signature = FALSE;
		/*	We just read 1 byte, so outbuf[0] should contain a ctb, 
			maybe a CTB_LITERAL byte. */
		ctb2 = outbuf[0];
		fixedLiteral = is_ctb_type(ctb2,CTB_LITERAL2_TYPE);
		if (is_ctb(ctb2) && (is_ctb_type(ctb2,CTB_LITERAL_TYPE)||fixedLiteral))
		{	/* Read literal data */
			text_len = getpastlength(ctb2, f); /* read packet length */
			lit_mode = '\0';
			fread (&lit_mode,1,1,f);	/* get literal packet mode byte */
			if (lit_mode != MODE_TEXT && lit_mode != MODE_BINARY &&
				lit_mode != MODE_LOCAL)
			{	fprintf(pgpout,"\n\007Error: Illegal mode byte %02x in literal packet.\n",
					lit_mode);	/* English-only diagnostic for debugging */
				(void) version_error(lit_mode, MODE_BINARY);
				goto err1;
			}
			if (verbose)
				fprintf(pgpout, PSTR("File type: '%c'\n"), lit_mode);
			/* Read literal file name, use it if possible */
			litfile[0] = 0;
			fread (litfile,1,1,f);
			if( fixedLiteral )
				/* Get corrected text_len value by subtracting the length of
				   the filename and the timestamp and mode byte and litfile length byte */
				text_len -= litfile[0] + sizeof(dummystamp) + 2;
			if (litfile[0] > 0)
				fread (litfile+1,1,litfile[0],f);
			/* Use litfile if it's writeable and he didn't say an outfile */
			if (litfile[0] < MAX_PATH)
			{	PascalToC( (char *)litfile );
				if (verbose)
					fprintf(pgpout, PSTR("Original plaintext file name was: '%s'\n"), litfile);
				if (preserved_name)
					strcpy(preserved_name, (char *) litfile);
			}
			if (lit_mode == MODE_LOCAL) {
			    fread(org_sys, 1, 4, f); org_sys[4] = '\0';
#ifdef VMS
#define LOCAL_TEST !strncmp("VMS ",org_sys,4)
#else
#define LOCAL_TEST FALSE
#endif
			    if (LOCAL_TEST) {
#ifdef VMS
					fread(&fdl_len, 2, 1, f);
					fdl = (char *) malloc(fdl_len);
					fread(fdl, 1, fdl_len, f);
					if ((g = fdl_create( fdl, fdl_len, outfile, (char *) litfile)) == NULL) {
						fprintf(pgpout,"\n\007Unable to create file %s\n", outfile);
						return(-1);
					}
					free(fdl);
					if (preserved_name)
						strcpy(preserved_name, (char *) litfile);
					text_len -= (fdl_len + sizeof(fdl_len));
#endif /* VMS */
			    } else {
					fprintf(pgpout,"\n\007Unrecognised local binary type %s\n",org_sys);
					return(-1);
			    }
			} else {
			    /* Discard file creation timestamp for now */
			    fread (&dummystamp, 1, sizeof(dummystamp), f);
			}
			start_text = ftell(f);	/* mark position of text for later */
		}	/* packet is CTB_LITERAL_TYPE */
	}

	/* Use keyID prefix to look up key... */

	/*	Get and validate public key from a key file: */
	if (getpublickey(FALSE, verbose, keyfile, &fp, &pktlen,
			keyID, (byte *)&dummystamp, userid, n, e) < 0)
	{	/* Can't get public key.  Complain and process file copy anyway. */
		fprintf(pgpout,PSTR("\n\007WARNING: Can't find the right public key-- can't check signature integrity.\n"));
		goto outsig;
	}	/* Can't find public key */

	if (warn_signatures(keyfile, fp, pktlen, (char *)userid, TRUE) < 0)
		goto err1;

	/* Recover message digest via public key */
	mp_modexp((unitptr)outbuf,(unitptr)inbuf,e,n);

	if (!filter_mode)
		fputc('.',pgpout);	/* Signal RSA completion. */

	/* Unblock message digest, and convert to external byte order: */
	count = postunblock(outbuf, (unitptr)outbuf, n);
	if (count < 0)
	{	fprintf(pgpout,PSTR("\n\007Error: RSA-decrypted block is corrupted.\n\
This may be caused either by corrupted data or by using the wrong RSA key.\n"));
		goto outsig;	/* Output data anyway */
	}

	/* outbuf should contain message digest packet */
	/*==================================================================*/
	/* Look at nested stuff within RSA block... */

#ifdef XLOWFIRST
	/* Position of algorithm byte assumes LSB-first byteorder... */
	if (outbuf[count-1] != MD5_ALGORITHM_BYTE)
#else
	/* Position of algorithm byte assumes MSB-first byteorder... */
	if (outbuf[0] != MD5_ALGORITHM_BYTE)
#endif
	{	fprintf(pgpout,PSTR("\007\nUnrecognized message digest algorithm.\n"));
		fprintf(pgpout,PSTR("This may require a newer version of PGP.\n"));
		fprintf(pgpout,PSTR("Can't check signature integrity.\n"));
		goto outsig;	/* Output data anyway */
	}

#ifdef XLOWFIRST
	if (outbuf[0] != mdlow2[0]  ||  outbuf[1] != mdlow2[1])
#else
	if (outbuf[1] != mdlow2[0]  ||  outbuf[2] != mdlow2[1])
#endif
	{	fprintf(pgpout,PSTR("\n\007Error: RSA-decrypted block is corrupted.\n\
This may be caused either by corrupted data or by using the wrong RSA key.\n"));
		goto outsig;	/* Output data anyway */
	}

	/* Reposition file to where that plaintext begins... */
	fseek(f,start_text,SEEK_SET); /* reposition file from last ftell */

	MDfile0_len(&MD,f,text_len);/* compute a message digest from rest of file */

	MD_addbuffer (&MD, mdextras, mdlensave, TRUE); /* Finish message digest */

	convert_byteorder(timestamp,4); /* convert timestamp from external form */
	PascalToC((char *)userid);	/* for display */

	/* now compare computed MD with claimed MD */
#ifdef XLOWFIRST
	if (!equal_buffers((byte *)(MD.digest), outbuf, 16))
#else
	if (!equal_buffers((byte *)(MD.digest), outbuf+1, 16))
#endif
	{
#ifndef CANONICAL_TEXT
		/*	IF the signature is bad, AND this machine does not use MSDOS-style
			canonical text as its native text format, AND this is a detached
			signature certificate, AND this file appears to contain non-
			canonical ASCII text, THEN we convert the file to canonical text
			form and check the signature again.  This is because a detached
			signature certificate probably means the file is not currently in
			a canonical text packet, but it was in canonical text form when 
			the signature was created, so by re-canonicalizing it we can
			check the signature. */
		if (class == SM_SIGNATURE_BYTE && separate_signature && is_text_file(outfile))
		{	/* Reposition file to where the plaintext begins and canonicalize it */
			rewind( f );
			tempFileName = tempfile( TMP_WIPE | TMP_TMPDIR );
			if (verbose)
				fprintf(stderr, "signature checking failed, trying in canonical mode\n");
			if( ( tempFile = fopen( tempFileName, FOPRWBIN ) ) != NULL )
			{	/* We've opened a temporary work file, copy the text to it
				   with canonicalization */
				copyfile_to_canon( f, tempFile, -1L );

				/* Move back to the start of the file and recalculate the MD */
				rewind( tempFile );
				MDfile0_len( &MD, tempFile, -1L );
				MD_addbuffer( &MD, mdextras, mdlensave, TRUE );

				/* Clean up behind us */
				fclose( tempFile );
				rmtemp( tempFileName );

				/* Check if the signature is OK this time round */
#ifdef XLOWFIRST
				if( equal_buffers( ( byte * )( MD.digest ), outbuf, 16 ) )
#else
				if( equal_buffers( ( byte * )( MD.digest ), outbuf + 1, 16 ) )
#endif /* XLOWFIRST */
					goto goodsig;
			}
		}
#endif	/* !CANONICAL_TEXT */

		fprintf(pgpout,PSTR("\007\nWARNING: Bad signature, doesn't match file contents!\007\n"));
		fprintf(pgpout,PSTR("\nBad signature from user \"%s\".\n"),
			LOCAL_CHARSET((char *)userid));
		fprintf(pgpout,PSTR("Signature made %s\n"),ctdate((word32 *)timestamp));
		if (moreflag)
		{	/* more will scroll the message off the screen */
			fprintf(pgpout, PSTR("\nPress ENTER to continue..."));
			fflush(pgpout);
			getyesno('n');
		}
		goto outsig;	/* Output data anyway */
	}

goodsig:
	fprintf(pgpout,PSTR("\nGood signature from user \"%s\".\n"),
		LOCAL_CHARSET((char *)userid));
	fprintf(pgpout,PSTR("Signature made %s\n"),ctdate((word32 *)timestamp));

outsig:
	/* Reposition file to where that plaintext begins... */
	fseek(f,start_text,SEEK_SET); /* reposition file from last ftell */

	if (separate_signature)
		fprintf(pgpout,PSTR("\nSignature and text are separate.  No output file produced. "));
	else	/* signature precedes plaintext in file... */
	{	/* produce a plaintext output file from signature file */
		/* open file g for write, in binary or text mode...*/
		if (lit_mode==MODE_LOCAL) {
#ifdef VMS
			if (status = fdl_copyfile2bin( f, g, text_len)) {   /*  Copy ok? */
				if (status > 0)
					fprintf(stderr,"\n...copying to literal file\n");
				else
					perror("\nError copying from work file");
				fdl_close(g);
				goto err1;
			}
			fdl_close(g);
#endif /*VMS */
		} else {
			if (lit_mode == MODE_BINARY)
				g = fopen(outfile, FOPWBIN);
			else
				g = fopen(outfile, FOPWTXT);
			if (g == NULL)
		    {	fprintf(pgpout,PSTR("\n\007Can't create plaintext file '%s'\n"),outfile);
			    goto err1;
		    }
		    CONVERSION = (lit_mode == MODE_TEXT) ? EXT_CONV : NO_CONV;
		    if (lit_mode == MODE_BINARY)
			    status = copyfile(f, g, text_len);
			else
			    status = copyfile_from_canon(f, g, text_len);
			CONVERSION = NO_CONV;
			if (write_error(g) || status < 0)
			{	fclose(g);
				goto err1;
			}
			fclose(g);
		}

		if (strip_signature)
		{	/* Copy signature to a .sig file */
			strcpy (sigfile, outfile);
			force_extension(sigfile,SIG_EXTENSION);
			if (file_exists(sigfile))
			{	fprintf(pgpout,PSTR("\n\007Signature file '%s' already exists.  Overwrite (y/N)? "),
					sigfile);
				if (!getyesno('n'))
					goto err1;
			}
			if ((g = fopen(sigfile,FOPWBIN)) == NULL)
			{	fprintf(pgpout,PSTR("\n\007Can't create signature file '%s'\n"),sigfile);
				goto err1;
			}
			fseek (f,0L,SEEK_SET);
			copyfile (f,g,(unsigned long)(cert_length+ctb_llength(ctb)+1));
			if (write_error(g))
			{	fclose(g);
				goto err1;
			}
			fclose(g);
			fprintf(pgpout,PSTR("\nWriting signature certificate to '%s'\n"),sigfile);
		}
	}

	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	fclose(f);
	if (separate_signature)
		return(0);	/* normal return, no nested info */
	if (is_ctb(ctb2) && (is_ctb_type(ctb2,CTB_LITERAL_TYPE) || fixedLiteral))
		/* we already stripped away the CTB_LITERAL */
		return(0);	/* normal return, no nested info */
	/* Otherwise, it's best to assume a nested CTB */
	return(1);	/* nested information return */

badcert:	/* Bad packet.  Complain. */
	fprintf(pgpout,PSTR("\n\007Error: Badly-formed or corrupted signature certificate.\n"));
	fprintf(pgpout,PSTR("File \"%s\" does not have a properly-formed signature.\n"),infile);
	/* Now just drop through to error exit... */

err1:
	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	fclose(f);
	return(-1);	/* error return */

}	/* check_signaturefile */


int check_key_sig(FILE *fkey, long fpkey, int keypktlen, char *keyuserid,
	 FILE *fsig, long fpsig, char *keyfile, char *siguserid, byte *xtimestamp,
	 byte *sigclass)
{	/* Check signature of key in file fkey at position fpkey, using signature
	   in file fsig and position fpsig.  keyfile tells the file to use to
	   look for the public key in to check the sig.  Return 0 if OK, -1 if
	   we can't check the signature, -2 if bad or other problem.
	*/
	byte ctb;	/* Cipher Type Bytes */
	long fp;	/* unused, just to satisfy getpublickey */
	int pktlen;	/* unused, just to satisfy getpublickey */
	word16 cert_length;
	int i, count;
	byte certbuf[MAX_SIGCERT_LENGTH];
	byteptr certificate; /* for parsing certificate buffer */
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte inbuf[MAX_BYTE_PRECISION];
	byte outbuf[MAX_BYTE_PRECISION];
	byte keyID[KEYFRAGSIZE];
	MD5_CTX MD;
	byte mdlensave;
	byte *mdextras;
	word32 tstamp;
	byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	byte	version;
	byte	mdlen;	/* length of material to be added to MD calculation */
	byte	class;
	byte	algorithm;
	byte	mdlow2[2];

	fill0( keyID, KEYFRAGSIZE );

	set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

	/******************** Read header CTB and length field ******************/

	fseek(fsig, fpsig, SEEK_SET);
	fread(&ctb,1,1,fsig);	/* read certificate CTB byte */
	certificate = certbuf;
	*certificate++ = ctb;	/* copy ctb into certificate */

	if (!is_ctb(ctb) || !is_ctb_type(ctb,CTB_SKE_TYPE))
		goto badcert2;	/* complain and return bad status */

	cert_length = getpastlength(ctb, fsig); /* read certificate length */
	certificate += ctb_llength(ctb);	/* either 1, 2, 4, or 8 */
	if (cert_length > MAX_SIGCERT_LENGTH-3)	/* Huge packet length */
		goto badcert2;	/* complain and return bad status */

	/* read whole certificate: */
	if (fread((byteptr) certificate, 1, cert_length, fsig) < cert_length)
		/* bad packet length field */
		goto badcert2;	/* complain and return bad status */

	version = *certificate++;
	if (version_error(version, VERSION_BYTE))
		goto err2;

	mdlensave = mdlen = *certificate++;	/* length of material to be added to MD */
	mdextras = certificate;	/* pointer to extra material for MD calculation */

	*sigclass = class = *certificate++;
	if (class != K0_SIGNATURE_BYTE  &&  class != K1_SIGNATURE_BYTE &&
		class != K2_SIGNATURE_BYTE  &&  class != K3_SIGNATURE_BYTE &&
		class != KC_SIGNATURE_BYTE)
	{	(void) version_error(class, K0_SIGNATURE_BYTE);
		goto err2;
	}
	mdlen--;

	if (mdlen>0)	/* if more MD material is included... */
	{	for (i=0; i<SIZEOF_TIMESTAMP; ++i)
		{	timestamp[i] = *certificate++;
			mdlen--;
		}
	}

	if (mdlen>0)	/* if more MD material is included... */
	{	certificate+=2;	/* skip past unused validity period word */
		mdlen-=2;
	}

	if (mdlen>0)	/* if more MD material is included... */
		certificate+=mdlen;	/* skip over the rest */

	for (i=0; i<KEYFRAGSIZE; i++)
		keyID[i] = *certificate++; /* copy rest of key fragment */

	algorithm = *certificate++;
	if (version_error(algorithm, RSA_ALGORITHM_BYTE))
		goto err2;

	algorithm = *certificate++;
	if (version_error(algorithm, MD5_ALGORITHM_BYTE))
		goto err2;

	/* Grab 1st 2 bytes of message digest */
	mdlow2[0] = *certificate++;
	mdlow2[1] = *certificate++;

	/* We used to set precision here based on certificate value,
	 * but it was sometimes less than that based on n.  Read public
	 * key here to set precision, before we go on.
	 */
	/* This sets precision, too, based on n. */
	if (getpublickey(TRUE, FALSE, keyfile, &fp, &pktlen,
			keyID, xtimestamp, (unsigned char *)siguserid, n, e) < 0)
		goto err1;

	if (mpi2reg((unitptr)inbuf,certificate) == -1)	/* get signed message digest */
		goto err1;
	certificate += countbytes((unitptr)inbuf)+2;

	if ((certificate-certbuf) != cert_length+3)
		/*	Bad length in signature certificate.  Off by 
			((certificate-certbuf) - (cert_length+3)) */
		goto badcert2;	/* complain and return bad status */

	/* Recover message digest via public key */
	mp_modexp((unitptr)outbuf,(unitptr)inbuf,e,n);

	/* Unblock message digest, and convert to external byte order: */
	if ((count = postunblock(outbuf, (unitptr)outbuf, n)) < 0)
		goto err2;	/* Bad RSA decrypt.  Corruption, or wrong key. */

	/* outbuf should contain message digest packet */
	/*==================================================================*/
	/* Look at nested stuff within RSA block... */

#ifdef XLOWFIRST	/* assumes LSB-first */
	if (outbuf[count-1] != MD5_ALGORITHM_BYTE)
		goto err2;	/* Bad RSA decrypt.  Corruption, or wrong key. */
	if (outbuf[0] != mdlow2[0]  ||  outbuf[1] != mdlow2[1])
		goto err2;	/* Bad RSA decrypt.  Corruption, or wrong key. */
#else
	if( count );	/* Get rid of unused variable warning */
	if (outbuf[0] != MD5_ALGORITHM_BYTE)
		goto err2;	/* Bad RSA decrypt.  Corruption, or wrong key. */
	if (outbuf[1] != mdlow2[0]  ||  outbuf[2] != mdlow2[1])
		goto err2;	/* Bad RSA decrypt.  Corruption, or wrong key. */
#endif

	/* Position file to where that plaintext begins... */
	fseek(fkey,fpkey,SEEK_SET);

	/* compute a message digest from key packet */
	MDfile0_len(&MD,fkey,keypktlen);
	/* Add data from user id */
	if (class != KC_SIGNATURE_BYTE)
		MD5Update(&MD, (unsigned char *) keyuserid+1, (int)(unsigned char)keyuserid[0]);
	/* Add time and class data */
	MD_addbuffer (&MD, mdextras, mdlensave, TRUE);	/* Finish message digest */

	/* now compare computed MD with claimed MD */
#ifdef XLOWFIRST	/* assumes LSB-first */
	if (!equal_buffers((byte *)(MD.digest), outbuf, 16))
	goto err2;
#else
	if (!equal_buffers((byte *)(MD.digest), outbuf+1, 16))
	goto err2;
#endif

	convert_byteorder(timestamp,4); /* convert timestamp from external form */
	memcpy (xtimestamp, timestamp, 4);	/* Return signature timestamp */

	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	return(0);	/* normal return */

err1:
	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	return(-1);	/* error return */

badcert2:	/* Bad packet.  Complain. */
	fprintf(pgpout,PSTR("\n\007Error: Badly-formed or corrupted signature certificate.\n"));
	/* Now just drop through to error exit... */

err2:
	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	return(-2);	/* error return */

}	/* check_key_sig */



/*======================================================================*/
int squish_and_idea_file(byte *ideakey, FILE *f, FILE *g, 
	boolean attempt_compression)
{
	FILE *t;
	char *tempf;
	byte ctb;
	word32 fpos, fpos0;
	extern char plainfile[];

	/*
	**  If the caller specified that we should attempt compression, we
	**  create a temporary file 't' and compress our input file 'f' into
	**  't'.  Ideally, we would see if we get a good compression ratio 
	**  and if we did, then use file 't' for input and write a 
	**  CTB_COMPRESSED prefix.  But in this implementation we just always
	**  use the compressed output, even if it didn't compress well.
	*/

	rewind( f );

	if (!attempt_compression)
		t = f;	/* skip compression attempt */
	else	/* attempt compression-- get a tempfile */ 
		if ((tempf = tempfile(TMP_TMPDIR|TMP_WIPE)) == NULL ||
			(t = fopen(tempf, FOPRWBIN)) == NULL) /* error: no tempfile */
			t = f;	/* skip compression attempt */
		else	/* attempt compression */ 
		{
			extern int zipup( FILE *, FILE * );


			if (verbose) fprintf(pgpout,"\nCompressing [%s] to %s ",
				plainfile, tempf );

			/* We don't put a length field on CTB_COMPRESSED yet */
			ctb = CTB_COMPRESSED;		/* use compression prefix CTB */
			fwrite( &ctb, 1, 1, t );	/* write CTB_COMPRESSED */
	  		/* No CTB packet length specified means indefinite length. */
			ctb = ZIP2_ALGORITHM_BYTE; 	/* use ZIP compression */
			fwrite( &ctb, 1, 1, t );	/* write ZIP algorithm byte */

			/* Compression the file */
			zipup( f, t);
			if (write_error(t))
			{	fclose(t);
				rmtemp(tempf);
				return(-1);
			}
			if (verbose) fprintf(pgpout, PSTR("compressed.  ") );
			else if (!filter_mode)
				fputc('.',pgpout);	/* show progress */
			rewind( t );
	  	}

	/*	Now write out file thru IDEA cipher... */

	/* Write CTB prefix, leave 4 bytes for later length */
	fpos0 = ftell(g);
	write_ctb_len (g, CTB_CKE_TYPE, 0L, TRUE);
	fpos = ftell(g) - fpos0;

	idea_file( ideakey, ENCRYPT_IT, t, g, fsize(t) );

	/* Now re-write CTB prefix, this time with length */
	fseek(g,fpos0,SEEK_SET);
	write_ctb_len (g, CTB_CKE_TYPE, fsize(g)-fpos, TRUE);

	if (t != f)	
	{	fclose( t );  /* close and remove the temporary file */
		rmtemp(tempf);
	}

	return(0);	/* normal return */

}	/* squish_and_idea_file */


int squish_file(char *infile, char *outfile)
{
	FILE *f, *g;
	byte ctb;
	extern int zip( FILE *, FILE * );

	if (verbose)
		fprintf(pgpout,"squish_file: infile = '%s', outfile = '%s'\n",
		infile,outfile);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen( infile, FOPRBIN )) == NULL)
	{
		fprintf(pgpout,PSTR("\n\007Can't open file '%s'\n"), infile );
		return(-1);
	}

	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen( outfile, FOPWBIN )) == NULL)
	{
		fprintf(pgpout,PSTR("\n\007Can't create compressed file '%s'\n"), outfile );
		fclose(f);
		return(-1);
	}


	if (verbose) fprintf(pgpout, PSTR("Compressing file..."));

	/* We don't put a length field on CTB_COMPRESSED yet */
	ctb = CTB_COMPRESSED;		/* use compression prefix CTB */
	fwrite( &ctb, 1, 1, g );	/* write CTB_COMPRESSED */
	/* No CTB packet length specified means indefinite length. */
	ctb = ZIP2_ALGORITHM_BYTE; 	/* use ZIP compression */
	fwrite( &ctb, 1, 1, g );	/* write ZIP algorithm byte */

	/* Compress/store the file */
	zipup( f, g );
	if (verbose) fprintf(pgpout, PSTR("compressed.  ") );

	fclose (f);
	if (write_error(g))
	{	fclose(g);
		return -1;
	}
	fclose (g);
	return(0);
}   /* squish_file */

#define NOECHO1 1	/* Disable password from being displayed on screen */
#define NOECHO2 2	/* Disable password from being displayed on screen */

int idea_encryptfile(char *infile, char *outfile, 
	boolean attempt_compression)
{
	FILE *f;	/* input file */
	FILE *g;	/* output file */
	byte ideakey[16];
	byte passphrase[256];
	extern char password[];

	if (verbose)
		fprintf(pgpout,"idea_encryptfile: infile = '%s', outfile = '%s'\n",
		infile,outfile);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen( infile, FOPRBIN )) == NULL)
	{
		fprintf(pgpout,PSTR("\n\007Can't open plaintext file '%s'\n"), infile );
		return(-1);
	}

	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen( outfile, FOPWBIN )) == NULL)
	{
		fprintf(pgpout,PSTR("\n\007Can't create ciphertext file '%s'\n"), outfile );
		fclose(f);
		return(-1);
	}

	/* Get IDEA password, hashed to a key */
	if (*password != '\0')
		hashpass(password, strlen(password), ideakey);
	else {
		fprintf(pgpout,PSTR("\nYou need a pass phrase to encrypt the file. "));
		if (GetHashedPassPhrase((char *)passphrase,(char *)ideakey,NOECHO2) <= 0)
		{	fclose(f);
			fclose(g);
			return(-1);
		}
	}

	if (!filter_mode)
	{	fprintf(pgpout,PSTR("Just a moment..."));  /* this may take a while */
		fflush(pgpout);
	}

	/* Now compress the plaintext and encrypt it with IDEA... */
	squish_and_idea_file( ideakey, f, g, attempt_compression );

	burn(ideakey);	/* burn sensitive data on stack */
	burn(passphrase);

	fclose(f);
	if (write_error(g))
	{	fclose(g);
		return -1;
	}
	fclose(g);

	return(0);

}	/* idea_encryptfile */


/*======================================================================*/


int encryptfile(char *mcguffin, char *infile, char *outfile, 
	boolean attempt_compression)
{
	byte randompad[MAX_BYTE_PRECISION];	/* buffer of random pad bytes */
	int i,blocksize,ckp_length;
	FILE *f;
	FILE *g;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte inbuf[MAX_BYTE_PRECISION];
	byte outbuf[MAX_BYTE_PRECISION];
	word32 tstamp; byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	byte userid[256];
	byte ideakey[16]; /* must be big enough for make_random_ideakey */
	char keyfile[MAX_PATH];	/* for getpublickey */
	long fp;	/* unused, just to satisfy getpublickey */
	int pktlen;	/* unused, just to satisfy getpublickey */
	word32 chksum;
	byte	ver, alg;


	buildfilename(keyfile,PUBLIC_KEYRING_FILENAME); /* use default pathname */

	if (verbose)
		fprintf(pgpout,"encryptfile: infile = %s, outfile = %s\n",
		infile,outfile);

	userid[0] = '\0';
	if (mcguffin)
		strcpy((char *)userid,mcguffin);/* Who we are looking for (C string) */

	/*	Get and validate public key from a key file: */
	if (getpublickey(FALSE, !filter_mode, keyfile, &fp, &pktlen, NULL, timestamp, userid, n, e) < 0)
	{	return(-1);
	}

	if (warn_signatures(keyfile, fp, pktlen, (char *)userid, FALSE) < 0)
		return(-1);

	/* set_precision has been properly called by getpublickey */

	/*	Note that RSA key must be at least big enough to encipher a 
		complete conventional key packet in a single RSA block.
	*/

	blocksize = countbytes(n)-1;	/* size of a plaintext block */
	if (blocksize < 31)
	{	fprintf(pgpout,"\n\007Error: RSA key length must be at least 256 bits.\n");
		return(-1);
	}

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen( infile, FOPRBIN )) == NULL)
	{
		fprintf(pgpout,PSTR("\n\007Can't open plaintext file '%s'\n"), infile );
		return(-1);
	}

	/* open file g for write, in binary (not text) mode...*/
	if ((g = fopen( outfile, FOPWBIN )) == NULL)
	{
		fprintf(pgpout,PSTR("\n\007Can't create ciphertext file '%s'\n"), outfile );
		fclose(f);
		return(-1);
	}

	/*	Now we have to time some user keystrokes to get some random
		bytes for generating a random IDEA cipher key.
		We would have to solicit fewer keystrokes for random IDEA
		key generation if we had already accumulated some keystrokes
		incidental to some other purpose, such as asking for a password 
		to decode an RSA secret key so that a signature could be applied 
		to the message before encrypting it.
	*/

	ckp_length = make_random_ideakey(ideakey);
	/* Returns an 16 byte random IDEA key */

#ifdef XLOWFIRST	/* Assumes LSB-first order */
	for (i=0; i<ckp_length; ++i)
		outbuf[i] = ideakey[i];
	/* Compute and append checksum to the key */
	chksum = checksum (outbuf, ckp_length);
	put_word16((word16) chksum, outbuf+ckp_length);
	ckp_length += 2;
	/* Append identifier byte to key */
	outbuf[ckp_length++] = IDEA_ALGORITHM_BYTE;
#else	/* Assumes MSB-first order */
	/* Prepend identifier byte to key */
	outbuf[0] = IDEA_ALGORITHM_BYTE;
	for (i=0; i<ckp_length; ++i)
		outbuf[i+1] = ideakey[i];
	/* Compute and append checksum to the key */
	chksum = checksum (outbuf+1, ckp_length);
	ckp_length++;
	put_word16((word16) chksum, outbuf+ckp_length);
	ckp_length += 2;
#endif

	/*
	**	Messages encrypted with a public key should use random padding, 
	**	while messages "signed" with a secret key should use constant 
	**	padding.
	*/

	for (i = 0; i < (blocksize - ckp_length); i++)
		while (!(randompad[i] = randombyte()))
			;	/* Allow only nonzero values */

	/*
	**	Note that RSA key must be at least big enough to encipher a
	**	complete conventional key packet in a single RSA block.
	*/

	/* ckp_length is conventional key packet length. */

	preblock( (unitptr)inbuf, outbuf, ckp_length, n, randompad );
	mp_modexp( (unitptr)outbuf, (unitptr)inbuf, e, n );	/* RSA encrypt */

	/* write out header record to outfile ... */

	/* PKE is Public Key Encryption */
	write_ctb_len (g, CTB_PKE_TYPE,
		 1+KEYFRAGSIZE+1+2+countbytes((unitptr)outbuf), FALSE);

	/* Write version byte */
	ver = VERSION_BYTE;
	fwrite (&ver, 1, 1, g);

	writekeyID( n, g );	/* write msg prefix fragment of modulus n */

	/* Write algorithm byte */
	alg = RSA_ALGORITHM_BYTE;
	fwrite (&alg, 1, 1, g);

	/* convert RSA ciphertext block via reg2mpi and write to file */

	write_mpi( (unitptr)outbuf, g, FALSE );

	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */

	/**	Finished with RSA block containing IDEA key. */

	/* Now compress the plaintext and encrypt it with IDEA... */
	squish_and_idea_file( ideakey, f, g, attempt_compression );

	burn(ideakey);	/* burn sensitive data on stack */

	fclose(f);
	if (write_error(g))
	{	fclose(g);
		return -1;
	}
	fclose(g);

	return(0);
}	/* encryptfile */


/*======================================================================*/
int make_literal(char *infile, char *outfile, char lit_mode, char *literalfile)
{	/*	Prepend a CTB_LITERAL prefix to a file.  Convert to canonical form if
		lit_mode is MODE_TEXT.
	*/
	char lfile[MAX_PATH];
	FILE *f;
	FILE *g;
	int status = 0;
#ifdef VMS
	char *fdl;
	short fdl_len;
#endif /* VMS */

	word32 flen, fpos;
	word32 dummystamp = 0;

	if (verbose)
		fprintf(pgpout,"make_literal: infile = %s, outfile = %s, mode = '%c', literalfile = '%s'\n",
		infile,outfile,lit_mode,literalfile);

	/* open file f for read, in binary or text mode...*/

#ifdef VMS
	if (lit_mode == MODE_LOCAL) {
	    if (!(fdl_generate(infile, &fdl, &fdl_len ) & 01)) {
		fprintf(pgpout,PSTR("\n\007Can't open input plaintext file '%s'\n"),infile);
		return(-1);
	    }
	}
#endif /*VMS*/
	if (lit_mode == MODE_TEXT)
		f = fopen(infile, FOPRTXT);
	else
		f = fopen(infile, FOPRBIN);
	if (f == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open input plaintext file '%s'\n"),infile);
		return(-1);
	}
	flen = fsize(f);

	/* 	open file g for write, in binary (not text) mode... */
	if ((g = fopen( outfile,FOPWBIN )) == NULL)
	{	fprintf(pgpout, PSTR("\n\007Can't create plaintext file '%s'\n"), outfile );
		goto err1;
	}

	if (literalfile == NULL)
	{	/* Put in a zero byte to indicate no filename */
		lfile[0] = '\0';
	}
	else
	{	strcpy( lfile, literalfile );
		file_to_canon( lfile );
		CToPascal( lfile );
	}

#ifdef USE_LITERAL2
#define	LENGTH_FIELD		(flen + (unsigned char) lfile[0] + 6)
#define	LIT_TYPE	CTB_LITERAL2_TYPE
#else
#define	LENGTH_FIELD	flen
#define	LIT_TYPE	CTB_LITERAL_TYPE
#endif
	if (lit_mode == MODE_BINARY)
		write_ctb_len (g, LIT_TYPE, LENGTH_FIELD, FALSE);
#ifdef VMS
	else if (lit_mode == MODE_LOCAL)
		write_ctb_len (g, CTB_LITERAL2_TYPE, flen + fdl_len + sizeof(fdl_len) + 6, TRUE);
#endif /* VMS */
	else /* Will put in size field later for text mode */
		write_ctb_len (g, LIT_TYPE, 0L, TRUE);
#ifdef USE_LITERAL2
	fpos = ftell(g);
#endif
	fwrite ( &lit_mode, 1, 1, g );	/*	write lit_mode */

	if (lit_mode == MODE_LOCAL) {
#ifdef VMS
	    write_litlocal( g, fdl, fdl_len);
	    free(fdl);
#else
	    ;   /*  Null statement if we don't have anything to do! */
#endif /* VMS */
	} else {
	    /* write literalfile name */
		fwrite (lfile, 1, (unsigned char) lfile[0]+1, g);
	    /* Dummy file creation timestamp */
	    fwrite ( &dummystamp, 1, sizeof(dummystamp), g);
	}
#ifndef USE_LITERAL2
	fpos = ftell(g);
#endif

	if ((lit_mode == MODE_BINARY) || (lit_mode == MODE_LOCAL)) {
		if (copyfile( f, g, -1L )) {
		    fprintf(pgpout,"\n\007Unable to append to literal plaintext file");
		    perror("\n");
		    fclose(g);
		    goto err1;
		}
	} else {
		CONVERSION = (lit_mode == MODE_TEXT) ? INT_CONV : NO_CONV;
		status = copyfile_to_canon( f, g, -1L );
		CONVERSION = NO_CONV;
		/* Re-write CTB with correct length info */
		rewind (g);
		write_ctb_len (g, LIT_TYPE, fsize(g)-fpos, TRUE);
	}
	if (write_error(g) || status < 0)
	{	fclose(g);
		goto err1;
	}
	fclose(g);
	fclose(f);
	return(0);	/* normal return */

err1:
	fclose(f);
	return(-1);	/* error return */

}	/* make_literal */
#undef LENGTH_FIELD
#undef LIT_TYPE


/*======================================================================*/
int strip_literal(char *infile, char *outfile, char *preserved_name,
		char *lit_mode)
{	/*	Strip off literal prefix from infile, copying to outfile.
		Get lit_mode and literalfile info from
		the prefix.  Replace outfile with literalfile unless
		literalfile is illegal
		the original filename is stored in preserved_name
		If lit_mode is MODE_TEXT, convert from canonical form as we
		copy the data.
	*/
	byte ctb;	/* Cipher Type Byte */
	FILE *f;
	FILE *g;
	word32 LITlength = 0;
	unsigned char litfile[MAX_PATH];
	word32 dummystamp;
	char	org_sys[5];	    /* Name of originating system */
	int	status;
#ifdef VMS
	char	*fdl;
	short	fdl_len;
#endif
	*lit_mode = MODE_BINARY;
	if (verbose)
		fprintf(pgpout,"strip_literal: infile = %s, outfile = %s\n",
		infile,outfile);

	if (preserved_name)
		*preserved_name = '\0';

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open input plaintext file '%s'\n"),infile);
		return(-1);
	}

	fread(&ctb,1,1,f);	/* read Cipher Type Byte */

	if (!is_ctb(ctb) || !(is_ctb_type(ctb,CTB_LITERAL_TYPE) ||
	    is_ctb_type(ctb,CTB_LITERAL2_TYPE)))
	{	/* debug message in English only -- something got corrupted */
		fprintf(pgpout,"\n\007'%s' is not a literal plaintext file.\n",infile);
		fclose(f);
		return(-1);
	}

	LITlength = getpastlength(ctb, f); /* read packet length */

	/* Read literal data */
	*lit_mode = '\0';
	fread (lit_mode,1,1,f);
	if ((*lit_mode != MODE_BINARY) && (*lit_mode != MODE_TEXT)
	    && (*lit_mode != MODE_LOCAL))
	{	(void) version_error(*lit_mode, MODE_TEXT);
		fclose(f);
		return(-1);
	}
	if (verbose)
		fprintf(pgpout, PSTR("File type: '%c'\n"), *lit_mode);
	/* Read literal file name, use it if possible */
	litfile[0] = 0;
	fread (litfile,1,1,f); 
	if (litfile[0] > 0)
		fread (litfile+1,1,litfile[0],f);
	if (is_ctb_type(ctb, CTB_LITERAL2_TYPE))
	{	/* subtract header length: namelength + lengthbyte + modebyte + stamp */
		LITlength -= litfile[0] + 2 + sizeof(dummystamp);
	}
	/* Use litfile if it's writeable and he didn't say an outfile */
	if (litfile[0] < MAX_PATH)
	{	PascalToC( (char *)litfile );
		if (verbose)
			fprintf(pgpout, PSTR("Original plaintext file name was: '%s'\n"), litfile);
		if (preserved_name)
			strcpy(preserved_name, (char *) litfile);
	}
	if (*lit_mode == MODE_LOCAL) {
	    fread(org_sys, 1, 4, f); org_sys[4] = '\0';
#ifdef VMS
#define LOCAL_TEST !strncmp("VMS ",org_sys,4)
#else
#define LOCAL_TEST FALSE
#endif
	    if (LOCAL_TEST) {
#ifdef VMS
			remove(outfile);  /*  Prevent litter, we recreate the file with correct chars. */
			fread(&fdl_len, 2, 1, f);
			fdl = (char *) malloc(fdl_len);
			fread(fdl, 1, fdl_len, f);
			if ((g = fdl_create( fdl, fdl_len, outfile, (char *) litfile)) == NULL) {
				fprintf(pgpout,"\n\007Unable to create file %s\n", outfile);
				return(-1);
			}
			free(fdl);
			if (preserved_name)
				strcpy(preserved_name, (char *) litfile);
			LITlength -= (fdl_len + sizeof(fdl_len));
#endif /* VMS */
	    } else {
			fprintf(pgpout,"\n\007Unrecognised local binary type %s\n",org_sys);
			return(-1);
	    }
	} else {
	    /* Discard file creation timestamp for now */
	    fread (&dummystamp, 1, sizeof(dummystamp), f);
	}

	if (*lit_mode==MODE_LOCAL) {
#ifdef VMS
	    if (status = fdl_copyfile2bin( f, g, LITlength)) {   /*  Copy ok? */
			if (status > 0)
				fprintf(stderr,"\n...copying to literal file\n");
			else
				perror("\nError copying from work file");
			fdl_close(g);
			goto err1;
	    }
	    fdl_close(g);
#endif /*VMS */
	} else {
	    if (*lit_mode == MODE_TEXT)
			g = fopen(outfile, FOPWTXT);
	    else
			g = fopen(outfile, FOPWBIN);
	    if (g == NULL)
	    {	fprintf(pgpout, PSTR("\n\007Can't create plaintext file '%s'\n"), outfile );
		    goto err1;
	    }
	    /* copy rest of literal plaintext file */
	    CONVERSION = (*lit_mode == MODE_TEXT) ? EXT_CONV : NO_CONV;
	    if (*lit_mode == MODE_BINARY)
		    status = copyfile(f, g, LITlength);
		else
		    status = copyfile_from_canon(f, g, LITlength);
	    CONVERSION = NO_CONV;
		if (write_error(g) || status < 0)
		{	fclose(g);
			goto err1;
		}
		fclose(g);
	}

	fclose(f);
	return(0);	/* normal return */

err1:
	fclose(f);
	return(-1);	/* error return */

}	/* strip_literal */


/*======================================================================*/


int decryptfile(char *infile, char *outfile)
{
	byte ctb;	/* Cipher Type Byte */
	byte ctbCKE; /* Cipher Type Byte */
	FILE *f;
	FILE *g;
	int count, status;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION], d[MAX_UNIT_PRECISION];
	unit p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION], u[MAX_UNIT_PRECISION];
	byte inbuf[MAX_BYTE_PRECISION];
	byte outbuf[MAX_BYTE_PRECISION];
	byte keyID[KEYFRAGSIZE];
	word32 tstamp; byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	byte userid[256];
	word32 flen;
	byte ver, alg;
	word16 chksum;

	set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */

	if (verbose)
		fprintf(pgpout,"decryptfile: infile = %s, outfile = %s\n",
		infile,outfile);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open ciphertext file '%s'\n"),infile);
		return(-1);
	}

	fread(&ctb,1,1,f);	/* read Cipher Type Byte */
	if (!is_ctb(ctb))
	{	fprintf(pgpout,PSTR("\n\007'%s' is not a cipher file.\n"),infile);
		fclose(f);
		return(-1);
	}

	/* PKE is Public Key Encryption */
	if (!is_ctb_type(ctb,CTB_PKE_TYPE))
	{	fprintf(pgpout,PSTR("\n\007'%s' is not enciphered with a public key.\n"),infile);
		fclose(f);
		return(-1);
	}

	getpastlength(ctb, f); /* read packet length */

	/* Read and check version */
	fread (&ver, 1, 1, f);
	if (version_error(ver, VERSION_BYTE))
	{	fclose (f);
		return (-1);
	}

	fread(keyID,1,KEYFRAGSIZE,f); /* read key ID */
	/* Use keyID prefix to look up key. */

	/* Read and check algorithm */
	fread (&alg, 1, 1, f);
	if (version_error(alg, RSA_ALGORITHM_BYTE))
	{	fclose (f);
		return (-1);
	}

	/*	Get and validate secret key from a key file: */
	if (getsecretkey(FALSE, !filter_mode, NULL, keyID, timestamp, NULL, NULL,userid,
						 n, e, d, p, q, u) < 0)
	{	fclose(f);
		return(-1);
	}

	/*	Note that RSA key must be at least big enough to encipher a
		complete conventional key packet in a single RSA block. */

	/*==================================================================*/
	/* read ciphertext block, converting to internal format: */
	read_mpi((unitptr)inbuf, f, FALSE, FALSE);

	if (!filter_mode)
	{	fprintf(pgpout,PSTR("Just a moment..."));	/* RSA will take a while. */
		fflush(pgpout);
	}

	rsa_decrypt((unitptr)outbuf, (unitptr)inbuf, d, p, q, u);

	if (!filter_mode)
		fputc('.',pgpout);	/* Signal RSA completion. */

	if ((count = postunblock(outbuf, (unitptr)outbuf, n)) < 0)
	{	fprintf(pgpout,PSTR("\n\007Error: RSA-decrypted block is corrupted.\n\
This may be caused either by corrupted data or by using the wrong RSA key.\n"));
		fclose(f);
		return(-1);
	}

	/* Verify that top of buffer has correct algorithm byte */
	--count;	/* one less byte to drop algorithm byte */
#ifdef XLOWFIRST	/* Assumes LSB-first order */
	if (version_error(outbuf[count], IDEA_ALGORITHM_BYTE))
#else		/* Assumes MSB-first order */
	if (version_error(outbuf[0], IDEA_ALGORITHM_BYTE))
#endif
	{	fclose(f);
		return(-1);
	}

	/* Verify checksum */
	count -= 2;	/* back up before checksum */
#ifdef XLOWFIRST	/* Assumes LSB-first order */
	chksum = fetch_word16(outbuf+count);
	if (chksum != checksum(outbuf, count))
#else		/* Assumes MSB-first order */
	chksum = fetch_word16(outbuf+1+count);
	if (chksum != checksum(outbuf+1, count))
#endif
	{	fprintf(pgpout,PSTR("\n\007Error: RSA-decrypted block is corrupted.\n\
This may be caused either by corrupted data or by using the wrong RSA key.\n"));
		fclose(f);
		return(-1);
	}

	/* outbuf should contain random IDEA key packet */
	/*==================================================================*/

	/* 	open file g for write, in binary (not text) mode... */

	if ((g = fopen( outfile, FOPWBIN )) == NULL)
	{	fprintf(pgpout, PSTR("\n\007Can't create plaintext file '%s'\n"), outfile );
		goto err1;
	}

	fread(&ctbCKE,1,1,f);	/* read Cipher Type Byte, should be CTB_CKE */
	if (!is_ctb(ctbCKE) || !is_ctb_type(ctbCKE,CTB_CKE_TYPE))
	{	/* Should never get here. */
		fprintf(pgpout,"\007\nBad or missing CTB_CKE byte.\n");
		goto err1;	/* Abandon ship! */
	}

	flen = getpastlength(ctbCKE, f); /* read packet length */

	/* Decrypt ciphertext file */
#ifdef XLOWFIRST	/* Assumes LSB-first order */
	status = idea_file( outbuf, DECRYPT_IT, f, g, flen );
#else		/* Assumes MSB-first order */
	status = idea_file( outbuf+1, DECRYPT_IT, f, g, flen );
#endif
	if (status < 0)
	{	fprintf(pgpout,PSTR("\n\007Error: Decrypted plaintext is corrupted.\n"));
	}
	if (!filter_mode)
		fputc('.',pgpout);	/* show progress */

	if (write_error(g))
	{	fclose(g);
		goto err1;
	}
	fclose(g);
	fclose(f);
	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	mp_burn(d);	/* burn sensitive data on stack */
	mp_burn(p);	/* burn sensitive data on stack */
	mp_burn(q);	/* burn sensitive data on stack */
	mp_burn(u);	/* burn sensitive data on stack */
	if (status < 0)	/* if idea_file failed, then error return */
		return(status);
	return(1);	/* always indicate output file has nested stuff in it. */

err1:
	fclose(f);
	burn(inbuf);	/* burn sensitive data on stack */
	burn(outbuf);	/* burn sensitive data on stack */
	mp_burn(d);	/* burn sensitive data on stack */
	mp_burn(p);	/* burn sensitive data on stack */
	mp_burn(q);	/* burn sensitive data on stack */
	mp_burn(u);	/* burn sensitive data on stack */
	return(-1);	/* error return */

}	/* decryptfile */



int idea_decryptfile(char *infile, char *outfile)
{
	byte ctb;	/* Cipher Type Byte */
	FILE *f;
	FILE *g;
	byte ideakey[16];
	byte passphrase[256];
	extern char password[];
	int status;
	word32 flen;

	if (verbose)
		fprintf(pgpout,"idea_decryptfile: infile = %s, outfile = %s\n",
		infile,outfile);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open ciphertext file '%s'\n"),infile);
		return(-1);
	}

	fread(&ctb,1,1,f);	/* read Cipher Type Byte, should be CTB_CKE */

	if (!is_ctb(ctb) || !is_ctb_type(ctb,CTB_CKE_TYPE))
	{	/* Should never get here. */
		fprintf(pgpout,"\007\nBad or missing CTB_CKE byte.\n");
		goto err1;	/* Abandon ship! */
	}

	flen = getpastlength(ctb, f); /* read packet length */

	/* 	open file g for write, in binary (not text) mode... */
	if ((g = fopen( outfile, FOPWBIN )) == NULL)
	{	fprintf(pgpout, PSTR("\n\007Can't create plaintext file '%s'\n"), outfile );
		goto err1;
	}

	/* Get IDEA password, hashed */
	if (*password != '\0')
		hashpass(password, strlen(password), ideakey);
	else {
		fprintf(pgpout,PSTR("\nYou need a pass phrase to decrypt this file. "));
		if (GetHashedPassPhrase((char *)passphrase,(char *)ideakey,NOECHO1) <= 0)
		{	fclose(f);
			fclose(g);
			return(-1);
		}
	}

	if (!filter_mode)
	{	fprintf(pgpout,PSTR("Just a moment..."));  /* this may take a while */
		fflush(pgpout);
	}

	status = idea_file( ideakey, DECRYPT_IT, f, g, flen );

	burn(ideakey);	/* burn sensitive data on stack */
	burn(passphrase);

	if (!filter_mode)
		fputc('.',pgpout);	/* show progress */

	if (write_error(g))
	{	fclose(g);
		goto err1;
	}
	fclose(g);
	fclose(f);

	if (status < 0)	/* if idea_file failed, then complain */
	{	fprintf(pgpout,PSTR("\n\007Error:  Bad pass phrase.\n"));
		remove(outfile);	/* throw away our mistake */
		return(status);		/* error return */
	}
	if (!filter_mode)
		fprintf(pgpout,PSTR("Pass phrase appears good. "));
	return(1);	/* always indicate output file has nested stuff in it. */

err1:
	fclose(f);
	return(-1);	/* error return */

}	/* idea_decryptfile */



int decompress_file(char *infile, char *outfile)
{
	byte ctb;
	FILE *f;
	FILE *g;
	extern void lzhDecode( FILE *, FILE * );
	extern void unzip( FILE *, FILE * );
	if (verbose) fprintf(pgpout, PSTR("Decompressing plaintext...") );

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(infile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open compressed file '%s'\n"),infile);
		return(-1);
	}

	fread(&ctb,1,1,f);	/* read and skip over Cipher Type Byte */
	if (!is_ctb_type( ctb, CTB_COMPRESSED_TYPE ))
	{	/* Shouldn't get here, or why were we called to begin with? */
		fprintf(pgpout,"\007\nBad or missing CTB_COMPRESSED byte.\n");
		goto err1;	/* Abandon ship! */
	}

	getpastlength(ctb, f); /* read packet length */
	/* The packet length is ignored.  Assume it's huge. */

	fread(&ctb,1,1,f);	/* read and skip over compression algorithm byte */
	if (ctb != ZIP2_ALGORITHM_BYTE)
	{	/* We only know how to uncompress deflate-compressed data.  We
		   may hit imploded or Lharc'ed data but treat it as an error just
		   the same */
		fprintf(pgpout,PSTR("\007\nUnrecognized compression algorithm.\n\
This may require a newer version of PGP.\n"));
		goto err1;	/* Abandon ship! */
	}

	/* 	open file g for write, in binary (not text) mode... */
	if ((g = fopen( outfile, FOPWBIN )) == NULL)
	{	fprintf(pgpout, PSTR("\n\007Can't create decompressed file '%s'\n"), outfile );
		goto err1;
	}

	unzip( f, g );
	if (verbose)
		fprintf(pgpout, PSTR("done.  ") );
	else if (!filter_mode)
		fputc('.',pgpout);	/* show progress */

	if (write_error(g))
	{	fclose(g);
		goto err1;
	}
	fclose(g);
	fclose(f);
	return(1);	/* always indicate output file has nested stuff in it. */
err1:
	fclose(f);
	return(-1);	/* error return */

}	/* decompress_file */

