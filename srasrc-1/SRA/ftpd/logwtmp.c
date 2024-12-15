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
 *
 */

#ifndef lint
static char sccsid[] = "@(#)logwtmp.c	5.7 (Berkeley) 2/25/91";
#endif /* not lint */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utmp.h>
#include <unistd.h>
#include <string.h>
#ifdef SOLARIS2
#include <syslog.h>
#include <utmpx.h>
#endif

static int fd = -1;
static int fdx = -1;

/*
 * Modified version of logwtmp that holds wtmp file open
 * after first call, for use with ftp (which may chroot
 * after login, but before logout).
 */
#ifndef SOLARIS2
logwtmp(line, name, host)
	char *line, *name, *host;
{
	struct utmp ut;
	struct stat buf;
	time_t time();
	char *strncpy();
#define WTMPFILE "/var/adm/wtmp"

	if (fd < 0 && (fd = open(WTMPFILE, O_WRONLY|O_APPEND, 0)) < 0)
		return;
	if (fstat(fd, &buf) == 0) {
		(void)strncpy(ut.ut_line, line, sizeof(ut.ut_line));
		(void)strncpy(ut.ut_name, name, sizeof(ut.ut_name));
		(void)strncpy(ut.ut_host, host, sizeof(ut.ut_host));
		(void)time(&ut.ut_time);
		if (write(fd, (char *)&ut, sizeof(struct utmp)) !=
		    sizeof(struct utmp))
			(void)ftruncate(fd, buf.st_size);
	}
}
#else
logwtmp(char *line, char *name, char *host)
{
    struct utmp ut;
    struct utmpx utx;
    struct stat buf;
    struct stat bufx;
    time_t time(time_t *);
    char *strncpy(char *, const char *, size_t);

    if (fd < 0 && (fd = open("/var/adm/wtmp", O_WRONLY | O_APPEND, 0)) < 0) {
        syslog(LOG_ERR, "wtmp %s %m", "/var/adm/wtmp");
        return;
    }
    if (fdx < 0 && (fdx = open(WTMPX_FILE, O_WRONLY | O_APPEND, 0)) < 0){
	 syslog(LOG_ERR, "wtmpx %s %m", UTMPX_FILE);
	 return;
    }
    if (fstat(fd, &buf) == 0 && fstat(fdx, &bufx) == 0) {
#ifdef UTMAXTYPE
        memset((void *)&ut, 0, sizeof(ut));
	memset((void *)&utx, 0, sizeof(utx));
        (void) strncpy(ut.ut_user, name, sizeof(ut.ut_user));
	(void) strncpy(utx.ut_user, name, sizeof(utx.ut_user));
        (void) strncpy(ut.ut_id, "ftp", sizeof(ut.ut_id));
	(void) strncpy(utx.ut_id, "ftp", sizeof(utx.ut_id));
        (void) strncpy(ut.ut_line, line, sizeof(ut.ut_line));
	(void) strncpy(utx.ut_line, line, sizeof(utx.ut_line));
        ut.ut_pid = getpid();
	utx.ut_pid = ut.ut_pid;
        if (name && *name){
            ut.ut_type = USER_PROCESS;
	    utx.ut_type = USER_PROCESS;
        }
        else {
            ut.ut_type = DEAD_PROCESS;
	    utx.ut_type = DEAD_PROCESS;
        }
        ut.ut_exit.e_termination = 0;
	utx.ut_exit.e_termination = 0;
        ut.ut_exit.e_exit = 0;
	utx.ut_exit.e_exit = 0;
#else
        (void) strncpy(ut.ut_line, line, sizeof(ut.ut_line));
        (void) strncpy(ut.ut_name, name, sizeof(ut.ut_name));
#endif /* UTMAXTYPE */

/*        (void) strncpy(ut.ut_host, host, sizeof(ut.ut_host));*/
	(void) strncpy(utx.ut_host, host, sizeof(utx.ut_host));
	utx.ut_syslen = strlen(host);
	if(utx.ut_syslen > 256)
	     utx.ut_syslen = 256;

        (void) time(&ut.ut_time);
	(void) gettimeofday(&utx.ut_tv);
	utx.ut_session = getsid(0);
        if (write(fd, (char *) &ut, sizeof(struct utmp)) !=
            sizeof(struct utmp))
              (void) ftruncate(fd, buf.st_size);
	else if(write(fdx, (char *)&utx, sizeof(struct utmpx)) !=
	   sizeof(struct utmpx)){
	     (void) ftruncate(fd, buf.st_size);
	     (void) ftruncate(fdx, bufx.st_size);
	}
    }
}
#endif
