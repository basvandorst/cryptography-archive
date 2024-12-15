/*____________________________________________________________________________
    globals.h

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

	struct wrappers around the former pgp262 global variables

    $Id: globals.h,v 1.18.6.2 1999/06/14 19:01:57 build Exp $
____________________________________________________________________________*/

#ifndef PGP_CMDLINE_GLOBALS

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpEnv.h"
#include "pgpGroups.h"

#include "lists.h"

#include "config.h"

struct pgpenvBones {
     struct pgpmainBones *mainbPtr;
     PGPEnv *m_env; /* owned by the pgpContext */

     char pager[MAX_PATH+1]; /* file lister command */
     char keyserverURL[MAX_PATH+1];

     pgpPassphraseListRef passwds;

     PGPBoolean interactiveAdd; /* ask user whether to ad each key to
                                    keyring */
     int versionByte;

     PGPBoolean moreFlag;  /* for your eyes only */
     PGPBoolean filterMode;
     PGPBoolean noManual;
     int makeRandom; /* If non-zero, initialize file to this many random
                        bytes */

     PGPBoolean compatible; /* try to maintain user interface
                               compatibility with PGP2.6.2 */

     PGPBoolean passThrough; /* in filter mode, pass unrecognised text
                                through. */
	 PGPBoolean	bShowpass;	/* echo passphrase */

     /*PGPBoolean verbose;*/
     /*PGPBoolean batchmode;*/
};

struct pgpargsBones {
     struct pgpenvBones *envbPtr;
     struct pgpargsPBones *argsPrivbPtr;
     char *outputFileName;      /* -o outfilename */

     PGPBoolean encryptFlag;    /* -e */
     PGPBoolean decryptOnlyFlag; /* -d */
     PGPBoolean signFlag;       /* -s */
     PGPBoolean keyFlag;        /* -k */
     PGPBoolean groupFlag;      /* -g */
     PGPBoolean conventionalFlag; /* -c */
     PGPBoolean armorFlag;      /* -a */
     PGPBoolean stripSigFlag;   /* -b separate signature */
     PGPBoolean wipeFlag;       /* -w */

     PGPBoolean preserveFileName; /* -p */
     char keyChar;              /* used with -k */
     char groupChar;            /* used with -g */
};

struct pgpfileBones {
     struct pgpenvBones *envbPtr;
     struct pgpmainBones *mainbPtr;

     /* system-wide file extensions... */
     char *PGP_EXTENSION;       /* ".pgp" binary */
     char *ASC_EXTENSION;       /* ".asc" ascii armor */
     char *SIG_EXTENSION;       /* ".sig" separate signature */
     char *BAK_EXTENSION;       /* ".bak" backup */
     char *PKR_EXTENSION;       /* ".pkr" public keyring */
     char *SKR_EXTENSION;       /* ".skr" secret keyring */
     char *HLP_EXTENSION;       /* ".hlp" help */
     char *CONSOLE_FILENAME;    /* "_CONSOLE" */
     char *HELP_FILENAME;       /* "pgp.hlp" */

     char const * const * manualDirs; /* from fileio.c */

     FILE *pgpout; /* Place for routine messages */
     FILE *savepgpout; /* state saved during fyeo */

     PGPBoolean stripSpaces;    /* to control canonicalization */
     /* more state saved during fyeo... */
     PGPBoolean piping,moreon;
     PGPSize screenRows,screenCols;
     PGPSize currentRow,currentCol;
};

struct pgpmainBones {

     PGPContextRef pgpContext;

     struct pgpenvBones *envbPtr;
     struct pgpfileBones *filebPtr;
     struct pgpargsBones *argsbPtr;

     char **recipients;

     PGPKeySetRef workingRingSet;
     PGPGroupSetRef workingGroupSet;
     PGPKeyServerRef workingKeyServer;

     pgpPointerListRef leaks;

     char *relVersion; /* release version */
     char *signonLegalese; /* for RSA banner */

     /* These files use the environmental variable PGPPATH as a default
        path */

     pgpFileNameListRef decodefilenames;
     char plainfilename[MAX_PATH+1];

     char *fyeoBuffer; /* for-your-eyes-only buffer */
     PGPSize fyeoBufferLength;

     /* Flags which are global across the driver code files */
     PGPBoolean signatureChecked;
     PGPBoolean deArmorOnly;
     PGPBoolean decryptMode;
     PGPBoolean outputStdout;
     PGPBoolean separateSignature;

}; /* pgpmainBones */

#ifdef PGP_WIN32 //mjm
#define pgpTtyGetString pgpCLGetString
#endif

#define DLOG(x)  fprintf( filebPtr->pgpout, (x) )
#define USERLOG(x) fprintf( filebPtr->pgpout, (x) )

extern void initMainBones( struct pgpmainBones *mainbPtr,
        PGPContextRef context );
extern void initArgsBones( struct pgpmainBones *mainbPtr );
extern void initFileBones( struct pgpmainBones *mainbPtr );
extern void initEnvBones ( struct pgpmainBones *mainbPtr );

/* from stubs.h */

#define CHECK_ALL 0

#define  MAINT_VERBOSE  0x02
#define  MAINT_CHECK    0x01

#define NO_CONV  0 /* No conversion needed */
#define INT_CONV 1 /* Convert text to internal representation */
#define EXT_CONV 2 /* Convert text to external representation */
#define CONVERSION 0



#define PGP_CMDLINE_GLOBALS
#endif
