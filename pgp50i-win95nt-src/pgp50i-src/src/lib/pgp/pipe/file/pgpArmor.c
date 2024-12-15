/*
 * pgpArmor.c -- a module to perform Ascii Armor
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpArmor.c,v 1.6.2.2.2.1 1997/07/16 18:51:41 colin Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <ctype.h>

#include "pgpDebug.h"
#include "pgpArmor.h"
#include "pgpCRC.h"
#include "pgpPktByte.h"
#include "pgpRadix64.h"
#include "pgpAnnotate.h"
#include "pgpFIFO.h"
#include "pgpHash.h"
#include "pgpJoin.h"
#include "pgpEnv.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpPipeline.h"
#include "pgpRndom.h"
#include "pgpSplit.h"
#include "pgpUsuals.h"

#define ARMORMAGIC	0xa4904f11

/* Max length of a line in MIME content-printable encoding */
#define MIMEMAX 76

/*
 * PGP-MIME headers.  We chose "=-" as boundary since it can't occur in
 * a quoted-printable encoding, nor in base64.
 */
#define MIMESEP "=--"
#define MIMESIGHDR "Mime-Version: 1.0\n" \
	"Content-Type: multipart/signed; boundary=\"" MIMESEP "\";\n" \
	"  protocol=\"application/pgp-signature\"; "
#define MIMEENCHDR "Mime-Version: 1.0\n" \
	"Content-Type: multipart/encrypted; boundary=\"" MIMESEP "\";\n" \
	"  protocol=\"application/pgp-encrypted\";\n\n" \
	"--" MIMESEP "\nContent-Type: application/pgp-encrypted\n\n" \
	"Version: 1\n\n--" MIMESEP "\n" \
	"Content-Type: application/octet-stream\n\n"
#define MIMEMIC "micalg=pgp-"
#define MIMEDFLTMIC "md5"
#define MIMETOPBOUND "\n\n--" MIMESEP "\n"
#define MIMEMIDBOUND "\n--" MIMESEP "\n" \
	"Content-Type: application/pgp-signature\n\n"
#define MIMEENDBOUND "\n--" MIMESEP "--\n"
#define MIMETOPHDR   "Content-Type: text/plain; charset=iso-8859-1\n"\
	"Content-Transfer-Encoding: quoted-printable\n\n"

/* Input size for a line of ASCII armor. */
#define ARMOR_LINE_INPUT 48

/*
 * Hairy context with lots of flags for the vsrious options.
 * The actual radix-64 encoding always takes the input[] array
 * as input and the produces a full line in the output[] array as
 * output.  Then armorFlushOutput() routine writes it out.
 */
struct Context {
	struct PgpPipeline *tail;
	struct PgpFifoDesc const *fd;
	struct PgpFifoContext *fifo;
	struct PgpFifoContext *header;
	unsigned long	crc;
	unsigned long	armorlines;	/* This must be an unsigned type */
	unsigned long	lineno;
	unsigned	thispart;
	unsigned	maxparts;
	int	scope_depth;
	byte	input[ARMOR_LINE_INPUT];
#if 0
	char	output[65];	/* Maximum 63 buyes + \r and/or \n */
#else
	char	output[MIMEMAX+2]; /* MIME line plus \n plus null */
#endif
	char *	outptr;		/* First unwritten byte in output[] */
	char *	messageid;	/* This is allocated in create */
	char const *	blocktype;
	char const *	comment;
	char const *	charset;
	PgpVersion	version;
	byte	inlen;		/* valid bytes in input[] */
	byte	outlen;		/* valid bytes in output[] */
	byte	clearsign;	/* boolean, suppresses final footer */
	byte	pgpmime;
#define PGPMIMESIG 1
#define PGPMIMEENC 2
	byte	didheader;	/* this part's header written to FIFO yet? */
	byte	didfooter;	/* this part's footer written to FIFO yet? */
	byte	pgpann_pending;	/* PGPANN_MULTIARMOR_END needs to be sent */
	byte	at_eof;		/* boolean, have got sizeAdvise(0) */
	byte	state;		/* Used by clearsign quoting */
	byte	linebuf;	/* Used by clearsign quoting */
};

/*
 * Armor 3 raw bytes into 4
 * If armoring n < 3 bytes, make the trailers zero, and
 * then overwrite the trailing 3-n bytes with '='
 */
static void
armorMorsel(byte const raw[3], char armor[4])
{
        armor[0] = armorTable[raw[0] >> 2 & 0x3f];
        armor[1] = armorTable[(raw[0] << 4 & 0x30) + (raw[1] >> 4 & 0x0f)];
        armor[2] = armorTable[(raw[1] << 2 & 0x3c) + (raw[2] >> 6 & 0x03)];
        armor[3] = armorTable[raw[2] & 0x3f];
}

/* Classify the data in the input buffer, if it hasn't already been done */
static void
armorWriteClassify (struct Context *context)
{
	if (context->blocktype)
		return;

	if (context->inlen && PKTBYTE_TYPE(context->input[0]) == PKTBYTE_PUBKEY)
		context->blocktype = "PUBLIC KEY BLOCK";
	else
		context->blocktype = "MESSAGE";
}

/* Write out the output buffer (outptr, outlen) */
static int
armorFlushOutput (struct Context *context)
{
	int error;
	size_t retlen;

	pgpAssert (context);
	pgpAssert (context->tail);

	/* Try to flush anything that we have buffered. */
	while (context->outlen) {
		retlen = context->tail->write (context->tail,
					       (byte *)context->outptr,
					       context->outlen,
					       &error);
		context->outlen -= retlen;
		context->outptr += retlen;

		if (error)
			return error;
	}

	return 0;
}

/*
 * Try to write out all pending output, the output buffer, the header/footer
 * FIFO, and a pending PGPANN_MULTIARMOR_END annotation.
 */
static int
armorFlushPending (struct PgpPipeline *myself)
{
	struct Context *context = (struct Context *)myself->priv;
	byte const *ptr;
	unsigned len;
	int error;
	size_t retlen;

	/* Output buffer */
	error = armorFlushOutput (context);
	if (error)
		return error;

	/* header/footer FIFO */
	ptr = pgpFifoPeek (context->fd, context->header, &len);
	while (len) {
		retlen = context->tail->write (context->tail, ptr, len,
					       &error);
		pgpFifoSeek (context->fd, context->header, retlen);
		if (error)
			return error;

		ptr = pgpFifoPeek (context->fd, context->header, &len);
	}

	/* Annotation */
	if (context->pgpann_pending) {
		error = context->tail->annotate (context->tail, myself,
						 PGPANN_MULTIARMOR_END, 0, 0);
		if (error)
			return error;
		context->pgpann_pending = 0;
	}
	return 0;
}

/* Build a header and write it to the header FIFO */
static int
armorMakeHeader (struct PgpPipeline *myself)
{
	struct Context *context = (struct Context *)myself->priv;
	char temp[20];

	/* Write out the opening annotation, if needed */
	if (context->maxparts != 1) {
		unsigned thispart = context->thispart+1;
		int error;

		error = context->tail->annotate (context->tail, myself,
						 PGPANN_MULTIARMOR_BEGIN,
						 (byte const *)&thispart,
						 sizeof (thispart));
		if (error)
			return error;
	}

	/* Do the goodies */

	context->thispart++;
	context->lineno = 0;
	context->didfooter = 0;

	if (context->pgpmime == PGPMIMEENC) {
		pgpFifoWrite (context->fd, context->header,
			      (byte const *)MIMEENCHDR, strlen(MIMEENCHDR));
	}
	pgpFifoWrite (context->fd, context->header,
		      (byte const *)"-----BEGIN PGP ", 15);
	pgpFifoWrite (context->fd, context->header, (byte *)context->blocktype,
		      strlen (context->blocktype));

	if (context->maxparts != 1) {
		sprintf (temp, ", PART %02u", context->thispart);
		pgpFifoWrite (context->fd, context->header, (byte const *)temp,
			      strlen (temp));
		if (context->version <= PGPVERSION_2_6 ||
		    context->thispart == context->maxparts) {
			sprintf (temp, "/%02u", context->maxparts);
			pgpFifoWrite (context->fd, context->header,
				      (byte const *)temp, strlen (temp));
		}
	}

	pgpFifoWrite (context->fd, context->header,
		      (byte const *)"-----\nVersion: ", 15);
	pgpFifoWrite (context->fd, context->header,
		      (byte const *)pgpLibVersionString,
		      strlen (pgpLibVersionString));

	if (context->messageid) {
		pgpFifoWrite (context->fd, context->header,
			      (byte const *)"\nMessageID: ", 12);
		pgpFifoWrite (context->fd, context->header,
			      (byte const *)context->messageid,
			      strlen (context->messageid));
	}

	if (context->comment) {
		pgpFifoWrite (context->fd, context->header,
			      (byte const *)"\nComment: ", 10);
		pgpFifoWrite (context->fd, context->header,
			      (byte const *)context->comment,
			      strlen (context->comment));
	}

	if (context->charset) {
		pgpFifoWrite (context->fd, context->header,
			      (byte const *)"\nCharset: ", 10);
		pgpFifoWrite (context->fd, context->header,
			      (byte const *)context->charset,
			      strlen (context->charset));
	}

	pgpFifoWrite (context->fd, context->header, (byte *)"\n\n", 2);

	context->didheader = 1;
	return 0;	/* Success */
}

/* Build a footer (CRC onwards) and write it to the header FIFO */
static void
armorMakeFooter (struct Context *context)
{
	char temp[20];

	/* What to do with 0 bytes of data? */
	if (!context->didheader)
		return;

	/* Emit CRC, MSB-first */
	temp[0] = '=';
	temp[5] = (byte)(context->crc >> 16);
	temp[6] = (byte)(context->crc >> 8);
	temp[7] = (byte)context->crc;
	armorMorsel (temp+5, temp+1);

	pgpFifoWrite (context->fd, context->header, (byte const *)temp, 5);

	/* Now emit the -----END */
	pgpFifoWrite (context->fd, context->header,
		      (byte const *)"\n-----END PGP ", 14);
	pgpFifoWrite (context->fd, context->header,
		      (byte const *)context->blocktype,
		      strlen (context->blocktype));

	if (context->maxparts != 1) {
		sprintf (temp, ", PART %02u", context->thispart);
		pgpFifoWrite (context->fd, context->header,
			      (byte const *)temp, strlen (temp));
		if (context->version <= PGPVERSION_2_6 ||
		    context->thispart == context->maxparts) {
			sprintf (temp, "/%02u", context->maxparts);
			pgpFifoWrite (context->fd, context->header,
				      (byte const *)temp, strlen (temp));
		}
	}
	pgpFifoWrite (context->fd, context->header, (byte const *)"-----\n", 6);

	if (context->pgpmime) {
		pgpFifoWrite (context->fd, context->header,
		       (byte const *)MIMEENDBOUND, strlen(MIMEENDBOUND));
	}

	context->didfooter = 1;
	if (context->maxparts != 1)
		context->pgpann_pending = 1;	/* Send annotate afterwards! */
	context->crc = CRC_INIT;
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
armorLine (byte const *in, unsigned inlen, char *out)
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

/*
 * Generate a line of output based on the current input buffer.
 * Generate a header if required, then a line of output, writing both out.
 * This should only be called when the input buffer is full, except for
 * the last partial line.   If it is called with a partial line, it
 * must be terminated with a trailing 0 byte.
 */
static int
armorWriteLine (struct PgpPipeline *myself)
{
	struct Context *context = (struct Context *)myself->priv;
	int error = 0;

	/* Make a header if required */
	if (!context->didheader) {
		armorWriteClassify (context);
		error = armorMakeHeader (myself);
		if (error)
			return error;
		error = armorFlushPending (myself);
		if (error)
			return error;
	}

	/* Update CRC */
	context->crc = crcUpdate (context->crc, context->input,
				  context->inlen);

	/* Refill the output buffer from the input buffer */
	context->outlen = armorLine (context->input, context->inlen,
				     context->output);
	context->output [context->outlen++] = '\n';
	context->outptr = context->output;

	/* Mark input as processed */
	context->inlen = 0;
	context->lineno++;

	/* Write a footer, if required (note tricky "-1") */
	if (context->lineno > context->armorlines - 1) {
		if (!context->didfooter)
			armorMakeFooter (context);
		context->didheader = 0;
	}

	return armorFlushPending (myself);
}

/*
 * Write out a final partial line in the input buffer.
 */
static int
armorWritePartial (struct PgpPipeline *myself)
{
	struct Context *context = (struct Context *)myself->priv;
	int error = 0;

	error = armorFlushPending (myself);
	if (error)
		return error;

	if (context->inlen) {
		/* Ensure that padding byte is there! */
		if (context->inlen < sizeof(context->input))
			context->input[context->inlen] = 0;
		error = armorWriteLine (myself);
	}

	return error;
}

/*
 * Write out as many full lines as possible, leaving the rest in the
 * input buffer.
 */
static size_t
armorWriteBytes (struct PgpPipeline *myself, byte const *buf, size_t size,
		 int *error)
{
	struct Context *context = (struct Context *)myself->priv;
	size_t size0 = size;
	size_t t;

	pgpAssert(!context->outlen);

	/* Repeat for each line... */
	while (size) {
		t = min ((size_t)(ARMOR_LINE_INPUT - context->inlen), size);
		memcpy (context->input + context->inlen, buf, t);
		buf += t;
		size -= t;
		context->inlen += (byte)t;

		/* If we've filled it up, then flush it out */
		if (context->inlen == ARMOR_LINE_INPUT) {
			*error = armorWriteLine (myself);
			if (*error)
				return size0-size;
		}
	}
	return size0-size;
}

/*
 * Write out as much of the input FIFO as is possible.
 *
 * If context->version > PGPVERSION_2_6 then we only start writing things
 * out if context->armorlines == 0 or when we have a full block worth of
 * data (ARMOR_LINE_BYTES * (context->armorlines - context->lineno)).
 * Otherwise, we wait until we have it all so we can set the number of
 * parts header properly.
 */
static int
armorFlushFifo (struct PgpPipeline *myself, size_t flushlen)
{
	struct Context *context = (struct Context *)myself->priv;
	byte const *ptr;
	unsigned len;
	size_t retlen;
	int error;

	error = armorFlushPending (myself);
	if (error)
		return error;

	ptr = pgpFifoPeek (context->fd, context->fifo, &len);
	if (len > flushlen)
		len = (unsigned)flushlen;
	while (len) {
		retlen = armorWriteBytes (myself, ptr, len, &error);
		pgpFifoSeek (context->fd, context->fifo, retlen);
		flushlen -= retlen;
		if (error)
			return error;

		ptr = pgpFifoPeek (context->fd, context->fifo, &len);
		if (len > flushlen)
			len = (unsigned)flushlen;
	}
	return 0;
}

/*
 * This is the function that processes a clearsigned message.  It uses
 * context->state to tell it what it is doing:
 *	0) at the beginning -- check it
 *	1) middle of line, pass it through
 *	2) end of line with \r -- check next character for \n
 */
static size_t
armorDoClearsign (struct PgpPipeline *myself, byte const *buf, size_t size,
		  int *error)
{
	struct Context *context = (struct Context *)myself->priv;
	size_t written, size0 = size;
	byte const *ptr;
	byte const from[] = "From ";
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
		 * Here, we check the beginning of the line for "-" and
		 * "From ".  We use the flag to denote what we've
		 * found/buffered:
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
				t = min (5, size - context->linebuf);
				for (i = 0; i < t; i++)
					if (buf[i] != from[context->linebuf+i])
					{
						pgpFifoWrite (context->fd,
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
					      (byte const *)"- ", 2);
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

		*error = armorFlushPending (myself);
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
 * It performs the MIME quoted printable canonicalization, except for
 * handling end of line characters (=20, etc.).
 * context->output is used to build up an output line.  MIME quoted printable
 * limits its size.
 * context->state is used to record its status:
 * 2 - just after '\r'
 * 1 - just after '\n'
 * 0 - otherwise
 */
static size_t
armorDoMimesign (struct PgpPipeline *myself, byte const *buf, size_t size,
		 int *error)
{
	struct Context *context = (struct Context *)myself->priv;
	size_t written, size0 = size;
	char const *ptr;	/* point into buf */
	char *ptr2;		/* point into context->output */
	char c;			/* character we are manipulating */
	char pushback[2];	/* pushback char array */
	unsigned pbsize = 0;	/* chars in pushback */
	int mimequote;		/* true if char needs mime quoting (=XX) */
	char const from[] = "From";

	ptr = (char *)buf;
	ptr2 = context->output + context->outlen;

	/* Loop till we have consumed all input */
	while (size  || pbsize) {
		/* Loop per line of output */
		while (ptr < (char *)buf + size  ||  pbsize) {
			if (pbsize) {
				c = pushback[--pbsize];
			} else {
				c = *ptr++;
			}
			/* state = 2 means prev line termed with cr */
			if (context->state == 2) {
				/* eat lf */
				if (c == '\n') {
					context->state = 1;
					continue;
				}
			}
			context->state = 0;
			mimequote = 0;
			if (c == '\r' || c == '\n') {
				if (ptr2 > context->output &&
				    (ptr2[-1] == ' ' || ptr2[-1] == '\t')) {
					/* Fixup space at end of line */
					--ptr2;
					pushback[pbsize++] = c;
					c = ptr2[0];
					mimequote = 1;
				} else {
					context->state = (c == '\r') ? 2 : 1;
					break;
				}
			} else if ((c!='\t' && c<32) || c == '=' || c > 126) {
				mimequote = 1;
			} else if (ptr2 - context->output >= MIMEMAX) {
				/* need soft break */
				pushback[pbsize++] = c;
				pushback[pbsize++] = ptr2[-1];
				ptr2[-1] = '=';
				context->state = 1; /* force newline */
				break;
			} else if (c == ' ' &&
				   ptr2 - context->output == sizeof(from)-1 &&
				   strncmp((char *)context->output, from,
					   sizeof(from) - 1) == 0) {
				mimequote = 1;
			} else {
				*ptr2++ = c;
			}
			if (mimequote) {
				mimequote = 0;
				if (ptr2 - context->output > MIMEMAX - 3) {
					/* Need soft break */
					pushback[pbsize++] = c;
					if (ptr2 - context->output >=
					    MIMEMAX) {
						/* Need another pushback */
						pgpAssert(pbsize <
							sizeof(pushback));
						pushback[pbsize++] = ptr2[-1];
						ptr2[-1] = '=';
					} else {
						*ptr2++ = '=';
					}
					context->state = 1; /* force newline */
					break;
				} else {
					*ptr2++ = '=';
					*ptr2++ = "0123456789ABCDEF"[c>>4];
					*ptr2++ = "0123456789ABCDEF"[c&0xf];
				}
			}
		}
		if (context->state) {
			/* Output line now */
			*error = armorFlushPending (myself);
			if (*error)
				return size0 - size;

			*ptr2++ = '\n';
			*ptr2 = '\0';
			written = 0;
			context->outptr = context->output;
			while (written < (size_t)(ptr2 - context->outptr)) {
				written += context->tail->write (context->tail,
						 (byte *)context->outptr,
						 ptr2-context->outptr, error);
				context->outptr += written;
				if (*error)
					return ptr - (char *)buf;
			}
		}
		size -= ptr - (char *)buf;
		buf = (byte *)ptr;
		ptr2 = context->output;
	}

	context->outlen = ptr2 - context->output;
	return size0 - size;
}

/*
 *
 * Do we have to buffer?  There are three cases:
 * - If maxparts is known (non-zero), we can exit immediately.
 * - Otherwise, if version > PGPVERSION_2_6, we can write as soon
 *   as we have *more* than one part's data, since we can postpone
 *   figuring out maxparts once we know it's more than the current
 *   part.
 * - Otherwise, we need to wait until maxlines is known
 */
static size_t
armorMustBuffer (struct Context *context)
{
	unsigned long needed;

	if (context->maxparts)	/* armorlines = 0 -> no buffering */
		return 0;
	pgpAssert(context->armorlines);
	if (context->version <= PGPVERSION_2_6)
		return 1;	/* Must buffer */
	needed = context->armorlines * ARMOR_LINE_INPUT;
	return pgpFifoSize (context->fd, context->fifo) > needed;
}

static size_t
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
{
	struct Context *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORMAGIC);
	pgpAssert (error);

	context = (struct Context *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);
	pgpAssert (!context->at_eof);	/* No writes after sizeAdvise(0)! */

	*error = armorFlushPending (myself);
	if (*error)
		return 0;

	/* XXX Should just change function pointers. */
	if (context->clearsign) {
		if (context->pgpmime)
			return armorDoMimesign (myself, buf, size, error);
		else
			return armorDoClearsign (myself, buf, size, error);
	}

	/* Can we write it straight through? */
	if (context->maxparts && !pgpFifoSize (context->fd, context->fifo))
		return armorWriteBytes (myself, buf, size, error);

	/* Nope, have to buffer it... */
	size = pgpFifoWrite (context->fd, context->fifo, buf, size);

	if (context->maxparts) {
		*error = armorFlushFifo (myself, (size_t)-1);
		return size;
	}

	/*
	 * Can we write out any of the buffered data?  The new format
	 * (Omitting the "/05" of "PART 01/05" on all but the last part)
	 * allows us to if we have enough to be sure that this isn't
	 * the last part.
	 */
	if (context->version > PGPVERSION_2_6) {
		unsigned long needed;

		for (;;) {
			needed = context->armorlines - context->lineno;
			if (!needed)
				needed = context->armorlines;
			needed *= ARMOR_LINE_INPUT;
			if (pgpFifoSize (context->fd, context->fifo) <= needed)
				break;
			pgpFifoSize (context->fd, context->fifo);
			*error = armorFlushFifo (myself, needed);
			if (*error)
				break;
		}
	}
	return size;
}

/*
 * We mostly ingore annotations, since they can't be passed through
 * interleaved with our own.
 */
static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
	  byte const *string, size_t size)
{
	struct Context *context;
	int error = 0;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);

#if 0
	/* Suppress - this could confuse our downstream neighbours */
	error = context->tail->annotate (context->tail, origin, type,
					 string, size);
	if (!error)
#else
	(void)origin;
	(void)string;
	(void)size;
#endif
		PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert(context->scope_depth != -1);
	return error;
}

/*
 * We could in theory generate downstream SizeAdvises,
 * but it's not worth the bother until somone wants the hint.
 */
static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
{
	struct Context *context;
	int error = 0;
	unsigned long total;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);
	pgpAssert (context->tail);

	/* Ignore inner-scope SizeAdvise */
	if (context->scope_depth)
		return 0;

	/* Then compute maxparts */
	if (!context->maxparts) {
		pgpAssert(context->armorlines);

		/* Add buffered bytes */
		total = bytes + pgpFifoSize (context->fd, context->fifo);
		/* Convert to lines */
		total = (total + ARMOR_LINE_INPUT-1) / ARMOR_LINE_INPUT;

		/* Add already-output lines (> PGPVERSION_2_6) */
		total += context->lineno;

		/* Convert to parts */
		total = (total + context->armorlines-1) / context->armorlines;

		/* Add already-emitted parts */
		total += context->thispart - (context->thispart != 0);

		context->maxparts = (unsigned)total;
	}
	if (bytes)
		return 0;	/* Stop futzing with non-zero sizeAdvises */

	/* The real end - this is it */
	context->at_eof = 1;

	/* Flush out any buffered data */
	error = armorFlushFifo (myself, (size_t)-1);
	if (error)
		return error;

	/* Force a write out of last partial line. */
	error = armorWritePartial (myself);
	if (error)
		return error;

	if (context->clearsign) {
		/* Clear out the clearsign buffer, if we have it. */
		if (context->linebuf) {
			pgpFifoWrite (context->fd, context->header,
			              context->input, context->linebuf);
			context->linebuf = 0;
		}
		/*
		 * If clearsign input ended with '\r', will be hashed with
		 * '\r\n' but the output file will be missing the '\n'.
		 * Add it here.
		 */
		if (context->state == 2) {
			pgpFifoWrite (context->fd, context->header,
				      (const unsigned char *) "\n", 1);
			context->state = 0;
		}
	} else {
		if (!context->didfooter)
			armorMakeFooter (context);

		error = armorFlushPending (myself);
		if (error)
			return error;
	}

	/* Finally, the downstream SizeAdvise */
	return context->tail->sizeAdvise (context->tail, 0);
}

/* Push anything available through */
static int
Flush (struct PgpPipeline *myself)
{
	struct Context *context = (struct Context *)myself->priv;
	int error;

	error = armorFlushPending (myself);
	if (error)
		return error;
	return context->tail->flush (context->tail);
}

/* Straightforward deallocate & destroy */
static void
Teardown (struct PgpPipeline *myself)
{
	struct Context *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == ARMORMAGIC);

	context = (struct Context *)myself->priv;
	pgpAssert (context);

	if (context->tail)
		context->tail->teardown (context->tail);

	pgpFifoDestroy (context->fd, context->fifo);
	pgpFifoDestroy (context->fd, context->header);
	if (context->messageid)
		pgpMemFree (context->messageid);
	memset (context, 0, sizeof (*context));
	pgpMemFree (context);
	memset (myself, 0, sizeof (*myself));
	pgpMemFree (myself);
}

/*
 * Add a "Hash: foo, bar, baz" header to the given FIFO if needed.
 * it is not needed if there are no hashes, or there is only MD5.
 */
static void
armorWriteHashHeader (struct PgpFifoDesc const *fd,
                      struct PgpFifoContext *header,
                      byte const *hashlist, unsigned hashlen)
{
	int dohash = 0;
	struct PgpHash const *hash;

	if (!hashlen || (hashlen == 1 && *hashlist == PGP_HASH_MD5))
		return;
	pgpFifoWrite (fd, header, (byte const *)"\nHash: ", 7);
	while (hashlen--) {
		hash = pgpHashByNumber (*hashlist++);
		if (!hash)
			continue;
		if (dohash)
			pgpFifoWrite (fd, header, (byte const *)", ", 2);
		pgpFifoWrite (fd, header,
			      (byte const *)hash->name, strlen (hash->name));
		dohash = 1;
	}
}

/*
 * The slightly different types of ASCII armoring that we do.
 */
#define ATYPE_NORMAL	0
#define ATYPE_CLEARSIG	10
#define ATYPE_SEPSIG	20
#define ATYPE_SEPSIGMSG	21
#define ATYPE_MIMESIG	30
#define ATYPE_MIMEENC	40
#define ATYPE_MIMESEPSIG 50

/*
 * The all-purpose ASCII armor pipeline module creator.
 */
static struct PgpPipeline **
armorDoCreate (struct PgpPipeline **head, struct PgpEnv const *env,
	       PgpVersion version, struct PgpFifoDesc const *fd,
	       struct PgpRandomContext const *rc, unsigned long armorlines,
	       int armortype, byte const *hashlist, unsigned hashlen)
{
	struct PgpPipeline *mod;
	struct Context *context;
	struct PgpFifoContext *fifo;
	struct PgpFifoContext *header;
	char *mid;
	byte message[24];	/* Messageid (binary) */
	int len;

	if (!head)
		return NULL;

	pgpAssert (fd);

	context = (struct Context *)pgpMemAlloc (sizeof (*context));
	if (!context)
		return NULL;
	mod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*mod));
	if (!mod) {
		pgpMemFree (context);
		return NULL;
	}
	fifo = pgpFifoCreate (fd);
	if (!fifo) {
		pgpMemFree (context);
		pgpMemFree (mod);
		return NULL;
	}
	header = pgpFifoCreate (fd);
	if (!header) {
		pgpFifoDestroy (fd, fifo);
		pgpMemFree (context);
		pgpMemFree (mod);
		return NULL;
	}
	if (rc) {
		mid = (char *)pgpMemAlloc ((sizeof(message)+3)*4/3 + 1);
		if (!mid) {
			pgpFifoDestroy (fd, fifo);
			pgpFifoDestroy (fd, header);
			pgpMemFree (context);
			pgpMemFree (mod);
			return NULL;
		}
		/* Create a random message id */
		pgpRandomGetBytes (rc, message, sizeof (message));
		len = armorLine (message, sizeof(message), mid);
		mid[len] = '\0';
	} else {
		mid = NULL;
	}

	mod->magic = ARMORMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "ASCII Armor Write Module";
	mod->priv = context;

	memset (context, 0, sizeof (*context));
	context->outptr = context->output;
	context->armorlines = armorlines;
	context->maxparts = armorlines ? 0 : 1;	/* 0 = unknown */
	context->crc = CRC_INIT;
	context->fd = fd;
	context->fifo = fifo;
	context->header = header;
	context->messageid = mid;
	context->comment = pgpenvGetString (env, PGPENV_COMMENT, NULL, NULL);
	context->version = version;

	/* The armortype-dependent whatnot */
	if (armortype == ATYPE_CLEARSIG) {
		static char const CLRSIGN[] =
			"-----BEGIN PGP SIGNED MESSAGE-----";

		pgpFifoWrite (fd, header, (byte const *)CLRSIGN,
			      sizeof(CLRSIGN)-1);
		context->clearsign = 1;
		context->armorlines = 0;

		armorWriteHashHeader (fd, header, hashlist, hashlen);
		pgpFifoWrite (fd, header, (byte const *)"\n\n", 2);
	} else if (armortype == ATYPE_MIMESIG) {
		pgpFifoWrite (fd, header, (byte const *)MIMETOPHDR,
			      strlen(MIMETOPHDR));
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
	}

	context->tail = *head;
	*head = mod;
	return &context->tail;
}

/*
 * Create a generic ASCII armor output filter.
 */
struct PgpPipeline **
pgpArmorWriteCreate (struct PgpPipeline **head, struct PgpEnv const *env,
		     struct PgpFifoDesc const *fd,
		     struct PgpRandomContext const *rc, PgpVersion version,
		     byte armortype)
{
	int type = 0;
	unsigned long armorlines;
	int error;

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


	return (armorDoCreate (head, env, version, fd, rc, armorlines,
			       type, NULL, 0));
}

/*
 * Create a clearsigned armored output.  This takes two tail pointers,
 * the text being signed (which will be lightly quoted on output) and
 * the signature (which will be ASCII-armored).  We build an ASCII armor
 * pipeline module for each (with different settings), and join them together.
 */
struct PgpPipeline **
pgpArmorWriteCreateClearsig (struct PgpPipeline **texthead,
			     struct PgpPipeline **signhead,
			     struct PgpEnv const *env,
			     struct PgpFifoDesc const *fd,
			     PgpVersion version, byte *hashlist,
			     unsigned hashlen)
{
	struct PgpPipeline *txthead = NULL, *sighead = NULL;
	struct PgpPipeline **joinhead, **sigtail, **tail;
	struct Context *context;

	if (!texthead || !signhead)
		return NULL;

	joinhead = armorDoCreate (&txthead, env, version, fd, NULL, 0,
				  ATYPE_CLEARSIG, hashlist, hashlen);
	if (!joinhead)
		return NULL;

	tail = pgpJoinCreate (joinhead, fd);
	if (!tail) {
		txthead->teardown (txthead);
		return NULL;
	}

	sigtail = armorDoCreate (&sighead, env, version, fd, NULL, 0,
				 ATYPE_SEPSIG, NULL, 0);
	if (!sigtail) {
		txthead->teardown (txthead);
		return NULL;
	}

	/* Add the charset used when generating the clearsigned message */
	context = (struct Context *)sighead->priv;
	pgpAssert (context);
	context->charset = pgpenvGetString (env, PGPENV_CHARSET, NULL, NULL);

	*sigtail = pgpJoinAppend (*joinhead);
	if (!*sigtail) {
		txthead->teardown (txthead);
		sighead->teardown (sighead);
		return NULL;
	}

	pgpJoinBuffer (*sigtail, (byte *)"\n", 1);

	*texthead = txthead;
	*signhead = sighead;
	return tail;
}

/*
 * Create a multipart/signed output.  This takes two tail pointers,
 * the text being signed (which will be lightly quoted on output) and
 * the signature (which will be ASCII-armored).  We build an ASCII armor
 * pipeline module for each (with different settings), and join them together.
 */
struct PgpPipeline **
pgpArmorWriteCreatePgpMimesig (struct PgpPipeline **texttail,
			     struct PgpPipeline **signtail,
			     struct PgpEnv const *env,
			     struct PgpFifoDesc const *fd,
			     PgpVersion version, byte *hashlist,
			     unsigned hashlen)
{
	struct PgpPipeline *txthead = NULL, *sighead = NULL;
	struct PgpPipeline **joinhead, **sigtail, **tail;
	struct PgpPipeline **splithead, **splittail;

	if (!texttail || !signtail)
		return NULL;

	splithead = armorDoCreate (&txthead, env, version, fd, NULL, 0,
				  ATYPE_MIMESIG, hashlist, hashlen);
	if (!splithead)
		return NULL;

	/* Create split to take output from clearmime armor module */
	joinhead = pgpSplitCreate(splithead);

	/* Set second split output to go to signing pipeline */
	splittail = pgpSplitAdd (*splithead);
	*splittail = *texttail;

	tail = pgpJoinCreate (joinhead, fd);
	if (!tail) {
		txthead->teardown (txthead);
		return NULL;
	}

	sigtail = armorDoCreate (&sighead, env, version, fd, NULL, 0,
				 ATYPE_MIMESEPSIG, NULL, 0);
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

	pgpJoinBuffer (*joinhead, (byte *)MIMESIGHDR, strlen(MIMESIGHDR));
	if (!hashlen) {
		pgpJoinBuffer (*joinhead, (byte *)MIMEMIC, strlen(MIMEMIC));
		pgpJoinBuffer (*joinhead, (byte *)MIMEDFLTMIC,
				strlen(MIMEDFLTMIC));
	} else {
		while (hashlen--) {
			struct PgpHash const *hash = NULL;
			unsigned i, len;
			char hashname[30]; /* lower case name */

			hash = pgpHashByNumber (*hashlist++);
			pgpAssert(hash);
			len = strlen(hash->name);
			pgpAssert (len < sizeof(hashname));
			pgpJoinBuffer (*joinhead, (byte *)MIMEMIC,
				       strlen(MIMEMIC));
			for (i=0; i<len; ++i)
				hashname[i] = tolower(hash->name[i]);
			pgpJoinBuffer (*joinhead, (byte *)hashname,
				       len);
			pgpJoinBuffer (*joinhead, (byte *)";", 1);
		}
	}
	pgpJoinBuffer (*joinhead, (byte *)MIMETOPBOUND, strlen(MIMETOPBOUND));

	pgpJoinBuffer (*sigtail, (byte *)MIMEMIDBOUND, strlen(MIMEMIDBOUND));

	*texttail = txthead;
	*signtail = sighead;
	return tail;
}
