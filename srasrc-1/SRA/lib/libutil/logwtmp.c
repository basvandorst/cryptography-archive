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
static char sccsid[] = "@(#)logwtmp.c	5.5 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <utmp.h>

#ifdef SOLARIS2
#include <fcntl.h>
#include <utmpx.h>
#endif


logwtmp(line, name, host)
	char *line, *name, *host;
{
#ifdef SOLARIS2
	struct utmpx utx;

        memset((void *)&utx, 0, sizeof(utx));
        (void) strncpy(utx.ut_user, name, sizeof(utx.ut_user));
        (void) strncpy(utx.ut_id, "ftp", sizeof(utx.ut_id));
        (void) strncpy(utx.ut_line,line,sizeof(utx.ut_line));
        utx.ut_pid = getpid();
        if (name && *name){
            utx.ut_type = USER_PROCESS;
        }
        else {
            utx.ut_type = DEAD_PROCESS;
        }
        utx.ut_exit.e_termination = 0;
        utx.ut_exit.e_exit = 0;
 
        (void) strncpy(utx.ut_host, host, sizeof(utx.ut_host));
        utx.ut_syslen = strlen(host);
        if(utx.ut_syslen > 256)
             utx.ut_syslen = 256;
 
        (void) gettimeofday(&utx.ut_tv);
        utx.ut_session = getsid(0);

	updwtmpx("/var/adm/wtmpx",&utx);

#else
	struct utmp ut;
	struct stat buf;
	int fd;
	time_t time();
	char *strncpy();

	if ((fd = open("/var/adm/wtmp", O_WRONLY|O_APPEND, 0)) < 0)
		return;
	if (fstat(fd, &buf) == 0) {
		(void) strncpy(ut.ut_line, line, sizeof(ut.ut_line));
		(void) strncpy(ut.ut_name, name, sizeof(ut.ut_name));
		(void) strncpy(ut.ut_host, host, sizeof(ut.ut_host));
		(void) time(&ut.ut_time);
		if (write(fd, (char *)&ut, sizeof(struct utmp)) !=
		    sizeof(struct utmp))
			(void) ftruncate(fd, buf.st_size);
	}
	(void) close(fd);
#endif
}
