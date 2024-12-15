/*____________________________________________________________________________
    config.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    Parses config file for PGP

    Modified 24 Jun 92 - HAJK
    Misc fixes for VAX C restrictions

    Updated by Peter Gutmann to only warn about unrecognized options,
    so future additions to the config file will give old versions a
    chance to still run.  A number of code cleanups, too.

    $Id: config.c,v 1.14.6.1 1999/06/11 17:51:02 sluu Exp $
____________________________________________________________________________*/

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "usuals.h"
#include "pgp.h"
#include "config.h"
#include "globals.h"
#include "fileio.h"
#include "language.h"
/*#include "charset.h"*/

/* from the SDK....*/
#include "pgpSDKPrefs.h"
#include "pgpEnv.h"
#include "pgpContext.h"

/*
 * Set Integer and String configuration parameters.  Strings are
 * copied by the environment; the caller need not preserve them.
 * (They must, of course, be null-terminated.)
 */

static struct pgpenvBones _envBones;

void initEnvBones(struct pgpmainBones *mainbPtr)
{
    PGPEnv *env = pgpContextGetEnvironment( mainbPtr->pgpContext );
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpenvBones *envbPtr = &_envBones;
    char version[256];

    envbPtr->m_env = env;

    pgpenvSetInt( env, PGPENV_ARMOR,  FALSE, PGPENV_PRI_PUBDEFAULT );
    /* armorlines */
    /* max_cert_depth */
    pgpenvSetString( env, PGPENV_CHARSET,  "noconv", PGPENV_PRI_PUBDEFAULT);
    /* charset */

    pgpenvSetInt( env, PGPENV_CLEARSIG,  TRUE, PGPENV_PRI_PUBDEFAULT );
    /* comment */ /*pgpenvGetCString( env, PGPENV_COMMENT, &pri )[128];*/
    /* compl_min */
    pgpenvSetInt( env, PGPENV_COMPRESS,  TRUE, PGPENV_PRI_PUBDEFAULT );
    /* attempt compression before encryption */

    /* companykey */
    pgpenvSetInt( env, PGPENV_ENCRYPTTOSELF,  FALSE, PGPENV_PRI_PUBDEFAULT);
    /* should I encrypt messages to myself? */

    envbPtr->interactiveAdd = FALSE; /* Ask for each key separately if
                                        it should be added to the keyring */

    pgpenvSetString( env, PGPENV_LANGUAGE,  "en", PGPENV_PRI_PUBDEFAULT );
    /* language */

    /* marg_min */
    /* my_name is substring of default userid for secret key to make
       signatures */

    pgpenvSetString( env, PGPENV_MYNAME, "", PGPENV_PRI_PUBDEFAULT );
    /* null my_name means take first userid in ring */

    /* pager */
    {
    char *p=getenv("PAGER");
    strncpy( envbPtr->pager, p ? p : "", MAX_PATH );
    envbPtr->pager[MAX_PATH]='\0';
    }

    /* pkcs_compat */
    /* pgp_mime */
    /* pgp_mimeparse */

    /* randomdevice */

#if PGP_UNIX
    /* randsource */
#endif

	envbPtr->bShowpass = FALSE;

    #ifdef VMS
    /* kludge for those stupid VMS variable-length text records.
        this probably doesn't work anymore. */

    pgpenvSetInt( env, PGPENV_TEXTMODE,  TRUE, PGPENV_PRI_PUBDEFAULT );
    /* MODE_TEXT (TRUE) or MODE_BINARY (FALSE) for literal packet */
    #else                              /* not VMS */
    pgpenvSetInt( env, PGPENV_TEXTMODE,  FALSE, PGPENV_PRI_PUBDEFAULT );
    /* MODE_TEXT or MODE_BINARY for literal packet */
    #endif                             /* not VMS */

    /* tmp */
    /* trusted */
    pgpenvSetInt( env, PGPENV_TZFIX,  0L, PGPENV_PRI_PUBDEFAULT );
    /* seconds from GMT timezone */
    pgpenvSetInt( env, PGPENV_VERBOSE,  FALSE, PGPENV_PRI_PUBDEFAULT );
    /* -l option: display maximum information */
    pgpenvSetInt( env, PGPENV_VERSION,  VERSION_BYTE_NEW,
            PGPENV_PRI_PUBDEFAULT );

    /* ciphernum */
    /* hashnum */
    /* fastkeygen */

    pgpenvSetInt( env, PGPENV_BATCHMODE,  FALSE, PGPENV_PRI_PUBDEFAULT );
    /* if TRUE: don't ask questions */
    pgpenvSetInt( env, PGPENV_FORCE,  FALSE, PGPENV_PRI_PUBDEFAULT );
    /* overwrite existing file without asking */
    /* magic */
    pgpenvSetInt( env, PGPENV_NOOUT,  FALSE, PGPENV_PRI_PUBDEFAULT );


    envbPtr->moreFlag = FALSE;       /* for your eyes only */
    envbPtr->filterMode = FALSE;
    envbPtr->passThrough = FALSE;
    envbPtr->noManual = TRUE;
        /* at the request of Jon Callas, nomanual is disabled. */

    /* If non-zero, initialize file to this many random bytes */
    envbPtr->makeRandom = 0;

#ifdef USER_INTERFACE_COMPATIBLE
    /*
       If PGP is running in an environment where you want user interface
       compatibility with older PGP versions, use this compile-time
       switch To make compatible the default.

       Note that you can specify a value to override this in the config
       file or as a long option on the command line.
     */

    envbPtr->compatible = TRUE;
#else
    envbPtr->compatible = FALSE;
#endif

    pgpNewPassphraseList( context, &envbPtr->passwds );

    sprintf(version, "PGP %s",mainbPtr->relVersion);

    pgpenvSetString( env, PGPENV_VERSION_STRING, version,
            PGPENV_PRI_PUBDEFAULT);

    envbPtr->mainbPtr = mainbPtr;
    mainbPtr->envbPtr = envbPtr;
}

/* Various maximum/minimum allowable settings for config options */

#define MIN_MARGINALS   1
#define MIN_COMPLETE    1
#define MAX_COMPLETE    4
#define MIN_CERT_DEPTH  0
#define MAX_CERT_DEPTH  8

/* Prototypes for local functions */

static int lookup( char *key, 
				   unsigned int keyLength, 
				   char *keyWords[], 
				   unsigned int range );
static int extractToken( char *buffer, int *endIndex, int *length );
static int getaString( char *buffer, int *endIndex );
static int getAssignment( char *buffer, int *endIndex, INPUT_TYPE
        settingType );
static void processAssignment( struct pgpenvBones *envbPtr, int
        intrinsicIndex, PGPInt32 pri);

/* The external config variables we can set here are referenced in pgp.h */

/* Return values */

#define ERROR -1
#define OK     0

/* The types of error we check for */

enum { NO_ERROR, ILLEGAL_CHAR_ERROR, LINELENGTH_ERROR };

#define CPM_EOF  0x1A /* ^Z = CPM EOF char */

#define MAX_ERRORS 3 /* Max.no.errors before we give up */

#define LINEBUF_SIZE 100 /* Size of input buffer */

static int line;  /* The line on which an error occurred */
static int errCount;  /* Total error count */
static PGPBoolean hasError; /* Whether this line has an error in it */

/* The settings parsed out by getAssignment() */

static char str[ LINEBUF_SIZE ];
static int value;
static char *errtag;  /* Prefix for printing error messages */
static char optstr[ 100 ]; /* Option being processed */

/* A .CFG file roughly follows the format used in the world-famous HPACK
   archiver and is as follows:

   - Leading spaces/tabs (whitespace) are ignored.

   - Lines with a '#' as the first non-whitespace character are treated
   as comment lines.

   - All other lines are treated as config options for the program.

   - Lines may be terminated by either linefeeds, carriage returns, or
   carriage return/linefeed pairs (the latter being the DOS default
   method of storing text files).

   - Config options have the form:

   <option> '=' <setting>

   where <setting> may be 'on', 'off', a numeric value, or a string
   value.

   - If strings have spaces or the '#' character inside them they must be
   surrounded by quote marks '"' */

/* Intrinsic variables */

#define NO_INTRINSICS  (sizeof(intrinsics) / sizeof(intrinsics[0]))
#define CONFIG_INTRINSICS BATCHMODE

enum {
    ARMOR, COMPRESS, SHOWPASS, KEEPBINARY, LANGUAGE,
    MYNAME, TEXTMODE, TMP, TZFIX, VERBOSE, BAKRING,
    ARMORLINES, COMPLETES_NEEDED, MARGINALS_NEEDED, PAGER,
    CERT_DEPTH, CHARSET, CLEAR, SELF_ENCRYPT,
    INTERACTIVE, PUBRING, SECRING, RANDSEED, RANDOMDEVICE,
    GROUPSFILE, COMPATIBLE, KEYSERVER_URL, PGP_MIME,
    PGP_MIMEPARSE, COMMENT,
    TRUSTED, VERSION, CIPHERNUM, HASHNUM, FASTKEYGEN,
    /* options below this line can only be used as command line
     * "long" options */
    BATCHMODE, FORCE, PASSTHROUGH, NOMANUAL, MAKERANDOM
};

static char *intrinsics[] = {
    "ARMOR", "COMPRESS", "SHOWPASS", "KEEPBINARY", "LANGUAGE",
    "MYNAME", "TEXTMODE", "TMP", "TZFIX", "VERBOSE", "BAKRING",
    "ARMORLINES", "COMPLETES_NEEDED", "MARGINALS_NEEDED", "PAGER",
    "CERT_DEPTH", "CHARSET", "CLEARSIG", "ENCRYPTTOSELF",
    "INTERACTIVE", "PUBRING", "SECRING", "RANDSEED", "RANDOMDEVICE",
    "GROUPSFILE", "COMPATIBLE", "KEYSERVER_URL", "PGP_MIME",
    "PGP_MIMEPARSE", "COMMENT",
    "TRUSTED", "VERSION", "CIPHERNUM", "HASHNUM", "FASTKEYGEN",
    /* command line only */
    "BATCHMODE", "FORCE", "PASSTHROUGH", "NOMANUAL", "MAKERANDOM"
};

static INPUT_TYPE intrinsicType[] = {
    BOOLE, BOOLE, BOOLE, BOOLE, STRING,
    STRING, BOOLE, STRING, NUMERIC, NUMERIC, STRING,
    NUMERIC, NUMERIC, NUMERIC, STRING,
    NUMERIC, STRING, BOOLE, BOOLE,
    BOOLE, STRING, STRING, STRING, STRING,
    STRING, BOOLE, STRING, BOOLE,
    BOOLE, STRING,
    NUMERIC, NUMERIC, NUMERIC, NUMERIC, BOOLE,
    /* command line only */
    BOOLE, BOOLE, BOOLE, BOOLE, NUMERIC
};

/* Possible settings for variables */

#define NO_SETTINGS   2

static char *settings[] = { "OFF", "ON" };

/* Search a list of keywords for a match */

static int lookup( char *key, 
				   unsigned int keyLength, 
				   char *keyWords[], 
				   unsigned int range )
{
    unsigned int index;
	int position = 0, noMatches = 0;

    strncpy( optstr, key, keyLength );
    optstr[ keyLength ] = '\0';

    /* Make the search case insensitive */
    for( index = 0; index < keyLength; index++ )
        key[ index ] = toUpper( key[ index ] );

    for( index = 0; index < range; index++ )
        if( !strncmp( key, keyWords[ index ], keyLength ) )
        {
            if( strlen( keyWords[ index ] ) == keyLength )
                return index; /* exact match */
            position = index;
            noMatches++;
        }

    switch( noMatches )
    {
        case 0:
            fprintf( stderr, LANG("%s: unknown keyword: \"%s\"\n"),
                    errtag, optstr );
            break;
        case 1:
            return( position ); /* Match succeeded */
        default:
            fprintf( stderr, LANG("%s: \"%s\" is ambiguous\n"),
                    errtag, optstr );
    }
    return ERROR;
}

/* Extract a token from a buffer */

static int extractToken( char *buffer, int *endIndex, int *length )
{
    int index = 0, tokenStart;
    char ch;

    /* Skip whitespace */
    for( ch = buffer[ index ]; ch && ( ch == ' ' || ch == '\t' );
            ch = buffer[ index ] )
        index++;
    tokenStart = index;

    /* Find end of setting */
    while( index < LINEBUF_SIZE && ( ch = buffer[ index ] ) != '\0'
            && ch != ' ' && ch != '\t' )
        index++;
    *endIndex += index;
    *length = index - tokenStart;

    /* Return start position of token in buffer */
    return tokenStart;
}

/* Get a string constant */

static int getaString( char *buffer, int *endIndex )
{
    PGPBoolean noQuote = FALSE;
    int stringIndex = 0, bufferIndex = 1;
    char ch = *buffer;

    /* Skip whitespace */
    while( ch && ( ch == ' ' || ch == '\t' ) )
        ch = buffer[ bufferIndex++ ];

    /* Check for non-string */
    if( ch != '\"' )
    {
        *endIndex += bufferIndex;

        /* Check for special case of null string */
        if( !ch )
        {
            *str = '\0';
            return OK;
        }

        /* Use nasty non-rigorous string format */
        noQuote = TRUE;
    }

    /* Get first char of string */
    if( !noQuote )
        ch = buffer[ bufferIndex++ ];

    /* Get string into string */
    while( ch && ch != '\"' )
    {
        /* Exit on '#' if using non-rigorous format */
        if( noQuote && ch == '#' )
            break;

        str[ stringIndex++ ] = ch;
        ch = buffer[ bufferIndex++ ];
    }

    /* If using the non-rigorous format, stomp trailing spaces */
    if( noQuote )
        while( stringIndex > 0 && str[ stringIndex - 1 ] == ' ' )
            stringIndex--;

    str[ stringIndex++ ] = '\0';
    *endIndex += bufferIndex;

    /* Check for missing string terminator */
    if( ch != '\"' && !noQuote )
    {
        if( line )
            fprintf( stderr,
                    LANG("%s: unterminated string in line %d\n"),
                    errtag, line );
        else
            fprintf( stderr,
                    LANG("unterminated string: '\"%s'\n"), str );
        hasError = TRUE;
        errCount++;
        return ERROR;
    }

    return OK;
}

/* Get an assignment to an intrinsic */

static int getAssignment( char *buffer, int *endIndex, INPUT_TYPE
        settingType )
{
    int settingIndex = 0;
	unsigned int length;
    long longval;
    char *p;

    buffer += extractToken( buffer, endIndex, &length );

    /* Check for an assignment operator */
    if( *buffer != '=' )
    {
        if( line )
            fprintf( stderr, LANG("%s: expected '=' in line %d\n"),
                    errtag, line );
        else
            fprintf( stderr,
                    LANG("%s: expected '=' after \"%s\"\n"),
                    errtag, optstr);
        hasError = TRUE;
        errCount++;
        return ERROR;
    }
    buffer++; /* Skip '=' */

    buffer += extractToken( buffer, endIndex, &length );

    switch( settingType )
    {
        case BOOLE:
            /* Check for known intrinsic - really more general
               than just checking for TRUE or FALSE */
            settingIndex = lookup( buffer, length, settings,
                    NO_SETTINGS );
            if( settingIndex == ERROR )
            {
                hasError = TRUE;
                errCount++;
                return ERROR;
            }

            value = ( settingIndex == 0 ) ? FALSE : TRUE;
            break;

        case STRING:
            /* Get a string */
            getaString( buffer, &length );
            break;

        case NUMERIC:
            longval = strtol(buffer, &p, 0);
            if (p == buffer+length &&
                    longval <= INT_MAX && longval >= INT_MIN) {
                value = (int)longval;
                break;
            }
            if( line )
                fprintf( stderr,
                        LANG("%s: numeric argument expected in line %d\n"),
                        errtag, line );
            else
                fprintf( stderr,
                        LANG("%s: numeric argument required for \"%s\"\n"),
                        errtag, optstr);
            hasError = TRUE;
            errCount++;
            return ERROR;
    }

    return settingIndex;
}

/* Process an assignment */

static void processAssignment( struct pgpenvBones *envbPtr, int
        intrinsicIndex, PGPInt32 pri )
/* pri is either PGPENV_PRI_SYSCONF, PGPENV_PRI_CONFIG, or
   PGPENV_PRI_CMDLINE*/
{
    PGPEnv *env = envbPtr->m_env;
    PGPContextRef context = pgpenvGetContext( env );

    PGPError err;
    if( !hasError )
        switch( intrinsicIndex )
        {
            case ARMOR:
                pgpenvSetInt( env, PGPENV_ARMOR,  value, pri );
                break;

            case ARMORLINES:
                pgpenvSetInt( env, PGPENV_ARMORLINES,  value, pri );
                break;

            case BAKRING:
                pgpenvSetString( env, PGPENV_BAKRING, str, pri );
                break;

            case BATCHMODE:
                pgpenvSetInt( env, PGPENV_BATCHMODE,  value, pri );
                break;

            case CERT_DEPTH:
                if( value < MIN_CERT_DEPTH )
                    value = MIN_CERT_DEPTH;
                if( value > MAX_CERT_DEPTH )
                    value = MAX_CERT_DEPTH;
                pgpenvSetInt( env, PGPENV_CERTDEPTH,  value, pri );
                break;

            case CHARSET:
                pgpenvSetString( env, PGPENV_CHARSET, str, pri );
                break;

            case CLEAR:
                pgpenvSetInt( env, PGPENV_CLEARSIG,  value, pri );
                break;

            case COMPATIBLE:
                envbPtr->compatible = value;
                break;

            case COMMENT:
                pgpenvSetString( env, PGPENV_COMMENT, str, pri );
                break;

            case COMPLETES_NEEDED:
                pgpenvSetInt( env, PGPENV_COMPLETES,  value, pri );
                /* Keep within range */
                if( pgpenvGetInt( env, PGPENV_COMPLETES, &pri, &err )
                        < MIN_COMPLETE )
                    pgpenvSetInt( env, PGPENV_COMPLETES,  MIN_COMPLETE,
                            pri );

                if( pgpenvGetInt( env, PGPENV_COMPLETES, &pri, &err )
                        > MAX_COMPLETE )
                    pgpenvSetInt( env, PGPENV_COMPLETES,  MAX_COMPLETE,
                            pri );

                break;

            case COMPRESS:
                pgpenvSetInt( env, PGPENV_COMPRESS,  value, pri );
                break;

            case FASTKEYGEN:
                pgpenvSetInt( env, PGPENV_FASTKEYGEN,  value, pri );
                break;

            case FORCE:
                pgpenvSetInt( env, PGPENV_FORCE,  value, pri );
                break;

            case INTERACTIVE:
                envbPtr->interactiveAdd = value;
                break;

            case KEEPBINARY:
                /*envbPtr->keepctx = value;*/
                break;

            case KEYSERVER_URL:
                strcpy( envbPtr->keyserverURL, str );
                break;

            case LANGUAGE:
                pgpenvSetString( env, PGPENV_LANGUAGE, str, pri );
                break;


            case MAKERANDOM:
                envbPtr->makeRandom = value;
                break;

            case MARGINALS_NEEDED:
                pgpenvSetInt( env, PGPENV_MARGINALS,  value, pri );
                /* Keep within range */
                if( pgpenvGetInt( env, PGPENV_MARGINALS, &pri, &err )
                        < MIN_MARGINALS )

                    pgpenvSetInt( env, PGPENV_MARGINALS,  MIN_MARGINALS,
                            pri );

                break;

            case MYNAME:
                pgpenvSetString( env, PGPENV_MYNAME, str, pri );
                break;

            case NOMANUAL:
                envbPtr->noManual = value;
                break;

            case PAGER:
                strcpy( envbPtr->pager, str );
                break;

            case PASSTHROUGH:
                envbPtr->passThrough = value;
                break;

            case PGP_MIME:
                pgpenvSetInt( env, PGPENV_PGPMIME,  value, pri );
                break;

            case PGP_MIMEPARSE:
                pgpenvSetInt( env, PGPENV_PGPMIMEPARSEBODY,  value, pri );
                break;

            case PUBRING:
                pgpenvSetString( env, PGPENV_PUBRINGnolongerused, str, pri );
                /*
                   what should we do now?  if priority
                   is config or higher we should set
                   the preferences.
                 */

                if( pri >= PGPENV_PRI_CONFIG )
                {
                    PGPFileSpecRef pubfilespec;
                    char pubring[MAX_PATH+1];
                    strcpy(pubring,str);
                    /*force_extension( pubring, ".pkr", filebPtr);*/
                    err = PGPNewFileSpecFromFullPath( context, pubring,
                            &pubfilespec );

                    pgpAssertNoErr(err);
                    err = PGPsdkPrefSetFileSpec( context,
                            kPGPsdkPref_PublicKeyring, pubfilespec);

                    pgpAssertNoErr(err);
                    PGPFreeFileSpec( pubfilespec );
                }

                break;

            case RANDSEED:
                pgpenvSetString( env, PGPENV_RANDSEEDnolongerused, str,
                        pri );

                if( pri >= PGPENV_PRI_CONFIG )
                {
                    PGPFileSpecRef rndfilespec;
                    char rndfile[MAX_PATH+1];
                    strcpy(rndfile,str);
                    /*force_extension( rndfile, ".pkr", filebPtr);*/
                    err = PGPNewFileSpecFromFullPath( context, rndfile,
                            &rndfilespec );

                    pgpAssertNoErr(err);
                    err = PGPsdkPrefSetFileSpec( context,
                            kPGPsdkPref_RandomSeedFile, rndfilespec);

                    pgpAssertNoErr(err);
                    PGPFreeFileSpec( rndfilespec );
                }
                break;

            case RANDOMDEVICE:
                pgpenvSetString( env, PGPENV_RANDOMDEVICE, str, pri );
                break;

            case SECRING:
                pgpenvSetString( env, PGPENV_SECRINGnolongerused, str, pri );

                if( pri >= PGPENV_PRI_CONFIG )
                {
                    PGPFileSpecRef secfilespec;
                    char secring[MAX_PATH+1];
                    strcpy(secring,str);
                    /*force_extension( secring, ".skr", filebPtr);*/
                    err = PGPNewFileSpecFromFullPath( context, secring,
                            &secfilespec );

                    pgpAssertNoErr(err);
                    err = PGPsdkPrefSetFileSpec( context,
                            kPGPsdkPref_PrivateKeyring, secfilespec);

                    pgpAssertNoErr(err);
                    PGPFreeFileSpec( secfilespec );
                }
                break;

            case GROUPSFILE:

                if( pri >= PGPENV_PRI_CONFIG )
                {
                    PGPFileSpecRef grpfilespec;
                    char grpfile[MAX_PATH+1];
                    strcpy(grpfile,str);
                    /*force_extension( grpfile, ".skr", filebPtr);*/
                    err = PGPNewFileSpecFromFullPath( context, grpfile,
                            &grpfilespec );

                    pgpAssertNoErr(err);
                    err = PGPsdkPrefSetFileSpec( context,
                            kPGPsdkPref_GroupsFile, grpfilespec);

                    pgpAssertNoErr(err);
                    PGPFreeFileSpec( grpfilespec );
                }
                break;

            case SELF_ENCRYPT:
                pgpenvSetInt( env, PGPENV_ENCRYPTTOSELF,  value, pri );
                break;

            case SHOWPASS:
				envbPtr->bShowpass = value;
				break;

            case TEXTMODE:
                if( value )
                    pgpenvSetInt( env, PGPENV_TEXTMODE,  TRUE, pri );
                else
                    pgpenvSetInt( env, PGPENV_TEXTMODE,  FALSE, pri );
                break;

            case TMP:
                /* directory pathname to store temp files */
                pgpAssertAddrValid( envbPtr->mainbPtr, struct pgpmainBones);
                setTempDir( envbPtr->mainbPtr->filebPtr, str );
                break;

            case TRUSTED:
                pgpenvSetInt( env, PGPENV_TRUSTED, value, pri );
                break;

            case CIPHERNUM:
                pgpenvSetInt( env, PGPENV_CIPHER, value, pri );
                break;

            case HASHNUM:
                pgpenvSetInt( env, PGPENV_HASH, value, pri );
                break;

            case TZFIX:
                /* How many hours to add to time() to get GMT.
                   We just compute the seconds from hours to
                   get the GMT shift */
                pgpenvSetInt( env, PGPENV_TZFIX,  3600L * ( long ) value,
                        pri );

                break;

            case VERBOSE:
                if( value < 1 )
                {
                    pgpenvSetInt( env, PGPENV_NOOUT,  TRUE, pri );
                    pgpenvSetInt( env, PGPENV_VERBOSE,  FALSE, pri );
                }
                else
                    if( value == 1 )
                    {
                        pgpenvSetInt( env, PGPENV_NOOUT,  FALSE, pri );
                        pgpenvSetInt( env, PGPENV_VERBOSE,  FALSE, pri );
                    }
                    else
                    {
                        /* Value > 1 */
                        pgpenvSetInt( env, PGPENV_NOOUT,  FALSE, pri );
                        pgpenvSetInt( env, PGPENV_VERBOSE,  TRUE, pri );
                    }
                break;

            case VERSION:
                pgpenvSetInt( env, PGPENV_VERSION, value, pri );
                break;

        }
}

/* Process an option on a line by itself.  This expects options which are
   taken from the command-line, and is less finicky about errors than the
   config-file version */

int processConfigLine( struct pgpenvBones *envbPtr, char *option,
        PGPInt32 pri )
{
    unsigned int index;
	int intrinsicIndex;
    char ch;

    /* Give it a pseudo-linenumber of 0 */
    line = 0;

    errtag = "pgp";
    errCount = 0;
    for( index = 0;
            index < LINEBUF_SIZE && ( ch = option[ index ] ) != '\0' &&
            ch != ' ' && ch != '\t' && ch != '=';
            index++ );
    if( ( intrinsicIndex = lookup( ( char * ) option, index, intrinsics,
            NO_INTRINSICS ) ) == ERROR )
        return -1;
    if( option[ index ] == '\0' && intrinsicType[ intrinsicIndex ] == BOOLE)
    {
        /* Boolean option, no '=' means TRUE */
        value = TRUE;
        processAssignment( envbPtr, intrinsicIndex, pri );
    }
    else
        /* Get the value to set to, either as a string, a numeric
           value, or a PGPBoolean flag */
        if( getAssignment( ( char * ) option + index,
                &index, intrinsicType[ intrinsicIndex ] ) != ERROR )
            processAssignment( envbPtr, intrinsicIndex, pri );

    return errCount ? -1 : 0;
}

/* Process a configuration file */

int processConfigFile( struct pgpenvBones *envbPtr, char *configFileName,
        PGPInt32 pri )
{
    FILE *configFilePtr;
    int ch = 0, theChar;
    int errType, errPos = 0, lineBufCount, intrinsicIndex;
    unsigned int index;
    char inBuffer[ LINEBUF_SIZE ];

    line = 1;
    errCount = 0;
    errtag = fileTail( configFileName );

    if( ( configFilePtr = fopen( configFileName, FOPRTXT ) ) == NULL )
    {
        fprintf( stderr, LANG("Cannot open configuration file %s\n"),
                configFileName );
        return OK; /* Treat it as if it were an empty file */
    }

    /* Process each line in the configFile */
    while( ch != EOF )
    {
        /* Skip whitespace */
        while( ( ( ch = getc( configFilePtr ) ) == ' ' || ch == '\t' )
                && ch != EOF )
            ;

        /* Get a line into the inBuffer */
        hasError = FALSE;
        lineBufCount = 0;
        errType = NO_ERROR;
        while( ch != '\r' && ch != '\n' && ch != CPM_EOF && ch != EOF )
        {
            /* Check for an illegal char in the data */
            if( ( ch < ' ' || ch > '~' ) &&
                    ch != '\r' && ch != '\n' &&
                    ch != ' ' && ch != '\t' && ch != CPM_EOF &&
                    ch != EOF )
            {
                if( errType == NO_ERROR )
                    /* Save pos of first illegal char */
                    errPos = lineBufCount;
                errType = ILLEGAL_CHAR_ERROR;
            }

            /* Make sure the path is of the correct length.  Note
               that the code is ordered so that a LINELENGTH_ERROR
               takes precedence over an ILLEGAL_CHAR_ERROR */
            if( lineBufCount > LINEBUF_SIZE )
                errType = LINELENGTH_ERROR;
            else
                inBuffer[ lineBufCount++ ] = ch;

            if( ( ch = getc( configFilePtr ) ) == '#' )
            {
                /* Skip comment section and trailing
                   whitespace */
                while( ch != '\r' && ch != '\n' &&
                        ch != CPM_EOF && ch != EOF )
                    ch = getc( configFilePtr );
                break;
            }
        }

        /* Skip trailing whitespace and add der terminador */
        while( lineBufCount &&
                ( ( theChar = inBuffer[ lineBufCount - 1 ] ) == ' ' ||
                  theChar == '\t' ) )
            lineBufCount--;
        inBuffer[ lineBufCount ] = '\0';

        /* Process the line unless its a blank or comment line */
        if( lineBufCount && *inBuffer != '#' )
        {
            switch( errType )
            {
                case LINELENGTH_ERROR:
                    fprintf( stderr,
                            LANG("%s: line '%.30s...' too long\n"),
                            errtag, inBuffer );
                    errCount++;
                    break;

                case ILLEGAL_CHAR_ERROR:
                    fprintf( stderr, "> %s\n  ", inBuffer );
                    fprintf( stderr, "%*s^\n", errPos, "" );
                    fprintf( stderr, LANG(
                            "%s: bad character in command on line %d\n"),
                            errtag, line );
                    errCount++;
                    break;

                default:
                    for( index = 0;
                            index < LINEBUF_SIZE &&
                            ( ch = inBuffer[ index ] ) != '\0'
                            && ch != ' ' && ch != '\t'
                            && ch != '=';
                            index++ )
                        /*Do nothing*/ ;

                        /* Try and find the intrinsic.  We
                           don't treat unknown intrinsics as
                           an error to allow older versions to
                           be used with new config files */
                        intrinsicIndex = lookup(inBuffer,
                                index, intrinsics,
                                CONFIG_INTRINSICS );

                    if( intrinsicIndex == ERROR )
                        break;

                    /* Get the value to set to, either as
                       a string, a numeric value, or a
                       PGPBoolean flag */
                    getAssignment( inBuffer + index, &index,
                            intrinsicType[ intrinsicIndex ] );
                    processAssignment( envbPtr, intrinsicIndex, pri );
                    break;
            }
        }

        /* Handle special-case of ^Z if configFile came off an
           MSDOS system */
        if( ch == CPM_EOF )
            ch = EOF;

        /* Exit if there are too many errors */
        if( errCount >= MAX_ERRORS )
            break;

        line++;
    }

    fclose( configFilePtr );

    /* Exit if there were errors */
    if( errCount )
    {
        fprintf( stderr, LANG("%s: %s%d error(s) detected\n\n"),
                configFileName, ( errCount >= MAX_ERRORS ) ?
                LANG("Maximum level of ") : "", errCount );
        return ERROR;
    }

    return OK;
}

