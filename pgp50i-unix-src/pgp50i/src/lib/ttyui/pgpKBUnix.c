/*
 * pgpKBUnix.c - Unix keyboard input routines.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpKBUnix.c,v 1.1.2.2.2.1 1997/08/18 22:12:28 quark Exp $
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * Define NOTERMIO if you don't have the termios stuff
 */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>

/* How to get cbreak mode */

#if defined(NOTERMIO)
#include <sgtty.h>		/* No termio: Use ioctl() TIOCGETP and TIOCSETP */
#elif defined(SVR2)
#include <termio.h>		/* SVR2: Use ioctl() TCGETA and TCSETAF */
#else /* Usual case */
#include <termios.h>	/* Posix: use tcgetattr/tcsetattr */
#endif

#ifdef sun /* including ioctl.h and termios.h gives a lot of warnings on sun */
#include <sys/filio.h>
#else
#include <sys/ioctl.h>			/* for FIONREAD */
#endif /* sun */

#ifndef FIONREAD
#define	FIONREAD	TIOCINQ
#endif

#include "pgpPOSIX.h"	/* For read(), sleep() */

#include "pgpKB.h"

#if UNITTEST	/* Dummy out some functions for unit testing */
#define randEvent(c)	(void)c
#else
#include "pgpRndPool.h"
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#endif

/* The structure to hold the keyuboard's state */
#if defined(NOTERMIO)
static struct sgttyb kbState0, kbState1;
#ifndef CBREAK
#define CBREAK RAW
#endif
#define KB_GET_STATE(fd,s) ioctl(fd, TIOCGETP, s)
#define KB_ALTER_STATE(s) ((s)->sg_flags |= CBREAK, (s)->sg_flags &= ~ECHO)
#define KB_SET_STATE(fd,s) ioctl(fd, TIOCSETP, s)

#elif defined(SVR2)
static struct termio kbState0, kbState1;
#define KB_GET_STATE(fd,s) ioctl(fd, TCGETA, s)
#define KB_ALTER_STATE(s) \
	((s)->c_cc[VMIN]=1, (s)->c_cc[VTIME]=0, (s)->c_lflag &= ~(ECHO|ICANON))
#define KB_SET_STATE(fd,s) ioctl(fd, TCSETA, s)

#else
static struct termios kbState0, kbState1;
#define KB_GET_STATE(fd,s) tcgetattr(fd, s)
#define KB_ALTER_STATE(s) \
	((s)->c_cc[VMIN]=1, (s)->c_cc[VTIME]=0, (s)->c_lflag &= ~(ECHO|ICANON))
#define KB_SET_STATE(fd,s) tcsetattr(fd, TCSAFLUSH, s)

#endif

static int kbStateFetched = 0;

/* The basic task of getting the terminal into CBREAK mode. */
static void
kbInternalCbreak(int fd)
{
		if (!kbStateFetched) {
				if (KB_GET_STATE(fd, &kbState0) < 0)
					return;	/* Trouble - what to do? */
				kbState1 = kbState0;
				KB_ALTER_STATE(&kbState1);
				kbStateFetched = 1;
		}
		KB_SET_STATE(fd, &kbState1);
}

/* Restore the terminal to normal operation */
static void
kbInternalNorm(int fd)
{
	if (kbStateFetched)
		KB_SET_STATE(fd, &kbState0);
}

/* State variables */
static volatile int kbCbreakFlag = 0;
static int kbFd = -1;

#ifdef SVR2
static int (*savesig)(int);
#else
static void (*savesig)(int);
#endif

/* A wrapper around SIGINT and SIGCONT to restore the terminal modes. */
static void
kbSig1(int sig)
{
		if (kbCbreakFlag)
			kbInternalNorm(kbFd);
		if (sig == SIGINT)
			signal(sig, savesig);
		else
			signal(sig, SIG_DFL);
		raise(sig);	/* Re-send the signal */
}

static void
kbAddSigs(void);

/* Resume cbreak after SIGCONT */
static void
kbSig2(int sig)
{
		(void)sig;
		if (kbCbreakFlag)
				kbInternalCbreak(kbFd);
		else
				kbAddSigs();
}

static void
kbAddSigs(void)
{
	savesig = signal (SIGINT, kbSig1);
#ifdef	SIGTSTP
	signal (SIGCONT, kbSig2);
	signal (SIGTSTP, kbSig1);
#endif
}

static void
kbRemoveSigs(void)
{
	signal (SIGINT, savesig);
#ifdef	SIGTSTP
	signal (SIGCONT, SIG_DFL);
	signal (SIGTSTP, SIG_DFL);
#endif
}


/* Now, at last, the externally callable functions */
int kbOpenKbd(int flags, int InBatchmode) {
    int lkbFD = -1;

    if(!InBatchmode) {
	if((lkbFD = open("/dev/tty", flags)) < 0) {
	    lkbFD = STDIN_FILENO;
	}
    }
    else {
	lkbFD = STDIN_FILENO;
    }
    return(lkbFD);
}

int kbCloseKbd(int fd) {
    int ReturnCode = 0;

    if(fd != STDIN_FILENO) /*We don't actually want to close STDIN*/
		ReturnCode = close(fd);

    return(ReturnCode);
}

void
kbCbreak(int InBatchmode)
{
    if(kbFd < 0) {
	kbFd = kbOpenKbd(O_RDWR, InBatchmode);
    }

    if(!InBatchmode)
    {
	kbAddSigs();
	kbCbreakFlag = 1;
	kbInternalCbreak(kbFd);
    }
}

void
kbNorm(void)
{
		kbInternalNorm(kbFd);
		kbCbreakFlag = 0;
		kbRemoveSigs();
}

int
kbGet(void)
{
		int i;
	char c;

	i = read(kbFd, &c, 1);
	if (i < 1)
		return -1;
	pgpRandPoolKeystroke(c);
	return (unsigned char)c;
}

/*
 * Flush any pending input. If "thorough" is set, tries to be more
 * thorough about it. Ideally, wait for 1 second of quiet, but we
 * may do something more primitive.
 *
 * kbCbreak() has the side effect of flushing the inout queue, so this
 * is not too critical.
 */
void
kbFlush(int thorough)
{
	if (thorough)
		sleep(1);
#if defined(TCIFLUSH)
	tcflush(kbFd, TCIFLUSH);
#elif defined(TIOCFLUSH)
#ifndef FREAD
#define FREAD 1	/* The usual value */
#endif
	ioctl(kbFd, TIOCFLUSH, FREAD);
#endif
}


#if UNITTEST	/* Self-contained test driver */

#include <ctype.h>

int
main(void)
{
		int c;

		puts("Going to cbreak mode...");
		kbCbreak();
		puts("In cbreak mode.  Please type.");
		for (;;) {
				c = kbGet();
				if (c == '\n' || c == '\r')
					break;
				printf("c = %d = '%c'\n", c, c);
				kbFlush(isupper(c));
		}
		puts("Returning to normal mode...");
		kbNorm();
		puts("Done.");
		return 0;
}

#endif /* UNITTEST */
