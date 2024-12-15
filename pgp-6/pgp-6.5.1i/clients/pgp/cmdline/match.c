/*____________________________________________________________________________
    match.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    smart searching for key sets.

    $Id: match.c,v 1.9 1999/05/12 21:01:04 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <string.h>

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpKeyServer.h"
#include "pgpUtilities.h"
#include "pgpSDKPrefs.h"

#include "usuals.h"
#include "pgp.h"
#include "globals.h"
#include "prototypes.h"
#include "language.h"

PGPError pgpGetMatchingKeySet(struct pgpmainBones *mainbPtr, const char
        *useridstr, PGPFlags matchFlags, PGPKeySetRef *resultset)
{
    PGPContextRef context = mainbPtr->pgpContext;
    PGPEnv *env = mainbPtr->envbPtr->m_env;
    char srchStr[ kPGPMaxUserIDSize  ];
    PGPError err,er2;
    PGPFilterRef outFilter = NULL;

    pgpAssertAddrValid( mainbPtr, struct pgpmainBones );
    pgpAssertAddrValid( resultset, PGPKeySetRef );

    if( mainbPtr->workingRingSet )
        pgpAssertAddrValid( mainbPtr->workingRingSet, PGPKeySetRef );
    else
        pgpAssertAddrValid( mainbPtr->workingKeyServer, PGPKeyServerRef );

    *resultset = NULL;
    if( !useridstr )
        return kPGPError_BadParams;

    strncpy( srchStr, useridstr, kPGPMaxUserIDSize-1 );
    srchStr[ kPGPMaxUserIDSize-1 ] = 0;

    /* for compatibility with pgp2.6.2, permit matches on ``0X'' as a
       keyID prefix.*/
    if( srchStr[0]=='0' && ( srchStr[1]=='x' || srchStr[1]=='X') ) {
        PGPKeyID kid;
        PGPFilterRef kidFilter,subFilter;
        srchStr[1]='x';
        PGPGetKeyIDFromString( srchStr, &kid );
        err = PGPNewKeyIDFilter( context, &kid, &kidFilter );
        pgpAssertNoErr(err);
        err = PGPNewSubKeyIDFilter( context, &kid, &subFilter);
        pgpAssertNoErr(err);
        err = PGPUnionFilters ( kidFilter, subFilter, &outFilter );
        pgpAssertNoErr(err);

        if( mainbPtr->workingKeyServer != NULL && !(matchFlags &
                kMatch_NotKeyServer) )
            err = PGPQueryKeyServer( mainbPtr->workingKeyServer,
                    outFilter, resultset );
        else
            err = PGPFilterKeySet( mainbPtr->workingRingSet, outFilter,
                    resultset );

        if(err)
            goto done;

    } else {
        PGPFilterRef abledFilter, expyFilter, tmpFilter;
        PGPKeySetRef workingSet;
        PGPInt32 pri;

        err = PGPNewUserIDStringFilter( context, srchStr,
                kPGPMatchSubString, &outFilter );
        if(err)
            goto done;

        if( mainbPtr->workingKeyServer != NULL && !(matchFlags &
                kMatch_NotKeyServer) ) {
            err = PGPQueryKeyServer( mainbPtr->workingKeyServer,
                    outFilter, &workingSet );
            if( IsPGPError(err) && pgpenvGetInt( env, PGPENV_VERBOSE,
                    &pri, &er2 ))
                pgpShowError(mainbPtr->filebPtr, err, __FILE__,__LINE__ );

        } else
            err = PGPFilterKeySet( mainbPtr->workingRingSet, outFilter,
                    &workingSet );

        PGPFreeFilter(outFilter);
        outFilter=NULL;

        if( mainbPtr->workingKeyServer != NULL && !(matchFlags &
                kMatch_NotKeyServer) )
            goto post;

        /* fold in any matches from groups..*/
        if( srchStr[0]!='\0' && mainbPtr->workingGroupSet != NULL ) {
            PGPGroupID group;
            PGPUInt32 numNotFound;

            err = pgpGetGroupByName( mainbPtr->workingGroupSet, srchStr,
                    &group );
            if( IsntPGPError(err) ) {
                PGPKeySetRef tmpSet;
                err = PGPNewKeySetFromGroup( mainbPtr->workingGroupSet,
                        group, mainbPtr->workingRingSet, &tmpSet,
                        &numNotFound );
                pgpAssertNoErr(err);

                if( workingSet ) {
                    err = PGPUnionKeySets( workingSet, tmpSet, resultset );
                    pgpAssertNoErr(err);

                    PGPFreeKeySet( workingSet );
                    PGPFreeKeySet( tmpSet );
                    workingSet = *resultset;
                    *resultset = NULL;
                } else
                    workingSet = tmpSet;
            }
        }

post:
        if( IsPGPError(err) && !workingSet )
            goto done;

        /* Strategy: to post-filter the resulting keyset with the
           modifiers may be much faster than intersecting all filters. */

        if( matchFlags & kMatch_NotDisabled ) {
            err = PGPNewKeyDisabledFilter( context, FALSE, &abledFilter );
            pgpAssertNoErr(err);
            if( outFilter ) {
                err = PGPIntersectFilters( outFilter, abledFilter,
                        &tmpFilter);

                pgpAssertNoErr(err);
                /*PGPFreeFilter(abledFilter);<-autofreed*/
                /*PGPFreeFilter(outFilter);<-autofreed*/
                outFilter = tmpFilter;
            } else
                outFilter = abledFilter;
        }

        if( matchFlags & kMatch_NotExpired ) {
            PGPTime timenow = PGPGetTime();
            err = PGPNewKeyExpirationTimeFilter( context, timenow,
                    kPGPMatchGreaterOrEqual, &expyFilter );
            pgpAssertNoErr(err);
            if( outFilter ) {
                err = PGPIntersectFilters( outFilter, expyFilter,
                        &tmpFilter);

                pgpAssertNoErr(err);
                /*PGPFreeFilter(expyFilter);<-autofreed*/
                /*PGPFreeFilter(outFilter);<-autofreed*/
                outFilter = tmpFilter;
            } else
                outFilter = expyFilter;
        }

        if( outFilter && workingSet ) {
            err = PGPFilterKeySet( workingSet, outFilter, resultset );
            PGPFreeKeySet( workingSet );
        } else {
            *resultset = workingSet;
            err = kPGPError_NoErr;
        }
    }
done:
    if(outFilter)
        PGPFreeFilter(outFilter);
    return err;
}

/*
   Returns: a list of keys associated with the the useridstr.
   if there are no matching keys, return an _empty_ list and succeed.
   if the useridstr is empty, return the whole keyring.
 */
PGPError pgpGetMatchingKeyList(struct pgpmainBones *mainbPtr,
        const char *useridstr, PGPFlags matchFlags,
        PGPKeyListRef *resultlist)
{
    PGPKeySetRef resultset=NULL;
    PGPError err,er2;

    *resultlist = NULL;
    if(useridstr && useridstr[0] != '\0') {

        err = pgpGetMatchingKeySet( mainbPtr, useridstr, matchFlags,
                &resultset );
        if( err == kPGPError_ItemNotFound ) {
            if( resultset == NULL) {
                err = PGPNewEmptyKeySet( mainbPtr->workingRingSet,
                        &resultset );

                pgpAssertNoErr(err);
            } else
                err = kPGPError_NoErr;
        }

        if( IsntPGPError(err) ) /* sort the result set..*/
            err = PGPOrderKeySet( resultset, kPGPAnyOrdering, resultlist );

        er2 = PGPFreeKeySet(resultset);
        pgpAssertNoErr(er2);
    } else {
        err = PGPOrderKeySet( mainbPtr->workingRingSet, kPGPAnyOrdering,
                resultlist );
    }
    return err;
}

/*
   Intelligently build a key set from a list of userid strings for
   encrypting to keys. Arguments are a list of C strings that represent
   substrings of user id's, or strings of key id's or group names.

   Discount disabled or expired keys.

   If, for a given string the key is not found, complain.

   The caller is responsible for freeing the resultant key set.
 */
PGPError pgpBuildKeySetFromUserIDStringList( struct pgpmainBones
        *mainbPtr, const char **ustrList, PGPKeySetRef *resultsetRef )
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPError err,er2;
    const char **r;
    PGPSize nkeys;

    pgpAssertAddrValid( mainbPtr, struct pgpmainBones );
    pgpAssertAddrValid( mainbPtr->workingRingSet, PGPKeySetRef );

    *resultsetRef = NULL;
    err = PGPNewEmptyKeySet( mainbPtr->workingRingSet, resultsetRef );
    pgpAssertNoErr(err);

    /* end of list is null or empty string. */
    for( r = ustrList; *r && **r; r++) {
        PGPKeySetRef filterset;

        /*mainbPtr->workingRingSet*/
        /*mainbPtr->workingGroupSet*/
        err = pgpGetMatchingKeySet(mainbPtr, *r,
                kMatch_NotDisabled | kMatch_NotExpired, &filterset );
        if ( IsPGPError(err) )
            break;

        PGPCountKeys( filterset, &nkeys );
        if( nkeys < 1 ) {
            fprintf( filebPtr->pgpout,
LANG("\nCannot find the public key matching userid '%s'\n"), *r );

            fprintf( filebPtr->pgpout,
LANG("This user will not be able to decrypt this message.\n"));
        } else {
            err = pgpUnionKeysAskInvalid( filebPtr, filterset,
                    *resultsetRef, resultsetRef );
            pgpAssertNoErr(err);
        }

        er2 = PGPFreeKeySet( filterset );
        pgpAssertNoErr(er2);
    }

    pgpAssertAddrValid( *resultsetRef, PGPKeySetRef );
    PGPCountKeys( *resultsetRef, &nkeys );
    if( nkeys < 1 )
        err = kPGPError_ItemNotFound;

    if( IsPGPError(err) ) {
        PGPError er2;
        er2 = PGPFreeKeySet( *resultsetRef );
        pgpAssertNoErr(er2);
        *resultsetRef = NULL;
    }

    return err;
}

PGPError pgpGetMySigningKey( struct pgpmainBones *mainbPtr,
        PGPKeyRef *myKey )
{
    PGPBoolean cansign = FALSE;
    PGPBoolean isdisabled = FALSE;
    PGPKeySetRef myset = NULL;
    PGPKeyListRef mylist = NULL;
    PGPKeyIterRef myiter = NULL;
    PGPEnv *env = mainbPtr->envbPtr->m_env;
    PGPError err;
    PGPInt32 pri;
    const char *myName = pgpenvGetCString( env, PGPENV_MYNAME, &pri );

	if(myName && myName[0] == '\0')
	{
		/* use default signing key */
		err = PGPGetDefaultPrivateKey(mainbPtr->workingRingSet, myKey);
		if(IsPGPError(err))
	        return kPGPError_SecretKeyNotFound;
		return kPGPError_NoErr;
	}

    err = pgpGetMatchingKeySet(mainbPtr, myName,
            kMatch_NotDisabled | kMatch_NotExpired | kMatch_NotKeyServer,
            &myset );

    pgpAssertNoErr(err);
    err = PGPOrderKeySet( myset, kPGPAnyOrdering, &mylist );
    pgpAssertNoErr(err);

    err = PGPNewKeyIter( mylist, &myiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( myiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( myiter, myKey);

    if( IsPGPError(err))
        goto done;

    while( *myKey != NULL )  {
        /* simply choose the first key with the needed property...*/
        /* matches MYNAME and is not disabled and is a signing key*/

        err = PGPGetKeyBoolean( *myKey, kPGPKeyPropIsDisabled, &isdisabled );
        pgpAssertNoErr(err);
        if( isdisabled )
            goto next;

        err = PGPGetKeyBoolean( *myKey, kPGPKeyPropCanSign, &cansign );
        pgpAssertNoErr(err);
        if( cansign )
            break;

next:
        err = PGPKeyIterNext( myiter, myKey);
    }

done:
    if(myiter)
        PGPFreeKeyIter( myiter );
    if(mylist)
        PGPFreeKeyList( mylist );
    if(myset)
        PGPFreeKeySet( myset );

    if( !cansign ) {
        *myKey = NULL;
        return kPGPError_SecretKeyNotFound;
    }

    return kPGPError_NoErr;
}


#include <ctype.h>

/*
   test whether argument name is a URL...
 */
PGPBoolean pgpLocationIsURL( const char *name )
{
    PGPSize i;

    /* expecting a valid string, long enough to be a URL*/

    if( !name || !name[0] || !name[1] || !name[2] )
        return FALSE;


    /* expecting //hostname but not /name and not /// */

    if( name[0]=='/' && name[1]!='/' )
        return FALSE;
    if( name[0]=='/' && name[1]=='/' && isalnum(name[2]) )
        return TRUE;
    if( name[0]=='/' && name[1]=='/' && name[2] == '/' )
        return FALSE;


    /* expecting protocol://hostname */

    for( i=0; name[i] && isalnum( name[i] ); i++)
        ;

    if( !name[i] || name[i]!=':' || name[i+1]!='/' || name[i+2]!='/' ||
            !isalnum( name[i+3] ) )
        return FALSE;

    return TRUE;
}

PGPError 
pgpGetSigningKey( struct pgpmainBones *mainbPtr,
        PGPKeyRef	*pKey )
{

    PGPError		err = kPGPError_NoErr;
	PGPContextRef	context = mainbPtr->pgpContext;
	struct pgpfileBones *filebPtr = mainbPtr->filebPtr; 
	PGPKeySetRef	workingSet = kPGPInvalidRef;
	PGPFilterRef	filter = kPGPInvalidRef;
	PGPKeyListRef	list = kPGPInvalidRef;
	PGPKeyIterRef	keyiter = kPGPInvalidRef;
	PGPBoolean		bFound = FALSE;
	PGPBoolean		bCanSign = FALSE;
	PGPSize			len = 0;
    PGPKeyID        keyid;
        

    err = PGPGetKeyIDFromString(mainbPtr->plainfilename, &keyid);
    pgpAssertNoErr(err);
    err = PGPNewKeyIDFilter(context, &keyid, &filter);
	pgpAssertNoErr(err);

	err = PGPFilterKeySet(mainbPtr->workingRingSet, filter, &workingSet);
	pgpAssertNoErr(err);

    err = PGPOrderKeySet( workingSet, kPGPAnyOrdering, &list );
    pgpAssertNoErr(err);

    err = PGPNewKeyIter( list, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNext( keyiter, pKey);

    if( IsPGPError(err))
        goto done;

    while( *pKey != NULL && IsntPGPError(err))  {
        /* simply choose the first key with the needed property...*/
        /* matches MYNAME and is not disabled and is a signing key*/

        err = PGPGetKeyBoolean( *pKey, kPGPKeyPropCanSign, &bCanSign);
        pgpAssertNoErr(err);
        if( bCanSign )
		{
            bFound = TRUE;
            break;
		}
        err = PGPKeyIterNext( keyiter, pKey);
    }

done: 
    if(filter != kPGPInvalidRef)
        PGPFreeFilter(filter);
    if(keyiter != kPGPInvalidRef)
        PGPFreeKeyIter( keyiter );
    if(list != kPGPInvalidRef)
        PGPFreeKeyList( list );
    if(workingSet != kPGPInvalidRef)
        PGPFreeKeySet( workingSet );
    if(!bFound) 
    {
        pKey = kPGPInvalidRef;
        return kPGPError_SecretKeyNotFound;
    }
    return kPGPError_NoErr;
}

