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
static char sccsid[] = "@(#)login.c	5.4 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/file.h>
#include <utmp.h>
#include <stdio.h>

#if defined(SOLARIS2) || defined(__sgi)
#ifndef USE_UTMPX
#define USE_UTMPX
#endif
#endif


#ifdef USE_UTMPX
#include <fcntl.h>
#include <utmpx.h>
#endif

void
login(user,host,line)
char *user, *host, *line;
{
#ifdef USE_UTMPX
	struct utmpx utx;
	struct utmp ut;

	memset((void *)&utx, 0, sizeof(utx));
        (void) strncpy(utx.ut_user, user, sizeof(utx.ut_user));
	/*
        (void) strncpy(utx.ut_id, "sra", sizeof(utx.ut_id));
	*/
	/* make a unique ID otherwise Solaris2 doesn't register 
	 * you in the who output after the next user logs in
	 */
        (void) sprintf(utx.ut_id, "%-*s", sizeof(utx.ut_id), 
	                line+strlen(line)-sizeof(utx.ut_id));
	(void) strncpy(utx.ut_line,line,sizeof(utx.ut_line));
        utx.ut_pid = getpid();
        if (user && *user){
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

	getutmp(&utx,&ut);
	pututxline(&utx);
	pututxline(&utx); /* yes -- really twice, otherwise the
				    hostname is not logged */

	endutxent();
	endutent();
	pututxline(&utx);
	endutxent();

#else
	struct utmp ut;
	register int fd;
	int tty;
	off_t lseek();

	strncpy(ut.ut_name,user,sizeof(ut.ut_name));
	strncpy(ut.ut_host,host,sizeof(ut.ut_host));
	strncpy(ut.ut_line,line,sizeof(ut.ut_line));
	time(&ut.ut_time);

	tty = ttyslot();
	if (tty > 0 && (fd = open("/etc/utmp", O_WRONLY|O_CREAT, 0644)) >= 0) {
		(void)lseek(fd, (long)(tty * sizeof(struct utmp)), L_SET);
		(void)write(fd, (char *)&ut, sizeof(struct utmp));
		(void)close(fd);
	}
	if ((fd = open("/var/adm/wtmp", O_WRONLY|O_APPEND, 0)) >= 0) {
		(void)write(fd, (char *)&ut, sizeof(struct utmp));
		(void)close(fd);
	}
#endif
}
