/*____________________________________________________________________________
    keyremove.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

    PGP 6.5 Command Line 

    use the PGP SDK to remove keys, signatures, and revoke keys and sigs.

    $Id: keyremove.c,v 1.13 1999/05/12 21:01:04 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <string.h>



#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpKeyServer.h"

#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpContext.h"

#include "usuals.h"
#include "pgp.h"
#include "globals.h"
#include "language.h"
#include "prototypes.h"
#include "pgpSDKPrefs.h"

#include "fileio.h"


/*
  remove keys from the key database associated with the keyset,
  but do it under user control.
 */
PGPError pgpUserChooseAndRemoveKeys( struct pgpmainBones *mainbPtr,
        PGPKeySetRef keySet)
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;    
    PGPEnv *env = envbPtr->m_env;
    PGPContextRef context = mainbPtr->pgpContext;
    PGPError err,er2;
    PGPInt32 pri;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyRef key, defaultSigningKey = NULL;
    PGPBoolean batchmode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err );
    PGPBoolean compatible = envbPtr->compatible;
    PGPSize removed;
    PGPByte *keyIDData = NULL;
    PGPSize  keyIDSize = 0;
    PGPKeyID keyID;
	PGPBoolean bIsPrivateKey = FALSE;
    pgpAssertNoErr(err);



    /* sort the result set..*/
    err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);

    err = PGPNewKeyIter( keylist, &keyiter );
    if(err) {
        pgpShowError(filebPtr, err, __FILE__, __LINE__);
        goto out;
    }

    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNext( keyiter, &key);
    /*if error, no keys found.*/

    /* In incompatible mode, get default signing key from prefs. */
    if (!compatible) {
        err = PGPsdkPrefGetData( context, kPGPsdkPref_DefaultKeyID,
          (void **)&keyIDData, &keyIDSize );

        if (err==kPGPError_NoErr) {
            err = PGPImportKeyID( keyIDData, &keyID );
            pgpAssertNoErr(err);
            PGPFreeData( keyIDData );

            err = PGPGetKeyByKeyID(keySet, &keyID,
              kPGPPublicKeyAlgorithm_Invalid, &defaultSigningKey);
            pgpAssertNoErr(err);
        }
    }

    removed=0;
    while( key != NULL )
    {
        PGPUInt32 numuserids;
        PGPBoolean ans;

		if(mainbPtr->workingKeyServer)
		{	/* check to see if key returned by server in key set */
			if(!PGPKeySetIsMember(key, mainbPtr->workingRingSet))
			{
				err = PGPKeyIterNext(keyiter, &key);
                if(IsPGPError(err))
                    break;
				continue;
			}
		}
		else
		{
			/* check to see if key is private/public pair */
			PGPGetKeyBoolean(key, kPGPKeyPropIsSecret, &bIsPrivateKey);
			err = pgpCountKeyUserIDs( keyiter, &numuserids );
			pgpAssertNoErr(err);

		}
        err = pgpShowKeyBrief( filebPtr, key );

        if (!compatible) {
            if (defaultSigningKey != NULL && !mainbPtr->workingKeyServer) {
                if (PGPCompareKeys(key, defaultSigningKey,
                   kPGPUserIDOrdering) == 0) {
                   fprintf(filebPtr->pgpout,
LANG("\nWARNING: You are about to remove your default signing key!\n"));
                }
            }
        }

        ans=FALSE;
        if( numuserids > 1 && !mainbPtr->workingKeyServer) {
            fprintf( filebPtr->pgpout,
                    LANG("\nKey has more than one user ID.\n"));
					fprintf( filebPtr->pgpout,
                    LANG("Do you want to remove the whole key (y/N)? "));
            ans = getyesno( filebPtr, 'n', batchmode );
        }
        else 
        {
            /* if user has only one userid,*/
            fprintf( filebPtr->pgpout,
                LANG("\nAre you sure you want this key removed (y/N)? "));
            ans = getyesno( filebPtr, 'n', batchmode );
        }

        if( ans ) {
            PGPKeySetRef    single;

            err = PGPNewSingletonKeySet( key, &single );
            pgpAssertNoErr(err);

            if( mainbPtr->workingKeyServer ) {
                PGPKeySetRef    failed = NULL;
                PGPKeyID        keyid;
                
                /* save keyid of key that user wants to delete */
                err = PGPGetKeyIDFromKey(key, &keyid);
                pgpAssertNoErr(err);
                /* plainfilename not being used, so use it to store keyid 
                   of key to be deleted from server */
                err = PGPGetKeyIDString(&keyid, kPGPKeyIDString_Full,
                                        mainbPtr->plainfilename);
                pgpAssertNoErr(err);


                err = PGPDeleteFromKeyServer( mainbPtr->workingKeyServer,
                        single, &failed);

                if( IsPGPError(err) ) {
                    if(failed) {
                        pgpShowKeyBrief( filebPtr, key );
                        PGPFreeKeySet(failed);
                    }
                }
            } 
			else
			{
                if( mainbPtr->workingRingSet )
				{
					if(bIsPrivateKey)
					{
						fprintf( filebPtr->pgpout, 
						LANG("\nKey or user ID is also present in secret keyring.\n"));
						fprintf( filebPtr->pgpout, 
						LANG("Do you want to remove it from the secret keyring (y/N)? "));
						ans = getyesno( filebPtr, 'n', batchmode );
						if(!ans)
							err = kPGPError_UserAbort;

					}
					if(IsntPGPError(err))
                        err = PGPRemoveKeys( single, mainbPtr->workingRingSet );
				}
			}

            if( IsPGPError(err) )
                pgpShowError( filebPtr, err, 0,0);
            else
            {
                if(!compatible)
                {
                    fprintf( filebPtr->pgpout, mainbPtr->workingKeyServer ? 
                        LANG("Key removed from key server.\n") :
                        LANG("Key removed from key ring.\n"));
                }
                else
                {
                    fprintf( filebPtr->pgpout,
                        LANG("Key removed from key ring.\n"));
                }
            }
            PGPFreeKeySet( single );
            removed++;
        } 
		else if( numuserids > 1 )
		{
			PGPUserIDRef userid;
			char useridstr[ kPGPMaxUserIDSize ];
			PGPSize actual;

			/* list all userids associated with the key. */
			err = PGPKeyIterRewindUserID( keyiter );
			pgpAssertNoErr(err);
			err = PGPKeyIterNextUserID( keyiter, &userid);
			pgpAssertNoErr(err);

			if( userid ) {
				while( userid ) {
					err = PGPGetUserIDStringBuffer( userid,
							kPGPUserIDPropName, kPGPMaxUserIDSize, useridstr,
							&actual );

					fprintf( filebPtr->pgpout, LANG("Remove \"%s\" (y/N)? "),
							useridstr );

					ans = getyesno( filebPtr, 'n', batchmode );

					if( ans ) {
						err = PGPRemoveUserID( userid );
						pgpAssertNoErr(err);
						removed++;
					}
					err = PGPKeyIterNextUserID( keyiter, &userid);
				}
				if(bIsPrivateKey && err == kPGPError_EndOfIteration && removed > 0)
				{
					fprintf( filebPtr->pgpout, 
					LANG("\nKey or user ID is also present in secret keyring.\n"));
					fprintf( filebPtr->pgpout, 
					LANG("Do you want to remove it from the secret keyring (y/N)? "));
					ans = getyesno( filebPtr, 'n', batchmode );
					if(!ans)
					{
						PGPRevertKeyRingChanges(keySet);
						err = kPGPError_UserAbort;
					}
					else
					{
						fprintf(filebPtr->pgpout,
									LANG("\nUser ID(s) removed from key ring.\n"));
					}
				}
			}
		}
		else
		{
			/* user cancelled delete operation */
            err = kPGPError_UserAbort;
            removed = 0;
		}

		break;
    }
    if(err == kPGPError_EndOfIteration && removed > 0)
        err = kPGPError_NoErr;

out:
    if( keyiter )
        er2 = PGPFreeKeyIter( keyiter );
    if( keylist )
        er2 = PGPFreeKeyList( keylist );
    return err;
}

/*
   Remove the first entry in key ring that has useridstr string in userid.
   Or it removes the first matching keyID from the ring.  A non-NULL
   keyID takes precedence over a useridstr specifier.  useridstr is a
   null-terminated C string.  If secringToo is TRUE, the secret keyring
   is also checked.
 */

int removeFromKeyring( struct pgpmainBones *mainbPtr, char *useridstr,
        char *ringFileName, char *origRingFileName)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPError			err;
    PGPKeySetRef		ringSet = kPGPInvalidRef;
    char				*ringFile = NULL;
    PGPKeySetRef		keysToRemove = kPGPInvalidRef;

	char				szTemp[MAX_PATH] = {'\0'};
	char				secringfilename[MAX_PATH] = {'\0'};
	char				pubringfilename[MAX_PATH] = {'\0'};
	PGPFileSpecRef		secRingFileSpec = kPGPInvalidRef;
	PGPFileSpecRef		pubRingFileSpec = kPGPInvalidRef;


	if(ringFileName == NULL)	/* use default key rings */
	{
		err = PGPOpenDefaultKeyRings(context, kPGPKeyRingOpenFlags_Mutable, &ringSet);
	}
	else
	{
		/* see if I can find corresponding key file to open as key pair */

		/* if file doesn't have an extension, try appending ".pkr" on it */
		char		*pszTemp = NULL;

		strcpy(szTemp, ringFileName);
		defaultExtension(filebPtr, szTemp, filebPtr->PKR_EXTENSION);

		if(hasExtension(szTemp, filebPtr->PKR_EXTENSION))
		{
			strcpy(pubringfilename, szTemp);
			if(!dropFilename(szTemp))
			{
				err = -1;
				goto done;
			}
			sprintf(secringfilename, "%s%s", szTemp, "secring.skr");
		}
		else
		{
			strcpy(secringfilename, szTemp);
			if(dropFilename(szTemp))
			{
				err = -1;
				goto done;
			}
			sprintf(pubringfilename, "%s%s", szTemp, "pubring.pkr");
		}

		if(!fileExists(pubringfilename) || !fileExists(secringfilename))
		{
			err = -1;
			goto done;
		}

		err = PGPNewFileSpecFromFullPath(context, secringfilename, &secRingFileSpec);
		if(IsPGPError(err))
			goto done;

		err = PGPNewFileSpecFromFullPath(context, pubringfilename, &pubRingFileSpec);
		if(IsPGPError(err))
			goto done;

		err = PGPOpenKeyRingPair(context, kPGPKeyRingOpenFlags_Mutable, pubRingFileSpec,
									secRingFileSpec, &ringSet);
	}
	if(IsPGPError(err))
		goto done;


#if 0
    PGPGetFullPathFromFileSpec( ringFileSpec, &ringFile );

    fprintf( filebPtr->pgpout,
            LANG("\nRemoving from key ring: '%s', userid \"%s\".\n"),
            ringFile, useridstr );
#endif 0

    mainbPtr->workingRingSet = ringSet;
    err = pgpGetMatchingKeySet(mainbPtr, useridstr, 0, &keysToRemove );
    if(IsPGPError(err))
		goto done;

    err = pgpUserChooseAndRemoveKeys( mainbPtr, keysToRemove);

    if( PGPKeySetNeedsCommit( ringSet ) && IsntPGPError(err)) {
        err = PGPCommitKeyRingChanges( ringSet );
    }

done:
	if(keysToRemove != kPGPInvalidRef)
		PGPFreeKeySet(keysToRemove);
	if(secRingFileSpec != kPGPInvalidRef)
		PGPFreeFileSpec(secRingFileSpec);
	if(pubRingFileSpec != kPGPInvalidRef)
		PGPFreeFileSpec(pubRingFileSpec);
	if(ringSet != kPGPInvalidRef)
		PGPFreeKeySet(ringSet);
    mainbPtr->workingRingSet = NULL;

	if( IsntPGPError(err))
        return 0;
    else
        return -1;
}


PGPError srvhandler(PGPContextRef context, struct PGPEvent *event,
        PGPUserValue userValue)
{
    struct pgpmainBones *mainbPtr = (struct pgpmainBones *)userValue;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = pgpContextGetEnvironment( context );
    PGPError err;
    PGPInt32 pri;
    PGPInt32 verbose = pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err );
    PGPJobRef job = event->job;

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
            pgpShowError( filebPtr, event->data.errorData.error, 0,0);
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

        case kPGPEvent_KeyServerEvent:       /* Key Server progress */
            if(verbose)
                fprintf( filebPtr->pgpout,
                        LANG("event %d: progress, state=%d\n"),
                        event->type, event->data.keyServerData.state);

            err = kPGPError_NoErr;
            break;

        case kPGPEvent_KeyServerSignEvent:     /* Key Server passphrase */
            {
                PGPKeyRef	sigkey;
                char *passphrase = NULL;
                /*PGPKeyServerRef keyServer =
                    event->data.keyServerSignData.keyServerRef;*/
                if(verbose)
				    fprintf( filebPtr->pgpout,
                            LANG("event %d: keyserver wants signature\n"),
                            event->type);
				err = pgpGetSigningKey(mainbPtr, &sigkey);

                if( IsntPGPError(err) ) {
                    PGPBoolean mine;
                    err = pgpRewindPassphrase( envbPtr->passwds );
                    pgpAssertNoErr(err);
                    err = pgpGetValidPassphrase( mainbPtr, sigkey,
                            &passphrase, &mine );
                    if( IsntPGPError(err) ) {
                        err = PGPAddJobOptions(job,
                                PGPOSignWithKey(context, sigkey,
                                        PGPOPassphrase(context, passphrase),
                                        PGPOLastOption( context )),
                                PGPOClearSign( context, TRUE ),
                                PGPOLastOption( context));
                        pgpAssertNoErr(err);
                    }
                    if(mine) {
                        PGPFreeData(passphrase);
                        pgpRemoveFromPointerList(mainbPtr->leaks,passphrase);
                    }
                }
                if( IsPGPError(err) )
                    pgpShowError(filebPtr, err, __FILE__,__LINE__);
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

int removeFromKeyServer(struct pgpmainBones *mainbPtr, char *useridstr,
        char *keyServerURL)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv * env = envbPtr->m_env;
    PGPtlsContextRef          tlsContext = kInvalidPGPtlsContextRef;
    PGPtlsSessionRef        tlsSession = kInvalidPGPtlsSessionRef;
    PGPKeyServerType          type;
    PGPError err,er2;
    PGPInt32 pri;
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
        err = -1;
		goto done;
	}
     
	if (type == kPGPKeyServerType_LDAPS) {
		if(IsPGPError(err = PGPNewTLSContext(context, &tlsContext)))
		{
            err = -1;
			goto done;
		}

		if(IsPGPError(err = PGPNewTLSSession(tlsContext, &tlsSession)))
		{
            err = -1;
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
        PGPKeySetRef keysToRemove=NULL;
        err = pgpGetMatchingKeySet(mainbPtr, useridstr, 0, &keysToRemove );
        pgpAssertNoErr(err);

        err = pgpUserChooseAndRemoveKeys( mainbPtr, keysToRemove);

        if( IsPGPError(err) ) {
            if( err == kPGPError_EndOfIteration ) {
                fprintf( filebPtr->pgpout,
                        LANG("\nKey not found on key server '%s'.\n"),
                        keyServerURL );
            } else if (!envbPtr->compatible)
                pgpShowError(filebPtr, err, __FILE__,__LINE__);
        }
        if( PGPKeySetNeedsCommit( keysToRemove ) ) {
            err = PGPCommitKeyRingChanges( keysToRemove );
            if( IsPGPError(err))
                pgpShowError(filebPtr,err,0,0);
        }
        if(keysToRemove)
            PGPFreeKeySet(keysToRemove);
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


PGPError countSigsOnKey( PGPKeyIterRef keyiter, PGPUInt32 *nsigs) {

    PGPUserIDRef userid;
    PGPSigRef sig;
    PGPError err;

    (*nsigs) = 0;

    err = PGPKeyIterRewindUserID( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNextUserID( keyiter, &userid );
    pgpAssertNoErr(err);

    /* for all userids of key */
    while( userid ) {

        err = PGPKeyIterRewindUIDSig( keyiter );
        pgpAssertNoErr(err);
        err = PGPKeyIterNextUIDSig( keyiter, &sig );
        /*if err, there are none.*/

        /* for all sigs of all userids of all keys */
        while( sig != NULL ) {

            (*nsigs)++;

            err = PGPKeyIterNextUIDSig( keyiter, &sig );
            /*if err, there are no more.*/
        }
        err = PGPKeyIterNextUserID(keyiter,&userid);
        /*if err, there are no more.*/
    }

    return kPGPError_NoErr;

} /* countSigsOnKey */


PGPError revokeSigs( struct pgpmainBones* mainbPtr, char* mcguffin,
        char* ringfile )
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPError err;
    PGPFileSpecRef ringFileSpec = NULL;
    PGPKeySetRef ringSet = NULL;
    PGPKeySetRef keyset = NULL;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyRef key;
    PGPKeyRef sigkey;
    PGPUInt32 nkeys = 0;
    PGPUInt32 nsigs = 0;
    int keeping = 0;
    int nremoved = 0;

    fprintf( filebPtr->pgpout,
LANG("\nRevoking your signatures from userid '%s' in key ring '%s'\n"),
             (mcguffin), ringfile );

    /* find mcguffin in specified ring */
    err = PGPNewFileSpecFromFullPath( context, ringfile, &ringFileSpec );
    pgpAssertNoErr(err);

    /* we need this for getting our secret key */
    err = pgpOpenKeyringsFromPubringSpec( mainbPtr, ringFileSpec,
				&ringSet, kPGPKeyRingOpenFlags_Mutable );
    PGPFreeFileSpec(ringFileSpec);
    if( IsPGPError(err) ) {
        fprintf(filebPtr->pgpout,
                LANG("\n\007Can't open keyring '%s'.\n"),ringfile );
        return -1;
    }

    mainbPtr->workingRingSet=ringSet;
    /*mainbPtr->workingGroupSet=NULL;*/
    err = pgpGetMatchingKeySet(mainbPtr, mcguffin, 0, &keyset );
    pgpAssertNoErr(err);

    err = PGPCountKeys( keyset, &nkeys );
    pgpAssertNoErr(err);

    if( nkeys == 0 ) {
        fprintf(filebPtr->pgpout,
                LANG("\n\007Key not found in key ring '%s'.\n"),
                ringfile );
        goto done;
    }

    keeping = 1;

    err = PGPCheckKeyRingSigs( keyset, ringSet, 0, NULL, NULL );

    err = PGPOrderKeySet( keyset, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( keyiter, &key);
    pgpAssertNoErr(err);

    /* for all keys */
    while( key != NULL ) {
        PGPUserIDRef userid;
        PGPSize actual;
        PGPSigRef sig;
        char useridstr[ kPGPMaxUserIDSize ];

        err = PGPKeyIterRewindUserID( keyiter );
        pgpAssertNoErr(err);
        err = PGPKeyIterNextUserID( keyiter, &userid );
        pgpAssertNoErr(err);

        /* for all userids of key */
        while( userid ) {
            err = PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName,
                    kPGPMaxUserIDSize, useridstr, &actual );

            pgpAssertNoErr(err);

            err = PGPKeyIterRewindUIDSig( keyiter );
            pgpAssertNoErr(err);
            err = PGPKeyIterNextUIDSig( keyiter, &sig );
            pgpAssertNoErr(err);
            /* for all sigs of all userids of all keys */
            while( sig != NULL ) {
                PGPBoolean propData;
                err = PGPGetSigBoolean( sig, kPGPSigPropIsMySig, &propData );
                if( propData ) {
                    nsigs++;
                    pgpShowKeyBrief( filebPtr, key );
                    fprintf( filebPtr->pgpout, LANG("\nuser ID: %s\n"),
                            useridstr );
                    pgpShowKeySig( filebPtr, ringSet, sig, kShow_Checks );
                    fprintf(filebPtr->pgpout, LANG(
                        "Revoke your signature of this user id (y/N)? "));
                    if (!(keeping = !getyesno(filebPtr,'n',0))) {
                        char* passphrase;
                        PGPBoolean mine = 0;
                        nremoved++;
                        err = PGPGetSigCertifierKey( sig, ringSet, &sigkey);
                        pgpAssertNoErr(err);
                        err = pgpGetValidPassphrase( mainbPtr, sigkey,
                                &passphrase, &mine );
                        pgpAssertNoErr(err);
                        err = PGPRevokeSig( sig, ringSet,
                                PGPOPassphrase( context, passphrase ),
                                PGPOLastOption(context) );

                        pgpAssertNoErr(err);
                        if(mine) {
                            PGPFreeData(passphrase);
                            pgpRemoveFromPointerList( mainbPtr->leaks,
                                    passphrase );

                        }
                        break;
                    }
                }
                err = PGPKeyIterNextUIDSig( keyiter, &sig );
            }
            err = PGPKeyIterNextUserID(keyiter,&userid);
        }
        err = PGPKeyIterNext( keyiter, &key );
    }

    if( nremoved == 0 )
        fprintf(filebPtr->pgpout,LANG("\nNo user id's revoked.\n"));
    else {
        fprintf(filebPtr->pgpout,LANG("\n%d user id(s) revoked.\n"),
                nremoved);
    }

    /*PGPPropagateTrust( ringSet ); <- not here.*/
    if( PGPKeySetNeedsCommit( ringSet ) ) {
        err = PGPCommitKeyRingChanges( ringSet );
        pgpAssertNoErr(err);
    }

done:
    if(keyiter)
        PGPFreeKeyIter(keyiter);
    if(keylist)
        PGPFreeKeyList(keylist);
    if(keyset)
        PGPFreeKeySet(keyset);
    if( ringSet ) {
        PGPFreeKeySet( ringSet );
        mainbPtr->workingRingSet = NULL;
    }
    return kPGPError_NoErr;
}

PGPError removeSigs(struct pgpmainBones* mainbPtr, char* mcguffin, char*
        ringfile, char *origRingFileName )
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPBoolean compatible = mainbPtr->envbPtr->compatible;
    PGPEnv *env = mainbPtr->envbPtr->m_env;
    PGPUInt32 pri;
    PGPError err, er2;
    PGPFileSpecRef ringFileSpec = NULL;
    PGPKeySetRef ringSet = NULL;
    PGPKeySetRef keyset = NULL;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyRef key;
    PGPUInt32 nkeys = 0;
    PGPUInt32 nsigs = 0;
    int keeping = 0;
    int nremoved = 0;

    fprintf( filebPtr->pgpout,
        LANG("\nRemoving signatures from userid '%s' in key ring '%s'\n"),
        (mcguffin), ringfile );

    /* find mcguffin in specified ring */
    err = PGPNewFileSpecFromFullPath( context, ringfile, &ringFileSpec );
    pgpAssertNoErr(err);
    err = pgpOpenKeyringsFromPubringSpec( mainbPtr, 
				ringFileSpec, &ringSet, kPGPKeyRingOpenFlags_Mutable);
    if( IsPGPError(err) ) {
        fprintf(filebPtr->pgpout,
                LANG("\nCan't open key ring file '%s'\n"),
                ringfile);
        goto done;
    }

    mainbPtr->workingRingSet=ringSet;
    /*mainbPtr->workingGroupSet=NULL;*/
    err = pgpGetMatchingKeySet(mainbPtr, mcguffin, 0, &keyset );
    pgpAssertNoErr(err);

    err = PGPCountKeys( keyset, &nkeys );
    pgpAssertNoErr(err);

    if( nkeys == 0 ) {
        fprintf(filebPtr->pgpout,
                LANG("\n\007Key not found in key ring '%s'.\n"),
                origRingFileName );
        goto done;
    }

    keeping = 1;

    err = PGPOrderKeySet( keyset, kPGPAnyOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( keyiter, &key);
    pgpAssertNoErr(err);

    /* for all keys */
    while( key != NULL ) {
        PGPUserIDRef userid;
        PGPSize actual;
        PGPSigRef sig;
        char useridstr[ kPGPMaxUserIDSize ];

        err = PGPKeyIterRewindUserID( keyiter );
        pgpAssertNoErr(err);
        err = PGPKeyIterNextUserID( keyiter, &userid );
        pgpAssertNoErr(err);

        pgpShowKeyBrief( filebPtr, key );

        err =  countSigsOnKey( keyiter, &nsigs );
        pgpAssertNoErr(err);

        if( nsigs == 0 ) {
            fprintf(filebPtr->pgpout,
                    LANG("\nKey has no signatures to remove.\n"));
        }
        else {
            fprintf(filebPtr->pgpout,
                    LANG("\nKey has %d signatures(s):\n"),nsigs);
        }

        nsigs=0;

	err = PGPKeyIterRewindUserID(keyiter);
	pgpAssertNoErr(err);
	err = PGPKeyIterNextUserID(keyiter, &userid);
	pgpAssertNoErr(err);

        /* for all userids of key */
        while( userid ) {
            err = PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName,
                    kPGPMaxUserIDSize, useridstr, &actual );
	    fprintf(filebPtr->pgpout,
		LANG("\nSingatures for userid %s:\n"), useridstr);

            pgpAssertNoErr(err);

            err = PGPKeyIterRewindUIDSig( keyiter );
            pgpAssertNoErr(err);
            err = PGPKeyIterNextUIDSig( keyiter, &sig );
            /*if err, there are none.*/
            /* for all sigs of all userids of all keys */
            while( sig != NULL ) {
                nsigs++;
                pgpShowKeySig( filebPtr, ringSet, sig, 0 );
                fprintf(filebPtr->pgpout,
                        LANG("Remove this signature (y/N)? "));

                if (!(keeping = !getyesno(filebPtr,'n',0))) {
                    nremoved++;
                    PGPRemoveSig( sig );
                }
                err = PGPKeyIterNextUIDSig( keyiter, &sig );
                /*if err, there are no more.*/
            }
            err = PGPKeyIterNextUserID(keyiter,&userid);
            /*if err, there are no more.*/
        }
        err = PGPKeyIterNext( keyiter, &key );
    }
    if( err == kPGPError_EndOfIteration )
        err = kPGPError_NoErr;

    if( nremoved == 0 )
        fprintf(filebPtr->pgpout,LANG("\nNo key signatures removed.\n"));
    else
        fprintf(filebPtr->pgpout,
        LANG("\n%d key signatures(s) removed.\n"), nremoved);

    if( PGPKeySetNeedsCommit( ringSet ) ) {
        err = PGPCommitKeyRingChanges( ringSet );
        pgpAssertNoErr(err);
    }

done:
    if( keyiter)
        PGPFreeKeyIter( keyiter );
    if( keylist )
        PGPFreeKeyList( keylist );
    if( keyset )
        PGPFreeKeySet( keyset );
    if( ringSet )
        PGPFreeKeySet( ringSet );
    mainbPtr->workingRingSet = NULL;
    if( ringFileSpec )
        PGPFreeFileSpec( ringFileSpec);

    if (!compatible && IsPGPError(err) &&
        pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &er2))
        pgpShowError( filebPtr, err, __FILE__,__LINE__);

    return err;
}















