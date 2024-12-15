/*____________________________________________________________________________
    keyexport.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    use the PGP SDK to export a key from the keyring and report in
    PGP262 style.

    $Id: keyexport.c,v 1.12 1999/05/12 21:01:03 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>

#include "pgpBase.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpKeyServer.h"
#include "pgpUtilities.h"

#include "usuals.h"
#include "pgp.h"
#include "globals.h"
#include "prototypes.h"
#include "fileio.h"

#include "language.h"

/*
   Upload to the keyserver in one swell foop.
 */
PGPError pgpUploadToURL( struct pgpmainBones *mainbPtr, char *keyserverURL,
        PGPKeySetRef keySet, PGPKeySetRef *failedKeys )
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPKeyServerRef destKeyServer = NULL;
    PGPtlsContextRef		tlsContext = kInvalidPGPtlsContextRef;
    PGPtlsSessionRef        tlsSession = kInvalidPGPtlsSessionRef;
    PGPKeyServerType		type;

    PGPError err,er2;

    err = PGPNewKeyServerFromURL( context, keyserverURL,
            kPGPKeyServerAccessType_Default,
            kPGPKeyServerKeySpace_Default,
            &destKeyServer );
    if( IsPGPError(err) )
        pgpShowError(filebPtr, err,__FILE__,__LINE__);

	if(IsPGPError(err = PGPGetKeyServerType(destKeyServer, &type)))
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


    err = PGPKeyServerOpen( destKeyServer, tlsSession );
    if( IsPGPError(err) )  {
        pgpShowError( filebPtr, err, __FILE__,__LINE__);
        goto done;
    }

    err = PGPUploadToKeyServer( destKeyServer, keySet, failedKeys );
    if( IsPGPError(err) )  {
        pgpShowError( filebPtr, err, __FILE__,__LINE__);
    }

    /* I wonder if we should upload private keys too??
       err = PGPUploadPrivateKeysToKeyServer( destKeyServer, keySet,
       &failedKeys ); */

    er2 = PGPKeyServerClose( destKeyServer );
    pgpAssertNoErr(er2);

done:
    if(tlsSession != kInvalidPGPtlsSessionRef)
        PGPFreeTLSSession(tlsSession);
    if(tlsContext != kInvalidPGPtlsContextRef)
        PGPFreeTLSContext(tlsContext);
    if( destKeyServer )
        PGPFreeKeyServer( destKeyServer );

    return err;
}

/*
   export keys to a file

   side effects: (historical origin, sorry)
    parameter keyfile is a buffer of length MAX_PATH
    that if empty, will be set and returned.
 */

int extractFromKeyring(struct pgpmainBones *mainbPtr, char *useridstr,
        char *keyfile, PGPFileSpecRef ringFileSpec, char
        *origRingFileName, PGPBoolean transflag)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr=mainbPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPBoolean compatible = envbPtr->compatible;
    PGPBoolean isprivate = FALSE;
    PGPFileSpecRef keyFileSpec = NULL;
    PGPError err,er2;
    PGPInt32 pri;
    PGPInt32 verbose = pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &err );
    PGPKeySetRef ringSet;
    PGPKeySetRef keySet = NULL;
    /*PGPBoolean armorout = pgpenvGetInt( env, PGPENV_ARMOR, &pri, &err);*/
    char *ringFile = NULL;
    PGPSize num;

    err = pgpOpenKeyringsIfSecringSpec( mainbPtr, ringFileSpec, &ringSet,
	    &isprivate, 0);
    if ( IsPGPError(err) ) goto done;

    if(verbose && isprivate)
        fprintf( filebPtr->pgpout, LANG("trying to export private keys\n"));

    err = PGPGetFullPathFromFileSpec( ringFileSpec, &ringFile );
    pgpAssertNoErr(err);

    if(useridstr && useridstr[0] != '\0')
        fprintf(filebPtr->pgpout, LANG(
"Extracting from keyring '%s', userid \"%s\".\n"), ringFile, useridstr);

    /*else we extract the whole keyring...*/

    mainbPtr->workingRingSet = ringSet;
    /*mainbPtr->workingGroupSet = NULL;*/
    err = pgpGetMatchingKeySet( mainbPtr, useridstr, 0, &keySet);

    pgpAssertNoErr(err);
    PGPCountKeys( keySet, &num );
    if( num < 1 ) {
        fprintf(filebPtr->pgpout,
                LANG("Key not found in key ring '%s'.\n"),origRingFileName);
        err = kPGPError_ItemNotFound;
        goto done;
    }

    if( keyfile[0] == '\0' ) {
        fprintf(filebPtr->pgpout,
                LANG("\nExtract the above key(s) into which file? "));
        pgpTtyGetString(keyfile, MAX_PATH - 1, filebPtr->pgpout);

        if (transflag)
            forceExtension(filebPtr, keyfile, filebPtr->ASC_EXTENSION );
        else
           forceExtension(filebPtr, keyfile, filebPtr->PGP_EXTENSION );
        /* XXX check that file exists? 262 DOESN'T here, but does later.*/
    }

    if( pgpLocationIsURL( keyfile ) ) {
        /* put the keys onto the key server...*/
        PGPKeySetRef failedKeys = NULL;
        err = PGPKeyServerInit();
        pgpAssertNoErr(err);
        err = pgpUploadToURL( mainbPtr, keyfile, keySet, &failedKeys );
        if( IsPGPError(err) ) {
            PGPSize num;
            /*pgpShowError( filebPtr, err, __FILE__,__LINE__ );*/
            if( failedKeys ) {
                PGPCountKeys( failedKeys, &num );
                fprintf( filebPtr->pgpout,
                        LANG("\nfailed to upload %d keys.\n"), num );
                PGPFreeKeySet( failedKeys );
            }
        }
        er2 = PGPKeyServerCleanup();
        pgpAssertNoErr(er2);

    } else {
        err = PGPNewFileSpecFromFullPath(context, keyfile, &keyFileSpec );
        pgpAssertNoErr(err);
        err = PGPExportKeySet( keySet,
            PGPOOutputFile(context, keyFileSpec ),
            PGPOExportPrivateKeys( context, isprivate ),
            transflag ? PGPOArmorOutput(context, transflag ) :
            PGPONullOption(context),
            PGPOLastOption(context));
    }

done:
    if( !compatible && IsPGPError( err ) && verbose )
        pgpShowError( filebPtr, err,__FILE__,__LINE__ );
    if(ringFile)
        PGPFreeData(ringFile);
    if(keyFileSpec)
        PGPFreeFileSpec(keyFileSpec);
    if(keySet)
        PGPFreeKeySet(keySet);
    if(ringSet)
        PGPFreeKeySet(ringSet);
    mainbPtr->workingRingSet = NULL;

    return err;
}

int extractFromKeyServer(struct pgpmainBones *mainbPtr, char *useridstr,
        char *keyfile, char *keyServerURL, PGPBoolean transflag)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr=mainbPtr->envbPtr;
    PGPBoolean compatible = envbPtr->compatible;
    PGPBoolean isprivate = FALSE;
    PGPFileSpecRef keyFileSpec = NULL;
    PGPError err,er2;
    PGPInt32 pri;
    PGPEnv * env = envbPtr->m_env;
    PGPKeySetRef keySet = NULL;
    char *ringFile = NULL;
    PGPSize num;
    PGPtlsContextRef		tlsContext = kInvalidPGPtlsContextRef;
    PGPtlsSessionRef        tlsSession = kInvalidPGPtlsSessionRef;
    PGPKeyServerType		type;

    err = PGPKeyServerInit();
    pgpAssertNoErr(err);

    err = PGPNewKeyServerFromURL( context, keyServerURL,
            kPGPKeyServerAccessType_Default,
            kPGPKeyServerKeySpace_Default,
            &mainbPtr->workingKeyServer );
    if( IsPGPError(err) )
        pgpShowError(filebPtr, err,__FILE__,__LINE__);

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
        if (!compatible) {
            if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &er2 )) {
                pgpShowError( filebPtr, err, __FILE__,__LINE__);
            }
            else {
                 pgpShowError( filebPtr, err, 0, 0);
            }
        }
        goto done;
    }

    /*mainbPtr->workingRingSet = NULL;*/
    err = pgpGetMatchingKeySet( mainbPtr, useridstr, 0, &keySet);

    pgpAssertNoErr(err);
    PGPCountKeys( keySet, &num );
    if( num < 1 ) {
        fprintf(filebPtr->pgpout,
                LANG("Key not found in key ring '%s'.\n"),ringFile);
        err = kPGPError_ItemNotFound;
        goto done;
    }

    if( keyfile[0] == '\0' ) {
        fprintf(filebPtr->pgpout,
                LANG("\nExtract the above key(s) into which file? "));
        pgpTtyGetString(keyfile, MAX_PATH - 1, filebPtr->pgpout);
        if (transflag)
            forceExtension(filebPtr, keyfile, filebPtr->ASC_EXTENSION );
        else
           forceExtension(filebPtr, keyfile, filebPtr->PGP_EXTENSION );
        /* XXX check that file exists? 262 DOESN'T here, but does later.*/
    }

    if( pgpLocationIsURL( keyfile ) ) {
        /* put the keys onto the key server...*/
        PGPKeySetRef failedKeys = NULL;
        /*err = PGPKeyServerInit();*/
        /*pgpAssertNoErr(err);*/

        err = pgpUploadToURL( mainbPtr, keyfile, keySet, &failedKeys );
        if( IsPGPError(err) ) {
            PGPSize num;
            /*pgpShowError( filebPtr, err, __FILE__,__LINE__ );*/
            if( failedKeys ) {
                PGPCountKeys( failedKeys, &num );
                fprintf( filebPtr->pgpout,
                        LANG("\nfailed to upload %d keys.\n"), num );
                PGPFreeKeySet( failedKeys );
            }
        }
        /*er2 = PGPKeyServerCleanup();*/
        /*pgpAssertNoErr(er2);*/
    } else {
        err = PGPNewFileSpecFromFullPath(context, keyfile, &keyFileSpec );
        pgpAssertNoErr(err);
        err = PGPExportKeySet( keySet,
            PGPOOutputFile(context, keyFileSpec ),
            PGPOExportPrivateKeys( context, isprivate ),
            transflag ? PGPOArmorOutput(context, transflag ) :
            PGPONullOption(context),
            PGPOLastOption(context));
    }
    if( !compatible && IsPGPError( err ) )
        pgpShowError( filebPtr, err,__FILE__,__LINE__ );

    er2 = PGPKeyServerClose( mainbPtr->workingKeyServer );
    pgpAssertNoErr(er2);

done:
    if(keyFileSpec)
        PGPFreeFileSpec(keyFileSpec);
    if(keySet)
        PGPFreeKeySet(keySet);
    /*PGPFreeKeySet(ringSet);*/
    mainbPtr->workingRingSet = NULL;
    if(tlsSession != kInvalidPGPtlsSessionRef)
        PGPFreeTLSSession(tlsSession);
    if(tlsContext != kInvalidPGPtlsContextRef)
        PGPFreeTLSContext(tlsContext);
    if(mainbPtr->workingKeyServer != kPGPInvalidRef) 
        PGPFreeKeyServer( mainbPtr->workingKeyServer );
    mainbPtr->workingKeyServer = NULL;

    er2 = PGPKeyServerCleanup();
    pgpAssertNoErr(er2);

    return IsPGPError(err) ? -1 : 0;
}

