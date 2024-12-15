From msuinfo!agate!howland.reston.ans.net!pipex!uunet!news.sprintlink.net!demon!pizzabox.demon.co.uk!gtoal Tue Dec 14 13:04:32 1993
Path: msuinfo!agate!howland.reston.ans.net!pipex!uunet!news.sprintlink.net!demon!pizzabox.demon.co.uk!gtoal
Newsgroups: alt.sources
From: gtoal@pizzabox.demon.co.uk ("gtoal)
Reply-To: Graham Toal <gtoal@gtoal.com>
Subject: Unix hardware random-number generator (bsd, maybe others)
Date: Fri, 3 Dec 1993 15:19:08 +0000
Message-ID: <9312031519.AA01429@pizzabox.demon.co.uk>
Sender: usenet@demon.co.uk
Lines: 394

Archive-Name: otp.c

/*
 * This is an *INCREDIBLY SLOW* random number generator, for use in
 * creating one-time pads (if you're a nutter) or a short very random
 * binary key for any program that needs such a thing.  You start it
 * up at the start of the day, and your interactive shell runs (mostly)
 * transparently underneath it.  Everything you type all day long is
 * used to build up the sequence of random numbers.
 *
 * It collects only *ONE BIT* per keystroke.  I thought it was safer
 * that way.  I'm somewhat dubious about the value of pgp's randstir()
 * logic... it seems to me just obfuscation rather than added value.
 *
 * The mechanics of this program are much like the bsd 'script' command
 * (actually it *is* the bsd script command, hacked about a little)
 * so if you know the limitations of script, you know the limitations
 * of this.  When you run it, it adds random bytes to ~/.otp, a file
 * I strongly recommend you create yourself using touch and then chmod 700.
 * (and even then, only on your own single-user home machine)
 *
 * Don't even consider using the output from this program until you've
 * tested the output yourself for randomicity.  I certainly haven't, and won't
 * vouch for it.  Also, the value of AUTOREPEAT_HACK needs to be determined
 * in a smarter way than it is at the moment.  (by eye, from running with
 * diags enabled)
 *
 * However, it's here and you can use it.  It might save someone a day or
 * two writing something equivalent, and I'd welcome any improvements you
 * might make.
 *
 * Graham Toal <gtoal@gtoal.com> 3 Dec 93.
 *
 *
 * NOTE: tested on 386bsd only.  cfmakeraw known to be missing on some
 * other systems.  (No, I don't have a workaround yet, sorry.  If you
 * have a diff for another system, mail me...)
 *
 * Known bugs:  The msgs command doesn't work when this is running.
 *              For (ahem) historical reasons, it invokes csh always
 *              as the shell.  This is daft but I can't be bothered
 *              changing it back.
 *              Autorepeat may or may not fuck it up.
 */


/*
 * Copyright (c) 1980 Regents of the University of California.
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)script.c    5.13 (Berkeley) 3/5/91";
#endif /* not lint */

/*
 * script
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/times.h> /* testing alternatives to clock() */
#include <stdio.h>
#include <errno.h>
#include <paths.h>
#ifndef _PATH_CSHELL
#define _PATH_CSHELL "/bin/csh"
#endif
#ifdef SHELL /* Allows shell to be overridden on the cc command line */
#undef _PATH_CSHELL
#define _PATH_CSHELL SHELL
#endif
#include <pwd.h>

FILE    *otp = NULL;
#define AUTOREPEAT_HACK 10  /* A trial-and-error value that might be dangerous
                               on a different system.  Ideally the program
                               should calibrate itself.  I might add that
                               later, or with luck someone else will :-) */


char    *shell;
int     master;
int     slave;
int     child;
int     subchild;
char    *fname;

struct  termios tt;
struct  winsize win;
int     lb;
int     l;
char    line[] = "/dev/ptyXX";
char	logname[128];

static void output_bit(FILE *f, int bit)
{
  static int byte = 0, bits = 0;
  byte = (byte << 1) | (bit&1);
  bits += 1;
  if (bits == 8) {
    fputc(byte, f);
    byte = 0; bits = 0;
  }
}

static void Add_one_bit_to_otp(int bytes)
{
  struct tms Tms;
  clock_t this_time, diff;
  static clock_t last_time = (clock_t)-1;
  
      /* ARGH!  clock() is too rough-grained!  Most diffs are 0 and only
         the occasional one is 1 !   Better try prz's times() instead... */

  this_time = times(&Tms);

  if (last_time != (clock_t)-1) {
    diff = this_time - last_time;
    if (diff > (clock_t)AUTOREPEAT_HACK) {
    
      /* A real problem here.  If clock() goes up in units of 1, then
         we're fine.  But what if it goes up 1000 at a time?  Unfortunately
         the bsd system I'm writing this on doesn't have the ANSI library
         or I'd know exactly what to do... :-( */

      int data, size;
      data = (int) diff;
      size = sizeof(int) * 8;  /* really * BITS_PER_BYTE */

      /* Split the integer in two, exor the two halves together, then redefine
         the size of the integer as half what it was.  Eventually we get a 
         single bit which is the exor of all the original bits in the integer.
         (Otherwise known as a parity bit :-) ) */

      size /= 2; /* Start off getting 16 bits from 32 or whatever */
      while (size > 0) {
        data = ((data >> size) & ((1 << size) - 1)) ^ (data & ((1 << size) - 1));
        size /= 2;
      }
      /*fprintf(otp, "abs: %8d  diff: %8d  bit: %c (%d chars read)\n",
        (int)this_time, diff, data+'0', bytes);*/ /* temp for debugging */
      output_bit(otp, data);
    }
  }
  last_time = this_time; /* Note this is deliberately outside
                            the autorepeat test, otherwise
                            they'd just aggregate */
}

static char *home(void)
{
  struct passwd *p;
  if ((p = getpwuid(getuid())) != NULL) return(p->pw_dir);
  return(NULL);
}


main(argc, argv)
        int argc;
        char *argv[];
{
        void finish();
        char *getenv();

        sprintf(logname, "%s/.otp", home());

        otp = fopen(logname, "ab");
	if (otp == NULL) otp = fopen(logname, "wb");
	if (otp == NULL) {
		fprintf(stderr, "Cannot create \"%s\" - %s\n",
		  logname, strerror(errno));
		exit(1);
	}

        fprintf(stderr, "*** OTP generation started on \"%s\".\n", logname);


        getmaster();
        fixtty();

        (void) signal(SIGCHLD, finish);
        child = fork();
        if (child < 0) {
                perror("fork");
                fail();
        }
        if (child == 0) {
                subchild = child = fork();
                if (child < 0) {
                        perror("fork");
                        fail();
                }
                if (child)
                        dooutput();
                else
                        doshell();
        }
        doinput();
}

doinput()
{
        register int cc;
        char ibuf[BUFSIZ];

        while ((cc = read(0, ibuf, BUFSIZ)) > 0) {
                (void) write(master, ibuf, cc);
		if (cc == 1) {
		  /* if cc > 1, typeahead probably, or worse.  Be safe... */
		  /* I *think* this test grabs most autorepeat, thus reducing
		     the danger of getting the AUTOREPEAT_HACK value wrong. */
		  Add_one_bit_to_otp(cc); /* (the cc parameter is for
		                              debug testing if you remove
		                              the cc == 1 above...) */
		}
        }
        done();
}

#include <sys/wait.h>

void
finish()
{
        union wait status;
        register int pid;
        register int die = 0;

        while ((pid = wait3((int *)&status, WNOHANG, 0)) > 0)
                if (pid == child)
                        die = 1;

        if (die)
                done();
}

dooutput()
{
        register int cc;
        time_t tvec, time();
        char obuf[BUFSIZ], *ctime();

        (void) close(0);
        tvec = time((time_t *)NULL);
        if (otp != NULL) (void) fclose(otp);
        for (;;) {
                cc = read(master, obuf, sizeof (obuf));
                if (cc <= 0)
                        break;
                (void) write(1, obuf, cc);
        }
	write(1, "logout\r\n", strlen("logout\r\n"));
        done();
}

doshell()
{
        int t;

        /***
        t = open(_PATH_TTY, O_RDWR);
        if (t >= 0) {
                (void) ioctl(t, TIOCNOTTY, (char *)0);
                (void) close(t);
        }
        ***/
        getslave();
        (void) close(master);
        if (otp != NULL) (void) fclose(otp);
        (void) dup2(slave, 0);
        (void) dup2(slave, 1);
        (void) dup2(slave, 2);
        (void) close(slave);
        execl(_PATH_CSHELL, "csh", 0);
        perror("csh");
        fail();
}

fixtty()
{
        struct termios rtt;

        rtt = tt;
        cfmakeraw(&rtt);
        rtt.c_lflag &= ~ECHO;
        (void) tcsetattr(0, TCSAFLUSH, &rtt);
}

fail()
{

        (void) kill(0, SIGTERM);
        done();
}

done()
{
        if (subchild) {
                (void) close(master);
        } else {
                (void) tcsetattr(0, TCSAFLUSH, &tt);
        }
        exit(0);
}

getmaster()
{
        char *pty, *bank, *cp;
        struct stat stb;

        pty = &line[strlen("/dev/ptyp")];
        for (bank = "pqrs"; *bank; bank++) {
                line[strlen("/dev/pty")] = *bank;
                *pty = '0';
                if (stat(line, &stb) < 0)
                        break;
                for (cp = "0123456789abcdef"; *cp; cp++) {
                        *pty = *cp;
                        master = open(line, O_RDWR);
                        if (master >= 0) {
                                char *tp = &line[strlen("/dev/")];
                                int ok;

                                /* verify slave side is usable */
                                *tp = 't';
                                ok = access(line, R_OK|W_OK) == 0;
                                *tp = 'p';
                                if (ok) {
                                        (void) tcgetattr(0, &tt);
                                        (void) ioctl(0, TIOCGWINSZ, 
                                                (char *)&win);
                                        return;
                                }
                                (void) close(master);
                        }
                }
        }
        fprintf(stderr, "Out of pty's\n");
        fail();
}

getslave()
{

        line[strlen("/dev/")] = 't';
        slave = open(line, O_RDWR);
        if (slave < 0) {
                perror(line);
                fail();
        }
        (void) tcsetattr(slave, TCSAFLUSH, &tt);
        (void) ioctl(slave, TIOCSWINSZ, (char *)&win);
        (void) setsid();
        (void) ioctl(slave, TIOCSCTTY, 0);
}

