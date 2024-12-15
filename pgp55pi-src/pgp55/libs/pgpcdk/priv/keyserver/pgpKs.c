/*____________________________________________________________________________
	pgpKs.c
	
	Copyright(C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGP Key Server LDAP/HTTP Library implementation

	$Id: pgpKs.c,v 1.35.6.2 1997/11/07 05:26:12 markm Exp $
____________________________________________________________________________*/

#include <string.h>
#include "pgpConfig.h"
#include "pgpContext.h"
#include "pgpKeyServerPriv.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpEncode.h"
#include "pgpEventPriv.h"

#include "pgpStrings.h"



	PGPBoolean
pgpKeyServerIsValid( PGPKeyServer const * keyServer )
{
	return( IsntNull( keyServer ) &&
		(keyServer->serverType == kPGPKeyServerLDAP ||
			keyServer->serverType == kPGPKeyServerHTTP ) 
		);
}



PGPError 
PGPKeyServerInit()
{
	short		result;
	PGPError	err	= kPGPError_NoErr;
	
	result	= PGPSocketsInit();
	pgpAssert( result == 0 );
	if ( result != 0 )
		err	= kPGPError_UnknownError;
		
	return err;
}

PGPError 
PGPKeyServerCleanup()
{
	PGPSocketsCleanup();
	return kPGPError_NoErr;
}

PGPError
PGPKeyServerOpen( PGPKeyServerRef keyServer )
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidateKeyServer( keyServer );

	switch ( keyServer->serverType )
	{
		case kPGPKeyServerLDAP:
		{
			err = pgpLDAPOpenKeyServer( keyServer, 
								NULL, NULL );

			if ( IsntPGPError(err) && 
				( keyServer->baseKeySpaceDN == NULL && 
				  keyServer->basePendingDN == NULL ))
			{
				err = pgpLDAPGetBaseDNFromKeyServer(keyServer,
												NULL,
												NULL );
			}
			break;
		}

		case kPGPKeyServerHTTP:
		case kPGPKeyServerInvalid:
		case kPGPKeyServerType_force:
		{
			break;
		}
	}
	return( err );

}

PGPError
PGPKeyServerClose( PGPKeyServerRef keyServer )
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidateKeyServer( keyServer );

	switch ( keyServer->serverType )
	{
		case kPGPKeyServerLDAP:
		{

			err = pgpLDAPCloseKeyServer( keyServer, 
								NULL, NULL);
			break;
		}

		case kPGPKeyServerHTTP:
		case kPGPKeyServerInvalid:
		case kPGPKeyServerType_force:
		{
			break;
		}
	}
	return( err );
}

	static PGPError 
pgpNewKeyServerFromURL( 
	PGPContextRef	theContext, 
	char const		*url, 
	PGPKeyServerAccessType accessType,
	PGPKeyServerKeySpace keySpace,
	PGPKeyServerRef *keyServer )
{
	/*
	 * The url is of the form:
	 * protocol://host.domain:port
	 */

	PGPKeyServerRef	newKeyServer= NULL;
	char		*localUrl	    = NULL;
	char		*currentItem	= NULL;
	char		*nextItem		= NULL;
	static const char	*protocolSep = "://";
	static const char	*protocolTag[3]				 = { "LDAP", "HTTP", NULL };
	static const PGPKeyServerType protocolType[3]   = { kPGPKeyServerLDAP, 
												 kPGPKeyServerHTTP, 
												 kPGPKeyServerInvalid };
	int			i;
	PGPError	err	= kPGPError_NoErr;

	PGPValidatePtr( keyServer );
	*keyServer	= NULL;
	PGPValidatePtr( theContext );
	PGPValidatePtr( url );
	PGPValidateParam( accessType == kPGPKSAccess_Normal ||
		accessType == kPGPKSAccess_Administrator || 
		accessType == kPGPKSAccess_Default);

	localUrl = (char*) pgpContextMemAlloc( theContext, 
			  					   (strlen(url) + 1),
								   kPGPMemoryFlags_Clear );
	if (!localUrl)
	{
		err	= kPGPError_OutOfMemory;
		goto done;
	}

	strcpy(localUrl, url);

	/*
	 * Create and initialize the new KeyServer object
	 */

	newKeyServer = (PGPKeyServerRef) pgpContextMemAlloc( theContext,
											sizeof(*newKeyServer),
											kPGPMemoryFlags_Clear );
	if (!newKeyServer) 
	{
		err	= kPGPError_OutOfMemory;
		goto done;
	}

	newKeyServer->myContext  = theContext;
	newKeyServer->serverType = kPGPKeyServerInvalid;
	newKeyServer->host		 = (char*) NULL;
	newKeyServer->port		 = kPGPKeyServerPortInvalid;

	newKeyServer->ldap		 = (LDAP*) NULL;
	newKeyServer->ldapResult = (LDAPMessage*) NULL;
	newKeyServer->baseKeySpaceDN = NULL;
	newKeyServer->basePendingDN  = NULL;

	newKeyServer->accessType  = accessType;
	newKeyServer->keySpace	  = keySpace;
	newKeyServer->http		  = (PGPSocketRef) NULL;
	newKeyServer->errorNum	  = 0;
	newKeyServer->error	 	  = NULL;

	/*
	 * Now start parsing the url and decomposing it into its base
	 * components
	 */

	currentItem = localUrl;
	nextItem = strchr(currentItem, ':');

	if ( nextItem ) 
	{
		/*
		 * Determine if we have found the colon prior to the port position
		 * by checking that it is followed by "//"
		 */

		if ( !pgpCompareStringsIgnoreCaseN( nextItem,
				protocolSep, strlen(protocolSep ) ) )
		{
			for ( i = 0; protocolTag[i] != NULL; i++ ) 
			{
				if ( !pgpCompareStringsIgnoreCaseN( currentItem, 
								protocolTag[i], 
								strlen(protocolTag[i] ) ) )
				{
					/*
					 * found the protocol
					 */

					newKeyServer->serverType = protocolType[i];
					break;
				}
			}
			if ( newKeyServer->serverType == kPGPKeyServerInvalid )
			{
				/*
				 * The protocol is not supported
				 */

				err	= kPGPError_ServerInvalidProtocol;
				goto done;

			} 
			else 
			{

				/*
				 * move past the "://"
				 */

				nextItem += 3;
			}

		} 
		else if ( nextItem[1] == '/' )
		{
			/*
			 * malformed url
			 */

			err	= kPGPError_ServerInvalidProtocol;
			goto done;
		}
		else
		{

			/*
			 * The colon was likely after the hostname, and the default
			 * protocol is http, so, we'll default to that and continue
			 * We will set nextItem back to the beginning of the URL
			 */

			newKeyServer->serverType = kPGPKeyServerHTTP;
			nextItem = localUrl;

		}
	} 
	else 
	{

		/*
		 * No colon at all, so no protocol, and no port.  We must reset 
		 * nextItem to the start of the URL so we can begin looking for 
		 * the host
		 */

		newKeyServer->serverType = kPGPKeyServerHTTP;
		nextItem				 = localUrl;
	}

	/*
	 * Begin looking for the hostname
	 */

	if ( newKeyServer->port == kPGPKeyServerPortInvalid ) 
	{
		currentItem = nextItem;

		nextItem = strchr( currentItem, ':');

		if ( nextItem != NULL )
		{
			/*
			 * Found the port number
			 */

			newKeyServer->port = atoi( ++nextItem );
			newKeyServer->host = (char*) pgpContextMemAlloc( theContext, 
											(nextItem - currentItem)+1,
											kPGPMemoryFlags_Clear );
			if (!newKeyServer->host)
			{
				err	= kPGPError_OutOfMemory;
				goto done;
			}
			strncpy( newKeyServer->host, 
				     currentItem, 
					 ((nextItem - currentItem) -1 ) );

		} else {

			/*
			 * No explicit port, so go with the default ports
			 */

			switch (newKeyServer->serverType)
			{
				case kPGPKeyServerLDAP :
				{	
					newKeyServer->port = kPGPKeyServerPortLDAP;
					break;
				}

				case kPGPKeyServerHTTP :
				{
					newKeyServer->port = kPGPKeyServerPortHTTP;
					break;
				}

				default :
				{
					/*
					 * should never get here!
					 */

					err	= kPGPError_ServerInvalidProtocol;
					goto done;
				}
			}
			newKeyServer->host = (char*) pgpContextMemAlloc( theContext, 
												(strlen( currentItem ) + 1),
												kPGPMemoryFlags_Clear );

			if ( newKeyServer->host == NULL )
			{
				err	=  kPGPError_OutOfMemory;
				goto done;
			}
			strcpy( newKeyServer->host, currentItem );
		}
	} else {

		/*
		 * We defaulted to http since there was no protocol,
		 * now just grab the hostname
		 */

		newKeyServer->host = (char*) pgpContextMemAlloc( theContext, 
											(strlen( currentItem ) + 1),
											kPGPMemoryFlags_Clear );
		if ( newKeyServer->host == NULL )
		{
			err = kPGPError_OutOfMemory;
			goto done;
		}

		strcpy( newKeyServer->host, currentItem );
	}

	/*
	 * Trim any paths (for now, we're only dealing with host names)
	 */

	nextItem = strchr(newKeyServer->host, '/');
	if ( nextItem != NULL )
	{
		*nextItem = '\0';
	}

	*keyServer = newKeyServer;

	pgpContextMemFree( theContext, localUrl );


done:

	if (IsPGPError(err))
	{
		if ( localUrl != NULL )
		{
			pgpContextMemFree( theContext, localUrl );
		}
		if ( newKeyServer != NULL )
		{
			pgpContextMemFree( theContext, newKeyServer );
		}
	}

	pgpAssertErrWithPtr( err, *keyServer );
	return err;
}


	PGPError 
PGPNewKeyServerFromURL( 
	PGPContextRef	theContext, 
	char const		*url, 
	PGPKeyServerAccessType accessType,
	PGPKeyServerKeySpace keySpace,
	PGPKeyServerRef *keyServer )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( keyServer );
	*keyServer	= NULL;
	PGPValidatePtr( theContext );
	PGPValidatePtr( url );
	PGPValidateParam( accessType == kPGPKSAccess_Normal ||
		accessType == kPGPKSAccess_Administrator ||
		accessType == kPGPKSAccess_Default );
	
	err	= pgpNewKeyServerFromURL( theContext,
			url, accessType, keySpace, keyServer );
	
	pgpAssertErrWithPtr( err, *keyServer );
	return err;
}


PGPError 
PGPFreeKeyServer( PGPKeyServerRef keyServer )
{
	PGPValidateKeyServer( keyServer );
	
	if ( keyServer->host != NULL )
	{
		pgpContextMemFree( keyServer->myContext, keyServer->host );
	}
	if ( keyServer->baseKeySpaceDN != NULL )
	{
		pgpContextMemFree( keyServer->myContext, keyServer->baseKeySpaceDN );
	}
	if ( keyServer->basePendingDN != NULL )
	{
		pgpContextMemFree( keyServer->myContext, keyServer->basePendingDN );
	}
	if ( keyServer->error != NULL )
	{
		pgpContextMemFree( keyServer->myContext, keyServer->error );
	}
	pgpContextMemFree( keyServer->myContext, keyServer );

	return kPGPError_NoErr;
}


PGPError 
PGPQueryKeyServer( 
	PGPKeyServerRef		keyServer, 
	PGPFilterRef		filter,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*resultSet,
	PGPFlags *			outFlags )
{
	PGPError err = kPGPError_NoErr;

	if ( IsntNull( outFlags ) )
		*outFlags	= 0;
		
	PGPValidatePtr( resultSet );
	*resultSet	= NULL;
	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( filter );

	keyServer->errorNum	= 0;
	keyServer->error	= NULL;

	switch ( keyServer->serverType )
	{
		case kPGPKeyServerLDAP:
		{
			PGPFlags	tempFlags;
			
			err = pgpQueryLDAPKeyServer( keyServer, 
									filter, 
									callBack,
									callBackArg,
									resultSet,
									&tempFlags );
			if ( IsntPGPError( err ) && 
				IsntNull( outFlags ) )
			{
				*outFlags	= tempFlags;
			}
				
			break;
		}

		case kPGPKeyServerHTTP:
		{
			err = pgpQueryHKSKeyServer( keyServer, 
									filter,
									callBack,
									callBackArg,
									resultSet );
			break;
		}

		case kPGPKeyServerInvalid:
		case kPGPKeyServerType_force:
		{
			break;
		}
	}
	
	if (err == kPGPSocketsErrorUserCanceled) {
		err = kPGPError_UserAbort;
	}

	return(err);
}

PGPError 
PGPUploadToKeyServer( 
	PGPKeyServerRef		keyServer,
	PGPKeySetRef		keysToUpload,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*keysThatFailed)
{
    PGPError		err = kPGPError_NoErr;

	PGPValidatePtr( keysThatFailed );
	*keysThatFailed	= NULL;
	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( keysToUpload );

	keyServer->errorNum	= 0;
	keyServer->error	= NULL;

	switch ( keyServer->serverType )
	{
		case kPGPKeyServerLDAP:
		{
			err = pgpLDAPUploadToKeyServer( keyServer, 
										keysToUpload, callBack,
										callBackArg,
										keysThatFailed);
			break;
		}

		case kPGPKeyServerHTTP:
		{
			err = pgpHKPUploadToKeyServer( keyServer, 
										keysToUpload, callBack,
										callBackArg,
										keysThatFailed );
			break;
		}

		case kPGPKeyServerInvalid:
		case kPGPKeyServerType_force:
		{
			break;
		}
	}

	if (err == kPGPSocketsErrorUserCanceled) {
		err = kPGPError_UserAbort;
	}

	return( err );
}

PGPError 
PGPDeleteFromKeyServer( 
	PGPKeyServerRef		keyServer,
	PGPKeySetRef		keysToDelete,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*keysThatFailed)
{
    PGPError		err = kPGPError_NoErr;

	PGPValidatePtr( keysThatFailed );
	*keysThatFailed	= NULL;
	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( keysToDelete );

	keyServer->errorNum	= 0;
	keyServer->error	= NULL;

	switch ( keyServer->serverType )
	{
		case kPGPKeyServerLDAP:
		{
			err = pgpLDAPDeleteFromKeyServer( keyServer, 
										keysToDelete, 
										FALSE,
										callBack,
										callBackArg,
										keysThatFailed);
			break;
		}

		case kPGPKeyServerHTTP:
		{
			err = kPGPError_ServerOperationNotAllowed;
			break;
		}

		case kPGPKeyServerInvalid:
		case kPGPKeyServerType_force:
		{
			break;
		}
	}

	if (err == kPGPSocketsErrorUserCanceled) {
		err = kPGPError_UserAbort;
	}

	return( err );
}

PGPError 
PGPDisableFromKeyServer( 
	PGPKeyServerRef		keyServer,
	PGPKeySetRef		keysToDisable,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*keysThatFailed)
{
    PGPError		err = kPGPError_NoErr;

	PGPValidatePtr( keysThatFailed );
	*keysThatFailed	= NULL;
	PGPValidateKeyServer( keyServer );
	PGPValidatePtr( keysToDisable );

	keyServer->errorNum = 0;
	keyServer->error = NULL;

	switch ( keyServer->serverType )
	{
		case kPGPKeyServerLDAP:
		{
			err = pgpLDAPDeleteFromKeyServer( keyServer, 
										keysToDisable, 
										TRUE,
										callBack,
										callBackArg,
										keysThatFailed);
			break;
		}

		case kPGPKeyServerHTTP:
		{
			err = kPGPError_ServerOperationNotAllowed;
			break;
		}

		case kPGPKeyServerInvalid:
		case kPGPKeyServerType_force:
		{
			break;
		}
	}

	if (err == kPGPSocketsErrorUserCanceled) {
		err = kPGPError_UserAbort;
	}

	return( err );

}

PGPError
PGPGetKeyServerErrorString( PGPKeyServerRef keyServer, char **errorString )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( errorString );
	*errorString	= NULL;
	PGPValidateKeyServer( keyServer );

	if ( keyServer->error != NULL && *keyServer->error != '\0' )
	{
		*errorString = (char*) PGPNewData( keyServer->myContext,
								strlen( keyServer->error)+1);
		
		if ( *errorString == NULL )
		{
			err	= kPGPError_OutOfMemory;
		}
		else
		{
			strcpy( *errorString, keyServer->error );
		}
	}

	pgpAssertErrWithPtr( err, *errorString );
	return err;
}

void pgpKeyServerSocketCallback( void * callBackArg )
{
	PGPError					err;
	PGPOptionListRef			newOptionList;
	PGPKeyServerCallbackDataRef	callBackData;

	if (callBackArg == NULL)
	{
		return;
	}

	callBackData = (PGPKeyServerCallbackDataRef) callBackArg;

	err = pgpEventKeyServer( callBackData->myContext, &newOptionList,
						callBackData->callBack, 
						callBackData->callBackArg, 
						callBackData->callBackState, 
						callBackData->soFar, 
						callBackData->total );

	if (IsPGPError(err) && callBackData->socket != NULL ) {
		PGPCloseSocket(callBackData->socket);
	} 

}
/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
