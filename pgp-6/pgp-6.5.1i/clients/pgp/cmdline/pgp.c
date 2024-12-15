/*____________________________________________________________________________
    pgp.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    PGP: Pretty Good(tm) Privacy - public key cryptography for the masses.

    $Id: pgp.c,v 1.14 1999/05/12 21:01:05 sluu Exp $
____________________________________________________________________________*/

#include <ctype.h>
#ifndef AMIGA
#include <signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __QNX__
#include <sys/stat.h>
#endif

#include <limits.h>

#include "usuals.h"
//#include "stubs.h"
#include "globals.h"
/*#include "charset.h"*/
#include "language.h"
#include "pgp.h"
#include "exitcodes.h"

#include "pgpErrors.h"
#include "pgpDebug.h"
#include "pgpUtilities.h"
#include "pgpKeyServer.h"

#include "fileio.h"

#ifdef  M_XENIX
char *strstr();
long time();
#endif

#define STACK_WIPE 4096

void initSignals(void);


/* Various compression signatures: PKZIP, Zoo, GIF, Arj, and HPACK.
   Lha(rc) is handled specially in the code; it is missing from the
   compressSig structure intentionally.  If more formats are added,
   put them before lharc to keep the code consistent.
 */
static char *compressSig[] =
{"PK\03\04", "ZOO ", "GIF8", "\352\140",
 "HPAK", "\037\213", "\037\235", "\032\013", "\032HP%",
 "BZh"
 /* lharc is special, must be last */ };
static char *compressName[] =
{"PKZIP", "Zoo", "GIF", "Arj",
 "Hpack", "gzip", "compressed", "PAK", "Hyper",
 "bzip2",
 "LHarc"};
static char *compressExt[] =
{".zip", ".zoo", ".gif", ".arj",
 ".hpk", ".gz", ".Z", ".pak", ".hyp",
 ".bz2",
 ".lzh"};

/* "\032\0??", "ARC", ".arc" */

/* Returns file signature type from a number of popular compression formats
   or -1 if no match. also returns the name and extension. */
int compressSignature(PGPByte * header, char **compressNam,
        char **compressEx )
{
    int i;

    for (i = 0; i < sizeof(compressSig) / sizeof(*compressSig); i++)
        if (!strncmp((char *) header, compressSig[i],
                strlen(compressSig[i])))
        {
            if(compressNam) *compressNam = compressName[i];
            if(compressEx) *compressEx = compressExt[i];
            return i;
        }
    /* Special check for lharc files */
    if (header[2] == '-' && header[3] == 'l' &&
            (header[4] == 'z' || header[4] == 'h') &&
            header[6] == '-')
    {
        if(compressNam) *compressNam = compressName[i];
        if(compressEx) *compressEx = compressEx[i];
        return i;
    }
    return -1;
} /* compressSignature */

/* returns TRUE iff file is likely to be compressible */
PGPBoolean fileCompressible(char *filename)
{
    PGPByte header[8];
    getHeaderInfoFromFile(filename, header, 8);
    if (compressSignature( header, NULL, NULL ) >= 0)
        return FALSE; /* probably not compressible */
    return TRUE; /* possibly compressible */
} /* compressible */

#ifdef SIGINT
/* This function is called if a BREAK signal is sent to the program.  In this
   case we zap the temporary files.
 */
void breakHandler(int sig)
{
    int errorLvl = INTERRUPT;
    extern struct pgpmainBones _pgp_mainBones;
#ifdef PGP_UNIX
    if (sig == SIGPIPE) {
        signal(SIGPIPE, SIG_IGN);
        pgpTearDown( &_pgp_mainBones, &errorLvl );
        exit( errorLvl );
    }
    if (sig != SIGINT)
        fprintf(stderr, LANG("\nreceived signal %d\n"), sig);
    else
#endif
        fprintf(_pgp_mainBones.filebPtr->pgpout,
                LANG("\nStopped at user request\n"));
    pgpTearDown( &_pgp_mainBones, &errorLvl );
    exit( errorLvl );
}
#endif


#ifdef TEMP_VERSION /* temporary experimental version of PGP */
#include <time.h>

#define CREATION_DATE 0x36e9983e
/* CREATION_DATE is Fri Mar 12 22:42:06 1999 UTC */

#define LIFESPAN ((unsigned long) 60L * (unsigned long) 86400L)
/* LIFESPAN is 60 days */

/* If this is an experimental version of PGP, cut its life short */
void checkExpirationDate(void)
{
    if (PGPGetTime() > (CREATION_DATE + LIFESPAN)) {
        fprintf(stderr,
LANG("\n\007This experimental version of PGP has expired.\n"));
        exit(-1); /* error exit */
    }
} /* check_expiration_date */
#else /* no expiration date */
#define checkExpirationDate() /* null statement */
#endif /* TEMP_VERSION */


void initSignals()
{
#ifndef PGP_WIN32
#ifdef MSDOS
#ifndef OS2
#ifdef __TURBOC__
    harderr(dostrap);
#else /* MSC */
#ifndef __GNUC__ /* DJGPP's not MSC */
    _harderr(dostrap);
#endif
#endif
#endif
#endif /* MSDOS */
#endif /* PGP_WIN32 */
#ifdef SIGINT
#ifdef ATARI
    signal(SIGINT, (sigfunc_t) breakHandler);
#else
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)
        signal(SIGINT, breakHandler);
#if defined(PGP_UNIX) || defined(VMS)
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
        signal(SIGHUP, breakHandler);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
        signal(SIGQUIT, breakHandler);
#ifdef PGP_UNIX
    signal(SIGPIPE, breakHandler);
#endif
    signal(SIGTERM, breakHandler);
#ifndef PGP_DEBUG
    signal(SIGTRAP, breakHandler);
    signal(SIGSEGV, breakHandler);
    signal(SIGILL, breakHandler);
#ifdef SIGBUS
    signal(SIGBUS, breakHandler);
#endif
#endif /* PGP_DEBUG */
#endif /* PGP_UNIX */
#endif /* not Atari */
#endif /* SIGINT */
} /* initSignals */


#if defined(PGP_DEBUG) && defined(HAVE_MSTATS)
#include <malloc.h>
#endif

/*
  Tear down PGP: wipes and removes temporary files and secure buffers.

  PGP 2.6.x tried to wipe the stack, but in the event of interrupts it
  wasn't always successful because the exitPGP() function was called by
  the signal handler, rather than from main().

  We try never to put sensitive data directly on the stack, instead we
  maintain a linked list of ``possible'' memory leaks; since pgpTearDown
  may be called from the interrupt handler, there may be some of these
  dangling before we exit.

  And yes we wipe the stack anyway. can't hurt, can it?
 */

void pgpTearDown(struct pgpmainBones *mainbPtr, int *perrorLvl )
{
    char buf[STACK_WIPE];
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;
    PGPContextRef context = mainbPtr->pgpContext;
    PGPBoolean verbose = pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err );

    if( filebPtr->moreon ) {
        moreFinish( filebPtr );
    }

    if( mainbPtr->decodefilenames )
        pgpFreeFileNameList( mainbPtr->decodefilenames );

    /* wipe the secure buffers...*/
    if( envbPtr->passwds ) {
        err = pgpFreePassphraseList( envbPtr->passwds );
        pgpAssertNoErr(err);
        envbPtr->passwds = NULL;
    }

    if( mainbPtr->fyeoBuffer ) {
        PGPFreeData( mainbPtr->fyeoBuffer ); /* for-your-eyes-only buffer*/
        mainbPtr->fyeoBuffer = NULL;
    }

    if( mainbPtr->workingKeyServer ) {
        PGPFreeKeyServer( mainbPtr->workingKeyServer );
        mainbPtr->workingKeyServer = NULL;
    }

    if( mainbPtr->workingGroupSet ) {
        pgpFinalizeWorkingGroupSet( mainbPtr );
        /* mainbPtr->workingGroupSet = NULL;*/
    }

    if( mainbPtr->workingRingSet ) {
        /* note: don't commit changes.*/
        PGPFreeKeySet( mainbPtr->workingRingSet );
        mainbPtr->workingRingSet = NULL;
    }

    /* scram any dangling buffers...*/
    if( mainbPtr->leaks ) {
        void *data;
        PGPSize count;
        pgpCountPointerList( mainbPtr->leaks, &count );
        if(verbose)
            fprintf( filebPtr->pgpout, LANG("\n%d memory frags found\n"),
                    count);

        if( count > 0) {
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("trying to plug leaks.\n"));
            pgpRewindPointer( mainbPtr->leaks );
            pgpNextPointer( mainbPtr->leaks, &data );
            while( data ) {
                PGPFreeData( data );
                pgpNextPointer( mainbPtr->leaks, &data );
            }
        }
        err = pgpFreePointerList( mainbPtr->leaks );
        pgpAssertNoErr(err);
        mainbPtr->leaks = NULL;
    }

    /* wipe temp files...*/
    cleanupTempFiles(filebPtr);

    if (verbose)
        fprintf(filebPtr->pgpout, LANG("exitPGP: exitcode = %d\n"),
                *perrorLvl);

#if defined(PGP_DEBUG) && defined(HAVE_MSTATS)
    if (verbose) {
        struct mstats mstat;
        mstat = mstats();
        printf(LANG("%d chunks used (%d bytes)  %d bytes total\n"),
                mstat.chunks_used, mstat.bytes_used, mstat.bytes_total);
    }
#endif
    memset(buf, 0, sizeof(buf)); /* wipe stack */

#ifdef VMS
    /*
       Fake VMS style error returns with severity in bottom 3 bits
     */
    if (*perrorLvl)
        *perrorLvl = (*perrorLvl << 3) | 0x10000002;
    else
        *perrorLvl = 0x10000001;
#endif /* VMS */

    PGPsdkNetworkLibCleanup();
    PGPFreeContext( context );
    PGPsdkCleanup();
    return; /* we want to return all control to main() before leaving.*/
}
