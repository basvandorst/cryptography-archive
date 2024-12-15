/*____________________________________________________________________________
    Keyedit.c

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    use the PGP SDK to edit the keyring.

    $Id: keyedit.c,v 1.11.6.1 1999/06/14 21:56:17 sluu Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <string.h>

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpSDKPrefs.h"

#include "usuals.h"
#include "pgp.h"
#include "globals.h"
#include "fileio.h"
#include "prototypes.h"
#include "language.h"


/* figure out the secret ring file name from the
   public ring file name. */

PGPError pgpGetCorrespondingSecretRingName( struct pgpfileBones *filebPtr,
        const char *pubringfile, char *secringfile )
{
    char *base;
    strcpy( secringfile, pubringfile );
    base = fileTail( secringfile );
    if( strcmp( base, "pubring.pkr" )==0 )
    {
        /* this depends on a side-effect of the above fileTail() call,
           that base points to an address _within_ the secringfile string. */

        base[0]='\0';
        strcat( base, "secring.skr" );
    }

    forceExtension( filebPtr, secringfile, filebPtr->SKR_EXTENSION );
    if( fileExists( secringfile ))
        return kPGPError_NoErr;
    else
        return kPGPError_FileNotFound;
}

PGPError pgpGetCorrespondingPublicRingName( struct pgpfileBones *filebPtr,
        const char *secringfile, char *pubringfile )
{
    char *base;
    strcpy( pubringfile, secringfile );
    base = fileTail( pubringfile );
    if( strcmp( base, "pubring.pkr" )==0 )
    {
        /* this depends on a side-effect of the above fileTail() call,
           that base points to an address _within_ the pubringfile string. */

        base[0]='\0';
        strcat( base, "secring.skr" );
    }

    forceExtension( filebPtr, pubringfile, filebPtr->SKR_EXTENSION );
    if( fileExists( pubringfile ))
        return kPGPError_NoErr;
    else
        return kPGPError_FileNotFound;
}

/*
   Try to get a valid passphrase.  Always trying getting it from the
   list of saved passphrases first.  If not in batchmode, maybe ask
   the user.  Perhaps we could redesign this to have the passphrase list
   use reference counts?
 */
PGPError pgpGetValidPassphrase( struct pgpmainBones *mainbPtr, PGPKeyRef
        key, char **passphrasePtr, PGPBoolean *needsfree )
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPBoolean valid;
    PGPInt32 attempts,pri;
    PGPError err;
    PGPBoolean quietmode = pgpenvGetInt( env, PGPENV_NOOUT, &pri, &err);
    PGPBoolean batchmode = pgpenvGetInt( env, PGPENV_BATCHMODE, &pri, &err );

    *needsfree=FALSE;

    pgpRewindPassphrase( envbPtr->passwds );
    for(;;) {
        pgpNextPassphrase( envbPtr->passwds, passphrasePtr );
        if(*passphrasePtr == NULL)
            break;
        valid = PGPPassphraseIsValid( key,
                PGPOPassphrase( context, *passphrasePtr ),
                PGPOLastOption( context ));
        if(valid) {
            if (!quietmode)
                fprintf( filebPtr->pgpout, LANG("\nPassphrase is good\n"));
            return kPGPError_NoErr;
        } else {
            if(batchmode)
                fprintf( filebPtr->pgpout,
                        LANG("\nError:  Bad pass phrase.\n"));
        }
    }

    if(!batchmode)
      for( attempts=0; attempts < 3; attempts++ )
      {
          fprintf( filebPtr->pgpout, LANG(
"\nYou need a pass phrase to unlock your secret key.") );
          pgpShowKeyUserID( filebPtr, key );

          err = pgpPassphraseDialogCmdline( mainbPtr, FALSE, NULL,
                  passphrasePtr);

          if( IsPGPError(err) )
              return err;

          *needsfree = (*passphrasePtr != NULL);

          valid = PGPPassphraseIsValid( key,
                  PGPOPassphrase( context, *passphrasePtr ),
                  PGPOLastOption( context ));

          if(valid) {
              if (!quietmode)
                  fprintf( filebPtr->pgpout, LANG("\nPassphrase is good\n"));

              /* add to the passphrase list...*/
              pgpAppendToPassphraseList( envbPtr->passwds, *passphrasePtr );

              return kPGPError_NoErr;
          }
          else {
              fprintf( filebPtr->pgpout,
                      LANG("\nError:  Bad pass phrase.\n"));
              PGPFreeData( *passphrasePtr );
              pgpRemoveFromPointerList( mainbPtr->leaks, *passphrasePtr );
              *passphrasePtr=NULL;
              *needsfree=FALSE;
          }
      }
    if(*needsfree) {
        PGPFreeData( *passphrasePtr );
        pgpRemoveFromPointerList( mainbPtr->leaks, *passphrasePtr );
    }
    *passphrasePtr=NULL;
    *needsfree=FALSE;
    return kPGPError_BadPassphrase;
}

/*
  inputs: a KeyIter, positioned at a particular key of interest.
  returns the Userid that matches the character string.
 */
PGPError pgpGetKeyIterMatchingUserid( PGPKeyIterRef keyiter, const char
        *searchstr, PGPUserIDRef *useridPtr )
{
    char useridstr[ kPGPMaxUserIDSize ];
    PGPSize actual;
    PGPError err;

    err = PGPKeyIterRewindUserID( keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterNextUserID( keyiter, useridPtr);
    pgpAssertNoErr(err);

    while( *useridPtr ) {
        err = PGPGetUserIDStringBuffer( *useridPtr, kPGPUserIDPropName,
                kPGPMaxUserIDSize, useridstr, &actual);
        if( strcmp( useridstr, searchstr ) == 0)
            return kPGPError_NoErr;
        err = PGPKeyIterNextUserID( keyiter, useridPtr);
        /*if err, there are no more*/
    }

   *useridPtr = NULL;
   return kPGPError_ItemNotFound;
}

PGPError pgpEditPublicTrustParameter( struct pgpfileBones *filebPtr,
        const char *useridstr, PGPKeyRef key)
{
    PGPError err;
    char anstr[ 6 ];
    PGPInt32 ann;
    char *str;

    /* print the key info*/

    /* check each of the certs*/

    pgpShowKeyValidity( filebPtr, key );

    /* Questionable certification from:*/
    /* (KeyID: 7A4A5505)*/
    /* Questionable certification from:*/
    /* (KeyID: BD236602)*/

    err = pgpGetKeyTrustString( key, &str );
    pgpAssertNoErr(err);
    fprintf( filebPtr->pgpout,
            LANG("\nCurrent trust for this key's owner is: %s\n"), str);
    fprintf(filebPtr->pgpout, LANG("\
\nMake a determination in your own mind whether this key actually\n\
belongs to the person whom you think it belongs to, based on available\n\
evidence.  If you think it does, then based on your estimate of\n\
that person's integrity and competence in key management, answer\n\
the following question:\n\n"));

        fprintf(filebPtr->pgpout, LANG("Would you trust \"%s\"\n\
to act as an introducer and certify other people's public keys to you?\n\
(1=I don't know (default). 2=No. 3=Usually. 4=Yes, always.) ? "), useridstr );

    fflush(filebPtr->pgpout);
    pgpTtyGetString(anstr, 6, filebPtr->pgpout);

    ann = atoi( anstr );
    switch( ann ) {
        case 4:
            err = PGPSetKeyTrust( key, kPGPKeyTrust_Complete );
            break;
        case 3:
            err = PGPSetKeyTrust( key, kPGPKeyTrust_Marginal );
            break;
        case 2:
            err = PGPSetKeyTrust( key, kPGPKeyTrust_Never );
            break;
        case 1:
        default:
            err = PGPSetKeyTrust( key, kPGPKeyTrust_Unknown );
            break;
    }
    if( IsPGPError(err) )
        pgpShowError( filebPtr, err,__FILE__,__LINE__ );
    return err;
}

static
PGPBoolean	IsDefaultPublicKeyring(PGPContextRef context, char *pszPubringfile)
{
	PGPError		err = kPGPError_NoErr;
	PGPFileSpecRef	pubringSpec = kPGPInvalidRef;
	char			*pubringName = NULL; 
	PGPBoolean		bDefault = FALSE;

	err = PGPsdkPrefGetFileSpec( context, kPGPsdkPref_PublicKeyring,
            &pubringSpec);

	if(IsPGPError(err))
		return FALSE;

    err = PGPGetFullPathFromFileSpec( pubringSpec, &pubringName );
    if(IsPGPError(err))
		goto done;
	if(strcmp(pubringName, pszPubringfile) == 0)
	{
		bDefault = TRUE;
	}

done:
	if(pubringName != NULL)
		PGPFreeData(pubringName);
	if(pubringSpec != kPGPInvalidRef)
		PGPFreeFileSpec(pubringSpec);
	return bDefault;
}


/*
   Takes an optional ring FileSpec that is the public keyring.  User may
   only specify a public keyring (or NULL), but we need
     1. to look on the corresponding secret keyring for the secret key;
     2. to open the keyrings mutable.
 */

PGPError pgpOpenKeyringsFromPubringSpec( struct pgpmainBones *mainbPtr,
        PGPFileSpecRef pubFileSpec, PGPKeySetRef *keyRingSet , 
	PGPUInt32 openFlags)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPError err,er2;

    if(pubFileSpec) {
        char secringfile[ MAX_PATH+1 ];
        char *pubringfile;
        PGPFileSpecRef secFileSpec;

        err = PGPGetFullPathFromFileSpec( pubFileSpec, &pubringfile );
        pgpAssertNoErr(err);

		if(!IsDefaultPublicKeyring(context, pubringfile))
		{
			err = pgpGetCorrespondingSecretRingName( filebPtr, pubringfile,
					secringfile );
			if (IsPGPError(err)) goto done;

			err = PGPNewFileSpecFromFullPath(context, secringfile, &secFileSpec);
			pgpAssertNoErr(err);

			err = PGPOpenKeyRingPair( context, openFlags, 
					pubFileSpec, secFileSpec, keyRingSet );
			er2 = PGPFreeFileSpec( secFileSpec );
			pgpAssertNoErr(er2);
		}
		else
		{
			err = PGPOpenDefaultKeyRings(context, openFlags, keyRingSet);
		}
    done:
        if( IsPGPError(err) ) {
            fprintf(filebPtr->pgpout,
                    LANG("Can't open key ring file '%s'\n"), pubringfile);
        }
	
	er2 = PGPFreeData( pubringfile );
	pgpAssertNoErr(er2);
    } else {
        fprintf(filebPtr->pgpout, LANG("in default key ring\n\n"));

        err = PGPOpenDefaultKeyRings( context,
		openFlags, keyRingSet);

        if( IsPGPError(err) ) {
            fprintf(filebPtr->pgpout, LANG("Can't open default key rings\n"));
        }
    }
    return err;
}

PGPError pgpOpenKeyringsIfSecringSpec( struct pgpmainBones *mainbPtr,
        PGPFileSpecRef fileSpec, PGPKeySetRef *keyRingSet, PGPBoolean
	*isprivate, PGPUInt32 openFlags)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    PGPError err,er2;

    *isprivate = FALSE;

    if( fileSpec ) {
        char pubringfile[ MAX_PATH+1 ];
		char *ringfile;

        err = PGPGetFullPathFromFileSpec( fileSpec, &ringfile );
        pgpAssertNoErr(err);

        if( hasExtension(ringfile, filebPtr->SKR_EXTENSION) )
        {
            PGPFileSpecRef pubFileSpec;

            *isprivate = TRUE;
            err = pgpGetCorrespondingPublicRingName( filebPtr, ringfile,
                    pubringfile );

            pgpAssertNoErr(err);
            err = PGPNewFileSpecFromFullPath(context, pubringfile,
                    &pubFileSpec);

            pgpAssertNoErr(err);
            err = PGPOpenKeyRingPair( context,
		    openFlags, pubFileSpec, fileSpec,
                    keyRingSet );

            er2 = PGPFreeFileSpec( pubFileSpec );
            pgpAssertNoErr(er2);
        } else
            *isprivate = FALSE;
	    err = PGPOpenKeyRing( context, openFlags,	
                    fileSpec, keyRingSet );

        if( IsPGPError(err) )
		{
            fprintf(filebPtr->pgpout,
                    LANG("Can't open key ring file '%s'\n"),
                    ringfile);
		}
        er2 = PGPFreeData( ringfile );
        pgpAssertNoErr(er2);

    } else {
        fprintf(filebPtr->pgpout, LANG("in default key ring\n\n"));

        err = PGPOpenDefaultKeyRings( context,
		openFlags, keyRingSet);

        if( IsPGPError(err) ) {
            fprintf(filebPtr->pgpout, LANG("Can't open default key rings\n"));
        }
    }
    return err;
}

/*
   Edit the userid and/or pass phrase for a key pair, and put them back
   into the ring files.
 */

int doKeyEdit(struct pgpmainBones *mainbPtr, const char *searchstr,
        PGPFileSpecRef pubFileSpec)
{
    PGPContextRef context = mainbPtr->pgpContext;
    struct pgpfileBones *filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones *envbPtr = filebPtr->envbPtr;
    PGPEnv *env = envbPtr->m_env;
    PGPKeySetRef keyRingSet = NULL;
    PGPKeyListRef keylist = NULL;
    PGPKeyIterRef keyiter = NULL;
    PGPKeyRef key = NULL;
    PGPError err,er2;
    PGPInt32 pri;

    if( !searchstr || searchstr[0]=='\0' )
        return -1;

    fprintf(filebPtr->pgpout, LANG("\nEditing userid \"%s\" "),searchstr);
    if(pubFileSpec) {
        char *pubringfile;
        err = PGPGetFullPathFromFileSpec( pubFileSpec, &pubringfile );
        pgpAssertNoErr(err);
        fprintf(filebPtr->pgpout, LANG("in key ring: '%s'.\n\n"),pubringfile);
        er2 = PGPFreeData( pubringfile );
    }

    err = pgpOpenKeyringsFromPubringSpec( mainbPtr, pubFileSpec,
	    &keyRingSet, kPGPKeyRingOpenFlags_Mutable);
    if( IsPGPError(err) )
        return -1;

    mainbPtr->workingRingSet=keyRingSet;
    /*mainbPtr->workingGroupSet=NULL;*/
    err = pgpGetMatchingKeyList( mainbPtr, searchstr, kMatch_NotKeyServer,
            &keylist);
    pgpAssertNoErr(err);

    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNext( keyiter, &key);
        /* pgp 2.6.2 does the edit on the first matching public key in the set
           but we believe that any user in the set should be edited. */

    if (key == NULL) {

        if(pubFileSpec) {
            char *pubringfile;
            err = PGPGetFullPathFromFileSpec( pubFileSpec, &pubringfile );
            pgpAssertNoErr(err);
            fprintf(filebPtr->pgpout,
                    LANG("\n\007Key not found in key ring '%s'.\n"),
                pubringfile);
            er2 = PGPFreeData( pubringfile );
        }

        if( keyiter != NULL)
            PGPFreeKeyIter( keyiter );
        if( keylist != NULL)
            PGPFreeKeyList( keylist );

        PGPFreeKeySet( keyRingSet );
        mainbPtr->workingRingSet = NULL;

        return -1;
    }

    while( key != NULL )
    {
        PGPBoolean ans,mine;
        PGPBoolean batchmode = pgpenvGetInt( env, PGPENV_BATCHMODE,
                &pri, &err );
        PGPBoolean issecret;
        char *passphrase;
        PGPUserIDRef userid;
        char useridstr[ kPGPMaxUserIDSize ];
        PGPInt32 actual;

        err = PGPGetPrimaryUserID( key, &userid);
        pgpAssertNoErr(err);
        err =  PGPGetUserIDStringBuffer( userid, kPGPUserIDPropName,
                256, useridstr, &actual );

        pgpAssertNoErr(err);

        err = PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &issecret);
        pgpAssertNoErr(err);

        err = pgpShowKeyBrief(filebPtr, key);
        pgpAssertNoErr(err);

        if( issecret ) {

            err = pgpGetValidPassphrase( mainbPtr, key, &passphrase, &mine );

            if( IsntPGPError(err) ) {
				fprintf(filebPtr->pgpout,
LANG("Use this key as an ultimately-trusted introducer (y/N)? "));

				ans = getyesno( filebPtr, 'n', batchmode );

				if(ans) {
					err = PGPSetKeyAxiomatic( key,
							PGPOPassphrase( context, passphrase ),
							PGPOLastOption( context ));
					pgpAssertNoErr(err);

					fprintf(filebPtr->pgpout,
LANG("Make this the default signing key (y/N)? "));
					ans = getyesno( filebPtr, 'n', batchmode );
					if(ans) {
						err = PGPSetDefaultPrivateKey(key);
						pgpAssertNoErr(err);
						err = PGPsdkSavePrefs( context );
						pgpAssertNoErr(err);
					}

				}
				else
				{	/* remove implicit trust */
					err = PGPUnsetKeyAxiomatic(key);
				}

                fprintf(filebPtr->pgpout, LANG("Current user ID: %s\n"),
                        useridstr );

                fprintf(filebPtr->pgpout, LANG(
                        "Do you want to add a new user ID (y/N)? "));
                ans = getyesno( filebPtr, 'n', batchmode );

                if(ans) {
                    PGPUserIDRef newuserid;
                    char newstr[ kPGPMaxUserIDSize ];
                    fprintf(filebPtr->pgpout,
                            LANG("Enter the new user ID: "));
                    fflush( filebPtr->pgpout );
                    pgpTtyGetString(newstr, kPGPMaxUserIDSize-1,
                            filebPtr->pgpout);

                    if( strlen( newstr ) == 0 ) {
                        PGPFreeData( passphrase );
                        pgpRemoveFromPointerList( mainbPtr->leaks,
                                passphrase );
                        return -1;
                    }

                    err = PGPAddUserID( key, newstr,
                            PGPOPassphrase( context, passphrase ),
                            PGPOLastOption( context ));
                    pgpAssertNoErr(err);

                    err = pgpGetKeyIterMatchingUserid( keyiter, newstr,
                            &newuserid );
                    pgpAssertNoErr(err);

                    /* if we gave a valid passphrase, then sdk should
                       already have signed the new userid.*/

                    fprintf(filebPtr->pgpout,
LANG("Make this user ID the primary user ID for this key (y/N)? "));
                    ans = getyesno( filebPtr, 'n', batchmode );

                    if( ans ) {
                        err = PGPSetPrimaryUserID( newuserid );
                        pgpAssertNoErr(err);
                    }
                }

                fprintf(filebPtr->pgpout,
                    LANG("Do you want to change your pass phrase (y/N)? "));
                ans = getyesno( filebPtr, 'n', batchmode );

                if(ans) {
                    char *newpassphrase;
                    PGPBoolean alsomine = FALSE;
                    err = pgpNextPassphrase( envbPtr->passwds,
                            &newpassphrase );

                    if( newpassphrase == NULL ) {
                        err = pgpPassphraseDialogCmdline( mainbPtr,
                                TRUE, NULL, &newpassphrase);

                        alsomine = TRUE;
                    }

                    err = PGPChangePassphrase( key,
                            PGPOPassphrase( context, passphrase ), /* old*/
                            PGPOPassphrase( context, newpassphrase ), /* new*/
                            PGPOLastOption( context ));
                    pgpAssertNoErr(err);

                    {
                        PGPSubKeyRef subKey;
                        //PGPKeyIterSeek(keyiter, key);
                        err = PGPKeyIterNextSubKey(keyiter, &subKey);

                        while( IsntPGPError( err ) ) {

                            if(!envbPtr->compatible) {
                                /* FUTURE functionality: show the user
                                   the subkey and ask whether to change
                                   the passphrase for this one.  This is
                                   to permit the signing and encryption
                                   passphrases to be different.

                                   To implement this functionality,
                                   want to do two things.  (1) create a
                                   function analogous to pgpShowKeyBrief()
                                   that will show the subKey.  Note,
                                   however, to hide the subkey id from
                                   the user.  (2) in key generation,
                                   also ask whether to have different
                                   passphrases.
                                 */
                                pgpFixBeforeShip("ask the user");
                                if(FALSE)
                                    goto next;
                            }
                            err = PGPChangeSubKeyPassphrase( subKey,
                               PGPOPassphrase( context, passphrase ),
                               PGPOPassphrase( context, newpassphrase ),
                               PGPOLastOption( context ));
                            pgpAssertNoErr(err);

                        next:
                            err = PGPKeyIterNextSubKey(keyiter, &subKey);
                        }
                    }

                    if(alsomine) {
                        PGPFreeData( newpassphrase );
                        pgpRemoveFromPointerList( mainbPtr->leaks,
                                newpassphrase );

                    }
                }

            } else {
                fprintf( filebPtr->pgpout,
                        LANG("No passphrase; secret key unavailable.\n"));

                issecret = FALSE;
            }
            if(mine) {
                PGPFreeData( passphrase );
                pgpRemoveFromPointerList( mainbPtr->leaks, passphrase );
            }
        }

        if( !issecret ) {
            fprintf(filebPtr->pgpout, LANG(
"No secret key available.  Editing public key trust parameter.\n"));
            err = pgpEditPublicTrustParameter( filebPtr, useridstr, key);
        }

        err = PGPKeyIterNext( keyiter, &key);
#ifndef SUPPORT_MULTIEDIT
        break;
#endif /* SUPPORT_MULTIEDIT */
    }
    if(err == kPGPError_EndOfIteration)
        err = kPGPError_NoErr;

    if( keyiter != NULL)
        PGPFreeKeyIter( keyiter );
    if( keylist != NULL)
        PGPFreeKeyList( keylist );

    if( PGPKeySetNeedsCommit( keyRingSet ) ) {
        PGPPropagateTrust( keyRingSet );
        err = PGPCommitKeyRingChanges( keyRingSet );
        pgpAssertNoErr(err);
    }
    PGPFreeKeySet( keyRingSet );
    mainbPtr->workingRingSet = NULL;

    return err;
}

