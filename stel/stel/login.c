/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)login.c	5.1 (Berkeley) 9/27/88";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/file.h>
#include <utmp.h>
#include <stdio.h>
#include <unistd.h>

#ifdef HAS_PATHS_H
#include <paths.h>
#endif /* HAS_PATHS_H */

#ifndef _PATH_UTMP
#define	_PATH_UTMP	"/etc/utmp"
#define	_PATH_WTMP	"/usr/adm/wtmp"
#endif

void
login(ut)
	struct utmp *ut;
{
	register int fd;
	int tty;

	tty = ttyslot();
	if (tty > 0 && (fd = open(_PATH_UTMP, O_WRONLY, 0)) >= 0) {
		(void)lseek(fd, (OFFSET_T)(tty * sizeof(struct utmp)), L_SET);
		(void)write(fd, (char *)ut, sizeof(struct utmp));
		(void)close(fd);
	}
	if ((fd = open(_PATH_WTMP, O_WRONLY|O_APPEND, 0)) >= 0) {
		(void)write(fd, (char *)ut, sizeof(struct utmp));
		(void)close(fd);
	}
}
