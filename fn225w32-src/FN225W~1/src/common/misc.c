/*
 # $Id: misc.c,v 1.5 1998/03/10 11:39:42 fbm Exp fbm $
 # Copyright (C) 1997-2000 Farrell McKay
 # All rights reserved.
 #
 # This file is part of the Fortify distribution, a toolkit for
 # upgrading the cryptographic strength of the Netscape web browsers.
 #
 # This toolkit is provided to the recipient under the
 # following terms and conditions:-
 #   1.  This copyright notice must not be removed or modified.
 #   2.  This toolkit may not be reproduced or included in any commercial
 #       media distribution, or commercial publication (for example CD-ROM,
 #       disk, book, magazine, journal) without first obtaining the author's
 #       express permission.
 #   3.  This toolkit, or any component of this toolkit, may not be
 #       used, resold, redeveloped, rewritten, enhanced or otherwise
 #       employed by a commercial organisation, or as part of a commercial
 #       venture, without first obtaining the author's express permission.
 #   4.  Subject to the above conditions being observed (1-3), this toolkit
 #       may be freely reproduced or redistributed.
 #   5.  To the extent permitted by applicable law, this software is
 #       provided "as-is", without warranty of any kind, including
 #       without limitation, the warrantees of merchantability,
 #       freedom from defect, non-infringement, and fitness for
 #       purpose.  In no event shall the author be liable for any
 #       direct, indirect or consequential damages however arising
 #       and however caused.
 #   6.  Subject to the above conditions being observed (1-5),
 #       this toolkit may be used at no cost to the recipient.
 #
 # Farrell McKay
 # Wayfarer Systems Pty Ltd		contact@fortify.net
 */

#include "includes.h"
#include "misc.h"
#include "morpher.h"
#include "callback.h"
#include "msgs.h"

void *
f_malloc(size_t sz)
{
	void	*p;
	p = malloc(sz);
	if (p == (void *)0)
		report(R_FATAL, Msgs_get("MFATALMEM"));
	return p;
}

void *
f_realloc(void *s, size_t sz)
{
	void	*p;
	p = s? realloc(s, sz): malloc(sz);
	if (p == (void *)0)
		report(R_FATAL, Msgs_get("MFATALMEM"));
	return p;
}

void *
f_calloc(size_t n, size_t sz)
{
	void	*p;
	p = calloc(n, sz);
	if (p == (void *)0)
		report(R_FATAL, Msgs_get("MFATALMEM"));
	return p;
}

char *
f_strdup(const char *s)
{
	char	*p;
	p = (char *) f_malloc(strlen(s) + 1);
	strcpy(p, s);
	return p;
}

char *
f_strndup(const char *s, int n)
{
	char	*p;
	p = (char *) f_calloc(n+1, sizeof(char));
	strncpy(p, s, n);
	return p;
}

#ifdef macintosh
int
strcasecmp(const char *s1, const char *s2)
{
	for (; tolower(*s1) == tolower(*s2); s1++, s2++)
		if (*s1 == '\0')
			return 0;
	return (*s1 - *s2);
}
#endif

char *
seg_name(int i)
{
	if (i == SEG_TEXT)
		return "text";
	if (i == SEG_DATA)
		return "data";
	if (i == SEG_RODATA)
		return "rodata";
	return "?unknown?";
}

int
isnumeric(char *s)
{
	while (isdigit(*s))
		s++;
	return (*s == '\0');
}

int
zread(int fd, char *b, int n, char *msg)
{
	int	i;

	i = read(fd, b, n);
	if (i != n)
		report(R_FATAL, Msgs_get("MZREADERR"), msg, i, n);
	return i;
}

void
zlseek(int fd, off_t off, char *msg)
{
	off_t	o;

	o = lseek(fd, off, SEEK_SET);
	if (o != off)
		report(R_FATAL, Msgs_get("MZLSEEKERR"), msg, (int) o, (int) off);
}

unsigned short
toShort(unsigned char *p)
{
	return (unsigned short) (p[0] | (p[1] << 8));
}

unsigned long
toLong(unsigned char *p)
{
	return (unsigned long) ((p[0]) | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}

char *
nextToken(char *p)
{
	if (*p == '"') {
		for (++p; *p && *p != '"';)
			p++;
		if (*p == '"')
			p++;
	}
	else {
		while(*p && !isspace(*p))
			p++;
	}
	while (isspace(*p))
		p++;
	return p;
}

char *
getProgDir(char *arg0)
{
	char	*progName = PROG_NAME;
	char	*progDir;
	char	*progEnd;
	int len;

	if (*arg0 == '"') {
		for (progEnd = ++arg0; *progEnd && *progEnd != '"';)
			progEnd++;
	}
	else {
		for (progEnd = arg0; *progEnd && *progEnd != ' ';)
			progEnd++;
	}

	len = progEnd - arg0;

	progDir = f_strndup(arg0, len);
	if (len >= (int) strlen(progName)) {
		if (f_stricmp(progDir + len - strlen(progName), progName) == 0) { 
			len -= strlen(progName);
			progDir[len] = '\0';
		}
	}
	if (len > 0 && progDir[len-1] == PATH_SEP)
		progDir[--len] = '\0';

	if (len == 0) {
		free(progDir);
		progDir = f_strdup(".");
	}

	return progDir;
}

int
unescape(char *src, int len)
{
	register char	*ptr = src;
	char		*end = ptr + len;
	char		*tgt = ptr;
	char		ch;

	while (ptr < end) {
		if (*ptr != '\\')
			*tgt++ = *ptr++;
		else if (++ptr < end) {
			ch = *ptr++;
			if (ch == '\\')
				*tgt++ = '\\';
			else if (ch == '"')
				*tgt++ = '"';
			else if (ch == 'n')
				*tgt++ = '\n';
			else if (ch == 'r')
				*tgt++ = '\r';
			else if (ch == 't')
				*tgt++ = '\t';
			else if (isoctal(ch)) {
				ch -= '0';
				if (ptr < end && isoctal(*ptr))
					ch = (ch << 3) + (*ptr++ - '0');
				if (ptr < end && isoctal(*ptr))
					ch = (ch << 3) + (*ptr++ - '0');
				*tgt++ = ch;
			}
			else
				*tgt++ = ch;
		}
	}
	return (tgt - src);
}

void
lftocr(char *s)
{
	for (; *s; s++)
		if (*s == '\n')
			*s = '\r';
}
