/*
 * Figure out the type of a file.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpFileType.c,v 1.6 1997/06/25 19:56:35 lloyd Exp $
 */

#include "pgpConfig.h"

#include <string.h>	/* For strcmp */
#include "pgpFileType.h"
#include "pgpPktByte.h"

#define FILETYPELEN	8	/* # of characters needed */

typedef struct PGPFileTypeStruct	PGPFileTypeStruct;

typedef struct FileTypeInternal {
	unsigned char const pattern[FILETYPELEN]; /* First char is length */
	PGPFileTypeStruct	 type;
} FileTypeInternal;

/* A character that does not appear in any pattern */
#define ANY	(unsigned char)255

static FileTypeInternal patterns[] =
{
	{ { 2,  26,  11 },                          { "PAK",     ".pak", 0 } },
	{ { 2,  31, 139 },                          { "gzip",     ".gz", 0 } },
	{ { 2,  31, 157 },                          { "compressed",".Z", 0 } },
	{ { 2, 234,  96 },                          { "Arj",     ".arj", 0 } },
		/* ^Z H P % */
	{ { 4,  26,  72,  80,  37 },                { "Hyper",   ".hyp", 0 } },
		/* G I F 8 */
	{ { 4,  71,  73,  70,  56 },                { "GIF",     ".gif", 0 } },
		/* HPAK */
	{ { 4,  72,  80,  65,  75 },                { "Hpack",   ".hpk", 0 } },
		/* P K ^C ^D */
	{ { 4,  80,  75,  3,   4  },                { "Zip",     ".zip", 0 } },
		/* Z O O   */
	{ { 4,  90,  79,  79,  32 },                { "Zoo",     ".zoo", 0 } },
		/* ? ? - l h a 0 -   -lha1- -lha2- and -lha3- */
	{ { 7, ANY, ANY,  45, 108, 104,  48,  45 }, { "LHArc",   ".lzh", 0 } },
	{ { 7, ANY, ANY,  45, 108, 104,  49,  45 }, { "LHArc",   ".lzh", 0 } },
	{ { 7, ANY, ANY,  45, 108, 104,  50,  45 }, { "LHArc",   ".lzh", 0 } },
	{ { 7, ANY, ANY,  45, 108, 104,  51,  45 }, { "LHArc",   ".lzh", 0 } },
		/* ? ? - l h a ? - */
	{ { 7, ANY, ANY,  45, 108, 104, ANY,  45 }, { "LHArc",   ".lha", 0 } }
};

static PGPFileTypeStruct sPGPFile = {"PGP", ".pgp", 0};
					/* \250 3 P G P */
static unsigned char const PGPPattern[] = { 5, 168, 3, 80, 71, 80 };

static int
fileTypeMatch (PGPByte const *bytes, PGPByte const *prefix, unsigned len)
{
	unsigned l = *bytes++;
	unsigned i;

	if (l > len)
		return 0;
	for (i = 0; i < l; i++)
		if (bytes[i] != prefix[i] && bytes[i] != ANY)
			return 0;
	return 1;	/* Success */
}

/* Figure out if the file is a recognized type */
	static PGPFileTypeStruct const *
pgpFileType (PGPByte const *prefix, unsigned len)
{
	unsigned i;

	for (i = 0; i < sizeof(patterns)/sizeof(*patterns); i++) {
		if (fileTypeMatch(patterns[i].pattern, prefix, len))
			return &patterns[i].type;
	}

	if (pgpFileTypePGP (prefix, len))
		return &sPGPFile;

	return NULL;
}

/*
 * Figure out if a file is a binary PGP file.
 * Returns 1 if it is a PGP file, 0 if it is not.
 */
int
pgpFileTypePGP (PGPByte const *prefix, unsigned len)
{
	if (!len)
		return 0;

	/* _All_ PGP messages start with a character 10xxxxxx */
	if (!IS_OLD_PKTBYTE (*prefix))
		return 0;

	/* Check for a new-style PGP file, using the PGP file header */
	if (fileTypeMatch (PGPPattern, prefix, len))
		return 1;
		
	/*
	 * Check for old-style PGP file, checking the middle bits for
	 * known packet types.  This needs to match lib/pipe/file/pgpHeader.c
	 */
	switch (OLD_PKTBYTE_TYPE (*prefix)) {
	case PKTBYTE_ESK:
	case PKTBYTE_SIG:
	case PKTBYTE_CONVESK:
	case PKTBYTE_SECKEY:
	case PKTBYTE_PUBKEY:
	case PKTBYTE_SECSUBKEY:
	case PKTBYTE_PUBSUBKEY:
	case PKTBYTE_COMPRESSED:
	case PKTBYTE_CONVENTIONAL:
	case PKTBYTE_LITERAL:
		/* These are the known packet types */
		return 1;
	default:
		/* Its not known -- perhaps something else? */
		return 0;
	}

	/* NOTREACHED */
	return 0;
}

/*
 * Figure out if a file is human-readable or not.
 * Returns 0 if text, 1 if binary.
 *
 * A file is considered binary if it contains any illegal control characters
 * or "too many" characters with the 8th bit set.  If you're using Latin-1,
 * you might have some such characters, but a binary file probably averages
 * about half odd characters.  This test assumes a file is binary if the
 * number of bytes with the high bit set exceeds a quarter of the buffer.
 * EXCEPTION: If working in Russian, Cyrillic is usually placed in the high
 * half, so ignore this test in that case.
 *
 * The legal control characters are:
 *  0  NU   null (nul)                       ILLEGAL
 *  1  SH   start of heading (soh)           ILLEGAL
 *  2  SX   start of text (stx)		     ILLEGAL
 *  3  EX   end of text (etx)                ILLEGAL
 *  4  ET   end of transmission (eot)        ILLEGAL
 *  5  EQ   enquiry (enq)                    ILLEGAL
 *  5  EQ   enquiry (enq)                    ILLEGAL
 *  6  AK   acknowledge (ack)                ILLEGAL
 *  7  BL   bell (bel)                       legal	'\a'
 *  8  BS   backspace (bs)                   legal	'\b'
 *  9  HT   character tabulation (ht)        legal	'\t'
 * 10  LF   line feed (lf)                   legal	'\n'
 * 11  VT   line tabulation (vt)             legal	'\v'
 * 12  FF   form feed (ff)                   legal	'\f'
 * 13  CR   carriage return (cr)             legal	'\r'
 * 14  SO   shift out (so)                   ILLEGAL
 * 15  SI   shift in (si)                    ILLEGAL
 * 16  DL   datalink escape (dle)            ILLEGAL
 * 17  D1   device control one (dc1)         ILLEGAL
 * 18  D2   device control two (dc2)         ILLEGAL
 * 19  D3   device control three (dc3)       ILLEGAL
 * 20  D4   device control four (dc4)        ILLEGAL
 * 21  NK   negative acknowledge (nak)       ILLEGAL
 * 22  SY   syncronous idle (syn)            ILLEGAL
 * 23  EB   end of transmission block (etb)  ILLEGAL
 * 24  CN   cancel (can)                     ILLEGAL
 * 25  EM   end of medium (em)               ILLEGAL
 * 26  SB   substitute (sub)                 legal (CP/M & MS-DOS EOF)
 * 27  EC   escape (esc)                     ILLEGAL
 * 28  FS   file separator (is4)             ILLEGAL
 * 29  GS   group separator (is3)            ILLEGAL
 * 30  RS   record separator (is2)           ILLEGAL
 * 31  US   unit separator (is1)             ILLEGAL
 *
 * TODO: Worry about EBCDIC
 * TODO: Use the charset rather than the language to decide.
 * TODO: Worry about shift-JIS and other heavy top-8-bit uses.
 */
 
 /* this appears to not be called. KEEP (as static) for now */
	static int
pgpFileTypeBinary(char const *lang, PGPByte const *buf, unsigned len)
{
	unsigned highlimit;
	char c;

	if (!len)
		return 1;	/* empty file or error, not a text file */
	if (pgpFileType(buf, len))
		return 1;

	/* Limit on number of 8th-bit-set characters allowed */
	highlimit = len/4;
	if (lang && strcmp(lang, "ru") == 0)
		highlimit = len;

	do {
		c = *buf++;
		if (c < ' ' && (c < '\a' || c > '\r') && c != 26)
			return 1;	/* Illegal control char */
		if ((c & 0x80) && highlimit-- == 0)
			return 1;	/* Too many 8th bits set */
	} while (--len);

	return 0;
}
