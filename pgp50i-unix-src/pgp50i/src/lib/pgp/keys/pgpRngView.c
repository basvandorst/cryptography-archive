/*
* pgpRngView.c -- display the contents of a keyring, with various
* options. This is a simple example of a scrolling list of
* keys and how it can be maintained. ringPick() is the
* function that gives the spiffy displays.
*
* Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by Colin Plumb.
*
* $Id: pgpRngView.c,v 1.3.2.1 1997/06/07 09:50:43 mhw Exp $
*/

#include "first.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>	 /* for strchr() */

#include "pgpDebug.h"
#include "ringview.h"
#include "pgpTimeDate.h"	/* for datePrint */
#include "pgpTrust.h"	/* For PGP_KEYTRUSTF_* */
#include "pgpUserIO.h"	/* for userTrans */

#include "pgpKludge.h"

static char const hexchar[16] = {
	'0','1','2','3','4','5','6','7',
	'8','9','A','B','C','D','E','F'
};

/*
* Print a key fingerprint, in one of the forms:
* 11111111112222222222333333333344444444
* 012345678901234567890123456789012345678901234567
* 01 23 45 67 89 AB CD EF 01 23 45 67 89 AB CD EF (48 chars)
	* 01 23 45 67 89 AB CD EF 01 23 45 67 89 AB CD EF	(47 chars)
	* 0123 4567 89AB CDEF 0123 4567 89AB CDEF	(40 chars)
	* 0123 4567 89AB CDEF 0123 4567 89AB CDEF	(39 chars)
	* 01234567 89ABCDEF 01234567 89ABCDEF	(36 chars)
	* 01234567 89ABCDEF 01234567 89ABCDEF	(35 chars)
	* 0123456789ABCDEF 0123456789ABCDEF	(33 chars)
	* 0123456789ABCDEF0123456789ABCDEF	(32 chars)
	* 0123456789ABCDEF0123456789ABCDE
	* 0123456789ABCDEF0123456789ABCD
	* 0123456789ABCDEF0123456789ABC
	* (truncation continues as needed)
	* depending on the "wid" argument.
	*/
int
putFingerprint(byte const *hash, FILE *f, unsigned wid)
{
			static char const limits[16] = {
				 -1, 46, 38, 46, 34, 46, 38, 46,
				 32, 46, 38, 46, 34, 46, 38, 46 };
			char buf[48];
			char *p = buf;
			int i;

			pgpAssert(f);
			pgpAssert(hash);
			for (i = 0; i < 16; i++) {
					/* Print each space iff it's okay to */
					if (wid > limits[i])
						*p++ = ' ';
					/* Double space in the middle if appropriate */
					if (i == 8 && wid >= 36 && wid != 39 && wid != 47)
						*p++ = ' ';
					*p++ = hexchar[*hash >> 4 & 15];
					*p++ = hexchar[*hash++ & 15];
			}
			/* Final truncation as necessary */
			i = p-buf;
			if (i > wid)
				 i = wid;
			return fwrite(buf, 1, i, f);
	}

int
putKeyID(byte const *keyID, FILE *f)
	{
			char buf[8];
			char *p = buf;
			int i;

			if (keyID) {
					keyID += 4;
					for (i = 0; i < 4; i++) {
						*p++ = hexchar[*keyID >> 4 & 15];
						*p++ = hexchar[*keyID++ & 15];
					}
			} else {
				 memset(buf, ' ', 8);
			}
			return fwrite(buf, 1, 8, f);
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
putString(char const *str, size_t len, unsigned maxlen, FILE *f,
			char q1, char q2)
	{
			int c;
			unsigned t;
			char const *p;
			unsigned remaining = maxlen;
			static char const escapes[] = "\a\b\f\n\r\t\v";
			static char const letters[] = {'a','b','f','n','r','t','v'};

/* Opening quote */
if (q1 && remaining) {
	if (f)
		putc(q1, f);
	remaining--;
}

for (;;) {
/* Printing can only expand the string, so truncate it */
if (len > remaining)
	len = remaining;

/* Find a directly printable substring */
p = str;	/* Remember start of substring */
while (len) {
			c = *str;
			if (!isprint(c) || c == '\\' || c == q2)
				 break;
			str++;
			len--;
	}

/* Print from p up to str */
t = (unsigned)(str - p);
if (t) {
	if (f)
		fwrite(p, 1, (size_t)t, f);
	remaining -= t;
}

/* Done with the string? */
if (!len)
	break;
/* Note that len <= remaining, so remaining != 0 */

					/*
					* c is a character to print that needs escaping.
					* Now we're going to print it, so remove it from the string.
					*/
					len--;
					str++;

					/* Start with the obligatory backslash */
					if (f)
						putc('\\', f);
					if (!--remaining)
						break;

					/* Simple case 1: escaping printable characters */
					if (isprint(c)) {
							if (f)
								 putc((char)c, f);
							--remaining;
							continue;
					}

					/* Simple case 2: standard C escape */
					p = strchr(escapes, c);
					if (p && c) {
							if (f)
								 putc(letters[p-escapes], f);
							--remaining;
							continue;
					}

					/* General octal escapes */
					/* If next char makes it ambiguous, force 3-char escape */
					if (len && isdigit(*str))	/* Force on 8 and 9, too! */
						c += 256;
					if (c > 077) {
							if (f)
								 putc('0' + (c>>6 & 3), f);
							if (!--remaining)
									break;
					}
					if (c > 07) {
							if (f)
									putc('0' + (c>>3 & 7), f);
							if (!--remaining)
								 break;
					}
					if (f)
							putc('0' + (c & 7), f);
					--remaining;
			}

			/* Closing quote */
			if (q2 && remaining) {
				 if (f)
				 	putc(q2, f);
				 remaining--;
			}

			return maxlen - remaining;
	}


/*
* Print info about a key, in the format
*
* 1024 bits, Key ID FBBB8AB1, created 1994/05/07
*
* The ", created" part is omitted if the creation timestamp is 0.
*/
void
keyInfoPrint(byte const keyID[8], word16 keybits, word32 tstamp,
			word16 validity, byte trust, FILE *f)
	{
			(void)validity;	/* Not used for now */
			fprintf(f, userTrans("%6u bits, Key ID "), (unsigned)keybits);
			putKeyID(keyID, f);
			if (tstamp) {
				 fputs(userTrans(", created "), f);
				 datePrint(tstamp, f);
			}
putc('\n', f);
if (trust & PGP_KEYTRUSTF_REVOKED)
	fputs(userTrans("This key has been revoked by its owner\n"),
		f);
}
