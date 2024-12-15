/*
 * Copyright (c) 1990 Marco Negri & siLAB Staff
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission of the authors.
 * The authors makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Please send any improvements, bug fixes, useful modifications, and comments
 * to security@ghost.unimi.it
 */
/*
 * $Author: marco $
 * $Id: noterm.c,v 1.3 1991/10/21 16:44:09 marco Exp marco $
 * $Date: 1991/10/21 16:44:09 $
 * $Revision: 1.3 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: noterm.c,v 1.3 1991/10/21 16:44:09 marco Exp marco $";
#endif	lint

#include <fcntl.h>
#include <signal.h>

noterm()
{
#ifndef	DEBUG
#ifdef	NOTERM
	signal(SIGCLD, SIG_IGN);
	switch (fork()) {
		case	-1 :
			perror("fork()");
			exit(1);
		
		case	0 :
#ifdef	TIOCNOTTY
			{
				int	fd;
				if ((fd = open("/dev/tty", O_RDWR)) != -1) {
					(void) ioctl(fd, TIOCNOTTY, 0);
					close(fd);
				}
				setpgrp(0, 0);
			}
#else	TIOCNOTTY
			setpgrp();
#endif	TIOCNOTTY
			break;

		default	:
			exit(0);
	}
#endif	NOTERM
#endif	DEBUG
}
