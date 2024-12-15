/*
 * pgpkUI.c - Various user-interface helpers for working with key
 * databases on a simple stdio tty interface.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Modified from pgpRingUI.c
 *
 * $Id: pgpkUI.c,v 1.1.2.6.2.6 1997/08/19 21:00:06 quark Exp $
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>     /* for strtol() */
#include <string.h>	/* for strchr() */
#include <time.h>       /* for time () */

#include "pgpDebug.h"
#include "pgpKeyDB.h"
#include "pgpEnv.h"
#include "pgpOutput.h"
#include "pgpkUI.h"
#include "pgpErr.h"
#include "pgpTimeDate.h"	/* for pgpDateString */
#include "pgpUserIO.h"
#include "pgpRngPub.h"
#include "pgpTrust.h"
#include "pgpTrstPkt.h"
#include "pgpTimeDate.h"
#include "pgpMem.h"


/*
 * The old PGP format...
 Type    Date       keyID/bits  User ID
 PUB  1996-03-30 58B96505/2048 F0 4A 30 95 20 17 29 E0  DC A3 3F FB C6 5B 79 3F
 foo
 sig! 1996-03-30 58B96505       foo
 PUB  1996-03-30 E9D1E021/4096 1A E2 EF AA FC CB 59 AC  D4 2A 1A 90 E3 DD 4F 45
 bar
 sig! 1996-03-30 E9D1E021       bar
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
 * 01 23 45 67 89 AB CD EF  01 23 45 67 89 AB CD EF/1024 (53 chars)
 * 01 23 45 67 89 AB CD EF 01 23 45 67 89 AB CD EF/1024  (52 chars)
 * 0123 4567 89AB CDEF  0123 4567 89AB CDEF/1024         (45 chars)
 * 0123 4567 89AB CDEF 0123 4567 89AB CDEF/1024          (44 chars)
 * 01234567 89ABCDEF  01234567 89ABCDEF/1024             (41 chars)
 * 01234567 89ABCDEF 01234567 89ABCDEF/1024              (40 chars)
 * 0123456789ABCDEF 0123456789ABCDEF/1024                (38 chars)
 * 0123456789ABCDEF0123456789ABCDEF/1024                 (37 chars)
 * 0123456789ABCDEF0123456789ABCDE/1024
 * 0123456789ABCDEF0123456789ABCD/1024
 * 0123456789ABCDEF0123456789ABC/1024  (etc., truncating as needed)
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
kdbTtyFormatFingerprint16(char *hash, char *buf, size_t len)
{
	static signed char const limits[16] = {
		-1, 46, 38, 46, 34, 46, 38, 46,
		32, 46, 38, 46, 34, 46, 38, 46 };
	char *p = buf;
	int i;

	for (i = 0; i < 16 && (size_t)(p-buf) != len; i++) {
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
	return p-buf;
}

/*
 * Pretty-print a 160-bit key fingerprint into the supplied buffer,
 * in varius forms, depending on the "len" argument:
 * Version a:
 *           11111111112222222222333333333344444444
 * 012345678901234567890123456789012345678901234567
 * 01234 56789 ABCDE F0123  45678 9ABCD EF012 34567 (48 chars)
 * 01234 56789 ABCDE F0123 45678 9ABCD EF012 34567  (47 chars)
 * 0123456789 ABCDEF0123  456789ABCD EF01234567     (44 chars)
 * 0123456789 ABCDEF0123 456789ABCD EF01234567      (43 chars)
 * 0123456789ABCDEF0123 456789ABCDEF01234567        (41 chars)
 * 0123456789ABCDEF0123456789ABCDEF01234567         (40 chars)
 * 0123456789ABCDEF0123456789ABCDEF0123456
 * 0123456789ABCDEF0123456789ABCDEF012345
 * 0123456789ABCDEF0123456789ABCDEF01234  (etc., truncating as needed)
 *
 * Version b:
 *           1111111111222222222233333333334444444444
 * 01234567890123456789012345678901234567890123456789
 * 0123 4567 89ab cdef 0123  4567 89ab cdef 0123 4567 (50 chars)
 * 0123 4567 89ab cdef 0123 4567 89ab cdef 0123 4567  (49 chars)
 * 01234567 89abcdef 01234567 89abcdef 01234567       (44 chars)
 * 0123456789abcdef0123 456789abcdef01234567          (41 chars)
 * 0123456789ABCDEF0123456789ABCDEF01234567           (40 chars)
 * 0123456789ABCDEF0123456789ABCDEF0123456
 * 0123456789ABCDEF0123456789ABCDEF012345
 * 0123456789ABCDEF0123456789ABCDEF01234  (etc., truncating as needed)
 */
size_t
kdbTtyFormatFingerprint20(char *hash, char *buf, size_t len)
{
	static signed char const limitsa[40] =
		{-1, 46, 42, 46, 40, 46, 42, 46};
	static signed char const limitsb[10] =
		{-1, 48, 43, 48, 43, 48, 43, 48, 43, 48};
	signed char const *limits = limitsb; /* Choose B */
	int period = 4;		/* Use 5 for A, 4 for B */
	char *p = buf;
	int i;

	/* Unlike the 16 byte version, this loops per character */
	for (i = 0; i < 40 && (size_t)(p-buf) != len; i++) {
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
kdbTtyPutFingerprint16(Boolean DisplayHeaders,
					   PgpOutputType OutputType,
					   char *hash,
					   unsigned wid)
{
	char buf[54];		/* Incestuous knowledge */

	if (wid > sizeof(buf))
		wid = sizeof(buf);
	wid = (unsigned) kdbTtyFormatFingerprint16(hash, buf, (size_t) wid);
	SpecifiedOutputBuffer(DisplayHeaders,
						  OutputType,
						  0,
						  buf,
						  sizeof(char),
						  wid);
	return(wid);
}

int
kdbTtyPutFingerprint20(Boolean DisplayHeaders,
					   PgpOutputType OutputType,
					   char *hash, unsigned wid)
{
	char buf[50];		/* Incestuous knowledge */

	if (wid > sizeof(buf))
		wid = sizeof(buf);
	wid = (unsigned) kdbTtyFormatFingerprint20(hash, buf, (size_t) wid);
	SpecifiedOutputBuffer(DisplayHeaders,
						  OutputType,
						  0,
						  buf,
						  sizeof(char),
						  wid);
	return(wid);
}

int
kdbTtyPutKeyID(Boolean DisplayHeaders,
			   PgpOutputType OutputType,
			   char *buf)
{
	int i;

	if(DisplayHeaders)
		SpecifiedOutputString(DisplayHeaders, OutputType, 0, "");

	SpecifiedOutputString(FALSE, OutputType, 0, "0x");

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
kdbTtyPutSigID(Boolean DisplayHeaders,
			   PgpOutputType OutputType,
			   PGPCert *cert)
{
	byte buf[8];
	int i;

    if(DisplayHeaders)
		SpecifiedOutputString(TRUE, OutputType, 0, "");

	SpecifiedOutputString(FALSE, OutputType, 0, "0x");

	i = sizeof (buf);
	pgpGetCertString (cert,
					  kPGPCertPropKeyID,
					  (char *) buf,
					  (size_t *)&i);
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
 * the manner of C strings, up to "maxlen" characters.  Returns the
 * actual number of characters printed, which will always be <= maxlen.
 * If f is NULL, prints nothing.  (May be useful for justification
 * computations.)  The string is surrounded by quotes q1 and q2
 * (if not '\0').  q2  is also \-escaped if it appears in the string.
 */
unsigned
kdbTtyPutString(char const *str,
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
kdbTtyPutKeyInfo(Boolean DisplayHeaders,
				 PgpOutputType OutputType,
				 PGPKey *key)
{
	char buf[PGPDATESTRINGLEN+1];
	PGPTime tstamp;
	long keybits;
	char keyid[8];
	size_t keyidlen = sizeof (keyid);

	pgpGetKeyNumber (key, kPGPKeyPropBits, &keybits);
	SpecifiedOutputString(DisplayHeaders,
						  OutputType,
						  0,
						  "%6u bits, Key ID ",
						  (unsigned) keybits);
	pgpGetKeyString (key, kPGPKeyPropKeyId, keyid, &keyidlen);
	kdbTtyPutKeyID(FALSE, OutputType, keyid);
	pgpGetKeyTime (key, kPGPKeyPropCreation, &tstamp);
	if (tstamp) {
		pgpDateString((word32) tstamp, buf);
		SpecifiedOutputString(FALSE, OutputType, 0, ", created %s\n", buf);
	}
	else
		SpecifiedOutputString(FALSE, OutputType, 0, "\n");

#if 0
	if (trust & PGP_KEYTRUSTF_REVOKED)
		fprintf(f, "%sThis key has been revoked by its owner\n",
		        prefix);
#endif
}

void
kdbKeyPrint(PgpOutputType OutputType, PGPKeyIter *keyiter, int level)
{
	char   name[256];
	PGPKey     *key = pgpKeyIterKey (keyiter);
	PGPKeyIter *iter = NULL;
	PGPUserID  *userid = NULL;
	size_t len;

	kdbTtyPutKeyInfo(TRUE, OutputType, key);
	iter = pgpCopyKeyIter (keyiter);
	pgpKeyIterSet (iter, key);
	while ((userid = pgpKeyIterNextUserID (iter))) {
		len = sizeof (name);
		pgpGetUserIDString (userid, kPGPUserIDPropName, name, &len);
		SpecifiedOutputString(TRUE, OutputType, 0, "%*s", level+2, "");
		(void) kdbTtyPutString(name,
							   len,
							   -1u,
							   FALSE,
							   OutputType,
							   '"',
							   '"');
		SpecifiedOutputString(FALSE, OutputType, 0, "\n");
			/*	break; */
	}
	pgpFreeKeyIter (iter);
}


/* Functions for displaying keys */


static const int name_indent = 32;
static const int sig_indent = 34;


/* List of algorithms indexed by the pkalg byte.  Each entry contains
   the name of the algorithm and what it can be used for.   */

static const char *pkalg_list[] =
{NULL, "RSA", "RSA", "RSA", NULL, NULL, NULL, NULL, NULL, NULL,
	 NULL, NULL, NULL, NULL, NULL, NULL, "Diffie-Hellman", "DSS"};

static const char *
getKeyAlg (PGPKey *key)
{
	long        pkalg;

	pgpGetKeyNumber (key, kPGPKeyPropAlgId, &pkalg);
	return pkalg_list[pkalg];
}


static const char *
getSubKeyAlg (PGPSubKey *subkey)
{
	(void)subkey;
	return pkalg_list[16];
}


static const char *keyuse_list[] =
{"", "Sign only", "Encrypt only", "Sign & Encrypt"};

static const char *
getKeyUse (PGPKey *key)
{
        Boolean cansign, canencrypt;

	pgpGetKeyBoolean (key, kPGPKeyPropCanSign, &cansign);
	pgpGetKeyBoolean (key, kPGPKeyPropCanEncrypt, &canencrypt);
	if (cansign && canencrypt)
	    return keyuse_list[3];
	else if (cansign)
	    return keyuse_list[1];
	else if (canencrypt)
	    return keyuse_list[2];
	return keyuse_list[0];
}

static const char *
getSubKeyUse (PGPSubKey *subkey)
{
	(void)subkey;
	return keyuse_list[0];
}


/* List of algorithms indexed by the pkalg byte.  Each entry contains
   the name of the algorithm and what it can be used for.   */

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
kdbTtyShowSig(PgpOutputType OutputType,
			  PGPKeyIter *sigiter,
			  PGPKeySet *allkeys,
			  int check)
{
    PGPKey			*sigkey = NULL;
    PGPCert         *sig = pgpKeyIterCert (sigiter);
    PGPTime			 creation;
    char			 datestring[11];
    char	 		 namestring[256];
    size_t			 len;
    Boolean          revoked = FALSE, valid, checked, tried, mycert;

	
	pgpGetCertifier (sig, allkeys, &sigkey);
    if(sigkey)
		pgpIncKeyRefCount(sigkey);

	pgpGetCertBoolean (sig, kPGPCertPropIsRevoked, &revoked);

    if (revoked)
		SpecifiedOutputString(TRUE, OutputType, 0, "ret");
    else {
		pgpGetCertBoolean (sig, kPGPCertPropIsMyCert, &mycert);
		if (mycert)
			SpecifiedOutputString(TRUE, OutputType, 0, "SIG");
		else
			SpecifiedOutputString(TRUE, OutputType, 0, "sig");
	}

    pgpGetCertBoolean (sig, kPGPCertPropIsValid, &valid);
    if (!valid)
		SpecifiedOutputString(FALSE, OutputType, 0, "%");
    else if (!sigkey)
		SpecifiedOutputString(FALSE, OutputType, 0, "?");
    else {
        pgpGetCertBoolean (sig, kPGPCertPropIsChecked, &checked);
		if (check && checked)
			SpecifiedOutputString(FALSE, OutputType, 0, "!");
		else {
			pgpGetCertBoolean (sig, kPGPCertPropIsTried, &tried);
			if (check && tried)
				SpecifiedOutputString(FALSE, OutputType, 0, "*");
			else
				SpecifiedOutputString(FALSE, OutputType, 0, " ");
		}
    }

	SpecifiedOutputString(FALSE, OutputType, 0, "      ");
    kdbTtyPutSigID (FALSE, OUTPUT_PRIMARY, sig);
	SpecifiedOutputString(FALSE, OutputType, 0, " ");

    /* Signature date */

    pgpGetCertTime (sig, kPGPCertPropCreation, &creation);
    if (creation > 0) {
        pgpDateString ((word32 )creation, datestring);
		SpecifiedOutputString(FALSE, OutputType, 0, datestring);
    }
    else
		SpecifiedOutputString(FALSE, OutputType, 0, "----------");

    if (sigkey) {
       	len = sizeof (namestring);
		pgpGetPrimaryUserIDName (sigkey, namestring, &len);
		SpecifiedOutputString(FALSE, OutputType, 0, " ");
		kdbTtyPutString (namestring,
						 len,
						 (unsigned) len,
						 FALSE,
						 OutputType,
						 0,
						 0);
		SpecifiedOutputString(FALSE, OutputType, 0, "\n");
		pgpFreeKey (sigkey);
    }
    else
		SpecifiedOutputString(FALSE,
							  OutputType,
							  0,
							  " (Unknown signator, can't be checked)\n");
    return 0;
}

/* This function displays a set of signatures attached to a name. */

int
kdbTtyShowSigs (PgpOutputType OutputType,
				PGPKeyIter *iter,
				PGPKeySet *allkeys,
				int mode)
{
	PGPCert             *sig = NULL;
	int		     check;

	pgpAssert (iter != NULL);
	pgpAssert (allkeys != NULL);

	check = (mode == 3 || mode == 4);

	/* Display sigs attached to the Name */

	while ((sig = pgpKeyIterNextUIDCert (iter))) {
		kdbTtyShowSig(OutputType, iter, allkeys, check);
	}
 	return 0;
}


int
kdbTtyCheckSigs (PgpOutputType OutputType,
				 PGPKeyIter *iter,
				 PGPKeySet *allkeys,
				 int mode)
{
    PGPKey		*sigkey = NULL;
    PGPCert		*sig = NULL;
    size_t		 len;
    char const          *output;
    char		 namestring[256];
    long		 sigtrust, validity;
    long	         confidence;
    Boolean		 revoked;
    PgpTrustModel        pgptrustmodel;

    pgpAssert (iter != NULL);
    pgpAssert (mode == 4 || mode == 5);

    pgptrustmodel = pgpGetTrustModel ();

    /* Display sigs attached to the Name */

    while ((sig = pgpKeyIterNextUIDCert (iter))) {
        pgpGetCertifier (sig, allkeys, &sigkey);
		if (sigkey)
			pgpIncKeyRefCount (sigkey);
		SpecifiedOutputString(TRUE, OutputType, 0, "%*s", 13, "");
		if (pgptrustmodel == PGPTRUST0) {
			if (!sigkey || sigkey ==
				pgpKeyIterKey (iter))
				sigtrust = PGP_SIGTRUST_NOKEY;
			else {
				pgpGetCertBoolean (sig, kPGPCertPropIsRevoked, &revoked);
				if (revoked)
					sigtrust = PGP_SIGTRUST_RETIRED;
				else {
					pgpGetKeyBoolean (sigkey, kPGPKeyPropIsRevoked, &revoked);
					if (revoked)
						sigtrust = PGP_SIGTRUST_REVOKED;
					else {
						pgpGetKeyNumber (sigkey, kPGPKeyPropValidity,
										 &validity);
						if (validity != PGP_VALIDITY_COMPLETE)
							sigtrust = PGP_KEYTRUST_NEVER;
						else
							pgpGetKeyNumber (sigkey, kPGPKeyPropTrust,
											 &sigtrust);
					}
				}
			}
			output = keyTrustTable[sigtrust];
			SpecifiedOutputString(FALSE,
								  OutputType,
								  0,
								  "%-10s%*s",
								  output,
								  10, 
								  "");
		}
		else {				/* new trust model */
			if (!sigkey || sigkey ==
				pgpKeyIterKey (iter))
				output = " ";
			else {
				pgpGetCertBoolean (sig, kPGPCertPropIsRevoked, &revoked);
				if (revoked)
					output = "retired ";
				else {
					pgpGetKeyBoolean (sigkey, kPGPKeyPropIsRevoked, &revoked);
					if (revoked)
						output = "revoked ";
					else {
						pgpGetKeyNumber (sigkey, kPGPKeyPropValidity,
										 &confidence);
						output = getTrustStatement ((word16) confidence);
					}
				}
			}
			SpecifiedOutputString(FALSE,
								  OutputType,
								  0,
								  "%10s%*s",
								  output,
								  12,
								  "");
		}
		
		if (sigkey) {
			len = sizeof (namestring);
			pgpGetPrimaryUserIDName (sigkey, namestring, &len);
			kdbTtyPutString (namestring, len,
							 (unsigned) len, FALSE, OutputType, 0, 0);
			SpecifiedOutputString(FALSE,
								  0,
								  OutputType,
								  "\n");
			pgpFreeKey (sigkey);
		}
		else {
			SpecifiedOutputString(FALSE,
								  0,
								  OutputType,
								  "(KeyID:  ");
			kdbTtyPutSigID (FALSE, OutputType, sig);
			SpecifiedOutputString(FALSE,
								  0,
								  OutputType,
								  ")\n");
		}
    }

    return 0;
}


/*  This function displays a key in -kv, -kvv, -kvc, or the first section
    of a -kc format. */

int
kdbTtyShowKey (PgpOutputType OutputType,
			   PGPKeyIter *iter,
			   PGPKeySet *allkeys,
			   int mode)
{
    PGPKey				*key;
	PGPSubKey           *subkey;
    PGPUserID			*name;
    size_t				len;
    PGPTime				creation, expiration;
    char				datestring[11];
    char  				namestring[256];
    Boolean				secret, axiomatic, disabled, revoked;
    long                keybits;
	char                keyid[8];
	size_t              keyidlen;

    pgpAssert (iter != NULL);
    pgpAssert (mode <= 5);

    key = pgpKeyIterKey (iter);
    pgpGetKeyBoolean (key, kPGPKeyPropIsSecret, &secret);
    if (secret)
		SpecifiedOutputString(TRUE, OutputType, 0, "sec");
    else
		SpecifiedOutputString(TRUE, OutputType, 0, "pub");

    pgpGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &axiomatic);
    pgpGetKeyBoolean (key, kPGPKeyPropIsDisabled, &disabled);
	if (disabled)
		SpecifiedOutputString(FALSE, OutputType, 0, "@ ");
    else if (axiomatic)
		SpecifiedOutputString(FALSE, OutputType, 0, "+ ");
    else
		SpecifiedOutputString(FALSE, OutputType, 0, "  ");

    pgpGetKeyNumber (key, kPGPKeyPropBits, &keybits);

	SpecifiedOutputString(FALSE, OutputType, 0, "%4u ", (unsigned) keybits);
	keyidlen = 8;
	pgpGetKeyString (key, kPGPKeyPropKeyId, keyid, &keyidlen);
    kdbTtyPutKeyID(FALSE, OutputType, keyid);
	SpecifiedOutputString(FALSE, OutputType, 0, " ");
    pgpGetKeyTime (key, kPGPKeyPropCreation, &creation);
    if (creation > 0) {
        pgpDateString ((word32) creation, datestring);
		SpecifiedOutputString(FALSE, OutputType, 0, "%s ", datestring);
    }
    else
		SpecifiedOutputString(FALSE, OutputType, 0, "---------- ");

    pgpGetKeyBoolean (key, kPGPKeyPropIsRevoked, &revoked);
    if (revoked)
		SpecifiedOutputString(FALSE, OutputType, 0,"*REVOKED*  ");
    else {
        pgpGetKeyTime (key, kPGPKeyPropExpiration, &expiration);
	if (expiration > 0) {
	    pgpDateString ((word32) expiration, datestring);
		SpecifiedOutputString(FALSE, OutputType, 0, "%s ", datestring);
	}
	else
		SpecifiedOutputString(FALSE, OutputType, 0, "---------- ");
    }

	SpecifiedOutputString(FALSE,
						  OutputType,
						  0,
						  "%-14s  %-15s\n",
						  getKeyAlg (key),
						  getKeyUse (key));

    if (mode == 2) {
        char fingerprint[20];
		size_t	flen = sizeof (fingerprint);
		pgpGetKeyString (key, kPGPKeyPropFingerprint, fingerprint, &flen);
		if (flen == 20) {
			SpecifiedOutputString(TRUE,
								  OutputType,
								  0,
								  "f20    Fingerprint20 = ");
			kdbTtyPutFingerprint20(FALSE, OutputType, fingerprint, 50);
		}
		else {
			SpecifiedOutputString(TRUE,
								  OutputType,
								  0,
								  "f16    Fingerprint16 = ");
			kdbTtyPutFingerprint16(FALSE, OutputType, fingerprint, 48);
		}
		SpecifiedOutputString(FALSE, OutputType, 0, "\n");
    }

	while ((subkey = pgpKeyIterNextSubKey (iter)) != NULL) {
		/* Print out its status */
		SpecifiedOutputString(TRUE, OutputType, 0, "sub  ");
		pgpGetSubKeyNumber (subkey, kPGPKeyPropBits, &keybits);
		SpecifiedOutputString(TRUE,
							  OutputType,
							  0,
							  "%4u ",
							  (unsigned) keybits);
		keyidlen = 8;
		pgpGetSubKeyString (subkey, kPGPKeyPropKeyId, keyid, &keyidlen);
		kdbTtyPutKeyID(FALSE, OutputType, keyid);
		SpecifiedOutputString(FALSE, OutputType, 0, " ");
		pgpGetSubKeyTime (subkey, kPGPKeyPropCreation, &creation);
		if (creation > 0) {
			pgpDateString (creation, datestring);
			SpecifiedOutputString(FALSE, OutputType, 0, "%s ", datestring);
		}
		else
			SpecifiedOutputString(FALSE, OutputType, 0, "---------- ");

		/* If superkey revoked, don't print expiration */
		if (revoked)
			SpecifiedOutputString(FALSE, OutputType, 0, "           ");
		else {
			pgpGetSubKeyTime (subkey, kPGPKeyPropExpiration, &expiration);
			if (expiration > 0) {
				pgpDateString (expiration, datestring);
				SpecifiedOutputString(FALSE,
									  OutputType,
									  0,
									  "%s ",
									  datestring);
			}
			else
				SpecifiedOutputString(FALSE, OutputType, 0, "---------- ");
		}
		SpecifiedOutputString(FALSE,
							  OutputType,
							  0,
							  "%-8s  %-15s\n",
							  getSubKeyAlg (subkey),
							  getSubKeyUse (subkey));
		if (mode == 2) {
			char fingerprint[20];
			size_t flen = sizeof (fingerprint);
			pgpGetSubKeyString (subkey, kPGPKeyPropFingerprint, fingerprint,
								&flen);
			SpecifiedOutputString(TRUE,
								  OutputType,
								  0,
								  "f20    Fingerprint20 = ");
			kdbTtyPutFingerprint20(FALSE, OutputType, fingerprint, 50);
			SpecifiedOutputString(FALSE, OutputType, 0, "\n");
		}
	}

	SpecifiedOutputString(TRUE, OutputType, 0, "uid  ");

    name = pgpKeyIterNextUserID (iter);
    if (!name)
		SpecifiedOutputString(FALSE,
							  OutputType,
							  0,
							  "*** This key is unnamed ***\n");

    while (name != NULL) {
        len = sizeof (namestring);
		pgpGetUserIDString (name, kPGPUserIDPropName, namestring, &len);
		kdbTtyPutString (namestring,
						 len,
						 (unsigned) len,
						 FALSE,
						 OutputType,
						 0,
						 0);
		SpecifiedOutputString(FALSE, OutputType, 0, "\n");
		
		if (mode > 0)
		    kdbTtyShowSigs(OutputType, iter, allkeys, mode);
		
		name = pgpKeyIterNextUserID (iter);
		if (name)
			SpecifiedOutputString(TRUE, OutputType, 0, "uid  ");
    } /* end while */
    return 0;
}


int
kdbTtyCheckKey (PgpOutputType OutputType,
				PGPKeyIter *iter,
				PGPKeySet *allkeys, int mode)
{
    PGPKey				  *key = NULL;
    PGPUserID             *name = NULL;
    char                   namestring[256];
    size_t				   namelen;
    long				   confidence;
    long				   validity;
    int                    name_leader = 0;
    long				   trust;
    PgpTrustModel          pgptrustmodel;
    Boolean				   revoked, axiomatic;
	char                   keyid[8];
	size_t                   keyidlen = sizeof(keyid);

    pgpAssert (iter != NULL);
    pgpAssert (mode == 4 || mode == 5);
    pgpAssert (pgpKeyIterKey (iter));

    pgptrustmodel = PGPTRUST0;
    key = pgpKeyIterKey (iter);

    pgpGetKeyBoolean (key, kPGPKeyPropIsRevoked, &revoked);
    if (revoked)
		SpecifiedOutputString(TRUE, OutputType, 0, "# ");
    else {
        pgpGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &axiomatic);
	if (axiomatic)
		SpecifiedOutputString(TRUE, OutputType, 0, "* ");
	else
		SpecifiedOutputString(TRUE, OutputType, 0, "  ");
    }

	keyidlen = 8;
	pgpGetKeyString (key, kPGPKeyPropKeyId, keyid, &keyidlen);
    kdbTtyPutKeyID (FALSE, OutputType, keyid);

    if (pgptrustmodel == PGPTRUST0) {
        pgpGetKeyNumber (key, kPGPKeyPropTrust, &trust);
		SpecifiedOutputString(FALSE,
							  OutputType,
							  0,
							  " %-10s",
							  keyTrustTable[trust]);
    }

    name = pgpKeyIterNextUserID (iter);
    if (!name)
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
			pgpGetUserIDNumber (name, kPGPUserIDPropValidity, &trust);
			SpecifiedOutputString(FALSE,
								  OutputType,
								  0,
								  "%-10s",
								  uidValidityTable[trust]);
		}
		else {
			if (name_leader)
				SpecifiedOutputString(FALSE,
									  OutputType,
									  0,
									  "%*s",
									  name_indent - 20,
									  "");
			name_leader = 1;
			pgpGetUserIDNumber (name, kPGPUserIDPropConfidence, &confidence);
			pgpGetUserIDNumber (name, kPGPUserIDPropValidity, &validity);
			SpecifiedOutputString(FALSE,
								  OutputType,
								  0,
								  " %10s",
								  getTrustStatement ((word16) confidence));
			SpecifiedOutputString(FALSE,
								  OutputType,
								  0,
								  "%10s",
								  getTrustStatement ((word16) validity));
		}
		
		namelen = sizeof (namestring);
		pgpGetUserIDString (name, kPGPUserIDPropName, namestring, &namelen);
		kdbTtyPutString (namestring,
						 namelen,
						 (unsigned) namelen,
						 FALSE,
						 OutputType,
						 0,
						 0);
		SpecifiedOutputString(FALSE, OutputType, 0, "\n");
		kdbTtyCheckSigs(OutputType, iter, allkeys, mode);
	
		name = pgpKeyIterNextUserID (iter);
    }
    return 0;
}


/* Note:  No mode 5 for the moment */

int
kdbTtyKeyCheck (PgpOutputType OutputType,
				PGPKeyList *viewkeys,
				PGPKeySet *allkeys, int mode)
{

    PGPKeyIter          *iter = NULL;
    char header0[] = "\n\n  KeyID      Trust     Validity  User ID\n";
    char header1[] = "\n\n  KeyID     Confidence Validity  User ID\n";
    PgpTrustModel       pgptrustmodel;
    int			        list_create = 0;;

    pgpAssert (allkeys != NULL);
    pgptrustmodel = pgpGetTrustModel ();

    if (viewkeys == NULL) {
        viewkeys = pgpOrderKeySet (allkeys, kPGPUserIDOrdering);
		list_create = 1;
    }

    if (pgptrustmodel == PGPTRUST0) {
		SpecifiedOutputString(TRUE, OutputType, 0, header0);
    } else {
		SpecifiedOutputString(TRUE, OutputType, 0, header1);
    }
    iter = pgpNewKeyIter (viewkeys);
    while (pgpKeyIterNext (iter) != NULL)
        kdbTtyCheckKey (OutputType, iter, allkeys, mode);

    pgpFreeKeyIter (iter);
    if (list_create)
        pgpFreeKeyList (viewkeys);
    return 0;
}


/* Modes:

   0         list keys, but no fingerprints or signatures (-kv)
   1         list keys with signatures, but no fingerprints (-kvv)
   2         list keys with fingerprints and signatures (-kvc)
   3         list keys with signature status info (-ka)
   4         list keys with signature status info, plus trust info (-kc)
   5         same as 4, but do not output numeric trust info
   (not implemented yet)
   */



int
kdbTtyKeyView (PGPKeyList *viewkeys, PGPKeySet *allkeys,
				char const *keyring, int mode)
{
    PGPKeyIter       *iter;
    char			 header[] =
		"Type Bits KeyID      Created    Expires    Algorithm       Use\n";
    int				  count = 0;
    int              list_create = 0;

    pgpAssert (mode <= 5);
    pgpAssert (allkeys != NULL);

    if (viewkeys == NULL) {
        viewkeys = pgpOrderKeySet (allkeys, kPGPUserIDOrdering);
		list_create = 1;
    }

    if (keyring)
		PrimaryOutputString("Key ring: \'%s\'\n", keyring);

    PrimaryOutputString(header);

    iter = pgpNewKeyIter (viewkeys);
    while (pgpKeyIterNext (iter) != NULL) {
        count++;
		kdbTtyShowKey (OUTPUT_PRIMARY, iter, allkeys, mode);
#if 0
		/* Temporary test of message recovery keys */
		{
			PGPKey *curkey = pgpKeyIterKey (iter);
			PGPKey *mrkey;
			byte kclass = 0;
			int nkey = 0;
			PGPError mrerr;
			for ( ; ; ) {
				mrerr = pgpGetMessageRecoveryKey (curkey, allkeys, nkey++,
					&mrkey, &kclass, NULL);
				if (mrkey) {
					fprintf (fp, " message recovery key, class 0x%02x:\n  ",
							 kclass);
					pgpKeyIterSet (iter, mrkey);
					kdbTtyShowKey (TRUE, OUTPUT_PRIMARY, iter, allkeys, mode);
					pgpKeyIterSet (iter, curkey);
				} else if (mrerr) {
					fprintf (fp,
					" *** missing message recovery key, class 0x%02x ***\n",
							 kclass);
				} else {
					break;
				}
			}
		}
#endif
		PrimaryOutputString("\n");
    }
    pgpFreeKeyIter (iter);

    if (mode < 4) {
        if (count == 1)
			PrimaryOutputString("1 matching key found\n");
		else
			PrimaryOutputString("%d matching keys found\n", count);
    }
    else
        kdbTtyKeyCheck (OUTPUT_PRIMARY, viewkeys, allkeys, mode);

    if (list_create)
        pgpFreeKeyList (viewkeys);
    return 0;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
