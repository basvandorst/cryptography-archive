/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpCLUtils.c,v 1.2.16.1 1999/07/23 00:17:14 heller Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pgpKB.h"
#include "pgpCLUtils.h"

int
pgpCLGetPass (FILE *fp, char *buffer, int buflen)
{
	int i;
	
	fprintf (fp, "Enter pass phrase (type ^D to cancel): ");
	fflush (fp);
/*	i = pgpTtyGetString (buffer, buflen, ui->showpass ? ui->fp : NULL); */
	i = pgpCLGetString (buffer, buflen, NULL);
	fprintf (fp, "\n");
	fflush (fp);
	return i;
}

int
pgpCLGetString(char *buf, int len, FILE *echo)
{
	int c;
	int n = 0;
	
	kbCbreak(0);
	
	for (;;) {
		if (echo)
			fflush(echo);
		c = kbGet();
		
		if (c == '\r' || c == '\n') {
			c = '\n';
			break;
		} else if (c == '\b' || c == 7 || c == 127) {
			if (n > 0) {
				n--;
				if (echo)
					fputs("\b \b", echo);
			} else
				fputc('\a', echo ? echo : stderr);
		} else if (c == 0x04) {
			if (echo) {
				fputc('\n', echo);
				fflush(echo);
			}
			kbNorm();
			return -1;
		} else if (c == 0x15) {
			fputc('\r', stdout);
			n = 0;
		} else if (c < ' ' || c > 256) {
			fputc('\a', echo ? echo : stderr);
		} else if (n + 1 >= len) {
			fputc('\a', echo ? echo : stderr);
			kbFlush(0);
		} else {
			if (echo)
				fputc(c, echo);
			buf[n++] = (char)c;
		}
	}
	if (echo) {
		fputc('\n', echo);
		fflush(echo);
	}
	buf[n] = '\0';
	
	kbNorm();
	return n;
}
