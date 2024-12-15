/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)logout.c	5.5 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <utmp.h>

#if defined(SOLARIS2) || defined(__sgi)
#ifndef USE_UTMPX
#define USE_UTMPX
#endif
#endif

#ifdef USE_UTMPX
#include <unistd.h>
#include <fcntl.h>
#include <utmpx.h>
#define L_INCR SEEK_CUR
#define _PATH_UTMP "/var/adm/utmp"
#else
#define _PATH_UTMP "/etc/utmp"
#endif


logout(line)
register char *line;
{
#ifdef USE_UTMPX

	register int fd;
	struct utmp ut;
	struct utmpx utx;
	int rval;
	off_t lseek();
	time_t time();

	if ((fd = open("/var/adm/utmp", O_RDWR)) < 0)
		return(0);
	rval = 0;
	while (read(fd, (char *)&ut, sizeof(ut)) == sizeof(ut)) {
		if (!ut.ut_name[0] || 
		    strncmp(ut.ut_line, line, sizeof(ut.ut_line)))
			continue;
		ut.ut_type = DEAD_PROCESS;
		(void)time(&ut.ut_time);
		(void)lseek(fd, -(long)sizeof(ut), L_INCR);
		(void)write(fd, (char *)&ut, sizeof(ut));
		rval = 1;
	}
	(void)close(fd);

	if ((fd = open("/var/adm/utmpx", O_RDWR)) < 0)
		return(0);
	while (read(fd, (char *)&utx, sizeof(utx)) == sizeof(utx)) {
		if (!utx.ut_name[0] || 
		    strncmp(utx.ut_line, line, sizeof(utx.ut_line)))
			continue;
		utx.ut_type = DEAD_PROCESS;
		(void)gettimeofday(&utx.ut_tv);
		(void)lseek(fd, -(long)sizeof(utx), L_INCR);
		(void)write(fd, (char *)&utx, sizeof(utx));
		rval = 1;
	}
	return(rval);

#else
	register int fd;
	struct utmp ut;
	int rval;
	off_t lseek();
	time_t time();

	if ((fd = open(_PATH_UTMP, O_RDWR)) < 0)
		return(0);
	rval = 0;
	while (read(fd, (char *)&ut, sizeof(ut)) == sizeof(ut)) {
		if (!ut.ut_name[0] || 
		    strncmp(ut.ut_line, line, sizeof(ut.ut_line)))
			continue;
		memset(ut.ut_name, 0, sizeof(ut.ut_name));
		memset(ut.ut_host, 0, sizeof(ut.ut_host));
		(void)time(&ut.ut_time);
		(void)lseek(fd, -(long)sizeof(ut), L_INCR);
		(void)write(fd, (char *)&ut, sizeof(ut));
		rval = 1;
	}
	(void)close(fd);
	return(rval);
#endif
}
