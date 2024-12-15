/*____________________________________________________________________________
    pgpAcquireEntropy.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    entropy collection support

    $Id: pgpAcquireEntropy.c,v 1.22.6.1 1999/06/17 18:52:24 heller Exp $
____________________________________________________________________________*/
#include <stdio.h>

/* open(2) and read(2) need these */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#if PGP_UNIX
#include <unistd.h>
#if SLEEP_POLL
#ifdef SVR4
#include <poll.h>
#else
#ifdef LINUX
#include <linux/asm/poll.h>
#endif
#endif
#endif
#endif

/* PGP SDK header files */
/*#include "pgpDialogs.h"*/
/*#include "pgpOptionListPriv.h"*/
#include "pgpUserInterface.h"
#include "pgpRandomPool.h"
#include "pgpKB.h"
#include "pgpEnv.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpUtilities.h"

#include "config.h"
#include "globals.h"
#include "language.h"
#include "prototypes.h"

/* local globals */
static PGPBoolean g_bEntropy = FALSE;

/*
   Entropy collection routine -- posts dialog for
   mouse/keyboard entropy collection.
*/

#define BUFFSIZE 128 /* buffer for read(2)s on /dev/random */

PGPError
pgpAcquireEntropy( struct pgpfileBones *filebPtr, unsigned bits )
{
    PGPContextRef context = filebPtr->mainbPtr->pgpContext;
    PGPEnv *env = pgpContextGetEnvironment( context );
    PGPMemoryMgrRef mmgr = PGPGetContextMemoryMgr( context );
    PGPBoolean bCollectionEnabled ;
    double dPercent ;
    PGPInt32 TotalNeeded, StillNeeded ;
    PGPInt32 RandBits = PGPGlobalRandomPoolGetEntropy();
#ifdef PGP_UNIX
    int fdesc, numread;
    const char *devrandom;
    PGPInt32 pri;
#endif /* PGP_UNIX */

    if (g_bEntropy)
        return kPGPError_ItemAlreadyExists;

    g_bEntropy = TRUE;
    bCollectionEnabled = TRUE;

    /*
       if zero passed in, use default
     */
    if(bits)
        TotalNeeded = bits;
    else
        TotalNeeded = PGPGlobalRandomPoolGetMinimumEntropy();

    /*
       if we have enough already, then just return
     */
    if (TotalNeeded < RandBits)
        return kPGPError_NoErr;

#ifndef RANDOM_DEVICE_UNSUPPORTED
    /*
       We'd like to use the kernel's random number generator and avoid
       having to bother the user to get keyboard input, but there are
       some problems with that.

       The generator itself looks fine: algorithms are well-chosen and
       the author, Theodore T'so, is highly credible. Looking at old
       kernel mailing list stuff, we find the author of PGP's routines
       contributing to the desgin of these.

       However, the generator supports two devices:
         random(4) which delivers only data it is sure of and blocks if
                it hasn't enough randomness
         urandom(4) which always delivers, does not block but may not
                always give fully random data.

       random(4) is recommended for key generation and other high-security
       applications but we want to avoid blocking.

       Our solution is to use non-blocking I/O on random(4) and go to the
       user whenever the device has no data for us. We feed our data,
       some mixture of random(4) output and user-derived data, to the
       PGP routines for building a random pool and creating keys. This
       avoids blocking.

       The user chooses whether to use the random device through
       the config variable RANDOMDEVICE.
     */

    devrandom = pgpenvGetCString( env, PGPENV_RANDOMDEVICE, &pri );

    if( devrandom && *devrandom )
    {
        PGPByte *buffer = PGPNewSecureData( mmgr, BUFFSIZE, 0 );

        /*
           If we have random(4) device, read until it blocks and dump
           results into PGP random pool
         */
        if( (fdesc=open( devrandom, O_RDONLY|O_NONBLOCK)) > 0) {
            char *p ;
            fprintf(filebPtr->pgpout,LANG("Using %s for some input\n"),
                    devrandom);

            while( (numread = read( fdesc, buffer, BUFFSIZE)) > 0)
            {
                for( p = buffer ;  numread > 0 ; numread--, p++ )
                {
                    /*
                       The global random pool estimates a rather low
                       entropy density when we stuff bits in from
                       /dev/random unless we introduce a timing delay.
                     */

/*
   How to introduce an indeterminate delay of a few milliseconds?
   See also: config.h for these settings.
 */
#if SLEEP_POLL
                    int i,n;
                    n=*p;
#ifdef PARANOID_ABOUT_TIMING_ATTACKS
                    /* limit what info can be obtained from timing attacks */
                    *p=0;
                    p++;
#endif
                    for(i=0; i<n; i++)
                        poll(0,0,0);
#endif

#if SLEEP_SELECT
                    /* if you don't have poll(), insert code here
                        to use the select() call. */
                    pgpFixBeforeShip("insert code to delay indeterminate number of milliseconds");
#endif

#if SLEEP_UNKNOWN
                    /* if you want to use the random device,
                    you really should find a way to insert a delay.*/
                    pgpFixBeforeShip("insert code to delay indeterminate number of milliseconds");
#endif

                    PGPGlobalRandomPoolAddKeystroke(*p);
                    *p=0; /* burn it.*/
                }
                RandBits = PGPGlobalRandomPoolGetEntropy();
                StillNeeded = TotalNeeded - RandBits;
            }
        }
        PGPFreeData(buffer);
    }

#endif /* !RANDOM_DEVICE_UNSUPPORTED */

    StillNeeded = TotalNeeded - RandBits;
    if(StillNeeded > 0)
    {
        fprintf(filebPtr->pgpout,LANG("\n\
PGP needs to generate some random data. This is done by measuring\n\
the time intervals between your keystrokes. Please enter some\n\
random text on your keyboard until the indicator reaches 100%%.\n\
Press ^D to cancel\n"));

        /* Go into CBreak Mode */
        kbCbreak(0);

        dPercent = 0.0;
        while (bCollectionEnabled) {
            int c ;
            if (RandBits > 0)
                dPercent = ((double) RandBits/TotalNeeded) * 100;
            if (dPercent > 100.5)
                dPercent = 100;
            if (dPercent >= 99.5 && StillNeeded > 0)
                dPercent = 99.0;

            fprintf(filebPtr->pgpout,"\r");
            fprintf(filebPtr->pgpout,LANG("%.0f%% of required data   "),
                    dPercent);
            fflush(filebPtr->pgpout);

            RandBits = PGPGlobalRandomPoolGetEntropy();
            StillNeeded = TotalNeeded - RandBits;

            if (StillNeeded <= 0) {
                bCollectionEnabled = FALSE;
                fprintf(filebPtr->pgpout,"\r");
                fprintf(filebPtr->pgpout,LANG("100%% of required data   "));
                fflush(filebPtr->pgpout);
                fprintf(filebPtr->pgpout,"\n");
                break;
            }
            kbFlush(0);
            c = kbGet(); /* get key and add to random pool */
            if (c == 0x04) {
                g_bEntropy = FALSE;
                fprintf(filebPtr->pgpout,
                        LANG("\a\nUser cancelled collection. \n"));
                kbFlush(1);
                kbNorm();
                return kPGPError_UserAbort;
            }
            PGPGlobalRandomPoolAddKeystroke(c);
        }
        fprintf(filebPtr->pgpout,LANG("\aEnough, thank you.\n"));

        kbFlush(1);
        kbNorm();
    }
    g_bEntropy = FALSE;
    return kPGPError_NoErr;
}
