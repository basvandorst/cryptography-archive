/*____________________________________________________________________________
    args.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    Based on PGP262 main module

    $Id: args.c,v 1.20.6.2 1999/06/14 19:41:46 sluu Exp $
____________________________________________________________________________*/


#include <stdio.h>
#include <assert.h>
#include <string.h>
#if PGP_WIN32
#include <io.h>
#endif
#if PGP_UNIX
#include <unistd.h>
#endif

#include "usuals.h"
#include "config.h"
#include "exitcodes.h"
#include "globals.h"
#include "language.h"
#include "pgp.h"
#include "fileio.h"
#include "lists.h"
#include "prototypes.h"
#include "getopt.h"

#include "pgpSDKPrefs.h"
#include "pgpEnv.h"
#include "pgpKeys.h"
#include "pgpPFLErrors.h"
#include "pgpUtilities.h"

static struct pgpargsBones _argsBones;

struct pgpargsPBones {

     /* thse used to be auto variables local to main()...*/
     int myArgc;
     char **myArgv;

     char mcguffin[256]; /* userid search tag */

     PGPBoolean cFlag;
     PGPBoolean uFlag; /* Did I get myName from -u? */

     char *inputFileName;
     char **mcguffins;
     char *workFileName, *tempf;

     char cipherFile[MAX_PATH+1];

     PGPByte ctb;
};

static struct pgpargsPBones argsPriv;

void initArgsPBonesbPtr(struct pgpmainBones *mainbPtr) 
{

     struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
     struct pgpargsPBones *argsPrivbPtr=&argsPriv;
     mainbPtr->argsbPtr->argsPrivbPtr = argsPrivbPtr;
     assert(filebPtr);

  /* private...*/

     argsPrivbPtr->myArgc = 2;

     argsPrivbPtr->cFlag = FALSE;
     argsPrivbPtr->uFlag = FALSE; /* Did I get myName from -u? */

     argsPrivbPtr->inputFileName = NULL;
}

void initArgsBones(struct pgpmainBones *mainbPtr) 
{

     struct pgpargsBones *argsbPtr=&_argsBones;
     mainbPtr->argsbPtr=argsbPtr;

     argsbPtr->encryptFlag = FALSE; /* -e */
     argsbPtr->decryptOnlyFlag = FALSE; /* -d */
     argsbPtr->signFlag = FALSE;    /* -s */
     argsbPtr->keyFlag = FALSE;     /* -k */
     argsbPtr->conventionalFlag = FALSE; /* -c */
     argsbPtr->armorFlag = FALSE;   /* -a */
     argsbPtr->stripSigFlag = FALSE; /* -b */
     argsbPtr->wipeFlag = FALSE;    /* -w */

     argsbPtr->preserveFileName = FALSE;
     argsbPtr->outputFileName = NULL;
     argsbPtr->keyChar = '\0';

     /*mainbPtr->deArmorOnly = FALSE;*/
     argsbPtr->envbPtr=mainbPtr->envbPtr;

     initArgsPBonesbPtr(mainbPtr);
}

void usage(struct pgpfileBones *filebPtr, int *errorLvl);

static void buildHelpFileName(struct pgpfileBones *filebPtr, char
        *helpFileName, char const *extra );

/*
 * The following 2 stub functions are required to
 * interface to the old methods
 */

static int maintUpdate(PGPFileSpecRef ringfilespec, 
					   struct newkey const *nkeys)
{ 
	/* 
	 *No "maintenance pass req'd.  The SDK takes care of this
	 */

	return 0; 
}

static int cryptRandWriteFile(char const *name, 
							  struct IdeaCfbContext *cfb,
							  unsigned bytes) 
{ 
	return -1;	/* not supported in 6.5 */
}

/* comes here if user made a boo-boo. */
void userError(struct pgpfileBones *filebPtr, int *perrorLvl)
{
    fprintf(filebPtr->pgpout, LANG("\nFor a usage summary, type:  pgp -h\n"));
    fprintf(filebPtr->pgpout,
            LANG("For more detailed help, consult the PGP User's Guide.\n"));
    if(!*perrorLvl)
        *perrorLvl = 127;
}

static void argError(struct pgpfileBones *filebPtr, int *perrorLvl)
{
    fprintf(filebPtr->pgpout, LANG("\nInvalid arguments.\n"));
    *perrorLvl = BAD_ARG_ERROR;
}


/* look for a keyring whose name matches the argument.
   return TRUE if found. */
PGPBoolean tryRingFileExtensions ( struct pgpfileBones *filebPtr, char
        *ringfilename )
{
    char tryFileName[MAX_PATH+1];
    strncpy(tryFileName, ringfilename, MAX_PATH);
    tryFileName[MAX_PATH] = '\0';

    if (!fileExists(tryFileName)) {
        defaultExtension( filebPtr, tryFileName, filebPtr->PKR_EXTENSION );
        if (!fileExists(tryFileName)) {
            dropExtension( filebPtr, tryFileName );
            defaultExtension( filebPtr, tryFileName,
                    filebPtr->SKR_EXTENSION );

            if (!fileExists(tryFileName)) {
                return FALSE;
            }
        }
    }

    /* If we are here we have found a legit file. */
    strcpy ( ringfilename, tryFileName);
    return TRUE;
}



int doGroupOpt(struct pgpmainBones *mainbPtr, char groupChar, int *perrorLvl)
{
    struct pgpargsBones *argsbPtr = mainbPtr->argsbPtr;
    struct pgpargsPBones *argsPrivbPtr = argsbPtr->argsPrivbPtr;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPContextRef context = mainbPtr->pgpContext;
    char ringfilename[MAX_PATH];
    char origRingFileName[MAX_PATH];
    PGPFileSpecRef ringfilespec;
    int status;
    PGPError err;
    PGPFileSpecRef pubringSpec;
    char *pubringName;
    PGPKeySetRef keySet = kPGPInvalidRef;

    /*
     * If we're not doing anything that uses stdout, produce output there,
     * in case user wants to redirect it.
     */
    if (!envbPtr->filterMode)
        filebPtr->pgpout = stdout;

    err = PGPsdkPrefGetFileSpec( context, kPGPsdkPref_PublicKeyring,
            &pubringSpec);

    pgpAssertNoErr(err);
    err = PGPGetFullPathFromFileSpec( pubringSpec, &pubringName );
    pgpAssertNoErr(err);

    switch (groupChar) {

    case 'a':
        { /*
             Add a group or keys to a group
             Arguments: groupname [ userids... ]
           */
            char *groupname;

            if (argsPrivbPtr->myArgc < 3 && !envbPtr->filterMode) {
                argError(filebPtr,perrorLvl);
                status=-1;
                goto out;
            }

            groupname = argsPrivbPtr->myArgv[2];

            if (argsPrivbPtr->myArgc >= 4) {
                strcpy(ringfilename, argsPrivbPtr->myArgv[
                        argsPrivbPtr->myArgc - 1 ]);

                if( tryRingFileExtensions( filebPtr, ringfilename ) )
                    argsPrivbPtr->myArgv[ argsPrivbPtr->myArgc - 1 ]=0;
                else
                    strcpy(ringfilename, pubringName);
            } else /* default key ring filename */
                strcpy(ringfilename, pubringName);
#ifdef MSDOS
            strlwr(ringfilename);
#endif
            err = PGPNewFileSpecFromFullPath( context, ringfilename,
                    &ringfilespec);

            pgpAssertNoErr(err);

            err = PGPOpenKeyRing( context, kPGPKeyRingOpenFlags_Mutable,
                    ringfilespec, &keySet);

            pgpAssertNoErr(err);
            mainbPtr->workingRingSet = keySet;


            status = addToGroup(mainbPtr, groupname,
                    &argsPrivbPtr->myArgv[3], ringfilespec);

            if (status < 0) {
                fprintf(filebPtr->pgpout, LANG("\007Group add error. "));
                *perrorLvl = KEYRING_ADD_ERROR;
            }

            /* mainbPtr->workingRingSet is autofreed elsewhere.*/

            if( pubringSpec != ringfilespec) {
                PGPFreeFileSpec(ringfilespec);
            }
            goto out;
        }

/*-------------------------------------------------------*/
    case 'r':
        { /*
             Remove a group or remove keys from a group
             Arguments: groupname [ userids... ]
           */
            char *groupname;

            if (argsPrivbPtr->myArgc < 3 && !envbPtr->filterMode) {
                argError(filebPtr,perrorLvl);
                status=-1;
                goto out;
            }

            groupname = argsPrivbPtr->myArgv[2];

            if (argsPrivbPtr->myArgc >= 4) {
                strcpy(ringfilename,
                        argsPrivbPtr->myArgv[ argsPrivbPtr->myArgc - 1 ]);

                if( tryRingFileExtensions( filebPtr, ringfilename ) )
                    argsPrivbPtr->myArgv[ argsPrivbPtr->myArgc - 1 ]=0;
                else
                    strcpy(ringfilename, pubringName);
            } else /* default key ring filename */
                strcpy(ringfilename, pubringName);

#ifdef MSDOS
            strlwr(ringfilename);
#endif
            err = PGPNewFileSpecFromFullPath( context, ringfilename,
                    &ringfilespec);

            pgpAssertNoErr(err);

            err = PGPOpenKeyRing( context, kPGPKeyRingOpenFlags_Mutable,
                    ringfilespec, &keySet);
            pgpAssertNoErr(err);

            mainbPtr->workingRingSet = keySet;
            status = removeFromGroup(mainbPtr, groupname,
                    &argsPrivbPtr->myArgv[3], ringfilespec);

            if (status < 0) {
                fprintf(filebPtr->pgpout, LANG("\007Group add error. "));
                *perrorLvl = KEYRING_ADD_ERROR;
            }

            /* mainbPtr->workingRingSet is autofreed elsewhere.*/

            if( pubringSpec != ringfilespec) {
                PGPFreeFileSpec(ringfilespec);
            }
            goto out;

        }  /* Remove keys from a group or remove a group
              Arguments: groupname [ userids... ] [ ringfilename ]
            */

/*-------------------------------------------------------*/
    case 'v':
    case 'V': /* -gvv */
        { /* View group entries, with group name match
             Arguments: groupname [ ringfilename ]
           */

            if (argsPrivbPtr->myArgc < 4) /* default key ring filename */
                strcpy(ringfilename, pubringName);
            else
                strcpy(ringfilename, argsPrivbPtr->myArgv[3]);

            if (argsPrivbPtr->myArgc > 2) {
                strcpy(argsPrivbPtr->mcguffin, argsPrivbPtr->myArgv[2]);
                if (strcmp(argsPrivbPtr->mcguffin, "*") == 0)
                    argsPrivbPtr->mcguffin[0] = '\0';
            } else {
                *argsPrivbPtr->mcguffin = '\0';
            }

            if ((argsPrivbPtr->myArgc == 3) &&
                    ( hasExtension(argsPrivbPtr->myArgv[2],
                                   filebPtr->PKR_EXTENSION) ||
                      hasExtension(argsPrivbPtr->myArgv[2],
                              filebPtr->SKR_EXTENSION)) ) {
                strcpy(ringfilename, argsPrivbPtr->myArgv[2]);
                argsPrivbPtr->mcguffin[0] = '\0';
            }
            CONVERT_TO_CANONICAL_CHARSET(argsPrivbPtr->mcguffin);

#ifdef MSDOS
            strlwr(ringfilename);
#endif
            strcpy(origRingFileName, ringfilename);
            tryRingFileExtensions ( filebPtr, ringfilename );

            err = PGPNewFileSpecFromFullPath( context, ringfilename,
                    &ringfilespec);

            pgpAssertNoErr(err);

            /* If a second 'v' (groupChar = V), show keys too */
            status = viewGroups(mainbPtr, argsPrivbPtr->mcguffin,
                    ringfilespec, origRingFileName,
                    (PGPBoolean) (groupChar == 'V') );

            PGPFreeFileSpec(ringfilespec);

            if( status < 0 ) {
                fprintf(filebPtr->pgpout, LANG("\007Group view error. "));
                *perrorLvl = KEYRING_VIEW_ERROR;
            }
            goto out;
        } /* view key ring entries, with groupname match */

    default:
        argError(filebPtr,perrorLvl);
        status=-1;
        goto out;
    }
    status=0;

out:
	if(keySet != kPGPInvalidRef)
		PGPFreeKeySet(keySet);

    PGPFreeFileSpec( pubringSpec );
    PGPFreeData( pubringName );
    return status;
}

int doKeyOpt(struct pgpmainBones *mainbPtr, char keyChar, int *perrorLvl)
{
    PGPContextRef context = mainbPtr->pgpContext;
    char keyfilename[MAX_PATH];
    PGPFileSpecRef keyfilespec;
    char ringfilename[MAX_PATH];
    char origRingFileName[MAX_PATH];
    PGPFileSpecRef ringfilespec = kPGPInvalidRef;
    PGPFileSpecRef secringfilespec = kPGPInvalidRef;
    char *workFileName;
    int status = 0;
    struct pgpargsBones *argsbPtr = mainbPtr->argsbPtr;
    struct pgpargsPBones *argsPrivbPtr = argsbPtr->argsPrivbPtr;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env; PGPInt32  pri; PGPError err;

    PGPFileSpecRef pubringSpec;
    PGPFileSpecRef secringSpec;
    char *pubringName;
    char *secringName;

    origRingFileName[0] = '\0';

    if ((envbPtr->filterMode || pgpenvGetInt( env, PGPENV_BATCHMODE,
            &pri, &err )) && (keyChar == 'g' || keyChar == 'e' ||
            keyChar == 'd' || (keyChar == 'r' && argsbPtr->signFlag))) {
        *perrorLvl = NO_BATCH;
        argError(filebPtr,perrorLvl);
        /* interactive process, no go in batch mode */
        return -1;
    }
    /*
     * If we're not doing anything that uses stdout, produce output there,
     * in case user wants to redirect it.
     */
    if (!envbPtr->filterMode)
        filebPtr->pgpout = stdout;

    err = PGPsdkPrefGetFileSpec( context, kPGPsdkPref_PublicKeyring,
            &pubringSpec);
    pgpAssertNoErr(err);
    err = PGPsdkPrefGetFileSpec( context, kPGPsdkPref_PrivateKeyring,
            &secringSpec);

    pgpAssertNoErr(err);
    err = PGPGetFullPathFromFileSpec( pubringSpec, &pubringName );
    pgpAssertNoErr(err);
    err = PGPGetFullPathFromFileSpec( secringSpec, &secringName );
    pgpAssertNoErr(err);

    switch (keyChar) {

/*-------------------------------------------------------*/
    case 'g':
        { /* Key generation
             Arguments: bitcount, bitcount
           */
            char keybits[6], ebits[6];
            if (argsPrivbPtr->myArgc > 2)
                strncpy(keybits, argsPrivbPtr->myArgv[2], sizeof(keybits)
                        - 1);
            else
                keybits[0] = '\0';

            if (argsPrivbPtr->myArgc > 3)
                strncpy(ebits, argsPrivbPtr->myArgv[3], sizeof(ebits) - 1);
            else
                ebits[0] = '\0';

            /* If the -u option is given, use that username */

            /* dokeygen writes the keys out to the key rings... */
            status = dokeygen( mainbPtr, keybits, ebits );

            if (status < 0) {
                fprintf(filebPtr->pgpout, LANG("\007Keygen error. "));
                *perrorLvl = KEYGEN_ERROR;
            }
            goto out;
        } /* Key generation */

/*-------------------------------------------------------*/
    case 'c':
        { /* Key checking
             Arguments: userid, ringfilnamee
           */

            if (argsPrivbPtr->myArgc < 3) { /* Default to all user ID's */
                argsPrivbPtr->mcguffin[0] = '\0';
            } else {
                strcpy(argsPrivbPtr->mcguffin, argsPrivbPtr->myArgv[2]);
                if (strcmp(argsPrivbPtr->mcguffin, "*") == 0)
                    argsPrivbPtr->mcguffin[0] = '\0';
            }
            CONVERT_TO_CANONICAL_CHARSET(argsPrivbPtr->mcguffin);

            if (argsPrivbPtr->myArgc < 4) /* default key ring filename */
            {
                strcpy(ringfilename, pubringName);
            } else
                strncpy(ringfilename, argsPrivbPtr->myArgv[3],
                        sizeof(ringfilename) - 1);

            if ((argsPrivbPtr->myArgc < 4 && argsPrivbPtr->myArgc > 2)
                    /* Allow just key file as arg */
                    && (hasExtension(argsPrivbPtr->myArgv[2],
                            filebPtr->PKR_EXTENSION) ||
                            hasExtension(argsPrivbPtr->myArgv[2],
                                    filebPtr->SKR_EXTENSION))) {

                strcpy(ringfilename, argsPrivbPtr->myArgv[2]);
                argsPrivbPtr->mcguffin[0] = '\0';
            }

            err = PGPNewFileSpecFromFullPath( context, ringfilename,
                    &ringfilespec);
            pgpAssertNoErr(err);

            err = dokeycheck(mainbPtr, argsPrivbPtr->mcguffin,
                    ringfilespec);

            if (IsPGPError(err)) {
                fprintf(filebPtr->pgpout,
                        LANG("\007Keyring check error.\n"));
                *perrorLvl = KEYRING_CHECK_ERROR;
            }
            else {
                err = pgpShowTrustAndValidityList(mainbPtr,
                        argsPrivbPtr->mcguffin, ringfilespec);

                pgpAssertNoErr(err);

                if (argsPrivbPtr->mcguffin[0] != '\0')
                {
                    PGPFreeFileSpec(ringfilespec);
                    goto out;
                    /* checking a single user, dont do maintenance */
                }

            }

            PGPFreeFileSpec(ringfilespec);
            status= (status == -7 ? 0 : status);
            goto out;
        } /* Key check */

/*-------------------------------------------------------*/
    case 'm':
        { /* Maintenance pass
             Arguments: ringfilnamee
           */

            if (argsPrivbPtr->myArgc < 3) /* default key ring filename */
                strcpy(ringfilename, pubringName);
            else
                strcpy(ringfilename, argsPrivbPtr->myArgv[2]);

#ifdef MSDOS
            strlwr(ringfilename);
#endif
            if (!fileExists(ringfilename))
                defaultExtension( filebPtr, ringfilename,
                filebPtr->PKR_EXTENSION );

            err = PGPNewFileSpecFromFullPath( context, ringfilename,
                    &ringfilespec);
            pgpAssertNoErr(err);

            /* Assume: ringfilename always public*/

            err = doMaintenance(mainbPtr, ringfilespec, ringfilename);

            if (IsPGPError(err)) {
                fprintf(filebPtr->pgpout,
                        LANG("\007Maintenance pass error.\n"));
                *perrorLvl = KEYRING_CHECK_ERROR;
            }

            PGPFreeFileSpec(ringfilespec);
            goto out;
        } /* Maintenance pass */

/*-------------------------------------------------------*/
    case 's':
        /* Key signing
           Arguments: her_id, keyfilename
         */
        {
            char *myName;
            if (argsPrivbPtr->myArgc >= 4)
                strncpy(keyfilename, argsPrivbPtr->myArgv[3],
                        sizeof(keyfilename) - 1);

            else
                strcpy(keyfilename, pubringName);

            if (argsPrivbPtr->myArgc >= 3) {
                strcpy(argsPrivbPtr->mcguffin, argsPrivbPtr->myArgv[2]);
                /* Userid to sign */
            } else {
                fprintf(filebPtr->pgpout, LANG(
"\nA user ID is required to select the public key you want to sign. "));
                if (pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err ))
                {
                    status=-1;
                    goto out;
                }
                fprintf(filebPtr->pgpout,
                        LANG("\nEnter the public key's user ID: "));
                pgpTtyGetString(argsPrivbPtr->mcguffin, 255,
                        filebPtr->pgpout); /* echo keyboard */

            }

            CONVERT_TO_CANONICAL_CHARSET(argsPrivbPtr->mcguffin);

            myName = (char *)pgpenvGetCString( env, PGPENV_MYNAME, &pri );
            if( myName[0] == '\0' || pri < PGPENV_PRI_CONFIG) {
                fprintf(filebPtr->pgpout, LANG(
"\nA secret key is required to make a signature. "));
                fprintf(filebPtr->pgpout, LANG(
"\nYou specified no user ID to select your secret key,\n"
"so the default user ID and key will be the most recently\n"
"added key on your secret keyring.\n"));
            }

            err = PGPNewFileSpecFromFullPath( context, keyfilename,
                    &keyfilespec );

            pgpAssertNoErr(err);

            err = signKeyInFileSpec(mainbPtr, argsPrivbPtr->mcguffin,
                    keyfilespec);

            PGPFreeFileSpec( keyfilespec );

            if (err) {
                fprintf(filebPtr->pgpout, LANG(
                        "\007Key signature error. "));

                *perrorLvl = KEY_SIGNATURE_ERROR;
            }

            goto out;

        } /* Key signing */

/*-------------------------------------------------------*/
    case 'd':
        { /* disable/revoke key
             Arguments: userid, keyfilnamee
           */

            if (argsPrivbPtr->myArgc >= 4)
                strncpy(keyfilename, argsPrivbPtr->myArgv[3],
                        sizeof(keyfilename) - 1);

            else
                strcpy(keyfilename, pubringName);

            if (argsPrivbPtr->myArgc >= 3) {
                strcpy(argsPrivbPtr->mcguffin, argsPrivbPtr->myArgv[2]);
                /* Userid to sign */

            } else {
                if(argsbPtr->signFlag) {
                    fprintf(filebPtr->pgpout, LANG(
"\nA user ID is required to select the key signature you want to revoke. "));

                } else {
                    fprintf(filebPtr->pgpout, LANG(
"\nA user ID is required to select the key you want to revoke or disable. "));
                }
                fprintf(filebPtr->pgpout, LANG("\nEnter user ID: "));
                pgpTtyGetString(argsPrivbPtr->mcguffin, 255,
                        filebPtr->pgpout); /* echo keyboard */

            }
            CONVERT_TO_CANONICAL_CHARSET(argsPrivbPtr->mcguffin);

            if (argsbPtr->signFlag) { /* Revoke signatures */
                if (revokeSigs(mainbPtr,argsPrivbPtr->mcguffin,
                        keyfilename) < 0) {
                    PGPFreeFileSpec(ringfilespec);
                    fprintf(filebPtr->pgpout, LANG(
                            "\007Key signature revoke error. "));
                    *perrorLvl = KEYSIG_REVOKE_ERROR;
                }
            } else {
                if( pgpLocationIsURL( keyfilename )) {
                    status = disableKeyOnKeyServer(mainbPtr,
                            argsPrivbPtr->mcguffin, keyfilename);

                } else {
                    err = PGPNewFileSpecFromFullPath( context,
                            keyfilename, &keyfilespec);

                    pgpAssertNoErr(err);
                    status = revokeOrDisableKey(mainbPtr,
                            argsPrivbPtr->mcguffin, keyfilespec);

                    PGPFreeFileSpec(keyfilespec);
                }
                if (status < 0)
                    fprintf(filebPtr->pgpout,
                            LANG("\007Key disable or revoke error. "));
                if (status < 0)
                    *perrorLvl = KEY_SIGNATURE_ERROR;
            }
            goto out;
        }   /* Key compromise */

/*-------------------------------------------------------*/
    case 'e':
        {   /*      Key editing
                            Arguments: userid, ringfilnamee
                          */

            if (argsPrivbPtr->myArgc >= 4)
			{
                strncpy(ringfilename, argsPrivbPtr->myArgv[3],
                        sizeof(ringfilename) - 1);
				err = PGPNewFileSpecFromFullPath( context, ringfilename,
                    &ringfilespec);
				pgpAssertNoErr(err);
			}

            else  /* default key ring filename */
                ringfilespec = kPGPInvalidRef;

            if (argsPrivbPtr->myArgc >= 3) {
                strcpy(argsPrivbPtr->mcguffin, argsPrivbPtr->myArgv[2]);
                /* Userid to edit */

            } else {
                fprintf(filebPtr->pgpout, LANG(
"\nA user ID is required to select the key you want to edit. "));

                fprintf(filebPtr->pgpout,
                        LANG("\nEnter the key's user ID: "));

                pgpTtyGetString(argsPrivbPtr->mcguffin, 255,
                        filebPtr->pgpout);  /* echo keyboard */

            }
            CONVERT_TO_CANONICAL_CHARSET(argsPrivbPtr->mcguffin);

            status = doKeyEdit(mainbPtr, argsPrivbPtr->mcguffin,
                    ringfilespec);

            if (status >= 0) {
                status = maintUpdate(ringfilespec, 0);
                if (status == -7)
                    status = 0; /* ignore "not a public keyring" error */
                if (status < 0)
                    fprintf(filebPtr->pgpout,
                            LANG("\007Maintenance pass error. "));

            }
            if (status < 0) {
                fprintf(filebPtr->pgpout, LANG("\007Keyring edit error. "));
                *perrorLvl = KEYRING_EDIT_ERROR;
            }
            PGPFreeFileSpec(ringfilespec);
            goto out;
        }   /* Key edit */

/*-------------------------------------------------------*/
    case 'a':
        {   /*      Add key to key ring
                            Arguments: keyfilename, ringfilename
                          */

            if (argsPrivbPtr->myArgc < 3 && !envbPtr->filterMode) {
                argError(filebPtr,perrorLvl);
                status=-1;
                goto out;
            }

            if (!envbPtr->filterMode) { /* Get the keyfilename from args */
                strncpy(keyfilename, argsPrivbPtr->myArgv[2],
                        sizeof(keyfilename) - 1);

#ifdef MSDOS
                strlwr(keyfilename);
#endif
                if (!fileExists(keyfilename))
                    defaultExtension( filebPtr, keyfilename,
                            filebPtr->PGP_EXTENSION );

                if (!fileExists(keyfilename)) {
                    fprintf(filebPtr->pgpout,
                            LANG("\n\007Key file '%s' does not exist.\n"),
                            keyfilename);
                    *perrorLvl = NONEXIST_KEY_ERROR;
                    status=-1;
                    goto out;
                }
                workFileName = keyfilename;

            } else {
                if((workFileName = tempFile(filebPtr, TMP_WIPE |
                        TMP_TMPDIR, perrorLvl)) == NULL && *perrorLvl != 0)
                    return -1;
                readPhantomInput(filebPtr, workFileName );
            }

            if (argsPrivbPtr->myArgc < (envbPtr->filterMode ? 3 : 4))
            { /* default key ring filename */

                PGPByte ctb;
                getHeaderInfoFromFile(workFileName, &ctb, 1);
                if (ctb == CTB_CERT_SECKEY)
                    strcpy(ringfilename, secringName);
                else
                    strcpy(ringfilename, pubringName);
            } else {
                strncpy(ringfilename,
                        argsPrivbPtr->myArgv[(envbPtr->filterMode ?
                                2 : 3)], sizeof(ringfilename) - 1);

                strcpy(origRingFileName, ringfilename);

                tryRingFileExtensions ( filebPtr, ringfilename );
            }

#ifdef MSDOS
            strlwr(ringfilename);
#endif

            err = PGPNewFileSpecFromFullPath( context, ringfilename,
                    &ringfilespec);
            pgpAssertNoErr(err);

            status = addToKeyring(mainbPtr, workFileName, ringfilespec,
                    origRingFileName);

            if (envbPtr->filterMode)
                rmTemp( filebPtr, workFileName );

            if (status < 0) {
                fprintf(filebPtr->pgpout, LANG("\007Keyring add error. "));
                *perrorLvl = KEYRING_ADD_ERROR;
            }
            PGPFreeFileSpec(ringfilespec);
            goto out;
        }   /* Add key to key ring */

/*-------------------------------------------------------*/
    case 'x':
        {   /* Extract key from key ring
               Arguments: argsPrivbPtr->mcguffin,
               keyfilename, ringfilnamee
             */

            PGPBoolean  bTargetIsURL = FALSE;

            if (argsPrivbPtr->myArgc >= (envbPtr->filterMode ? 4 : 5))
                /* default key ring filename */
                strncpy(ringfilename,
                argsPrivbPtr->myArgv[(envbPtr->filterMode ? 3 : 4)],
                        sizeof(ringfilename) - 1);
            else
                strcpy(ringfilename, pubringName);

            if (argsPrivbPtr->myArgc >= (envbPtr->filterMode ? 2 : 3)) {
                if (argsPrivbPtr->myArgv[2])
                    /* Userid to extract */
                    strcpy(argsPrivbPtr->mcguffin, argsPrivbPtr->myArgv[2]);
                else
                    strcpy(argsPrivbPtr->mcguffin, "");
            } else {
                fprintf(filebPtr->pgpout,
LANG("\nA user ID is required to select the key you want to extract. "));
                if (pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err ))
                    /* not interactive, userid must be on command line */
                {
                    status=-1;
                    goto out;
                }
                fprintf(filebPtr->pgpout,
                        LANG("\nEnter the key's user ID: "));

                pgpTtyGetString(argsPrivbPtr->mcguffin, 255,
                        filebPtr->pgpout);  /* echo keyboard */

            }
            CONVERT_TO_CANONICAL_CHARSET(argsPrivbPtr->mcguffin);

            if (!envbPtr->filterMode) {
                if (argsPrivbPtr->myArgc >= 4)
                    strncpy(keyfilename, argsPrivbPtr->myArgv[3],
                            sizeof(keyfilename) - 1);

                else
                    keyfilename[0] = '\0';

                workFileName = keyfilename;
            } else {
                if((workFileName = tempFile(filebPtr, TMP_WIPE |
                        TMP_TMPDIR, perrorLvl))==NULL && *perrorLvl != 0)
                {
                    status=-1;
                    goto out;
                }
            }

#ifdef MSDOS
            strlwr(workFileName);
            strlwr(ringfilename);
#endif
            if( pgpLocationIsURL( ringfilename ) ) {
                status = extractFromKeyServer(mainbPtr,
                        argsPrivbPtr->mcguffin, workFileName,
                        ringfilename, (PGPBoolean)(envbPtr->filterMode ?
                                FALSE : pgpenvGetInt( env, PGPENV_ARMOR, &pri,
                                        &err )));

            } else {
                strcpy(origRingFileName, ringfilename);

                tryRingFileExtensions ( filebPtr, ringfilename );

                err = PGPNewFileSpecFromFullPath( context, ringfilename,
                        &ringfilespec);

                pgpAssertNoErr(err);
                status = extractFromKeyring(mainbPtr,
                        argsPrivbPtr->mcguffin, workFileName,
                        ringfilespec, origRingFileName,
                        (PGPBoolean)(envbPtr->filterMode ? FALSE :
                         pgpenvGetInt( env, PGPENV_ARMOR, &pri,
                                 &err )));

                PGPFreeFileSpec(ringfilespec);
            }

            if (status < 0) {
                if(status == kPGPError_ItemNotFound) {
                    status=0;
                    goto out;
                }
                fprintf(filebPtr->pgpout,
                        LANG("\007Keyring extract error. "));

                *perrorLvl = KEYRING_EXTRACT_ERROR;
                if (envbPtr->filterMode)
                    rmTemp(filebPtr, workFileName );
                goto out;
            }
            if (envbPtr->filterMode && !status) {
                    if (writePhantomOutput( filebPtr, workFileName ) < 0) {
                        *perrorLvl = UNKNOWN_FILE_ERROR;
                        status=-1;
                        goto out;
                    }
                rmTemp( filebPtr, workFileName );
            }

            bTargetIsURL = pgpLocationIsURL( workFileName);
            if ( pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err ) && !bTargetIsURL)
                fprintf(filebPtr->pgpout,
                        LANG("\nTransport armor file: %s\n"), workFileName);
            fprintf(filebPtr->pgpout,
                LANG("\nKey extracted to %s '%s'.\n"), bTargetIsURL ? "keyserver" : "file" ,workFileName);
            status=0;
            goto out;
        }   /* Extract key from key ring */

/*-------------------------------------------------------*/
    case 'r':
        { /*      Remove keys or selected key signatures from userid keys
Arguments: userid, ringfilename
           */
            if (argsPrivbPtr->myArgc >= 4)
                strcpy(ringfilename, argsPrivbPtr->myArgv[3]);
			else
				strcpy(ringfilename, pubringName);

            if (argsPrivbPtr->myArgc >= 3) {
                strcpy(argsPrivbPtr->mcguffin,
                        argsPrivbPtr->myArgv[2]); /* Userid to work on */
            } 
			else 
			{
				/* print usage message */
                if (argsbPtr->signFlag) 
				{
                    fprintf(filebPtr->pgpout,
						LANG("\nA user ID is required to select the public key you want to\n\
						remove certifying signatures from. "));
                }
				else
				{
                    fprintf(filebPtr->pgpout, LANG(
						"\nA user ID is required to select the key you want to remove. "));
                }
                if (pgpenvGetInt( env, PGPENV_BATCHMODE,
                        &pri, &err ))
                    /* not interactive, userid must be on command line */

                {
                    status=-1;
                    goto out;
                }
                fprintf(filebPtr->pgpout,
                        LANG("\nEnter the key's user ID: "));
                pgpTtyGetString(argsPrivbPtr->mcguffin,
                        255, filebPtr->pgpout);  /* echo keyboard */

            }
            CONVERT_TO_CANONICAL_CHARSET(argsPrivbPtr->mcguffin);

			/* check to see if keyring or keyserver */
            if( pgpLocationIsURL( ringfilename )) {

                if (argsbPtr->signFlag) 
				{
					/* Remove signatures */
                    fprintf(filebPtr->pgpout,
                    LANG("\nRemoving signatures from keyserver is NOT supported.\n"));
                    *perrorLvl = KEYSIG_REMOVE_ERROR;
                    status = -1;
                    goto out;

                } 
				else
				{ 
					/* Remove from keyserver */
                    if (removeFromKeyServer(mainbPtr, 
								argsPrivbPtr->mcguffin, ringfilename) < 0) 
					{
                        fprintf(filebPtr->pgpout,
                                LANG("\007Keyring remove error. "));
                        *perrorLvl = KEYRING_REMOVE_ERROR;
                        status=-1;
                        goto out;
                    }
                }
                status=0;
                goto out;
            }
			else	/* operate on keyring */
			{
#ifdef MSDOS
                strlwr(ringfilename);
#endif
                strcpy(origRingFileName, ringfilename);
                tryRingFileExtensions ( filebPtr, ringfilename );

                if (argsbPtr->signFlag)
				{ /* Remove signatures */
                    if ( removeSigs( mainbPtr,
                            argsPrivbPtr->mcguffin, ringfilename,
                            origRingFileName) < 0) 
					{

                        PGPFreeFileSpec(ringfilespec);
                        fprintf(filebPtr->pgpout,
                                LANG("\007Key signature remove error. "));
                        *perrorLvl = KEYSIG_REMOVE_ERROR;
                        status=-1;
                        goto out;
                    }
                } 
				else
				{  /* Remove keyring */
                    if ( removeFromKeyring( mainbPtr,
                            argsPrivbPtr->mcguffin, 
							argsPrivbPtr->myArgc >= 4 ? ringfilename : NULL,
							origRingFileName))
					{
                        fprintf(filebPtr->pgpout,
                                LANG("\007\nKeyring remove error. "));
                        *perrorLvl = KEYRING_REMOVE_ERROR;
                        status=-1;
                        goto out;
                    }
                }
                status=0;
                goto out;

            }

        }   /* remove key signatures from userid */

/*-------------------------------------------------------*/
    case 'v':
    case 'V':   /* -kvv */
        {   /* View or remove key ring entries,
                            with userid match
                            Arguments: userid, ringfilename
                          */

            if (argsPrivbPtr->myArgc < 4) /* default key ring filename */
                strcpy(ringfilename, pubringName);
            else
                strcpy(ringfilename, argsPrivbPtr->myArgv[3]);

            if (argsPrivbPtr->myArgc > 2) {
                strcpy(argsPrivbPtr->mcguffin, argsPrivbPtr->myArgv[2]);
                if (strcmp(argsPrivbPtr->mcguffin, "*") == 0)
                    argsPrivbPtr->mcguffin[0] = '\0';
            } else {
                *argsPrivbPtr->mcguffin = '\0';
            }

            if ((argsPrivbPtr->myArgc == 3) &&
                    ( hasExtension(argsPrivbPtr->myArgv[2],
                                    filebPtr->PKR_EXTENSION) ||
                      hasExtension(argsPrivbPtr->myArgv[2],
                              filebPtr->SKR_EXTENSION)) ) {
                strcpy(ringfilename, argsPrivbPtr->myArgv[2]);
                argsPrivbPtr->mcguffin[0] = '\0';
            }
            CONVERT_TO_CANONICAL_CHARSET(argsPrivbPtr->mcguffin);

            if( pgpLocationIsURL( ringfilename )) {
                status = viewKeyServer( mainbPtr, argsPrivbPtr->mcguffin,
                        ringfilename,
                        (keyChar == 'V' ? kShow_Sigs | kShow_Subkeys : 0)
                        | (argsPrivbPtr->cFlag ? kShow_Hashes : 0)
                        /*| (verbose ? kShow_Subkeys : 0)*/
                                      );
            } else {
#ifdef MSDOS
                strlwr(ringfilename);
#endif

                strcpy(origRingFileName, ringfilename);

                tryRingFileExtensions (filebPtr, ringfilename);

                err = PGPNewFileSpecFromFullPath( context, ringfilename,
                        &ringfilespec);

                pgpAssertNoErr(err);

                /* If a second 'v' (keyChar = V), show signatures too */
                status = viewKeyring(mainbPtr, argsPrivbPtr->mcguffin,
						argsPrivbPtr->myArgc == 4 ? ringfilespec : NULL,
						origRingFileName,
                        (keyChar == 'V' ? kShow_Sigs | kShow_Subkeys : 0)
                        | (argsPrivbPtr->cFlag ? kShow_Hashes : 0)
                        /*| (verbose ? kShow_Subkeys : 0)*/
                                    );

                PGPFreeFileSpec(ringfilespec);
            }

            if( status < 0 ) {
                fprintf(filebPtr->pgpout, LANG("\007Keyring view error. "));
                *perrorLvl = KEYRING_VIEW_ERROR;
            }
            goto out;
        }   /* view key ring entries, with userid match */

    default:
        argError(filebPtr,perrorLvl);
        status=-1;
        goto out;
    }
    status=0;

out:
    PGPFreeFileSpec( pubringSpec );
    PGPFreeFileSpec( secringSpec );
    PGPFreeData( pubringName );
    PGPFreeData( secringName );
    return status;
}

/* We had to process the config file first to possibly select the
   foreign language to translate the sign-on line that follows... */

void signonMsg(struct pgpmainBones *mainbPtr)
{
    struct pgpenvBones  *envbPtr = mainbPtr->envbPtr;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPTime tstamp;
    PGPEnv *env = envbPtr->m_env; PGPInt32  pri; PGPError err;
    /* display message only once to allow calling multiple times */
    static PGPBoolean printed = FALSE;
    PGPBoolean compatible = envbPtr->compatible;

    if ( pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err ) || printed )
        return;
    printed = TRUE;

    fprintf(stderr,
LANG("Pretty Good Privacy(tm) Version %s"),
            mainbPtr->relVersion);

    if (compatible) fprintf(stderr,
LANG("- Public-key encryption for the masses."));

    fprintf(stderr, "\n");

#if PGP_DEBUG
    fputs( LANG(
"Internal development version only - not for general release.\n"),
        stderr);
#endif
    fprintf(stderr, LANG("(c) 1999 Network Associates Inc.\n"));
	fputs( LANG(mainbPtr->signonLegalese), stderr);
    fprintf(stderr, "\n");

    fputs(
LANG("Export of this software may be restricted by the U.S. government.\n"),
          stderr);

    if (compatible) {
        tstamp = PGPGetTime();
        fprintf(filebPtr->pgpout, LANG("Current time: %s\n\n"),
           ctdate(&tstamp));
    }
	else
	{
		fputs("\n", stderr);
	}

}

/* -f means act as a unix-style filter */
/* -i means internalize extended file attribute information, only supported
 *          between like (or very compatible) operating systems. */
/* -l means show longer more descriptive diagnostic messages */
/* -m means display plaintext output on screen, like unix "more" */
/* -d means decrypt only, leaving inner signature wrapping intact */
/* -t means treat as pure text and convert to canonical text format */

/* Used by getopt function... */
#define OPTIONS "abcdefghiklmn:o:prstu:vwxz:ABCDEFGHIKLMN:O:PRSTU:VWX?"

/* unused option flags: j, q, y*/

int pgpParseArgs(struct pgpmainBones *mainbPtr, int argc, char *argv[],
        int *perrorLvl)
{
    PGPContextRef context = mainbPtr->pgpContext;
    int opt;
    char *p;
    struct pgpenvBones   *envbPtr = mainbPtr->envbPtr;
    struct pgpargsBones  *argsbPtr = mainbPtr->argsbPtr;
    struct pgpargsPBones *argsPrivbPtr = argsbPtr->argsPrivbPtr;
    struct pgpfileBones  *filebPtr = mainbPtr->filebPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPError err;
    extern int optind;
    extern char *optarg;

    /* Initial messages to stderr */
    filebPtr->pgpout = stderr;

#ifdef DEBUG1
    pgpenvSetInt( env, PGPENV_VERBOSE,  TRUE, PGPENV_PRI_FORCE );
    /*filebPtr->verbose = TRUE;*/
#endif		  

	/* set FASTKEYGEN to TRUE */
	pgpenvSetInt( env, PGPENV_FASTKEYGEN, TRUE, PGPENV_PRI_PRIVDEFAULT);

    /* The various places one can get passwords from.
     * We accumulate them all into two lists.  One is
     * to try on keys only, and is stored in no particular
     * order, while the other is of unknown purpose so
     * far (they may be used for conventional encryption
     * or decryption as well), and are kept in a specific
     * order.  If any password in the general list is found
     * to decode a key, it is moved to the key list.
     * The general list is not grown after initialization,
     * so the tail pointer is not used after this.
     */

    if ((p = getenv("PGPPASS")) != NULL) {
        if((err = pgpAppendToPassphraseList( envbPtr->passwds, p )) != 0) {
            *perrorLvl = OUT_OF_MEM;
            return -1;
        }
    }

    /* The -z "password" option should be used instead of PGPPASS if
     * the environment can be displayed with the ps command (eg. BSD).
     * If the system does not allow overwriting of the command line
     * argument list but if it has a "hidden" environment, PGPPASS
     * should be used.
     */
    for (opt = 1; opt < argc; ++opt) {
        p = argv[opt];
        if (p[0] != '-' || p[1] != 'z')
            continue;
        /* Accept either "-zpassword" or "-z password" */
        p += 2;
        if (!*p)
            p = argv[++opt];
        /* p now points to password */
        if (!p)
            break;  /* End of arg list - ignore */

        if((err = pgpAppendToPassphraseList( envbPtr->passwds, p )) != 0) {
            *perrorLvl = OUT_OF_MEM;
            return -1;
        }
        /* Wipe password */
        while (*p)
            *p++ = ' ';
    }
    /*
     * If PGPPASSFD is set in the environment try to read the password
     * from this file descriptor.  If you set PGPPASSFD to 0 pgp will
     * use the first line read from stdin as password.
     */
    if ((p = getenv("PGPPASSFD")) != NULL) {
        int passfd;
        if (*p && (passfd = atoi(p)) >= 0) {
            char pwbuf[256];
            p = pwbuf;
            while (read(passfd, p, 1) == 1 && *p != '\n')
                ++p;
            *p='\0';

            if((err = pgpAppendToPassphraseList( envbPtr->passwds,
                    pwbuf )) != 0) {
                *perrorLvl = OUT_OF_MEM;
                return -1;
            }
            memset(pwbuf, 0, p - pwbuf);
        }
    }
    /* Process the config file.  The following override each other:
       - Hard-coded defaults
       - The system config file
       - Hard-coded defaults for security-critical things
       - The user's config file
       - Environment variables
       - Command-line options.
     */
    opt = 0;   /* Number of config files read */
#ifdef PGP_SYSTEM_DIR
    strcpy(argsPrivbPtr->mcguffin, PGP_SYSTEM_DIR);
    strcat(argsPrivbPtr->mcguffin, "config.txt");
    if (access(argsPrivbPtr->mcguffin, 0) == 0) {
        opt++;
        /*
         * Note: errors here are NOT fatal, so that people
         * can use PGP with a corrputed system file.
         */
        processConfigFile( envbPtr, argsPrivbPtr->mcguffin,
                PGPENV_PRI_SYSCONF);

    }
#endif


    /*
       This new Preferences interface seems problematic for us.  It is
       somewhat inflexible, being a binary file that is difficult
       for users to edit; and it is incompatible with the system of
       overrides established by the Config file processing.

       A command-line user still ought to be able to set preferences
       by editing her config file and using the names
           PUBRING, SECRING, RANDSEED, MYNAME, and GROUPSFILE

       In fact, if the user specifies on the command line
           +pubring=pathtothepubring
       then PGP is likely to set the pubring preference to that and
       leave it that way, unless it's also set in the config file.
       I guess that's acceptible.
     */

    {
        PGPByte *keyIDData = NULL;
        PGPSize  keyIDSize = 0;
        PGPKeyID keyID;
        char myname[ kPGPMaxKeyIDStringSize ];

        /* set myname based on default signing key in prefs*/
        err = PGPsdkPrefGetData( context, kPGPsdkPref_DefaultKeyID,
                (void **)&keyIDData, &keyIDSize );

        switch(err)
        {
            case kPGPError_PrefNotFound:
                myname[0]='\0';
                break;
            case kPGPError_NoErr:
                err = PGPImportKeyID( keyIDData, &keyID );
                pgpAssertNoErr(err);
                err = PGPGetKeyIDString( &keyID, kPGPKeyIDString_Full,
                        myname );
                pgpAssertNoErr(err);
                PGPFreeData( keyIDData );
                break;
            default:
                break;
        }
        if ( IsntPGPError(err) )
            pgpenvSetString( env, PGPENV_MYNAME, myname,
                    PGPENV_PRI_PRIVDEFAULT );

    }


    /* Process the config file first.  Any command-line arguments will
       override the config file settings */
#if defined(PGP_UNIX) || defined(MSDOS) || defined(WIN32) || defined(OS2)
    /* Try "pgp.ini" on MS-DOS or ".pgprc" on Unix */
 #ifdef PGP_UNIX
    buildFileName(argsPrivbPtr->mcguffin, "pgp.cfg");
    /* look for a 5.0i config file */

    if(access(argsPrivbPtr->mcguffin, 0) != 0)
       buildFileName(argsPrivbPtr->mcguffin, ".pgprc");
 #elif PGP_WIN32
    buildFileName(argsPrivbPtr->mcguffin, "pgp.cfg");
 #else
    buildFileName(argsPrivbPtr->mcguffin, "pgp.ini");
 #endif
    if (access(argsPrivbPtr->mcguffin, 0) != 0)
        buildFileName(argsPrivbPtr->mcguffin, "config.txt");
#else
    buildFileName(argsPrivbPtr->mcguffin, "config.txt");
#endif
    if (access(argsPrivbPtr->mcguffin, 0) == 0) {
        opt++;
        if (processConfigFile( envbPtr, argsPrivbPtr->mcguffin,
                PGPENV_PRI_CONFIG) < 0) {

            *perrorLvl=BAD_ARG_ERROR;
            /*exit(BAD_ARG_ERROR);*/
            return -1;
        }
    }
    if (!opt)
        fprintf(filebPtr->pgpout,
                LANG("\007No configuration file found.\n"));

    init_charset();

#ifdef MSDOS   /* only on MSDOS systems */
    if ((p = getenv("TZ")) == NULL || *p == '\0') {
		if (_timezone == 0) {
			fprintf(filebPtr->pgpout,LANG(
	"\007WARNING: Environmental variable TZ is not \
	defined, so GMT timestamps\n\
	may be wrong.  See the PGP User's Guide to properly define TZ\n"));
	    }
	}
#endif    /* MSDOS */

#ifdef VMS
#define TEMP "SYS$SCRATCH"
#else
#define TEMP "TMP"
#endif    /* VMS */
    if ((p = getenv(TEMP)) != NULL && *p != '\0')
        setTempDir( filebPtr, p );

    if ((argsPrivbPtr->myArgv = (char **) malloc(
            (argc + 2) * sizeof(char **))) == NULL) {

        fprintf(stderr, LANG("\n\007Out of memory.\n"));
        *perrorLvl = 7;
        return -1;
    }
    argsPrivbPtr->myArgv[0] = NULL;
    argsPrivbPtr->myArgv[1] = NULL;

    /* Process all the command-line option switches: */
    while (optind < argc) {
        /*
         * Allow random order of options and arguments (like GNU getopt)
         * NOTE: this does not work with GNU getopt, use getopt.c from
         * the PGP distribution.
         */
        if ((opt = pgpGetOpt(argc, argv, OPTIONS)) == EOF) {
            if (optind == argc) /* -- at end */
                break;
            argsPrivbPtr->myArgv[argsPrivbPtr->myArgc++] = argv[optind++];
            continue;
        }
        opt = toLower(opt);
        if (argsbPtr->keyFlag && (argsbPtr->keyChar == '\0' ||
                (argsbPtr->keyChar == 'v' && opt == 'v'))) {

            if (argsbPtr->keyChar == 'v')
                argsbPtr->keyChar = 'V';
            else
                argsbPtr->keyChar = opt;
            continue;
        }
        if (argsbPtr->groupFlag && (argsbPtr->groupChar == '\0' ||
                (argsbPtr->groupChar == 'v' && opt == 'v'))) {

            if (argsbPtr->groupChar == 'v')
                argsbPtr->groupChar = 'V';
            else
                argsbPtr->groupChar = opt;
            continue;
        }

        switch (opt) {
        case 'a':
            argsbPtr->armorFlag = TRUE;
            pgpenvSetInt( env, PGPENV_ARMOR,  1, PGPENV_PRI_CMDLINE );
            break;
        case 'b':
            mainbPtr->separateSignature = argsbPtr->stripSigFlag = TRUE;
            break;
        case 'c':
            argsbPtr->encryptFlag = argsbPtr->conventionalFlag = TRUE;
            argsPrivbPtr->cFlag = TRUE;
            break;
        case 'd':
            argsbPtr->decryptOnlyFlag = TRUE;
            break;
        case 'e':
            argsbPtr->encryptFlag = TRUE;
            break;
        case 'f':
            envbPtr->filterMode = TRUE;
            break;
        case 'g':
            argsbPtr->groupFlag = TRUE;
            break;
        case '?':
        case 'h':
            usage(filebPtr,perrorLvl);
            return -1;
            break;
#ifdef VMS
        case 'i':
            /*pgpenvSetInt( env, PGPENV_TEXTMODE,  MODE_LOCAL,
              PGPENV_PRI_CMDLINE );*/
            /* hmm... can't do*/
            break;
#endif    /* VMS */
        case 'k':
            argsbPtr->keyFlag = TRUE;
            break;
        case 'l':
            pgpenvSetInt( env, PGPENV_VERBOSE,  TRUE, PGPENV_PRI_CMDLINE );
            break;
        case 'm':
            envbPtr->moreFlag = TRUE;
            break;
        case 'n':
                /* XXX placeholder to specify new something */
            break;
        case 'p':
            argsbPtr->preserveFileName = TRUE;
            break;
        case 'o':
            argsbPtr->outputFileName = optarg;
            break;
        case 's':
            argsbPtr->signFlag = TRUE;
            break;
        case 't':
            pgpenvSetInt( env, PGPENV_TEXTMODE,  TRUE, PGPENV_PRI_CMDLINE );
            break;
        case 'u':
            {
                char *myName = (char *)malloc( strlen(optarg)+1);
                if(myName == NULL) {
                    fprintf(stderr, LANG("\n\007Out of memory.\n"));
                    *perrorLvl=OUT_OF_MEM;
                    return -1;
                }
                strcpy( myName, optarg );
                CONVERT_TO_CANONICAL_CHARSET(myName);
                pgpenvSetString( env, PGPENV_MYNAME, myName,
                        PGPENV_PRI_CMDLINE );

                argsPrivbPtr->uFlag = TRUE;
                free(myName);
            }
            break;
        case 'w':
            argsbPtr->wipeFlag = TRUE;
            break;
        case 'z':
            /* "-zpassword" or "-z password" */
            break;
        case '+':
            /* '+' special option: does not require - */
            if (processConfigLine( envbPtr, optarg, PGPENV_PRI_CMDLINE) == 0)
                break;
            fprintf(stderr, "\n");
            /* fallthrough */
        default:
            argError(filebPtr,perrorLvl);
            return -1;
        }
    }
    argsPrivbPtr->myArgv[argsPrivbPtr->myArgc] = NULL;
    /* Just to make it NULL terminated */

    return 0;
}

void pgpPrintOutfileName( struct pgpmainBones *mainbPtr) {
  /* Called only after DoEncrypt now. Could be modified to include
     filename options created by DoDecrypt. */

    struct pgpargsBones *argsbPtr = mainbPtr->argsbPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;
    PGPBoolean encrypted = argsbPtr->encryptFlag;
    PGPBoolean sig = argsbPtr->signFlag;
    PGPBoolean quietmode = pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err);
    PGPBoolean textmode = pgpenvGetInt( env, PGPENV_TEXTMODE, &pri, &err);
    PGPBoolean armorout = pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err);
    PGPBoolean clearsig = pgpenvGetInt( env, PGPENV_CLEARSIG, &pri, &err)
                && textmode && armorout && !argsbPtr->stripSigFlag  &&
                !encrypted;

         /* clear-sign only makes sense if file is text and armor is on */

    if (quietmode) return;

           if (armorout) {

               if (clearsig)
                fprintf(filebPtr->pgpout,
                        LANG("\nClear signature file: %s\n"),
                        argsbPtr->outputFileName);

               else
                fprintf(filebPtr->pgpout,
                        LANG("\nTransport armor file: %s\n"),
                        argsbPtr->outputFileName);

               /*check also if >1 transport armor fileS */
           }
           else {

                if (encrypted)
                    fprintf(filebPtr->pgpout,
                    LANG("\nCiphertext file: %s\n"),
                    argsbPtr->outputFileName);

                else if (sig)
                    fprintf(filebPtr->pgpout,
                    LANG("\nSignature file: %s\n"),
                    argsbPtr->outputFileName);

           }
}


int pgpProcessArgs( struct pgpmainBones *mainbPtr, int *perrorLvl)
{
    struct pgpargsBones *argsbPtr = mainbPtr->argsbPtr;
    struct pgpargsPBones *argsPrivbPtr = argsbPtr->argsPrivbPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;
    int status;
    PGPBoolean quietmode = pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err);

    /* -db means break off signature certificate into separate file */
    if (argsbPtr->decryptOnlyFlag && argsbPtr->stripSigFlag)
        argsbPtr->decryptOnlyFlag = FALSE;

    if (argsbPtr->decryptOnlyFlag && argsbPtr->armorFlag)
        mainbPtr->decryptMode = mainbPtr->deArmorOnly = TRUE;

    if (argsbPtr->outputFileName != NULL)
        argsbPtr->preserveFileName = FALSE;

    if (!argsbPtr->signFlag && !argsbPtr->encryptFlag &&
            !argsbPtr->conventionalFlag && !argsbPtr->armorFlag) {

        if (argsbPtr->wipeFlag) { /* wipe only */
            if (argsPrivbPtr->myArgc != 3) {
                argError(filebPtr,perrorLvl); /* need one argument */
                return -1;
            }
            if (wipefile(filebPtr, argsPrivbPtr->myArgv[2]) == 0 &&
                    remove(argsPrivbPtr->myArgv[2]) == 0) {

                if (!quietmode) {
                    fprintf(filebPtr->pgpout,
                        LANG("\nFile %s wiped and deleted. "),
                        argsPrivbPtr->myArgv[2]);

                    fprintf(filebPtr->pgpout, "\n");
                }
                *perrorLvl=EXIT_OK;
                return 0;
            }
            *perrorLvl=UNKNOWN_FILE_ERROR;
            return -1;
        }
        /* decrypt if none of the -s -e -c -a -w options are specified */
        mainbPtr->decryptMode = TRUE;
    }
    if (argsPrivbPtr->myArgc == 2) { /* no arguments */
#ifdef PGP_UNIX
        if (!envbPtr->filterMode && !isatty(fileno(stdin))) {
            /* piping to pgp without arguments and no -f:
             * switch to filter mode but don't write output to stdout
             * if it's a tty, use the preserved filename */
            if (!envbPtr->moreFlag)
                filebPtr->pgpout = stderr;
            envbPtr->filterMode = TRUE;
            if (isatty(fileno(stdout)) && !envbPtr->moreFlag)
                argsbPtr->preserveFileName = TRUE;
        }
#endif
        if (!envbPtr->filterMode) {
            char *p;
            if (pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err )) {
                pgpenvSetInt( env, PGPENV_NOOUT,  FALSE, PGPENV_PRI_FORCE );
            }               if (strcmp((p = LANG("@translator@")),
                                "@translator@"))

                            fprintf(filebPtr->pgpout, p);
                    fprintf(filebPtr->pgpout,
                            LANG("\nFor a usage summary, type:  pgp -h\n"));
                    *perrorLvl=BAD_ARG_ERROR;
                    return -1;
        }
    } else {
        if (envbPtr->filterMode) {
            mainbPtr->recipients = &argsPrivbPtr->myArgv[2];
        } else {
            argsPrivbPtr->inputFileName = argsPrivbPtr->myArgv[2];
            mainbPtr->recipients = &argsPrivbPtr->myArgv[3];
        }
    }

    if (envbPtr->filterMode) {
        argsPrivbPtr->inputFileName = "stdin";
    } else if (envbPtr->makeRandom > 0) { /* Create the input file */
        /*
         * +makerandom=<bytes>: Create an input file consisting of <bytes>
         * cryptographically strong random bytes, before applying the
         * encryption options of PGP.  This is an advanced option, so
         * assume the user knows what he's doing and don't bother about
         * overwriting questions.  E.g.
         * pgp +makerandom=24 foofile
         * Create "foofile" with 24 random bytes in it.
         * pgp +makerandom=24 -ea foofile recipient
         * The same, but also encrypt it to "recipient", creating
         * foofile.asc as well.
         * This feature was created to allow PGP to create and send keys
         * around for other applications to use.
         */
        status = cryptRandWriteFile(argsPrivbPtr->inputFileName,
                (struct IdeaCfbContext *)0, (unsigned)envbPtr->makeRandom);

        if (status < 0) {
            fprintf( stderr, LANG("Error writing file \"%s\"\n"),
                    argsPrivbPtr->inputFileName );

            *perrorLvl = INVALID_FILE_ERROR;
            return -1;
        }
        if (!quietmode) {
            fprintf(filebPtr->pgpout,
                LANG("File %s created containing %d random bytes.\n"),
                argsPrivbPtr->inputFileName, envbPtr->makeRandom);
        }

        /* If we aren't encrypting, don't bother trying to decrypt this! */
        if (mainbPtr->decryptMode) {
            *perrorLvl = EXIT_OK;
            return 0;
        }

        /* This is obviously NOT a text file */
        pgpenvSetInt( env, PGPENV_TEXTMODE,  FALSE, PGPENV_PRI_FORCE );
    } else {
        if (mainbPtr->decryptMode && noExtension( filebPtr,
                argsPrivbPtr->inputFileName )) {
            strcpy(argsPrivbPtr->cipherFile, argsPrivbPtr->inputFileName);
            forceExtension( filebPtr, argsPrivbPtr->cipherFile,
                    filebPtr->ASC_EXTENSION );

            if (fileExists(argsPrivbPtr->cipherFile)) {
                argsPrivbPtr->inputFileName = argsPrivbPtr->cipherFile;
            } else {
                forceExtension( filebPtr, argsPrivbPtr->cipherFile,
                        filebPtr->PGP_EXTENSION );
                if (fileExists(argsPrivbPtr->cipherFile)) {
                    argsPrivbPtr->inputFileName = argsPrivbPtr->cipherFile;
                } else {
                    forceExtension( filebPtr, argsPrivbPtr->cipherFile,
                            filebPtr->SIG_EXTENSION );
                    if (fileExists(argsPrivbPtr->cipherFile))
                        argsPrivbPtr->inputFileName =
                            argsPrivbPtr->cipherFile;

                }
            }
        }
        if (!fileExists(argsPrivbPtr->inputFileName)) {
            fprintf(filebPtr->pgpout,
                    LANG("\007File [%s] does not exist.\n"),
                    argsPrivbPtr->inputFileName);

            *perrorLvl = FILE_NOT_FOUND_ERROR;
            userError(filebPtr,perrorLvl);
            return -1;
        }
    }

    if (strlen(argsPrivbPtr->inputFileName) >= (unsigned) MAX_PATH - 4) {
        fprintf(filebPtr->pgpout,
                LANG("\007Invalid filename: [%s] too long\n"),
                argsPrivbPtr->inputFileName);

        *perrorLvl = INVALID_FILE_ERROR;
        userError(filebPtr,perrorLvl);
        return -1;
    }
    strcpy(mainbPtr->plainfilename, argsPrivbPtr->inputFileName);

    if (envbPtr->filterMode) {
        setOutDir( filebPtr, NULL ); /* NULL means use tmpdir */
    } else {
        if (argsbPtr->outputFileName)
            setOutDir( filebPtr, argsbPtr->outputFileName );
        else
            setOutDir( filebPtr, argsPrivbPtr->inputFileName );
    }

    if (envbPtr->filterMode) {
        if((argsPrivbPtr->workFileName = tempFile( filebPtr, TMP_WIPE |
                TMP_TMPDIR, perrorLvl))==NULL && *perrorLvl != 0)

            return -1;
        readPhantomInput(filebPtr, argsPrivbPtr->workFileName );
    } else {
        argsPrivbPtr->workFileName = argsPrivbPtr->inputFileName;
    }

    /* -- start scanning the input file -------------------------------- */


    getHeaderInfoFromFile(argsPrivbPtr->workFileName, &argsPrivbPtr->ctb, 1);

    if (mainbPtr->decryptMode) {
        PGPUInt32 count;

        /* -- pgpDoDecode is responsible for everything:
           loop while part of the file remains unprocessed,
           0. scan for each lexical section
           1. de armor if needed
           2. decrypt if needed
           ask for pass phrase if needed and keep it for the next ciphertext
           3. check signature
           4. if text mode, convert from canonical text to local format.

           complexities...
           1. may have a sequence of armor texts.
           2. may want to de-armor but not decrypt
           3. may be armored but not encrypted.
           4. may be signed but not encrypted.
           5. may be armored and signed but not encrypted.
           6. may want to output the clear-signed message after de-armor.
           7. may want to output the clear-signed message after decryption.
           8. may want to output the signature detached from the cleartext
           after decryption.
           Q. what to do in filter mode if multiple armor texts appear?
           it seems that the new PGP versions place it inline into the text.
         */

        err = pgpDoDecode( mainbPtr, argsPrivbPtr->workFileName, perrorLvl );
        if( IsPGPError(err ) )
        {
           /* XXX clean up tempfiles??*/
           *perrorLvl = DECR_ERROR;
           return -1;
        }

        /* delete the workfile if it is not the inputfile */
        if( argsPrivbPtr->inputFileName && strcmp(
                argsPrivbPtr->workFileName, argsPrivbPtr->inputFileName ) )

            rmTemp(filebPtr, argsPrivbPtr->workFileName );

        /* emit the output. special cases to consider: in the event
           of multiple files processed, send to stdout if outputStdout,
           or name them otherwise. */

        err = pgpCountFileNameList( mainbPtr->decodefilenames, &count);
        if( count > 0 ) {
            char *tempf;
            if( mainbPtr->outputStdout ) {
                err = pgpRewindFileName( mainbPtr->decodefilenames );
                pgpAssertNoErr(err);
                err = pgpNextFileName( mainbPtr->decodefilenames, &tempf );
                while( tempf )
                {
                    if (writePhantomOutput( filebPtr, tempf) < 0) {
                        *perrorLvl = UNKNOWN_FILE_ERROR;
                        userError(filebPtr,perrorLvl);
                        return -1;
                    }
                    rmTemp( filebPtr, tempf);
                    pgpNextFileName( mainbPtr->decodefilenames, &tempf );
                }
            } else {
                char name[MAX_PATH];
                if (argsbPtr->outputFileName) {
                    strcpy(name, argsbPtr->outputFileName);
                } else {
                    strcpy(name, argsPrivbPtr->inputFileName);
                    dropExtension( filebPtr, name );
                }

                /* check to see if I need to free memory allocated when
                   '-p' specified on the decode */
                if(mainbPtr->argsbPtr->preserveFileName &&
                    mainbPtr->argsbPtr->outputFileName)
                {
                    free(mainbPtr->argsbPtr->outputFileName);
                    mainbPtr->argsbPtr->outputFileName = NULL;
                }

                err = pgpRewindFileName( mainbPtr->decodefilenames );
                pgpAssertNoErr(err);

                err = pgpNextFileName( mainbPtr->decodefilenames, &tempf );
                while( tempf )
                {
                    if((argsbPtr->outputFileName = saveTemp( filebPtr,
                            tempf, name )) == NULL) {

                        *perrorLvl = UNKNOWN_FILE_ERROR;
                        userError(filebPtr, perrorLvl);
                        return -1;
                    }

                    if (!quietmode
                        && !pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err ))
                        /* if other filename messages were suppressed */
                        fprintf(filebPtr->pgpout,
                            LANG("\nPlaintext filename: %s\n"),
                            argsbPtr->outputFileName);

                    pgpNextFileName( mainbPtr->decodefilenames, &tempf );

                }
            }
        }
        return 0;
    }

    /* -- encapsulation ----------------------------------------------- */
    /* -- signing ----------------------------------------------------- */
    /* -- encryption -------------------------------------------------- */
    /* -- armoring ---------------------------------------------------- */

    status = pgpDoEncode(mainbPtr, argsPrivbPtr->workFileName,
            &argsPrivbPtr->tempf, perrorLvl );

    /* pgpAssertNoErr(err);*/

    /* delete the workfile if it is not the inputfile */
    if( argsPrivbPtr->inputFileName && strcmp( argsPrivbPtr->workFileName,
            argsPrivbPtr->inputFileName ) )

        rmTemp( filebPtr, argsPrivbPtr->workFileName );

    argsPrivbPtr->workFileName = argsPrivbPtr->tempf;
    if( status != 0 )
        return status;


    /* -- emit output ------------------------------------------------- */
    /* emit the output. it is significant that
       in filter mode we should output to stdout. */

    if( mainbPtr->outputStdout) {

        if (writePhantomOutput( filebPtr, argsPrivbPtr->workFileName ) < 0) {
            *perrorLvl = UNKNOWN_FILE_ERROR;
            userError(filebPtr,perrorLvl);
            return -1;
        }
        rmTemp( filebPtr, argsPrivbPtr->workFileName );


    } else {

        char name[MAX_PATH];
        if (argsbPtr->outputFileName) {
            strcpy(name, argsbPtr->outputFileName);
        } else {
            strcpy(name, argsPrivbPtr->inputFileName);
			dropExtension( filebPtr, name );
        }

		if(envbPtr->compatible)
		{
			/*
				drop off extension of file, so that ls.txt becomes ls.pgp
				instead of ls.txt.pgp.
			*/
			char	*psz = NULL;
			psz = strrchr(name, '.');
			if(psz)
				*psz = '\0';
		}

        if (noExtension( filebPtr, name )) {
            if ( pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err ))
                forceExtension( filebPtr, name, filebPtr->ASC_EXTENSION );
            else if (argsbPtr->signFlag && mainbPtr->separateSignature)
                forceExtension( filebPtr, name, filebPtr->SIG_EXTENSION );
            else
                forceExtension( filebPtr, name, filebPtr->PGP_EXTENSION );
        }
        if((argsbPtr->outputFileName = saveTemp( filebPtr,
                argsPrivbPtr->workFileName, name ))==NULL) {

            *perrorLvl = UNKNOWN_FILE_ERROR;
            userError(filebPtr, perrorLvl);
            return -1;
        }
        else {
            pgpPrintOutfileName(mainbPtr);
        }
        /* hmm... i think we lose name[] here and therefore
           outputFileName. do we need it?*/

    }

    if (argsbPtr->wipeFlag) {
        /* destroy every trace of plaintext */
        if (wipefile(filebPtr, argsPrivbPtr->inputFileName) == 0) {
            remove(argsPrivbPtr->inputFileName);
            fprintf(filebPtr->pgpout, LANG("\nFile %s wiped and deleted. "),
                    argsPrivbPtr->inputFileName);

            fprintf(filebPtr->pgpout, "\n");
        }
    }
    *perrorLvl=EXIT_OK;
    return 0;
}

void usage(struct pgpfileBones *filebPtr, int *perrorLvl)
{
    char helpFileName[MAX_PATH];
    char *tmphelp = helpFileName;

    buildHelpFileName( filebPtr, helpFileName, "" );

    if (get_ext_c_ptr()) {
        /* conversion to external format necessary */
        if((tmphelp = tempFile( filebPtr, TMP_TMPDIR, perrorLvl))==NULL &&
                *perrorLvl != 0)
            return;
        set_CONVERSION( EXT_CONV );
        if (copyFilesByName( filebPtr, helpFileName, tmphelp ) < 0) {
            rmTemp( filebPtr, tmphelp );
            tmphelp = helpFileName;
        }
        set_CONVERSION ( NO_CONV );
    }
    /* built-in help if pgp.hlp is not available */
    if (moreFile( filebPtr, tmphelp ) < 0) {
        fprintf(filebPtr->pgpout, LANG("\nUsage summary:\
\nTo encrypt a plaintext file with recipent's public key, type:\
\n   pgp -e textfile her_userid [other userids] (produces textfile.pgp)\
\nTo sign a plaintext file with your secret key:\
\n   pgp -s textfile [-u your_userid]           (produces textfile.pgp)\
\nTo sign a plaintext file with your secret key, and then encrypt it\
\n   with recipent's public key, producing a .pgp file:\
\n   pgp -es textfile her_userid [other userids] [-u your_userid]\
\nTo encrypt with conventional encryption only:\
\n   pgp -c textfile\
\nTo decrypt or check a signature for a ciphertext (.pgp) file:\
\n   pgp ciphertextfile [plaintextfile]\
\nTo produce output in ASCII for email, add the -a option to other options.\
\nTo generate your own unique public/secret key pair:  pgp -kg\
\nFor help on other key management functions, type:   pgp -k\n"));

        if( !filebPtr->envbPtr->compatible )
            fprintf(filebPtr->pgpout, LANG("\
For help on group management functions, type:   pgp -g\n\n"));
    }

    if (get_ext_c_ptr())
        rmTemp( filebPtr, tmphelp );
    *perrorLvl=BAD_ARG_ERROR;
    return;
}

void keyUsage(struct pgpfileBones *filebPtr, int *perrorLvl)
{
    char helpFileName[MAX_PATH];
    char *tmphelp = helpFileName;
    PGPEnv *env = filebPtr->envbPtr->m_env; PGPInt32  pri; PGPError err;
    PGPBoolean quietmode = pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err);

    if (quietmode) return;

    buildHelpFileName( filebPtr, helpFileName, "key" );

    if (get_ext_c_ptr()) {
        /* conversion to external format necessary */
        if((tmphelp = tempFile( filebPtr, TMP_TMPDIR, perrorLvl))==NULL &&
                *perrorLvl != 0)
            return;
        set_CONVERSION ( EXT_CONV );
        if (copyFilesByName( filebPtr, helpFileName, tmphelp ) < 0) {
            rmTemp( filebPtr, tmphelp );
            tmphelp = helpFileName;
        }
        set_CONVERSION ( NO_CONV );
    }
    /* built-in help if pgp.hlp is not available */
    if ( moreFile( filebPtr, tmphelp ) < 0)
        /* only use built-in help if there is no helpfile */
        fprintf(filebPtr->pgpout, LANG("\nKey management functions:\
\nTo generate your own unique public/secret key pair:\
\n   pgp -kg\
\nTo add a key file's contents to your public, secret key ring or key server:\
\n   pgp -ka keyfile [keyring] \
\nTo remove a key or user ID from your public, secret key ring or key server:\
\n   pgp -kr userid [keyring | URL]\
\nTo edit your user ID or pass phrase:\
\n   pgp -ke your_userid [keyring]\
\nTo extract (copy) a key from your public or secret key ring:\
\n   pgp -kx userid keyfile [keyring | URL]\
\nTo view the contents of your public key ring or query the keyserver:\
\n   pgp -kv[v] [userid] [keyring | URL]\
\nTo check signatures on your public key ring:\
\n   pgp -kc [userid] [keyring]\
\nTo sign someone else's public key on your public key ring:\
\n   pgp -ks her_userid [-u your_userid] [keyring]\
\nTo remove selected signatures from a userid on a keyring or key server:\
\n   pgp -krs userid [keyring] \
\n"));

    *perrorLvl=BAD_ARG_ERROR;
    return;
}

void groupUsage(struct pgpfileBones *filebPtr, int *perrorLvl)
{
    char helpFileName[MAX_PATH];
    char *tmphelp = helpFileName;
    PGPEnv *env = filebPtr->envbPtr->m_env; PGPInt32  pri; PGPError err;
    PGPBoolean quietmode = pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err);

    if (quietmode) return;

    buildHelpFileName( filebPtr, helpFileName, "key" );

    if (get_ext_c_ptr()) {
        /* conversion to external format necessary */
        if((tmphelp = tempFile( filebPtr, TMP_TMPDIR, perrorLvl))==NULL &&
                *perrorLvl != 0)

            return;
        set_CONVERSION ( EXT_CONV );
        if (copyFilesByName( filebPtr, helpFileName, tmphelp ) < 0) {
            rmTemp( filebPtr, tmphelp );
            tmphelp = helpFileName;
        }
        set_CONVERSION ( NO_CONV );
    }
    /* built-in help if pgp.hlp is not available */
    if ( moreFile( filebPtr, tmphelp ) < 0)
        /* only use built-in help if there is no helpfile */
        fprintf(filebPtr->pgpout, LANG("\nGroup management functions:\
\nTo add keys to a group or create a new group:\
\n   pgp -ga groupname [ userids... ]\
\nTo remove a key from a group, or remove a group:\
\n   pgp -gr groupname [ userids... ]\
\nTo view the groups or the contents of a group:\
\n   pgp -gv[v] [groupname] [keyring]\
\n"));

    *perrorLvl=BAD_ARG_ERROR;
    return;
}

/*
 * Check for language specific help files in PGPPATH, then the system
 * directory.  If that fails, check for the default pgp.hlp, again
 * firat a private copy, then the system-wide one.
 *
 * System-wide copies currently only exist on Unix.
 */
static void buildHelpFileName(struct pgpfileBones *filebPtr, char
        *helpFileName, char const *extra )
{
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;

    if (strcmp( pgpenvGetCString( env, PGPENV_LANGUAGE, &pri ), "en")) {
        buildFileName(helpFileName, pgpenvGetCString( env,
                PGPENV_LANGUAGE, &pri ));

        strcat(helpFileName, extra);
        forceExtension( filebPtr, helpFileName, filebPtr->HLP_EXTENSION );
        if (fileExists(helpFileName))
            return;
#ifdef PGP_SYSTEM_DIR
        strcpy(helpFileName, PGP_SYSTEM_DIR);
        strcat(helpFileName, pgpenvGetCString( env, PGPENV_LANGUAGE, &pri ));
        strcat(helpFileName, extra);
        forceExtension( filebPtr, helpFileName, filebPtr->HLP_EXTENSION );
        if (fileExists(helpFileName))
            return;
#endif
    }
    buildFileName(helpFileName, "pgp");
    strcat(helpFileName, extra);
    forceExtension( filebPtr, helpFileName, filebPtr->HLP_EXTENSION );
#ifdef PGP_SYSTEM_DIR
    if (fileExists(helpFileName))
        return;
    strcpy(helpFileName, PGP_SYSTEM_DIR);
    strcat(helpFileName, "pgp");
    strcat(helpFileName, extra);
    forceExtension( filebPtr, helpFileName, filebPtr->HLP_EXTENSION );
#endif
}












































