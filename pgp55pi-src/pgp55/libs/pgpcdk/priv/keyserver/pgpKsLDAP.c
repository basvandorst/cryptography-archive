/*
 * pgpKsLDAP.c
 * PGP Key Server LDAP Library implementation
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
 * All rights reserved
 *
 * $Id: pgpKsLDAP.c,v 1.79.4.2 1997/11/07 05:26:27 markm Exp $
 */

#include <stdio.h>
#include <string.h>
#include "pgpConfig.h"
#include "pgpContext.h"
#include "pgpKeyServerPriv.h"
#include "pgpErrors.h"
#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpEncode.h"
#include "pgpEventPriv.h"
#include "PGPSockets.h"	
#include "ldap.h"

#include "pgpStrings.h"


#if PGP_MACINTOSH
#define PGP_CALLBACK	1
#else
#define PGP_CALLBACK	0
#endif

#if PGP_CALLBACK
extern PGPSocketRef *	gCallbackDataSocket;
#endif


static PGPError pgpLDAPAddKey( 
					PGPKeyServerRef		keyServer, 
					PGPByte				*keyBuffer,
					PGPEventHandlerProcPtr callBack,
					PGPUserValue		callBackArg,
					PGPUInt32			currentRecord,
					PGPUInt32			totalKeys);

static PGPError pgpLDAPProcessSearchResults( 
					PGPKeyServerRef		keyServer,
					PGPInt32			msgId,
					PGPEventHandlerProcPtr callBack,
					PGPUserValue		callBackArg,
					PGPKeySetRef		*resultSet );

static PGPError	pgpLDAPProcessInitResults( 
					PGPKeyServerRef		keyServer,
					PGPInt32			msgId,
					PGPEventHandlerProcPtr callBack,
					PGPUserValue		callBackArg );

static PGPError pgpLDAPBuildRequest(
					PGPKeyServerRef		keyServer,
					const char			*requestVerb,
					PGPKeySetRef		keysToProcess,
					PGPEventHandlerProcPtr callBack,
					PGPUserValue		callBackArg,
					char				**request );

static PGPError pgpLDAPGetResults( 
					PGPKeyServerRef		keyServer,
					PGPInt32			msgId,
					PGPEventHandlerProcPtr callBack,
					PGPUserValue		callBackArg,
					PGPBoolean			*successFlag );

static PGPError pgpSaveLDAPError( 
					PGPKeyServerRef keyServer );

static PGPError pgpScanForErrorKeys( 
					PGPKeyServerRef keyServer, 
					PGPKeySetRef inKeys, 
					PGPKeySetRef *keysThatFailed );

PGPError		pgpLDAPSignEvent(
					PGPContextRef context, 
					PGPEvent *event, 
					PGPUserValue userValue);

static PGPError	pgpLDAPSendRequest( 
					PGPKeyServerRef		keyServer,
					PGPKeySetRef		keys,
					char				*request,
					PGPEventHandlerProcPtr callBack,
					PGPUserValue		callBackArg,
					PGPKeySetRef		*keysThatFailed);

/*
 * Delete or Disable a set of keys from the keyserver
 *
 * LDAP support ONLY!
 */

PGPError 
pgpLDAPDeleteFromKeyServer( 
	PGPKeyServerRef		keyServer,
	PGPKeySetRef		keysToDelete,
	PGPBoolean			disableKeys,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*keysThatFailed)
{
	PGPError		err			  = kPGPError_NoErr;
	PGPBoolean		mustCloseLDAP = FALSE;
	char			*request	  = NULL;

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( keysToDelete );
	PGPValidatePtr( keysThatFailed );

	/*
	 * Access type *must* be Administrator for this operation
	 */

	if (keyServer->accessType != kPGPKSAccess_Administrator)
	{
		err = kPGPError_ServerAuthorizationRequired;
		goto done;
	}

	if ( keyServer->ldap == NULL )
	{
		/*
		 * The caller has not opened an LDAP connection for us,
		 * so we'll do it, and clean it up later
		 */

		err = pgpLDAPOpenKeyServer( keyServer, callBack, callBackArg );

		if (IsPGPError(err))
		{
			goto done;
		}
		mustCloseLDAP = TRUE;
	}

	if ( disableKeys == TRUE )
	{
		err = pgpLDAPBuildRequest( keyServer,
								"disable",
								keysToDelete,
								callBack,
								callBackArg,
								&request );
	}
	else 
	{
		err = pgpLDAPBuildRequest( keyServer,
								"delete",
								keysToDelete,
								callBack,
								callBackArg,
								&request );
	}

	if (IsntPGPError(err))
	{
		err = pgpLDAPSendRequest( keyServer, keysToDelete, 
						request, callBack, callBackArg, 
						keysThatFailed );
	}

done:
	
	if ( request != NULL )
	{
		PGPFreeData(request);
	}

#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif
	if ( mustCloseLDAP == TRUE )
	{
		pgpLDAPCloseKeyServer( keyServer, callBack, callBackArg);
	}
	return err;
}


/*
 * To enable the directory to be moved without impacting the clients,
 * the DN for the key spaces is read dynamically
 */

PGPError 
pgpLDAPGetBaseDNFromKeyServer( 
	PGPKeyServerRef		keyServer,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg)
{
	PGPError	err = kPGPError_NoErr;
	PGPInt32	msgId;
	char		*attr[3];
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( keyServer->ldap );

	attr[0] = (char*) "baseKeyspaceDN";
	attr[1] = (char*) "basePendingDN";
	attr[2] = NULL;

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateSend;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
			pgpKeyServerSocketCallback,
			(void *) &callBackData);
#endif

	msgId = ldap_search( keyServer->ldap, 
					(char*) "cn=PGPServerInfo", LDAP_SCOPE_BASE,
					(char*) "(objectclass=*)", attr, 0 ); 

#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif
	if ( msgId != -1 )
	{
		PGPBoolean	successFlag = FALSE;

		err = pgpLDAPGetResults( keyServer,
							msgId, callBack,
							callBackArg, 
							&successFlag );

		if (IsntPGPError(err) && successFlag == TRUE)
		{
			err = pgpLDAPProcessInitResults( keyServer,
										msgId, 
										callBack,
										callBackArg);
		}
		else if (IsntPGPError(err))
		{
			err = kPGPError_ServerSearchFailed;
		}
	}
	return err;
}

/*
 * Send the Query to the LDAP server, and get the set of keys
 * which statisfy the request
 */

PGPError 
pgpQueryLDAPKeyServer(	
	PGPKeyServerRef		keyServer, 
	PGPFilterRef		filter,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*resultSet,
	PGPFlags *			outFlags)
{
	PGPError		err			= kPGPError_NoErr;
	char			*query		= NULL;
	char			*keySpace	= NULL;
	char			*attr[2];
	PGPInt32		msgId;
	PGPBoolean		mustCloseLDAP = FALSE;
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	if ( IsntNull( resultSet ) )
		*resultSet	= kInvalidPGPKeySetRef;
	if ( IsntNull( outFlags ) )
		*outFlags	= 0;
	PGPValidatePtr( resultSet );
	PGPValidatePtr( outFlags );
	PGPValidateKeyServer( keyServer );

	if ( keyServer->ldap == NULL )
	{
		/*
		 * The caller has not opened an LDAP connection for us,
		 * so we'll do it, and clean it up later
		 */

		err = pgpLDAPOpenKeyServer( keyServer, callBack, callBackArg );

		if (IsPGPError(err))
		{
			goto done;
		}
		mustCloseLDAP = TRUE;
	}

	/*
	 * Turn the filter object into an LDAP RPN query
	 * This must be freed with PGPFreeData() later
	 */
	err = PGPLDAPQueryFromFilter( filter, &query );

	if ( IsPGPError(err) )
	{
		goto done;
	}

	/*
	 * base   = base dn for search
	 * filter = the actual search in ldap format "(!(cn=bob)(sn=bob))"
	 * attrs  = the attributes we want back
	 */

	attr[0] = (char*) "pgpkey";
	attr[1] = NULL;

	/*
	 * Determine which keyspace we are looking for these keys in
	 */

	if ( keyServer->keySpace == kPGPKSKeySpaceNormal ||
	 	keyServer->keySpace == kPGPKSKeySpaceDefault )
	{
		keySpace = keyServer->baseKeySpaceDN;
	}
	else
	{
		keySpace = keyServer->basePendingDN;
	}

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateSend;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	/*
	 * Fire off the async search
	 */

	msgId = ldap_search( keyServer->ldap, 
					keySpace, LDAP_SCOPE_SUBTREE,
					query, attr, 0 ); 

#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif
	if ( msgId != -1 )
	{
		PGPBoolean	successFlag = FALSE;

		err = pgpLDAPGetResults( keyServer,
							msgId, callBack,
							callBackArg, 
							&successFlag );

		if ( err == kPGPError_ServerTooManyResults && successFlag )
		{
			/* note this fact in output flags */
			/* and don't consider it an error */
			*outFlags	|= kPGPKeyServerQuery_PartialResults;
			err	= kPGPError_NoErr;
		}
		
		if ( IsntPGPError(err) )
		{
			pgpAssert( successFlag );
			err = pgpLDAPProcessSearchResults( keyServer,
										msgId, 
										callBack,
										callBackArg, 
										resultSet );
		}
	}
	else
	{
		err = kPGPError_ServerSearchFailed;
	}

done:

	if ( query )
	{
		PGPFreeData( query );
	}

	if ( mustCloseLDAP == TRUE )
	{
		pgpLDAPCloseKeyServer( keyServer, callBack, callBackArg);
	}
	
	pgpAssertErrWithPtr( err, *resultSet );
	return err;
}

/*
 * Send the set of keys to the LDAP server
 */

PGPError 
pgpLDAPUploadToKeyServer( 
	PGPKeyServerRef		keyServer, 
	PGPKeySetRef		keysToUpload,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*keysThatFailed)
{
    PGPError		err		= kPGPError_NoErr;
	PGPKeyListRef	keyList	= NULL;
	PGPKeyIterRef	keyIter	= NULL;
	PGPKeyRef		key		= NULL;
	PGPKeySetRef	keySetSingle	= NULL;
	PGPKeySetRef	keySetError		= NULL;
	PGPByte			*keyBuffer		= NULL;
	PGPByte			*tempKeyBuffer	= NULL;
	PGPUInt32		totalKeys		= 0;
	PGPUInt32		currentRecord	= 1;
	PGPSize			bufSize;
	PGPBoolean		mustCloseLDAP	= FALSE;
	PGPBoolean		partialFailure	= FALSE;
	PGPBoolean		keyAlreadyExists= FALSE;
	PGPBoolean		keyFailedPolicy = FALSE;
	char			*request        = NULL;
	PGPOptionListRef	newOptionList;

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( keysToUpload );
	PGPValidatePtr( keysThatFailed );

	if ( keyServer->ldap == NULL )
	{
		/*
		 * The caller has not opened an LDAP connection for us,
		 * so we'll do it, and clean it up later
		 */

		err = pgpLDAPOpenKeyServer( keyServer, callBack, callBackArg );

		if (IsPGPError(err))
		{
			goto done;
		}
		mustCloseLDAP = TRUE;
	}

	err = PGPNewKeySet( keyServer->myContext, &keySetError );

	if ( IsPGPError(err) ) 
	{
		goto done;
	}

	err = PGPCountKeys( keysToUpload, &totalKeys);
	if ( IsPGPError(err) )
	{
		goto done;
	}

	/*
	 * We must create a keyList by ordering the keySet
	 */

	err = PGPOrderKeySet( keysToUpload, kPGPAnyOrdering, &keyList );

	if ( IsPGPError(err) )
	{
		goto done;
	}

	err = PGPNewKeyIter( keyList, &keyIter );

	if ( IsPGPError(err) )
	{
		goto done;
	}

	/*
	 * Allocate a 1K buffer to hold the ascii armored key data
	 * If this buffer is too small, we will grow it
	 */

	while (( err = PGPKeyIterNext(keyIter, &key)) == kPGPError_NoErr)
	{
		err = PGPNewSingletonKeySet( key, &keySetSingle );

		if ( IsPGPError(err) )
		{
			break;
		}

		/*
		 * The buffer should come back with the byte after the last
		 * character in the keyblock set to null.  This byte is *not*
		 * included in bufSize, but it does prevent us from doing a gross
		 * new buffer copy just to add a null terminator
		 */

		bufSize  = 0;
		err = PGPExportKeySet( keySetSingle,
					PGPOAllocatedOutputBuffer( keyServer->myContext,
						(void**)&tempKeyBuffer, MAX_PGPSize, &bufSize ),
					PGPOLastOption( keyServer->myContext ) );

		if ( IsPGPError(err) )
		{
			break;
		}

		keyBuffer = (PGPByte*) pgpContextMemAlloc( keyServer->myContext, 
										bufSize+1, 0 );

		if (IsNull(keyBuffer))
		{
			PGPFreeData( tempKeyBuffer );
			tempKeyBuffer = NULL;

			err = kPGPError_OutOfMemory;
			break;
		}

		pgpCopyMemory( tempKeyBuffer, keyBuffer, bufSize );
		keyBuffer[bufSize] = '\0';

		PGPFreeData( tempKeyBuffer );
		tempKeyBuffer = NULL;

		err = pgpEventKeyServer( keyServer->myContext, 
							&newOptionList,
							callBack, callBackArg, 
							kPGPKeyServerStateSend, 
							currentRecord, totalKeys );

		if ( IsPGPError(err) )
		{
			break;
		}

		err = pgpLDAPAddKey( keyServer, keyBuffer,
						callBack,  callBackArg,
						currentRecord, totalKeys );

		if ( err == kPGPError_ServerAuthorizationRequired )
		{
			/*
			 * Now, build a request containing all of the keyblocks
			 * and send it as a signed request to the server
			 */

			request = NULL;

			err = pgpLDAPBuildRequest( keyServer,
									"add",
									keysToUpload,
									callBack,
									callBackArg,
									&request );
			if (IsPGPError(err))
			{
				break;
			}
			else
			{
				err = pgpLDAPSendRequest( keyServer, keysToUpload,
								request, callBack, callBackArg, 
								keysThatFailed );

				if ( request != NULL )
				{
					PGPFreeData( request );
					request = NULL;
				}
				break;
			}

			if ( IsPGPError(err) )
			{
				PGPAddKeys( keySetSingle, keySetError );
				partialFailure = TRUE;
			}
		}
		else if ( IsPGPError(err) )
		{
			PGPAddKeys( keySetSingle, keySetError );

			if ( err == kPGPError_ServerKeyAlreadyExists )
			{
				keyAlreadyExists = TRUE;
			}
			else if (err == kPGPError_ServerKeyFailedPolicy )
			{
				keyFailedPolicy = TRUE;
			}
			partialFailure = TRUE;
		}

		pgpContextMemFree( keyServer->myContext, keyBuffer );
		keyBuffer = NULL;

		PGPFreeKeySet( keySetSingle );
		keySetSingle = NULL;

		currentRecord++;
	}	

	if ( err == kPGPError_EndOfIteration )
	{
		err = kPGPError_NoErr;
	}

done:

	if ( request != NULL )
	{
		PGPFreeData( request );
	}

	if ( keySetSingle != NULL )
	{
		PGPFreeKeySet( keySetSingle );
	}

	if ( keyBuffer != NULL )
	{
		pgpContextMemFree( keyServer->myContext, keyBuffer );
	}

	if ( keyList != NULL )
	{
		PGPFreeKeyList( keyList );
	}

	if ( keyIter != NULL )
	{
		PGPFreeKeyIter( keyIter );
	}

	if ( mustCloseLDAP == TRUE )
	{
		pgpLDAPCloseKeyServer( keyServer, callBack, callBackArg);
	}

	if ( partialFailure == TRUE || IsPGPError(err) )
	{
		*keysThatFailed = keySetError;

		/*
		 * most interested in:
		 * 1) failed policy
		 * 2) already exists
		 * 3) general failure
		 */

		if ( keyFailedPolicy == TRUE )
		{
			err = kPGPError_ServerKeyFailedPolicy;
		} 
		else if ( keyAlreadyExists == TRUE )
		{
			err = kPGPError_ServerKeyAlreadyExists;
		}
		else if (IsntPGPError(err))
		{
			/*
			 * general add failure
			 */
			err = kPGPError_ServerPartialAddFailure;
		}
		return err;
	}
	else
	{
		/*
		 * complete success
		 */

		PGPFreeKeySet( keySetError );
		*keysThatFailed = NULL;

		pgpAssertErrWithPtr( err, keysThatFailed );
		return err;
	}
}

/*
 * Determine if the search the search request was processed successfully
 */

static PGPError
pgpLDAPGetResults( 
	PGPKeyServerRef		keyServer,
	PGPInt32			msgId,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPBoolean			*successFlag )
{
	PGPError			err			  = kPGPError_NoErr;
	PGPInt32			rc;
	PGPSocketsTimeValue	tm;
	PGPOptionListRef	newOptionList;
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( successFlag );

	*successFlag = FALSE;

	tm.tv_sec  = 0;
	tm.tv_usec = 100;		/* 100 ms wait time? */

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateWait;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	while (TRUE) 
	{
	
		rc = ldap_result( keyServer->ldap,
					msgId, 1, (struct timeval *) &tm, 
					&keyServer->ldapResult );


		if ( rc == LDAP_RES_SEARCH_RESULT )
		{
			*successFlag = TRUE;
			break;
		} 
		else if ( rc == -1 )
		{
			pgpSaveLDAPError( keyServer );
			if ( keyServer->errorNum == LDAP_USER_CANCELLED )
			{
				err = kPGPError_UserAbort;
			}
			else
			{
				err = kPGPError_ServerSearchFailed;
			}
			break;
		}

		err = pgpEventKeyServer( keyServer->myContext, &newOptionList, 
							callBack, callBackArg, 
							kPGPKeyServerStateWait, 
							MAX_PGPUInt32, MAX_PGPUInt32 );

		if ( IsPGPError(err) )
		{
			ldap_abandon( keyServer->ldap, msgId );
			break;
		}
	};

	if ( *successFlag == TRUE )
	{
		/*
		 * Now, check the return code of the Message structure to 
		 * ensure it worked
		 */

		pgpSaveLDAPError( keyServer );

		if ( keyServer->errorNum == 0 )
		{
			err = kPGPError_NoErr;
		}
		else if ( keyServer->errorNum == LDAP_INSUFFICIENT_ACCESS )
		{
			err = kPGPError_ServerAuthorizationFailed;
		}
		else if ( keyServer->errorNum == LDAP_SIZELIMIT_EXCEEDED  ||
				  keyServer->errorNum == LDAP_TIMELIMIT_EXCEEDED)
		{
			err = kPGPError_ServerTooManyResults;
		}
		else 
		{
			err = kPGPError_ServerSearchFailed;
		}
	}


#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif
	return err;
}

/*
 * Process the results of the LDAP search, and add the returned keys (if any)
 * to the keyset
 */

static PGPError
pgpLDAPProcessSearchResults( 
	PGPKeyServerRef		keyServer,
	PGPInt32			msgId,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*resultSet )
{
	PGPKeySetRef		keySetMaster		= NULL;
	PGPError			err					= kPGPError_NoErr;
	PGPOptionListRef	newOptionList		= NULL;
	PGPUInt32			resultCount			= 0;
	PGPUInt32			currentRecord		= 1;
	LDAPMessage			*nextMessage		= NULL;
	char				*attribute			= NULL;
	char				**value				= NULL;
	BerElement			*iterAttribute		= NULL;
	PGPKeySetRef		keySetAdd			= NULL;
	PGPBoolean			badKeyReceived		= FALSE;
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( resultSet );


	*resultSet      = NULL;

	/*
	 * Create a new KeySet to store the accumulated keys
	 */

	err = PGPNewKeySet( keyServer->myContext, &keySetMaster );

	if ( IsPGPError(err) ) 
	{
		if ( keyServer->ldapResult != NULL ) {
			ldap_msgfree( keyServer->ldapResult );
			keyServer->ldapResult = NULL;
		}
		err = kPGPError_OutOfMemory;
		goto done;
	}

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateReceive;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	/*
	 * Count the number of records returned...may be useful data
	 * during a callback!
	 */

	resultCount = ldap_count_entries( keyServer->ldap, 
								keyServer->ldapResult );

	if ( resultCount < 1 )
	{
		/*
		 * A totally fruitless search
		 */

		if ( keyServer->ldapResult != NULL ) {
			ldap_msgfree( keyServer->ldapResult );
			keyServer->ldapResult = NULL;
		}
		*resultSet = keySetMaster;

		/*
		 * abort the transaction, just in case of ldap leaks
		 */
		ldap_abandon( keyServer->ldap, msgId );
		
		err = kPGPError_NoErr;
		goto done;
	}

	/*
	 * Start at the beginning of the results record
	 */

	nextMessage		  = ldap_first_entry( keyServer->ldap, 
									keyServer->ldapResult );

	/*
	 * Assume the worst
	 */

	err = kPGPError_ServerSearchFailed;

	/*
	 * Enumerate through the LDAP result message records
	 */

	while ( nextMessage != NULL )
	{
		err = pgpEventKeyServer( keyServer->myContext, 
							&newOptionList,
							callBack, callBackArg, 
							kPGPKeyServerStateReceive, 
							currentRecord, resultCount );

		if ( IsPGPError(err) )
		{
#if PGP_CALLBACK
			callBackData.callBackState	= kPGPKeyServerStateSend;
#endif
			ldap_abandon( keyServer->ldap, msgId );
			break;
		}

		/*
		 * Start at the first attribute:
		 * - we've only asked for pgpkey, but doing it this
		 *   way will make it easier for someone to add the support
		 *   for processing other attributes in the future.
		 */

		attribute = ldap_first_attribute( keyServer->ldap, 
										nextMessage, 
										&iterAttribute );

		/*
		 * Enumerate throught each attribute of the current record
		 */

		while ( attribute != NULL )
		{
			/*
			 * Get the values for this attribute
			 * - pgpkey is a single value so we will only do this once.
			 *   But, note above, you could add other attribute types
			 */

			value = ldap_get_values( keyServer->ldap, 
								nextMessage, attribute );

			if ( value != NULL ) 
			{
				if ( *value != NULL &&
					! pgpCompareStringsIgnoreCase( attribute, "pgpkey" ) )
				{
					/*
					 * Import this keyblock into the transient keyset
					 */

					err = PGPImportKeySet(
								keyServer->myContext,
								&keySetAdd,
								PGPOInputBuffer( keyServer->myContext, 
										*value, strlen(*value)),
								PGPOLastOption(keyServer->myContext) );

					if ( IsntPGPError(err) )
					{

						/*
						 * Add the transient keyset to the master keyset
						 */

						err = PGPAddKeys( keySetAdd, keySetMaster );

						if (IsntPGPError(err))
						{
							err = PGPCommitKeyRingChanges( keySetMaster );
						}
						PGPFreeKeySet( keySetAdd );
						keySetAdd = NULL;
					}
					else
					{
						/*
						 * There was something wrong with this key block
						 * so, continue, and remember for later that all
						 * did not go perfectly with this search
						 */

						badKeyReceived = TRUE;
						err = kPGPError_NoErr;
					}
				}
				ldap_value_free( value );
				value = NULL;

				if ( IsPGPError(err) )
				{
					break;
				}

			}

			attribute = ldap_next_attribute( keyServer->ldap, 
										nextMessage, 
										iterAttribute );
		}
		if ( IsPGPError(err) )
		{
			break;
		}

		currentRecord++;
		nextMessage = ldap_next_entry( keyServer->ldap, nextMessage );
	}

	*resultSet = keySetMaster;

done:

	if ( keySetAdd != NULL )
	{
		PGPFreeKeySet( keySetAdd );
	}

	if ( value != NULL )
	{
		ldap_value_free( value );
	}

	if ( keyServer->ldapResult != NULL ) {
		ldap_msgfree( keyServer->ldapResult );
		keyServer->ldapResult = NULL;
	}

#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif

	if (IsPGPError(err) && keySetMaster != NULL)
	{
		PGPFreeKeySet( keySetMaster );
		*resultSet = NULL;
	}
	if ( err == kPGPError_NoErr && badKeyReceived == TRUE )
	{
		err = kPGPError_ServerBadKeysInSearchResults;
	}
	return err;
}	

/*
 * Process the search results for the DN request of the key spaces
 */

static PGPError
pgpLDAPProcessInitResults( 
	PGPKeyServerRef		keyServer,
	PGPInt32			msgId,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg)
{
	PGPError			err			  = kPGPError_NoErr;
	PGPOptionListRef	newOptionList;
	PGPUInt32			resultCount			= 0;
	PGPUInt32			currentRecord		= 1;
	LDAPMessage			*nextMessage		= NULL;
	char				*attribute			= NULL;
	char				**value				= NULL;
	BerElement			*iterAttribute		= NULL;
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateWait;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	/*
	 * Count the number of records returned...may be useful data
	 * during a callback!
	 */

	resultCount = ldap_count_entries( keyServer->ldap, 
								keyServer->ldapResult );

	if (resultCount < 1)
	{
#if PGP_CALLBACK
		PGPSetCallback( 
				(PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL );
#endif
		if ( keyServer->ldapResult != NULL ) 
		{
			ldap_msgfree( keyServer->ldapResult );
			keyServer->ldapResult = NULL;
		}

		err = kPGPError_ServerSearchFailed;
		goto done;
	}

	/*
	 * Start at the beginning of the results record
	 */

	nextMessage	  = ldap_first_entry( keyServer->ldap, 
									keyServer->ldapResult );

	/*
	 * Assume the worst
	 */

	err = kPGPError_ServerSearchFailed;

	/*
	 * Enumerate through the LDAP result message records
	 */

	while ( nextMessage != NULL )
	{
		err = pgpEventKeyServer( keyServer->myContext, 
							&newOptionList,
							callBack, callBackArg, 
							kPGPKeyServerStateWait, 
							currentRecord, resultCount );

		if ( IsPGPError(err) )
		{
			ldap_abandon( keyServer->ldap, msgId );
			break;
		}
		attribute = ldap_first_attribute( keyServer->ldap, 
										nextMessage, 
										&iterAttribute );

		/*
		 * Enumerate throught each attribute of the current record
		 */

		while ( attribute != NULL )
		{
			value = ldap_get_values( keyServer->ldap, 
								nextMessage, attribute );

			if ( value != NULL ) 
			{
				if ( *value != NULL && 
					 pgpCompareStringsIgnoreCase( attribute,
					 	"baseKeyspaceDN" ) == 0 )
				{
					keyServer->baseKeySpaceDN = 
									(char*)pgpContextMemAlloc( 
													keyServer->myContext,
													strlen(*value)+1,
													kPGPMemoryFlags_Clear);

					if ( keyServer->baseKeySpaceDN == NULL )
					{
						err = kPGPError_OutOfMemory;
						break;
					}
					strcpy(keyServer->baseKeySpaceDN, *value);
				}

				if ( *value != NULL && 
					 pgpCompareStringsIgnoreCase( attribute,
					 	"basePendingDN" ) == 0)
				{
					keyServer->basePendingDN = 
									(char*)pgpContextMemAlloc( 
													keyServer->myContext,
													strlen(*value)+1,
													kPGPMemoryFlags_Clear );

					if ( keyServer->basePendingDN == NULL )
					{
						err = kPGPError_OutOfMemory;
						break;
					}
					strcpy(keyServer->basePendingDN, *value);
				}

				ldap_value_free( value );
				value = NULL;

				if ( IsPGPError(err) )
				{
					break;
				}
			}

			attribute = ldap_next_attribute( keyServer->ldap, 
										nextMessage, 
										iterAttribute );
		}
		if ( IsPGPError(err) )
		{
			break;
		}

		currentRecord++;
		nextMessage = ldap_next_entry( keyServer->ldap, nextMessage );
	}

done:

#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif

	if ( value != NULL )
	{
		ldap_value_free( value );
		value = NULL;
	}

	if ( keyServer->ldapResult != NULL ) {
		ldap_msgfree( keyServer->ldapResult );
		keyServer->ldapResult = NULL;
	}

	if ( IsPGPError(err) )  
	{
		if ( keyServer->baseKeySpaceDN != NULL )
		{
			pgpContextMemFree( keyServer->myContext, keyServer->baseKeySpaceDN);
			keyServer->baseKeySpaceDN = NULL;
		}
		if ( keyServer->basePendingDN != NULL )
		{
			pgpContextMemFree( keyServer->myContext, keyServer->basePendingDN);
			keyServer->basePendingDN = NULL;
		}
	}
	return err;
}	

	static PGPError
pgpLDAPProcessMonitorResults( 
	PGPKeyServerRef		keyServer,
	PGPInt32			msgId,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeyServerMonitorRef *monitor )
{
	PGPError			err					= kPGPError_NoErr;
	PGPOptionListRef	newOptionList;
	PGPUInt32			resultCount			= 0;
	PGPUInt32			valueCount			= 0;
	PGPUInt32			currentRecord		= 1;
	LDAPMessage			*nextMessage		= NULL;
	char				*attribute			= NULL;
	char				**value				= NULL;
	char				**headValue			= NULL;
	BerElement			*iterAttribute		= NULL;
	PGPKeyServerMonitorRef	currentMonitor	= NULL;
	PGPKeyServerMonitorRef	newMonitor		= NULL;
	char				**newValue			= NULL;
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateWait;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	/*
	 * Count the number of records returned...may be useful data
	 * during a callback!
	 */

	resultCount = ldap_count_entries( keyServer->ldap, 
								keyServer->ldapResult );

	if (resultCount < 1)
	{
#if PGP_CALLBACK
		PGPSetCallback( 
			(PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif
		if ( keyServer->ldapResult != NULL ) 
		{
			ldap_msgfree( keyServer->ldapResult );
			keyServer->ldapResult = NULL;
		}

		err = kPGPError_ServerSearchFailed;
		goto done;
	}

	/*
	 * Start at the beginning of the results record
	 */

	nextMessage	  = ldap_first_entry( keyServer->ldap, 
									keyServer->ldapResult );

	*monitor = NULL;

	/*
	 * Assume the worst
	 */

	err = kPGPError_ServerSearchFailed;

	/*
	 * Enumerate through the LDAP result message records
	 */

	while ( nextMessage != NULL )
	{
		err = pgpEventKeyServer( keyServer->myContext, 
							&newOptionList,
							callBack, callBackArg, 
							kPGPKeyServerStateWait, 
							currentRecord, resultCount );

		if ( IsPGPError(err) )
		{
			ldap_abandon( keyServer->ldap, msgId );
			break;
		}
		attribute = ldap_first_attribute( keyServer->ldap, 
										nextMessage, 
										&iterAttribute );

		currentMonitor = NULL;

		/*
		 * Enumerate through each attribute of the current record
		 */

		while ( attribute != NULL )
		{
			newMonitor = 
				(PGPKeyServerMonitorRef) pgpContextMemAlloc( 
												keyServer->myContext,
												sizeof(PGPKeyServerMonitor),
												kPGPMemoryFlags_Clear);


			if ( newMonitor == NULL )
			{
				err = kPGPError_OutOfMemory;
				break;
			}

			newMonitor->magic	= kPGPKeyServerMonitorMagic;

			if ( *monitor == NULL )
			{
				/*
				 * this is the head of the list
				 */
				*monitor       = newMonitor;
				currentMonitor = newMonitor;
			}
			else
			{
				currentMonitor->next = newMonitor;
				currentMonitor = newMonitor;
			}

			newMonitor->monitorTag = (char*) pgpContextMemAlloc( 
													keyServer->myContext,
													strlen(attribute)+1, 
													kPGPMemoryFlags_Clear);

			if ( newMonitor->monitorTag == NULL )
			{
				err = kPGPError_OutOfMemory;
				break;
			}

			strcpy( newMonitor->monitorTag, attribute );


			value = ldap_get_values( keyServer->ldap, 
								nextMessage, attribute );

			headValue = value;

			if ( value != NULL )
			{
				valueCount = ldap_count_values( value  );
				newValue = (char**) pgpContextMemAlloc(
											keyServer->myContext,
											(valueCount+1)*sizeof(char *), 
											kPGPMemoryFlags_Clear );

				if ( newValue == NULL )
				{
					err = kPGPError_OutOfMemory;
					break;
				}

				newValue[valueCount] = NULL;
				currentMonitor->monitorValues = newValue;

				while ( *value != NULL )
				{
					/*
					 * Allocate a new value string
					 */

					*newValue = (char*) pgpContextMemAlloc( 
													keyServer->myContext, 
													strlen(*value)+1, 
													kPGPMemoryFlags_Clear);

					if ( *newValue == NULL )
					{
						err = kPGPError_OutOfMemory;
						break;
					}
					strcpy( *newValue, *value );

					newValue++;
					value++;
				}

				ldap_value_free( headValue );
				headValue = NULL;

				if ( IsPGPError(err) )
				{
					break;
				}
			}

			attribute = ldap_next_attribute( keyServer->ldap, 
										nextMessage, 
										iterAttribute );
		}
		
		if ( IsPGPError(err) )
		{
			break;
		}

		currentRecord++;
		nextMessage = ldap_next_entry( keyServer->ldap, nextMessage );
	}

done:

#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif

	if ( keyServer->ldapResult != NULL ) {
		ldap_msgfree( keyServer->ldapResult );
		keyServer->ldapResult = NULL;
	}

	if ( headValue != NULL )
	{
		ldap_value_free( headValue );
	}

	if ( IsPGPError(err) )
	{
		/*
		 * we must dispose of the entire list
		 */

		PGPLDAPFreeServerMonitor( keyServer, *monitor );
		*monitor = NULL;
	}
		
	pgpAssertErrWithPtr( err, *monitor );
	return err;
}	

static PGPError 
pgpLDAPAddKey( 
	PGPKeyServerRef		keyServer, 
	PGPByte				*keyBuffer,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPUInt32			currentRecord,
	PGPUInt32			totalKeys )

{
	PGPError			err				= kPGPError_NoErr;
	Boolean				successResult	= FALSE;
	LDAPMod				*attrs[2];
	PGPOptionListRef	newOptionList;
	char				*strVals[2];
	char				*keySpace		= NULL;
	PGPInt32			msgId;
	const char			*prefix			= "pgpcertid=virtual, ";
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( keyBuffer );

	attrs[1] = NULL;
	attrs[0] = (LDAPMod*) pgpContextMemAlloc( keyServer->myContext,
										sizeof(LDAPMod), 
										kPGPMemoryFlags_Clear );


	if ( attrs[0] == NULL ) 
	{
		return kPGPError_OutOfMemory;
	}

	strVals[0] = (char*) keyBuffer;
	strVals[1] = NULL;

	attrs[0]->mod_op		= LDAP_MOD_ADD;
	attrs[0]->mod_type		= (char*)"PGPkey";
	attrs[0]->mod_values	= strVals;
	attrs[0]->mod_next		= NULL;

	if ( keyServer->keySpace == kPGPKSKeySpaceNormal ||
	 	keyServer->keySpace == kPGPKSKeySpaceDefault )
	{
		keySpace = (char *) pgpContextMemAlloc( 
										keyServer->myContext,
										strlen(prefix) + 
										strlen(keyServer->baseKeySpaceDN)+1,
										kPGPMemoryFlags_Clear );
		if ( keySpace == NULL )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}

		strcpy( keySpace, prefix );
		strcat( keySpace, keyServer->baseKeySpaceDN );
	}
	else
	{
		keySpace = (char *) pgpContextMemAlloc( 
										keyServer->myContext,
										strlen(prefix) + 
										strlen(keyServer->basePendingDN)+1,
										kPGPMemoryFlags_Clear );
		if ( keySpace == NULL )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}

		strcpy( keySpace, prefix );
		strcat( keySpace, keyServer->basePendingDN );
	}

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateSend;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	msgId = ldap_add( keyServer->ldap, 
					  keySpace, 
					  attrs );

	if ( msgId != -1 )
	{
		PGPSocketsTimeValue	tm;
		int				rc;

		tm.tv_sec  = 0;
		tm.tv_usec = 100;		// 100 ms wait time?

#if PGP_CALLBACK
		callBackData.callBackState	= kPGPKeyServerStateWait;
#endif

		while (TRUE) {

			rc = ldap_result( keyServer->ldap,
							msgId, 1, (struct timeval *) &tm, 
							&keyServer->ldapResult );


			if ( rc == LDAP_RES_ADD )
			{
				successResult = TRUE;
				break;
			}
			else if ( rc == -1 )
			{
				pgpSaveLDAPError( keyServer );
				if ( keyServer->errorNum == LDAP_USER_CANCELLED )
				{
					err = kPGPError_UserAbort;
				}
				else
				{
					err = kPGPError_ServerAddFailed;
				}
				break;
			}

			err = pgpEventKeyServer( keyServer->myContext, &newOptionList, 
								callBack, callBackArg, 
								kPGPKeyServerStateWait, 
								currentRecord, totalKeys );

			if ( IsPGPError(err) )
			{
				ldap_abandon( keyServer->ldap, msgId );
				break;
			}
		};
	}
	else 
	{
		err = kPGPError_ServerAddFailed;
	}

done:

	if ( successResult == TRUE )
	{
		/*
		 * Now, check the return code of the Message structure 
		 * to ensure it worked
		 */

		pgpSaveLDAPError( keyServer );

		if ( keyServer->errorNum == 0 )
		{
			err = kPGPError_NoErr;
		}
		else if ( keyServer->errorNum == LDAP_ALREADY_EXISTS )
		{
			err = kPGPError_ServerKeyAlreadyExists;
		}
		else if ( keyServer->errorNum == LDAP_INSUFFICIENT_ACCESS )
		{
			err = kPGPError_ServerAuthorizationFailed;
		}
		else if ( keyServer->errorNum == LDAP_INVALID_CREDENTIALS )
		{
			err = kPGPError_ServerKeyFailedPolicy;
		} 
		else 
		{
			err = kPGPError_ServerAddFailed;
		}
	}


	/*
	 * Delete the returned message, and free our buffers
	 */

	if ( keyServer->ldapResult != NULL ) {
		ldap_msgfree( keyServer->ldapResult );
		keyServer->ldapResult = NULL;
	}
	
	pgpContextMemFree( keyServer->myContext, attrs[0] );
	pgpContextMemFree( keyServer->myContext, keySpace );

#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif

	return err;
}

PGPError
pgpLDAPbind( PGPKeyServerRef keyServer,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg )
{
	int							err;
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );

	/*
	 * Args used
	 */

	callBack    = callBack;
	callBackArg = callBackArg;

	/*
	 * This functionality should be rolled into the PGPKeyServerRef
	 * structure.
	 */

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateConnect;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	err = ldap_bind_s( keyServer->ldap, 
				(char*) "cn=root, o=PGP keyspace, c=us", 
				(char*) "password", LDAP_AUTH_SIMPLE ); 


#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif

	if ( err == -1 )
	{
		if ( keyServer->errorNum == 0 )
		{
			/*
			 * We're not stomping over a previous error
			 */

			pgpSaveLDAPError( keyServer );
		}

		return kPGPError_ServerBindFailed;
	}
	else
	{
		return kPGPError_NoErr;
	}
}

PGPError 
pgpLDAPOpenKeyServer( 
	PGPKeyServerRef		keyServer,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg)
{
	PGPError					err = kPGPError_NoErr;
	PGPOptionListRef			newOptionList;
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );

	err = pgpEventKeyServer( keyServer->myContext, &newOptionList, 
					callBack, callBackArg, 
					kPGPKeyServerStateConnect, 
					MAX_PGPUInt32, MAX_PGPUInt32 );

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateConnect;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetDefaultCallback(pgpKeyServerSocketCallback, (void*) &callBackData);
	gCallbackDataSocket = &callBackData.socket;

#endif

	keyServer->ldap = ldap_open(keyServer->host, keyServer->port);

#if PGP_CALLBACK
	PGPSetDefaultCallback(NULL, NULL);
	gCallbackDataSocket = NULL;
#endif

	if ( keyServer->ldap == NULL )
	{
		return kPGPError_ServerOpenFailed;
	}

#if PGP_CALLBACK

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	if ( keyServer->baseKeySpaceDN == NULL &&
		 keyServer->basePendingDN == NULL )
	{
		/*
		 * We will only refetch the DN's if we don't already have
		 * them
		 */
		 
		err = pgpLDAPGetBaseDNFromKeyServer(keyServer,
										callBack,
										callBackArg );
	}

#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif

	return err;
}

PGPError 
pgpLDAPCloseKeyServer( 
	PGPKeyServerRef		keyServer,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg)
{
	PGPError			err = kPGPError_NoErr;
	PGPOptionListRef	newOptionList;
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );

	err = pgpEventKeyServer( keyServer->myContext, 
						&newOptionList,
						callBack, callBackArg, 
						kPGPKeyServerStateDisconnect, 
						MAX_PGPUInt32, MAX_PGPUInt32 );

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateDisconnect;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	if ( keyServer->ldap != NULL )
	{
		ldap_unbind( keyServer->ldap );
		keyServer->ldap = NULL;
	}

	/*
	 * The socket is now cleared, so no need to reset the callback
	 */

	if ( keyServer->baseKeySpaceDN != NULL )
	{
		pgpContextMemFree( keyServer->myContext, keyServer->baseKeySpaceDN );
		keyServer->baseKeySpaceDN = NULL;
	}
	if ( keyServer->basePendingDN != NULL )
	{
		pgpContextMemFree( keyServer->myContext, keyServer->basePendingDN );
		keyServer->basePendingDN = NULL;
	}
	return err;
}

static PGPError
pgpLDAPBuildRequest(
	PGPKeyServerRef		keyServer,
	const char			*requestVerb,
	PGPKeySetRef		keysToProcess,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	char				**request)
{
	PGPError			err				= kPGPError_NoErr;
	PGPUInt32			bufferSize		= 10000;
	PGPUInt32			requestLen;
	PGPKeyListRef		keyList			= NULL;
	PGPKeyIterRef		keyIter			= NULL;
	PGPKeyID			keyID;
	PGPKeyRef			key;
	PGPOptionListRef	options			= NULL;
	char				*requestBuffer	= NULL;
	PGPSize				outSize;
	PGPUInt32			keyIDLen;

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( keysToProcess );
	PGPValidatePtr( request );

	*request = NULL;

	requestBuffer = (char*) pgpContextMemAlloc( keyServer->myContext,
										bufferSize+1,
										kPGPMemoryFlags_Clear );

	if ( requestBuffer == NULL) 
	{
		err = kPGPError_OutOfMemory;
		goto done;
	}

	sprintf( requestBuffer, "Request: %s\nLocation: %s\n\n", 
		requestVerb, 
		(keyServer->keySpace == kPGPKSKeySpacePending) ? "pending":"active");

	err = PGPOrderKeySet( keysToProcess, kPGPAnyOrdering, &keyList );

	if ( IsPGPError(err) )
	{
		goto done;
	}

	requestLen = strlen(requestBuffer);

	err = PGPNewKeyIter( keyList, &keyIter );

	if ( IsPGPError(err) )
	{
		goto done;
	}

	while (( err = PGPKeyIterNext(keyIter, &key)) == kPGPError_NoErr)
	{
		char	keyIDBuffer[ 128 ];
	
		err = PGPGetKeyIDFromKey( key, &keyID );
		
		if ( IsPGPError(err) )
		{
			break;
		}

		err = PGPGetKeyIDString( &keyID,
							kPGPKeyIDString_Full,
							keyIDBuffer );
								
		if ( IsPGPError(err) )
		{
			break;
		}
		keyIDLen = strlen(keyIDBuffer);

		if ( keyIDLen+requestLen >= bufferSize )
		{
			bufferSize += 10000;
			err = pgpContextMemRealloc( keyServer->myContext,
									(void**)&requestBuffer, bufferSize+1,
									0 );
			if ( IsPGPError(err) )
			{
				break;
			}
		}
		pgpCopyMemory( keyIDBuffer,
			requestBuffer + requestLen, keyIDLen );

		pgpCopyMemory( "\n", 
			requestBuffer + requestLen + keyIDLen, sizeof("\n"));

		requestBuffer[requestLen+keyIDLen+sizeof("\n")] = '\0';
		requestLen += (keyIDLen + sizeof("\n")-1);
	}	

	if ( err == kPGPError_EndOfIteration )
	{
		do 
		{
			err = pgpEventKeyServerSign( keyServer->myContext, &options,
							callBack, callBackArg, 0 );

			if ( IsPGPError(err) )
			{
				break;
			}

			requestLen = strlen(requestBuffer);

			err = PGPEncode( keyServer->myContext,
					PGPOInputBuffer(keyServer->myContext, 
							requestBuffer, requestLen),
					PGPOAllocatedOutputBuffer(keyServer->myContext, 
							(void**)request, INT_MAX, &outSize),
					options,
					PGPOLastOption(keyServer->myContext));

		} 
		while ( err == kPGPError_BadPassphrase );
	}

done:
	if ( requestBuffer != NULL )
	{
		pgpContextMemFree( keyServer->myContext, requestBuffer );
		requestBuffer = NULL;
	}

	if ( keyList != NULL )
	{
		PGPFreeKeyList( keyList );
	}

	if ( keyIter != NULL )
	{
		PGPFreeKeyIter( keyIter );
	}

	if ( requestBuffer != NULL )
	{
		pgpContextMemFree( keyServer->myContext, requestBuffer );
	}

	return err;
}


static PGPError 
pgpSaveLDAPError( PGPKeyServerRef keyServer )
{
	PGPValidateKeyServer( keyServer );

	keyServer->errorNum = ldap_result2error( keyServer->ldap,
									keyServer->ldapResult, 
									0);

	if ( keyServer->ldap->ld_error != NULL &&
		 *keyServer->ldap->ld_error != '\0' )
	{
		if ( keyServer->error != NULL )
		{
			pgpContextMemFree( keyServer->myContext, keyServer->error );
		}

		keyServer->error = (char*) pgpContextMemAlloc( 
										keyServer->myContext,
										strlen(keyServer->ldap->ld_error)+1,
										kPGPMemoryFlags_Clear );

		if ( keyServer->error == NULL )
		{
			return kPGPError_OutOfMemory;
		}

		strcpy( keyServer->error, keyServer->ldap->ld_error );
	}
	return kPGPError_NoErr;
}
		
static PGPError
pgpScanForErrorKeys( PGPKeyServerRef keyServer, 
				PGPKeySetRef inKeys, 
				PGPKeySetRef *keysThatFailed )
{
	char			*start;
	char			*next;
	PGPKeyID		keyID;
	PGPKeyRef		key;
	PGPKeySetRef	keySetSingle;
	PGPError		err = kPGPError_NoErr;
	PGPBoolean		partialFailure = FALSE;
	PGPPublicKeyAlgorithm	keyAlgorithm;

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( inKeys );
	PGPValidatePtr( keysThatFailed );

	if ( keyServer->error == NULL || *keyServer->error == '\0' )
	{
		return kPGPError_NoErr;
	}

	start = strstr( keyServer->error, ":" );
	
	if ( start == NULL )
	{
		return kPGPError_NoErr;
	}

	start++;

	if ( *start == '\0' )
	{
		return kPGPError_NoErr;
	}

	while ( start != NULL && *start != '\0' )
	{
		/*
		 * Eat white space
		 */

		while ( *start != '\0' && *start == ' ')
		{
			start++;
		}

		if ( *start == '\0' )
		{
			return kPGPError_NoErr;
		}

		/*
		 * Find next keyid, and null terminate this one
		 */

		next = strstr( start, " " );

		if ( next != NULL ) 
		{
			/*
			 * step past the newline
			 */

			*next = '\0';
			next++;
		}
		err = PGPGetKeyIDFromString( start, &keyID );

		if (IsPGPError(err)) 
		{
			partialFailure = TRUE;
			goto done;
		}

		keyAlgorithm = kPGPPublicKeyAlgorithm_First;

		while ( keyAlgorithm <= kPGPPublicKeyAlgorithm_Last )
		{
			err = PGPGetKeyByKeyID( inKeys,
							&keyID,
							keyAlgorithm,
							&key );

			if (IsntPGPError(err) && key != NULL )
			{
				break;
			}
			keyAlgorithm++;
		}

		if (IsPGPError(err))
		{
			partialFailure = TRUE;
			goto done;
		}

		if ( *keysThatFailed == NULL )
		{
			err = PGPNewKeySet( keyServer->myContext, keysThatFailed );

			if ( IsPGPError(err) ) 
			{
				partialFailure = TRUE;
				goto done;
			}
		}

		err = PGPNewSingletonKeySet( key, &keySetSingle );

		if ( IsPGPError(err) )
		{
			partialFailure = TRUE;
			goto done;
		}

		err = PGPAddKeys( keySetSingle, *keysThatFailed );

done:
		if ( keySetSingle != NULL )
		{
			PGPFreeKeySet( keySetSingle );
			keySetSingle = NULL;
		}
		start = next;
	}

	return err;
}

static PGPError
pgpLDAPSendRequest( 
	PGPKeyServerRef		keyServer,
	PGPKeySetRef		keys,
	char				*request,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*keysThatFailed)
{
	PGPError			err				= kPGPError_NoErr;
	PGPBoolean			successResult	= FALSE;
	LDAPMod				*attrs[2];
	PGPOptionListRef	newOptionList;
	char				*strVals[2];
	char				*keySpace		= NULL;
	const char			*prefix			= "pgpcertid=virtual, ";
	PGPInt32			msgId;
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( keys );
	PGPValidatePtr( request );
	PGPValidatePtr( keysThatFailed );

	attrs[1] = NULL;
	attrs[0] = (LDAPMod*) pgpContextMemAlloc( keyServer->myContext,
										sizeof(LDAPMod), 
										kPGPMemoryFlags_Clear );


	if ( attrs[0] == NULL ) 
	{
		pgpContextMemFree( keyServer->myContext, request );
		return kPGPError_OutOfMemory;
	}

	strVals[0] = (char*) request;
	strVals[1] = NULL;

	attrs[0]->mod_op		= LDAP_MOD_ADD;
	attrs[0]->mod_type		= (char*) "PGPrequest";
	attrs[0]->mod_values	= strVals;
	attrs[0]->mod_next		= NULL;

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateSend;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	keySpace = (char *) pgpContextMemAlloc( 
									keyServer->myContext,
									strlen(prefix) + 
									strlen(keyServer->baseKeySpaceDN)+1,
									kPGPMemoryFlags_Clear );
	if ( keySpace == NULL )
	{
		pgpContextMemFree( keyServer->myContext, attrs );
		return kPGPError_OutOfMemory;
	}

	strcpy( keySpace, prefix );
	strcat( keySpace, keyServer->baseKeySpaceDN );

	/*
	 * Send the request to the server
	 */

	msgId = ldap_add( keyServer->ldap, 
				keySpace, 
				attrs );

	if ( msgId != -1 )
	{
		PGPSocketsTimeValue	tm;
		int				rc;

		tm.tv_sec  = 0;
		tm.tv_usec = 100;		// 100 ms wait time?

#if PGP_CALLBACK
		callBackData.callBackState	= kPGPKeyServerStateReceive;
#endif

		/*
		 * Wait here until the server responds with either success
		 * or failure
		 */

		while (TRUE) {

			rc = ldap_result( keyServer->ldap,
							msgId, 1, (struct timeval *) &tm, 
							&keyServer->ldapResult );


			if ( rc == LDAP_RES_ADD )
			{
				successResult = TRUE;
				break;
			}
			else if ( rc == -1 )
			{
				/*
				 * The add request failed
				 */

				if ( keyServer->errorNum == LDAP_USER_CANCELLED )
				{
					err = kPGPError_UserAbort;
				}
				else
				{
					err = kPGPError_ServerRequestFailed;
				}
				pgpSaveLDAPError( keyServer );
				break;
			}

			err = pgpEventKeyServer( keyServer->myContext, &newOptionList, 
								callBack, callBackArg, 
								kPGPKeyServerStateWait, 
								MAX_PGPUInt32, MAX_PGPUInt32 );

			if ( IsPGPError(err) )
			{
				/*
				 * User has requested a cancellation
				 */

				ldap_abandon( keyServer->ldap, msgId );
				err = kPGPError_UserAbort;
				break;
			}
		};
	}
	else
	{
		err = kPGPError_ServerRequestFailed;
	}

	if ( successResult == TRUE )
	{
		/*
		 * Now, check the return code of the Message structure 
		 * to ensure it worked
		 */

		pgpSaveLDAPError( keyServer );

		if ( keyServer->errorNum != 0 )
		{
			/*
			 * Look in the error string, and build a keyset of keys
			 * that the server could not deal with
			 */

			pgpScanForErrorKeys( keyServer, keys, keysThatFailed );

			if ( keyServer->errorNum == LDAP_ALREADY_EXISTS )
			{
				err = kPGPError_ServerKeyAlreadyExists;
			}
			else if ( keyServer->errorNum == LDAP_INSUFFICIENT_ACCESS )
			{
				err = kPGPError_ServerAuthorizationFailed;
			}
			else if ( keyServer->errorNum == LDAP_INVALID_CREDENTIALS )
			{
				err = kPGPError_ServerKeyFailedPolicy;
			} 
			else 
			{
				err = kPGPError_ServerRequestFailed;
			}
		}
		else
		{
			err = kPGPError_NoErr;
		}
	}

	/*
	 * Delete the returned message, and free our buffers
	 */

	if ( keyServer->ldapResult != NULL ) {
		ldap_msgfree( keyServer->ldapResult );
		keyServer->ldapResult = NULL;
	}

	if ( keySpace != NULL )
	{
		pgpContextMemFree( keyServer->myContext, keySpace );
	}
	if ( attrs[0] != NULL )
	{
		pgpContextMemFree( keyServer->myContext, attrs[0] );
	}
	return err;
}

	static PGPError 
pgpLDAPNewServerMonitor( 
	PGPKeyServerRef		keyServer,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeyServerMonitorRef *monitor )
{
	PGPError	err = kPGPError_NoErr;
	PGPInt32	msgId;
	PGPBoolean	mustCloseLDAP = FALSE;
	char		*attr[1];
#if PGP_CALLBACK
	PGPKeyServerCallbackData	callBackData;
#endif

	PGPValidatePtr( monitor );
	*monitor	= NULL;
	PGPValidateKeyServer( keyServer );

	if ( keyServer->ldap == NULL )
	{
		/*
		 * The caller has not opened an LDAP connection for us,
		 * so we'll do it, and clean it up later
		 */

		err = pgpLDAPOpenKeyServer( keyServer, callBack, callBackArg );

		if (IsPGPError(err))
		{
			return err;
		}
		mustCloseLDAP = TRUE;
	}

	/*
	 * Access type *must* be Administrator for this operation
	 */

	if (keyServer->accessType != kPGPKSAccess_Administrator)
	{
		return kPGPError_ServerAuthorizationRequired;
	}

	attr[0] = NULL;

#if PGP_CALLBACK
	callBackData.callBack		= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState	= kPGPKeyServerStateSend;
	callBackData.socket			= NULL;
	callBackData.soFar			= MAX_PGPUInt32;
	callBackData.total			= MAX_PGPUInt32;

	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, 
				pgpKeyServerSocketCallback,
				(void *) &callBackData);
#endif

	msgId = ldap_search( keyServer->ldap, 
					(char*) "cn=monitor", LDAP_SCOPE_BASE,
					(char*) "(objectclass=*)", attr, 0 ); 

#if PGP_CALLBACK
	PGPSetCallback( (PGPSocketRef) keyServer->ldap->ld_sb.sb_sd, NULL, NULL);
#endif
	if ( msgId != -1 )
	{
		PGPBoolean	successFlag = FALSE;

		err = pgpLDAPGetResults( keyServer,
							msgId, callBack,
							callBackArg, 
							&successFlag );

		if (IsntPGPError(err) && successFlag == TRUE)
		{
			err = pgpLDAPProcessMonitorResults( keyServer,
										msgId, 
										callBack,
										callBackArg,
										monitor );
		}
		else
		{
			err = kPGPError_ServerSearchFailed;
		}
	}
	if ( mustCloseLDAP == TRUE )
	{
		pgpLDAPCloseKeyServer( keyServer, callBack, callBackArg);
	}
	return err;
}


	PGPError 
PGPLDAPNewServerMonitor( 
	PGPKeyServerRef		keyServer,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeyServerMonitorRef *monitor )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( monitor );
	*monitor	= NULL;
	PGPValidateKeyServer( keyServer );
	
	err	= pgpLDAPNewServerMonitor( keyServer,
			callBack, callBackArg, monitor );
	
	pgpAssertErrWithPtr( err, *monitor );
	return err;
}


	PGPError 
PGPLDAPFreeServerMonitor( 
	PGPKeyServerRef keyServer,
	PGPKeyServerMonitorRef monitor )
{
	PGPKeyServerMonitorRef	currentMonitor	= NULL;
	PGPKeyServerMonitorRef	nextMonitor		= NULL;
	char					**value			= NULL;
	char					**currentValue	= NULL;

	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( monitor );
	PGPValidateParam( monitor->magic == kPGPKeyServerMonitorMagic );
	
	currentMonitor = monitor;

	while ( currentMonitor != NULL )
	{
		PGPValidateParam( currentMonitor->magic == kPGPKeyServerMonitorMagic );
		
		nextMonitor = currentMonitor->next;
		value = currentMonitor->monitorValues;
		currentValue = value;

		while ( *currentValue != NULL )
		{
			pgpContextMemFree( keyServer->myContext, *currentValue );
			currentValue++;
		}
		if ( currentMonitor->monitorValues != NULL ) {
			pgpContextMemFree( keyServer->myContext, 
						currentMonitor->monitorValues );
		}
		if ( currentMonitor->monitorTag != NULL )
		{
			pgpContextMemFree( keyServer->myContext,
						currentMonitor->monitorTag );
		}
		pgpContextMemFree( keyServer->myContext, currentMonitor );
		currentMonitor = nextMonitor;
	}
	return kPGPError_NoErr;
}






