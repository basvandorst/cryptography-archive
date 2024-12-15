/*____________________________________________________________________________
    keyview.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    use the PGP SDK to view the keyring and report in PGP262 style.

    $Id: keyview.c,v 1.15 1999/05/21 20:38:29 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <string.h>

#include "pgpBase.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpKeyServer.h"
#include "pgpUtilities.h"

#include "usuals.h"
#include "pgp.h"
#include "globals.h"
#include "prototypes.h"
#include "language.h"

void pgpShowError( struct pgpfileBones *filebPtr, PGPError err, char
        *file, int line )
{
#if PGP_DEBUG
    char buffer[256];
    buffer[0] = 0;
    PGPGetErrorString(err, 255, buffer);
    buffer[255]=0;
    fprintf(filebPtr->pgpout,LANG("\nError: %s\n"),buffer);
    if(file)
        fprintf(filebPtr->pgpout,LANG("at %s:%d\n"),file,line);
#endif 
}

static const char *pgpTrustString( PGPInt32 trust )
{
    switch( trust )
    {
        case kPGPKeyTrust_Undefined: return LANG("undefined");
        case kPGPKeyTrust_Unknown:   return LANG("unknown");
        case kPGPKeyTrust_Never:     return LANG("untrusted");
        case kPGPKeyTrust_Marginal:  return LANG("marginal");
        case kPGPKeyTrust_Complete:  return LANG("complete");
        case kPGPKeyTrust_Ultimate:  return LANG("ultimate");
        default:                     return LANG("????????");
    }
}

static const char *pgpValidityString( PGPInt32 validity )
{
    switch( validity )
    {
        case kPGPValidity_Unknown:   return LANG("unknown");
        case kPGPValidity_Invalid:   return LANG("invalid");
        case kPGPValidity_Marginal:  return LANG("marginal");
        case kPGPValidity_Complete:  return LANG("complete");
        default:                     return LANG("????????");
    }
}

static const char *pgpPKAlgIDString( PGPInt32 algID )
{
    switch( algID ) {
        case kPGPPublicKeyAlgorithm_RSA:
            return LANG("RSA");
        case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
            return LANG("RSA encrypt");
        case kPGPPublicKeyAlgorithm_RSASignOnly:
            return LANG("RSA sign");
        case kPGPPublicKeyAlgorithm_ElGamal:
            return LANG("DH");
        case kPGPPublicKeyAlgorithm_DSA:
            return LANG("DSS");
        default:
            return LANG("Unknown type");
    }
}

PGPError pgpGetKeyPKAlgIDString( PGPKeyRef key, char **str )
{
    PGPError err;
    PGPInt32 algID;

    err = PGPGetKeyNumber( key, kPGPKeyPropAlgID, &algID );

    *str = (char *)pgpPKAlgIDString( algID );
    return err;
}

PGPError pgpGetKeyTrustString( PGPKeyRef key, char **str )
{
    PGPInt32 trust;
    PGPError err;
    err = PGPGetKeyNumber( key, kPGPKeyPropTrust, &trust);
    pgpAssertNoErr(err);
    *str = (char *)pgpTrustString( trust );
    return err;
}

PGPError pgpGetKeyValidityString( PGPKeyRef key, char **str )
{
    PGPInt32 validity;
    PGPError err;
    err = PGPGetKeyNumber( key, kPGPKeyPropValidity, &validity);
    pgpAssertNoErr(err);
    *str = (char *)pgpValidityString( validity );
    return err;
}

PGPError pgpShowKeyTrust( struct pgpfileBones *filebPtr, PGPKeyRef key )
{
    PGPInt32 trust;
    PGPError err;

    err = PGPGetKeyNumber( key, kPGPKeyPropTrust, &trust);
    pgpAssertNoErr(err);

    switch( trust ) {
        case kPGPKeyTrust_Never:
            fprintf(filebPtr->pgpout,
LANG("This user is untrusted to certify other keys.\n"));
            break;
        case kPGPKeyTrust_Marginal:
            fprintf(filebPtr->pgpout,
LANG("This user is generally trusted to certify other keys.\n"));
            break;
        case kPGPKeyTrust_Complete:
            fprintf(filebPtr->pgpout,
LANG("This user is completely trusted to certify other keys.\n"));
            break;
        case kPGPKeyTrust_Ultimate:
            fprintf(filebPtr->pgpout,
LANG("This axiomatic key is ultimately trusted to certify other keys.\n"));
            break;
        default:
            /* Just don't say anything in this case */
            break;

    }
    return err;
}

PGPError pgpShowKeyValidity( struct pgpfileBones *filebPtr, PGPKeyRef key )
{
    PGPInt32 validity;
    PGPError err;
    err = PGPGetKeyNumber( key, kPGPKeyPropValidity, &validity);
    pgpAssertNoErr(err);
    switch( validity ) {
        case kPGPValidity_Invalid:
            fprintf(filebPtr->pgpout,
LANG("This key/userID association is not certified.\n"));
            break;
        case kPGPValidity_Marginal:
            fprintf(filebPtr->pgpout,
LANG("This key/userID association is marginally certified.\n"));
            break;
        case kPGPValidity_Complete:
            fprintf(filebPtr->pgpout,
LANG("This key/userID association is fully certified.\n"));
            break;
        default:
            /* Just don't say anything in this case */
            break;
    }
    return err;
}

#if 0
 /* for debugging, what are we looking at?? */
static void pgpShowCertDebug( struct pgpfileBones *filebPtr, PGPSigRef cert )
{
    PGPBoolean boo;
    PGPInt32 prop;

    PGPGetSigBoolean( cert, kPGPSigPropIsRevoked, &boo );
    fprintf( filebPtr->pgpout, "IsRevoked: %d\n", boo );

    PGPGetSigBoolean( cert, kPGPSigPropIsNotCorrupt, &boo );
    fprintf( filebPtr->pgpout, "IsNotCorrupt: %d\n", boo );

    PGPGetSigBoolean( cert, kPGPSigPropIsTried, &boo );
    fprintf( filebPtr->pgpout, "IsTried: %d\n", boo );

    PGPGetSigBoolean( cert, kPGPSigPropIsVerified, &boo );
    fprintf( filebPtr->pgpout, "IsVerified: %d\n", boo );

    PGPGetSigBoolean( cert, kPGPSigPropIsMySig, &boo );
    fprintf( filebPtr->pgpout, "IsMySig: %d\n", boo );

    PGPGetSigBoolean( cert, kPGPSigPropIsExportable, &boo );
    fprintf( filebPtr->pgpout, "IsExportable: %d\n", boo );

    PGPGetSigBoolean( cert, kPGPSigPropHasUnverifiedRevocation, &boo );
    fprintf( filebPtr->pgpout, "HasUnverifiedRevocation: %d\n", boo );

    PGPGetSigBoolean( cert, kPGPSigPropIsExpired, &boo );
    fprintf( filebPtr->pgpout, "IsExpired: %d\n", boo );

    PGPGetSigBoolean( cert, kPGPSigPropIsX509, &boo );
    fprintf( filebPtr->pgpout, "IsX509: %d\n", boo );


    PGPGetSigNumber( cert, kPGPSigPropAlgID, &prop );
    fprintf( filebPtr->pgpout, "AlgID: %d\n", prop );

    PGPGetSigNumber( cert, kPGPSigPropTrustLevel, &prop );
    fprintf( filebPtr->pgpout, "TrustLevel: %d\n", prop );

    PGPGetSigNumber( cert, kPGPSigPropTrustValue, &prop );
    fprintf( filebPtr->pgpout, "TrustValue: %d\n", prop );
}

 /* show in PGP 262 format */
static void pgpShowCert( struct pgpfileBones *filebPtr, PGPSigRef cert )
{
    /* String properties */
    kPGPSigPropKeyID                    = 120,
    kPGPSigPropX509Certificate          = 121,

    /* Number properties */
    kPGPSigPropAlgID                    = 140,
    kPGPSigPropTrustLevel               = 141,
    kPGPSigPropTrustValue               = 142,

    /* Time properties */
    kPGPSigPropCreation                 = 160,
    kPGPSigPropExpiration               = 161,

    /* PGPBoolean properties */
    kPGPSigPropIsRevoked                = 180,
    kPGPSigPropIsNotCorrupt             = 181,
    kPGPSigPropIsTried                  = 182,
    kPGPSigPropIsVerified               = 183,
    kPGPSigPropIsMySig                  = 184,
    kPGPSigPropIsExportable             = 185,
    kPGPSigPropHasUnverifiedRevocation  = 186,
    kPGPSigPropIsExpired                = 187,
    kPGPSigPropIsX509                   = 188,
}
#endif

 /* create a key iter from a singleton key, for iterating thru the userids.
    output: position the iter at the key.
  */
PGPError pgpNewUserIDIterFromKey( PGPKeyRef key, PGPKeyIterRef *newiter)
{
    PGPKeySetRef single = NULL;
    PGPKeyListRef keylist = NULL;
    PGPError err;

    err = PGPNewSingletonKeySet( key, &single );
    if( IsPGPError(err) )
        goto out;

    err = PGPOrderKeySet( single, kPGPAnyOrdering, &keylist );
    if( IsPGPError(err) )
        goto out;

    err = PGPNewKeyIter( keylist, newiter );
    if( IsPGPError(err) )
        goto out;

    /* it may not be correctly positioned, we have to rewind and next..*/
    err = PGPKeyIterRewind( *newiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNext( *newiter, &key);

    if( err == kPGPError_EndOfIteration )
        err = kPGPError_NoErr;
out:
    if(keylist)
        PGPFreeKeyList( keylist );
    if(single)
        PGPFreeKeySet( single );
    return err;
}

PGPError pgpGetKeyIDStringCompat( PGPKeyID const *kidPtr, PGPBoolean
        abbrev, PGPBoolean compat, char *kidstr )
{
    PGPError err;
    char kstr[kPGPMaxKeyIDStringSize];

    err = PGPGetKeyIDString( kidPtr, abbrev ? kPGPKeyIDString_Abbreviated
            : kPGPKeyIDString_Full, kstr );

    pgpAssertNoErr(err);

    if( compat && kstr[0]=='0' && kstr[1]=='x' )
        strcpy( kidstr, &kstr[2] );
    else
        strcpy( kidstr, kstr );
    return err;
}

PGPError pgpGetKeyIDStringCompatFromKey( PGPKeyRef key, PGPBoolean abbrev,
        PGPBoolean compat, char *kidstr )
{
    PGPError err;
    PGPKeyID kid;

    err = PGPGetKeyIDFromKey( key, &kid );
    pgpAssertNoErr(err);

    err = pgpGetKeyIDStringCompat( &kid, abbrev, compat, kidstr );
    return err;
}

PGPError pgpGetUserIDStringFromSig(PGPKeySetRef ringSet, PGPSigRef sig,
        PGPBoolean compat, char *useridstr )

{
    PGPError err;
    PGPSize actual;
    PGPKeyID kid;
    char kstr[kPGPMaxKeyIDStringSize];
    PGPKeyRef key;

    /* print the certifier...*/
    err = PGPGetSigCertifierKey( sig, ringSet, &key );

    if( IsntPGPError(err) ) {
        PGPUserIDRef userid;

        err = PGPGetPrimaryUserID( key, &userid);
        pgpAssertNoErr(err);
        err =  PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName,
                kPGPMaxUserIDSize, useridstr, &actual );

        pgpAssertNoErr(err);

        /*PGPFreeUserID( userid );*/

        return kPGPError_NoErr;
    }

    err = PGPGetKeyIDOfCertifier( sig, &kid );
    pgpAssertNoErr(err);

    err = pgpGetKeyIDStringCompat( &kid, TRUE, compat, kstr );
    pgpAssertNoErr(err);
    sprintf( useridstr, "(KeyID: %s)", kstr);

    return kPGPError_ItemNotFound;
}

PGPError pgpGetUserIDStringFromKey( PGPKeyRef key, char *useridstr )
{
    PGPUserIDRef userid;
    PGPSize actual;
    PGPError err;

    err = PGPGetPrimaryUserID( key, &userid);
    pgpAssertNoErr(err);
    err = PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName,
            kPGPMaxUserIDSize, useridstr, &actual );

    pgpAssertNoErr(err);
    return err;
}

PGPError pgpShowKeyUserID( struct pgpfileBones *filebPtr, PGPKeyRef key )
{
    char useridstr[ kPGPMaxUserIDSize ];
    PGPError err;

    err = pgpGetUserIDStringFromKey( key, useridstr );

    fprintf( filebPtr->pgpout, LANG("\nKey for user ID \"%s\"\n"),
            useridstr );
    return err;
}


/*
  equivalent to PGP262's show_key(f,pos,0);
 */

PGPError pgpShowKeyBrief( struct pgpfileBones *filebPtr, PGPKeyRef key )
{
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPUInt32 numbits,subbits,algorithm;
    PGPTime creation;
    PGPUInt16 yyyy, mm, dd;
    char kstr[kPGPMaxKeyIDStringSize];
    PGPUserIDRef userid, alias;
    char useridstr[ kPGPMaxUserIDSize ], aliasstr[ kPGPMaxUserIDSize ];
    PGPKeyIterRef uiter;
    PGPSize actual;
    PGPError err;
    PGPBoolean compatible = envbPtr->compatible;

    /* what number of bits?*/
    err = PGPGetKeyNumber( key, kPGPKeyPropBits, &numbits );
    pgpAssertNoErr(err);

    err = pgpNewUserIDIterFromKey( key, &uiter );
    pgpAssertNoErr(err);

    /* what number of bits for the subkey(s)?... show the first one only.*/
    subbits=0;
    PGPGetKeyNumber(key, kPGPKeyPropAlgID, &algorithm);
    if( algorithm == kPGPPublicKeyAlgorithm_DSA )
    {
        PGPSubKeyRef subKey;
        PGPKeyIterSeek( uiter, key);
        err = PGPKeyIterNextSubKey(uiter, &subKey);
        if( IsntPGPError( err ) )
        {
            err = PGPGetSubKeyNumber(subKey, kPGPKeyPropBits, &subbits);
            pgpAssertNoErr(err);
        }
    }

    err = pgpGetKeyIDStringCompatFromKey( key, TRUE, compatible, kstr );
    pgpAssertNoErr(err);

    /* when was it created?*/
    err = PGPGetKeyTime ( key, kPGPKeyPropCreation, &creation );
    pgpAssertNoErr(err);
    PGPGetYMDFromPGPTime( creation, &yyyy, &mm, &dd);

    err = PGPGetPrimaryUserID( key, &userid);
    pgpAssertNoErr(err);
    err = PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName,
            kPGPMaxUserIDSize, useridstr, &actual );

    pgpAssertNoErr(err);

    fprintf( filebPtr->pgpout, LANG("\nKey for user ID: %s\n"), useridstr );
    if(compatible)
        fprintf( filebPtr->pgpout,
        LANG("%d-bit key, Key ID %s, created %4d/%02d/%02d"),
        numbits, kstr, yyyy, mm, dd );
    else {
        char *algstr;
        err = pgpGetKeyPKAlgIDString( key, &algstr );
        fprintf( filebPtr->pgpout,
        LANG("%d-bit %s key, Key ID %s, created %4d/%02d/%02d"),
        numbits, algstr, kstr, yyyy, mm, dd );
    }

    if(!compatible)
    {
        PGPTime expyTime;
        PGPBoolean isexp;
        PGPGetKeyBoolean( key, kPGPKeyPropIsExpired, &isexp );
        if(isexp) {
            fprintf( filebPtr->pgpout, LANG(". Key is expired"));
        } else {
            err = PGPGetKeyTime( key, kPGPKeyPropExpiration, &expyTime);
            pgpAssertNoErr(err);
            if(expyTime != kPGPExpirationTime_Never) {
                PGPGetYMDFromPGPTime( expyTime, &yyyy, &mm, &dd);
                fprintf( filebPtr->pgpout, LANG(", expires %4d/%02d/%02d"),
                        yyyy, mm, dd );
            }
        }
    }
    fprintf( filebPtr->pgpout, "\n");

    { /* print if key format is corrupted*/
        PGPBoolean isok;
        PGPGetKeyBoolean( key, kPGPKeyPropIsNotCorrupt, &isok );
        if( !isok )
            fprintf(filebPtr->pgpout, LANG("Bad key format\n"));
    }
    { /* print if it is revoked*/
        PGPBoolean isrev;
        PGPGetKeyBoolean( key, kPGPKeyPropIsRevoked, &isrev );
        if( isrev )
            fprintf(filebPtr->pgpout, LANG("Key has been revoked\n"));
    }
    { /* print if it is disabled*/
        PGPBoolean isdis;
        PGPGetKeyBoolean( key, kPGPKeyPropIsDisabled, &isdis );
        if( isdis )
            fprintf(filebPtr->pgpout, LANG("Key is disabled\n"));
    }
    if( !compatible )
    {
        PGPBoolean cansign,canencrypt;
        PGPBoolean x=FALSE;
        err = PGPGetKeyBoolean( key, kPGPKeyPropCanEncrypt, &canencrypt );
        pgpAssertNoErr(err);
        err = PGPGetKeyBoolean( key, kPGPKeyPropCanSign, &cansign );
        pgpAssertNoErr(err);
        if( cansign ) {
            fprintf( filebPtr->pgpout, LANG("Key can sign. "));
            x=TRUE;
        }
        if( !canencrypt ) {
            fprintf( filebPtr->pgpout,
                    LANG("Users cannot encrypt to this key. "));
            x=TRUE;
        }
        if(x)
            fprintf( filebPtr->pgpout, "\n");
    }


    err = PGPKeyIterRewindUserID( uiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNextUserID( uiter, &alias);
    pgpAssertNoErr(err);
    if( alias ) {
        while( alias ) {
            err = PGPGetUserIDStringBuffer( alias,
                    kPGPUserIDPropName, kPGPMaxUserIDSize,
                    aliasstr, &actual );
            pgpAssertNoErr(err);

            if( strcmp( useridstr, aliasstr ) !=0 )
                fprintf( filebPtr->pgpout, LANG("Also known as: %s\n"),
                        aliasstr);
            err = PGPKeyIterNextUserID( uiter, &alias );
            /* if err, there are no more*/
        }
    }
    if(uiter)
        PGPFreeKeyIter( uiter );

    return kPGPError_NoErr;
}

/*
   Look for the sig in the target key.
   returns: *outSig
        is the sig found. if not found, *outSig will be NULL.
 */
PGPError pgpKeyFindSig( PGPKeyRef key, PGPSigRef srchSig, PGPSigRef *outSig)
{
    PGPUserIDRef alias;
    PGPKeyIterRef uiter;
    PGPKeyID srchKID;
    PGPError err;
    PGPTime srchTime;

    *outSig = NULL;

    err = PGPGetKeyIDOfCertifier( srchSig, &srchKID );
    pgpAssertNoErr(err);
    err = PGPGetSigTime( srchSig, kPGPSigPropCreation, &srchTime );
    pgpAssertNoErr(err);

    err = pgpNewUserIDIterFromKey( key, &uiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterRewindUserID( uiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNextUserID( uiter, &alias);
    pgpAssertNoErr(err);

    while( alias ) {
        PGPSigRef sig;
        PGPKeyID kid;

        err = PGPKeyIterRewindUIDSig( uiter );
        pgpAssertNoErr(err);
        err = PGPKeyIterNextUIDSig( uiter, &sig );
        pgpAssertNoErr(err);
        while( sig ) {
            PGPTime sigTime;
            err = PGPGetKeyIDOfCertifier( sig, &kid );
            pgpAssertNoErr(err);
            err = PGPGetSigTime(sig, kPGPSigPropCreation, &sigTime );
            pgpAssertNoErr(err);

            if( memcmp( &kid, &srchKID, sizeof( PGPKeyID )) && sigTime ==
                    srchTime )
            {
                *outSig = sig;
                return kPGPError_NoErr;
            }
            err = PGPKeyIterNextUIDSig( uiter, &sig );
            /*if error, no more sigs.*/
        }
        err = PGPKeyIterNextUserID( uiter, &alias );
        /* if err, there are no more*/
    }
    PGPFreeKeyIter( uiter );
    return err;
}

PGPError pgpShowKeyChanges( struct pgpfileBones *filebPtr, PGPKeyRef
        existingkey, PGPKeyRef newkey )
{
    /* equivalent to show_key(f,pos, SHOW_CHANGE); prints the differences
    between the key and the one seen on the keyring*/

    /*PGPSigRef sig;*/

    /*
       The SDK offers no easy way to search for or compare certs to one
        another
     */

    fprintf( filebPtr->pgpout, "pgpShowKeyChanges() stub\n" );

#if 0
    err = PGPKeyIterRewindUIDSig( iterContext );
    pgpAssertNoErr(err);
    err = PGPKeyIterNextUIDSig( iterContext, &sig );
    pgpAssertNoErr(err);
    while( sig ) {

    err = pgpKeyFindSig( key, PGPSigRef srchSig, PGPSigRef *outSig);
    }
#endif

    return kPGPError_NoErr;
}

/*
   equivalent to show_key(f,pos, SHOW_TRUST|SHOW_SIGS)
   and show_key(f,pos, SHOW_TRUST|SHOW_SIGS|SHOW_HASH)
 */
PGPError pgpShowKeyTrustAndValidity( struct pgpfileBones *filebPtr,
        PGPKeySetRef ringSet, PGPKeyIterRef iterContext, PGPKeyRef key,
        PGPBoolean showFingerprints )
{
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPUInt32 trust;
    PGPUInt32 validity;
    char kstr[kPGPMaxKeyIDStringSize];
    PGPUserIDRef userid;
    char useridstr[ kPGPMaxUserIDSize ];
    PGPSize actual;
    PGPKeyRef skey = NULL;
    PGPSigRef sig;
    PGPError err;
    PGPBoolean mine;
    PGPBoolean compatible = envbPtr->compatible;
    PGPSize keyIDLength;

    PGPSize trustLength = 9;
    PGPSize validityLength = 9;
    keyIDLength = ( compatible ? 8 : 10 );

    err = PGPGetKeyBoolean(key, kPGPKeyPropIsAxiomatic, &mine);
    pgpAssertNoErr(err);

    err = PGPGetKeyNumber( key, kPGPKeyPropTrust, &trust);
    pgpAssertNoErr(err);
    err = PGPGetKeyNumber(key, kPGPKeyPropValidity, &validity);
    pgpAssertNoErr(err);

    err = pgpGetKeyIDStringCompatFromKey( key, TRUE, compatible, kstr );
    pgpAssertNoErr(err);

    err = PGPGetPrimaryUserID( key, &userid);
    pgpAssertNoErr(err);

    /* print the key's stats */
    fprintf(filebPtr->pgpout, "%c ", mine ? '*' : ' ');
    fprintf( filebPtr->pgpout,"%s ", LANG(kstr));

    fprintf( filebPtr->pgpout,"%-*s ", trustLength,
            pgpTrustString( trust ) );

    /* list all userids associated with the key. */
    err = PGPKeyIterRewindUserID( iterContext );
    pgpAssertNoErr(err);
    err = PGPKeyIterNextUserID( iterContext, &userid);
    pgpAssertNoErr(err);

    while( userid ) {
        err = PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName,
                kPGPMaxUserIDSize, useridstr, &actual );
        pgpAssertNoErr(err);
        fprintf( filebPtr->pgpout,"%-*s ", validityLength,
                pgpValidityString( validity ) );
        fprintf( filebPtr->pgpout,"%s\n", useridstr );

        /* get sigs */
        err = PGPKeyIterRewindUIDSig( iterContext );
        pgpAssertNoErr(err);
        err = PGPKeyIterNextUIDSig( iterContext, &sig );
        /*if err, there are none.*/

        /* for each signature */
        while( sig ) {

            /* print the certifier...*/
            err = pgpGetUserIDStringFromSig( ringSet, sig, compatible,
                    useridstr );

            if( IsntPGPError(err) ) {
                err = PGPGetSigCertifierKey( sig, ringSet, &skey );
                pgpAssertNoErr(err);

                err = PGPGetKeyNumber( skey, kPGPKeyPropTrust, &trust);
                pgpAssertNoErr(err);
                err = PGPGetKeyNumber( skey, kPGPKeyPropValidity, &validity);
                pgpAssertNoErr(err);

            } else if( err == kPGPError_ItemNotFound ) {
                trust = kPGPKeyTrust_Undefined;
                validity = kPGPValidity_Unknown;
            } else
                pgpAssertNoErr(err);

            fprintf( filebPtr->pgpout, "%c ",
                    (trust >= kPGPKeyTrust_Marginal && validity >=
                    kPGPValidity_Complete) ? 'c' : ' ');

            fprintf( filebPtr->pgpout, "%-*s ", keyIDLength,"");
            fprintf( filebPtr->pgpout, "%-*s ", trustLength,
                    pgpTrustString( trust ));

            fprintf( filebPtr->pgpout, "%-*s ", validityLength,"");
            fprintf( filebPtr->pgpout, " %-s\n", useridstr );

            err = PGPKeyIterNextUIDSig( iterContext, &sig );

        } /* end for each sig */


        err = PGPKeyIterNextUserID( iterContext, &userid);
        /*if err, no more userids for this key*/
        if(userid)
            fprintf( filebPtr->pgpout, "  %-*s %-*s ",
                    keyIDLength, "", trustLength, "");

    }

    if( err == kPGPError_EndOfIteration )
        err = 0;

    if(showFingerprints)
        pgpShowKeyFingerprint( filebPtr, key );

    return err;
}

static void pgpGetSigStatusChar( PGPSigRef sig, char *prop )
{
    PGPError err;
    PGPBoolean boo;
    /**prop = ' ';*/
    /* return based on some state ??*/

    err = PGPGetSigBoolean( sig, kPGPSigPropIsTried, &boo );
    pgpAssertNoErr(err);
    if(boo) {
        err = PGPGetSigBoolean( sig, kPGPSigPropIsVerified, &boo );
        pgpAssertNoErr(err);
        if(boo)
            *prop = '!';
        else
            *prop = '*';

        err = PGPGetSigBoolean( sig, kPGPSigPropIsNotCorrupt, &boo );
        pgpAssertNoErr(err);
        if(!boo)
            *prop = '%';
    } else
        *prop = '?';

}

PGPError pgpShowKeySig( struct pgpfileBones *filebPtr, PGPKeySetRef
        keyringset, PGPSigRef sig, PGPFlags showFlags )
{
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPError err;
    PGPKeyRef skey;
    PGPKeyID kid;
    char kstr[kPGPMaxKeyIDStringSize];
    PGPUserIDRef userid;
    char useridstr[ kPGPMaxUserIDSize ];
    char prop = ' ';
    PGPSize actual;
    PGPBoolean compatible = envbPtr->compatible;

    /* print the certifier...*/
    err = PGPGetSigCertifierKey( sig, keyringset, &skey );

    if( IsntPGPError(err) ) {
        err = PGPGetKeyIDFromKey( skey, &kid );
        pgpAssertNoErr(err);
        err = PGPGetPrimaryUserID( skey, &userid);
        pgpAssertNoErr(err);
        err =  PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName,
                256, useridstr, &actual );

        pgpAssertNoErr(err);
    } else {
        err = PGPGetKeyIDOfCertifier( sig, &kid );
        pgpAssertNoErr(err);
        strcpy(useridstr, LANG("(Unknown signator, can't be checked)"));
    }

    err = pgpGetKeyIDStringCompat( &kid, TRUE, compatible, kstr );
    pgpAssertNoErr(err);

    if( showFlags & kShow_Checks )
        pgpGetSigStatusChar( sig, &prop );

    if( !compatible )
    fprintf( filebPtr->pgpout, "%s%c           %s             %s\n",
            "sig", prop, kstr, useridstr );
    else
    fprintf( filebPtr->pgpout, "%s%c      %s             %s\n",
            "sig", prop, kstr, useridstr );

    return err;
}

PGPError pgpShowKeyFingerprint( struct pgpfileBones *filebPtr,
        PGPKeyRef key )
{
    PGPError err;
    PGPSize actual;
    char h1[30], h2[30], h3[15];
    unsigned char fp[20];

    err = PGPGetKeyPropertyBuffer ( key, kPGPKeyPropFingerprint, 20,
            &fp, &actual);
    pgpAssertNoErr(err);
    sprintf( h1, "  %02x %02x %02x %02x %02x %02x %02x %02x",
            fp[0],fp[1],fp[2],fp[3],fp[4],fp[5],fp[6],fp[7] );
    strupr( h1 );
    sprintf( h2, "  %02x %02x %02x %02x %02x %02x %02x %02x",
            fp[8],fp[9],fp[10],fp[11],fp[12],fp[13],fp[14],fp[15]);
    strupr( h2 );
    if(actual > 16) {
        sprintf( h3, "  %02x %02x %02x %02x",fp[16],fp[17],fp[18],fp[19] );
        strupr( h3 );
    } else
        h3[0] = '\0';
    fprintf( filebPtr->pgpout,
            LANG("          Key fingerprint =%s%s%s\n"), h1, h2, h3 );
    return err;
}


/* show the key properties, but not the userid's yet. */
PGPError pgpShowKeyPropertiesListFormat( struct pgpfileBones *filebPtr,
        PGPKeyRef key, PGPKeyIterRef iterContext, char *kstr, char schar,
        PGPFlags showFlags )

{
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPBoolean compatible = envbPtr->compatible;
    PGPTime creation;
    PGPUInt32 numbits,subbits,algorithm;
    PGPError err;
    PGPUInt16 yyyy, mm, dd;
    PGPBoolean isSecret,isRevoked;
    PGPSize keyIDLength,bitsLength;
    PGPBoolean tab=FALSE;

    keyIDLength = (compatible ? 8 : 10);
    bitsLength = (compatible ? 4 : 9);

    err = PGPGetKeyNumber(key, kPGPKeyPropAlgID, &algorithm);
    pgpAssertNoErr(err);

    err = PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &isSecret);
    pgpAssertNoErr(err);

    /* what number of bits?*/
    err = PGPGetKeyNumber( key, kPGPKeyPropBits, &numbits );
    pgpAssertNoErr(err);

    /* when was it created?*/
    err = PGPGetKeyTime ( key, kPGPKeyPropCreation, &creation );
    pgpAssertNoErr(err);
    PGPGetYMDFromPGPTime( creation, &yyyy, &mm, &dd);

    /* now what about the subkey(s)?*/
    subbits=0;
    if( (showFlags & kShow_Subkeys) && !compatible) {
        char bitsstr[20];
        char *algstr;
        PGPBoolean isexp;

        /* Enumerate all subkeys.  NOTE, do not expose the subkey ID
           because the rules for selecting what subkey to use are complex
           and governed by the user's cipher preferences, expiration,
           validity, etc. so that if the user tries to specify what
           subkey to use, she may not get what she's asking for. */

        algstr = (char *)pgpPKAlgIDString( algorithm );

        sprintf( bitsstr, "%4d",numbits);
        fprintf( filebPtr->pgpout, "%3s%c %-9s %10s %4d/%02d/%02d ",
                algstr, schar, bitsstr, kstr, yyyy, mm, dd );

        /* what is the status? */
        err = PGPGetKeyBoolean( key, kPGPKeyPropIsRevoked, &isRevoked );
        pgpAssertNoErr(err);
        if( isRevoked ) {
           fprintf( filebPtr->pgpout, LANG("*** KEY REVOKED ***"));
           /*"\n     %-*s %-*s            "),bitsLength,"",keyIDLength,"" );*/
           tab=TRUE;
        }
        PGPGetKeyBoolean( key, kPGPKeyPropIsExpired, &isexp );
        if(isexp) {
           fprintf( filebPtr->pgpout, LANG("*** KEY EXPIRED ***"));
           /*"\n     %-*s %-*s            "),bitsLength,"",keyIDLength,"" );*/
           tab=TRUE;
        } else {
            PGPTime expyTime;
            err = PGPGetKeyTime( key, kPGPKeyPropExpiration, &expyTime);
            pgpAssertNoErr(err);
            if(expyTime != kPGPExpirationTime_Never) {
                PGPGetYMDFromPGPTime( expyTime, &yyyy, &mm, &dd);
                fprintf( filebPtr->pgpout, LANG("expires %4d/%02d/%02d"),
                        yyyy, mm, dd);
                tab=TRUE;
            }
        }

        /* what number of bits for the subkey(s)?...*/
        /*if( algorithm == kPGPPublicKeyAlgorithm_DSA )*/
        {
            PGPSubKeyRef subKey;
            PGPKeyIterSeek(iterContext, key);
            err = PGPKeyIterNextSubKey(iterContext, &subKey);
            while( IsntPGPError( err ) )
            {
                err = PGPGetSubKeyNumber(subKey, kPGPKeyPropBits, &subbits);
                pgpAssertNoErr(err);
                sprintf( bitsstr, "%4d",subbits);

                err = PGPGetSubKeyNumber(subKey, kPGPKeyPropAlgID,
                        &algorithm);

                pgpAssertNoErr(err);
                algstr = (char *)pgpPKAlgIDString( algorithm );

                /* when was it created?*/
                err = PGPGetSubKeyTime ( subKey, kPGPKeyPropCreation,
                        &creation );

                pgpAssertNoErr(err);
                PGPGetYMDFromPGPTime( creation, &yyyy, &mm, &dd);

                tab=FALSE;
                fprintf( filebPtr->pgpout, "\n%3s  %-*s %-*s %4d/%02d/%02d ",
                        algstr, bitsLength, bitsstr, keyIDLength, kstr,
                        yyyy, mm, dd );

                /* what is the status? */
                err = PGPGetSubKeyBoolean( subKey, kPGPKeyPropIsRevoked,
                        &isRevoked );

                pgpAssertNoErr(err);
                if( isRevoked ) {
                    fprintf( filebPtr->pgpout, LANG("*** KEY REVOKED ***"));
                    tab=TRUE;
                }
                PGPGetSubKeyBoolean( subKey, kPGPKeyPropIsExpired, &isexp );
                if(isexp) {
                    fprintf( filebPtr->pgpout, LANG("*** KEY EXPIRED ***"));
                    tab=TRUE;
                } else {
                    PGPTime expyTime;
                    err = PGPGetSubKeyTime( subKey, kPGPKeyPropExpiration,
                            &expyTime);
                    pgpAssertNoErr(err);
                    if(expyTime != kPGPExpirationTime_Never) {
                        PGPGetYMDFromPGPTime( expyTime, &yyyy, &mm, &dd);
                        fprintf( filebPtr->pgpout,
                                LANG("expires %4d/%02d/%02d"), yyyy, mm, dd);

                    tab=TRUE;
                    }
                }
                err = PGPKeyIterNextSubKey(iterContext, &subKey);
            }
        }
    } else {
        /* what number of bits for the subkey(s)?... show the first one
           only.*/

        /*if( algorithm == kPGPPublicKeyAlgorithm_DSA ) not just DSA, uknow*/
        {
            PGPSubKeyRef subKey;
            PGPKeyIterSeek(iterContext, key);
            err = PGPKeyIterNextSubKey(iterContext, &subKey);
            if( IsntPGPError( err ) )
            {
                err = PGPGetSubKeyNumber(subKey, kPGPKeyPropBits, &subbits);
                pgpAssertNoErr(err);
            }
        }

        if(!compatible) {
            char bitsstr[20];
            const char *algstr = pgpPKAlgIDString( algorithm );

            if( subbits > 0 )
                sprintf( bitsstr, "%4d/%d",subbits,numbits);
            else
                sprintf( bitsstr, "%4d",numbits);
            fprintf( filebPtr->pgpout, "%3s%c %-9s %10s %4d/%02d/%02d ",
                    algstr, schar, bitsstr, kstr, yyyy, mm, dd );
        } else
            fprintf( filebPtr->pgpout, "%3s%c %4d/%8s %4d/%02d/%02d ",
                    isSecret ? "sec" : "pub", schar, numbits, kstr,
                    yyyy, mm, dd );

        /* what is the status? */
        err = PGPGetKeyBoolean( key, kPGPKeyPropIsRevoked, &isRevoked );
        pgpAssertNoErr(err);
        if( isRevoked ) {
            fprintf( filebPtr->pgpout, LANG("*** KEY REVOKED ***"
                    "     %-*s %-*s            "),
                    bitsLength,"",keyIDLength,"" );
                    tab=TRUE;
        }
        if(!compatible)
        {
            PGPTime expyTime;
            PGPBoolean isexp;
            PGPGetKeyBoolean( key, kPGPKeyPropIsExpired, &isexp );
            if(isexp) {
                fprintf( filebPtr->pgpout, LANG("*** KEY EXPIRED ***"
                        "     %-*s %-*s            "),
                        bitsLength,"",keyIDLength,"" );
                    tab=TRUE;
            } else {
                err = PGPGetKeyTime( key, kPGPKeyPropExpiration, &expyTime);
                pgpAssertNoErr(err);
                if(expyTime != kPGPExpirationTime_Never) {
                    PGPGetYMDFromPGPTime( expyTime, &yyyy, &mm, &dd);
                    fprintf( filebPtr->pgpout, LANG("expires %4d/%02d/%02d"
                            "     %-*s %-*s             "), yyyy, mm, dd,
                            bitsLength,"",keyIDLength,"" );
                    tab=TRUE;
                }
            }
        }
    }
    if(tab)
        fprintf( filebPtr->pgpout, "\n     %-*s %-*s             ",
                bitsLength,"",keyIDLength,"" );

    if(err == kPGPError_EndOfIteration)
        err = kPGPError_NoErr;
    return err;
}

PGPError pgpShowKeyListFormat( struct pgpfileBones *filebPtr, PGPKeySetRef
        keyringset, PGPKeyIterRef iterContext, PGPKeyRef key, PGPFlags
        showFlags )
{
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPBoolean isSecret;
    char kstr[kPGPMaxKeyIDStringSize];
    PGPUserIDRef userid;
    char useridstr[ kPGPMaxUserIDSize ];
    PGPSize actual;
    PGPError err;
    PGPBoolean compatible = envbPtr->compatible;
    char schar;
    PGPSize keyIDLength,bitsLength;

	/* used to display default signing key */
	static PGPKeyRef	defaultKey = kPGPInvalidRef;
	static PGPBoolean	bNoDefaultKey = FALSE;


    keyIDLength = (compatible ? 8 : 10);
    bitsLength = (compatible ? 4 : 9);

    /* get the key properties*/
    /* is this a public or secret key?*/
    err = PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &isSecret);
    pgpAssertNoErr(err);

    err = pgpGetKeyIDStringCompatFromKey( key, TRUE, compatible, kstr );
    pgpAssertNoErr(err);

    schar = ' ';
    { /* print if key format is corrupted*/
        PGPBoolean isok;
        PGPGetKeyBoolean( key, kPGPKeyPropIsNotCorrupt, &isok );
        if( !isok )
            schar='?';
    }
    { /* print if it is disabled*/
        PGPBoolean isdis;
        PGPGetKeyBoolean( key, kPGPKeyPropIsDisabled, &isdis );
        if( isdis )
            schar='@';
    }

    err = pgpShowKeyPropertiesListFormat( filebPtr, key, iterContext,
            kstr, schar, showFlags );

	if(defaultKey == kPGPInvalidRef && !bNoDefaultKey)
	{
		err = PGPGetDefaultPrivateKey(keyringset, &defaultKey);
		if(IsPGPError(err))
		{
			bNoDefaultKey = TRUE;
			defaultKey = kPGPInvalidRef;
		}
	}
	if(defaultKey != kPGPInvalidRef)
	{
		if(!PGPCompareKeys(defaultKey, key, kPGPValidityOrdering))
		{
			/* this is the default key */
			fprintf(filebPtr->pgpout, "*** DEFAULT SIGNING KEY ***\n");
            fprintf( filebPtr->pgpout, "     %-*s %-*s            ",
                    bitsLength,"",keyIDLength,"");
		}
	}


    /* now list all userids associated with the key. */
    err = PGPKeyIterRewindUserID( iterContext );
    pgpAssertNoErr(err);
    err = PGPKeyIterNextUserID( iterContext, &userid);
    pgpAssertNoErr(err);

    if( userid ) {
        int before=0;
        while( userid ) {
            err = PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName,
                    256, useridstr, &actual );

            if( err == kPGPError_InvalidProperty ) {
                /* the userid field was empty? rare, but possible.*/
                useridstr[0]='\0';
            } else
                pgpAssertNoErr(err);

            fprintf( filebPtr->pgpout, "%s\n", useridstr);

            if( (showFlags & kShow_Hashes) && !before ) {
                pgpShowKeyFingerprint( filebPtr, key );
                before++;
            }

            if( showFlags & kShow_Sigs ) {
                PGPSigRef sig;
                err = PGPKeyIterRewindUIDSig( iterContext );
                pgpAssertNoErr(err);
                err = PGPKeyIterNextUIDSig( iterContext, &sig );
                /*if error, there are no sig's*/
                while( sig ) {

                    err = pgpShowKeySig( filebPtr, keyringset, sig,
                            showFlags );

                    pgpAssertNoErr(err);

                    err = PGPKeyIterNextUIDSig( iterContext, &sig );
                    /*if error, no more userids.*/
                }
            }

            err = PGPKeyIterNextUserID( iterContext, &userid);
            /*if err, no more userids for this key*/
            if(userid)
                fprintf( filebPtr->pgpout, "     %-*s %-*s            ",
                        bitsLength,"",keyIDLength,"");
        }
    }
    if( err == kPGPError_EndOfIteration )
        err = 0;
    return err;
}

PGPError viewKeySet(struct pgpmainBones *mainbPtr, PGPKeySetRef keyset,
        PGPFlags showFlags )
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyRef key = NULL;
    PGPError err;
    PGPSize keyIDLength,bitsLength;
    PGPBoolean compatible = mainbPtr->envbPtr->compatible;
    int keycount = 0;

    err = PGPOrderKeySet( keyset, kPGPUserIDOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);

    keyIDLength = ( compatible ? 8 : 10 );
    bitsLength = ( compatible ? 4 : 9 );

    if(!compatible)
        fprintf(filebPtr->pgpout,
                LANG("\nType %-*s %-*s Date       User ID\n"),
                bitsLength,LANG("bits"),keyIDLength, LANG("keyID"));
    else
        fprintf(filebPtr->pgpout,
                LANG("\nType %-*s/%-*s Date       User ID\n"),
                bitsLength,LANG("bits"),keyIDLength, LANG("keyID"));

    err = PGPKeyIterNext( keyiter, &key);
    /*if error, no keys found.*/
    while( key != NULL )
    {
        keycount++;

        err = pgpShowKeyListFormat( filebPtr, mainbPtr->workingRingSet,
                keyiter, key, showFlags );

        pgpAssertNoErr(err);

        err = PGPKeyIterNext( keyiter, &key);
        /*if err, no more keys*/
    }
    if(err == kPGPError_EndOfIteration)
        err = kPGPError_NoErr;
    fprintf( filebPtr->pgpout, LANG("%d matching key%s found.\n"),
            keycount, keycount != 1 ? "s" : "" );

    if(keyiter)
        PGPFreeKeyIter( keyiter );
    if(keylist)
        PGPFreeKeyList( keylist );
    return err;
}

int viewKeyring(struct pgpmainBones *mainbPtr, char *mcguffin,
PGPFileSpecRef ringfilespec, char *origRingFileName, PGPFlags showFlags )
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPKeyRingOpenFlags openflags = 0;
    PGPKeySetRef keyringset = NULL;
    PGPKeySetRef keyset = NULL;
    PGPError err;
    char *ringfile = NULL;
    PGPInt32 pri;
    PGPBoolean quietmode = pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err);

    if(ringfilespec) {
        err = PGPGetFullPathFromFileSpec( ringfilespec, &ringfile );
        pgpAssertNoErr(err);

        err = PGPOpenKeyRing( context, openflags, ringfilespec,
                &keyringset );

        if( IsPGPError(err) ) {
            fprintf(filebPtr->pgpout,
                    LANG("\nCan't open key ring file '%s'\n"),
                    origRingFileName);
            goto done;
        } else
            if (!quietmode) fprintf(filebPtr->pgpout,
                LANG("\nKey ring: '%s'"), ringfile);
    } else {
        err = PGPOpenDefaultKeyRings( context, openflags, &keyringset );
        if( IsPGPError(err) ) {
            fprintf(filebPtr->pgpout,
                    LANG("\nCan't open default key ring file\n"));
            goto done;
        }
    }

    if((mcguffin && mcguffin[0] != '\0') && !quietmode)
        fprintf(filebPtr->pgpout, LANG(", looking for user ID \"%s\"."),
                mcguffin);

    mainbPtr->workingRingSet=keyringset;

    err = pgpGetMatchingKeySet( mainbPtr, mcguffin, 0, &keyset);
    if( IsPGPError(err) )
        pgpShowError( filebPtr, err, 0,0); /*__FILE__,__LINE__);*/
    else
        err = viewKeySet(mainbPtr, keyset, showFlags );

done:
    if(keyset)
        PGPFreeKeySet(keyset);
    if(keyringset) {
        PGPFreeKeySet( keyringset );
        mainbPtr->workingRingSet = NULL;
    }
    if(ringfile)
        PGPFreeData(ringfile);
    return err == kPGPError_NoErr ? 0 : -1;
}

int viewKeyServer(struct pgpmainBones *mainbPtr, char *mcguffin, char
        *keyServerURL, PGPFlags showFlags )

{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = mainbPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPtlsContextRef tlsContext = kInvalidPGPtlsContextRef;
    PGPtlsSessionRef tlsSession = kInvalidPGPtlsSessionRef;
    PGPKeyServerType type;

    PGPError err, er2;
    int status;
    PGPInt32 pri;
    PGPBoolean quietmode = pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err);

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
        if (!envbPtr->compatible) {
            if (pgpenvGetInt( env, PGPENV_VERBOSE, &pri, &er2 )) {
                pgpShowError( filebPtr, err, __FILE__,__LINE__);
            }
            else {
                 pgpShowError( filebPtr, err, 0, 0);
            }
        }
        status = -1;
        goto done;
    } else {
        if (!quietmode) fprintf(filebPtr->pgpout,
            pgpLocationIsURL( keyServerURL ) ? 
            LANG("\nKey server: '%s'") :
            LANG("\nKey ring: '%s'"), keyServerURL);
        status = viewKeyring( mainbPtr, mcguffin, NULL, keyServerURL,
                showFlags );
    }

    err = PGPKeyServerClose( mainbPtr->workingKeyServer );
    pgpAssertNoErr(err);
done:
    if(tlsSession != kInvalidPGPtlsSessionRef)
        PGPFreeTLSSession(tlsSession);
    if(tlsContext != kInvalidPGPtlsContextRef)
        PGPFreeTLSContext(tlsContext);
    if(mainbPtr->workingKeyServer != kPGPInvalidRef)
        PGPFreeKeyServer( mainbPtr->workingKeyServer );
    mainbPtr->workingKeyServer = NULL;

    err = PGPKeyServerCleanup();
    pgpAssertNoErr(err);

    return status;
}

/*
  Called by -kc and -km, this function prints a keyring's stats:
   KeyID         Trust         Validity         User ID
 */
PGPError pgpShowKeySetTrustAndValidity( struct pgpfileBones *filebPtr,
        PGPKeySetRef ringSet, PGPKeySetRef keyset )
{
    PGPError err;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyRef key = NULL;

    PGPSize keyIDLength;
    PGPSize trustLength = 9;
    PGPSize validityLength = 9;
    PGPBoolean compatible = filebPtr->envbPtr->compatible;

    keyIDLength = ( compatible ? 8 : 10 );

    err = PGPOrderKeySet( keyset, kPGPAnyOrdering, &keylist );
    /* iterate thru and print interesting info */

    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);

    fprintf( filebPtr->pgpout, LANG("\n  %-*s %-*s %-*s %s\n"),
            keyIDLength, LANG("KeyID"), trustLength, LANG("Trust"),
            validityLength, LANG("Validity"), LANG("User ID") );

    err = PGPKeyIterNext( keyiter, &key);
    /*if error, no keys found.*/

    while( key != NULL )
    {
        err = pgpShowKeyTrustAndValidity( filebPtr, ringSet, keyiter,
                key, FALSE );
        pgpAssertNoErr(err);

        err = PGPKeyIterNext( keyiter, &key);
        /*if err, no more keys*/
    }
    if( err == kPGPError_EndOfIteration )
        err = kPGPError_NoErr;

    if( keyiter )
        PGPFreeKeyIter( keyiter );
    if( keylist )
        PGPFreeKeyList( keylist );
    return err;
}

PGPError pgpShowTrustAndValidityList(struct pgpmainBones *mainbPtr,
        char *useridStr, PGPFileSpecRef ringFileSpec)
{
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPError err;
    PGPKeySetRef ringSet;

    PGPKeySetRef keysToCheck;

    err = pgpOpenKeyringsFromPubringSpec( mainbPtr, ringFileSpec, &ringSet , 0);
    if( IsPGPError(err) )
        return -1;

    mainbPtr->workingRingSet=ringSet;
    err = pgpGetMatchingKeySet(mainbPtr, useridStr, kMatch_NotKeyServer,
            &keysToCheck );
    pgpAssertNoErr(err);

    err = pgpShowKeySetTrustAndValidity( filebPtr, ringSet, keysToCheck );
    pgpAssertNoErr(err);

    PGPFreeKeySet( keysToCheck );
    PGPFreeKeySet( ringSet );
    mainbPtr->workingRingSet = NULL;

    return kPGPError_NoErr;
}

