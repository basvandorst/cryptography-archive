/*
 * pgpRingUI.c -- Various user-interface helpers for working with keyrings
 * on a simple stdio tty interface.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRingUI.c,v 1.6.2.3.2.1 1997/07/14 16:29:56 quark Exp $
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>	/* for strtol() */
#include <string.h>	/* for strchr() */
#include <time.h>	/* for time () */

#include "pgpDebug.h"
#include "pgpErr.h"
#include "pgpTimeDate.h"	/* for pgpDateString */
#include "pgpUserIO.h"
#include "pgpOutput.h"
#include "pgpRingUI.h"
#include "pgpTrstPkt.h"
#include "pgpTrust.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"
#include "pgpPubKey.h"
#include "pgpSigSpec.h"
#include "pgpTimeDate.h"
#include "pgpEnv.h"
#include "pgpMem.h"

/*
 * The old PGP format...
  Type Date keyID/bits User ID
  PUB 1996-03-30 58B96505/2048 F0 4A 30 95 20 17 29 E0 DC A3 3F FB C6 5B 79 3F
  foo
  sig! 1996-03-30 58B96505 foo
  PUB 1996-03-30 E9D1E021/4096 1A E2 EF AA FC CB 59 AC D4 2A 1A 90 E3 DD 4F 45
  bar
  sig! 1996-03-30 E9D1E021 bar
 2 key(s) examined
 */

static char const hexchar[16] = {
 '0','1','2','3','4','5','6','7',
 '8','9','A','B','C','D','E','F'
};

/*
 * Pretty-print a 128-bit key fingerprint into the supplied buffer,
 * in varius forms, depending on the "len" argument:
 *           1111111111222222222233333333334444444444555
 * 01234567890123456789012345678901234567890123456789012
 * 01 23 45 67 89 AB CD EF 01 23 45 67 89 AB CD EF/1024 (53 chars)
 * 01 23 45 67 89 AB CD EF 01 23 45 67 89 AB CD EF/1024	(52 chars)
 * 0123 4567 89AB CDEF 0123 4567 89AB CDEF/1024	(45 chars)
 * 0123 4567 89AB CDEF 0123 4567 89AB CDEF/1024	(44 chars)
 * 01234567 89ABCDEF 01234567 89ABCDEF/1024	(41 chars)
 * 01234567 89ABCDEF 01234567 89ABCDEF/1024	(40 chars)
 * 0123456789ABCDEF 0123456789ABCDEF/1024	(38 chars)
 * 0123456789ABCDEF0123456789ABCDEF/1024	(37 chars)
 * 0123456789ABCDEF0123456789ABCDE/1024
 * 0123456789ABCDEF0123456789ABCD/1024
 * 0123456789ABCDEF0123456789ABC/1024 (etc., truncating as needed)
 * ...
 * /1024
 * /102
 * /10
 * /1
 * /
 *
 * Returns the number of characters actually used, guaranteed to
 * be <= the given len.
 * Does NOT null-terminate the array.
 */
size_t
ringTtyFormatFingerprint16(struct RingSet const *set, union RingObject *key,
			   char *buf, size_t len)
{
	static signed char const limits[16] = {
		-1, 46, 38, 46, 34, 46, 38, 46,
		 32, 46, 38, 46, 34, 46, 38, 46 };
	char *p = buf;
	byte hash[16];
	char bits[7];	/* '/' + 65535 max length + '\0' */
	int i;
	int hlen;
	unsigned keylen;

	keylen = ringKeyBits(set, key);	/* Length of key in bits */
	sprintf(bits, "/%u", keylen);	/* Length of decimal length */
	keylen = (unsigned)strlen(bits);
	if (keylen >= len) {
		memcpy(buf, bits, len);
		return len;
	}
	len -= keylen;

	hlen = ringKeyFingerprint16(set, key, hash);

	for (i = 0; i < hlen && (size_t)(p-buf) != len; i++) {
		/* Print each space iff it's okay to */
		if (len > (unsigned)(int)limits[i])
			*p++ = ' ';
		/* Double space in the middle if appropriate */
		if (i == 8 && len >= 36 && len != 39 && len != 47)
			*p++ = ' ';
		*p++ = hexchar[hash[i] >> 4 & 15];
		if ((size_t)(p-buf) == len)
			break;
		*p++ = hexchar[hash[i] & 15];
	}
	memcpy(p, bits, keylen);
	return p+keylen-buf;
}

/*
 * Pretty-print a 160-bit key fingerprint into the supplied buffer,
 * in varius forms, depending on the "len" argument:
 * Version a:
 *           11111111112222222222333333333344444444
 * 012345678901234567890123456789012345678901234567
 * 01234 56789 ABCDE F0123 45678 9ABCD EF012 34567 (48 chars)
 * 01234 56789 ABCDE F0123 45678 9ABCD EF012 34567	(47 chars)
 * 0123456789 ABCDEF0123 456789ABCD EF01234567	(44 chars)
 * 0123456789 ABCDEF0123 456789ABCD EF01234567	(43 chars)
 * 0123456789ABCDEF0123 456789ABCDEF01234567	(41 chars)
 * 0123456789ABCDEF0123456789ABCDEF01234567	(40 chars)
 * 0123456789ABCDEF0123456789ABCDEF0123456
 * 0123456789ABCDEF0123456789ABCDEF012345
 * 0123456789ABCDEF0123456789ABCDEF01234 (etc., truncating as needed)
 *
 * Version b:
 *           1111111111222222222233333333334444444444
 * 01234567890123456789012345678901234567890123456789
 * 0123 4567 89ab cdef 0123 4567 89ab cdef 0123 4567 (50 chars)
 * 0123 4567 89ab cdef 0123 4567 89ab cdef 0123 4567	(49 chars)
 * 01234567 89abcdef 01234567 89abcdef 01234567	(44 chars)
 * 0123456789abcdef0123 456789abcdef01234567	(41 chars)
 * 0123456789ABCDEF0123456789ABCDEF01234567	(40 chars)
 * 0123456789ABCDEF0123456789ABCDEF0123456
 * 0123456789ABCDEF0123456789ABCDEF012345
 * 0123456789ABCDEF0123456789ABCDEF01234 (etc., truncating as needed)
 */
size_t
ringTtyFormatFingerprint20(struct RingSet const *set, union RingObject *key,
			   char *buf, size_t len)
{
	static signed char const limitsa[40] =
		{-1, 46, 42, 46, 40, 46, 42, 46};
	static signed char const limitsb[10] =
		{-1, 48, 43, 48, 43, 48, 43, 48, 43, 48};
	signed char const *limits = limitsb; /* Choose B */
	int period = 4;		/* Use 5 for A, 4 for B */
	char *p = buf;
	byte hash[20];
	int i;
	int hlen;

	hlen = ringKeyFingerprint20(set, key, hash);
	if (hlen < 0)
		return hlen;

	hlen *= 2;

	/* Unlike the 16 byte version, this loops per character */
	for (i = 0; i < hlen && (size_t)(p-buf) != len; i++) {
		/* Print each space iff it's okay to */
		if (i%period == 0)
			 if (len > (unsigned)(int)limits[i/period])
				*p++ = ' ';
		/* Double space in the middle if appropriate */
		if (i == 20 && len >= (unsigned)(limits==limitsa ? 44 : 50) &&
		len != 47)
		 *p++ = ' ';
		if (i & 1)
			*p++ = hexchar[hash[i/2] & 15];
		else
			*p++ = hexchar[hash[i/2] >> 4 & 15];
	}
	return p-buf;
}

int
ringTtyPutFingerprint16(PgpOutputType OutputType, struct RingSet const *set,
			union RingObject *key, unsigned wid)
{
	char buf[54];		/* Incestuous knowledge */

	if (wid > sizeof(buf))
		wid = sizeof(buf);
	wid = (unsigned)ringTtyFormatFingerprint16(set, key, buf, (size_t)wid);
	SpecifiedOutputString(FALSE, OutputType, 0, buf);
	return(wid);
}

int
ringTtyPutFingerprint20(PgpOutputType OutputType, struct RingSet const *set,
	union RingObject *key, unsigned wid)
{
	char buf[50];		/* Incestuous knowledge */

	if (wid > sizeof(buf))
		wid = sizeof(buf);
	wid = (unsigned)ringTtyFormatFingerprint20(set, key, buf, (size_t)wid);
	SpecifiedOutputString(FALSE, OutputType, 0, buf);

	return(wid);
}

int
ringTtyPutKeyID(Boolean DisplayHeaders,
		PgpOutputType OutputType,
		struct RingSet const *set,
		union RingObject *key)
{
	byte buf[8];
	int i;

	if(DisplayHeaders)
	    SpecifiedOutputString(DisplayHeaders,
				  OutputType,
				  0,
				  "");

	ringKeyID8(set, key, NULL, buf);
	for (i = 4; i < 8; i++) {
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%c%c",
				  hexchar[buf[i] >> 4 & 15],
				  hexchar[buf[i] & 15]);
	}
	return 8;
}

int
ringTtyPutSigID(Boolean DisplayHeaders,
		PgpOutputType OutputType,
		struct RingSet const *set,
		union RingObject *sig)
{
    byte buf[8];
    int i;

    if(DisplayHeaders)
	SpecifiedOutputString(TRUE, OutputType, 0, "");

    ringSigID8(set, sig, NULL, buf);
    for (i = 4; i < 8; i++) {
	SpecifiedOutputString(FALSE,
			      OutputType,
			      0,
			      "%c%c",
			      hexchar[buf[i] >> 4 & 15],
			      hexchar[buf[i] & 15]);
    }
    return 8;
}



/*
 * Write out the given string with all funny characters \-escaped in
 * the manner of C strings, up to "maxlen" characters. Returns the
 * actual number of characters printed, which will always be <= maxlen.
 * If f is NULL, prints nothing. (May be useful for justification
 * computations.) The string is surrounded by quotes q1 and q2
 * (if not '\0'). q2 is also \-escaped if it appears in the string.
 */
unsigned
ringTtyPutString(char const *str,
		 size_t len,
		 unsigned maxlen,
		 Boolean DisplayHeaders,
		 PgpOutputType OutputType,
                 char q1,
		 char q2)
{
    int c = 0;
    size_t t;
    char const *p;
    unsigned remaining = maxlen;
    static char const escapes[] = "\a\b\f\n\r\t\v";
    static char const letters[] = {'a','b','f','n','r','t','v'};

    /* Opening quote */
    if(remaining) {
	if (q1) {
	    SpecifiedOutputString(DisplayHeaders, OutputType, 0, "%c", q1);
	    remaining--;
	}
	else {
	    if(DisplayHeaders)
		/*Display the headers, if desired:*/
		SpecifiedOutputString(TRUE, OutputType, 0, "");
	}
    }

    for (;;) {
	/* Printing can only expand the string, so truncate it */
	if (len > remaining)
	    len = remaining;

	/* Find a directly printable substring */
	p = str;	/* Remember start of substring */
	while (len) {
	    c = (unsigned char)*str;
	    if (!isprint(c) || c == '\\' || c == q2)
		break;
	    str++;
	    len--;
	}

	/* Print from p up to str */
	t = (size_t)(str - p);
	if (t) {
	    char *tmp;
	    if((tmp = pgpAlloc(sizeof(char) * (t + 1)))) {
		memcpy(tmp, p, t);
		*(tmp + t) = '\0';
		SpecifiedOutputString(FALSE, OutputType, 0, tmp);
		pgpFree(tmp);
	    }
	    remaining -= t;
	}

	/* Done with the string? */
	if (!len)
	    break;
	/* Note that remaining >= len > 0, so remaining > 0. */

	/*
	 * c is a character to print that needs escaping.
	 * Now we're going to print it, so remove it from the string.
	 */
	len--;
	str++;

	/* Start with the obligatory backslash */
	SpecifiedOutputString(FALSE, OutputType, 0, "\\");
	if (!--remaining)
	    break;

	/* Simple case 1: escaping printable characters */
	if (isprint(c)) {
	    SpecifiedOutputString(FALSE, OutputType, 0, "%c", c);
	    --remaining;
	    continue;
	}

	/* Simple case 2: standard C escape */
	p = strchr(escapes, c);
	if (p && c) {
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%c",
				  letters[p - escapes]);
	    --remaining;
	    continue;
	}

	/* General octal escapes */
	/* If next char makes it ambiguous, force 3-char escape */
	if (len && isdigit(*str)) /* Force on 8 and 9, too! */
	    c += 256;
	if (c > 077) {
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%c",
				  '0' + (c>>6 & 3));

	    if (!--remaining)
		break;
	}
	if (c > 07) {	
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%c",
				  '0' + (c>>3 & 7));
	    if (!--remaining)
		break;
	}

	SpecifiedOutputString(FALSE, OutputType, 0, "%c", '0' + (c & 7));

	--remaining;
    }

    /* Closing quote (if string ended) */
    if (q2 && remaining) {
	SpecifiedOutputString(FALSE, OutputType, 0, "%c", q2);
	remaining--;
    }

    return maxlen - remaining;
}


/*
 * Print info about a key, in the format
 *
 *   1024 bits, Key ID FBBB8AB1, created 1984-12-25
 *
 * The ", created" part is omitted if the creation timestamp is 0.
 */
void
ringTtyPutKeyInfo(Boolean DisplayHeader,
		  PgpOutputType OutputType,
		  struct RingSet const *set,
		  union RingObject *obj)
{
	char buf[PGPDATESTRINGLEN+1];
	word32 tstamp;

	SpecifiedOutput(DisplayHeader,
			OutputType,
			0,
			"BITS_AND_KEYID",
			(unsigned)ringKeyBits(set, obj));

	ringTtyPutKeyID(FALSE,
			OutputType,
			set,
			obj);

	tstamp = ringKeyCreation(set, obj);
	if (tstamp) {
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  ", Created ");
		pgpDateString(tstamp, buf);
		SpecifiedOutputString(FALSE,
				      OutputType,
				      0,
				      buf);
	}

	SpecifiedOutputString(FALSE,
			      OutputType,
			      0,
			      "\n");
#if 0
	if (trust & PGP_KEYTRUSTF_REVOKED)
		fprintf(f, "%sThis key has been revoked by its owner\n",
		        prefix);
#endif
}


void ringKeyIDprint(Boolean DisplayHeaders,
		    PgpOutputType OutputType,
		    char const *prompt,
		    byte const keyID[8])
{
	char buf[18];
	char *p = buf;
	int i;

	for (i = 0; i < 4; i++) {
		*p++ = hexchar[keyID[i] >> 4];
		*p++ = hexchar[keyID[i] & 15];
	}
	*p++ = ' ';
	for (i = 4; i < 8; i++) {
		*p++ = hexchar[keyID[i] >> 4];
		*p++ = hexchar[keyID[i] & 15];
	}
	*p = '\n';

	SpecifiedOutputString(DisplayHeaders, OutputType, 0, prompt);
	SpecifiedOutputBuffer(FALSE, OutputType, 0, buf, sizeof(char), 18);
}

void
ringKeyPrint(PgpOutputType OutputType,
	     struct RingSet const *set,
	     union RingObject *obj,
	     int level)
{
	struct RingIterator *iter;
	char const *name;
	size_t len;
	int i;

	ringTtyPutKeyInfo(TRUE, OutputType, set, obj);
	iter = ringIterCreate(set);
	if (iter) {
		i = ringIterSeekTo(iter, obj);
		while (ringIterNextObject(iter, i+1) > 0) {
			obj = ringIterCurrentObject(iter, i+1);
			pgpAssert(obj);
			if (ringObjectType(obj) != RINGTYPE_NAME)
				continue;
			name = ringNameName(set, obj, &len);
			SpecifiedOutputString(TRUE,
					      OutputType,
					      0,
					      "%*s",
					      level+2,
					      "");
			(void)ringTtyPutString(name,
					       len,
					       -1u,
					       FALSE,
					       OutputType,
					       '"',
					       '"');
			SpecifiedOutputString(FALSE, OutputType, 0, "\n");
			/*	break; */
		}
		ringIterDestroy(iter);
	}
}


void
ringObjPrint(PgpOutputType OutputType,
	     struct RingSet const *set,
	     union RingObject *obj,
	     int level)
{
	int i = ringObjectType(obj);
	int err;
	char const *name;
	size_t len;
	char buf[PGPDATESTRINGLEN+1];

	switch (i) {
	    case RINGTYPE_KEY:
		err = ringKeyError(set, obj);
		SpecifiedOutputString(TRUE,
				      OutputType,
				      0,
				      "%*sKey: error=%d ",
				      level, "", err);
		ringTtyPutKeyInfo(TRUE, OutputType, set, obj);
		SpecifiedOutputString(TRUE,
				      OutputType,
				      0,
				      "%*s     fingerprint16=",
				      level, "");
		ringTtyPutFingerprint16(OutputType, set, obj, 100);
		SpecifiedOutputString(FALSE, OutputType, 0, "\n");
		SpecifiedOutputString(TRUE,
				      OutputType,
				      0,
				      "%*s     fingerprint20=",
				      level, "");
		ringTtyPutFingerprint20(OutputType, set, obj, 100);
		SpecifiedOutputString(FALSE, OutputType, 0, "\n");
		break;

	    case RINGTYPE_SEC:
		SpecifiedOutputString(TRUE,
				      OutputType,
				      0,
				      "%*sSecret: (not much to say)\n",
				      level, "");
		break;

	    case RINGTYPE_NAME:
		name = ringNameName(set, obj, &len);
		SpecifiedOutputString(TRUE,
				      OutputType,
				      0,
				      "%*sName: ",
				      level, "");
		(void)ringTtyPutString(name,
				       len,
				       -1u,
				       FALSE,
				       OutputType,
				       '"',
				       '"');
		SpecifiedOutputString(FALSE, OutputType, 0, "\n");
		break;

	    case RINGTYPE_SIG:
		err = ringSigError(set, obj);
		SpecifiedOutputString(TRUE,
				      OutputType,
				      0,
				      "%*sSig: error=%d ",
				      level, "", err);
		ringTtyPutSigID(FALSE, OutputType, set, obj);
		SpecifiedOutputString(FALSE, OutputType, 0, " ");
		pgpDateString(ringSigTimestamp(set, obj), buf);
		SpecifiedOutputString(FALSE,
				      OutputType,
				      0,
				      "%s, type=%x",
				      buf,
				      ringSigType(set, obj));

		obj = ringSigMaker(set, obj, set);
		if (!obj) {
		    SpecifiedOutputString(FALSE, OutputType, 0, "\n");
		} else {
		    SpecifiedOutputString(FALSE,
					  OutputType,
					  0,
					  " made by:\n");
		    ringKeyPrint(OutputType, set, obj, level);
		    ringObjectRelease(obj);
		}
		break;
	    default:
		SpecifiedOutputString(TRUE,
				      OutputType,
				      0,
				      "%*s### Unknown object: type %d\n",
				      level,
				      "",
				      i);
		break;
	}
}

static void
ringObjPrintAll(PgpOutputType OutputType,
		struct RingSet const *set,
		union RingObject *obj,
		int level)
{
	struct RingIterator *iter;
	int i, j;

	iter = ringIterCreate(set);
	if (iter) {
		i = ringIterSeekTo(iter, obj);
		for (j = 1; j < i; j++) {
			obj = ringIterCurrentObject(iter, j);
			ringObjPrint(OutputType, set, obj, level+2*(j-1));
		}
		ringIterDestroy(iter);
	} else {
	    ringObjPrint(OutputType, set, obj, level);
	}
}

#if 0
* - PGPERR_KEYIO_READING
* - PGPERR_KEYIO_EOF
* - PGPERR_KEYIO_BADPKT
* - PGPERR_NOMEM
*/
#define PGPERR_TROUBLE_BADTRUST	-320
PGPERR (PGPERR_TROUBLE_BADTRUST,	"Trust packet malformed")
#define PGPERR_TROUBLE_UNKPKTBYTE -321
PGPERR (PGPERR_TROUBLE_UNKPKTBYTE,	"Unknown packet byte in keyring")
#define PGPERR_TROUBLE_UNXNAME	-322
PGPERR (PGPERR_TROUBLE_UNXNAME,		"Unexpected name (before key)")
#define PGPERR_TROUBLE_UNXSIG 	-323
PGPERR (PGPERR_TROUBLE_UNXSIG,	 	"Unexpected sig (before key)")
#define PGPERR_TROUBLE_UNXTRUST	-324
PGPERR (PGPERR_TROUBLE_UNXTRUST,	"Unexpected trust packet")
#define PGPERR_TROUBLE_KEY2BIG	-325
PGPERR (PGPERR_TROUBLE_KEY2BIG,		"Key too damn big")
#define PGPERR_TROUBLE_NAME2BIG	-326
PGPERR (PGPERR_TROUBLE_NAME2BIG,	"Name too damn big")
#define PGPERR_TROUBLE_SIG2BIG	-327
PGPERR (PGPERR_TROUBLE_SIG2BIG,		"Sig too damn big")
#define PGPERR_TROUBLE_DUPKEYID	-328
PGPERR (PGPERR_TROUBLE_DUPKEYID, 	"Duplicate KeyID, different keys")
#define PGPERR_TROUBLE_DUPKEY	-329
PGPERR (PGPERR_TROUBLE_DUPKEY, 		"Duplicate key (within keyring)")
#define PGPERR_TROUBLE_DUPNAME	-330
PGPERR (PGPERR_TROUBLE_DUPNAME,		"Duplicate name (in same keyring)")
#define PGPERR_TROUBLE_DUPSIG	-331
PGPERR (PGPERR_TROUBLE_DUPSIG,		"Duplicate signature")
#define PGPERR_TROUBLE_BAREKEY	-332
PGPERR (PGPERR_TROUBLE_BAREKEY,		"Key found with no names")
#define PGPERR_TROUBLE_VERSION_BUG_PREV	-333
PGPERR (PGPERR_TROUBLE_VERSION_BUG_PREV, "Bug introduced by legal_kludge")
#define PGPERR_TROUBLE_VERSION_BUG_CUR	-334
PGPERR (PGPERR_TROUBLE_VERSION_BUG_CUR,	"Bug introduced by legal_kludge")
#endif
/*
 * Print a single trouble message
 * A message with extra lines of information (such as a key) is followed
 * by a blank line. This is a feature, for clarity, not an accident of
 * the way it is implemented.
 */
#if 0
struct RingTrouble const *next;
union RingObject *obj;	/* The pertinent object, if applicable */
word32 num;	/* An integer parameter, if applicable */
word32 fpos;	/* File position related to the error */
int type;	/* PGPERR_ code from pgpErr.h */
#endif

static void
ringTopDoPrintTrouble(PgpOutputType OutputType, struct RingFile *file,
		struct RingTrouble const *t)
{
#if 0
    fprintf(f, "%7lu: ", (unsigned long)t->fpos);
#endif

    switch(t->type) {
	/* Non-fatal errors */
	case PGPERR_TROUBLE_BADTRUST: /* Trust packet malformed */
	    SpecifiedOutput(OutputType,
			    TRUE,
			    0,
			    "PGPERR_TROUBLE_BADTRUST_LONG",
			    (unsigned long) t->num);
	    break;
	case PGPERR_TROUBLE_UNKPKTBYTE:	/* Unknown packet byte */
	    SpecifiedOutput(OutputType,
			    TRUE,
			    0,
			    "PGPERR_TROUBLE_UNKPKTBYTE_LONG",
			    (unsigned long) t->num);
	    break;
	case PGPERR_TROUBLE_KEY2BIG: /* Key too damn big */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_TROUBLE_KEY2BIG_LONG",
			    (unsigned long) t->num);
	    break;
	case PGPERR_TROUBLE_NAME2BIG: /* Uid too damn big */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_TROUBLE_NAME2BIG_LONG",
			    (unsigned long) t->num);
	    break;
	case PGPERR_TROUBLE_SIG2BIG: /* Sig too damn big */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_TROUBLE_SIG2BIG_LONG",
			    (unsigned long) t->num);
	    break;
	case PGPERR_TROUBLE_DUPKEYID: /* Duplicate KeyID, different keys */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_TROUBLE_DUPKEYID_LONG");
	    ringKeyPrint(OutputType, ringFileSet(file), t->obj, 2);
	    break;
	case PGPERR_TROUBLE_DUPKEY: /* Duplicate key (within keyring) */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_TROUBLE_DUPKEY_LONG");
	    ringKeyPrint(OutputType, ringFileSet(file), t->obj, 2);
	    break;
	case PGPERR_TROUBLE_DUPNAME: /* Duplicate name (within keyring) */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_TROUBLE_DUPNAME_LONG");
	    ringObjPrintAll(OutputType, ringFileSet(file), t->obj, 2);
	    break;
	case PGPERR_TROUBLE_BAREKEY: /* Duplicate key (within keyring) */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_TROUBLE_BAREKEY_LONG");
	    ringKeyPrint(OutputType, ringFileSet(file), t->obj, 2);
	    break;
	
	case PGPERR_TROUBLE_VERSION_BUG_CUR: /* legal_kludge bug */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_TROUBLE_VERSION_BUG_CUR_LONG");
		/* Obj is "sec" */
		ringKeyPrint(OutputType, ringFileSet(file), t->obj, 2);
		break;
	case PGPERR_TROUBLE_VERSION_BUG_PREV: /* legal_kludge bug */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_TROUBLE_VERSION_BUG_PREV_LONG");
	    ringKeyPrint(OutputType, ringFileSet(file), t->obj, 2);
	    break;

		/* Fatal errors */
	case PGPERR_KEYIO_READING: /* Read error */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_KEYIO_READING_LONG",
			    strerror((int)t->num));
		break;
	case PGPERR_KEYIO_FTELL: /* Read error */
 	    SpecifiedOutput(OutputType,
	 		    TRUE,
		 	    0,
			    "PGPERR_KEYIO_FTELL_LONG",
			    strerror((int)t->num));
		break;
	default:
	    PGPErrCodeOutput(TRUE, LEVEL_SEVERE, t->type);
	
	    ringObjPrintAll(OUTPUT_ERROR, ringFileSet(file), t->obj, 2);
	    break;
    }
}

/*
 * List any trouble spots encountered opening a file.
 * Ask the user if they wish to continue.  If not,
 * return an error code <0.
 *
 * Prints nothing if there is no trouble.
 */
int
ringTopListTrouble(Boolean DisplayOutput,
		   PgpOutputType OutputType,
		   struct RingFile *file,
		   int writeflag)
{
    struct RingTrouble const *trouble;
    unsigned u = 0;

    (void) writeflag;

    trouble = ringFileTrouble(file);
    if (!trouble)
	return 0;

    /*
     * Special case: if first packet in the file isn't a packet,
     * it's unconditionally an error.
     */
    if (trouble->fpos == 0 && trouble->type == PGPERR_KEYIO_BADPKT) {
	SpecifiedOutput(TRUE, OutputType, 0, "NOT_PGP_KEYFILE");
	ringFilePurgeTrouble(file);
	return PGPERR_KEYIO_BADPKT;
    }

    SpecifiedOutput(TRUE, OutputType, 0, "FOLLOWING_KEYRING_PROBLEMS");
    SpecifiedOutput(TRUE, OutputType, 0, "OFFSET_DESCRIPTION");

    do {
	u++;
	ringTopDoPrintTrouble(OutputType, file, trouble);
    } while ((trouble = trouble->next) != NULL);

    if (u > 20)
	SpecifiedOutput(TRUE, OutputType, 0, "OFFSET_DESCRIPTION");

#if 1
	u = 1;
#else
	if (writeflag) {
		 u = userAsk("\n\
If you continue, PGP will ignore the errors and remove the\n\
offending data from the key file.  Do you want to continue with\n\
\"%s\"", 'y', ring->ring[bit].logname);
	 } else {
		u = userAsk("\n\
If you continue, PGP will ignore the errors.  Do you want to continue with\n\
\"%s\"", 'y', ring->ring[bit].logname);
		}
#endif

#if 0
	/*
	 * KLUDGE Version bug kludge: if told to correct, flags the
	 * offending keys appropriately.
	 */
	if (u) {
		trouble = ringFileTrouble(file);
		do {
			if (trouble->type == TROUBLE_VERSION_BUG_SEC ||
			    trouble->type == TROUBLE_VERSION_BUG_PUB) {
				trouble->obj->g.flags |= SECF_VERSION_BUG;
				ringPoolMarkDirty(ring,
					          trouble->obj->g.mask);
			}
		} while ((trouble=trouble->next) != NULL);
	}
#endif

	ringFilePurgeTrouble(file);

	return u ? 0 : -1;
}



/* Functions for displaying keys */


const int name_indent = 30;
const int sig_indent = 32;

/* Given a key object, return the first Name object attached to it. */

static union RingObject *
getFirstName (struct RingSet const *set, union RingObject *key)
{
	int				status;
	struct RingIterator		*iter = ringIterCreate (set);
	union RingObject		*object;

	pgpAssert (ringObjectType (key) == RINGTYPE_KEY);

	ringIterSeekTo (iter, key);
	while ((status = ringIterNextObject (iter, 2)) > 0) {
		object = ringIterCurrentObject (iter, 2);
		pgpAssert (object != NULL);
		if (ringObjectType (object) == RINGTYPE_NAME) {
			ringIterDestroy (iter);
			return object;
		}
	}
	ringIterDestroy (iter);
	return NULL;
}



/* Passed the keyid of a signature, its level, and its parent object,
   determine if the signature has been retired. */

static int
sigRetired (struct RingSet const *set, union RingObject *sig)
{
  struct RingIterator	*iter = ringIterCreate (set);
  union RingObject	*object;
  int 			status;
  int			level;

  level = ringIterSeekTo (iter, sig);
  if (level < 0)
    return level;
  ringIterRewind(iter, level);

  while ((status = ringIterNextObject (iter, level)) > 0) {
    object = ringIterCurrentObject (iter, level);
    pgpAssert (object != NULL);
    if (ringObjectType (object) == RINGTYPE_SIG) {
      if (ringSigError (set, object) == 0 &&
	  ringSigType (set, object) == PGP_SIGTYPE_KEY_UID_REVOKE &&
	  ringSigChecked (set, object) &&
	  ringSigMaker (set, object, set) == ringSigMaker (set, sig, set)) {
	ringIterDestroy (iter);
	return 1;
      }
    }
  }
  ringIterDestroy (iter);
  return 0;
}

/* List of algorithms indexed by the pkalg byte. Each entry contains
   the name of the algorithm and what it can be used for. */

static const char *pkalg_list[] =
{NULL, "RSA", "RSA", "RSA", NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, "Diffie-Hellman", "DSS"};

static const char *
getKeyAlg (union RingObject *key, struct RingSet const *set)
{
	byte pkalg;

	ringKeyID8 (set, key, &pkalg, NULL);
	return pkalg_list[pkalg];
}

static const char *keyuse_list[] =
{NULL, "Sign only", "Encrypt only", "Sign and Encrypt"};

static const char *
getKeyUse (union RingObject *key, struct RingSet const *set)
{
	if (ringKeySubkey(set, key))
		return keyuse_list[PGP_PKUSE_SIGN];
	return keyuse_list[ringKeyUse (set, key) & PGP_PKUSE_SIGN_ENCRYPT];
}

static char statement[11];
static char *
getTrustStatement (word16 trust)
{
	int d = trust - PGP_TRUST_DECADE - PGP_TRUST_OCTAVE;
	int i;
	unsigned long l;
	int wid = 9;
	char number[10];

	if (trust == PGP_TRUST_INFINITE)
		strcpy (statement, "  maximum ");
	else if (trust == 0)
		strcpy (statement, "     none ");
	else {
		if (d < 0)
			d = 0;

		/*
		 * Note: The following was taken from Colin's
		 * prettyprint routine.
		 */

		d -= d % PGP_TRUST_DECADE;
		i = d / PGP_TRUST_DECADE;
		l = ringTrustToInt(trust - d);
		if (i)
			wid -= sprintf(number, "%lu%0*u ", l, i, 0);
		else
			wid -= sprintf(number, "%lu ", l);
		sprintf (statement, "%*s%s", wid, "", number);
	}
	return statement;
}


/* This function displays a single signature. */
int
ringTtyShowSig(union RingObject *sig,
	       struct RingSet const *set,
	       Boolean DisplayHeader,
	       PgpOutputType OutputType,
	       int check)
{
	union RingObject	*sigkey, *signame;
	word32			creation;
	char			datestring[11];
	char const		*namestring;
	size_t			len;
	byte			trust;

	sigkey = ringSigMaker (set, sig, set);

	/* Output "sig" followed by key bits and key id. */
   		   	
	trust = ringSigTrust (set, sig);

	if (ringSigType (set, sig) == PGP_SIGTYPE_KEY_UID_REVOKE)
	    SpecifiedOutputString(DisplayHeader, OutputType, 0, "ret");
	else
	    SpecifiedOutputString(DisplayHeader, OutputType, 0, "sig");

	if (ringSigError (set, sig) < 0)
	    SpecifiedOutputString(FALSE, OutputType, 0, "%%");
	else if (!sigkey)
	    SpecifiedOutputString(FALSE, OutputType, 0, "?");
	else if (check && (ringSigChecked(set, sig)))
	    SpecifiedOutputString(FALSE, OutputType, 0, "!");
	else if (check && (ringSigTried(set, sig)))
	    SpecifiedOutputString(FALSE, OutputType, 0, "*");
	else
	    SpecifiedOutputString(FALSE, OutputType, 0, " ");

	SpecifiedOutputString(FALSE, OutputType, 0, "      ");
	ringTtyPutSigID (FALSE, OutputType, set, sig);
	SpecifiedOutputString(FALSE, OutputType, 0, " ");

	/* Signature date */

	creation = ringSigTimestamp (set, sig);
	if (creation > 0) {
	    pgpDateString (creation, datestring);
	    SpecifiedOutputString(FALSE, OutputType, 0, datestring);
	}
	else
	    SpecifiedOutputString(FALSE, OutputType, 0, "----------");

	if (sigkey) {
	    signame = getFirstName (set, sigkey);
	    if (signame) {
		SpecifiedOutputString(FALSE, OutputType, 0, "   ");
		namestring = ringNameName (set, signame, &len);
		ringTtyPutString (namestring,
				  len,
				  (unsigned) len,
				  FALSE,
				  OutputType,
				  0,
				  0);
		}
	    SpecifiedOutputString(FALSE, OutputType, 0, "\n");
	}
	else
	    SpecifiedOutput(FALSE, OutputType, 0, "UNKNOWN_SIGNATOR");

	ringObjectRelease (sigkey);
	return 0;
}

/* This function displays a set of signatures attached to a name. */

int
ringTtyShowSigs (Boolean DisplayHeaders,
		 PgpOutputType OutputType,
		 union RingObject *name,
		 struct RingSet const *set,
		 int mode)
{
	struct RingIterator	*iter = ringIterCreate (set);
	int	 	         status;
	union RingObject	*sig;
	int			 check;

	pgpAssert (name != NULL);
	pgpAssert (set != NULL);
	pgpAssert (ringObjectType (name) == RINGTYPE_NAME);

	check = (mode == 3 || mode == 4);

	/* Display sigs attached to the Name */

	ringIterSeekTo (iter, name);

	while ((status = ringIterNextObject (iter, 3)) > 0) {
	    sig = ringIterCurrentObject (iter, 3);
	    pgpAssert (sig != NULL);
	    ringTtyShowSig(sig, set, DisplayHeaders, OutputType, check);
	}

	ringIterDestroy (iter);
	return 0;
}




int
ringTtyCheckSigs (Boolean DisplayHeaders,
		  PgpOutputType OutputType,
		  union RingObject *name,
		  struct RingSet const *set,
		  int mode)
{
    struct RingIterator	*iter = ringIterCreate (set);
    int			 status;
    union RingObject	*sig, *sigkey, *signame;
    size_t		    len;
    char const      *output;
    char const		*namestring;
    byte			 sigtrust;
    unsigned         confidence;
    PgpTrustModel    pgptrustmodel;

    pgpAssert (name != NULL);
    pgpAssert (set != NULL);
    pgpAssert (mode == 4 || mode == 5);
    pgpAssert (ringObjectType (name) == RINGTYPE_NAME);

    pgptrustmodel = pgpTrustModel (ringSetPool (set));

    /* Display sigs attached to the Name */

    ringIterSeekTo (iter, name);

    while ((status = ringIterNextObject (iter, 3)) > 0) {
	sig = ringIterCurrentObject (iter, 3);
	pgpAssert (sig != NULL);
	if (ringSigType (set, sig) == PGP_SIGTYPE_KEY_UID_REVOKE)
	    continue;
	sigkey = ringSigMaker (set, sig, set);
	SpecifiedOutputString(DisplayHeaders,
			      OutputType,
			      0,
			      "%*s",
			      11, "");
	if (pgptrustmodel == PGPTRUST0) {
	    if (!sigkey || sigkey ==
		ringIterCurrentObject (iter, 1))
		sigtrust = PGP_SIGTRUST_NOKEY;
	    else
		if (sigRetired (set, sig))
		    sigtrust = PGP_SIGTRUST_RETIRED;
	    else
		if (ringKeyRevoked (set, sigkey))
		    sigtrust = PGP_SIGTRUST_REVOKED;
	    else
		sigtrust = ringSigTrust (set, sig);
	    output = keyTrustTable[sigtrust];
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%-10s%*s",
				  output, 12, "");
	}
	else {				/* new trust model */
	    if (!sigkey || sigkey ==
		ringIterCurrentObject (iter, 1))
		output = " ";
	    else
		if (sigRetired (set, sig))
		    output = "retired ";
	    else
		if (ringKeyRevoked (set, sigkey))
		    output = "revoked ";
		else {
		    confidence = ringSigConfidence (set, sig);
		    output = getTrustStatement (confidence);
		}
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%10s%*s",
				  output, 12, "");
	}
	
	if (sigkey) {
	    signame = getFirstName (set, sigkey);
	    if (signame) {
		namestring = ringNameName (set, signame, &len);
		ringTtyPutString (namestring,
				  len,
				  (unsigned) len,
				  FALSE,
				  OutputType,
				  0,
				  0);
	    }
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "\n");
	}
	else {
	    SpecifiedOutput(FALSE, OutputType, 0, "OPEN_PAREN_KEYID");
	    ringTtyPutSigID (FALSE, OutputType, set, sig);
	    SpecifiedOutputString(FALSE, OutputType, 0, "\n");
	}
	ringObjectRelease (sigkey);
    }

    ringIterDestroy (iter);
    return 0;
}


/*  This function displays a key in -kv, -kvv, -kvc, or the first section
    of a -kc format. */

int
ringTtyShowKey (Boolean DisplayHeaders,
		PgpOutputType OutputType,
		union RingObject *key,
		struct RingSet const *set,
		int mode)
{
    struct RingIterator	*iter = ringIterCreate (set);
    union RingObject	*name;
    union RingObject	*subkey = NULL;
    size_t		 len;
    int			 status;
    word32		 creation, expiration;
    char			 datestring[11];
    char const 		*namestring;

    pgpAssert (key != NULL);
    pgpAssert (set != NULL);
    pgpAssert (mode <= 5);
    pgpAssert (ringObjectType (key) == RINGTYPE_KEY);

    if (ringKeyIsSec (set, key))
	SpecifiedOutputString(DisplayHeaders, OutputType, 0, "sec");
    else
	SpecifiedOutputString(DisplayHeaders, OutputType, 0, "pub");

    if (ringKeyError (set, key) != 0) {
	SpecifiedOutputString(FALSE, OutputType, 0, "? \n");
	/* return, otherwise some strange behaviour occurs */
	return 0;
    }
    else if (ringKeyDisabled(set, key))
	SpecifiedOutputString(FALSE, OutputType, 0, "@ ");
    else
	SpecifiedOutputString(FALSE, OutputType, 0, "  ");

    SpecifiedOutputString(FALSE,
			  OutputType,
			  0,
			  "%4u",
			  (unsigned) ringKeyBits (set, key));
    ringTtyPutKeyID(FALSE, OutputType, set, key);
    SpecifiedOutputString(FALSE, OutputType, 0, " ");
    creation = ringKeyCreation (set, key);
    if (creation > 0) {
	pgpDateString (creation, datestring);
	SpecifiedOutputString(FALSE, OutputType, 0, "%s ", datestring);
    }
    else
	SpecifiedOutputString(FALSE,
			      OutputType,
			      0,
			      "---------- ");

    if (ringKeyRevoked (set, key))
	SpecifiedOutput(FALSE,
			OutputType,
			0,
			"REVOKED");
    else {
	expiration = ringKeyExpiration (set, key);
	if (expiration > 0) {
	    pgpDateString (expiration, datestring);
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%s ",
				  datestring);
	}
	else
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "---------- ");
    }

    SpecifiedOutputString(FALSE,
			  OutputType,
			  0,
			  "%-8s  %-15s\n",
			  getKeyAlg (key, set),
			  getKeyUse (key, set));

    if (mode == 2) {
	byte        pkalg;
	ringKeyID8 (set, key, &pkalg, NULL);
	if (pkalg > PGP_PKALG_RSA_ENC) {
	    SpecifiedOutputString(DisplayHeaders,
				  OutputType,
				  0,
				  "     Fingerprint20 = ");
	    ringTtyPutFingerprint20 (OutputType,
				     set,
				     key,
				     50);
	} else {
	    SpecifiedOutputString(DisplayHeaders,
				  OutputType,
				  0,
				  "     Fingerprint16 = ");
	    ringTtyPutFingerprint16 (OutputType,
				     set,
				     key,
				     48);
	}
	SpecifiedOutputString(FALSE, OutputType, 0, "\n");
    }

    if ((subkey=ringKeySubkey(set, key)) != NULL) {
	/* Print out its status */
	SpecifiedOutputString(DisplayHeaders,
			      OutputType,
			      0,
			      " sub %4u",
			      (unsigned) ringKeyBits (set, subkey));
	ringTtyPutKeyID(FALSE, OutputType, set, subkey);
	SpecifiedOutputString(FALSE, OutputType, 0, " ");
	creation = ringKeyCreation (set, subkey);
	if (creation > 0) {
	    pgpDateString (creation, datestring);
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%s ",
				  datestring);
	}
	else
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "---------- ");
	
	/* If superkey revoked, don't print expiration */
	if (ringKeyRevoked (set, key))
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "           ");
	else {
	    expiration = ringKeyExpiration (set, subkey);
	    if (expiration > 0) {
		pgpDateString (expiration, datestring);
		SpecifiedOutputString(FALSE,
				      OutputType,
				      0,
				      "%s ",
				      datestring);
	    }
	    else
		SpecifiedOutputString(FALSE,
				      OutputType,
				      0,
				      "---------- ");
	}
	SpecifiedOutputString(DisplayHeaders,
			      OutputType,
			      0,
			      "%-8s  %-15s\n",
			      getKeyAlg (subkey, set),
			      getKeyUse (subkey, set));

	if (mode == 2) {
	    SpecifiedOutputString(DisplayHeaders,
				  OutputType,
				  0,
				  "     Fingerprint20 = ");
	    ringTtyPutFingerprint20(OutputType,
				    set,
				    subkey,
				    50);
	    SpecifiedOutputString(FALSE, OutputType, 0, "\n");
	}
    }

    SpecifiedOutputString(DisplayHeaders,
			  OutputType,
			  0,
			  "%*s",
			  name_indent,
			  "");

    if ((name = getFirstName (set, key)) != NULL)
	ringIterSeekTo (iter, name);
    else
	SpecifiedOutputString(FALSE,
			      OutputType,
			      0,
			      "*** This key is unnamed ***\n");

    while (name != NULL) {
	namestring = ringNameName (set, name, &len);
	ringTtyPutString (namestring,
			  len,
			  (unsigned) len,
			  FALSE,
			  OutputType,
			  0,
			  0);
	SpecifiedOutputString(FALSE,
			      OutputType,
			      0,
			      "\n");
	
	if (mode > 0)
	    ringTtyShowSigs (DisplayHeaders, OutputType, name, set, mode);
	
	name = NULL;
	do {
	    status = ringIterNextObject (iter, 2);
	    if (status > 0) {
		name = ringIterCurrentObject (iter, 2);
		pgpAssert (name != NULL);
		if (ringObjectType (name) != RINGTYPE_NAME)
		    name = NULL;
		else
		    SpecifiedOutputString(DisplayHeaders,
					  OutputType,
					  0,
					  "%*s",
					  name_indent,
					  "");
	    }
	} while (status > 0 && name == NULL);
    } /* end while */
    ringIterDestroy (iter);
    return 0;
}


int
ringTtyCheckKey (Boolean DisplayHeaders,
		 PgpOutputType OutputType,
		 union RingObject *key,
		 struct RingSet const *set,
		 int mode)
{
    struct RingIterator   *iter = ringIterCreate (set);
    union RingObject      *name;
    size_t                 len;
    int                    status;
    char const            *namestring;
    unsigned               confidence;
    unsigned               validity;
    int                    name_leader = 0;
    PgpTrustModel          pgptrustmodel;

    pgpAssert (key != NULL);
    pgpAssert (set != NULL);
    pgpAssert (mode == 4 || mode == 5);
    pgpAssert (ringObjectType (key) == RINGTYPE_KEY);

    pgptrustmodel = pgpTrustModel (ringSetPool (set));

    if (ringKeyError (set, key) != 0) {
	SpecifiedOutputString(DisplayHeaders, OutputType, 0, "? \n");
	/* return, otherwise some strange behaviour occurs */
	return 0;
    }

    if (ringKeyRevoked (set, key))
	SpecifiedOutputString(DisplayHeaders, OutputType, 0, "# ");
    else if (ringKeyAxiomatic(set, key))
	SpecifiedOutputString(DisplayHeaders, OutputType, 0, "* ");
    else
	SpecifiedOutputString(DisplayHeaders, OutputType, 0, "  ");

    ringTtyPutKeyID (FALSE, OutputType, set, key);

    if (pgptrustmodel==PGPTRUST0) {
	SpecifiedOutputString(FALSE,
			      OutputType,
			      0,
			      " %-10s",
			      keyTrustTable[ringKeyTrust(set, key)]);
    }

    if ((name = getFirstName (set, key)) != NULL)
	ringIterSeekTo (iter, name);
    else
	SpecifiedOutputString(FALSE,
			      OutputType,
			      0,
			      "\
                     *** This key is unnamed ***\n");

    while (name != NULL) {
	if (pgptrustmodel==PGPTRUST0) {
	    if (name_leader)
		SpecifiedOutputString(FALSE,
				      OutputType,
				      0,
				      "%*s",
				      name_indent - 9,
				      "");
	    name_leader = 1;
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%-10s",
				  uidValidityTable[ringNameTrust (set, name)]);
	} else {
	    if (name_leader)
		SpecifiedOutputString(FALSE,
				      OutputType,
				      0,
				      "%*s",
				      name_indent - 20,
				      "");
	    name_leader = 1;
	    confidence = (unsigned) ringNameConfidence (set, name);
	    validity = (unsigned) ringNameValidity (set, name);
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  " %10s",
				  getTrustStatement (confidence));
	    SpecifiedOutputString(FALSE,
				  OutputType,
				  0,
				  "%10s",
				  getTrustStatement (validity));
	}
	
	namestring = ringNameName (set, name, &len);
	ringTtyPutString (namestring,
			  len,
			  (unsigned) len,
			  FALSE,
			  OutputType,
			  0,
			  0);
	SpecifiedOutputString(FALSE, OutputType, 0, "\n");
	
	ringTtyCheckSigs(DisplayHeaders, OutputType, name, set, mode);
	
	name = NULL;
	do {
	    status = ringIterNextObject (iter, 2);
	    if (status > 0) {
		name = ringIterCurrentObject (iter, 2);
		pgpAssert (name != NULL);
		if (ringObjectType (name) != RINGTYPE_NAME)
		    name = NULL;
	    }
	} while (status > 0 && name == NULL);
    }
    ringIterDestroy (iter);
    return 0;
}


/* Note:  No mode 5 for the moment */

int
ringTtyKeyCheck (Boolean DisplayHeaders,
		 PgpOutputType OutputType,
		 struct RingSet const *viewkeys,
		 struct RingSet const *allkeys,
		 int mode)
{
    struct RingIterator   *iter;
    union RingObject	*key;
    int			 status;
    PgpTrustModel    pgptrustmodel;

    pgpAssert (allkeys != NULL);

    pgptrustmodel = pgpTrustModel (ringSetPool (allkeys));

    if (viewkeys == NULL)
	viewkeys = allkeys;

    if (pgptrustmodel == PGPTRUST0) {
	SpecifiedOutputString(DisplayHeaders,
			      OutputType,
			      0,
			      "\n\n  KeyID    Trust     Validity  User ID\n");
    } else {
	SpecifiedOutputString(DisplayHeaders,
			      OutputType,
			      0,
			      "\n\n  KeyID   Confidence  Validity User ID\n");
    }
    iter = ringIterCreate (viewkeys);
    while ((status = ringIterNextObject (iter, 1)) > 0) {
	key = ringIterCurrentObject (iter, 1);
	ringTtyCheckKey(DisplayHeaders,
			OutputType,
			key,
			allkeys,
			mode);
    }
    ringIterDestroy (iter);

    return 0;
}


/* Modes:

   0          list keys, but no fingerprints or signatures (-kv)
   1          list keys with signatures, but no fingerprints (-kvv)
   2          list keys with fingerprints and signatures (-kvc)
   3          list keys with signature status info (-ka)
   4          list keys with signature status info, plus trust info (-kc)
   5          same as 4, but do not output numeric trust info
             (not implemented yet)
   */



int
ringTtyKeyView (struct RingSet const *viewkeys,
		struct RingSet const *allkeys,
		char const *keyring,
		int mode)
{
    struct RingIterator *iter;
    union RingObject	*key;
    char			 header[] =
	"Type Bits KeyID    Created    Expires    Algorithm Use\n";
    int			 count = 0;
    int			 status;

    pgpAssert (mode <= 5);
    pgpAssert (allkeys != NULL);

    if (viewkeys == NULL)
	viewkeys = allkeys;

    if (keyring)
	PrimaryOutputString("Key ring: \'%s\'\n", keyring);

    PrimaryOutputString(header);

    iter = ringIterCreate (viewkeys);
    while ((status = ringIterNextObject (iter, 1)) > 0) {
	count++;
	key = ringIterCurrentObject (iter, 1);
	ringTtyShowKey(TRUE, OUTPUT_PRIMARY, key, allkeys, mode);
    }
    ringIterDestroy (iter);

    if (mode < 4) {
	if (count == 1)
	    PrimaryOutputString("1 matching key found\n");
	else
	    PrimaryOutputString("%d matching keys found\n", count);
    }
    else
	ringTtyKeyCheck (TRUE,
			 OUTPUT_PRIMARY,
			 viewkeys,
			 allkeys,
			 mode);

    return 0;
}



/* Key selected for encryption.  Determine is the user really wants to
   use it. If 'batchmode' is set, we fail rather than ask. */

int
ringTtyKeyOKToEncrypt (struct RingSet const *set,
		       union RingObject *key)
{
    union RingObject *name;
    char const *namestring;
    size_t len;
    struct RingIterator *iter;
    time_t expiration;
    char datestring[11];
    int trust;
    unsigned validity;
    PgpTrustModel pgptrustmodel;

    pgptrustmodel = pgpTrustModel (ringSetPool (set));

    ringKeyPrint(OUTPUT_INFORMATION, set, key, 1);

    if (ringKeyRevoked (set, key)) {
	WarningOutput(TRUE,
		      LEVEL_SEVERE,
		      "ABOVE_KEY_REVOKED");
	return 0;
    }

    if (ringKeyDisabled (set, key)) {
	InteractionOutput(TRUE, "STILL_USE_REVOKED_KEY");
	
	if (!pgpTtyGetBool (0, TRUE))
	    return 0;
    }

    if ((expiration = ringKeyExpiration (set, key)) > 0 &&
	expiration <= time ((time_t *) 0)) {
	pgpDateString (expiration, datestring);
	InteractionOutput(TRUE,
			  "STILL_USE_EXPIRED_KEY",
			  datestring);
	if (!pgpTtyGetBool (0, TRUE))
	    return 0;
    }

    iter = ringIterCreate (set);
    pgpAssert(iter != NULL);
    ringIterSeekTo (iter, key);
    while (ringIterNextObject (iter, 2) > 0) {
	name = ringIterCurrentObject (iter, 2);
	pgpAssert(name != NULL);
	if (ringObjectType (name) == RINGTYPE_NAME) {
	    int warn = 0;
	
	    if (pgptrustmodel == PGPTRUST0) {
		switch (trust = ringNameTrust (set, name)) {
		    case PGP_NAMETRUST_UNKNOWN:
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NAMETRUST_UNKNOWN");
			warn = 1;
			break;
		    case PGP_NAMETRUST_UNTRUSTED:
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NAMETRUST_UNTRUSTED");
			warn = 1;
			break;
		    case PGP_NAMETRUST_MARGINAL:
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NAMETRUST_MARGINAL");
			warn = 1;
			break;
		}
	    }
	    else {			/* new trust model */
		validity = ringNameValidity (set, name);
		if (validity == 0) {
		    WarningOutput(TRUE,
				  LEVEL_INFORMATION,
				  "PGP_NEWTRUST_NOT_TRUSTED");
		    warn = 1;
		}
		else if (!ringTrustValid (set, validity)) {
		    WarningOutput(TRUE,
				  LEVEL_INFORMATION,
				  "PGP_NEWTRUST_PARTIAL_TRUST",
				  strtol (getTrustStatement
					  (validity),
					  NULL, 10));
		    warn = 1;
		}
	    }
	    if (warn) {
		namestring = ringNameName (set, name, &len);
		ringTtyPutString (namestring,
				  len,
				  (unsigned) len,
				  TRUE,
				  OUTPUT_WARNING,
				  0,
				  0);
		WarningOutputString(FALSE, 0, "\n");
		if (ringNameWarnonly (set, name))
		    WarningOutput(TRUE,
				  LEVEL_INFORMATION,
				  "PREVIOUSLY_APPROVED_KEY");
		else {
		    InteractionOutput(TRUE,
				      "DO_YOU_WISH_TO_USE_UNTRUSTED_KEY");
		    if (pgpTtyGetBool (0, TRUE))
			ringNameSetWarnonly (set,
					     name);
		    else
			return 0;
		}
		InformationOutputString(FALSE, "\n");
	    }
	}
    }
    return 1;
}

void
ringTtyKeyOKToSign (struct RingSet const *set,
		    union RingObject *key)
{
    struct RingIterator *iter;
    union RingObject *name;
    size_t len;
    char const *namestr;
    int warn = 0;
    time_t expiration;
    char datestring[11];
    byte trust;
    unsigned validity;
    PgpTrustModel pgptrustmodel;

    pgptrustmodel = pgpTrustModel (ringSetPool (set));

    if (ringKeyRevoked (set, key)) {
	WarningOutput(TRUE,
		      LEVEL_SEVERE,
		      "DONT_TRUST_SIGS_FROM_REVOKED_KEYS");
	return;
    }
    if (ringKeyDisabled (set, key))
	WarningOutput(TRUE,
		      LEVEL_INFORMATION,
		      "YOU_HAVE_DISABLED_SIGNING_KEY");

    if ((expiration = ringKeyExpiration (set, key)) > 0 &&
	expiration <= time ((time_t *) 0)) {
	pgpDateString (expiration, datestring);
	WarningOutput(TRUE,
		      LEVEL_INFORMATION,
		      "KEY_HAS_EXPIRED",
		      datestring);
    }

    iter = ringIterCreate (set);
    pgpAssert(iter != NULL);

    ringIterSeekTo (iter, key);
    while (ringIterNextObject (iter, 2) > 0) {
	name = ringIterCurrentObject (iter, 2);
	if (name && ringObjectType (name) == RINGTYPE_NAME) {
	    warn = 0;
	    if (pgptrustmodel == PGPTRUST0) {
		trust = ringNameTrust (set, name);
		switch (trust = ringNameTrust (set, name)) {
		    case PGP_NAMETRUST_UNKNOWN:
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NAMETRUST_UNKNOWN");
			warn = 1;
			break;
		    case PGP_NAMETRUST_UNTRUSTED:
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NAMETRUST_UNTRUSTED_SIGNING_KEY");
			warn = 1;
			break;
		    case PGP_NAMETRUST_MARGINAL:
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NAMETRUST_MARGINAL");
			warn = 1;
			break;
		}
	    }
	    else {			/* new trust model */
		validity = ringNameValidity (set, name);
		if (validity == 0) {
		    WarningOutput(TRUE,
				  LEVEL_INFORMATION,
				  "PGP_NEWTRUST_NOT_TRUSTED_SIGNING_KEY");
		    warn = 1;
		}
		else
		    if (!ringTrustValid (set, validity)) {
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NEWTRUST_PARTIAL_TRUST",
				      strtol (getTrustStatement
					      (validity),
					      NULL, 10));
			warn = 1;
		    }
	    }
	    if (warn) {
		namestr = ringNameName (set, name, &len);
		ringTtyPutString (namestr,
				  len, (unsigned)
				  len,
				  TRUE,
				  OUTPUT_WARNING,
				  0,
				  0);
		WarningOutputString(FALSE, 0, "\n");
	    }
	}
    }
    if (warn)
	WarningOutputString(FALSE, 0, "\n");
}
