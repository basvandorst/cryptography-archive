/*____________________________________________________________________________
    more.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    display a secure buffer or a file to support
    For-your-eyes-only mode.

	$Id: more.c,v 1.15 1999/05/12 21:01:05 sluu Exp $
____________________________________________________________________________*/
#include <stdio.h>
#include <ctype.h>
#if PGP_UNIX
#include <unistd.h>
#endif

#include "pgpBase.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"

#include "globals.h"
#include "config.h"
#include "prototypes.h"
#include "fileio.h"
#include "language.h"

#ifdef WIN32
#include "windows.h"
#endif /* WIN32 */

#ifdef MSDOS
#ifndef __GO32__
#include <conio.h>
#endif
#define DEFAULT_LINES 25 /* MSDOS actually has a 25-line screen */
#else
#define DEFAULT_LINES 24
#endif /* MSDOS */
#define DEFAULT_COLUMNS 80


/* Clears screen and homes the cursor. */
void clearScreen(struct pgpfileBones *filebPtr)
{
    fprintf(filebPtr->pgpout,
            "\n\033[0;0H\033[J\r           \r");
    /* near-universal ANSI sequence, erased if it didnt work. */
    fflush(filebPtr->pgpout);
}

PGPError getScreenSize(struct pgpfileBones *filebPtr )
{ /* Rot bilong kargo */
    /* Return the screen size */
#ifndef WIN32
    char *envLines, *envColumns;
    long rowTemp = 0, colTemp = 0;
#else /* WIN32 */
    HKEY        hKey = NULL;
    PGPUInt32   dwValue = 0, dwType = REG_DWORD, dwLen = sizeof(PGPUInt32);
#endif /* WIN32 */
#if PGP_UNIX
#ifdef USE_TERMCAP
    char termBuffer[TERMBUFSIZ], *termInfo;
#endif
#ifdef TIOCGWINSZ
    struct winsize windowInfo;
#else
#ifdef TIOCGSIZE
    struct ttysize windowInfo;
#else
#ifdef WIOCGETD
    struct uwdata windowInfo;
#endif /* WIOCGETD */
#endif /* TIOCGSIZE */
#endif /* TIOCGWINSZ */

    /* Make sure that we're outputting to a terminal */
    if (!isatty(fileno( filebPtr->pgpout ))) {
        filebPtr->screenRows = DEFAULT_LINES;
        filebPtr->screenCols = DEFAULT_COLUMNS;
        return kPGPError_NoErr;
    }
    filebPtr->screenRows = filebPtr->screenCols = 0;
#endif /* UNIX */

#ifndef WIN32
    /* LINES & COLUMNS environment variables override everything else */
    envLines = getenv("LINES");
    if (envLines != NULL && (rowTemp = atol(envLines)) > 0)
        filebPtr->screenRows = (int) rowTemp;
    envColumns = getenv("COLUMNS");
    if (envColumns != NULL && (colTemp = atol(envColumns)) > 0)
        filebPtr->screenCols = (int) colTemp;
#else /* WIN32 */
	/* Read console size from registry */
	if (RegOpenKeyEx( HKEY_CURRENT_USER, "Console", 0, KEY_READ, &hKey ) != ERROR_SUCCESS)
    {
        /* can't open the key, just set it to 80x25 */
        filebPtr->screenRows = 25;
        filebPtr->screenCols = 80;
        return kPGPError_NoErr;
    }
    if (RegQueryValueEx( hKey, "WindowSize", NULL, &dwType, 
						 (LPBYTE)&dwValue, &dwLen) != ERROR_SUCCESS)
    {
        filebPtr->screenRows = 25;
        filebPtr->screenCols = 80;
        RegCloseKey(hKey);
        return kPGPError_NoErr;
    }
    filebPtr->screenRows = HIWORD(dwValue) - 1;
    filebPtr->screenCols = LOWORD(dwValue);
    RegCloseKey(hKey);
#endif /* WIN32 */


#if PGP_UNIX
#ifdef TIOCGWINSZ
    /* See what ioctl() has to say (overrides terminfo & termcap) */
    if ((!filebPtr->screenRows || !filebPtr->screenCols) &&
        ioctl(fileno( filebPtr->pgpout ), TIOCGWINSZ, &windowInfo)
        != -1) {
        if (!filebPtr->screenRows && windowInfo.ws_row > 0)
            filebPtr->screenRows = (int) windowInfo.ws_row;

        if (!filebPtr->screenCols && windowInfo.ws_col > 0)
            filebPtr->screenCols = (int) windowInfo.ws_col;
    }
#else
#ifdef TIOCGSIZE
    /* See what ioctl() has to say (overrides terminfo & termcap) */
    if ((!filebPtr->screenRows || !filebPtr->screenCols) &&
        ioctl(fileno( filebPtr->pgpout ), TIOCGSIZE, &windowInfo) != -1) {
        if (!filebPtr->screenRows && windowInfo.ts_lines > 0)
            filebPtr->screenRows = (int) windowInfo.ts_lines;

        if (!filebPtr->screenCols && windowInfo.ts_cols > 0)
            filebPtr->screenCols = (int) windowInfo.ts_cols;
    }
#else
#ifdef WIOCGETD
    /* See what ioctl() has to say (overrides terminfo & termcap) */
    if ((!filebPtr->screenRows || !filebPtr->screenCols) &&
        ioctl(fileno( filebPtr->pgpout ), WIOCGETD, &windowInfo) != -1) {
        if (!filebPtr->screenRows && windowInfo.uw_height > 0)
            filebPtr->screenRows = (int) (windowInfo.uw_height
                    / windowInfo.uw_vs);

        if (!filebPtr->screenCols && windowInfo.uw_width > 0)
            filebPtr->screenCols = (int) (windowInfo.uw_width
                    / windowInfo.uw_hs);
    } /* You are in a twisty maze of standards, all different */
#endif
#endif
#endif

#ifdef USE_TERMCAP
    /* See what terminfo/termcap has to say */
    if (!filebPtr->screenRows || !filebPtr->screenCols) {
        if ((termInfo = getenv("TERM")) == (char *) NULL)
            termInfo = UNKNOWN_TERM;

        if ((tgetent(termBuffer, termInfo) <= 0))
            strcpy(termBuffer, DUMB_TERMBUF);

        if (!filebPtr->screenRows && (rowTemp = tgetnum("li")) > 0)
            filebPtr->screenRows = (int) rowTemp;

        if (!filebPtr->screenCols && (colTemp = tgetnum("co")) > 0)
            filebPtr->screenCols = (int) colTemp;
    }
#endif
    if (filebPtr->screenRows == 0) /* nothing worked, use defaults */
        filebPtr->screenRows = DEFAULT_LINES;
    if (filebPtr->screenCols == 0)
        filebPtr->screenCols = DEFAULT_COLUMNS;
#endif /* UNIX */

   return kPGPError_NoErr;
}

/*
   display the line worth of text from the buffer.
 */
PGPError moreLineFromBuffer(struct pgpfileBones *filebPtr,
        const char *textbuffer, PGPSize start, PGPSize limit, PGPSize *next)
{
    PGPSize i;
    int st;


    for(i=start; i<limit; i++)
    {
        int c = textbuffer[i];

        /*
          if the char is printable, do so, otherwise what?
         */
        if( isprint( c ) || c == '\n' || c == '\r' ) {
            if( (st= fputc( c, filebPtr->pgpout))==EOF) {
                /* err.*/
            }
            filebPtr->currentCol++;
        } else {
            /* maybe print something?*/
            if( c == '\t' )
                c = ' ';
            else
                c = '\\';
            if( (st= fputc( c, filebPtr->pgpout))==EOF) {
                /* err.*/
            }
            filebPtr->currentCol++;
        }

        if( c=='\r' ) {
            filebPtr->currentCol=0;
        }

        if( c=='\n' ) {
            char scratch[1];

            filebPtr->currentCol=0;
            filebPtr->currentRow++;
            if( filebPtr->currentRow < filebPtr->screenRows )
            {
                *next = i+1;
                return kPGPError_NoErr;
            }

            /* wait...*/
            pgpTtyGetString(scratch, 1, filebPtr->pgpout);
            filebPtr->currentRow=0;
        }
    }
    *next = limit;
    return kPGPError_NoErr;
}

/*
   if piping, close the pipe.
   otherwise finalize state.
 */
PGPError moreFinish( struct pgpfileBones *filebPtr )
{
    if( !filebPtr->moreon )
        return kPGPError_ItemNotFound;

#if PGP_UNIX
    if(filebPtr->piping) {
        pclose(filebPtr->pgpout);
        filebPtr->pgpout = filebPtr->savepgpout;
        filebPtr->piping = FALSE;
    }
#endif

    filebPtr->moreon=FALSE;
    return kPGPError_NoErr;
}

/*
   if PAGER is defined, open the pipe.
   otherwise initialize state.
 */
PGPError moreStart( struct pgpfileBones *filebPtr )
{
    struct pgpenvBones *envbPtr=filebPtr->envbPtr;
    PGPError err;
#if PGP_UNIX
    FILE *out;
#endif

    if( filebPtr->moreon ) {
        err = moreFinish( filebPtr );
        pgpAssertNoErr(err);
    }

#if PGP_UNIX
    if( envbPtr->pager[0]!='\0' && strcmp("pgp", envbPtr->pager)) {
        /* if pager is available, use it.*/
        if ((out = popen(envbPtr->pager, "w")) != NULL) {
            filebPtr->piping = TRUE;
            filebPtr->moreon = TRUE;

            filebPtr->savepgpout = filebPtr->pgpout;
            filebPtr->pgpout = out;
            return kPGPError_NoErr;
        }
        perror("popen");
    }
#endif

    /* otherwise send it to the screen...*/

    fprintf(filebPtr->pgpout, LANG("Just a moment...\n"));
    fflush(filebPtr->pgpout);

    getScreenSize( filebPtr );

    filebPtr->currentRow=0;
    filebPtr->currentCol=0;
    filebPtr->moreon=TRUE;

    return kPGPError_NoErr;
}

/*
   stuff the next buffer down the pipe.
 */
PGPError moreBuffer(struct pgpfileBones *filebPtr, const char *textbuffer,
        PGPSize length)
{
    PGPError err;
    int pos;

    if( !filebPtr->moreon ) {
        err = moreStart( filebPtr );
        if( IsPGPError(err) )
            return err;
    }

    if( filebPtr->piping ) {
        while( length > 0 ) {
            pos = fwrite( textbuffer, 1, length, filebPtr->pgpout );
            textbuffer += pos;
            length -= pos;
            if( pos == 0 )
                break;
        }
        return kPGPError_NoErr;
    }

    /* for each line of text in the buffer, write it to the screen;*/
    /* save state in the filebPtr: the current line number*/

    for( pos=0; pos < length; ) {
        err = moreLineFromBuffer(filebPtr, textbuffer, pos, length, &pos );
        if( err == kPGPError_UserAbort )
            break;
        pgpAssertNoErr(err);
    }
    return err;
}

/*
 */
int moreFile(struct pgpfileBones *filebPtr, char *fileName)
{
    PGPContextRef context = filebPtr->mainbPtr->pgpContext;
    PGPMemoryMgrRef mmgr = PGPGetContextMemoryMgr( context );
    FILE *fp;
    char *textbuffer = NULL;
    PGPSize actual,tbLength;
    PGPError err,er2;

    if( (fp = fopen(fileName, FOPRTXT ))==NULL) {
        return -1;
    }

    tbLength = kMaxMoreBufferLength < 1024 ? kMaxMoreBufferLength : 1024;

    textbuffer = PGPNewSecureData( mmgr, tbLength, 0 );
    if( !textbuffer ) {
        err = kPGPError_OutOfMemory;
        goto done;
    }
    pgpAppendToPointerList( filebPtr->mainbPtr->leaks, textbuffer );

    err = moreStart( filebPtr );
    pgpAssertNoErr(err);

    /* while there is more to do, grab a textbuffers' worth*/
    /* and stuff it down the pipe.*/

    while( !feof( fp ) ) {
        actual = fread( textbuffer, 1, tbLength, fp);
        err = moreBuffer(filebPtr, textbuffer, actual );
        if( err == kPGPError_UserAbort )
            break;
        pgpAssertNoErr(err);
    }

    er2 = moreFinish( filebPtr );
    pgpAssertNoErr(err);

done:
    if( IsPGPError(err) )
        pgpShowError(filebPtr,err,0,0); /*__FILE__,__LINE__);*/

    if( textbuffer ) {
        PGPFreeData( textbuffer );
        pgpRemoveFromPointerList( filebPtr->mainbPtr->leaks, textbuffer );
    }

    return IsPGPError(err) ? -1 : 0;
}

