/*___________________________________________________________________________
    doencode.c

    Copyright 1998 by Network Associates Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    uses the PGP SDK to encode messages.

    $Id: doencode.c,v 1.12 1999/05/12 21:01:03 sluu Exp $
____________________________________________________________________________*/
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUserInterface.h"
#include "pgpUtilities.h"
#include "pgpEnv.h"
#include "pgpContext.h"

#include "config.h"
#include "usuals.h"
#include "language.h"
#include "pgp.h"
#include "globals.h"
#include "fileio.h"
#include "exitcodes.h"
#include "prototypes.h"

#define RECIP_MAGIC 0x4fd450c0

struct RecipList {
    long magic;
    int size,maxsize;
    char **recips;
    /* a null-terminated array of pointers to strings, all privately
    allocated, specifying the list of recipients */

};


/* count the nubmer of strings in the array. terminates with NULL or
   the first empty string.. */
static int pgpCountStrArray( char **ptr )
{
    int i;
    if(!ptr)
        return 0;
    for(i=0; *ptr && *ptr[0]; ptr++, i++)
        ;
    return i;
}

static PGPError pgpNewRecipList( int maxsize, struct RecipList *reclist )
{
    int i;
    char **recips;

    assert( reclist->magic != RECIP_MAGIC );

    if ((recips= (char **) malloc((maxsize +1) * sizeof(char *))) == NULL)
        return kPGPError_OutOfMemory;
    for(i=0; i<=maxsize; i++)
        recips[i]=NULL;
    reclist->recips = recips;
    reclist->size = 0;
    reclist->maxsize = maxsize;
    reclist->magic = RECIP_MAGIC;
    return kPGPError_NoErr;
}

 /* add a recipient to the list. */
static PGPError pgpAddRecip( struct RecipList *reclist, char *recip )
{
    char *p;
    assert( reclist->magic == RECIP_MAGIC );

    if(reclist->size >= reclist->maxsize)
        return -1;
    if((p = (char *)malloc(strlen( recip ) + 1))==NULL)
        return kPGPError_OutOfMemory;
    strcpy(p,recip);
    reclist->recips[ reclist->size ] = p;
    reclist->size++;
    return kPGPError_NoErr;
}

static PGPError pgpFreeRecipList( struct RecipList *reclist )
{
    int i;
    assert( reclist->magic == RECIP_MAGIC );
    /*return -1;*/
    for(i=0; i< reclist->size; i++)
        free( reclist->recips[ i ] );
    free( reclist->recips );
    reclist->magic = 0;
    return kPGPError_NoErr;
}


/*
   Build the list of recipients, from command line arguments or whatever.
   If necessary, prompt the user.  The caller is responsible to call
   pgpFreeRecipList().
 */

PGPError pgpBuildRecipList( struct pgpmainBones *mainbPtr, struct
        RecipList *reclist )
{
    char mcguffin[256];
    int numr;
    PGPInt32 pri;
    PGPError err;
    char **pp;

    struct pgpfileBones *filebPtr=mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr=mainbPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;

    numr = pgpCountStrArray( mainbPtr->recipients );

    if (!envbPtr->filterMode && !pgpenvGetInt( env, PGPENV_NOOUT, &pri,
            &err ))

        fprintf(filebPtr->pgpout, LANG(
"\n\nRecipients' public key(s) will be used to encrypt.\n"));

    if ( numr == 0 ) {
        /* no Recipient specified on command line */
        fprintf(filebPtr->pgpout, LANG(
"\nA user ID is required to select the Recipient's public key.\n"));
        fprintf(filebPtr->pgpout, LANG("Enter the Recipient's user ID: "));
        pgpTtyGetString(mcguffin, 255, filebPtr->pgpout);

        if((err = pgpNewRecipList( 1, reclist )) != 0)
            return err;
        err = pgpAddRecip( reclist, mcguffin );
    } else {
        if((err = pgpNewRecipList( numr, reclist )) != 0)
            return err;
        for( pp= mainbPtr->recipients; pp && *pp && **pp; pp++)
            if((err = pgpAddRecip( reclist, *pp )) != 0)
                return err;
    }
    pgpAssertNoErr(err);

    for( pp = reclist->recips; pp && *pp && **pp; pp++ )
        CONVERT_TO_CANONICAL_CHARSET(*pp);
    return kPGPError_NoErr;
}


PGPError enchandler(PGPContextRef context, struct PGPEvent *event,
        PGPUserValue userValue)
{
    struct pgpmainBones *mainbPtr = (struct pgpmainBones *)userValue;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = pgpContextGetEnvironment( context );
    PGPUInt32 need;
    PGPError err;
    PGPInt32 pri;
    PGPInt32 verbose = pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err );
    PGPBoolean compatible = envbPtr->compatible;

    /* get the event type*/
    switch ( event->type ) {
        case kPGPEvent_FinalEvent:
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("event %d: final\n"),
                         event->type);
            err = kPGPError_NoErr;
            /* tear down anything we need to here*/
            break;

        case kPGPEvent_InitialEvent:
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("\nevent %d: initial\n"),
                         event->type);
            err = kPGPError_NoErr;
            /* set up whatever we need to here*/
            break;

        case kPGPEvent_ErrorEvent:
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("event %d: error %d\n"),
                         event->type,event->data.errorData.error);
                pgpShowError( filebPtr, event->data.errorData.error,
                        __FILE__,__LINE__);

            if(errno == ENOSPC)
                fprintf(filebPtr->pgpout,
                        LANG("Error: No space left on device.\n"));

            err = kPGPError_NoErr;
            break;

        case kPGPEvent_WarningEvent:
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("event %d: warning %d\n"),
                         event->type,event->data.warningData.warning);
            if( event->data.warningData.warning == kPGPError_KeyInvalid )
            {
                /*PGPKeySetRef warnset = event->data.warningData.warningArg;*/
            }
            break;

        case kPGPEvent_EntropyEvent:
            /* how much entropy do we need?*/

            need = event->data.entropyData.entropyBitsNeeded;
            if(verbose)
                fprintf( filebPtr->pgpout,
                        LANG("event %d: entropy needed: %d\n"),
                        event->type, need);

            /* usually equals this...*/
            /*need = PGPGlobalRandomPoolGetMinimumEntropy()*/
            /*    - PGPGlobalRandomPoolGetEntropy();*/

#if PGP_UNIX || PGP_WIN32
            err = pgpAcquireEntropy( filebPtr, need );
#else
            err = PGPCollectRandomDataDialog( context, need,
                    PGPOLastOption( context ) );

#endif

            if( !compatible && IsPGPError( err ) ) {
                if(verbose)
                    pgpShowError(filebPtr,err,__FILE__,__LINE__);
            }
            break;

        default:
            /* ignore the event...*/
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("event %d: unknown\n"),
                        event->type);

            err = kPGPError_NoErr;
    }
    return err;
}

/*
  During encode, if invalid keys are used then a warning is generated
  but it's too late to ask the user. This should be called before all
  that takes place.
 */
PGPError pgpUnionKeysAskInvalid( struct pgpfileBones *filebPtr,
        PGPKeySetRef fromSet, PGPKeySetRef toSet, PGPKeySetRef *resultset )
{
    PGPEnv *env = filebPtr->envbPtr->m_env;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyRef key;
    PGPError err;
    PGPInt32 pri;
    PGPBoolean batchmode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err );
    PGPBoolean verbose = pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err );
    PGPBoolean quietmode = pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err);
    PGPKeySetRef tmpset;

    err = PGPNewEmptyKeySet( toSet, &tmpset );
    pgpAssertNoErr(err);
    err = PGPUnionKeySets( toSet, tmpset, resultset );
    pgpAssertNoErr(err);
    PGPFreeKeySet( tmpset );

    err = PGPOrderKeySet( fromSet, kPGPAnyOrdering, &keylist );
    if( IsPGPError(err) )
        goto done;

    err = PGPNewKeyIter( keylist, &keyiter );
    if( IsPGPError(err) )
        goto done;

    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNext( keyiter, &key);
    /* if err, there are none.*/

    while( key != NULL )
    {
        PGPInt32 validity;

        if (!quietmode) pgpShowKeyBrief( filebPtr, key );

        err = PGPGetKeyNumber(key, kPGPKeyPropValidity, &validity);
        pgpAssertNoErr(err);

        if(verbose)
            pgpShowKeyValidity( filebPtr, key );

        if( validity < kPGPValidity_Marginal ) {
            char useridstr[ kPGPMaxUserIDSize ];
            PGPBoolean answer;
            err = pgpGetUserIDStringFromKey( key, useridstr );
            pgpAssertNoErr(err);

            fprintf( filebPtr->pgpout, LANG(
"WARNING:  Because this public key is not certified with a trusted\n"));
            fprintf( filebPtr->pgpout, LANG(
"signature, it is not known with high confidence that this public key\n"));
            fprintf( filebPtr->pgpout, LANG(
"actually belongs to: \"%s\".\n"), useridstr );

            /*
               XXX need to find out whether we previously approved using
               the key, and if so don't ask, but print the following
               message...  But you previously approved using this public
               key anyway. Trouble is, I don't know how to do that here.
             */

            if (!(batchmode && quietmode)) fprintf( filebPtr->pgpout, LANG(
    "\nAre you sure you want to use this public key (y/N)?") );

            answer = getyesno( filebPtr, 'N', batchmode );
            if( !answer ) {
                if (!quietmode) fprintf( filebPtr->pgpout,
                        LANG("Ok, skipping userid %s\n"), useridstr );

                goto next;
            }
        }

        {
            PGPKeySetRef myset;
            err = PGPNewSingletonKeySet( key, &myset );
            err = PGPUnionKeySets( myset, *resultset, &tmpset );
            PGPFreeKeySet( *resultset );
            PGPFreeKeySet( myset );
            *resultset = tmpset;
        }
        pgpAssertNoErr(err);
next:
        err = PGPKeyIterNext( keyiter, &key);
        /* if err, there are no more*/
    }
    if( err == kPGPError_EndOfIteration )
        err = kPGPError_NoErr;
done:
    if(keyiter)
        PGPFreeKeyIter(keyiter);
    if(keylist)
        PGPFreeKeyList(keylist);
    return err;
}

int signfile(struct pgpmainBones *mainbPtr, PGPBoolean separateSignature,
        char *infile, char *outfile, PGPBoolean attemptCompression )
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpargsBones *argsbPtr = mainbPtr->argsbPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = pgpContextGetEnvironment( context );
    PGPBoolean mine=FALSE;

    PGPFileSpecRef infilespec = NULL, outfilespec = NULL;
    PGPKeySetRef keyringset = NULL;
    PGPKeyRef mykey=NULL;
    PGPError err,er2;
    PGPInt32 pri;
    char *passphrase;
    PGPBoolean compatible = envbPtr->compatible;

    PGPBoolean textmode = pgpenvGetInt( env, PGPENV_TEXTMODE, &pri, &err);
    PGPBoolean batchMode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err );
    PGPBoolean armorout = pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err);

    PGPBoolean clearsig = pgpenvGetInt( env, PGPENV_CLEARSIG, &pri, &err)
                && textmode && armorout && !argsbPtr->stripSigFlag;
        /* clear-sign only makes sense if file is text and armor is on */

    err = PGPNewFileSpecFromFullPath(context, infile, &infilespec);
    pgpAssertNoErr(err);
    err = PGPNewFileSpecFromFullPath(context, outfile, &outfilespec);
    pgpAssertNoErr(err);

    err = PGPOpenDefaultKeyRings( context, 0, &keyringset );

    if( IsPGPError( err )) {
        fprintf( filebPtr->pgpout, LANG("\nCan't open key rings\n"));
        goto done;
    }

    mainbPtr->workingRingSet=keyringset;
    /*mainbPtr->workingGroupSet=NULL;*/
    err = pgpGetMySigningKey( mainbPtr, &mykey );
    if( IsPGPError(err)) {
        pgpShowError(filebPtr, err,0,0); /*, __FILE__,__LINE__);*/
        goto done;
    }

    /* should we add all the passphrases we know of to the job options?*/
    err = pgpRewindPassphrase( envbPtr->passwds );
    pgpAssertNoErr(err);

    err = pgpGetValidPassphrase( mainbPtr, mykey, &passphrase, &mine );

    if( IsntPGPError( err ) ) {

        /* encode the file... */
        err = PGPEncode( context,
                PGPOInputFile( context, infilespec ),
                PGPOOutputFile( context, outfilespec ),

                argsbPtr->signFlag ? PGPOSignWithKey( context, mykey,
                        PGPOPassphrase( context, passphrase ),
                        PGPOLastOption( context )  ) : PGPONullOption(
                                context ),
                argsbPtr->signFlag ? PGPOClearSign( context, clearsig )
                        : PGPONullOption( context ),
                argsbPtr->stripSigFlag ? PGPODetachedSig( context,
                        PGPOLastOption( context ) ) : PGPONullOption(
                                context ),
                PGPODataIsASCII( context, textmode ),
                PGPOCompression( context, attemptCompression),
                PGPOAskUserForEntropy( context, !batchMode ),
                PGPOArmorOutput(context, armorout ),

                /* warn below validity,*/
                PGPOEventHandler( context, enchandler, (PGPUserValue)
                        mainbPtr),

                PGPOLastOption( context ) );

        if( !compatible && IsPGPError( err ) )
            pgpShowError(filebPtr,err,__FILE__,__LINE__);

        if (mine) {
            er2 = PGPFreeData( passphrase );
            pgpRemoveFromPointerList( mainbPtr->leaks, passphrase );
            pgpAssertNoErr(er2);
        }
    } else if( !compatible )
        pgpShowError(filebPtr,err,__FILE__,__LINE__);
done:
    if(keyringset) {
        PGPFreeKeySet( keyringset );
        mainbPtr->workingRingSet=NULL;
    }
    if(infilespec)
        PGPFreeFileSpec(infilespec);
    if(outfilespec)
        PGPFreeFileSpec(outfilespec);
    return err;
}

PGPError issueAlgorithmWarning(struct pgpmainBones * mainbPtr,
                      PGPKeySetRef keyset)
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyRef key = NULL;
    PGPError err;
    PGPUInt32 algorithm;
    PGPBoolean RSAKeys = FALSE, nonRSAKeys = FALSE;
    /* Warn if user is encrypting to both RSA and non-RSA keys. */

    err = PGPOrderKeySet( keyset, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( keyiter, &key);
    /*if error, no keys found.*/

    while( key != NULL )
    {
        err = PGPGetKeyNumber(key, kPGPKeyPropAlgID, &algorithm);
        pgpAssertNoErr(err);

        switch (algorithm) {
            case kPGPPublicKeyAlgorithm_RSA:
            RSAKeys = TRUE;
            break;
            default:
            nonRSAKeys = TRUE;
        }

        err = PGPKeyIterNext( keyiter, &key);
        /*if err, no more keys*/
    }
    if(err == kPGPError_EndOfIteration)
        err = kPGPError_NoErr;

    if(keyiter)
        PGPFreeKeyIter( keyiter );
    if(keylist)
        PGPFreeKeyList( keylist );

    if ( RSAKeys && nonRSAKeys) {
        fprintf(filebPtr->pgpout,
LANG("WARNING: Encrypting to both RSA and non-RSA keys.\n"));
        fprintf(filebPtr->pgpout,
LANG("         PGP 2.6.2 users may not be able to decrypt this message.\n"));
    }
    return err;

}


int encryptFile(struct pgpmainBones *mainbPtr, const char **recipients,
        char *infile, char *outfile, PGPBoolean attemptCompression)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpargsBones *argsbPtr = mainbPtr->argsbPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = pgpContextGetEnvironment( context );

    PGPFileSpecRef infilespec=NULL, outfilespec=NULL;
    PGPKeySetRef keyringset=NULL, resultset=NULL;
    PGPUInt32 numkeys;
    PGPKeyRef mykey=NULL;
    PGPError err,er2;
    PGPInt32 pri;
    char *passphrase;
    PGPBoolean mine=FALSE;
    PGPBoolean compatible = envbPtr->compatible;

    PGPBoolean textmode = pgpenvGetInt( env, PGPENV_TEXTMODE, &pri, &er2);
    PGPBoolean batchMode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &er2 );
    PGPBoolean armorout = pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err);
    PGPBoolean encrypttoself = pgpenvGetInt( env, PGPENV_ENCRYPTTOSELF,
            &pri, &er2);
    PGPBoolean clearsig;

    clearsig = pgpenvGetInt( env, PGPENV_CLEARSIG, &pri, &er2);
    /* prevent certain feature interactions with clearsig.*/
    if( pri <= PGPENV_PRI_PUBDEFAULT )
        clearsig = FALSE;

    err = PGPNewFileSpecFromFullPath(context, infile, &infilespec);
    pgpAssertNoErr(err);
    err = PGPNewFileSpecFromFullPath(context, outfile, &outfilespec);
    pgpAssertNoErr(err);

    err = PGPOpenDefaultKeyRings( context, 0, &keyringset );

    if( IsPGPError( err )) {
        fprintf( filebPtr->pgpout, LANG("\nCan't open key rings\n"));
        PGPFreeFileSpec( infilespec );
        PGPFreeFileSpec( outfilespec );
        return -1;
    }

    if( argsbPtr->signFlag || encrypttoself) {
        mainbPtr->workingRingSet=keyringset;
        /*mainbPtr->workingGroupSet=NULL;*/
        err = pgpGetMySigningKey( mainbPtr, &mykey );
        if( IsPGPError(err) ) {
            pgpShowError(filebPtr, err, 0,0); /*__FILE__,__LINE__);*/
            goto done;
        }
        if(argsbPtr->signFlag)
            err = pgpGetValidPassphrase( mainbPtr, mykey, &passphrase,
                    &mine );

    }

    /* build a key set containing the public keys of the recipients...*/
    if( IsntPGPError(err)) {
        mainbPtr->workingRingSet = keyringset;

        err = pgpBuildKeySetFromUserIDStringList( mainbPtr, recipients,
                &resultset );

        if( IsPGPError(err) )
            goto done;
    }

    if( IsntPGPError(err) && encrypttoself )
    {
        PGPKeySetRef tmpset,myset;
        err = PGPNewSingletonKeySet( mykey, &myset );
        err = PGPUnionKeySets( myset, resultset, &tmpset );
        PGPFreeKeySet( resultset );
        PGPFreeKeySet( myset );
        resultset = tmpset;
    }

    if ( IsntPGPError(err) ) {
        PGPCountKeys( resultset, &numkeys );

        err = issueAlgorithmWarning(mainbPtr, resultset);
        pgpAssertNoErr(err);

        if( pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err) ) {
            fprintf( filebPtr->pgpout, LANG("\nnum keys : %d keys\n"),
                    numkeys);
            fprintf( filebPtr->pgpout, LANG("armor out: %d\n"), armorout );
            fprintf( filebPtr->pgpout, LANG("sign flag: %d\n"),
                    argsbPtr->signFlag  );
            if(argsbPtr->signFlag)
                fprintf( filebPtr->pgpout, LANG("clearsign: %d\n"),
                        clearsig );
            fprintf( filebPtr->pgpout, LANG("strip sig: %d\n"),
                    argsbPtr->stripSigFlag  );
            fprintf( filebPtr->pgpout, LANG("textmode : %d\n"), textmode );
            fprintf( filebPtr->pgpout, LANG("compressn: %d\n"),
                    attemptCompression);
            fprintf( filebPtr->pgpout, LANG("youreyeso: %d\n"),
                    envbPtr->moreFlag );
            fprintf( filebPtr->pgpout, LANG("batchMode: %d\n"), batchMode );
        }

        /* encrypt the file... */
        err = PGPEncode( context,
                PGPOInputFile( context, infilespec ),
                PGPOOutputFile( context, outfilespec ),

                argsbPtr->signFlag ? PGPOSignWithKey( context, mykey,
                        PGPOPassphrase( context, passphrase ),
                        PGPOLastOption( context )  ) : PGPONullOption(
                                context ),
                argsbPtr->signFlag ? PGPOClearSign( context, clearsig )
                : PGPONullOption( context ),
                argsbPtr->stripSigFlag ? PGPODetachedSig( context,
                        PGPOLastOption( context ) ) : PGPONullOption(
                        context ),
                PGPODataIsASCII( context, textmode ),
                PGPOCompression( context, attemptCompression),
                PGPOForYourEyesOnly( context, envbPtr->moreFlag ),
                PGPOAskUserForEntropy( context, !batchMode ),
                PGPOEncryptToKeySet( context, resultset ),
                PGPOArmorOutput(context, armorout ),

                /* warn below validity,*/
                PGPOEventHandler( context, enchandler, (PGPUserValue)
                        mainbPtr),
                PGPOLastOption( context ) );

        if( mine ) {
            PGPFreeData(passphrase);
            pgpRemoveFromPointerList( mainbPtr->leaks, passphrase );
        }
    }

    if( !compatible && IsPGPError( err ) )
        pgpShowError( filebPtr, err,__FILE__,__LINE__ );

done:
    if( resultset )
        PGPFreeKeySet( resultset );
    if( keyringset ) {
        PGPFreeKeySet( keyringset );
        mainbPtr->workingRingSet=NULL;
    }
    if( infilespec )
        PGPFreeFileSpec(infilespec);
    if( outfilespec )
        PGPFreeFileSpec(outfilespec);
    return err;
}

int conventionalEncryptFile(struct pgpmainBones *mainbPtr, char *infile,
        char *outfile, PGPBoolean attemptCompression)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpargsBones *argsbPtr = mainbPtr->argsbPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = pgpContextGetEnvironment( context );
    PGPBoolean mine=FALSE, signMine=FALSE;

    PGPFileSpecRef infilespec = NULL, outfilespec = NULL;
    PGPKeySetRef keyringset=NULL;
    PGPKeyRef mykey=NULL;
    PGPError err,er2;
    PGPInt32 pri;
    char *passphrase, *signPassphrase;

    PGPBoolean textmode = pgpenvGetInt( env, PGPENV_TEXTMODE, &pri, &er2);
    PGPBoolean batchMode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &er2 );
    PGPBoolean armorout = pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err);

    PGPBoolean compatible = envbPtr->compatible;

    PGPBoolean clearsig;

    clearsig = pgpenvGetInt( env, PGPENV_CLEARSIG, &pri, &er2);
    /* prevent certain feature interactions with clearsig.*/
    if( pri <= PGPENV_PRI_PUBDEFAULT )
        clearsig = FALSE;

    err = PGPNewFileSpecFromFullPath(context, infile, &infilespec);
    pgpAssertNoErr(err);
    err = PGPNewFileSpecFromFullPath(context, outfile, &outfilespec);
    pgpAssertNoErr(err);

    err = PGPOpenDefaultKeyRings( context, 0, &keyringset );

    if( IsPGPError( err )) {
        fprintf( filebPtr->pgpout, LANG("\nCan't open key rings\n"));
        PGPFreeFileSpec( infilespec );
        PGPFreeFileSpec( outfilespec );
        return -1;
    }

    err = pgpRewindPassphrase( envbPtr->passwds );
    pgpAssertNoErr(err);

    /* set up mykey*/
    if( argsbPtr->signFlag) {
        mainbPtr->workingRingSet=keyringset;
        err = pgpGetMySigningKey( mainbPtr, &mykey );

        if( IsPGPError(err) ) {
            pgpShowError(filebPtr, err, 0,0); /*__FILE__,__LINE__);*/
            goto done;
        }

        err = pgpGetValidPassphrase( mainbPtr, mykey, &signPassphrase,
                &signMine );
    }

    /* should we add all the passphrases we know of to the job options?*/
    /* Try any passphrases already stored first,
           then try each one until we get a hit. once they're used up, */
    pgpNextPassphrase( envbPtr->passwds, &passphrase );

    if(passphrase == NULL) {
        mine=TRUE;

        if (!pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err ))
            fprintf(filebPtr->pgpout,
            LANG("\nYou need a pass phrase to encrypt the file. "));

        err = pgpPassphraseDialogCmdline( mainbPtr, TRUE, NULL,
                &passphrase );

        if IsntPGPError(err) {
            err = pgpAppendToPassphraseList( envbPtr->passwds, passphrase );
            pgpAssertNoErr(err);
        }
    }

    if( IsntPGPError( err ) ) {

          if( pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err) )
          {
              fprintf( filebPtr->pgpout, LANG("armor out: %d\n"), armorout );
              fprintf( filebPtr->pgpout, LANG("sign flag: %d\n"),
                      argsbPtr->signFlag  );
              if(argsbPtr->signFlag)
                  fprintf( filebPtr->pgpout, LANG("clearsign: %d\n"),
                          clearsig );
              fprintf( filebPtr->pgpout, LANG("strip sig: %d\n"),
                      argsbPtr->stripSigFlag  );
              fprintf( filebPtr->pgpout, LANG("textmode : %d\n"), textmode );
              fprintf( filebPtr->pgpout, LANG("compressn: %d\n"),
                      attemptCompression);
              fprintf( filebPtr->pgpout, LANG("youreyeso: %d\n"),
                      envbPtr->moreFlag );
              fprintf( filebPtr->pgpout, LANG("batchMode: %d\n"),
                      batchMode );

          }



          /* encrypt the file... */
          err = PGPEncode( context,
                  PGPOInputFile( context, infilespec ),
                  PGPOOutputFile( context, outfilespec ),

                  argsbPtr->signFlag ? PGPOSignWithKey( context, mykey,
                      PGPOPassphrase( context, signPassphrase ),
                      PGPOLastOption( context )  ) : PGPONullOption(
                      context ),
                  argsbPtr->signFlag ? PGPOClearSign( context, clearsig )
                  : PGPONullOption( context ),

                  argsbPtr->stripSigFlag ? PGPODetachedSig( context,
                          PGPOLastOption( context ) ) : PGPONullOption(
                                  context ),

                  PGPODataIsASCII( context, textmode ),
                  PGPOCompression( context, attemptCompression),
                  PGPOForYourEyesOnly( context, envbPtr->moreFlag ),
                  PGPOAskUserForEntropy( context, !batchMode ),
                  PGPOArmorOutput(context, armorout ),
                  PGPOConventionalEncrypt( context, PGPOPassphrase(
                          context, passphrase ), PGPOLastOption( context )),
                  /* warn below validity,*/
                  PGPOEventHandler( context, enchandler, (PGPUserValue)
                          mainbPtr),

                  PGPOLastOption( context ) );

          if (mine) {
              er2 = PGPFreeData( passphrase );
              pgpRemoveFromPointerList( mainbPtr->leaks, passphrase );
              pgpAssertNoErr(er2);
          }

          if (signMine) {
              er2 = PGPFreeData( signPassphrase );
              pgpRemoveFromPointerList( mainbPtr->leaks, signPassphrase );
              pgpAssertNoErr(er2);
          }
    }

    if( !compatible && IsPGPError( err ) )
        pgpShowError( filebPtr, err,__FILE__,__LINE__);

done:

    if( keyringset ) {
        PGPFreeKeySet( keyringset );
        mainbPtr->workingRingSet=NULL;
    }

    er2 = PGPFreeFileSpec(infilespec);
    pgpAssertNoErr(er2);
    er2 = PGPFreeFileSpec(outfilespec);
    pgpAssertNoErr(er2);

    return err;
}

/* Armorout or compress the file without encrypting. */
int encodeOnly(struct pgpmainBones *mainbPtr, char *infile, char *outfile,
        PGPBoolean attemptCompression)
{

    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpargsBones *argsbPtr = mainbPtr->argsbPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = pgpContextGetEnvironment( context );

    PGPFileSpecRef infilespec = NULL, outfilespec = NULL;
    PGPError err,er2;
    PGPInt32 pri;

    PGPBoolean textmode = pgpenvGetInt( env, PGPENV_TEXTMODE, &pri, &er2);
    PGPBoolean batchMode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &er2 );
    PGPBoolean armorout = pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err);

    PGPBoolean compatible = envbPtr->compatible;

    PGPBoolean clearsig;

    clearsig = pgpenvGetInt( env, PGPENV_CLEARSIG, &pri, &er2);
    /* prevent certain feature interactions with clearsig.*/
    if( pri <= PGPENV_PRI_PUBDEFAULT )
        clearsig = FALSE;

    err = PGPNewFileSpecFromFullPath(context, infile, &infilespec);
    pgpAssertNoErr(err);
    err = PGPNewFileSpecFromFullPath(context, outfile, &outfilespec);
    pgpAssertNoErr(err);


    if( IsntPGPError( err ) ) {

          if( pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err) )
          {
              fprintf( filebPtr->pgpout, LANG("armor out: %d\n"), armorout );
              fprintf( filebPtr->pgpout, LANG("sign flag: %d\n"),
                      argsbPtr->signFlag  );
              if(argsbPtr->signFlag)
                  fprintf( filebPtr->pgpout, LANG("clearsign: %d\n"),
                  clearsig );

              fprintf( filebPtr->pgpout, LANG("strip sig: %d\n"),
                      argsbPtr->stripSigFlag  );
              fprintf( filebPtr->pgpout, LANG("textmode : %d\n"), textmode );
              fprintf( filebPtr->pgpout, LANG("compressn: %d\n"),
                      attemptCompression);
              fprintf( filebPtr->pgpout, LANG("youreyeso: %d\n"),
                      envbPtr->moreFlag );
              fprintf( filebPtr->pgpout, LANG("batchMode: %d\n"),
                      batchMode );
          }

          /* encode the file... */
          err = PGPEncode( context,
                  PGPOInputFile( context, infilespec ),
                  PGPOOutputFile( context, outfilespec ),
                  textmode ? PGPODataIsASCII( context, textmode ) :
                  PGPONullOption( context ),

                  attemptCompression ? PGPOCompression( context,
                          attemptCompression) : PGPONullOption( context ),

                  armorout ? PGPOArmorOutput( context, armorout ) :
                  PGPONullOption( context ),

                  PGPOEventHandler( context, enchandler, (PGPUserValue)
                          mainbPtr),

                  PGPOLastOption( context ) );

    }

    if( !compatible && IsPGPError( err ) )
        pgpShowError( filebPtr, err,__FILE__,__LINE__);


    er2 = PGPFreeFileSpec(infilespec);
    pgpAssertNoErr(er2);
    er2 = PGPFreeFileSpec(outfilespec);
    pgpAssertNoErr(er2);

    return err;

}   /* encodeOnly */



 /*
    Do everything, from encapsulation and signing to encryption and armoring.
  */

int pgpDoEncode(struct pgpmainBones *mainbPtr, char *workfilename,
        char **resultfilename, int *perrorLvl )
{
    struct pgpargsBones *argsbPtr=mainbPtr->argsbPtr;
    struct pgpfileBones *filebPtr=mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr=mainbPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err;
    char *tempf;
    int status;
    PGPBoolean attemptCompression;

    /* -- encapsulation ----------------------------------------------- */

    /*
     * See if mainbPtr->plainfilename looks like it might be incompressible,
     * by examining its contents for compression headers for
     * commonly-used compressed file formats like PKZIP, etc.
     * Remember this information for later, when we are deciding
     * whether to attempt compression before encryption.
     *
     * Naturally, don't bother if we are making a separate signature or
     * clear-signed message.  Also, don't bother trying to compress a
     * PGP message, as it's probably already compressed.
     */
    attemptCompression = pgpenvGetInt( env, PGPENV_COMPRESS, &pri, &err )
        && !mainbPtr->separateSignature && envbPtr->makeRandom == 0 &&
        fileCompressible(mainbPtr->plainfilename);

    /* -- signing ----------------------------------------------------- */

    if (argsbPtr->signFlag) {
        if (!envbPtr->filterMode && !pgpenvGetInt( env, PGPENV_NOOUT,
                &pri, &err ))
            fprintf(filebPtr->pgpout,
                    LANG("\nA secret key is required to make a signature. "));
    }


    /* first create a tempfile to hold the encrypted or signed output. if
       armor or output-stdout then the tempfile will be wiped upon
       exit(). */

    if (pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err ) ||
            mainbPtr->outputStdout) {

        if((tempf = tempFile( filebPtr, TMP_WIPE | TMP_TMPDIR, perrorLvl))
                == NULL && *perrorLvl != 0)
            return -1;
    } else {
        if((tempf = tempFile( filebPtr, TMP_WIPE, perrorLvl)) == NULL &&
                *perrorLvl != 0)
            return -1;
    }

    /* -- signing only ---------------------------------------------- */
    if (argsbPtr->signFlag && !argsbPtr->encryptFlag) {
        status = signfile(mainbPtr, mainbPtr->separateSignature,
                workfilename, tempf, attemptCompression );

        if (status < 0) {
            fprintf(filebPtr->pgpout, LANG("\007Signature error\n"));
            *perrorLvl = SIGNATURE_ERROR;
            userError(filebPtr,perrorLvl);
            return -1;
        }

        *resultfilename = tempf;
        return 0;
    }

    /* -- encryption --------------------------------------------------- */
    if (argsbPtr->encryptFlag) {
        /* conventional or public-key encrypt?
           if public-key encrypt, get the recipients first. */

        if (!argsbPtr->conventionalFlag) {
            struct RecipList reclist;
            if((err = pgpBuildRecipList(mainbPtr, &reclist)) != 0) {
                *perrorLvl = OUT_OF_MEM;
                return -1;
            }
            status = encryptFile(mainbPtr, (const char **)reclist.recips,
                    workfilename, tempf, attemptCompression);

            pgpFreeRecipList( &reclist );

        } else {
            status = conventionalEncryptFile(mainbPtr, workfilename,
                    tempf, attemptCompression);

        }

        *resultfilename = tempf;

        if (status < 0) {
            fprintf(filebPtr->pgpout, LANG("\007Encryption error\n"));
            *perrorLvl = (argsbPtr->conventionalFlag ? ENCR_ERROR :
                    PK_ENCR_ERROR);

            userError(filebPtr,perrorLvl);
            return -1;
        }
    } else if (attemptCompression && !mainbPtr->separateSignature) {
        /*
         * PGP used to be parsimonious about compressin; originally, it only
         * did it for files that were being encrypted (to reduce the
         * redundancy in the plaintext), but it should really do it for
         * anything where it's not a bad idea.
         */
        if (pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err ) ||
                mainbPtr->outputStdout) {
            if((tempf = tempFile( filebPtr, TMP_WIPE | TMP_TMPDIR,
                    perrorLvl))==NULL && *perrorLvl != 0)
                return -1;
        } else {
            if((tempf = tempFile( filebPtr, TMP_WIPE, perrorLvl))==NULL &&
                    *perrorLvl != 0)
                return -1;
        }
        status = encodeOnly( mainbPtr, workfilename, tempf,
                attemptCompression );

        *resultfilename = tempf;

        if (status < 0) {
            fprintf(filebPtr->pgpout, LANG("\007Encryption error\n"));
            *perrorLvl = (argsbPtr->conventionalFlag ? ENCR_ERROR :
                    PK_ENCR_ERROR);
            userError(filebPtr,perrorLvl);
            return -1;
        }

    }
    return 0;
}