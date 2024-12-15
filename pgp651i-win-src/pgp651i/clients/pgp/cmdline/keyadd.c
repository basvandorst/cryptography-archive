/*____________________________________________________________________________
    keymgmt.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    $Id: keyadd.c,v 1.11 1999/05/12 21:01:03 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpEnv.h"

#include "usuals.h"
#include "pgp.h"
#include "globals.h"
#include "prototypes.h"

#include "language.h"

static int pgpGetKeyTrust( PGPKeyRef key ) {
    PGPUInt32 propData;
    PGPError err;
    err = PGPGetKeyNumber( key, kPGPKeyPropTrust, &propData );
    pgpAssertNoErr( err );
    return propData;
}

static int pgpGetKeyValidity( PGPKeyRef key ) {
    PGPUInt32 propData;
    PGPError err;
    err = PGPGetKeyNumber( key, kPGPKeyPropValidity, &propData );
    pgpAssertNoErr( err );
    return propData;
}

static PGPError askToSign( struct pgpmainBones *mainbPtr, PGPKeyRef key,
        PGPBoolean firsttimeaskfirst )
{

    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
/*
    - seems to get your public key
    - then it opens the ringfile i guess for the target key
    - and if key is compromised then it returns 0
    - and on a file i/o error it returns -1
    - and if not a key then it returns zero
    - and it scans till it reaches the userid
    - or fails
    - and then it reads the trust level
    - and if the trust is fine then it returns 0
    - and if asking is enabled it asks you generally speaking
    - and then it shows the key
    - and then it asks you specifically if you want to certify
    - and then it does another maint update pass
*/
    PGPError err;
    PGPInt32 pri;
    PGPEnv *env = pgpContextGetEnvironment( context );
    PGPBoolean batchmode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err );
    PGPBoolean issec;

    pgpAssertAddrValid( mainbPtr, struct pgpmainBones );
    pgpAssertAddrValid( mainbPtr->workingRingSet, PGPKeySetRef );

    if( pgpGetKeyValidity( key ) > kPGPValidity_Unknown ) {
        return kPGPError_ItemAlreadyExists;
    }
    PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &issec );
    if(issec)
        return kPGPError_ItemAlreadyExists;

    if( firsttimeaskfirst == TRUE ) {
        fprintf(filebPtr->pgpout,LANG(
             "\nOne or more of the new keys are not fully certified.\n"
             "Do you want to certify any of these keys yourself (y/N)? "));
        if(!getyesno(filebPtr,'n',batchmode )) {
           return kPGPError_UserAbort;
        }
    }

    pgpShowKeyBrief( filebPtr, key );

    fprintf(filebPtr->pgpout, LANG(
"\nDo you want to certify this key yourself (y/N)? "));

    if( getyesno(filebPtr,'n',batchmode )) {
        PGPKeyRef sigkey;
        /* mainbPtr->workingRingSet=ringSet;*/
        /* mainbPtr->workingGroupSet=NULL;*/
        err = pgpGetMySigningKey( mainbPtr, &sigkey );
        if( err ) {
            return err;
        }

        if( pgpSignKey( mainbPtr, sigkey, key, NULL ) ) {
           PGPPropagateTrust( mainbPtr->workingRingSet );
        }
    }

    return kPGPError_NoErr;
}

PGPError addToWorkingRingSetFinish( struct pgpmainBones *mainbPtr ) {
    PGPKeySetRef ringSet = mainbPtr->workingRingSet;
    PGPError err;

    if(PGPKeySetNeedsCommit( ringSet ))
        err = PGPCommitKeyRingChanges( ringSet );
    return err;
}

PGPError pgpAddKeyToKeySet( PGPKeyRef key, PGPKeySetRef keySet)
{
    PGPError err,er2;
    PGPKeySetRef tmpset;
    err = PGPNewSingletonKeySet( key, &tmpset );
    if( IsPGPError(err))
        return err;
    err = PGPAddKeys( tmpset, keySet );

    er2 = PGPFreeKeySet( tmpset );
    pgpAssertNoErr(er2);

    return err;
}

enum {
    kMark_Undefined = 0,
    kMark_Ordinary = 1,
    kMark_Revocation = 2,
    kMark_Secret = 4,
    kMark_Trust = 8
};

PGPError pgpMarkKey( PGPKeyIterRef iterContext, PGPKeyRef key )
{
    PGPError err;
    PGPUserIDRef alias;
    PGPBoolean iskrev,issec;

    err = PGPGetKeyBoolean( key, kPGPKeyPropIsRevoked, &iskrev );
    pgpAssertNoErr(err);

    err = PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &issec );
    pgpAssertNoErr(err);

    err = PGPSetKeyUserVal( key, (PGPUserValue) (( iskrev ?
            kMark_Revocation :
            kMark_Ordinary )|( issec ? kMark_Secret : 0)) );

    pgpAssertNoErr(err);

    err = PGPKeyIterRewindUserID( iterContext );
    pgpAssertNoErr(err);

    err = PGPKeyIterNextUserID( iterContext, &alias);
    pgpAssertNoErr(err);

    while( alias ) {
        PGPSigRef cert;

        err = PGPSetUserIDUserVal( alias, (PGPUserValue) kMark_Ordinary );
        pgpAssertNoErr(err);

        err = PGPKeyIterRewindUIDSig( iterContext );
        pgpAssertNoErr(err);
        err = PGPKeyIterNextUIDSig( iterContext, &cert );
        /*if err, there are none.*/
        while( cert ) {
            PGPBoolean isrev;

            err = PGPGetSigBoolean( cert, kPGPSigPropIsRevoked, &isrev );

            if( isrev )
                err = PGPSetSigUserVal( cert, (PGPUserValue)
                        kMark_Revocation );
            else
                err = PGPSetSigUserVal( cert, (PGPUserValue)
                        kMark_Ordinary );
            pgpAssertNoErr(err);

            err = PGPKeyIterNextUIDSig( iterContext, &cert );
            /*if err, there are no more.*/
        }
        err = PGPKeyIterNextUserID( iterContext, &alias );
    }
    if(err == kPGPError_EndOfIteration )
        err = kPGPError_NoErr;
    return err;
}


PGPError pgpMarkKeyInSet( PGPKeySetRef keySet, PGPKeyRef srchkey )
{
    PGPError err,er2;
    PGPKeyRef key;
    PGPKeyListRef keylist;
    PGPKeyIterRef iter;

    err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &iter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( iter );
    pgpAssertNoErr(err);

    PGPKeyIterNext( iter, &key);
    /*if err, there are none.*/
    while( key )
    {
        if( PGPCompareKeys( key, srchkey, kPGPKeyIDOrdering ) == 0)
        {
            err = pgpMarkKey( iter, key );
            if(err)
                break;
        }
        PGPKeyIterNext( iter, &key);
        /*if err, there are no more.*/
    }

    er2 = PGPFreeKeyIter( iter );
    pgpAssertNoErr(err);
    er2 = PGPFreeKeyList( keylist );
    pgpAssertNoErr(err);

    return err;
}


 /*
    mark an existing keySet before merging in a new keySet, so
    we can determine new userids, sigs / revocations
  */
PGPError pgpMarkKeySet( PGPKeySetRef keySet )
{
    PGPError err,er2;
    PGPKeyRef key;
    PGPKeyListRef keylist;
    PGPKeyIterRef iter;

    err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &iter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( iter );
    pgpAssertNoErr(err);

    PGPKeyIterNext( iter, &key);
    /*if err, there are none.*/
    while( key )
    {
        err = pgpMarkKey( iter, key );
        if(err)
            break;
        PGPKeyIterNext( iter, &key);
        /*if err, there are no more.*/
    }

    er2 = PGPFreeKeyIter( iter );
    pgpAssertNoErr(err);
    er2 = PGPFreeKeyList( keylist );
    pgpAssertNoErr(err);

    return err;
}

 /*
    Show the keys, userids, sigs that have not been marked by pgpMarkKey().

    If interactive mode, ask the user whether to add each new userid.
    Note, the key passed to this function should not be associated with
    any persistent key database or undesired behaviour may result.
  */
PGPError pgpShowKeyUnmarked( struct pgpfileBones *filebPtr,
        PGPKeyIterRef iterContext, PGPKeyRef key,
        PGPSize *newKeyCount, PGPSize *newUIDCount,
        PGPSize *newSigCount, PGPSize *newRevokeCount )
{
    PGPError err;
    PGPUserIDRef alias;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPBoolean batchmode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err );
    PGPBoolean verbose = pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err );
    PGPUserValue val;
    PGPBoolean iskrev,issec;
    PGPBoolean compatible = envbPtr->compatible;
    PGPInt32 interactive = envbPtr->interactiveAdd;

    if(newKeyCount)
        *newKeyCount = 0;
    if(newUIDCount)
        *newUIDCount = 0;
    if(newSigCount)
        *newSigCount = 0;
    if(newRevokeCount)
        *newRevokeCount = 0;

    err = PGPGetKeyBoolean( key, kPGPKeyPropIsRevoked, &iskrev );
    pgpAssertNoErr(err);

    err = PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &issec );
    pgpAssertNoErr(err);

    err = PGPGetKeyUserVal( key, &val );
    if( iskrev ) {
        if( !( (PGPInt32)val & kMark_Revocation ) ) {
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("\nNew revocation\n") );
            if(newRevokeCount)
                (*newRevokeCount)++;
        }
    } else {
        if( !( (PGPInt32)val & kMark_Ordinary ) ) {
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("\nnew key\n") );

            /* new key. return early.*/
            if(newKeyCount)
                (*newKeyCount)++;
            /*return kPGPError_NoErr;*/
        }
    }

    if( issec ) {
        if( !( (PGPInt32)val & kMark_Secret ) ) {
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("\nNew secret key\n") );
            if(newKeyCount)
                (*newKeyCount)++;
        }
    }

    err = PGPKeyIterRewindUserID( iterContext );
    pgpAssertNoErr(err);

    err = PGPKeyIterNextUserID( iterContext, &alias);

    while( alias ) {
        PGPSize actual;
        char useridstr[ kPGPMaxUserIDSize ];

        err = PGPGetUserIDStringBuffer( alias, kPGPUserIDPropName,
                kPGPMaxUserIDSize, useridstr, &actual );

        pgpAssertNoErr(err);

        err = PGPGetUserIDUserVal( alias, &val);
        pgpAssertNoErr(err);
        if( val != (PGPUserValue) kMark_Ordinary ) {
            PGPBoolean ans;

            fprintf( filebPtr->pgpout, LANG("\nNew userid: \"%s\".\n"),
                    useridstr );

            if( interactive ) {
                fprintf( filebPtr->pgpout,
                        LANG("\nWill be added to the following key:\n"));

                pgpShowKeyBrief( filebPtr, key );
                fprintf( filebPtr->pgpout, LANG("Add this userid (y/N)? "));
                fflush( filebPtr->pgpout );
                ans = getyesno( filebPtr, 'n', batchmode );
            }
            if( interactive && !ans ) {
                err = PGPRemoveUserID( alias );
                if( err == kPGPError_BadParams )
                    err = kPGPError_NoErr;
                pgpAssertNoErr(err);
                goto next;
            } else
                if(newUIDCount)
                    (*newUIDCount)++;
        }
        {
            PGPSigRef cert;

            err = PGPKeyIterRewindUIDSig( iterContext );
            pgpAssertNoErr(err);
            err = PGPKeyIterNextUIDSig( iterContext, &cert );
            /*if err, there are none.*/
            while( cert ) {
                PGPKeyID kid;
                char kstr[kPGPMaxKeyIDStringSize];
                PGPBoolean isrev;

                err = PGPGetKeyIDOfCertifier( cert, &kid );
                pgpAssertNoErr(err);
                err = pgpGetKeyIDStringCompat( &kid, TRUE, compatible,
                        kstr );
                pgpAssertNoErr(err);

                err = PGPGetSigBoolean( cert, kPGPSigPropIsRevoked, &isrev );
                pgpAssertNoErr(err);

                if( isrev ) {
                    err = PGPGetSigUserVal( cert, &val );
                    if( val != (PGPUserValue) kMark_Revocation ) {
                        fprintf(  filebPtr->pgpout, LANG(
"New revocation from keyID %s on userid %s\n"), kstr, useridstr);
                        if(newRevokeCount)
                            (*newRevokeCount)++;
                    }
                } else {
                    err = PGPGetSigUserVal( cert, &val );
                    if( val != (PGPUserValue) kMark_Ordinary ) {
                        fprintf(  filebPtr->pgpout, LANG(
"New signature from keyID %s on userid %s\n"), kstr, useridstr);
                        if(newSigCount)
                            (*newSigCount)++;
                    }
                }
                err = PGPKeyIterNextUIDSig( iterContext, &cert );
                /*if err, there are no more.*/
            }
        }
next:
        err = PGPKeyIterNextUserID( iterContext, &alias);
    }
    if(err == kPGPError_EndOfIteration)
        err = kPGPError_NoErr;
    return err;
}

PGPError pgpShowKeyMarked( struct pgpfileBones *filebPtr, PGPKeyIterRef
        iterContext, PGPKeyRef key, PGPSize *newKeyCount, PGPSize
        *newUIDCount, PGPSize *newSigCount, PGPSize *newRevokeCount )
{
    PGPError err;
    PGPUserIDRef alias;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPBoolean batchmode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err );
    PGPBoolean verbose = pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err );
    PGPUserValue val;
    PGPBoolean iskrev,issec;
    PGPBoolean compatible = envbPtr->compatible;
    PGPInt32 interactive = envbPtr->interactiveAdd;

    if(newKeyCount)
        *newKeyCount = 0;
    if(newUIDCount)
        *newUIDCount = 0;
    if(newSigCount)
        *newSigCount = 0;
    if(newRevokeCount)
        *newRevokeCount = 0;

    err = PGPGetKeyBoolean( key, kPGPKeyPropIsRevoked, &iskrev );
    pgpAssertNoErr(err);

    err = PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &issec );
    pgpAssertNoErr(err);

    err = PGPGetKeyUserVal( key, &val );
    if( iskrev ) {
        if( ( (PGPInt32)val & kMark_Revocation ) ) {
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("\nNew revocation\n") );
            if(newRevokeCount)
                (*newRevokeCount)++;
        }
    } else {
        if( ( (PGPInt32)val & kMark_Ordinary ) ) {
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("\nnew key\n") );

            /* new key. return early.*/
            if(newKeyCount)
                (*newKeyCount)++;
            /*return kPGPError_NoErr;*/
        }
    }

    if( issec ) {
        if( ( (PGPInt32)val & kMark_Secret ) ) {
            if(verbose)
                fprintf( filebPtr->pgpout, LANG("\nNew secret key\n") );
            if(newKeyCount)
                (*newKeyCount)++;
        }
    }

    err = PGPKeyIterRewindUserID( iterContext );
    pgpAssertNoErr(err);

    err = PGPKeyIterNextUserID( iterContext, &alias);

    while( alias ) {
        PGPSize actual;
        char useridstr[ kPGPMaxUserIDSize ];

        err = PGPGetUserIDStringBuffer( alias, kPGPUserIDPropName,
                kPGPMaxUserIDSize, useridstr, &actual );

        pgpAssertNoErr(err);

        err = PGPGetUserIDUserVal( alias, &val);
        pgpAssertNoErr(err);
        if( val != (PGPUserValue) kMark_Ordinary ) {
            PGPBoolean ans;

            if( interactive ) {
                fprintf( filebPtr->pgpout, LANG("\nNew userid: \"%s\".\n"),
                        useridstr);
                fprintf( filebPtr->pgpout,
                        LANG("\nWill be added to the following key:\n"));
                pgpShowKeyBrief( filebPtr, key );
                fprintf( filebPtr->pgpout, LANG("Add this userid (y/N)? "));
                fflush( filebPtr->pgpout );
                ans = getyesno( filebPtr, 'n', batchmode );
            }
            if( interactive && !ans ) {
                err = PGPRemoveUserID( alias );
                pgpAssertNoErr(err);
            } else
                if(newUIDCount)
                    (*newUIDCount)++;
        } else {
            PGPSigRef cert;

            err = PGPKeyIterRewindUIDSig( iterContext );
            pgpAssertNoErr(err);
            err = PGPKeyIterNextUIDSig( iterContext, &cert );
            /*if err, there are none.*/
            while( cert ) {
                PGPKeyID kid;
                char kstr[kPGPMaxKeyIDStringSize];
                PGPBoolean isrev;

                err = PGPGetKeyIDOfCertifier( cert, &kid );
                pgpAssertNoErr(err);
                err = pgpGetKeyIDStringCompat( &kid, TRUE, compatible,
                        kstr );
                pgpAssertNoErr(err);

                err = PGPGetSigBoolean( cert, kPGPSigPropIsRevoked, &isrev );
                pgpAssertNoErr(err);

                if( isrev ) {
                    err = PGPGetSigUserVal( cert, &val );
                    if( val == (PGPUserValue) kMark_Revocation ) {
                        fprintf(  filebPtr->pgpout, LANG(
"New revocation from keyID %s on userid %s\n"), kstr, useridstr);
                        if(newRevokeCount)
                            (*newRevokeCount)++;
                    }
                } else {
                    err = PGPGetSigUserVal( cert, &val );
                    if( val == (PGPUserValue) kMark_Ordinary ) {
                        fprintf(  filebPtr->pgpout, LANG(
"New signature from keyID %s on userid %s\n"), kstr, useridstr);
                        if(newSigCount)
                            (*newSigCount)++;
                    }
                }
                err = PGPKeyIterNextUIDSig( iterContext, &cert );
                /*if err, there are no more.*/
            }
        }
        err = PGPKeyIterNextUserID( iterContext, &alias);
    }
    if(err == kPGPError_EndOfIteration)
        err = kPGPError_NoErr;
    return err;
}

PGPError pgpShowKeySetUnmarked( struct pgpfileBones *filebPtr,
        PGPKeySetRef keySet, PGPSize *newKeyCount, PGPSize *newUIDCount,
        PGPSize *newSigCount, PGPSize *newRevokeCount )
{
    PGPError err,er2;
    PGPKeyRef key;
    PGPKeyListRef keylist;
    PGPKeyIterRef iter;
    *newKeyCount = 0;
    *newUIDCount = 0;
    *newSigCount = 0;
    *newRevokeCount = 0;

    err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &iter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( iter );
    pgpAssertNoErr(err);

    PGPKeyIterNext( iter, &key);
    /*if err, there are none.*/
    while( key )
    {
        PGPSize keyc, uidc, sigc, rvkc;
        err = pgpShowKeyUnmarked( filebPtr, iter, key, &keyc, &uidc,
                &sigc, &rvkc );
        (*newKeyCount) += uidc;
        (*newUIDCount) += uidc;
        (*newSigCount) += sigc;
        (*newRevokeCount) += rvkc;
        if(err)
            break;
        PGPKeyIterNext( iter, &key);
        /*if err, there are no more.*/
    }

    er2 = PGPFreeKeyIter( iter );
    pgpAssertNoErr(err);
    er2 = PGPFreeKeyList( keylist );
    pgpAssertNoErr(err);

    return err;
}

PGPError pgpShowKeySetMarked( struct pgpfileBones *filebPtr, PGPKeySetRef
        keySet, PGPSize *newKeyCount, PGPSize *newUIDCount, PGPSize
        *newSigCount, PGPSize *newRevokeCount )
{
    PGPError err;
    PGPKeyRef key;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    *newKeyCount = 0;
    *newUIDCount = 0;
    *newSigCount = 0;
    *newRevokeCount = 0;

    err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);

    PGPKeyIterNext( keyiter, &key);
    /*if err, there are none.*/
    while( key )
    {
        PGPSize keyc, uidc, sigc, rvkc;
        err = pgpShowKeyMarked( filebPtr, keyiter, key, &keyc, &uidc,
                &sigc, &rvkc );
        (*newKeyCount) += uidc;
        (*newUIDCount) += uidc;
        (*newSigCount) += sigc;
        (*newRevokeCount) += rvkc;
        if(err)
            break;
        PGPKeyIterNext( keyiter, &key);
        /*if err, there are no more.*/
    }

    if( keyiter )
        PGPFreeKeyIter( keyiter );
    if( keylist )
        PGPFreeKeyList( keylist );
    pgpAssertNoErr(err);

    return err;
}

/*
   Examines the keys in keyset and updates the corresponding certs in
   ringSet that are uncertified.
   It is not assumed that parameter keyset comes from the same key
   database as ringSet.
 */

PGPError pgpProcessUncertifiedKeys( struct pgpmainBones *mainbPtr,
        PGPKeySetRef keyset )
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPKeyRef key,origkey;
    PGPKeyID keyid;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyListRef keylist = NULL;
    PGPError err;
    PGPBoolean ask_first_latch = TRUE;

    pgpAssertAddrValid( mainbPtr, struct pgpmainBones );
    pgpAssertAddrValid( mainbPtr->workingRingSet, PGPKeySetRef );

    err = PGPOrderKeySet( keyset, kPGPAnyOrdering, &keylist);
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);

    /* Sign unverifiable keys by hand */
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( keyiter, &key);
    pgpAssertNoErr(err);
    while( key ) {
        PGPBoolean issec;

        PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &issec );
        if(issec)
            goto next;

        err = PGPGetKeyIDFromKey( key, &keyid );
        if( IsPGPError(err) )  {
            pgpShowError( filebPtr, err,__FILE__,__LINE__ );
            goto next;
        }
        err = PGPGetKeyByKeyID( mainbPtr->workingRingSet, &keyid,
                kPGPPublicKeyAlgorithm_Invalid, &origkey );
        if( IsPGPError(err) ) {
            if( err != kPGPError_ItemNotFound )
                pgpShowError( filebPtr, err,__FILE__,__LINE__ );
            goto next;
        }
        /*mainbPtr->workingRingSet=ringSet;*/
        /*mainbPtr->workingGroupSet=NULL;*/
        err=askToSign( mainbPtr, origkey, ask_first_latch );
        switch(err) {
            case kPGPError_NoErr:
                break;
            case kPGPError_UserAbort:
                /*err = kPGPError_NoErr;*/
                goto done;
            case kPGPError_ItemAlreadyExists:
                goto next;
            default:
                pgpShowError( filebPtr, err,__FILE__,__LINE__ );
                goto done;
        }
        ask_first_latch = FALSE; /* should stay false now */
next:
        err = PGPKeyIterNext( keyiter, &key );
        /*if err, there are no more*/
    }
    if( err == kPGPError_EndOfIteration )
        err = kPGPError_NoErr;
done:
    if( keyiter )
        PGPFreeKeyIter( keyiter );
    if( keylist )
        PGPFreeKeyList( keylist );
    return err;
}

/*
   Examines the keys in keyset and updates the trust of corresponding
   keys in ringSet.

   It is not assumed that parameter keyset comes from the same key
   database as ringSet.
 */
PGPError pgpProcessUntrustedKeys( struct pgpmainBones *mainbPtr,
        PGPKeySetRef keyset, PGPKeySetRef ringSet )
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPKeyRef key,origkey;
    PGPKeyID keyid;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyListRef keylist = NULL;
    PGPError err;

    err = PGPOrderKeySet( keyset, kPGPAnyOrdering, &keylist);
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);

    for(;;) {
        PGPBoolean endloop;

        /* loop while there are any (at least marginally valid) keys
           whose trust has not been set. */

        /* while keys still with undefined trust, allow wetware editing
           of trust parameter (avoid iterating main ringSet due to ldap
           servers and potential size of set) */

        endloop = TRUE;
        err = PGPKeyIterRewind( keyiter );
        pgpAssertNoErr(err);
        err = PGPKeyIterNext( keyiter, &key);
        while( key ) {
            PGPInt32 validity;
            PGPUserValue val;

            err = PGPGetKeyIDFromKey( key, &keyid );
            if( err )  {
                pgpShowError( filebPtr, err,__FILE__,__LINE__ );
                goto next;
            }

            err = PGPGetKeyByKeyID( ringSet, &keyid,
                    kPGPPublicKeyAlgorithm_Invalid, &origkey );

            if( err ) {
                pgpShowError( filebPtr, err,__FILE__,__LINE__ );
                goto next;
            }

            err = PGPGetKeyNumber( key, kPGPKeyPropValidity, &validity);
            pgpAssertNoErr(err);

            err = PGPGetKeyUserVal( key, &val );
            pgpAssertNoErr(err);

            if( (validity > kPGPValidity_Invalid) && ! ((PGPInt32)val &
                    kMark_Trust ) ) /* key has not been processed*/

            {
                PGPUserIDRef userid;
                PGPSize scrap;
                char useridstr[ kPGPMaxUserIDSize ];
                err = PGPGetPrimaryUserID( origkey, &userid );
                pgpAssertNoErr(err);
                err = PGPGetUserIDStringBuffer( userid,
                       kPGPUserIDPropName, kPGPMaxUserIDSize, useridstr,
                       &scrap);

                pgpAssertNoErr(err);
                if( pgpGetKeyTrust( origkey ) <= kPGPKeyTrust_Unknown ) {
                    err = pgpShowKeyBrief( filebPtr, key );
                    pgpAssertNoErr(err);
                    err = pgpEditPublicTrustParameter( filebPtr,
                            useridstr, key );

                    if( err ) {
                        pgpShowError( filebPtr, err,__FILE__,__LINE__ );
                        goto done;
                    }
                    PGPPropagateTrust( ringSet );
                    PGPSetKeyUserVal( key, (PGPUserValue)( (PGPInt32)val |
                            kMark_Trust )); /* key has been processed*/

                    endloop = FALSE;
                }
            }
next:
            err = PGPKeyIterNext( keyiter, &key );
            /*if err, no more*/
        }

        if( endloop == TRUE ) break;
    }
done:
    if( keyiter )
        PGPFreeKeyIter( keyiter );
    if( keylist )
        PGPFreeKeyList( keylist );
    return err;
}

PGPError addToWorkingRingSet(struct pgpmainBones *mainbPtr, PGPKeySetRef
        keySet )
{
    PGPContextRef context = mainbPtr->pgpContext;
    PGPEnv *env = pgpContextGetEnvironment( context );
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPKeyIterRef iter = NULL;
    PGPError err;
    PGPKeyRef key = 0;
    PGPKeyRef origkey = 0;
    PGPKeyID keyid;
    int copying = 0;
    int newkeys = 0, newsigs = 0, newuids = 0, newrvks = 0;
    PGPKeyListRef keylist = 0;
    char* ringfile = "";
    PGPInt32 pri;
    PGPInt32 verbose = pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err );
    PGPInt32 interactive = envbPtr->interactiveAdd;
    PGPBoolean batchmode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err );
    PGPKeySetRef keepset = NULL, origset = NULL;
    PGPBoolean ans;

    pgpAssertAddrValid( mainbPtr, struct pgpmainBones );
    pgpAssertAddrValid( mainbPtr->workingRingSet, PGPKeySetRef );

    err = PGPNewEmptyKeySet( mainbPtr->workingRingSet, &origset );
    pgpAssertNoErr(err);

    err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);

    err = PGPNewKeyIter( keylist, &iter );
    pgpAssertNoErr(err);

    if(verbose)
        fprintf(filebPtr->pgpout,
                LANG("\nPASS 1.. find the existing keys\n"));

    err = PGPKeyIterRewind( iter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( iter, &key);
    if( err ) {
        fprintf( filebPtr->pgpout,LANG("Could not read key from keyfile."));
        goto done;
    }
    while( key ) {
        err = PGPGetKeyIDFromKey( key, &keyid );
        pgpAssertNoErr(err);
        err = PGPGetKeyByKeyID( mainbPtr->workingRingSet, &keyid,
                kPGPPublicKeyAlgorithm_Invalid,
                &origkey );
        if(err == 0 )
            err = pgpAddKeyToKeySet( origkey, origset );
        err = PGPKeyIterNext( iter, &key );
    }

    if(verbose)
        fprintf(filebPtr->pgpout,
                LANG("\nPASS 2.. ask whether to add the new keys\n"));

    err = PGPKeyIterRewind( iter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNext( iter, &key);
    if( err ) {
        fprintf( filebPtr->pgpout,LANG("Could not read key from keyfile."));
        goto done;
    }

    while( key ) {
        PGPBoolean isnew;
        err = PGPGetKeyIDFromKey( key, &keyid );
        pgpAssertNoErr(err);
        err = PGPGetKeyByKeyID( mainbPtr->workingRingSet, &keyid,
                kPGPPublicKeyAlgorithm_Invalid,
                &origkey );

        /* Copy if all criteria are met */
        copying = 0;
        isnew = FALSE;
        if(err == 0 ) {
                copying = 1;
        } else {
            /* is a new key*/
            err = pgpShowKeyListFormat( filebPtr,
                    mainbPtr->workingRingSet, iter, key, kShow_Sigs |
                    kShow_Checks );

            if( interactive ) {
                fprintf( filebPtr->pgpout, LANG(
"\nDo you want to add this key to keyring '%s' (y/N)? "), ringfile);

                copying = getyesno(filebPtr,'n', batchmode ) ? 1 : 0;
            } else
                copying = 1;
            if(copying) {
                isnew=TRUE;
                newkeys++;
            }
        }
        if( copying ) {
            if(keepset == NULL) {
                err = PGPNewKeySet( context, &keepset );
                pgpAssertNoErr(err);
            }
            err = pgpAddKeyToKeySet( key, keepset );
            if(IsPGPError(err)) {
                pgpShowError(filebPtr, err,__FILE__,__LINE__);
            }
            if( isnew && !batchmode ) {
                /* This is not efficient, because it loops on
                   keepset, so avoid calling it in batchmode. */
                err = pgpMarkKeyInSet( keepset, key );
            }
        }
        err = PGPKeyIterNext( iter, &key );
        if( err != 0 || key == 0 ) {
            break;
        }
    }
    if(err == kPGPError_EndOfIteration)
        err = 0;


    if( newkeys > 0 && !interactive ) {
        fprintf( filebPtr->pgpout, LANG("\nkeyfile contains %d new keys. "),
                newkeys);
        fprintf( filebPtr->pgpout, LANG(
"Add these keys to keyring '%s'? (Y/n) "), ringfile );
        fflush( filebPtr->pgpout );
        ans = getyesno(filebPtr, 'y', batchmode);
        if(!ans)
           goto done;
    }

    if(verbose)
        fprintf(filebPtr->pgpout, LANG(
"\nPASS 3.. show the new userids and new sigs\n"));
    {
        PGPSize keys,uids,sigs,rvks;

        err = pgpMarkKeySet( origset );
        pgpAssertNoErr(err);

        err = PGPAddKeys( origset, keepset );
        pgpAssertNoErr(err);

        err = pgpShowKeySetUnmarked( filebPtr, keepset, &keys, &uids,
                &sigs, &rvks );

        pgpAssertNoErr(err);
        newkeys += keys;
        newuids += uids;
        newsigs += sigs;
        newrvks += rvks;
    }

    if(verbose)
        fprintf(filebPtr->pgpout,
                LANG("\nPASS 4... add them to the keyring\n"));

    err = PGPAddKeys( keepset, mainbPtr->workingRingSet );
    pgpAssertNoErr(err);

    /* For now AddKeys should be more or less the same as mergekeys */

    /* User feedback */

    if( newsigs == 0 && newkeys == 0 && newuids == 0 && newrvks == 0 ) {
        fprintf( filebPtr->pgpout,
                LANG("No new keys or signatures in keyfile.\n" ) );
        goto done;
    }

    /* Check signatures */
    PGPCheckKeyRingSigs( keepset, mainbPtr->workingRingSet, 0, NULL, NULL );

    if( err ) {
        if( verbose ) {
            fprintf(filebPtr->pgpout, LANG(
"addToWorkingRingSet: pgpDoCheckKeySet() returned %d\n"), err);
        }
        goto done;
    }

    /* More user feedback */

    fprintf(filebPtr->pgpout, LANG("\nKeyfile contains:\n"));
    if (newkeys)
        fprintf(filebPtr->pgpout, LANG("%4d new key(s)\n"), newkeys);
    if (newsigs)
        fprintf(filebPtr->pgpout, LANG("%4d new signatures(s)\n"), newsigs);
    if (newuids)
        fprintf(filebPtr->pgpout, LANG("%4d new user ID(s)\n"), newuids);
    if (newrvks)
        fprintf(filebPtr->pgpout, LANG("%4d new revocation(s)\n"), newrvks);

    err = PGPPropagateTrust( mainbPtr->workingRingSet );
    pgpAssertNoErr(err);

    /* mainbPtr->workingRingSet = ringSet;*/
    /* mainbPtr->workingGroupSet = NULL;*/
    err = pgpProcessUncertifiedKeys( mainbPtr, keepset );
    if( IsPGPError(err)) {
        if(err == kPGPError_UserAbort)
            err = kPGPError_NoErr;
        goto done;
    }

    err = pgpProcessUntrustedKeys( mainbPtr, keepset,
            mainbPtr->workingRingSet );

done:
    if(keepset != NULL)
        PGPFreeKeySet( keepset );
    if( iter )
        PGPFreeKeyIter( iter );
    if( keylist)
        PGPFreeKeyList( keylist );
    if( origset )
        PGPFreeKeySet( origset );

    if(PGPKeySetNeedsCommit(mainbPtr->workingRingSet))
        PGPCommitKeyRingChanges(mainbPtr->workingRingSet);

    /* The event handler will call addToRingSetFinish( ringSet ); before
       this is done */

    return err;
}


int addToKeyring(struct pgpmainBones *mainbPtr, char* keyfile,
PGPFileSpecRef ringFileSpec, char *origRingFileName )

{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;

    PGPFileSpecRef  keyFileSpec = kPGPInvalidRef;
    PGPKeySetRef    ringSet = kPGPInvalidRef;
    PGPKeySetRef    keySet = kPGPInvalidRef;
    PGPKeySetRef    emptyKeySet = kPGPInvalidRef;
    PGPError err,er2;
    char *ringfile = NULL;
    PGPEnv *env = mainbPtr->envbPtr->m_env;
    PGPInt32 pri;
    PGPBoolean compatible = mainbPtr->envbPtr->compatible;


    err = PGPGetFullPathFromFileSpec( ringFileSpec, &ringfile );
    pgpAssertNoErr(err);

    /* Manually open the input file, gives us an early out if it is
       missing */

    err = PGPNewFileSpecFromFullPath(context, keyfile, &keyFileSpec);
    if(err) {
        pgpShowError( filebPtr, err, __FILE__,__LINE__);
        goto done;
    }

    /* add the keys to a in memory key set, then
       do one add operation at end to the target key set */
    err = PGPNewKeySet(context, &emptyKeySet);
    if(IsPGPError(err))
        goto done;
    mainbPtr->workingRingSet = emptyKeySet;

    /* Get or create target keySet. Must be both public and secret
       keyrings, in case the input contains new secret keys. */

    err = pgpOpenKeyringsFromPubringSpec( mainbPtr, ringFileSpec, &ringSet, 
					  kPGPKeyRingOpenFlags_Mutable);
    if(err) {
        /* XXX The error msg context here may differ from 262 */
        fprintf(filebPtr->pgpout, ringFileSpec ? LANG(
                "\nKey ring file '%s' cannot be created.\n")
                : LANG("Default ring file cannot be created"),
                origRingFileName);
        goto done;
    }

    /* Allow eventhandler in dodecode.c to do the remaining work */

    fprintf(filebPtr->pgpout,LANG("\nLooking for new keys...\n"));

    err = PGPDecode(
            context,
            PGPOInputFile( context, keyFileSpec ),
            PGPOKeySetRef( context, ringSet ),
            PGPOSendEventIfKeyFound( context, TRUE ),
            PGPOEventHandler( context, dechandler, (PGPUserValue) mainbPtr ),
            PGPOLastOption( context )
            );

    if(err) {
        pgpShowError( filebPtr, err,__FILE__,__LINE__ );
        goto done;
    }
    else
    {
        err = PGPAddKeys(mainbPtr->workingRingSet, ringSet);
        if(IsntPGPError(err) && PGPKeySetNeedsCommit(ringSet))
        {
            PGPCommitKeyRingChanges(ringSet);
        }
    }

#ifdef NOTPREFERREDMETHOD
    /* Rather than calling this here, we call it from the callback
       Copy all keys from keySet which aren't in ringSet already */
    err = addToWorkingRingSet( mainbPtr, keySet );
#endif

done:
    if(emptyKeySet != kPGPInvalidRef)
    {
        er2 = PGPFreeKeySet(emptyKeySet);
        pgpAssertNoErr(er2);
    }

    if( keySet != kPGPInvalidRef ) {
       er2 = PGPFreeKeySet( keySet );
       pgpAssertNoErr(er2);
    }

    if( ringSet != kPGPInvalidRef ) {
       er2 = PGPFreeKeySet( ringSet );
       pgpAssertNoErr(er2);
       mainbPtr->workingRingSet = NULL;
    }

    if( keyFileSpec != kPGPInvalidRef ) {
       er2 = PGPFreeFileSpec(keyFileSpec);
       pgpAssertNoErr(er2);
    }

    if(!compatible && IsPGPError(err) &&
      pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &er2 ) ) {
        pgpShowError(filebPtr, err,__FILE__,__LINE__);
    }
    if(ringfile)
        PGPFreeData(ringfile);

    return err ? -1 : 0;
}


