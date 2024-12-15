/*____________________________________________________________________________
    keymaint.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    $Id: keymaint.c,v 1.10 1999/05/12 21:01:04 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpErrors.h"

#include "usuals.h"
#include "pgp.h"
#include "globals.h"
#include "language.h"
#include "prototypes.h"

/*
   Called by the pgp -kc option.
 */
PGPError pgpDoCheckKeyRing(struct pgpmainBones *mainbPtr, char *useridStr )
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPError err;
    PGPKeySetRef checkSet = NULL;

    err = pgpGetMatchingKeySet(mainbPtr, useridStr, 0, &checkSet );
    if( IsPGPError(err) ) goto done;

    err = PGPCheckKeyRingSigs( checkSet, mainbPtr->workingRingSet, TRUE,
            NULL, NULL );

    pgpAssertNoErr(err);

    err = viewKeySet( mainbPtr, checkSet, kShow_Sigs | kShow_Checks );
    pgpAssertNoErr(err);

done:
    if( checkSet )
        PGPFreeKeySet( checkSet );

    return err;
}

PGPError dokeycheck(struct pgpmainBones *mainbPtr, char *useridStr,
        PGPFileSpecRef ringFileSpec)
{
    PGPKeySetRef ringSet = NULL;
    char *ringfile = NULL;
    PGPError err, er2;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPInt32 pri;
    PGPBoolean quietmode = pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err);
    PGPBoolean compatible = envbPtr->compatible;

    err = PGPGetFullPathFromFileSpec( ringFileSpec, &ringfile );
    pgpAssertNoErr(err);


    err = PGPOpenKeyRing(mainbPtr->pgpContext, 0, ringFileSpec, &ringSet);
#if 0
    err = pgpOpenKeyringsFromPubringSpec( mainbPtr, ringFileSpec, &ringSet, 0);
#endif 0

    if( IsPGPError(err) ) goto done;

    if (!quietmode) fprintf(filebPtr->pgpout,
         LANG("\nKey ring: '%s'"), ringfile);

    mainbPtr->workingRingSet=ringSet;
    err = pgpDoCheckKeyRing(mainbPtr, useridStr);
    pgpAssertNoErr(err);

done:
    if (ringfile)
        er2 = PGPFreeData( ringfile );
    if (ringSet)
        er2 = PGPFreeKeySet( ringSet );
    mainbPtr->workingRingSet = NULL;

    if( !compatible && IsPGPError(err) &&
        pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &er2) )
        pgpShowError(filebPtr, err, __FILE__,__LINE__);

    return err;
}

PGPError findUltimatelyTrustedKeys(struct pgpmainBones *mainbPtr,
        PGPKeySetRef secRingSet,
        PGPKeySetRef bothRingsSet,
        PGPFilterRef *depthFilter)
{

    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPKeyIterRef keyiter;
    PGPKeyRef skey;
    PGPKeyListRef secKeylist;
    PGPKeyListRef bothKeylist;
    PGPError err;
    PGPUInt32 trust;
    PGPBoolean compatible = mainbPtr->envbPtr->compatible;
    char kstr[kPGPMaxKeyIDStringSize];
    char useridstr[ kPGPMaxUserIDSize ];
    PGPContextRef context = mainbPtr->pgpContext;
    PGPKeyID kid;
    PGPBoolean firstUltimatelyTrustedKey = TRUE;
    PGPFilterRef singletonFilter = NULL;

    /* Pass 1: locate ultimately-trusted keys. */

    fprintf(filebPtr->pgpout,
        LANG("\nPass 1: Looking for the \"ultimately-trusted\" keys...\n"));

    err = PGPOrderKeySet( secRingSet, kPGPAnyOrdering, &secKeylist );
    if( IsPGPError(err) )
        return -1;

    err = PGPOrderKeySet( bothRingsSet, kPGPAnyOrdering, &bothKeylist );
    if( IsPGPError(err) )
        return -1;

    err = PGPNewKeyIter( secKeylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNext( keyiter, &skey);
    /*if error, no keys found.*/

    if (err) {
        fprintf(filebPtr->pgpout, LANG("No ultimately-trusted keys.") );
        return err;
    }

    while( skey != NULL )
    {

        err = PGPGetKeyNumber( skey, kPGPKeyPropTrust, &trust);
        pgpAssertNoErr(err);

        if (trust == kPGPKeyTrust_Ultimate) {

            err = pgpGetKeyIDStringCompatFromKey( skey, TRUE, compatible,
                    kstr );

            pgpAssertNoErr(err);
            err =  pgpGetUserIDStringFromKey( skey, useridstr );
            pgpAssertNoErr(err);

            /*  '*' means axiomatically trusted.  */

            fprintf(filebPtr->pgpout, LANG("* %s %s\n") ,kstr,useridstr);

            /* Make filter for depth 0 (=ultimately-trusted) keys. */

            err = PGPGetKeyIDFromKey  (skey, &kid);
            pgpAssertNoErr(err);

            if (firstUltimatelyTrustedKey) {

                err = PGPNewKeyIDFilter( context, &kid, depthFilter );
                pgpAssertNoErr(err);
                firstUltimatelyTrustedKey = FALSE;
            }
            else {

                err = PGPNewKeyIDFilter( context, &kid, &singletonFilter );
                pgpAssertNoErr(err);

                err = PGPUnionFilters ( singletonFilter, *depthFilter,
                        depthFilter );

                pgpAssertNoErr(err);
            }
        }
        err = PGPKeyIterNext( keyiter, &skey);

    }

    err = PGPFreeKeyList( bothKeylist );
    pgpAssertNoErr(err);
    err = PGPFreeKeyList( secKeylist );
    pgpAssertNoErr(err);

    return 0;
} /* findUltimatelyTrustedKeys */


PGPError markSignatureChainDepths(struct pgpmainBones *mainbPtr,
        PGPKeySetRef bothRingsSet,
        PGPFilterRef depthFilter)
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPContextRef context = mainbPtr->pgpContext;
    PGPBoolean compatible = mainbPtr->envbPtr->compatible;

    PGPKeyIterRef keyiter;
    PGPKeyID kid;
    PGPKeySetRef depthSet;
    PGPKeyListRef depthList;
    PGPKeyRef key;
    PGPFilterRef signedByMeFilter = NULL;
    PGPFilterRef usedKeysFilter = NULL;
    PGPFilterRef unusedKeysFilter = NULL;
    char kstr[kPGPMaxKeyIDStringSize];
    char useridstr[ kPGPMaxUserIDSize ];

    PGPError err;
    PGPBoolean firstKeyAtThisDepth;
    PGPUInt32 depth = 0;
    PGPUInt32 count;
    PGPUserValue traceValue;
    /* Trace value contains highest depth in sig chain where this key
       appears.  High bit ='visited' flag - indicates if chain has been
       followed from this point. */

    fprintf(filebPtr->pgpout,
            LANG("\nPass 2: Tracing signature chains...\n"));

    usedKeysFilter = depthFilter;
    err = PGPNegateFilter( usedKeysFilter, &unusedKeysFilter );
    pgpAssertNoErr(err);

    err = PGPFilterKeySet( bothRingsSet, depthFilter, &depthSet);
    pgpAssertNoErr(err);

    err = PGPCountKeys( depthSet, &count);
    pgpAssertNoErr(err);

    if (count == 0) {
        fprintf(filebPtr->pgpout, LANG("No ultimately-trusted keys.") );
        return -1;
    }

    while ( TRUE ) {
        firstKeyAtThisDepth = TRUE;
        err = PGPOrderKeySet(depthSet, kPGPAnyOrdering, &depthList);
        pgpAssertNoErr(err);
        err = PGPNewKeyIter( depthList, &keyiter );
        pgpAssertNoErr(err);
        err = PGPKeyIterRewind( keyiter );
        pgpAssertNoErr(err);
        err = PGPKeyIterNext( keyiter, &key);

        while( key != NULL ) {

            err = pgpGetKeyIDStringCompatFromKey( key, TRUE, compatible,
                    kstr );
            pgpAssertNoErr(err);
            err =  pgpGetUserIDStringFromKey( key, useridstr );
            pgpAssertNoErr(err);

            /* Mark its highest depth level on each key. */

            err = PGPGetKeyUserVal( key, &traceValue);
            pgpAssertNoErr(err);

            if ( (PGPUInt32)traceValue == 0) {
                (PGPUInt32)traceValue = depth;

                err = PGPSetKeyUserVal( key, traceValue);
                pgpAssertNoErr(err);
            }

            err = PGPGetKeyIDFromKey( key, &kid );
            pgpAssertNoErr(err);

            if (firstKeyAtThisDepth) {

                err = PGPNewSigKeyIDFilter(context, &kid, &depthFilter);
                pgpAssertNoErr(err);
                firstKeyAtThisDepth = FALSE;
            } else {

                err = PGPNewSigKeyIDFilter(context, &kid, &signedByMeFilter);
                pgpAssertNoErr(err);
                err = PGPUnionFilters( signedByMeFilter, depthFilter,
                        &depthFilter);
                pgpAssertNoErr(err);
            }

            err = PGPKeyIterNext( keyiter, &key);
        }

        err = PGPIntersectFilters(unusedKeysFilter, depthFilter,
                &depthFilter);

        pgpAssertNoErr(err);

        /* Create next depth level. */
        err = PGPFreeKeySet( depthSet );
        pgpAssertNoErr(err);
        err = PGPFilterKeySet( bothRingsSet, depthFilter, &depthSet);
        pgpAssertNoErr(err);
        err = PGPCountKeys( depthSet, &count);
        pgpAssertNoErr(err);

        err = PGPFreeKeyIter( keyiter );
        pgpAssertNoErr(err);

        if (count == 0) return 0;

        err = PGPUnionFilters( depthFilter, usedKeysFilter, &usedKeysFilter);
        pgpAssertNoErr(err);
        err = PGPNegateFilter( usedKeysFilter, &unusedKeysFilter );
        pgpAssertNoErr(err);
        depth ++;
    }
} /* markSignatureChainDepths */


const PGPUInt32 VISITED_MASK = 0x80000000;
const PGPUInt32 DEPTH_MASK = 0x7FFFFFFF;


/*  Print userid of a key's signers. Ignore self-signing.
    Recursive:  Given a node, traverse its children.
    Arguments: key id of parent node, whether parent node has been
    traversed. */

PGPError printNextSignatureChainLink(struct pgpmainBones *mainbPtr,
        PGPKeySetRef bothRingsSet,
        PGPKeyRef key,
        PGPBoolean visited,
        PGPUInt32 currentDepth)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPUserValue traceValue;
    PGPUInt32 keyDepth;
    PGPKeyListRef kidsList;
    PGPKeySetRef kidsSet;
    PGPFilterRef kidsFilter;
    PGPFilterRef singletonFilter;
    PGPFilterRef notParentFilter;
    PGPKeyIterRef keyIter;
    char useridstr[kPGPMaxUserIDSize];
    PGPError err = kPGPError_NoErr;
    PGPKeyID keyID;

    /* For first time at node? Create keyset containing its children. */

    if (!visited)  {

        err = PGPGetKeyIDFromKey( key, &keyID);
        pgpAssertNoErr(err);

        err = PGPNewSigKeyIDFilter(context, &keyID, &kidsFilter);
        pgpAssertNoErr(err);

        err = PGPNewKeyIDFilter( context, &keyID, &singletonFilter );
        pgpAssertNoErr(err);

        err = PGPNegateFilter( singletonFilter, &notParentFilter);
        pgpAssertNoErr(err);

        err = PGPIntersectFilters( notParentFilter, kidsFilter,
                &kidsFilter );

        pgpAssertNoErr(err);


        if (kidsFilter == NULL) return 0;           /* childless node */

        err = PGPFilterKeySet(bothRingsSet, kidsFilter, &kidsSet);
        pgpAssertNoErr(err);

        err = PGPOrderKeySet( kidsSet, kPGPAnyOrdering, &kidsList);
        pgpAssertNoErr(err);

        err = PGPNewKeyIter(kidsList, &keyIter);
        pgpAssertNoErr(err);

        err = PGPKeyIterRewind( keyIter);
        pgpAssertNoErr(err);

        err = PGPKeyIterNext( keyIter, &key);

        if (IsPGPError(err)) {  /* childless */

            PGPFreeKeyIter( keyIter);
            return 0;
        }
    }

    while (key != NULL) {

        err = PGPGetKeyUserVal(key, &traceValue);
        pgpAssertNoErr(err);

        visited = (PGPUInt32)traceValue & VISITED_MASK;
        keyDepth = (PGPUInt32)traceValue & DEPTH_MASK;

        err = pgpGetUserIDStringFromKey(key, useridstr);
        pgpAssertNoErr(err);

        fprintf(filebPtr->pgpout, LANG("%*s"), (2 * currentDepth), " ");
        fprintf(filebPtr->pgpout, LANG("> %s\n"), useridstr);

        if (visited || (keyDepth < currentDepth) )  return 0;

        traceValue = (PGPUserValue)( VISITED_MASK | keyDepth);
        err = PGPSetKeyUserVal(key, traceValue);
        pgpAssertNoErr(err);

        err = PGPGetKeyIDFromKey( key, &keyID);
        pgpAssertNoErr(err);

        err = printNextSignatureChainLink(mainbPtr, bothRingsSet, key,
                visited, currentDepth+1);

        err = PGPKeyIterNext( keyIter, &key );

        if ( IsPGPError(err) ) {    /* traversed all children of this node */

            PGPFreeKeyIter( keyIter);
            return 0;
        }
    }
    return err;
} /* printNextSignatureChainLink */


PGPError printSignatureChains(struct pgpmainBones *mainbPtr,
        PGPKeySetRef bothRingsSet,
        PGPFilterRef depthFilter)
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPKeyListRef depthList;
    PGPKeySetRef depthSet;
    PGPKeyRef key;
    PGPKeyIterRef keyiter;
    PGPKeyID keyID;
    PGPUserValue traceValue;

    PGPError err;
    char useridstr[ kPGPMaxUserIDSize ];
    char nulluseridstr[kPGPMaxUserIDSize];

    nulluseridstr[0] = '\0';

    err = PGPFilterKeySet( bothRingsSet, depthFilter, &depthSet);
    pgpAssertNoErr(err);

    err = PGPOrderKeySet(depthSet, kPGPAnyOrdering, &depthList);
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( depthList, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( keyiter, &key );

    while (key != NULL) {
        err =  pgpGetUserIDStringFromKey( key, useridstr );
        pgpAssertNoErr(err);
        fprintf(filebPtr->pgpout, LANG("* %s\n"), useridstr);

        err = PGPGetKeyIDFromKey( key, &keyID);
        pgpAssertNoErr(err);

        traceValue = (PGPUserValue)( VISITED_MASK | 0 );
        err = PGPSetKeyUserVal(key, traceValue);
        pgpAssertNoErr(err);

        err = printNextSignatureChainLink(mainbPtr, bothRingsSet, key, 0, 1);
        pgpAssertNoErr(err);
        err = PGPKeyIterNext( keyiter, &key );
    }
    return 0;

} /* printSignatureChains */



PGPError traceSignatureChains(struct pgpmainBones *mainbPtr,
        PGPKeySetRef bothRingsSet,
        PGPFilterRef depthFilter)
{
    PGPError err;

    err = markSignatureChainDepths(mainbPtr, bothRingsSet, depthFilter);

    if ( IsntPGPError(err) ) {
        err = printSignatureChains(mainbPtr, bothRingsSet, depthFilter);
    }
    return err;

}  /* traceSignatureChains */


PGPError doMaintenance(struct pgpmainBones *mainbPtr, PGPFileSpecRef
        pubRingFileSpec, char * ringfilename)
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPContextRef context = mainbPtr->pgpContext;
    PGPEnv *env = mainbPtr->envbPtr->m_env;
    PGPBoolean compatible = mainbPtr->envbPtr->compatible;
    PGPUInt32 pri;
    PGPFilterRef depthFilter = NULL;
    PGPKeySetRef bothRingsSet = NULL;
    PGPKeySetRef secRingSet = NULL;
    PGPError err, er2;
    char secringfilename[MAX_PATH];
    PGPFileSpecRef secRingFileSpec = NULL;
    PGPBoolean cantopen = FALSE;

    /* Assume: ringfilename always public */
    err = pgpGetCorrespondingSecretRingName( filebPtr,
                ringfilename, secringfilename);
    if( IsPGPError(err) ) cantopen = TRUE;

    err = PGPNewFileSpecFromFullPath( context, secringfilename,
                &secRingFileSpec);
    pgpAssertNoErr(err);

    err = PGPOpenKeyRing( context, kPGPKeyRingOpenFlags_Private,
            secRingFileSpec, &secRingSet);
    if( IsPGPError(err) ) cantopen = TRUE;

    err = pgpOpenKeyringsFromPubringSpec( mainbPtr, pubRingFileSpec,
	    &bothRingsSet, 0);

    if( IsPGPError(err) ) goto done;
    /* It generates its own error message. */

    if (cantopen) {
        fprintf(filebPtr->pgpout,
                LANG("\nCan't open key ring file '%s'\n"),
                ringfilename);
        goto done;
    }

    /* Give trust a meaningful value. */
    err = PGPCheckKeyRingSigs(secRingSet, bothRingsSet,
                                       TRUE, NULL, NULL);
    pgpAssertNoErr(err);

    /* Pass 1: */
    err = findUltimatelyTrustedKeys(mainbPtr,
            secRingSet, bothRingsSet, &depthFilter );

    pgpAssertNoErr(err);

    /* Pass 2: trace signature chains */
    if ( IsntPGPError(err) ) {

        err = traceSignatureChains(mainbPtr, bothRingsSet, depthFilter);

    }

    /* Pass 3: */

    err = pgpShowKeySetTrustAndValidity( filebPtr, bothRingsSet,
            bothRingsSet);

    pgpAssertNoErr(err);

 done:

    if (bothRingsSet)
        PGPFreeKeySet( bothRingsSet );
    if (secRingSet)
        PGPFreeKeySet( secRingSet );

    if (secRingFileSpec)
        PGPFreeFileSpec(secRingFileSpec);

    if (!compatible && IsPGPError(err) &&
        pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &er2))
        pgpShowError( filebPtr, err, __FILE__,__LINE__);

    return err;
}




