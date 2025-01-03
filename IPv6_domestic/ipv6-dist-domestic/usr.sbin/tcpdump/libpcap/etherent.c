/*
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef lint
static char rcsid[] =
    "@(#) /master/usr.sbin/tcpdump/libpcap/etherent.c,v 2.1 1995/02/03 18:11:42 polk Exp (LBL)";
#endif

#include <sys/types.h>

#include <ctype.h>
#include <pcap.h>
#include <pcap-namedb.h>
#include <stdio.h>

#ifndef __GNUC__
#define inline
#endif

static inline int xdtoi(int);
static inline int skip_space(FILE *);
static inline int skip_line(FILE *);

/* Hex digit to integer. */
static inline int
xdtoi(c)
	register int c;
{
	if (isdigit(c))
		return c - '0';
	else if (islower(c))
		return c - 'a' + 10;
	else
		return c - 'A' + 10;
}

static inline int
skip_space(f)
	FILE *f;
{
	int c;

	do {
		c = getc(f);
	} while (isspace(c) && c != '\n');

	return c;
}

static inline int
skip_line(f)
	FILE *f;
{
	int c;

	do
		c = getc(f);
	while (c != '\n' && c != EOF);

	return c;
}

struct pcap_etherent *
pcap_next_etherent(FILE *fp)
{
	register int c, d, i;
	char *bp;
	static struct pcap_etherent e;
	static int nline = 1;
 top:
	while (nline) {
		/* Find addr */
		c = skip_space(fp);
		if (c == '\n')
			continue;
		/* If this is a comment, or first thing on line
		   cannot be ethernet address, skip the line. */
		else if (!isxdigit(c))
			c = skip_line(fp);
		else {
			/* must be the start of an address */
			for (i = 0; i < 6; i += 1) {
				d = xdtoi(c);
				c = getc(fp);
				if (c != ':') {
					d <<= 4;
					d |= xdtoi(c);
					c = getc(fp);
				}
				e.addr[i] = d;
				if (c != ':')
					break;
				c = getc(fp);
			}
			nline = 0;
		}
		if (c == EOF)
			return 0;
	}

	/* If we started a new line, 'c' holds the char past the ether addr,
	   which we assume is white space.  If we are continuing a line,
	   'c' is garbage.  In either case, we can throw it away. */

	c = skip_space(fp);
	if (c == '\n') {
		nline = 1;
		goto top;
	}
	else if (c == '#') {
		(void)skip_line(fp);
		nline = 1;
		goto top;
	}
	else if (c == EOF)
		return 0;

	/* Must be a name. */
	bp = e.name;
	/* Use 'd' to prevent buffer overflow. */
	d = sizeof(e.name) - 1;
	do {
		*bp++ = c;
		c = getc(fp);
	} while (!isspace(c) && c != EOF && --d > 0);
	*bp = '\0';
	if (c == '\n')
		nline = 1;

	return &e;
}
