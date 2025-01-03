/*____________________________________________________________________________
    keysign.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    use the PGP SDK to sign the key on the keyRing, PGP262 style.

    $Id: keysign.c,v 1.10 1999/05/12 21:01:04 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <string.h>

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpErrors.h"

#include "usuals.h"
#include "pgp.h"
#include "globals.h"
#include "prototypes.h"
#include "language.h"


/* simple case insensitive prefix match. */
static PGPBoolean prefix( char *srch, char *text )
{
    for( ; *srch && *text; srch++,text++ )
        if( toUpper( *srch ) != toUpper( *text ) )
            return FALSE;
    if( *srch )
        return FALSE;
    return TRUE;
}

/* simple case insensitive substring match. */
static PGPBoolean substr( char *srch, char *text )
{
    while( *text ) {
        while( *text && toUpper(*srch) != toUpper(*text) )
            text++;

        if( prefix( srch, text ) )
            return TRUE;

        if( *text )
            text++;
    }
    return FALSE;
}

PGPError pgpGetKeyUserIDFromSubstring( PGPKeyRef key, const char
        *useridstr, PGPUserIDRef *userID )
{
    PGPKeyIterRef iter;
    PGPKeySetRef single = NULL;
    PGPKeyListRef keylist = NULL;
    PGPError err;

    err = PGPNewSingletonKeySet( key, &single );
    if( IsPGPError(err) )
        goto out;

    err = PGPOrderKeySet( single, kPGPAnyOrdering, &keylist );
    if( IsPGPError(err) )
        goto out;

    err = PGPNewKeyIter( keylist, &iter );
    if( IsPGPError(err) )
        goto out;

    err = PGPKeyIterRewind( iter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( iter, &key);
    pgpAssertNoErr(err);
    err = PGPKeyIterRewindUserID( iter );
    pgpAssertNoErr(err);

    /* I hate going through all this just to match a
       simple userid, don't you? */

    err = PGPKeyIterNextUserID( iter, userID );

    while( *userID ) {
        char aliasstr[ kPGPMaxUserIDSize ];
        PGPInt32 actual;

        /* find whether it's a substring..*/

        err = PGPGetUserIDStringBuffer( *userID,
                kPGPUserIDPropName, kPGPMaxUserIDSize,
                aliasstr, &actual );
        pgpAssertNoErr(err);

        if( substr( (char *)useridstr, aliasstr ) )
            goto out; /* success */

        err = PGPKeyIterNextUserID( iter, userID );
    }

out:
    if(keylist)
        PGPFreeKeyList( keylist );
    if(single)
        PGPFreeKeySet( single );
    return err;
}

/*
  Certify the key for user keyguffin in keyfile, using
  signature from user sigguffin.
 */

int pgpSignKey(struct pgpmainBones *mainbPtr, PGPKeyRef sigkey, PGPKeyRef
        key, const char *useridstr )
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPError err;
    PGPInt32 pri;
    PGPBoolean needsfree = FALSE;
    char* passphrase = 0;
    PGPUserIDRef userID;
    int batchmode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err );
    char *regExp = NULL;
    PGPBoolean compatible = envbPtr->compatible;
    char regBuf[ 256 ];

    pgpShowKeyBrief( filebPtr, key );
    pgpShowKeyFingerprint( filebPtr, key );

    if( !batchmode ) {
        fprintf( filebPtr->pgpout,LANG(
"\n\nREAD CAREFULLY:  Based on your own direct first-hand knowledge, "
"are\nyou absolutely certain that you are prepared to solemnly certify "
"that\nthe above public key actually belongs to the user specified by "
"the\nabove user ID (y/N)? "));

        if(!getyesno( filebPtr, 'n',0)) {
            return kPGPError_UserAbort;
        }
    }

    err = pgpGetValidPassphrase( mainbPtr, sigkey, &passphrase,&needsfree);
    if( IsPGPError(err) ) {
        return err;
    }

    if(!batchmode && !compatible) {
        /* Ask about regular expression to attach to the signature,
           sRegExp used by sCertifyUserID() */

        fprintf( filebPtr->pgpout,
LANG("\nAttach a regular expression to this signature, or\n"
"press enter for none: "));
        fflush( filebPtr->pgpout );
        pgpTtyGetString( regBuf, 255, filebPtr->pgpout );

        if( strlen(regBuf) > 0)
            regExp = regBuf;
    }

    if( useridstr )
        err = pgpGetKeyUserIDFromSubstring( key, useridstr, &userID );
    else
        err = PGPGetPrimaryUserID( key , &userID );
    pgpAssertNoErr( err );

    err = PGPSignUserID( userID, sigkey,
           PGPOExportable(context, TRUE),
           PGPOPassphrase(context,passphrase),
           (regExp) ? PGPOSigRegularExpression(context, regExp) :
           PGPONullOption( context ),

           PGPOLastOption(context) );

    if( IsPGPError(err) ) {
        if(!filebPtr->envbPtr->compatible)
           pgpShowError( filebPtr, err, __FILE__,__LINE__);
    }

    if( needsfree ) {
        err = PGPFreeData( passphrase );
        pgpRemoveFromPointerList( mainbPtr->leaks, passphrase );
        pgpAssertNoErr( err );
    }

    return err;
}


int signKeyInFileSpec(struct pgpmainBones *mainbPtr, char *useridstr,
        PGPFileSpecRef keyFileSpec)
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPError err;
    PGPInt32 pri;
    PGPBoolean verbose = pgpenvGetInt(filebPtr->envbPtr->m_env,
                               PGPENV_VERBOSE, &pri, &err );
    PGPKeyRef sigkey;
    PGPKeyRef key;
    PGPKeySetRef ringSet = NULL;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;

    err = pgpOpenKeyringsFromPubringSpec( mainbPtr, keyFileSpec, &ringSet, kPGPKeyRingOpenFlags_Mutable );

    if( IsPGPError(err) )
    {
        if ((!filebPtr->envbPtr->compatible) && verbose)
            pgpShowError(filebPtr, err, __FILE__,__LINE__);
        return  -1;
    }

    mainbPtr->workingRingSet=ringSet;
    /*mainbPtr->workingGroupSet=NULL;*/
    err = pgpGetMySigningKey( mainbPtr, &sigkey );
    if( IsPGPError(err) ) {
        pgpShowError( filebPtr, err, __FILE__,__LINE__);
        /*fprintf(filebPtr->pgpout, LANG("Key matching userid '%s'
        not found in file '%s'.\n"),myName,secring); */
        goto done;
    }

    /* get all matches */

    err = pgpGetMatchingKeyList( mainbPtr, useridstr, kMatch_NotDisabled,
            &keylist);
    pgpAssertNoErr(err);
    err = PGPNewKeyIter(keylist,&keyiter);
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( keyiter,&key);
    if(err)
        goto done;

    /* for all matches, ask for signing */

    while(key != NULL ) {
        err = pgpSignKey( mainbPtr, sigkey, key, useridstr );
        switch( err ) {
            case kPGPError_UserAbort:
            case kPGPError_NoErr:
                break;
            case kPGPError_BadPassphrase:
            case kPGPError_MissingPassphrase:
                goto done;
                break;
            default:
                if(!filebPtr->envbPtr->compatible)
                    pgpShowError(filebPtr,err,__FILE__,__LINE__);

                goto done;
        }
        err = PGPKeyIterNext( keyiter, &key );
    }
    if(err == kPGPError_EndOfIteration)
        err = kPGPError_NoErr;

    if( IsntPGPError(err) && PGPKeySetNeedsCommit( ringSet ) ) {
        err = PGPCommitKeyRingChanges( ringSet );
        pgpAssertNoErr(err);
    }
done:
    if(keyiter)
        PGPFreeKeyIter( keyiter );
    if(keylist)
    PGPFreeKeyList( keylist );
    PGPFreeKeySet( ringSet );
    mainbPtr->workingRingSet = NULL;
    return err;
}






