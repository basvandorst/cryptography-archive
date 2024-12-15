/*
 * pgpExit.c -- This code is for a runtime app.  It is *NOT* threaded,
 * in any sense of the word.  It stores state in static variables
 * and wipes system stack and heap when exiting from the application.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpExit.c,v 1.4.2.4.2.11 1997/08/23 00:42:46 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>
#include <string.h>		/* For memset() */
#include <stdio.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>		/* for getenv() */
#endif
#ifdef UNIX
#include <sys/types.h>		/* For sbrk() */
#elif defined(MSDOS)
#include <malloc.h>		/* For stackavail(), alloca() */
#if MSC_VER >= 800
/*
 * See flame elsewhere about Microsoft's stupid ideas about the name spaces
 * that are reserved by ANSI C if you do something undefined by ANSI like
 * include an undefined header (like <malloc.h>).
 */
#define alloca _alloca
#define stackavail _stackavail
#endif /* MSC_VER >= 800 */
#endif /* MSDOS */

#include "pgpExit.h"
#include "pgpKeyRings.h"
#include "pgpEnv.h"
#include "pgpRndPool.h"
#include "pgpRndSeed.h"
#include "pgpTimeDate.h"
#include "pgpVersion.h"
#include "pgpOutput.h"

static void *stack0 = NULL, *heap0 = NULL;
static struct PgpEnv *env0 = NULL;
static int program_name = 0;
static int newversion = 0;  /* whether keyrings should be rewritten as
			       latest version (1) or original version (0) */

#if (LICENSE == 2) /*Beta License*/
/* Expiry date in days - defined for beta versions */
#define EXPIRY_DATE pgpDateFromYMD(1997, 10, 19)
#endif

/*
 * This usually returns, but will exit if this is an old beta version that
 * shouldn't be in use any more.
 */
void
exitExpiryCheck (struct PgpEnv *env)
{
#ifdef EXPIRY_DATE
	int tzFix = pgpenvGetInt (env, PGPENV_TZFIX, NULL, NULL);
        unsigned today = (unsigned)(pgpTimeStamp (tzFix)/86400);
        unsigned expires = EXPIRY_DATE;

        if (expires >= today) {
                expires -= today;
                if (expires < 30)
			fprintf (stderr,
"This beta version of PGP will expire in %u days\n", expires);
                return;
	}
	fputs ("\aThis beta evaluation version of PGP has expired.\n", stderr);
                exitCleanup(PGPEXIT_VERSION);
#endif
}

void
exitUsage (int code)
{
	switch (program_name) {
	case EXIT_PROG_PGPE:
		fputs ("\n\
pgpe [[-r <recip1> -r <recip2>] [-s [-u <myid>]] | [-c]] [-afqtvz]\n\
     <file1> [-o outfile1] <file2> [-o <outfile2>]\n\
\n\
PGP Encrypt file(s)\n\
\n\
-a                ASCII armoring\n\
-c                Conventional Encryption (IDEA only; is mutually exclusive\n\
                  with -s, -u and -r)\n\
-f                Filter Mode; Read from stdin to stdout\n\
-o <output file>  Output file for most recent input file\n\
-q                Quiet mode\n\
-r <userid>       UserID to encrypt to.  May be specified multiple times.\n\
-s                Sign, as well as encrypt (use pgps to just sign).  If no\n\
                  userid is specified with -u, the default userid is used.\n\
-t                Text mode\n\
-u <userid>       UserID of the key you wish to sign with.  May only be\n\
                  specified once.\n\
-v                Verbose\n\
-z                Batch mode (assumes no user interaction; not yet\n\
                  implemented.\n\n\
--license         Display usage license\n\
Other programs in this suite include pgps to sign, pgpv to decrypt/verify,\n\
and pgpk for key management.\n", stdout);
		break;

	case EXIT_PROG_PGPS:
		fputs ("\n\
pgps [-u userid] [-bafqtvz] <file1> [-o <outfile> file2 [-o <outfile2>]]\n\
\n\
PGP Sign file(s)\n\
\n\
-a                ASCII armoring\n\
-b                Detached signature\n\
-f                Filter Mode; Read from stdin to stdout\n\
-o <output file>  Output file for most recent input file\n\
-q                Quiet mode\n\
-t                Text mode\n\
-u <userid>       UserID of the key you wish to sign with.  May only be\n\
                  specified once.  If not specified, your default signing\n\
                  key will be used.\n\
-v                Verbose\n\
-z                Batch mode (assumes no user interaction; not yet\n\
                  implemented.)\n\n\
--license         Display usage license\n\
Other programs in this suite include pgpe to encrypt, pgpv to\n\
decrypt/verify, and pgpk for key management.\n", stdout);
		break;

	    case EXIT_PROG_PGPV:
		fputs ("\n\
pgpv [-dfKmqvz] <file1> [-o <outfile> file2 -o <outfile2>]\n\
\n\
Decrypt/Verify PGP encrypted and/or signed file(s)\n\
\n\
-f                Filter Mode; Read from stdin to stdout\n\
-K                Do not process any keys that are present (normally pgpv\n\
                  will add keys to your keyring if found in an input file)\n\
-m                More Mode; display using pager rather than saving\n\
-o <output file>  Output file for most recent input file\n\
-q                Quiet mode\n\
-v                Verbose\n\
-z                Batch mode (assumes no user interaction; not yet\n\
                  implemented).\n\n\
--license         Display usage license\n\
Other programs in this suite include pgpe to encrypt, pgps to sign,\n\
and pgpk for key management.\n", stdout);
		break;

	    case EXIT_PROG_PGPO:
		fputs ("\n\
pgp_old [-dfKmqvz] <file1> [-o <outfile> file2 -o <outfile2>]\n\
\n\
Old-style PGP 2.6.2 emulation command-line\n\
\n\
--license         Display usage license\n\
Sadly, this isn't implemented yet.\n", stdout);
		break;

/*XXX Rewrite this to be more Unix-like*/
	case EXIT_PROG_PGPK:
		fputs ("\n"\
"Usage summary:  (for full details, see the User's guide)\n"\
"\n"\
"To generate your own unique public/private key pair:\n"\
"   pgpk -g [<userid> DSS|RSA <keysize> <userid> <validity> <passphrase>]\n"\
"To add a key file's contents to your public or private key ring:\n"\
"   pgpk -a keyfile [keyfile ...]\n"\
"To remove a key from your public and private key ring:\n"\
"   pgpk -r userid  (or pgpk -rk userid)\n"\
"To remove a user ID from your public and private key ring:\n"\
"   pgpk -ru userid\n"\
"To remove a signature from your public key ring:\n"\
"   pgpk -rs userid\n"\
"To edit your user ID or pass phrase:\n"\
"   pgpk -e your_userid\n"\
"To edit the confidence you have in a person as an introducer:\n"\
"   pgpk -e her_userid\n"\
"To extract (copy) a key from your public key ring:\n"\
"   pgpk -x userid -o keyfile\n"\
"To extract (copy) a key from your public key ring in ascii form:\n"\
"   pgpk -xa userid -o keyfile\n"\
"To list the contents of your key rings:\n"\
"   pgpk -l[l] [userid]\n"\
"To check signatures on your public key ring:\n"\
"   pgpk -c [userid]\n"\
"To sign someone else's public key on your public key ring:\n"\
"   pgpk -s her_userid [-u your_userid]\n"\
"To disable or re-enable some else's public key on your public key ring:\n"\
"   pgpk -d her_userid\n"\
"To permanently revoke your own key on your public and private key rings:\n"\
"   pgpk --revoke your_userid\n"\
"To revoke a signature you made on someone else's key on your pub key ring:\n"\
"   pgpk --revokes your_userid\n"\
"To view the license granted you:\n"\
"   pgpk --license\n"\
"", stdout);
		break;
	default:
		fputs ("\n\
Usage summary unavailable for this program.\n\
\n", stdout);
		break;
	}

	exitCleanup(code);
}

/* attribute(x) expands to nothing unless __GNUC__ is defined. */
static void exitWipe(int code) attribute((noreturn));
static void exitWipe1(int code, void *base) attribute((noreturn));

#ifdef MSDOS

#include <dos.h>	/* FOR MK_FP, FP_SEG and FP_OFF */
#include <malloc.h>	/* for alloca(), stackavail(), _heapwalk() */
/*
 * Now, Microsoft has recently developed a habit of prepending an underscore
 * to the beginning of everything.  They wave their arms and mutter about ANSI.
 * If they have a really broken linker then I can see how they might not be
 * able to deal with user code declaring functions that are also supplied in
 * the library, so they need to put the functions in a name space that user
 * code can't declare in, BUT THIS SURE DOESN'T APPLY TO MACROS.  For any
 * header *not* defined by ANSI (such as, say, <dos.h>, <conio.h>, <io.h>,
 * <unistd.h>, <sys/types.h>, etc.), any structures, unions, macros, enums,
 * and so on - anything not visible to the linker - can be defined any
 * way you like.
 *
 * So I don't understand Microsoft.  Defining prefix-prepended aliases
 * for library-internal calls is fine (so my extern variable "write"
 * won't get called when I call printf()), but to get rid of more convenient
 * names if I *do* include the relevant file?  Wierd.
 */

#ifndef MK_FP
#ifdef _MK_FP
#define MK_FP _MK_FP
#else
#define MK_FP(seg,off) ((void __far *)((seg)<<16 | (off)))
#endif
#endif /* !MK_FP */

#ifndef FP_SEG
#ifdef _FP_SEG
#define FP_SEG _FP_SEG
#else
#define FP_SEG(ptr) ((unsigned)((unsigned long)(ptr) >> 16))
#endif
#endif /* !FP_SEG */

#ifndef FP_OFF
#ifdef _FP_OFF
#define FP_OFF _FP_OFF
#else
#define FP_OFF(ptr) ((unsigned)(ptr))
#endif
#endif /* !FP_OFF */

#if MSC_VER >= 800
#define alloca _alloca
#define stackavail _stackavail
#endif

static void
exitWipe1(int code, void *base)
{
	int status;

	/* Wipe the stack */
	if (stack0)
		memset(base, 0, (char *)(stack0)-(char *)base);
#ifdef UNIX
	/* ACHTUNG!  This code might be dangerous */
	{
		struct _heapinfo info;
		/* Wipe the heap */
		/* First pass - initial wipe with 0xff */
		_heapset(0xff);
		info._pentry = 0;
		while (_heapwalk(&info) == _HEAPOK) {
			if (info._useflag == _USEDENTRY) {
				memset(info._pentry, 0xff, info._size);
				free(info._pentry);
				/* free() mucks up _heapwalk, so restart */
				info._pentry = 0;
			}
		}
		/* Second pass - wipe all free areas */
		_heapset(0);
	}
#endif /* UNIX */
	exit(code);
}

#if __BORLANDC__
#if __BORLANDC <= 0x0400
unsigned _stklen = 16384;
#else
unsigned const _stklen = 16384;
#endif
#endif

static void
exitWipe(int code)
{
	exitWipe1(code, alloca(stackavail()-500));
}

#else	/* !MSDOS */

static void
exitWipe1(int code, void *base)
{
	/* Wipe the stack */
	if (stack0)
		memset(base, 0, ((char *)(stack0)-(char *)base));

#if 0	/* Disabled until stdio problems can be resolved */
	/* Wipe the heap */
	if (heap0) {
		base = sbrk(0);
		memset(heap0, 0, (char *)base-(char *)heap0);
	}
#endif
#ifdef VMS /* On VMS, the bottom 3 bits are a severity */
	code = (1<<28) | (code << 3) | (1 + (code != 0));
#endif
	exit(code);
}

/*
 * This recursively calls itself until it has eaten enough stack,
 * then wipes it.
 * ASSUMPTIONS: it assumes that the stack is contiguous and grows downwards.
 * (This is reasonably portable in practice,
 * even though counterexamples exist.)
 * If these are violated, who knows what will happen!
 */
static void
exitWipe(int code)
{
	char buf[1024];

	if (stack0 && (char *)stack0 - buf < 16384)
		exitWipe(code);	/* Recursive call to fill stack */
	exitWipe1(code, buf);
}

#endif	/* !MSDOS */

void exitVersion(void)
{
    const char *type = "for Personal Privacy";

#if (LICENSE == 1)
    type = "Freeware";
#else
#if (LICENSE == 2)
    type = "Beta";
#endif
#endif

    PrimaryOutputString("PGP %s Version:  %s%s%s%s\n",
			type,
			VERSION_OS,
			VERSION_MAJOR,
			VERSION_MINOR,
			VERSION_SUB);
    exitCleanup(0);
}

void
exitCleanup(int code)
{
	FILE *rs;
	char const *fn;
	/* close everything */

	mainCloseKeyrings (1, newversion);
	
	if (env0) {
		pgpRandPoolKeystroke (code); /* Maybe it's an interrupt? */
		fn = pgpenvGetString (env0, PGPENV_RANDSEED, NULL, NULL);
		rs = fopen (fn, "wb");
		if (rs) {
			pgpRandSeedWrite (rs, NULL, NULL);
			fclose (rs);
		}
		pgpenvDestroy (env0);
	}

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

	exitWipe(code);
}

#ifdef MSDOS
/*
 * DOS Error codes from 0x13 to 0x27 are reported as critical errors,
 * But they are mapped to 0..0x14 before being passed in di.  Some
 * fancy extensions (like networks) can create more.
 * Actually, 0x
 */
static char const * const doserrs[] = {
	"Write protect error$",
	"Unknown unit$",
	"Drive not ready$",
	"Unknown command$",
	"Data error (bad CRC)$",
	"Bad request structure length$",
	"Write protect error$",
	"Seek error$",
	"Unknown media type$",
	"Sector not found$",
	"Printer out of paper$",
	"Write fault$",
	"Read fault$",
	"General failure"
	"Sharing violation$",
	"Lock violation$",
	"Invalid disk change$",
	"FCD unavailable$",
	"System resource exhausted$",
	"Code page mismatch$",
	"Out of input$",
	"Insufficient disk space$",
	"Unknown error$"		/* Catch-all */
};

static char const * const readwrite[] = {
	"Reading", "Writing"
};

static char const * const area[] = {
	"DOS area", "file allocation table", "disk directory", "file area"
};


#ifndef __BORLANDC__
/*
 * Microsoft Visual C somehow distinguishes between "const void __far *"
 * (which works) and "void const __far *" and "void __far const *"
 * (which don't). I have no idea why.  If you move the const after the void,
 * trying to pass a "char const *" (or a "const char *") produces a warning.
 * Ugh!  (It also blows up if the "__far" comes before the "void".  Huh?)
 */
int
bdosptr(int code, const void __far *ptr, int subcode)
{
	union _REGS regs;
	struct _SREGS segregs;
	
	regs.h.ah = code;
	regs.h.al = subcode;
	regs.x.dx = _FP_OFF(ptr);
	segregs.ds = _FP_SEG(ptr);
	_intdosx(&regs, &regs, &segregs);
	return regs.x.cflag ? -1 : regs.x.ax;
}
#endif /* !__BORLANDC__ */

/*
 * The registers are passed in the following order.
 * If ax bit 15 (ah bit 7) is CLEAR, it's a disk error.
 * ah bit 0 is 1 if a write error; 0 if a read error.
 * ah bits 1 and 2 identify the area of the disk with the error:
 *  0 (00) for DOS area
 *  1 (01) for FAT
 *  2 (10) for disk directory
 *  3 (11) for files area
 * al gives the drive code (0 = a:, 1 = b:, etc.)
 *
 * If ax bit 15 (ah bit 7) is SET, it's a non-disk error.
 * Either a character device error or a corrupted in-memory FAT.
 * In this case, the rest of ax is meaningless.
 *
 * In either case, the bottom half of di gives the driver error code.
 * bp:si point to the device driver header responsible
 *
 * If a character device, examine the device attribute word at bp:si+4.
 * If bit 15 is set (character device), you can use the name field.
 * Device driver header:
 * Offset Len Purpose
 *  0     4   Next device link (-1 if no others)
 *  4     2   Device flags
 *  6     2   Strategy function
 *  8     2   Interrupt function
 * 10     8   Device name (space-padded)
 *
 * Flag bits:
 *  0 - standard input device
 *  1 - standard output device
 *  2 - Null devoce
 *  3 - Clock device
 *  4 - Sepcial
 *  5-10 Reserved, MBZ
 * 11 - Device supports removable media
 * 12 - Reserved, MBZ
 * 13 - Non-IBM format
 * 14 - IOCTL
 * 15 - Character device
 *
 * A DOS CRITICAL ERROR HANDLER MAY ONLY USE DOS FUNCTIONS 0-12!
 * Return code is: 0 = _HARDERR_IGNORE, 1 = _HARDERR_RETRY,
 * 2 = _HARDERR_ABORT (int 23), 3 = _HARDERR_FAIL (DOS 3+).
 */
void __far
hardErrHandler(unsigned di, unsigned ax, unsigned __far *devhdr)
{
	char buf[64];
	int i;

	di &= 0xff;
	sprintf(buf, "\r\nDos error %d: $", di);
	bdosptr(9, buf, 0);
	if (di >= sizeof(doserrs)/sizeof(*doserrs))
		di = sizeof(doserrs)/sizeof(*doserrs) - 1;
	bdosptr(9, doserrs[di], 0);
	bdosptr(9, "\r\n$", 0);

	if (ax & 0x8000) {	/* Disk error */
		sprintf(buf, "%s %s of drive %c:\r\n$",
			readwrite[ax>>8 & 1], area[ax>>9 & 3], 'A' + ax & 255);
		bdosptr(9, buf, 0);
	} else {
		if (FP_SEG(devhdr)+1 && FP_OFF(devhdr)+1
		 && (devhdr[2] & 0x8000)) {
			/* Character device */
			i = 8;
			while (((char __far *)devhdr)[10+--i] && i)
				;
			sprintf(buf, "Accessing device \"%.*s\"\r\n$",
			        i, ((char __far *)devhdr)+10);
			bdosptr(9, buf, 0);
		} else {
			/* Error with in-memory FAT */
			bdosptr(9, "Accessing in-memory"\
" file allocation table.\r\n$", 0);
		}
	}
	_hardresume(_HARDERR_FAIL);
}
#endif /* MSDOS */

static int exitSigpipeCount = 0;

static void
exitBreak (int sig)
{
	int code = PGPEXIT_SIGNAL;

#ifdef SIGPIPE
	if (sig == SIGPIPE && ++exitSigpipeCount < 5) {
		(void)signal (sig, exitBreak);
		return;
	}
#endif
#ifdef SIGINT
	if (sig == SIGINT) {
		fputs ("\nStopped at user request.\n", stderr);
		code = PGPEXIT_BREAK;
	} else
#endif
#ifdef SIGABRT
	if (sig == SIGABRT) {
		fputs ("\nInternal error.\n", stderr);
		code = PGPEXIT_INTERNAL;
	} else
#endif
		fprintf (stderr, "\nReceived signal %d.\n", sig);
	exitCleanup (code);
}

/* The signals to trap */
static int const
sigTable[] = {
#ifndef EXPIRY_DATE	/* Do NOT trap on beta versions */
#ifdef SIGABRT
	SIGABRT,
#endif
#endif
#ifdef SIGALRM
	SIGALRM,
#endif
#ifdef SIGBUS
	SIGBUS,
#endif
#ifdef SIGEMT
	SIGEMT,
#endif
#ifdef SIGFPE
	SIGFPE,
#endif
#ifdef SIGHUP
	SIGHUP,
#endif
#ifdef SIGILL
	SIGILL,
#endif
#ifdef SIGINT
	SIGINT,
#endif
#if defined(SIGIOT) && SIGIOT != SIGABRT
	SIGIOT,
#endif
#ifdef SIGPIPE
	SIGPIPE,
#endif
#ifdef SIGQUIT
	SIGQUIT,
#endif
#ifdef SIGSEGV
	SIGSEGV,
#endif
#ifdef SIGSYS
	SIGSYS,
#endif
#ifdef SIGTERM
	SIGTERM,
#endif
#ifdef SIGTRAP
	SIGTRAP,
#endif
#ifdef SIGXCPU
	SIGXCPU,
#endif
#ifdef SIGXFSZ
	SIGXFSZ,
#endif
	-1	/* Terminate list */
};

void
exitSetup (struct PgpEnv *env, void *stacktop, void *heaptop, int prog,
	   int newvers)
{
	int i;

	for (i = 0; sigTable[i] >= 0; i++) {
		if (signal (sigTable[i], SIG_IGN) != SIG_IGN)
			(void)signal (sigTable[i], exitBreak);
	}

	program_name = prog;
	env0 = env;
	stack0 = stacktop;
	newversion = newvers;
#if defined(UNIX) || defined(__BORLANDC__)
	heap0 = heaptop;
#endif

#ifdef MSDOS
	_harderr(hardErrHandler);
#endif
}

void SetProgramName(int prog)
{
    program_name = prog;
}
