/*____________________________________________________________________________
    keyrevoke.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    use the PGP SDK to revoke and disable keys.

    $Id: keyrevoke.c,v 1.11 1999/05/12 21:01:04 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpKeyServer.h"

#include "usuals.h"
#include "pgp.h"
#include "globals.h"
#include "prototypes.h"
#include "language.h"

PGPError pgpUserChooseAndDisableKeys( struct pgpmainBones *mainbPtr,
        PGPKeySetRef keysToDisable )
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPError err;
    PGPKeyListRef keylist;
    PGPKeyIterRef keyiter;
    PGPKeyRef key;

    err = PGPOrderKeySet( keysToDisable, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);

    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( keyiter, &key );

    while( key != NULL ) {
        PGPBoolean propData = FALSE;
        pgpShowKeyBrief( filebPtr, key );

        err = PGPGetKeyBoolean( key, kPGPKeyPropIsDisabled, &propData );
        pgpAssertNoErr( err );

        if( propData ) {
            fprintf( filebPtr->pgpout,
                    LANG("\nKey is already disabled.\n"
                            "Do you want to enable this key again (y/N)? "));
            if( getyesno( filebPtr,'n',0) ) {
                if( mainbPtr->workingKeyServer ) {
                    PGPKeySetRef single,failed=NULL;
                    err = PGPNewSingletonKeySet( key, &single );
                    pgpAssertNoErr(err);

                    /* does it get re-enabled??*/
                    err = PGPDisableFromKeyServer(
                            mainbPtr->workingKeyServer, single, &failed );

                    if( IsPGPError(err) ) {
                        pgpShowError( filebPtr, err, 0,0);
                        if( failed )
                            PGPFreeKeySet(failed);
                    }
                    PGPFreeKeySet( single );
                } else {
                    err = PGPEnableKey( key );
                    pgpAssertNoErr( err );
                }
            }
        } else {
            fprintf( filebPtr->pgpout,
                    LANG("\nDisable this key (y/N)? "));
            if( getyesno( filebPtr,'n',0) ) {
                if( mainbPtr->workingKeyServer ) {
                    PGPKeySetRef single,failed=NULL;
                    err = PGPNewSingletonKeySet( key, &single );
                    pgpAssertNoErr(err);

                    err = PGPDisableFromKeyServer(
                            mainbPtr->workingKeyServer, single, &failed );

                    if( IsPGPError(err) ) {
                        pgpShowError( filebPtr, err, 0,0);
                        if( failed )
                            PGPFreeKeySet(failed);
                    }
                    PGPFreeKeySet( single );
                } else {
                    err = PGPDisableKey( key );
                    pgpAssertNoErr( err );
                }
            }
        }
        err = PGPKeyIterNext( keyiter, &key );
    }
    if( err == kPGPError_EndOfIteration )
        err = kPGPError_NoErr;

    return err;
}

int disableKeyOnKeyServer(struct pgpmainBones *mainbPtr, char *keyguffin,
char *keyServerURL)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPtlsContextRef tlsContext = kInvalidPGPtlsContextRef;
    PGPtlsSessionRef tlsSession = kInvalidPGPtlsSessionRef;
    PGPKeyServerType type;
    PGPEnv * env = envbPtr->m_env;
    PGPInt32 pri;
    PGPError err,er2;
    PGPKeySetRef ringSet = NULL;
    /*int status;*/

    err = PGPOpenDefaultKeyRings( context, 0, &ringSet );
    if( IsPGPError(err) ) {
        fprintf(filebPtr->pgpout,
                LANG("\nCan't open default key ring file\n"));
        return -1;
    }
    mainbPtr->workingRingSet=ringSet;

    err = PGPKeyServerInit();
    pgpAssertNoErr(err);

    err = PGPNewKeyServerFromURL( context, keyServerURL,
            kPGPKeyServerAccessType_Administrator,
            kPGPKeyServerKeySpace_Default,
            &mainbPtr->workingKeyServer );
    if( IsPGPError(err) )
        pgpShowError(filebPtr, err,__FILE__,__LINE__);

    err = PGPSetKeyServerEventHandler( mainbPtr->workingKeyServer,
            srvhandler, (PGPUserValue)mainbPtr);

    pgpAssertNoErr(err);

	if(IsPGPError(err = PGPGetKeyServerType(mainbPtr->workingKeyServer, &type)))
	{
        err = 1;
		goto done;
	}

	if (type == kPGPKeyServerType_LDAPS) {
		if(IsPGPError(err = PGPNewTLSContext(context, &tlsContext)))
		{
            err = 1;
			goto done;
		}

		if(IsPGPError(err = PGPNewTLSSession(tlsContext, &tlsSession)))
		{
            err = 1;
			goto done;
		}
	}


    err = PGPKeyServerOpen( mainbPtr->workingKeyServer, tlsSession );
    if( IsPGPError(err) )  {
        if (!envbPtr->compatible) {
            if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &er2 )) {
                pgpShowError( filebPtr, err, __FILE__,__LINE__);
            }
            else {
                 pgpShowError( filebPtr, err, 0, 0);
            }
        }
        goto done;
    } else {
        PGPKeySetRef keysToDisable=NULL;
        err = pgpGetMatchingKeySet(mainbPtr, keyguffin, 0, &keysToDisable );
        pgpAssertNoErr(err);

        err = pgpUserChooseAndDisableKeys( mainbPtr, keysToDisable );

        if( IsPGPError(err) ) {
            if( err == kPGPError_EndOfIteration ) {
                fprintf( filebPtr->pgpout,
                        LANG("\nKey not found in key ring '%s'.\n"),
                        keyServerURL );
            } else if (!envbPtr->compatible)
                pgpShowError(filebPtr, err, __FILE__,__LINE__);
        }
        if( PGPKeySetNeedsCommit( keysToDisable ) ) {
            err = PGPCommitKeyRingChanges( keysToDisable );
            if( IsPGPError(err))
                pgpShowError(filebPtr,err,0,0);
        }
        if( keysToDisable )
            PGPFreeKeySet( keysToDisable );
    }

    er2 = PGPKeyServerClose( mainbPtr->workingKeyServer );
    pgpAssertNoErr(er2);
done:
    if(tlsSession != kInvalidPGPtlsSessionRef)
        PGPFreeTLSSession(tlsSession);
    if(tlsContext != kInvalidPGPtlsContextRef)
        PGPFreeTLSContext(tlsContext);
    if(mainbPtr->workingKeyServer != kPGPInvalidRef) 
        PGPFreeKeyServer( mainbPtr->workingKeyServer );
    mainbPtr->workingKeyServer = NULL;

    er2 = PGPKeyServerCleanup();
    pgpAssertNoErr(er2);

    if( ringSet ) {
        PGPFreeKeySet( ringSet );
        mainbPtr->workingRingSet = NULL;
    }

    return IsPGPError(err) ? -1 : 0;
}

int revokeOrDisableKey(struct pgpmainBones *mainbPtr, char *keyguffin,
        PGPFileSpecRef keyfilespec)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPEnv *env = mainbPtr->envbPtr->m_env;
    PGPBoolean compatible = mainbPtr->envbPtr->compatible;
    PGPKeySetRef ringset = NULL;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyRef key;
    PGPBoolean propData = FALSE;
    PGPError err, er2;
    PGPUInt32 pri;
    PGPBoolean needsfree = FALSE;
    char* passphrase = 0;
    int nkeysrevoked = 0;

    err = pgpOpenKeyringsFromPubringSpec( mainbPtr, keyfilespec, &ringset, kPGPKeyRingOpenFlags_Mutable );
    if ( IsPGPError(err)) goto done;

    mainbPtr->workingRingSet=ringset;
    /*mainbPtr->workingGroupSet=NULL;*/
    err = pgpGetMatchingKeyList( mainbPtr, keyguffin, kMatch_NotKeyServer,
            &keylist );

    pgpAssertNoErr(err);

    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( keyiter, &key );

    while( key != NULL ) {

        pgpShowKeyBrief( filebPtr, key );
        pgpShowKeyFingerprint( filebPtr, key );

        /* XXX the old version used to ask for your password here... now
           is different a bit*/

        err = PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &propData );
        if( propData ) {
            PGPUserIDRef alias;
            PGPSize actual;
            char useridstr[ kPGPMaxUserIDSize ];
            err = PGPGetPrimaryUserID( key, &alias );
            pgpAssertNoErr(err);
            err = PGPGetUserIDStringBuffer( alias, kPGPUserIDPropName,
                    kPGPMaxUserIDSize, useridstr, &actual );

            pgpAssertNoErr(err);
            fprintf(filebPtr->pgpout,
                LANG("\nDo you want to permanently revoke your public key\n"
                "by issuing a secret key compromise certificate\n"
                "for \"%s\" (y/N)? "), useridstr );
            if( getyesno( filebPtr, 'n', 0 ) ) {
                 err = pgpGetValidPassphrase( mainbPtr, key, &passphrase,
                         &needsfree);

                 if( err == 0 ) {
                     nkeysrevoked++;
                     err = PGPRevokeKey( key,
                         PGPOPassphrase( context, passphrase ),
                         PGPOLastOption( context )
                         );
                     pgpAssertNoErr( err );
                 }
                 goto tail;
            }
        }

        err = PGPGetKeyBoolean( key, kPGPKeyPropIsDisabled, &propData );
        pgpAssertNoErr( err );

        if( propData ) {
            fprintf( filebPtr->pgpout,
                LANG("\nKey is already disabled.\n"
                     "Do you want to enable this key again (y/N)? "));
            if( getyesno( filebPtr,'n',0) ) {
                err = PGPEnableKey( key );
                pgpAssertNoErr( err );
            }
        } else {
            fprintf( filebPtr->pgpout,
                LANG("\nDisable this key (y/N)? "));
            if( getyesno( filebPtr,'n',0) ) {
                err = PGPDisableKey( key );
                pgpAssertNoErr( err );
            }
        }
tail:
        err = PGPKeyIterNext( keyiter, &key );
    }

    /*if( nkeysrevoked > 0 ) {
          err = PGPPropagateTrust( ringset );
          pgpAssertNoErr( err );
      }*/

    if( PGPKeySetNeedsCommit( ringset ) ) {
        err = PGPCommitKeyRingChanges( ringset );
    }

 done:
    if( needsfree ) {
        PGPFreeData( passphrase );
        pgpRemoveFromPointerList( mainbPtr->leaks, passphrase );
    }
    if (ringset)
        er2 = PGPFreeKeySet( ringset );
    mainbPtr->workingRingSet = NULL;
    if (keyiter)
        er2 = PGPFreeKeyIter( keyiter );
    if (keylist)
        er2 = PGPFreeKeyList( keylist );

    if ( !compatible && IsPGPError(err) &&
       pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &er2) )
       pgpShowError(filebPtr, err,__FILE__,__LINE__);

    return err;
}



