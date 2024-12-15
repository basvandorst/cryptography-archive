/*
 * pgpConf.c - Parse configuration files and lines.
 *
 * Copyright (C) 1993-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpConf.c,v 1.6.2.3.2.8 1997/10/09 00:06:35 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>	 /* For strtol() */
#include <string.h>

#include "pgpConf.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpMsg.h"
#include "pgpUI.h"

struct Config {
			char const *name;
			int var;
			unsigned flags;
			int min, max;
	};

/* The following are in the flags: */
	#define OPT_TYPEMASK	3
	#define OPT_IGNORE	 0		/* Ignore if encountered */
	#define OPT_BINARY	 1		/* Boolean options: min = false, max = true */
	#define OPT_INT		2		/* Integer option, in range [min,max] */
	#define OPT_STRING	 3		/* String, of length <= max */
/* The following additional flags are also used: */
#define OPT_CMD		4	 /* Command line only */
#define OPT_LOWER	8	/* Lowercase strings before storing */
#define OPT_FUNC	16	/* Call function rather than set conf */

/* Up to this many errors are ignored when reading a config file. */
#define MAX_ERRORS 3	/* Don't give up right away on error. */

#define FUNC_RANDSOURCE	2

/* The options list. Indexes in the left margin are used in configInit. */
static struct Config
EnvOptions[] = {
	{ "armor",            PGPENV_ARMOR,       OPT_BINARY,     0, 1 },
	{ "armorlines",       PGPENV_ARMORLINES,  OPT_INT,        0, INT_MAX },
	{ "bakring",          PGPENV_BAKRING,     OPT_STRING,     0, 0 },
	{ "cert_depth",       PGPENV_CERTDEPTH,   OPT_INT,        0, 8 },
 	{ "charset",          PGPENV_CHARSET,     OPT_STRING,     0, 0 },
	{ "clearsign",        PGPENV_CLEARSIG,    OPT_BINARY,     0, 1 },
	{ "comment",          PGPENV_COMMENT,     OPT_STRING,     0, 70 },
	{ "completes_needed", PGPENV_COMPLETES,   OPT_INT,        1, 4 },
	{ "compress",         PGPENV_COMPRESS,    OPT_BINARY,     0, 1 },
	{ "encrypttoself",    PGPENV_ENCRYPTTOSELF,OPT_BINARY,     0, 1 },
	{ "httpkeyserverhost",PGPENV_HTTPKEYSERVERHOST, OPT_STRING, 0, 255},
        { "httpkeyserverport",PGPENV_HTTPKEYSERVERPORT, OPT_INT,  0, INT_MAX},
/*	{ "interactive",      PGPENV_INTERACTIVE, OPT_BINARY,     0, 1 },*/
/*	{ "keepbinary",       PGPENV_KEEPBINARY,  OPT_BINARY,     0, 1 },*/
	{ "language",         PGPENV_LANGUAGE,  OPT_STRING|OPT_LOWER,  0, 16 },
	{ "languagefile",     PGPENV_LANGUAGEFILE, OPT_STRING,    0, 0},
	{ "marginals_needed", PGPENV_MARGINALS,   OPT_INT,        1, 4 },
	{ "myname",           PGPENV_MYNAME,      OPT_STRING,     0, 255 },
/*	{ "pager",            PGPENV_PAGER,       OPT_STRING,     0, 255 },*/
/*	{ "pkcs_compat",      PGPENV_PKCS_COMPAT, OPT_INT,      0, INT_MAX },*/
	{ "pgp_mime",         PGPENV_PGPMIME,     OPT_BINARY,     0, 1 },
	{ "outputprimaryfd",  PGPENV_OUTPUTPRIMARYFD, OPT_INT,    0, INT_MAX },
	{ "outputinformationfd",  PGPENV_OUTPUTINFORMATIONFD, OPT_INT,    
	  0, INT_MAX },
	{ "outputinteractionfd",  PGPENV_OUTPUTINTERACTIONFD, OPT_INT,    
	  0, INT_MAX },
	{ "outputwarningfd",  PGPENV_OUTPUTWARNINGFD, OPT_INT,    0, INT_MAX },
	{ "outputerrorfd",    PGPENV_OUTPUTERRORFD, OPT_INT,      0, INT_MAX },
	{ "outputstatusfd",   PGPENV_OUTPUTPRIMARYFD, OPT_INT,    0, INT_MAX },
			{ "pgp_mimeparse",	PGPENV_PGPMIMEPARSEBODY, OPT_BINARY,0, 1 },
			{ "pubring",	PGPENV_PUBRING, OPT_STRING, 0, 0 },
			{ "randomdevice",	PGPENV_RANDOMDEVICE, OPT_STRING, 0, 0 },
			{ "randseed",	PGPENV_RANDSEED, OPT_STRING, 0, 0 },
#ifdef UNIX
	/*		{ "randsource",	FUNC_RANDSOURCE, OPT_STRING|OPT_FUNC, 0, 0 },*/
#endif
			{ "secring",	PGPENV_SECRING,	OPT_STRING,	0, 0 },
			{ "showpass",	PGPENV_SHOWPASS,	OPT_BINARY,	0, 1 },
			{ "textmode",	PGPENV_TEXTMODE,	OPT_BINARY,	0, 1 },
			{ "tmp",	PGPENV_TMP,	OPT_STRING,	0, 0 },
/* Temp for new trust - change to something better */
		{ "trusted",		PGPENV_TRUSTED,	OPT_INT,	0, 255 },
		{ "tzfix",		PGPENV_TZFIX,	OPT_INT,	-24, 24 },
		{ "verbose",		PGPENV_VERBOSE,	OPT_INT,	0, INT_MAX },
/* Experimental XXX@@@ */
			{ "version",	PGPENV_VERSION,	OPT_INT,	2, 4 },
			{ "ciphernum",		PGPENV_CIPHER,		OPT_INT,		1, INT_MAX },
			{ "hashnum",		PGPENV_HASH,		OPT_INT,		1, INT_MAX },
			{ "fastkeygen",	PGPENV_FASTKEYGEN, OPT_BINARY,	0, 1 },
	{ "autoserverfetch",  PGPENV_AUTOSERVERFETCH, OPT_BINARY, 0, 1},
        { "nobatchinvalidkeys", PGPENV_NOBATCHINVALIDKEYS, OPT_BINARY, 0, 1},

			/* command line only */
			{ "batchmode",	PGPENV_BATCHMODE, OPT_BINARY|OPT_CMD, 0, 1 },
			{ "force",	PGPENV_FORCE,		OPT_BINARY|OPT_CMD, 0, 1 },
			{ "magic",	PGPENV_MAGIC,		OPT_BINARY|OPT_CMD, 0, 1 },
			{ "noout",	PGPENV_NOOUT,		OPT_BINARY|OPT_CMD, 0, 1 },
	{ "license",          PGPENV_LICENSE,     OPT_BINARY|OPT_CMD,  0, 1 },
	{ "headers",          PGPENV_HEADERS,     OPT_BINARY|OPT_CMD,  0, 1 },
			/* Null-terminated */
			{ (char const *)0,	0,	0,		0, 0 }
};

/* Case-insensitive memory compare */
static int
xmemicmp (char const *in1, char const *in2, int len)
	{
			while (len--) {
					if (tolower(*in1) != tolower(*in2))
							return 1;
					in1++;
					in2++;
			}
			return 0;
	}

/*
 * Find a keyword in a null-terminated options list.
 * Returns the option, or NULL if the option is ambiguous,
 * or a pointer to the null option if it is not found.
 * optmask is a mask of bits in the opt->flags list which cause
 * entries with any of those bits set to be ignored.
 */
static struct Config const *
configLookup (struct PgpUICb const *ui, void *arg, unsigned linenum,
			char const *key, unsigned keylen, int optmask)
	{
			struct Config *hit = NULL;
			int i;
			struct PgpUICbArg msgarg1, msgarg2, msgarg3, msgarg4;

			for (i=0; EnvOptions[i].name; i++) {
					if ((EnvOptions[i].flags & optmask) != 0)
						continue;
					if (xmemicmp(EnvOptions[i].name, key, keylen) != 0)
						continue;
					/* We have a match! */
					if (strlen(EnvOptions[i].name) == keylen)
						return (&(EnvOptions[i])); /* exact match */
					/* Do we have a double match? */
					if (hit) {
							msgarg1.type = PGP_UI_ARG_UNSIGNED;
							msgarg1.val.u = linenum;
							msgarg2.type = PGP_UI_ARG_BUFFER;
							msgarg2.val.buf.buf = (byte *)key;
							msgarg2.val.buf.len = keylen;
							msgarg3.type = PGP_UI_ARG_STRING;
							msgarg3.val.s = hit->name;
							msgarg4.type = PGP_UI_ARG_STRING;
							msgarg4.val.s = EnvOptions[i].name;
							ui->message (arg, PGPERR_CONFIG,
								 PGPMSG_CONFIG_AMBIGUOUS, 4, &msgarg1,
								 &msgarg2, &msgarg3, &msgarg4);

							return (struct Config const *)0;
					}
					/* Remember this match */
					hit = &(EnvOptions[i]);
					/* Keep looking for an exact match or ambiguity. */
			}

			if (hit) {
				 return hit;
			}

			msgarg1.type = PGP_UI_ARG_UNSIGNED;
			msgarg1.val.u = linenum;
			msgarg2.type = PGP_UI_ARG_BUFFER;
			msgarg2.val.buf.buf = (byte *)key;
			msgarg2.val.buf.len = keylen;
			ui->message (arg, PGPERR_CONFIG, PGPMSG_CONFIG_UNKNOWN_KEYWORD,
				 2, &msgarg1, &msgarg2);

			return (&EnvOptions[i]);
}

static int
configCallFunci (struct PgpEnv *env, int varnum, int val, int pri)
	{
			(void)env;
			(void)val;
			(void)pri;
			switch (varnum) {
			default:
				 return PGPERR_CONFIG_BADFUNC;
			}
	}

static int
configCallFuncs (struct PgpEnv *env, int varnum, char const *string, int len,
				 int pri)
	{
			(void)env;
			(void)string;
			(void)len;
			(void)pri;
			switch (varnum) {
			case FUNC_RANDSOURCE:
			default:
				 return PGPERR_CONFIG_BADFUNC;
			}
	}

/* Return the number of leading whitespace characters in a buffer */
static unsigned
skipWhite(char const *buf)
	{
			unsigned i = 0;

while (isspace(buf[i]))
	i++;

return i;
}

/*
 * Reduce "len" by the amount of trailing whitespace characters in the
 * string starting at buf and len characters long.
 */
static unsigned
unskipWhite(char const *buf, unsigned len)
	{
			while (len && isspace(buf[len-1]))
				 --len;

return len;
}

/*
 * Get a word up to the next non-graphic character, comment symbol (#)
 * or delimiter (such as '='). Set delimiter to a non-graphic character
 * (0 works well) to not use it.
 * Returns the length of the word found.
 */
static unsigned
getWord(char const *buf, char delimiter)
	{
			char c;
			int i = 0;

			do {
				 c = buf[i++];
			} while (isgraph(c) && c != '#' && c != delimiter);

			return i-1;
}

/*
 * Get a string in the nasty non-quoted format.
 * Spaces are allowed, but the string ends at a comment and
 * trailing whitespace is suppresed.
 * Returns the length of the string found.
 */
static unsigned
getString(char const *buf)
	{
			char c;
			unsigned i = 0;

			do
				 c = buf[i++];
			while (isprint(c) && c != '#');

			return unskipWhite(buf, i-1);
}

/*
 * Get a quoted string. Anything is allowed up to a trailing double
 * quote. Returns the length of the string found, or -1 on error.
 */
static unsigned
getQString(struct PgpUICb const *ui, void *arg, unsigned linenum,
			char const *buf)
{
			char c;
			unsigned i = 0;
			struct PgpUICbArg msgarg1, msgarg2;

			do
				 c = buf[i++];
			while (c && c != '"');

			--i;

if (c == '"')
	return i;

			/* Error case - drop trailing whitespace (like NL) when printing */
			msgarg1.type = PGP_UI_ARG_UNSIGNED;
			msgarg1.val.u = linenum;
			msgarg2.type = PGP_UI_ARG_BUFFER;
			msgarg2.val.buf.buf = (byte *)buf;
			msgarg2.val.buf.len = unskipWhite (buf, i);
	ui->message (arg, PGPERR_CONFIG, PGPMSG_CONFIG_STRINGEND,
		2, &msgarg1, &msgarg2);

	return (unsigned)-1;
}


/*
 * Parse a line of input. linenum == 0 indicates the command line,
 * and causes a few changes in activity:
 * - some extra parameters are recognized (OPT_CMD)
 * - "=ON" is implicit for boolean options
 * - error messages are printed differently
 * - Unknown keywords are fatal
 * - Strings need not be copied, since the argv array isn't going away
 *
 * Note that the input "line" array is not permanently modified, but it
 * is occasionally modified in place temporarily to install a trailing
 * null character in the middle.
 *
 * Returns 0 on success, a PGPERR_* on failure.
 */
static int
configLineParse(struct PgpUICb const *ui, void *arg, struct PgpEnv *env,
				 unsigned linenum, char *line, int pri)
	{
			char const *key;
			unsigned keylen;
			unsigned i;
			struct Config const *opt;
			long val;
			char *p;
			struct PgpUICbArg msgarg1, msgarg2, msgarg3, msgarg4, msgarg5;

			line += skipWhite (line);

			keylen = getWord (line, '=');
			if (!keylen)
				 return 0;	/* Blank line */

			key = line;

			opt = configLookup (ui, arg, linenum, key, keylen,
				 	linenum ? OPT_CMD : 0);
			if (!opt)
				 return PGPERR_CONFIG_BADOPT;
			if (!opt->name)	/* In the config file, not found isn't fatal */
				 return linenum ? 0 : PGPERR_CONFIG_BADOPT;
			if ((opt->flags & OPT_TYPEMASK) == OPT_IGNORE)
				 return 0;

			msgarg1.type = PGP_UI_ARG_UNSIGNED;
			msgarg1.val.u = linenum;
			msgarg2.type = PGP_UI_ARG_BUFFER;
			msgarg2.val.buf.buf = (byte *)key;
			msgarg2.val.buf.len = keylen;

			line += keylen;
line += skipWhite(line);
if (*line != '=') {
 			/* On the command line, "armor" means "armor=on" */
 			if (!linenum && *line == '\0' &&
 			    (opt->flags & OPT_TYPEMASK) == OPT_BINARY)
 			{
 			if (opt->flags & OPT_FUNC)
 					(void)configCallFunci(env, opt->var, opt->max,
 							pri);
 			else
 					(void)pgpenvSetInt(env,
 							(enum PgpEnvInts)opt->var,
 							opt->max, pri);
 			return 0;
	}

ui->message (arg, PGPERR_CONFIG, PGPMSG_CONFIG_MISSING_EQUAL,
	2, &msgarg1, &msgarg2);

					return PGPERR_CONFIG;
			}
			line++; /* skip '=' */

			/* Skip assignment and following whitespace */
			line += skipWhite(line);

			switch (opt->flags & OPT_TYPEMASK) {
			case OPT_BINARY:
					i = getWord(line, 0);
					if ((i == 3 && xmemicmp("off", line, 3) == 0) ||
					(i == 1 && line[0] == '0'))
					{
							if (opt->flags & OPT_FUNC)
									(void)configCallFunci(env, opt->var, opt->min,
											 pri);
							else
									(void)pgpenvSetInt(env,
											 (enum PgpEnvInts)opt->var,
											 opt->min, pri);
							break;
					} else if ((i == 2 && xmemicmp("on", line, 2) == 0) ||
					(i == 1 && line[0] == '1'))
					{
							if (opt->flags & OPT_FUNC)
									(void)configCallFunci(env, opt->var, opt->max,
											 pri);
							else
							(void)pgpenvSetInt(env,
								 	(enum PgpEnvInts)opt->var,
								 	opt->max, pri);
					break;
} else if (i == 0) {
					ui->message (arg, PGPERR_CONFIG,
						PGPMSG_CONFIG_MISSING_BOOLEAN, 2,
						&msgarg1, &msgarg2);
					break;
			}
/* Fall through: default case */
msgarg3.type = PGP_UI_ARG_BUFFER;
msgarg3.val.buf.buf = (byte *)line;
msgarg3.val.buf.len = i;

ui->message (arg, PGPERR_CONFIG,
	PGPMSG_CONFIG_UNKNOWN_BOOLEAN, 3, &msgarg1,
	&msgarg2, &msgarg3);
return PGPERR_CONFIG;

				case OPT_INT:
					val = strtol (line, &p, 0);
					i = getWord (line, 0);

msgarg3.type = PGP_UI_ARG_BUFFER;
msgarg3.val.buf.buf = (byte *)line;
msgarg3.val.buf.len = i;

if (!isspace(*p) && *p != '\0') {
			if (i) {
				 ui->message
				 	(arg, PGPERR_CONFIG,
				 	PGPMSG_CONFIG_UNKNOWN_INTEGER, 3,
				 	&msgarg1, &msgarg2, &msgarg3);
			} else {
					ui->message
						(arg, PGPERR_CONFIG,
						PGPMSG_CONFIG_MISSING_INTEGER, 2,
						&msgarg1, &msgarg2);
			}
			return PGPERR_CONFIG;
	}
msgarg4.type = PGP_UI_ARG_INT;
msgarg4.val.i = val;
if (val > (long)opt->max) {
			msgarg5.type = PGP_UI_ARG_INT;
			msgarg5.val.i = opt->max;
			ui->message (arg, PGPERR_CONFIG,
				 PGPMSG_CONFIG_INT_TOO_HIGH, 4,
				 &msgarg1, &msgarg2, &msgarg4, &msgarg5);
			val = (long)opt->max;
} else if (val < (long)opt->min) {
			msgarg5.type = PGP_UI_ARG_INT;
			msgarg5.val.u = opt->min;
			ui->message (arg, PGPERR_CONFIG,
				 PGPMSG_CONFIG_INT_TOO_LOW, 4,
				 &msgarg1, &msgarg2, &msgarg4, &msgarg5);
			val = (long)opt->min;
	}
if ((opt->flags & OPT_FUNC) == 0) {
	(void)pgpenvSetInt(env, (enum PgpEnvInts)opt->var,
			(int)val, pri);
} else if (configCallFunci(env, opt->var, (int)val, pri) < 0) {
			ui->message (arg, PGPERR_CONFIG,
				 PGPMSG_CONFIG_INVALID_INTEGER, 3,
				 &msgarg1, &msgarg2, &msgarg3);
			return PGPERR_CONFIG;
	}
					break;

				case OPT_STRING:
					if (*line == '"') {
						i = getQString(ui, arg, linenum, ++line);
						if (i == (unsigned)-1)
							 return PGPERR_CONFIG;
					} else {
						i = getString(line);
					}

					msgarg3.type = PGP_UI_ARG_BUFFER;
					msgarg3.val.buf.buf = (byte *)line;
					msgarg3.val.buf.len = i;

					if (i > (unsigned)opt->max-1) {
							msgarg4.type = PGP_UI_ARG_INT;
							msgarg4.val.i = opt->max-1;
							ui->message (arg, PGPERR_CONFIG,
								 PGPMSG_CONFIG_STRING_TOO_LONG, 3,
								 &msgarg1, &msgarg2, &msgarg4);
							return PGPERR_CONFIG;
							/* Call function to set: never any need to copy */
					}
					{
							char c;
							int status;

							c = line[i];
							line[i] = '\0';	/* Null-terminate the string */

							if (opt->flags & OPT_FUNC)
									status = configCallFuncs (env, opt->var,
											 	line, i, pri);
							else
									status = pgpenvSetString
										(env, (enum PgpEnvStrings) opt->var,
										line, pri);

							line[i] = c;	 /* Restore the string */
							if (status >= 0)
								 break;	 /* Success */
							ui->message (arg, PGPERR_CONFIG,
								 PGPMSG_CONFIG_INVALID_STRING, 3,
								 &msgarg1, &msgarg2, &msgarg3);
							return PGPERR_CONFIG;
					}
					break;
			}

			return 0;
}

/*
 * "option" may be modified temporarily, but it is restored prior to
 * returning.
 */
int
pgpConfigLineProcess(struct PgpUICb const *ui, void *arg,
			struct PgpEnv *env, char *option, int pri)
	{
			if (!ui || !ui->message)
				 return PGPERR_UI_INVALID;

			return configLineParse (ui, arg, env, 0, option, pri);
	}

#define CPM_EOF	26	/* ^Z - accept this as a file terminator. */
/*
 * Read a line from file f, into buf, up to the given length. Anything after
 * that is ignored. Strips trailing spaces and line terminator, can read
 * LF, CRLF and CR terminated lines. Also accepts CPM_EOF (^Z) as a file
 * terminator.
 */
static char *
configLineGet(FILE *f, char *buf, unsigned len)
	{
			int c;
			char *p = buf;

			/* Skip leading whitespace */
			do {
				 c = getc(f);
			} while (c == ' ' || c == '\t');

			while (len > 0 && c != '\n' && c != '\r' && c != EOF && c != CPM_EOF)
			{
					*p++ = c;
					--len;
					c = getc(f);
			}

			/* Return NULL on EOF */
			if (p == buf && (c == EOF || c == CPM_EOF)) {
					*buf = '\0';
					return (char *)0;
			}
		
			/*
			* Skip to end of line, setting len to non-zero if anything trailing is
			* not a space (meaning that any trailing whitespace in the buffer is
			* not trailing whitespace on the line and should not be stripped).
			*/
			len = 0;
			while (c != '\n' && c != '\r' && c != EOF && c != CPM_EOF) {
				 len |= c ^ ' ';	/* Avoid branches for fast processors */
				 c = getc(f);
			}

			/* Put back CPM_EOF, and character after \r if it's not \n */
			if (c == CPM_EOF || (c == '\r' && (c = getc(f)) != '\n'))
				 ungetc(c, f);
			/* If line wasn't too long, skip trailing WS and null-terminate */
			if (!len) { /* Skip trailing whitespace, as described above */
				 while (p >= buf && p[-1] == ' ')
				 	--p;
				 *p = '\0';
		}
		return buf;
}

int
pgpConfigFileProcess(struct PgpUICb const *ui, void *arg,
				 struct PgpEnv *env, char const *filename, int pri)
	{
			FILE *f;
			unsigned linenum = 0;
			unsigned errors = 0;
			int i;
			char buf[130];	/* Maximum allowable line size */
			struct PgpUICbArg msgarg1, msgarg2, msgarg3;

if (!ui || !ui->message)
	return PGPERR_UI_INVALID;

f = fopen (filename, "r");
if (f == NULL) {
			msgarg1.type = PGP_UI_ARG_STRING;
			msgarg1.val.s = filename;
			ui->message (arg, PGPERR_CONFIG,
				 PGPMSG_CONFIG_NO_FILE, 1, &msgarg1);
			return 0;	/* treat like empty config file */
	}

buf[sizeof(buf)-1] = '\0';
while (errors < MAX_ERRORS && configLineGet(f, buf, sizeof(buf))) {
			++linenum;
			msgarg1.type = PGP_UI_ARG_UNSIGNED;
			msgarg1.val.u = linenum;
			msgarg2.type = PGP_UI_ARG_STRING;
			msgarg2.val.s = buf;

			/* Line too long? */
			if (buf[sizeof(buf)-1] != '\0') {
					buf[sizeof(buf)-1] = '\0';
					ui->message (arg, PGPERR_CONFIG,
							PGPMSG_CONFIG_LINE_TOO_LONG, 2,
							&msgarg1, &msgarg2);
					errors++;
					continue;
			}
					/* Illegal character? */
					for (i = 0; buf[i] != '\0'; i++) {
						if (!isprint(buf[i]) && buf[i] != '\t')
							break;
					}
					if (buf[i] != '\0') {
							msgarg3.type = PGP_UI_ARG_INT;
							msgarg3.val.i = i;
							ui->message (arg, PGPERR_CONFIG,
									PGPMSG_CONFIG_BAD_CHAR, 3,
									&msgarg1, &msgarg2, &msgarg3);
							errors++;
							continue;
				}
if (configLineParse (ui, arg, env, linenum, buf, pri)
				< 0)
		errors++;
}

fclose (f);

if (errors > 0) {

msgarg1.type = PGP_UI_ARG_INT;
msgarg1.val.i = errors;

if (errors >= MAX_ERRORS)
					ui->message (arg, PGPERR_CONFIG,
						PGPMSG_CONFIG_MAX_ERRORS, 1, &msgarg1);
		else
					ui->message (arg, PGPERR_CONFIG,
						PGPMSG_CONFIG_NUM_ERRORS, 1, &msgarg1);

return PGPERR_CONFIG;
}

			return 0;
}
