/*
 * pgpArmor.c -- a module to perform Ascii Armor
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpArmor.c,v 1.45 1998/07/15 05:35:57 hal Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpArmor.h"
#include "pgpCRC.h"
#include "pgpPktByte.h"
#include "pgpRadix64.h"
#include "pgpAnnotate.h"
#include "pgpFIFO.h"
#include "pgpHashPriv.h"
#include "pgpJoin.h"
#include "pgpEnv.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpPipeline.h"
#include "pgpRandomX9_17.h"
#include "pgpSplit.h"
#include "pgpUsuals.h"
#include "pgpFeatures.h"
#include "pgpContext.h"

#define ARMORMAGIC	0xa4904f11
		
/* Max length of a line in MIME content-printable encoding */
#define MIMEMAX 76

/* Number characters in our random MIME separator */
#define NSEPCHARS	48

/*
 * PGP-MIME headers.  We chose "=-" as boundary since it can't occur in
 * a quoted-printable encoding, nor in base64.
 */
#define MIMEENCSEP "=--"
#define MIMESIGHDR1a "Mime-Version: 1.0\n"
#define MIMESIGHDR1b \
	"Content-Type: multipart/signed;\n boundary=\""
#define MIMESIGHDR2 "\";\n" \
	" protocol=\"application/pgp-signature\"; "
#define MIMESIGHDRLINESa 1
#define MIMESIGHDRLINESb 2
#define MIMEENCHDR1a "Mime-Version: 1.0\n"
#define MIMEENCHDR1b \
	"Content-Type: multipart/encrypted; boundary=\"" MIMEENCSEP "\";\n" \
	" protocol=\"application/pgp-encrypted\"\n\n"
#define MIMEENCHDR2 "--" MIMEENCSEP "\n" \
	"Content-Type: application/pgp-encrypted\n\n" \
	"Version: 1\n\n--" MIMEENCSEP "\n" \
	"Content-Type: application/octet-stream\n\n"
#define MIMEENCHDRLINES1a	1
#define MIMEENCHDRLINES1b	3
#define MIMEMIC "micalg=pgp-"
#define MIMEDFLTMIC "pgp-md5"
#define MIMEMIDBOUND "Content-Type: application/pgp-signature\n\n"

typedef struct ArmorContext
{
	/* by placing this here, we can just allocate everything at once */
	PGPPipeline	pipe;
	
	PGPPipeline *tail;
	PGPFifoDesc const *fd;
	PGPFifoContext *fifo;
	PGPFifoContext *header;
	PGPContextRef cdkContext;
	unsigned long	crc;
	unsigned long	armorlines;
	unsigned long	lineno;
	unsigned	thispart;
	unsigned	maxparts;
	int	scope_depth;
	PGPByte	input[48];	/* Maximum 48 bytes */
#if 0
	char	output[65];	/* Maximum 63 buyes + \r and/or \n */
#else
	char	output[MIMEMAX+2]; /* MIME line plus \n plus null */
#endif
	char	mimesigsep[NSEPCHARS+1];
	char *	outptr;
	PGPBoolean needmessageid;
	char *	messageid;
	char const *	blocktype;
	char const *	comment;
	char const *	charset;
	char const *	versionString;
	PGPSize		mimebodyoff;		/* Offset to start of body (1 char/nl) */
	PGPUInt32	mimeheaderlines;	/* # lines in mime header */
	PgpVersion	version;
	PGPByte	inlen;
	PGPByte	outlen;
	PGPByte	clearsign;
	PGPByte	pgpmime;
#define PGPMIMESIG 1
#define PGPMIMEENC 2
	PGPByte pgpmimeversionline;
	PGPByte	didheader;
	PGPByte	didfooter;
	PGPByte	sizevalid;
	PGPByte	state;
	PGPByte	linebuf;
	DEBUG_STRUCT_CONSTRUCTOR( ArmorContext )
} ArmorContext;


/* Forward declarations */
static int
armorLine (PGPByte const *in, unsigned inlen, char *out);



/*
 * Armor 3 raw bytes into 4
 * If armoring n < 3 bytes, make the trailers zero, and
 * then overwrite the trailing 3-n bytes with '='
 */
static void
armorMorsel(PGPByte const raw[3], char armor[4])
{
        armor[0] = armorTable[raw[0] >> 2 & 0x3f];
        armor[1] = armorTable[(raw[0] << 4 & 0x30) + (raw[1] >> 4 & 0x0f)];
        armor[2] = armorTable[(raw[1] << 2 & 0x3c) + (raw[2] >> 6 & 0x03)];
        armor[3] = armorTable[raw[2] & 0x3f];
}

static void
armorWriteClassify (ArmorContext *context, PGPByte const *buf)
{
	if (context->blocktype)
		return;

	if (PKTBYTE_TYPE(*buf) == PKTBYTE_PUBKEY)
		context->blocktype = "PUBLIC KEY BLOCK";
	else if (PKTBYTE_TYPE(*buf) == PKTBYTE_SECKEY)
		context->blocktype = "PRIVATE KEY BLOCK";
	else
		context->blocktype = "MESSAGE";
}

/* Message ID is derived by hashing the beginning of the message data */
static void
armorDeriveMessageID (ArmorContext *context)
{
	PGPHashVTBL const *h;
	PGPHashContext *hc;
    PGPMemoryMgrRef	memoryMgr;
	PGPByte const *p;
	PGPUInt32 len;
	PGPByte hashdata[24];		/* Produces 32 chars of messageid */

	memoryMgr = PGPGetContextMemoryMgr( context->cdkContext );
	pgpAssert (IsntNull (memoryMgr) );
	h = pgpHashByNumber (kPGPHashAlgorithm_SHA);
	if (!h)
		goto error;
	hc = pgpHashCreate (memoryMgr, h);
	if (!hc)
		goto error;
	PGPContinueHash (hc, context->input, context->inlen);
	p = (PGPByte *) pgpHashFinal (hc);
	pgpClearMemory (hashdata, sizeof(hashdata));
	pgpCopyMemory (p, hashdata, pgpMin (h->hashsize, sizeof(hashdata)));
	PGPFreeHashContext (hc);
	context->messageid = (char *)PGPNewData (memoryMgr,
											 sizeof(hashdata)*4/3 + 1, 0);
	if (!context->messageid)
		return;

	len = armorLine (hashdata, sizeof(hashdata), context->messageid);
	context->messageid[len] = '\0';

	return;

error:
	/* Create an empty messageid */
	context->messageid = (char *)PGPNewData ( memoryMgr, 1, 0 );
	context->messageid[0] = '\0';
	return;
}

static PGPError
armorFlushHeader (ArmorContext *context)
{
	PGPByte const *ptr;
	PGPSize len;
	PGPError	error;
	size_t retlen;

	ptr = pgpFifoPeek (context->fd, context->header, &len);
	while (len) {
		retlen = context->tail->write (context->tail, ptr, len,
					       &error);
		pgpFifoSeek (context->fd, context->header, retlen);
		if (error)
			return error;
		
		ptr = pgpFifoPeek (context->fd, context->header, &len);
	}
	return kPGPError_NoErr;
}

static void
armorMakeHeader (ArmorContext *context)
{
	char temp[20];

	if (context->pgpmime == PGPMIMEENC) {
		context->mimebodyoff = 0;
		context->mimeheaderlines = 0;
		if( context->pgpmimeversionline ) {
			pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)MIMEENCHDR1a, strlen(MIMEENCHDR1a));
			context->mimebodyoff += strlen(MIMEENCHDR1a);
			context->mimeheaderlines += MIMEENCHDRLINES1a;
		}
		pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)MIMEENCHDR1b, strlen(MIMEENCHDR1b));
		context->mimebodyoff += strlen(MIMEENCHDR1b);
		context->mimeheaderlines += MIMEENCHDRLINES1b;
		pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)MIMEENCHDR2, strlen(MIMEENCHDR2));
	}
	pgpFifoWrite (context->fd, context->header,
		      (PGPByte const *)"-----BEGIN PGP ", 15);
	pgpFifoWrite (context->fd, context->header, (PGPByte *)context->blocktype,
		      strlen (context->blocktype));

	if (context->maxparts) {
		pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)", PART ", 7);
		sprintf (temp, "%02u", context->thispart);
		pgpFifoWrite (context->fd, context->header, (PGPByte const *)temp,
			      strlen (temp));
		if (context->version <= PGPVERSION_3 ||
		       context->thispart == context->maxparts) {
			pgpFifoWrite (context->fd, context->header,
				      (PGPByte const *)"/", 1);
			sprintf (temp, "%02u", context->maxparts);
			pgpFifoWrite (context->fd, context->header,
				      (PGPByte const *)temp, strlen (temp));
		}
	}

	pgpFifoWrite (context->fd, context->header,
		      (PGPByte const *)"-----\nVersion: ", 15);
	if( IsntNull( context->versionString ) ) {
		pgpFifoWrite (context->fd, context->header,
		      (PGPByte const *)context->versionString,
		      strlen (context->versionString));
	} else {
		char sVersionString[ 256 ];
		PGPGetSDKString( sVersionString );
		pgpFifoWrite (context->fd, context->header,
		      (PGPByte const *)sVersionString, strlen (sVersionString));
	}

	if (context->needmessageid && context->maxparts) {
		if (!context->messageid) {
			armorDeriveMessageID (context);
		}
		if (context->messageid) {
			pgpFifoWrite (context->fd, context->header,
					  (PGPByte const *)"\nMessageID: ", 12);
			pgpFifoWrite (context->fd, context->header,
					  (PGPByte const *)context->messageid,
					  strlen (context->messageid));
		}
	}

	/* Don't do comment if empty string.  This is more convenient for
	 * callers who want no comment, in some cases. */
	if (context->comment && context->comment[0]) {
		pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)"\nComment: ", 10);
		pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)context->comment,
			      strlen (context->comment));
	}

	if (context->charset) {
		pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)"\nCharset: ", 10);
		pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)context->charset,
			      strlen (context->charset));
	}

	pgpFifoWrite (context->fd, context->header, (PGPByte *)"\n\n", 2);
	context->didheader = 1;
}

static void
armorMakeFooter (ArmorContext *context)
{
	char temp[20];
	PGPByte crc[3];

	/* Emit CRC, MSB-first */
	crc[0] = (PGPByte)(context->crc >> 16);
	crc[1] = (PGPByte)(context->crc >> 8);
	crc[2] = (PGPByte)context->crc;
	armorMorsel (crc, temp);

	pgpFifoWrite (context->fd, context->header, (PGPByte const *)"=", 1);
	pgpFifoWrite (context->fd, context->header, (PGPByte const *)temp, 4);

	/* Now emit the end */
	pgpFifoWrite (context->fd, context->header,
		      (PGPByte const *)"\n-----END PGP ", 14);
	pgpFifoWrite (context->fd, context->header,
		      (PGPByte const *)context->blocktype,
		      strlen (context->blocktype));

	if (context->maxparts) {
		pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)", PART ", 7);
		sprintf (temp, "%02u", context->thispart);
		pgpFifoWrite (context->fd, context->header,
			      (PGPByte const *)temp, strlen (temp));
		if (context->version <= PGPVERSION_3 ||
		    context->thispart == context->maxparts) {
			pgpFifoWrite (context->fd, context->header,
				      (PGPByte const *)"/", 1);
			sprintf (temp, "%02u", context->maxparts);
			pgpFifoWrite (context->fd, context->header,
				      (PGPByte const *)temp, strlen (temp));
		}
	}
	pgpFifoWrite (context->fd, context->header,
		      (PGPByte const *)"-----\n", 6);

	if (context->pgpmime) {
		pgpFifoWrite (context->fd, context->header, (PGPByte *)"\n--", 3);
		if (context->pgpmime == PGPMIMEENC) {
			pgpFifoWrite (context->fd, context->header,
						  (PGPByte *)MIMEENCSEP, strlen(MIMEENCSEP));
		} else {
			pgpFifoWrite (context->fd, context->header,
						  (PGPByte *)context->mimesigsep, NSEPCHARS);
		}
		pgpFifoWrite (context->fd, context->header, (PGPByte *)"--\n", 3);
	}

	context->didfooter = 1;
	context->crc = CRC_INIT;
}

static PGPError
armorNewFile (PGPPipeline *myself)
{
	ArmorContext *context = (ArmorContext *)myself->priv;
	PGPError	error;
	PGPSize thispart;

	/*
	 * First, if we're already in a part, then close off the last
	 * one.
	 */
	if (context->thispart) {
		if (!context->didfooter)
			armorMakeFooter (context);

		error = armorFlushHeader (context);
		if (error)
			return error;

		error = context->tail->sizeAdvise (context->tail, 0);
		if (error)
			return error;

		error = context->tail->annotate (context->tail, myself,
						 PGPANN_MULTIARMOR_END, 0, 0);
		if (error)
			return error;
	}

	thispart = context->thispart+1;
	error = context->tail->annotate (context->tail, myself,
					 PGPANN_MULTIARMOR_BEGIN,
					 (PGPByte const *)&thispart,
					 sizeof (thispart));
	if (error)
		return error;

	context->thispart++;
	context->didheader = 0;
	context->didfooter = 0;
	context->lineno = 0;

	/* And give it an appropriate header */
	armorMakeHeader (context);

	return kPGPError_NoErr;
}

/*
 * The method here to do the armoring is somewhat tricky.
 * Most lines just have inlen = 48 which maps to 48*4/3 = 64
 * output characters.  But the last line has a short inlen.
 * This leads to a truncated last group, which looks like one of:
 * xx== (if the last group contains 1 byte - 4 bits of padding are zero)
 * xxx= (if the last group contains 2 bytes - 2 bits of padding are zero)
 * xxxx (if the last group contains 3 bytes)
 * To do this, we make sure that we've added an extra 0 byte to the
 * end of the input, then encode it in blocks of 3 bytes, then note by
 * how much the encoding overshot the input length, len - inlen.
 * This is 2, 1, or 0.  Overwrite that many trailing characters with '='.
 * Then a newline can be appended for output.
 */
static int
armorLine (PGPByte const *in, unsigned inlen, char *out)
{
	unsigned len;
	int t;
	char const *out0 = out;

	/* Fill the output buffer from the input buffer */
	for (len = 0; len < inlen; len += 3) {
		armorMorsel (in, out);
		in += 3;
		out += 4;
	}

	/* Now back up and erase any overrun */
	t = (int)(inlen - len);		/* Zero or negative */
	while (t)
		out[t++] = '=';
	      
	return (out - out0);
}

static void
armorProcessLine (ArmorContext *context)
{
	/* Update CRC */
	context->crc = crcUpdate (context->crc, context->input,
				  context->inlen);

	/* Apply padding so that we can overshoot */
	if (context->inlen < sizeof (context->input))
		context->input[context->inlen] = 0;
	
	/* Refill the output buffer from the input buffer */
	context->outlen = armorLine (context->input, context->inlen,
				     context->output);

	context->output [context->outlen++] = '\n';
	context->outptr = context->output;
	context->inlen = 0;
}

static PGPError
Flush (PGPPipeline *myself)
{
	ArmorContext *context;
	PGPError	error;
	size_t retlen;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORMAGIC);

	context = (ArmorContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	/* Try to flush anything that we have buffered. */
	while (context->outlen) {
		retlen = context->tail->write (context->tail,
					       (PGPByte *)context->outptr,
					       context->outlen,
					       &error);
		context->outlen -= retlen;
		context->outptr += retlen;

		if (!context->outlen)
			context->lineno++;

		if (error)
			return error;
	}

	return kPGPError_NoErr;
}

/*
 * First, try to flush out the output buffer.  Once that is empty,
 * refill the output buffer from the input buffer, append a newline,
 * and set the input buffer to 0 input.  Then try to write that out,
 * too.
 */
static PGPError
DoFlush (PGPPipeline *myself)
{
	ArmorContext *context = (ArmorContext *)myself->priv;
	PGPError	error = kPGPError_NoErr;

	/*
	 * Try to flush anything that we have buffered.  I know that
	 * the first time Flush is called, outlen WILL be zero!
	 */
	error = Flush (myself);
	if (error)
		return error;

	/* Next, try to output anything in the header fifo */
	error = armorFlushHeader (context);
	if (error)
		return error;


	/* Finally, process the input buffer and flush it out again */
	if (context->inlen) {
		armorProcessLine (context);
		error = DoFlush (myself);
	}

	return error;
}

/*
 * Write a single line.
 */
static size_t
armorWriteBytes (PGPPipeline *myself, PGPByte const *buf, size_t size,
		 PGPError *error)
{
	ArmorContext *context = (ArmorContext *)myself->priv;
	size_t size0 = size;
	unsigned t;

	/*
	 * Try to flush anything that we have buffered.  I know that
	 * the first time Flush is called, outlen WILL be zero!
	 */
	*error = Flush (myself);
	if (*error)
		return 0;

	/* try to fill up the input buffer */
	t = pgpMin ((sizeof (context->input) - context->inlen), size);
	memcpy (context->input + context->inlen, buf, t);
	buf += t;
	size -= t;
	context->inlen += t;

	if( context->inlen ) {
		armorWriteClassify (context, context->input);
		
		/*
		 * check if we need a new file.  This is called if we
		 * exceed armorlines, or when we do have multiparts and we do
		 * not have a current part.
		 */
		if (context->armorlines && (context->lineno >=
					    context->armorlines ||
					    !context->thispart)) {

			/*
			 * Set maxparts to -1 if this _is_ multipart armor
			 * using one-pass processing.
			 */
			if (context->version > PGPVERSION_3 &&
			    !context->maxparts && !context->sizevalid)
				context->maxparts = -1;

			*error = armorNewFile (myself);
			if (*error)
				return size0-size;
		}
	}

	/* If we've filled a line, then flush it out */
	if (context->inlen == sizeof (context->input)) {
		if (!context->didheader)
			armorMakeHeader (context);
		*error = DoFlush (myself);
		if (*error)
			return size0-size;
	}

	return size0-size;
}

/*
 * Minimally perform a DoFlush(), but try to flush the fifo, too!
 *
 * If context->version > PGPVERSION_3 then we only start writing
 * things out context->armorlines == 0 or when we have a full block
 * worth of data (48 * (context->armorlines - context->lineno)).  If
 * context->sizevalid == 2, we have an EOF and should flush the rest
 * of the fifo regardless.
 */
static PGPError
armorFlushFifo (PGPPipeline *myself)
{
	ArmorContext *context = (ArmorContext *)myself->priv;
	PGPByte const *ptr;
	PGPSize len;
	size_t retlen;
	PGPError	error;

	ptr = pgpFifoPeek (context->fd, context->fifo, &len);
	while (len) {
		if (context->version > PGPVERSION_3 &&
		    context->sizevalid < 2 &&
		    context->armorlines &&
		    pgpFifoSize (context->fd, context->fifo) < 48 *
		    (context->lineno == context->armorlines ? 
		     context->armorlines :
		     (context->armorlines - context->lineno)))
			return kPGPError_NoErr;

		retlen = armorWriteBytes (myself, ptr, len, &error);
		pgpFifoSeek (context->fd, context->fifo, retlen);
		if (error)
			return error;

		ptr = pgpFifoPeek (context->fd, context->fifo, &len);
	}
	return kPGPError_NoErr;
}

/*
 * This is the function that processes a clearsigned message.  It uses
 * context->state to tell it what it is doing:
 *	0) at the beginning -- check it
 *	1) middle of line, pass it through
 * 2) end of line with \r -- check next character for \n */
static size_t
armorDoClearsign (PGPPipeline *myself, PGPByte const *buf, size_t size,
		  PGPError *error)
{
	ArmorContext *context = (ArmorContext *)myself->priv;
	size_t written, size0 = size;
	PGPByte const *ptr;
	static PGPByte const from[] = "From ";
	int i, t, flag;

	
	while (size) {
		if (context->state == 2) {
			context->state = 0;
			if (*buf == '\n') {
				ptr = buf+1;
				goto do_write;
			}
		}
			
		/* 
		 * Here, we check the beginning of the line.  We use the
		 * flag to denote what we've found/buffered:
		 *	0) no match
		 *	1) full match
		 *	2) partial match (and we buffered data)
		 */
		if (!context->state) {
			flag = 0;
			if (!context->linebuf) {
				/* nothing is buffered */
				if (*buf == '-')
					flag = 1;
			}
			if (!flag) {
				t = pgpMin (5, size - context->linebuf);
				for (i = 0; i < t; i++)
					if (buf[i] != from[context->linebuf +
							   i]) {
						pgpFifoWrite
							(context->fd,
							 context->header,
							 context->input,
							 context->linebuf);
						context->linebuf = 0;
						break;
					}
				if (i == t) {
					/* We matched the whole input buffer */
					if (i + context->linebuf == 5)
						flag = 1;
					else {
						memcpy (context->input +
							context->linebuf, buf,
							t);
						context->linebuf += t;
						size -= t;
						buf += t;
						flag = 2;
					}
				}
			}
			if (flag == 1) {
				pgpFifoWrite (context->fd, context->header,
					      (PGPByte const *)"- ", 2);
				if (context->linebuf) {
					pgpFifoWrite (context->fd,
						      context->header,
						      context->input,
						      context->linebuf);
					context->linebuf = 0;
				}
			}

			if (flag != 2)
				context->state = 1;
		}

		*error = armorFlushHeader (context);
		if (*error)
			return size0 - size;

		for (ptr = buf; ptr < buf+size; ptr++) {
			if (*ptr == '\r' || *ptr == '\n') {
				context->state = (*ptr++ == '\r') ?
					2 : 0;
				break;
			}
		}

do_write:
		written = context->tail->write (context->tail, buf,
						ptr-buf, error);
		buf += written;
		size -= written;
		if (*error)
			return size0 - size;

	}
	
	return size0 - size;
}

/*
 * This function creates a pgp-mime multipart/signed (clearsigned) message.
 * We are now assuming that caller is responsible for giving us a good,
 * MIME formatted body part.  So we just pass it through.
 */
static size_t
armorDoMimesign (PGPPipeline *myself, PGPByte const *buf, size_t size,
		 PGPError *error)
{
	ArmorContext *context = (ArmorContext *)myself->priv;
	size_t written, size0 = size;

	while (size) {
		written = context->tail->write (context->tail, buf, size, error);
		buf += written;
		size -= written;
		if (*error)
			return size0 - size;
	}
	
	return size0 - size;
}

static size_t
Write (PGPPipeline *myself, PGPByte const *buf, size_t size, PGPError *error)
{
	ArmorContext *context;
	size_t retval, written = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORMAGIC);
	pgpAssert (error);

	context = (ArmorContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	if (context->outlen) {
		*error = DoFlush (myself);
		if (*error)
			return 0;
	}

	if (context->clearsign) {
		if (context->pgpmime)
			return armorDoMimesign (myself, buf, size, error);
		else
			return armorDoClearsign (myself, buf, size, error);
	}

	if (!context->sizevalid || context->version > PGPVERSION_3) {
		written = pgpFifoWrite (context->fd, context->fifo, buf, size);

		if (context->version <= PGPVERSION_3)
			return written;
	}

	*error = armorFlushFifo (myself);
	if (*error || context->version > PGPVERSION_3)
		return written;

	/* Try to write all the data we were given */
	do {
		retval = armorWriteBytes (myself, buf, size, error);
		written += retval;
		buf += retval;
		size -= retval;
		if (*error)
			return written;
	} while (size);

	return written;
}

static PGPError
Annotate (PGPPipeline *myself, PGPPipeline *origin, int type,
	  PGPByte const *string, size_t size)
{
	ArmorContext *context;
	PGPError	error = kPGPError_NoErr;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORMAGIC);

	context = (ArmorContext *)myself->priv;
	pgpAssert (context);

	switch( type ) {
	case PGPANN_PGPMIME_HEADER_SIZE:
		pgpAssert( size == sizeof(PGPSize) );
		*(PGPSize *)string = context->mimebodyoff;
		break;
	case PGPANN_PGPMIME_HEADER_LINES:
		pgpAssert( size == sizeof(PGPUInt32) );
		*(PGPUInt32 *)string = context->mimeheaderlines;
		break;
	case PGPANN_PGPMIME_SEPARATOR:
		if (context->pgpmime == PGPMIMEENC) {
			strncpy((char *)string, MIMEENCSEP,
					pgpMin(size, strlen(MIMEENCSEP)+1));
		} else {
			strncpy((char *)string, context->mimesigsep,
					pgpMin(size, NSEPCHARS+1));
		}
		break;
	default:
		if (context->tail)
			error = context->tail->annotate (context->tail, origin, type,
						 string, size);
		break;
	}
	if (!error)
		PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);
	return error;
}

static PGPError
SizeAdvise (PGPPipeline *myself, unsigned long bytes)
{
	ArmorContext *context;
	PGPError	error = kPGPError_NoErr;
	unsigned long total;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORMAGIC);

	context = (ArmorContext *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	/* Only handle bytes until EOF */
	if (context->scope_depth)
		return kPGPError_NoErr;

	/* Then compute maxparts */
	if (!context->sizevalid) {
		if (context->armorlines) {
			total = bytes + pgpFifoSize (context->fd,
						     context->fifo);

			if (context->version > PGPVERSION_3 && 
			    context->thispart) {
				total += 48 * context->armorlines *
					(context->thispart - 1);
				total += 48 * context->lineno;
			}

			/* 48 bytes / line */
			context->maxparts = total / (context->armorlines * 48);
			if (context->maxparts)
				context->maxparts++;
		}
	}
	context->sizevalid = 1;
	if (bytes)
		return kPGPError_NoErr;	/* dont handle any more non-zero sizeAdvises */

	/* This is the end */
	context->sizevalid = 2;

	/* Clear out the clearsign buffer, if we have it. */
	if (context->clearsign && context->linebuf) {
		pgpFifoWrite (context->fd, context->header, context->input,
			      context->linebuf);
		context->linebuf = 0;
	}

	/*
	 * If clearsign input ended with '\r', will be hashed with '\r\n' but
	 * the output file will be missing the '\n'.  Add it here.
	 */
	if (context->clearsign && context->state==2) {
		pgpFifoWrite (context->fd, 
			 context->header,  (const unsigned char *) "\n", 
			      1);
		context->state = 0;
	}

	error = armorFlushFifo (myself);
	if (error)
		return error;
	
	if (!context->didheader && !context->clearsign)
		armorMakeHeader (context);
	error = DoFlush (myself);
	if (error)
		return error;

	if (!context->clearsign) {
		armorMakeFooter (context);

		/* Set clearsign so we don't emit another CRC */
		context->clearsign = 1;

		error = DoFlush (myself);
		if (error)
			return error;
	}

	if (context->tail)
		error = context->tail->sizeAdvise (context->tail, bytes);
	return error;
}

static PGPError
Teardown (PGPPipeline *myself)
{
	ArmorContext *	context;
	PGPContextRef	cdkContext;
	
	pgpAssertAddrValid( myself, PGPPipeline );
	cdkContext	= myself->cdkContext;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORMAGIC);

	context = (ArmorContext *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	pgpFifoDestroy (context->fd, context->fifo);
	pgpFifoDestroy (context->fd, context->header);
	if (context->messageid)
	{
		pgpContextMemFree( cdkContext, context->messageid);
	}
		
		
	pgpClearMemory ( context, sizeof (*context) );
	pgpContextMemFree( cdkContext, context);
	
	return( kPGPError_NoErr );
}

#define ATYPE_NORMAL	0
#define ATYPE_CLEARSIG	10
#define ATYPE_SEPSIG	20
#define ATYPE_SEPSIGMSG	21
#define ATYPE_MIMESIG	30
#define ATYPE_MIMEENC	40
#define ATYPE_MIMESEPSIG 50

static PGPPipeline **
armorDoCreate (
	PGPContextRef			cdkContext,
	PGPPipeline **			head,
	ArmorContext **			pcontext,
	PGPEnv const *			env,
	PgpVersion				version,
	PGPFifoDesc const *		fd,
	PGPRandomContext const *rc,
	unsigned long			armorlines,
	int						armortype,
	PGPByte const *			hashlist,
	unsigned				hashlen)
{
	PGPPipeline *mod;
	ArmorContext *context;
	PGPFifoContext *fifo;
	PGPFifoContext *header;

	if (!head)
		return NULL;

	pgpAssert (fd);

	context = (ArmorContext *)pgpContextMemAlloc( cdkContext,
		sizeof (*context), kPGPMemoryMgrFlags_Clear );
	if ( IsNull( context ) )
		return NULL;
	mod	= &context->pipe;
	if( IsntNull( pcontext ) )
		*pcontext = context;

	fifo = pgpFifoCreate ( cdkContext, fd);
	if (!fifo) {
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}
	header = pgpFifoCreate (cdkContext, fd);
	if (!header) {
		pgpFifoDestroy (fd, fifo);
		pgpContextMemFree( cdkContext, context);
		return NULL;
	}
	mod->magic = ARMORMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "ASCII Armor Write Module";
	mod->priv = context;
	mod->cdkContext	= cdkContext;

	context->cdkContext = cdkContext; 
	context->outptr = context->output;
	context->armorlines = armorlines;
	context->crc = CRC_INIT;
	context->fd = fd;
	context->fifo = fifo;
	context->header = header;
	context->version = version;
	context->comment	= pgpenvGetCString (env, PGPENV_COMMENT, NULL );
	context->versionString = pgpenvGetCString( env,
											   PGPENV_VERSION_STRING, NULL );

	/* Will need to create a message id; now done deterministically later */
	if (rc) {
		context->needmessageid = TRUE;
	}

	if (armortype == ATYPE_CLEARSIG) {
#define CLRSIGN "-----BEGIN PGP SIGNED MESSAGE-----"
		int dohash=0;
		PGPHashVTBL const *hash;

		pgpFifoWrite (fd, header, (PGPByte const *)CLRSIGN,
			      strlen (CLRSIGN));
		context->clearsign = 1;
		context->armorlines = 0;

		/*
		 * Now deal with multiple hash types.  If we don't specify
		 * the hash type, or the only hash used is MD5, then ignore
		 * the Hash header.  Otherwise, we should print the out
		 */
		if (!hashlen || (hashlen == 1 && *hashlist == kPGPHashAlgorithm_MD5))
			goto clearsig_end;
		pgpFifoWrite (fd, header, (PGPByte const *)"\nHash: ", 7);
		while (hashlen--) {
			hash = pgpHashByNumber ( (PGPHashAlgorithm) (*hashlist++) );
			if (hash) {
				if (dohash)
					pgpFifoWrite (fd, header,
						      (PGPByte const *)", ", 2);
				pgpFifoWrite (fd, header,
					      (PGPByte const *)hash->name,
					      strlen (hash->name));
				dohash = 1;
			}
		}
clearsig_end:
		pgpFifoWrite (fd, header, (PGPByte const *)"\n\n", 2);
		context->didheader = TRUE;		/* No need for header here */
	} else if (armortype == ATYPE_MIMESIG) {
//		pgpFifoWrite (fd, header, (PGPByte const *)MIMETOPHDR,
//			      strlen(MIMETOPHDR));
		context->clearsign = 1;
		context->armorlines = 0;
		context->pgpmime = PGPMIMESIG;
	} else if (armortype == ATYPE_SEPSIG) {
		context->blocktype = "SIGNATURE";
	} else if (armortype == ATYPE_SEPSIGMSG) {
		context->blocktype = "MESSAGE";
	} else if (armortype == ATYPE_MIMESEPSIG) {
		context->blocktype = "MESSAGE";
		context->pgpmime = PGPMIMESIG;
		context->armorlines = 0;
	} else if (armortype == ATYPE_MIMEENC) {
		context->pgpmime = PGPMIMEENC;
		context->armorlines = 0;
		context->pgpmimeversionline =
			pgpenvGetInt( env, PGPENV_PGPMIMEVERSIONLINE, NULL, NULL );
	}

	context->tail = *head;
	*head = mod;
	return &context->tail;
}

PGPPipeline **
pgpArmorWriteCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PGPEnv const *env,
	PGPFifoDesc const *fd,
	PGPRandomContext const *rc,
	PgpVersion version,
	PGPByte armortype)
{
	int type = 0;
	unsigned long armorlines;
	PGPError	error;

	if (!head)
		return NULL;

	if (armortype == PGP_ARMOR_NORMAL) {
		if (pgpenvGetInt (env, PGPENV_PGPMIME, NULL, &error)) {
			type = ATYPE_MIMEENC;
			rc = NULL; /* prevents messageid appearing */
		} else {
			type = ATYPE_NORMAL;
		}
	} else if (armortype == PGP_ARMOR_SEPSIG) {
		type = ATYPE_SEPSIG;
	} else if (armortype == PGP_ARMOR_SEPSIGMSG) {
		type = ATYPE_SEPSIGMSG;
	} else {
		return NULL;
	}

	armorlines = pgpenvGetInt (env, PGPENV_ARMORLINES, NULL, NULL);


	return (armorDoCreate ( cdkContext,
		head, NULL, env, version, fd, rc, armorlines, type, NULL, 0));
}

PGPPipeline **
pgpArmorWriteCreateClearsig (
	PGPPipeline **texthead,
	PGPPipeline **signhead,
	PGPEnv const *env,
	PGPFifoDesc const *fd,
	PgpVersion version, PGPByte *hashlist,
	unsigned hashlen)
{
	PGPPipeline *txthead = NULL, *sighead = NULL;
	PGPPipeline **joinhead, **sigtail, **tail;
	ArmorContext *context;
	PGPContextRef	cdkContext	= pgpenvGetContext( env );

	if (!texthead || !signhead)
		return NULL;

	joinhead = armorDoCreate ( cdkContext,
			&txthead, NULL, env, version, fd, NULL, 0,
				  ATYPE_CLEARSIG, hashlist, hashlen);
	if (!joinhead)
		return NULL;

	tail = pgpJoinCreate (cdkContext, joinhead, fd);
	if (!tail) {
		txthead->teardown (txthead);
		return NULL;
	}

	sigtail = armorDoCreate ( cdkContext,
			&sighead, NULL, env, version, fd, NULL, 0, ATYPE_SEPSIG, NULL, 0);
	if (!sigtail) {
		txthead->teardown (txthead);
		return NULL;
	}

	/* Add the charset used when generating the clearsigned message */
	context = (ArmorContext *)sighead->priv;
	pgpAssert (context);
	context->charset	= pgpenvGetCString (env, PGPENV_CHARSET, NULL);
	/* noconv is default, don't put it in */
	if( strcmp( context->charset, "noconv" ) == 0) {
		context->charset = NULL;
	}

	*sigtail = pgpJoinAppend (*joinhead);
	if (!*sigtail) {
		txthead->teardown (txthead);
		sighead->teardown (sighead);
		return NULL;
	}
		
	pgpJoinBuffer (*sigtail, (PGPByte *)"\n", 1);

	*texthead = txthead;
	*signhead = sighead;
	return tail;
}

PGPPipeline **
pgpArmorWriteCreatePgpMimesig (
	PGPPipeline **texttail,
	PGPPipeline **signtail,
	PGPEnv const *env,
	PGPFifoDesc const *fd,
	PGPRandomContext const *rc,
	PgpVersion version, PGPByte *hashlist,
	unsigned hashlen)
{
	ArmorContext *context, *sigcontext;
	PGPPipeline *txthead = NULL, *sighead = NULL;
	PGPPipeline **joinhead, **sigtail, **tail;
	PGPPipeline **splithead, **splittail;
	PGPContextRef	cdkContext	= pgpenvGetContext( env );
	PGPByte		sepbits[(NSEPCHARS+7)/8];
	PGPSize		bodyoff = 0;
	PGPUInt32	headerlines = 0;
	PGPUInt32	i;

	if (!texttail || !signtail || !rc)
		return NULL;

	splithead = armorDoCreate ( cdkContext,
		&txthead, &context, env, version, fd, NULL, 0,
		 ATYPE_MIMESIG, hashlist, hashlen);
	if (!splithead)
		return NULL;

	/* Create split to take output from clearmime armor module */
	joinhead = pgpSplitCreate(cdkContext, splithead);

	/* Set second split output to go to signing pipeline */
	splittail = pgpSplitAdd (*splithead);
	*splittail = *texttail;

	tail = pgpJoinCreate ( cdkContext, joinhead, fd);
	if (!tail) {
		txthead->teardown (txthead);
		return NULL;
	}

	sigtail = armorDoCreate ( cdkContext, &sighead, &sigcontext, env, version,
				fd, NULL, 0, ATYPE_MIMESEPSIG, NULL, 0);
	if (!sigtail) {
		txthead->teardown (txthead);
		return NULL;
	}

	*sigtail = pgpJoinAppend (*joinhead);
	if (!*sigtail) {
		txthead->teardown (txthead);
		sighead->teardown (sighead);
		return NULL;
	}

	/* Create a random message separator */
	pgpRandomGetBytes (rc, sepbits, (NSEPCHARS+7)/8);
	for (i=0; i<NSEPCHARS; ++i) {
		context->mimesigsep[i] = ((sepbits[i/8]>>(i&7))&1) ? '-' : '=';
	}
	context->mimesigsep[NSEPCHARS] = '\0';
	pgpCopyMemory(context->mimesigsep, sigcontext->mimesigsep, NSEPCHARS+1);

	bodyoff = 0;
	headerlines = 0;
	context->pgpmimeversionline = pgpenvGetInt( env,
									PGPENV_PGPMIMEVERSIONLINE, NULL, NULL );
	if( context->pgpmimeversionline ) {
		pgpJoinBuffer (*joinhead, (PGPByte *)MIMESIGHDR1a,
					   strlen(MIMESIGHDR1a));
		bodyoff += strlen(MIMESIGHDR1a);
		headerlines += MIMESIGHDRLINESa;
	}
	pgpJoinBuffer (*joinhead, (PGPByte *)MIMESIGHDR1b, strlen(MIMESIGHDR1b));
	pgpJoinBuffer (*joinhead, (PGPByte *)context->mimesigsep, NSEPCHARS);
	pgpJoinBuffer (*joinhead, (PGPByte *)MIMESIGHDR2, strlen(MIMESIGHDR2));
	bodyoff += strlen(MIMESIGHDR1b) + NSEPCHARS + strlen(MIMESIGHDR2);
	headerlines += MIMESIGHDRLINESb;
	if (!hashlen) {
		pgpJoinBuffer (*joinhead, (PGPByte *)MIMEMIC, strlen(MIMEMIC));
		pgpJoinBuffer (*joinhead, (PGPByte *)MIMEDFLTMIC,
				strlen(MIMEDFLTMIC));
		bodyoff += strlen(MIMEMIC) + strlen(MIMEDFLTMIC);
	} else {
		while (hashlen--) {
			PGPHashVTBL const *hash = NULL;
			unsigned j, len;
			char hashname[30]; /* lower case name */

			hash = pgpHashByNumber ( (PGPHashAlgorithm) (*hashlist++) );
			pgpAssert(hash);
			len = strlen(hash->name);
			pgpAssert (len < sizeof(hashname));
			pgpJoinBuffer (*joinhead, (PGPByte *)MIMEMIC,
				       strlen(MIMEMIC));
			for (j=0; j<len; ++j)
				hashname[j] = tolower(hash->name[j]);
			pgpJoinBuffer (*joinhead, (PGPByte *)hashname,
				       len);
			bodyoff += strlen(MIMEMIC) + len;
			if( hashlen ) {
				pgpJoinBuffer (*joinhead, (PGPByte *)";", 1);
				bodyoff += 1;
			}
		}
	}
	pgpJoinBuffer (*joinhead, (PGPByte *)"\n\n", 2);
	bodyoff += 2;
	headerlines += 2;
	context->mimebodyoff = bodyoff;
	context->mimeheaderlines = headerlines;
	pgpJoinBuffer (*joinhead, (PGPByte *)"--", 2);
	pgpJoinBuffer (*joinhead, (PGPByte *)context->mimesigsep, NSEPCHARS);
	pgpJoinBuffer (*joinhead, (PGPByte *)"\n", 1);

	pgpJoinBuffer (*sigtail, (PGPByte *)"\n--", 3);
	pgpJoinBuffer (*sigtail, (PGPByte *)context->mimesigsep, NSEPCHARS);
	pgpJoinBuffer (*sigtail, (PGPByte *)"\n", 1);
	pgpJoinBuffer (*sigtail, (PGPByte *)MIMEMIDBOUND, strlen(MIMEMIDBOUND));

	*texttail = txthead;
	*signtail = sighead;
	return tail;
}
