/*	armor.c  - ASCII/binary encoding/decoding based partly on PEM RFC1113.
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
*/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "mpilib.h"
#include "fileio.h"
#include "mpiio.h"
#include "language.h"
#include "pgp.h"
#include "crypto.h"
#include "armor.h"

static int dpem_file(char *infile, char *outfile);
static crcword crchware(byte ch, crcword poly, crcword accum);
static int pem_file(char *infilename, char *outfilename, char *clearfilename);
static int pemdecode(FILE *in, FILE *out);
static void mk_crctbl(crcword poly);
static boolean is_pemfile(char *infile);

/* 	Begin PEM routines.
	This converts a binary file into printable ASCII characters, in a
	radix-64 form mostly compatible with the PEM RFC1113 format.
	This makes it easier to send encrypted files over a 7-bit channel.
*/

/* Index this array by a 6 bit value to get the character corresponding
 * to that value.
 */
static
unsigned char bintoasc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\
abcdefghijklmnopqrstuvwxyz0123456789+/";

/* Index this array by a 7 bit value to get the 6-bit binary field
 * corresponding to that value.  Any illegal characters return high bit set.
 */
static
unsigned char asctobin[] = {
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0076,0200,0200,0200,0077,
	0064,0065,0066,0067,0070,0071,0072,0073,
	0074,0075,0200,0200,0200,0200,0200,0200,
	0200,0000,0001,0002,0003,0004,0005,0006,
	0007,0010,0011,0012,0013,0014,0015,0016,
	0017,0020,0021,0022,0023,0024,0025,0026,
	0027,0030,0031,0200,0200,0200,0200,0200,
	0200,0032,0033,0034,0035,0036,0037,0040,
	0041,0042,0043,0044,0045,0046,0047,0050,
	0051,0052,0053,0054,0055,0056,0057,0060,
	0061,0062,0063,0200,0200,0200,0200,0200
};
static long	infile_line;		/* Current line number for mult decodes */

/************************************************************************/

/* CRC Routines. */
/*	These CRC functions are derived from code in chapter 19 of the book 
	"C Programmer's Guide to Serial Communications", by Joe Campbell.
	Generalized to any CRC width by Philip Zimmermann.
*/

#define byte unsigned char

#define CRCBITS 24	/* may be 16, 24, or 32 */
/* #define maskcrc(crc) ((crcword)(crc)) */	/* if CRCBITS is 16 or 32 */
#define maskcrc(crc) ((crc) & 0xffffffL)	/* if CRCBITS is 24 */
#define CRCHIBIT ((crcword) (1L<<(CRCBITS-1))) /* 0x8000 if CRCBITS is 16 */
#define CRCSHIFTS (CRCBITS-8)

/*	Notes on making a good 24-bit CRC--
	The primitive irreducible polynomial of degree 23 over GF(2),
	040435651 (octal), comes from Appendix C of "Error Correcting Codes,
	2nd edition" by Peterson and Weldon, page 490.  This polynomial was
	chosen for its uniform density of ones and zeros, which has better
	error detection properties than polynomials with a minimal number of
	nonzero terms.  Multiplying this primitive degree-23 polynomial by
	the polynomial x+1 yields the additional property of detecting any
	odd number of bits in error, which means it adds parity.  This 
	approach was recommended by Neal Glover.

	To multiply the polynomial 040435651 by x+1, shift it left 1 bit and
	bitwise add (xor) the unshifted version back in.  Dropping the unused 
	upper bit (bit 24) produces a CRC-24 generator bitmask of 041446373 
	octal, or 0x864cfb hex.  

	You can detect spurious leading zeros or framing errors in the 
	message by initializing the CRC accumulator to some agreed-upon 
	nonzero "random-like" value, but this is a bit nonstandard.  
*/

#define CCITTCRC 0x1021 /* CCITT's 16-bit CRC generator polynomial */
#define PRZCRC 0x864cfbL /* PRZ's 24-bit CRC generator polynomial */
#define CRCINIT 0xB704CEL	/* Init value for CRC accumulator */

static
crcword crctable[256];		/* Table for speeding up CRC's */

/*	mk_crctbl derives a CRC lookup table from the CRC polynomial.
	The table is used later by the crcbytes function given below.
	mk_crctbl only needs to be called once at the dawn of time.

	The theory behind mk_crctbl is that table[i] is initialized
	with the CRC of i, and this is related to the CRC of i>>1,
	so the CRC of i>>1 (pointed to by p) can be used to derive
	the CRC of i (pointed to by q).
*/
static
void mk_crctbl(crcword poly)
{	int i;
	crcword t, *p, *q;
	p = q = crctable;
	*q++ = 0;
	*q++ = poly;
	for (i = 1; i < 128; i++)
	{	t = *++p;
		if (t & CRCHIBIT)
		{	t <<= 1;
			*q++ = t ^ poly;
			*q++ = t;
		}
		else
		{	t <<= 1;
			*q++ = t;
			*q++ = t ^ poly;
		}
	}
}

/*
 * Accumulate a buffer's worth of bytes into a CRC accumulator,
 * returning the new CRC value.
 */
crcword
crcbytes(byte *buf, unsigned len, register crcword accum)
{
	do {
		accum = accum<<8 ^ crctable[(byte)(accum>>CRCSHIFTS) ^ *buf++];
	} while (--len);
	return maskcrc(accum);
} /* crcbytes */

/* Initialize the CRC table using our codes */
void init_crc(void)
{	mk_crctbl(PRZCRC);
}


/************************************************************************/


/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) ((int)bintoasc[((c) & 077)])
#define PAD		'='

/*
 * output one group of up to 3 bytes, pointed at by p, on file f.
 * if fewer than 3 are present, the 1 or two extras must be zeros.
 */
static void outdec(char *p, FILE *f, int count)
{
	int c1, c2, c3, c4;

	c1 = *p >> 2;
	c2 = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
	c3 = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
	c4 = p[2] & 077;
	putc(ENC(c1), f);
	putc(ENC(c2), f);
	if (count == 1)
	{	putc(PAD, f);
		putc(PAD, f);
	}
	else
	{	putc(ENC(c3), f);
		if (count == 2)
			putc(PAD, f);
		else
			putc(ENC(c4), f);
	}
}	/* outdec */


/* Output the CRC value, MSB first per normal CRC conventions */
static void outcrc (word32 crc, FILE *outFile)
{	/* Output crc */
	char crcbuf[4];
	crcbuf[0] = (crc>>16) & 0xff;
	crcbuf[1] = (crc>>8) & 0xff;
	crcbuf[2] = (crc>>0) & 0xff;
	putc(PAD,outFile);
	outdec (crcbuf,outFile,3);
	putc('\n',outFile);
}	/* outcrc */

/* Return filename for output (text mode), but replace last letter(s) of
 * filename with the ascii for num.  It will use the appropriate number
 * of digits for ofnum when converting num, so if ofnum < 10, use 1 digit,
 * >= 10 and < 100 use 2 digits, >= 100 and < 1000 use 3 digits.  If its
 * >= 1000, then we have other problems to worry about, and this might do
 * weird things.
 */
static char *numFilename( char *fname, int num, int ofnum)
{	static char	fnamenum[MAX_PATH];
	int		len;
	int		offset = 1;

	strcpy (fnamenum, fname);
	len = strlen (fnamenum);
	do {
	  fnamenum[len-offset] = '0' + (num%10);
	  num /= 10;
	  ofnum /= 10;
	  offset++;
	} while (ofnum >= 1 && offset < 4);
	return(fnamenum);
}

/*
 * Reads and discards a line from the given file.  Returns -1 on error or
 * EOF, 0 if the line is blank, and 1 if the line is not blank.
 */
static int
skipline(FILE *f)
{
	int state, flag, c;

	state = 0;
	flag = 0;
	for (;;) {
		c = getc(f);
		if (c == '\n')
			return flag;
		if (state)
		{	ungetc(c, f);
			return flag;
		}
		if (c == EOF)
			return -1;
		if (c == '\r')
			state = 1;
		else if (c != ' ')
			flag = 1;
	}
} /* skipline */

/*
 * Copies a line from the input file to the output.  Does NOT copy the
 * trailing newline.  Returns -1 on EOF or error, 0 if the line was terminated
 * by EOF, and 1 if the line was terminated with a newline sequence.
 */
static int
copyline(FILE *in, FILE *out)
{
	int state, flag, c;

	state = 0;
	for (;;) {
		c = getc(in);
		if (c == '\n')
			return 1;
		if (state)
		{	ungetc(c, in);
			return 1;
		}
		if (c == EOF)
			return 0;
		if (c == '\r')
			state = 1;
		else
			putc(c, out);
	}
} /* copyline */

/*
 * Reads a line from file f, up to the size of the buffer.  The line in the
 * buffer will NOT include line termination, although any of (CR, LF, CRLF)
 * is accepted on input.  The return value is -1 on error, 0 if the line
 * was terminated abnormally (EOF, error, or out of buffer space), and
 * 1 if the line was terminated normally.
 *
 * Passing in a buffer less than 2 characters long is not a terribly bright
 * idea.
 */
static int
getline(char *buf, int n, FILE *f)
{
	int state;
	char *p;
	int c;

	state = 0;
	p = buf;
	for (;;)
	{	c = getc(f);
		if (c == '\n')
		{ 	*p = 0;
			return 1;	/* Line terminated with \n or \r\n */
		}
		if (state)
		{	ungetc(c, f);
			*p = 0;
			return 1;	/* Line terminated with \r */
		}
		if (c == EOF)
		{	*p = 0;
			return (p == buf) ? -1 : 0;	/* Error */
		}
		if (c == '\r')
			state = 1;
		else if (--n > 0)
			*p++ = c;
		else
		{
			ungetc(c, f);
			*p = 0;
			return 0;	/* Out of buffer space */
		}
	} /* for (;;) */
} /* getline */

/*
 * Read a line from file f, buf must be able to hold at least 80 characters.
 * Strips trailing spaces and line terminator, can read LF, CRLF and CR
 * textfiles.  Anything after 80 characters is ignored.  It can't be ASCII
 * armor anyway.
 */
static char *
get_armor_line(char *buf, FILE *f)
{
	int c, n = 79;
	char *p = buf;

	do {
		c = getc(f);
		if (c == '\n' || c == '\r' || c == EOF)
			break;
		*p++ = c;
	} while (--n > 0);
	if (p == buf && c == EOF)
	{	*buf = '\0';
		return NULL;
	}
	/* skip to end of line */
	while (c != '\n' && c != '\r' && c != EOF)
		c = getc(f);
	if (c == '\r' && (c = getc(f)) != '\n')
		ungetc(c, f);
	while (--p >= buf && *p == ' ')
		;
	*++p = '\0';
	return buf;
}


/*
 * Encode a file in sections.  64 ASCII bytes * 720 lines = 46K, 
 * recommended max.  Usenet message size is 50K so this leaves a nice 
 * margin for .signature.  In the interests of orthogonality and 
 * programmer laziness no check is made for a message containing only 
 * a few lines (or even just an 'end')	after a section break. 
 */
#define LINE_LEN	48L
int pem_lines = 720;
#define BYTES_PER_SECTION	(LINE_LEN * pem_lines)

#if 1
/* This limit is advisory only; longer lines are handled properly.
 * The only requirement is that this be at least as long as the longest
 * delimiter string used by PGP
 * (e.g. "-----BEGIN PGP MESSAGE, PART %02d/%02d-----\n")
 */
#define MAX_LINE_SIZE 80
#else
#ifdef MSDOS	/* limited stack space */
#define MAX_LINE_SIZE	256
#else
#define MAX_LINE_SIZE	1024
#endif
#endif

#ifndef VMS
/* armored files are opened in binary mode so that CRLF/LF/CR files
   can be handled by all systems */
#define	FOPRPEM	FOPRBIN
#else
#define	FOPRPEM	FOPRTXT
#endif

extern boolean verbose;	/* Undocumented command mode in PGP.C */
extern boolean filter_mode;

/*
 * Copy from infilename to outfilename, PEM encoding as you go along,
 * and with breaks every
 * pem_lines lines.
 * If clearfilename is non-NULL, first output that file preceded by a
 * special header line.
 */
static
int pem_file(char *infilename, char *outfilename, char *clearfilename)
{
	char buffer[MAX_LINE_SIZE];
	int i, rc, bytesRead, lines = 0;
	int noSections, currentSection = 1;
	long fileLen;
	crcword crc;
	FILE *inFile, *outFile, *clearFile;
	char *blocktype = "MESSAGE";

	/* open input file as binary */
	if ((inFile = fopen(infilename,FOPRBIN)) == NULL)
	{   
		return(1);
	}

	if (!outfilename || pem_lines == 0)
		noSections = 1;
	else
	{	/* Evaluate how many parts this file will comprise */
		fseek(inFile,0L,SEEK_END);
		fileLen = ftell(inFile);
		rewind(inFile);
		noSections = (fileLen + BYTES_PER_SECTION - 1) / BYTES_PER_SECTION;
		if (noSections > 99)
		{
			pem_lines = ((fileLen+LINE_LEN-1)/LINE_LEN + 98) / 99;
			noSections = (fileLen + BYTES_PER_SECTION - 1) / BYTES_PER_SECTION;
			fprintf(pgpout, "value for \"armorlines\" is too low, using %d\n", pem_lines);
		}
	}
	
	if (outfilename == NULL)
		outFile = stdout;
	else
	{	if (noSections > 1)
		{	force_extension(outfilename, ASC_EXTENSION);
			outFile = fopen (numFilename (outfilename, 1, noSections), FOPWTXT);
		}
		else
			outFile = fopen(outfilename,FOPWTXT);
	}

	if (outFile == NULL)
	{	fclose(inFile);
		return(1);
	}

	if (clearfilename)
	{	if ((clearFile = fopen(clearfilename,FOPRTXT)) == NULL)
		{	fclose (inFile);
			if (outFile != stdout)
				fclose (outFile);
			return(1);
		}
		fprintf (outFile, "-----BEGIN PGP SIGNED MESSAGE-----\n\n");
		while ((i = getline(buffer, sizeof buffer, clearFile)) >= 0)
		{
			/* Quote lines beginning with '-' as per RFC1113;
			 * Also quote lines beginning with "From "; this is
			 * for Unix mailers which add ">" to such lines.
			 */
			if (buffer[0] == '-' || strncmp(buffer, "From ", 5)==0)
				fputs("- ", outFile);
			fputs(buffer, outFile);
			/* If there is more on this line, copy it */
			if (i == 0)
				if (copyline(clearFile, outFile) <= 0)
					break;
			fputc('\n', outFile);
		}
		fclose (clearFile);
		putc('\n', outFile);
		blocktype = "SIGNATURE";
	}


	if (noSections == 1)
	{
		byte ctb = 0;
		ctb = getc(inFile);
		if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE))
			blocktype = "PUBLIC KEY BLOCK";
		fprintf (outFile, "-----BEGIN PGP %s-----\n",blocktype);
		rewind(inFile);
	}
	else
		fprintf (outFile, "-----BEGIN PGP MESSAGE, PART %02d/%02d-----\n",
					1, noSections);
	fprintf (outFile, "Version: %s\n",rel_version);
	fprintf (outFile, "\n");

	init_crc();
	crc = CRCINIT;

	while((bytesRead = fread(buffer,1,LINE_LEN,inFile)) > 0)
	{	/* Munge up LINE_LEN characters */
		if (bytesRead < LINE_LEN)
			fill0 (buffer+bytesRead, LINE_LEN-bytesRead);

		crc = crcbytes((byte *)buffer, bytesRead, crc);
		for (i=0; i<bytesRead-3; i+=3)
			outdec(buffer+i, outFile, 3);
		outdec(buffer+i, outFile, bytesRead-i);
		putc('\n',outFile);

		if (++lines == pem_lines && currentSection < noSections)
		{	lines = 0;
			outcrc (crc, outFile);
			fprintf(outFile,"-----END PGP MESSAGE, PART %02d/%02d-----\n\n",
				currentSection, noSections);
			if (write_error(outFile))
			{	fclose(outFile);
				return(-1);
			}
			fclose (outFile);
			outFile = fopen (numFilename (outfilename, ++currentSection, noSections), FOPWTXT);
			if (outFile == NULL)
			{	fclose(inFile);
				return(-1);
			}
			fprintf(outFile,"-----BEGIN PGP MESSAGE, PART %02d/%02d-----\n",
					currentSection, noSections);
			fprintf(outFile,"\n");
			crc = CRCINIT;
		}
	}
	outcrc (crc, outFile);

	if (noSections == 1)
		fprintf (outFile, "-----END PGP %s-----\n",blocktype);
	else
		fprintf(outFile,"-----END PGP MESSAGE, PART %02d/%02d-----\n",
				noSections, noSections);

	/* Done */
	fclose(inFile);
	rc = write_error(outFile);
	if (outFile == stdout)
		return rc;
	fclose(outFile);

	if (rc)
		return(-1);

	if (clearfilename)
		fprintf (pgpout, PSTR("\nClear signature file: %s\n"), outfilename);
	else if (noSections == 1)
		fprintf (pgpout, PSTR("\nTransport armor file: %s\n"), outfilename);
	else
	{
		fprintf (pgpout, PSTR("\nTransport armor files: "));
		for (i=1; i<=noSections; ++i)
			fprintf (pgpout, "%s%s", numFilename(outfilename, i, noSections),
						i==noSections?"\n":", ");
	}
	return(0);
}	/* pem_file */

/* 	End PEM encode routines. */


/*	PEM decode routines.
*/

static
int dpem_buffer(char *inbuf, char *outbuf, int *outlength)
{
	unsigned char *bp;
	int	length;
	unsigned int c1,c2,c3,c4;
	register int j;

	length = 0;
	bp = (unsigned char *)inbuf;

	/* FOUR input characters go into each THREE output charcters */

	while (*bp!='\0')
	{	if (*bp&0x80 || (c1=asctobin[*bp])&0x80)
			return -1;
		++bp;
		if (*bp&0x80 || (c2=asctobin[*bp])&0x80)
			return -1;
		if (*++bp == PAD)
		{	c3 = c4 = 0;
			length += 1;
			if (c2 & 15)
				return -1;
			if (strcmp((char *)bp, "==") == 0)
				bp += 1;
			else if (strcmp((char *)bp, "=3D=3D") == 0)
				bp += 5;
			else
				return -1;
		}
		else if (*bp&0x80 || (c3=asctobin[*bp])&0x80)
				return -1;
		else
		{	if (*++bp == PAD)
			{	c4 = 0;
				length += 2;
				if (c3 & 3)
					return -1;
				if (strcmp((char *)bp, "=") == 0)
					; /* All is well */
				else if (strcmp((char *)bp, "=3D") == 0)
					bp += 2;
				else
					return -1;
			}
			else if (*bp&0x80 || (c4=asctobin[*bp])&0x80)
				return -1;
			else
				length += 3;
		}
		++bp;
		j = (c1 << 2) | (c2 >> 4);
		*outbuf++=j;
		j = (c2 << 4) | (c3 >> 2);
		*outbuf++=j;
		j = (c3 << 6) | c4;
		*outbuf++=j;
	}

	*outlength = length;
	return(0);	/* normal return */

}	/* dpem_buffer */

static char pemfilename[MAX_PATH];
/*
 * try to open the next file of a multi-part armored file
 * the sequence number is expected at the end of the file name
 */
static FILE *
open_next(void)
{
	char *p, *s, c;
	FILE *fp;

	p = pemfilename + strlen(pemfilename);
	while (--p >= pemfilename && isdigit(*p))
	{
		if (*p != '9')
		{
			++*p;
			return(fopen(pemfilename, FOPRPEM));
		}
		*p = '0';
	}

	/* need an extra digit */
	if (p >= pemfilename)
	{	/* try replacing character ( .as0 -> .a10 ) */
		c = *p;
		*p = '1';
		if ((fp = fopen(pemfilename, FOPRPEM)) != NULL)
			return(fp);
		*p = c;	/* restore original character */
	}
	++p;
	for (s = p + strlen(p); s >= p; --s)
		s[1] = *s;
	*p = '1'; /* insert digit ( fn0 -> fn10 ) */

	return(fopen(pemfilename, FOPRPEM));
}

/*
 * Copy from in to out, decoding as you go, with handling for multiple
 * 500-line blocks of encoded data.
 */
static
int pemdecode(FILE *in, FILE *out)
{
char inbuf[80];
char outbuf[80];

int i, n, status;
int line;
int section, currentSection = 1;
int noSections = 0;
int gotcrc = 0;
long crc=CRCINIT, chkcrc = -1;
char crcbuf[4];
int ret_code = 0;
int end_of_message;

	init_crc();

	for (line = 1; ; line++)	/* for each input line */
	{
		if (get_armor_line(inbuf, in) == NULL)
			end_of_message = 1;
		else
		{	end_of_message = (strncmp(inbuf,"-----END PGP MESSAGE,", 21) == 0);
			++infile_line;
		}

		if (currentSection!=noSections && end_of_message)
		{	/* End of this section */
			if (gotcrc)
			{	if (chkcrc != crc)
				{	fprintf(pgpout,PSTR("ERROR: Bad ASCII armor checksum in section %d.\n"), currentSection);
					ret_code = -1;	/* continue with decoding to see if there are other bad parts */
				}
			}
			gotcrc = 0;
			crc = CRCINIT;
			section = 0;

			/* Try and find start of next section */
			do
			{	if (get_armor_line(inbuf,in) == NULL)
				{	FILE *nextf;
					if ((nextf = open_next()) != NULL)
					{
						fclose(in);
						in = nextf;
						continue;
					}
					fprintf(pgpout,PSTR("Can't find section %d.\n"),currentSection + 1);
					return(-1);
				}
				++infile_line;
			}
			while (strncmp(inbuf,"-----BEGIN PGP MESSAGE",22));

			/* Make sure this section is the correct one */
			if (2 != sscanf(inbuf,"-----BEGIN PGP MESSAGE, PART %d/%d",
				&section,&noSections))
			{	fprintf(pgpout,PSTR("Badly formed section header, part %d.\n"),
					currentSection+1);
				return(-1);
			}
			if (section != ++currentSection)
			{	fprintf(pgpout,PSTR("Sections out of order, expected part %d"),currentSection);
				if (section)
					fprintf(pgpout,PSTR(", got part %d\n"),section);
				else
					fputc('\n',pgpout);
				return(-1);
			}

			/* Skip header after BEGIN line */
			do {
				++infile_line;
				if (get_armor_line(inbuf, in) == NULL)
				{
					fprintf(pgpout,PSTR("ERROR: Hit EOF in header of section %d.\n"),
						currentSection);
					return(-1);
				}
			} while (inbuf[0] != '\0');

			/* Continue decoding */
			continue;
		}

		/* Quit when hit the -----END PGP MESSAGE----- line or a blank,
			or handle checksum */
		if (inbuf[0] == PAD)	/* Checksum lines start with PAD char */
		{
			/* If the already-armored file is sent through MIME
			 * and gets armored again, '=' will become '=3D'.
			 * To make life easier, we detect and work around this
			 * idiosyncracy.
			 */
			if (strlen(inbuf) == 7 &&
			    inbuf[1] == '3' && inbuf[2] == 'D')
				status = dpem_buffer(inbuf+3, crcbuf, &n);
			else
				status = dpem_buffer(inbuf+1, crcbuf, &n);
			if ( status < 0 || n != 3 )
			{	fprintf(pgpout,PSTR("ERROR: Badly formed ASCII armor checksum, line %d.\n"),line);
				return -1;
			}
			chkcrc = (((long)crcbuf[0]<<16)&0xff0000L) +
				((crcbuf[1]<<8)&0xff00L) + (crcbuf[2]&0xffL);
			gotcrc = 1;
			continue;
		}
		if (inbuf[0] == '\0')
		{	fprintf(pgpout,PSTR("WARNING: No ASCII armor `END' line.\n"));
			break;
		}
		if (strncmp(inbuf, "-----END PGP ", 13) == 0)
			break;

		status = dpem_buffer(inbuf,outbuf,&n);

		if (status == -1)
		{	fprintf(pgpout,PSTR("ERROR: Bad ASCII armor character, line %d.\n"), line);
			gotcrc = 1;	/* this will print part number, continue with next part */
			ret_code = -1;
		}

		if (n > sizeof outbuf)
		{	fprintf(pgpout,PSTR("ERROR: Bad ASCII armor line length %d on line %d.\n"),
					n, line);
			return -1;
		}

		crc = crcbytes((byte *)outbuf, n, crc);
		if (fwrite(outbuf,1,n,out) != n)
		{	ret_code = -1;
			break;
		}

	}	/* line */

	if (gotcrc)
	{	if (chkcrc != crc)
		{	fprintf(pgpout,PSTR("ERROR: Bad ASCII armor checksum"));
			if (noSections > 0)
				fprintf(pgpout,PSTR(" in section %d"), noSections);
			fputc('\n',pgpout);
			return -1;
		}
	}
	else
		fprintf(pgpout,PSTR("Warning: Transport armor lacks a checksum.\n"));

	return(ret_code);	/* normal return */
}   /* pemdecode */


static
boolean is_pemfile(char *infile)
{
	FILE	*in;
	char	inbuf[80];
	char	outbuf[80];
	int 	n, status;
	long	il;

	if ((in = fopen(infile, FOPRPEM)) == NULL)
	{	/* can't open file */
		return(FALSE);
	}

	/* Read to infile_line before we begin looking */
	for (il=0; il<infile_line; ++il)
	{
		if (get_armor_line(inbuf, in) == NULL)
		{	fclose(in);
			return(FALSE);
		}
	}

	/* search file for header line */
	for (;;)
	{
		if (get_armor_line(inbuf, in) == NULL)
			break;
		else
		{
			if (strncmp(inbuf, "-----BEGIN PGP ", 15) == 0)
			{
				if (strncmp(inbuf,"-----BEGIN PGP SIGNED MESSAGE-----",34)==0) {
					fclose(in);
					return(TRUE);
				}
				do {
					if (get_armor_line(inbuf, in) == NULL)
						break;
#ifndef STRICT_PEM
					if (dpem_buffer(inbuf,outbuf,&n) == 0 && n == 48)
					{	fclose(in);
						return(TRUE);
					}
#endif
				} while (inbuf[0] != '\0');
				if (get_armor_line(inbuf, in) == NULL)
					break;
				status = dpem_buffer(inbuf,outbuf,&n);
				if (status < 0)
					break;
				fclose(in);
				return(TRUE);
			}
		}
	}

	fclose(in);
	return(FALSE);

}	/* is_pemfile */


static
int dpem_file(char *infile, char *outfile)
{
FILE	*in, *out;
char	buf[MAX_LINE_SIZE];
char	outbuf[80];
int	status, n;
long	il, fpos;
char	*litfile = NULL;

	if ((in = fopen(infile, FOPRPEM)) == NULL)
	{
		fprintf(pgpout,PSTR("ERROR: Can't find file %s\n"), infile);
		return(10);
	}
	strcpy(pemfilename, infile);	/* store filename for multi-parts */

	/* Skip to infile_line */
	for (il=0; il<infile_line; ++il)
	{
		if (get_armor_line(buf, in) == NULL)
		{	fclose(in);
			return -1;
		}
	}

	/* Loop through file, searching for header.  Decode anything with a
	   header, complain if there were no headers. */

	/* search file for header line */
    for (;;)
	{
		++infile_line;
		if (get_armor_line(buf, in) == NULL)
		{
			fprintf(pgpout,PSTR("ERROR: No ASCII armor `BEGIN' line!\n"));
			fclose(in);
			return(12);
		}
		if (strncmp(buf, "-----BEGIN PGP ", 15) == 0)
			break;
	}
	if (strncmp(buf, "-----BEGIN PGP SIGNED MESSAGE-----", 34) == 0) {
		FILE	*litout;
		char	*canonfile, *p;
		int	nline;

		litfile = tempfile(TMP_WIPE|TMP_TMPDIR);
		if ((litout = fopen (litfile, FOPWTXT)) == NULL)
		{	fprintf(pgpout,PSTR("\n\007Unable to write ciphertext output file '%s'.\n"), litfile);
			fclose(in);
			return(-1);
		}
		/* Skip header lines until a blank is hit */
		do
		{	++infile_line;
			status = skipline(in);
		} while (status != 0);
		/* Ignore error; getline will discover it again */
		status = 0;
		for (;;)
		{	++infile_line;
			nline = status;
			status = getline(buf, sizeof buf, in);
			if (status < 0)
			{	fprintf(pgpout,PSTR("ERROR: ASCII armor decode input ended unexpectedly!\n"));
				fclose(in);
				fclose(litout);
				rmtemp (litfile);
				return(12);
			}

			if (strncmp(buf,"-----BEGIN PGP ", 15) == 0)
				break;
			if (nline)
				putc('\n', litout);
			/* De-quote lines starting with '- ' */
			fputs(buf + ((buf[0]=='-'&&buf[1]==' ')?2:0), litout);
			/* Copy trailing part of line, if any. */
			if (!status)
				status = copyline(in, litout);
			/* Ignore error; getline will discover it again */
		}
		fflush (litout);
		if (ferror(litout))
		{	fclose(litout);
			fclose(in);
			rmtemp (litfile);
			return(-1);
		}
		fclose (litout);
		canonfile = tempfile(TMP_WIPE|TMP_TMPDIR);
		strip_spaces = TRUE;
		make_canonical (litfile, canonfile);
		rmtemp (litfile);
		litfile = canonfile;
	}

	/* Skip header after BEGIN line */
	do {
		++infile_line;
		fpos = ftell(in);
		if (get_armor_line(buf, in) == NULL)
		{
			fprintf(pgpout,PSTR("ERROR: Hit EOF in header.\n"));
			fclose(in);
			return(13);
		}
#ifndef STRICT_PEM
		if (dpem_buffer(buf,outbuf,&n) == 0 && n == 48)
		{	fseek(in, fpos, SEEK_SET);
			--infile_line;
			break;
		}
#endif
	} while (buf[0] != '\0');

	if ((out = fopen(outfile, FOPWBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Unable to write ciphertext output file '%s'.\n"), outfile);
		fclose(in);
		return(-1);
	}

	status = pemdecode(in, out);

	if (litfile)
	{	/* Glue the literal file read above to the signature */
		char lit_mode=MODE_TEXT;	
		word32 dummystamp = 0;
		FILE *f = fopen(litfile,FOPRBIN);
		write_ctb_len (out, CTB_LITERAL2_TYPE, fsize(f)+6, FALSE);
		fwrite ( &lit_mode, 1, 1, out );	/*	write lit_mode */
		fputc ('\0', out);			/* No filename */
		fwrite ( &dummystamp, 1, sizeof(dummystamp), out); /* dummy timestamp */
		copyfile(f,out,-1L);		/* Append literal file */
		fclose (f);
		rmtemp(litfile);
	}

	if (write_error(out))
		status = -1;
	fclose(out);
	fclose(in);
	return(status);
}   /* dpem_file */

/* Entry points for generic interface names */
int
armor_file (char *infile, char *outfile, char *filename, char *clearname)
{
	if (verbose)
		fprintf(pgpout,"armor_file: infile = %s, outfile = %s, filename = %s, clearname = %s\n",
			infile, outfile, filename, clearname);
	return pem_file (infile, outfile, clearname);
}

int
de_armor_file(char *infile, char *outfile, long *curline)
{
	int status;

	if (verbose)
		fprintf(pgpout,"de_armor_file: infile = %s, outfile = %s, curline = %ld\n",
			infile, outfile, *curline);
	infile_line = (curline ? *curline : 0);
	status = dpem_file (infile, outfile);
	if (curline)
		*curline = infile_line;
	return status;
}

boolean
is_armor_file (char *infile, long startline)
{
	infile_line = startline;
	return is_pemfile (infile);
}
