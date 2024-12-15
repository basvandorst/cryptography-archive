/*____________________________________________________________________________
	pgpKsHTTP.c
	
	Copyright(C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Horowitz Key Server support via HTTP

	$Id: pgpKsHTTP.c,v 1.46.4.1 1997/11/07 05:22:49 markm Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pgpKeyServerPriv.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpContext.h"
#include "pgpEncode.h"
#include "pgpEventPriv.h"
#include "pgpDebug.h"
#include "pgpMem.h"

static PGPError			pgpUrlEncode(PGPContextRef context, 
								char **Dest, char *Source);

static PGPError			pgpReceiveBuffer( PGPKeyServerRef keyServer, 
								PGPEventHandlerProcPtr callBack,
								PGPUserValue		callBackArg,
								char **buffer );

static PGPError			pgpHKPGetKeyInfo( PGPKeyServerRef keyServer, 
								char	**KeyBuffer,
								char	*query,
								PGPEventHandlerProcPtr callBack,
								PGPUserValue callBackArg);

static PGPError			pgpHKPProcessResult( PGPContextRef context, 
								char *buffer );


struct PGPHKPServerMsgs {
	const char *serverMsg;
	PGPError	err;
};

static struct PGPHKPServerMsgs	svr[] = 
{
    { "Key block corrupt: pubkey decode failed", 
		kPGPError_ServerCorruptKeyBlock },
    { "Key block added to key server database.\n  New public keys added: ",
		kPGPError_NoErr},
    {"Key block in add request contained no new\n"\
	 "keys, userid's, or signatures.", 
		kPGPError_ServerKeyAlreadyExists },
	{ NULL, 
		kPGPError_NoErr }
};


PGPError 
pgpQueryHKSKeyServer( 
	PGPKeyServerRef keyServer, 
	PGPFilterRef	filter,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue	callBackArg,
	PGPKeySetRef	*resultSet )
{
	PGPKeySetRef	newKeySet  = NULL;
	PGPError		err = 0;
	char			*query	   = NULL;
	char			*keyBuffer = NULL;

	PGPValidatePtr( resultSet );
	*resultSet	= NULL;
	PGPValidatePtr( keyServer );
	PGPValidatePtr( filter );

	err = PGPHKSQueryFromFilter( filter, &query );

	if ( IsPGPError(err) )
	{
		return err;
	}
	err = pgpHKPGetKeyInfo( keyServer, &keyBuffer, query, 
						callBack, callBackArg );

	if ( IsPGPError(err) )
	{
		goto done;
	}
	
	err = PGPImportKeySet(
				keyServer->myContext,
				&newKeySet,
				PGPOInputBuffer( keyServer->myContext, 
					keyBuffer, strlen(keyBuffer)),
				PGPOLastOption(keyServer->myContext) );

	if ( IsntPGPError(err) )
	{
		*resultSet = newKeySet;
	}
	else
	{
		*resultSet = NULL;
	}

done:

	if ( query != NULL )
	{
		PGPFreeData( query );
	}

	if ( keyBuffer != NULL )
	{
		pgpContextMemFree( keyServer->myContext, keyBuffer );
	}
	return( err );
}

PGPError 
pgpHKPUploadToKeyServer( 
	PGPKeyServerRef		keyServer, 
	PGPKeySetRef		keysToUpload,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg,
	PGPKeySetRef		*keysThatFailed)
{
	PGPHostEntry	*pHostEnt		= NULL;
	PGPSocketAddressInternet sockAddr;
	PGPKeyListRef	keyList			= NULL;
	PGPKeyIterRef	keyIter			= NULL;
	PGPKeyRef		key				= NULL;
	PGPKeySetRef	keySetSingle	= NULL;
	PGPKeySetRef	keySetError		= NULL;
	PGPByte			*keyBuffer		= NULL;
	PGPByte			*tempKeyBuffer	= NULL;
	PGPBoolean		partialFailure	= FALSE;
	PGPBoolean		keyAlreadyExists= TRUE;
	PGPUInt32		currentRecord	= 1;
	PGPUInt32		totalKeys		= 0;
	PGPSize			bufSize			= 0;
	PGPByte			*tempBuffer		= NULL;
	PGPByte			*postBuffer		= NULL;
	PGPByte			*ErrBuffer		= NULL;
	PGPInt32		result			= 0;
    PGPError		err = kPGPError_NoErr;
	PGPKeyServerCallbackData	callBackData;
	PGPOptionListRef	newOptionList;
	char const		*prefix =	"POST /pks/add HTTP/1.0\n"\
								"User-Agent:PGP/5.5 (Unix)\n"\
								"Content-length: ";
	pgpAssert( keyServer );
	pgpAssert( keysToUpload );

	err = pgpEventKeyServer( keyServer->myContext, &newOptionList, 
						callBack, callBackArg, 
						kPGPKeyServerStateConnect, 0, 0 );

	if ( IsPGPError(err) )
	{
		return kPGPError_UserAbort;
	}

	callBackData.callBack	= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState = kPGPKeyServerStateConnect;
	callBackData.socket		= NULL;
	callBackData.soFar		= 0;
	callBackData.total		= 0;

	PGPSetDefaultCallback(pgpKeyServerSocketCallback, (void*)&callBackData);

	pHostEnt = PGPGetHostByName(keyServer->host);
	if ( pHostEnt == NULL )
	{
		PGPSetDefaultCallback(NULL, NULL);
		return kPGPError_ServerUnknownHost;
	}

	sockAddr.sin_family = kPGPAddressFamilyInternet;
	sockAddr.sin_port   = (ushort) PGPHostToNetShort((short)keyServer->port);
	sockAddr.sin_addr   = *((PGPInternetAddress*)*pHostEnt->h_addr_list);

	err = PGPNewKeySet( keyServer->myContext, &keySetError );

	if ( IsPGPError(err) ) 
	{
		goto done;
	}

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
	
	err = PGPCountKeys( keysToUpload, &totalKeys);
	if ( IsPGPError( err ) )
		goto done;

	while (( err = PGPKeyIterNext(keyIter, &key)) == kPGPError_NoErr)
	{
		PGPContextRef	context	= keyServer->myContext;
		
		err = PGPNewSingletonKeySet( key, &keySetSingle );

		if ( IsPGPError(err) )
		{
			break;
		}


		/*
		 * The first call is just to get the bufSize
		 */

		err = PGPExportKeySet( keySetSingle,
				PGPOAllocatedOutputBuffer( context, (void**)&tempKeyBuffer, 
					MAX_PGPSize, &bufSize ),
				PGPOLastOption( context ) );

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

		/*
		 * Create horowitz HTTP request
		 */

		err = pgpUrlEncode( keyServer->myContext, 
							(char **) &tempBuffer, (char *) keyBuffer);

		if ( IsPGPError(err) )
		{
			goto done;
		}

		err = pgpEventKeyServer( keyServer->myContext, 
								&newOptionList,
								callBack, callBackArg, 
								kPGPKeyServerStateSend, 
								currentRecord, totalKeys );

	    postBuffer = pgpContextMemAlloc( keyServer->myContext, 
									(strlen((char *) tempBuffer) + 512),
									kPGPMemoryFlags_Clear );
	    if ( postBuffer != NULL ) 
		{
			sprintf( (char *) postBuffer, "%s%i\n\nkeytext=", prefix,
					 strlen((char *) tempBuffer) + 8 );

			strcat((char *) postBuffer, (char *) tempBuffer);

			// Connect the socket
			keyServer->http = PGPOpenSocket( kPGPAddressFamilyInternet, 
										kPGPSocketTypeStream, 
										kPGPTCPProtocol );

			if ( keyServer->http == kPGPInvalidSocket )
			{
				err =  kPGPError_ServerSocketError;
				break;
			}

			PGPSetCallback( keyServer->http, 
					pgpKeyServerSocketCallback, 
					(void*)&callBackData);

			PGPSetDefaultCallback(NULL, NULL);

			callBackData.socket		= keyServer->http;

			result = PGPConnect(keyServer->http, 
							(PGPSocketAddress*)&sockAddr, sizeof(sockAddr));

			if ( result != 0 )
			{
				err = kPGPError_ServerConnectFailed;
				break;
			}

			callBackData.callBackState = kPGPKeyServerStateSend;
			callBackData.soFar		= currentRecord;
			callBackData.total		= totalKeys;

			PGPSend(keyServer->http, postBuffer, 
				(short) strlen((char *) postBuffer), 0);

			callBackData.callBackState = kPGPKeyServerStateReceive;
			callBackData.soFar		= currentRecord;
			callBackData.total		= totalKeys;

			err = pgpReceiveBuffer( keyServer, 
							callBack,
							callBackArg,
							(char **) &ErrBuffer );

			if ( IsntPGPError(err) && ErrBuffer != NULL)
			{
				err = pgpHKPProcessResult( keyServer->myContext, 
										(char*)ErrBuffer);
			}
			if ( IsPGPError(err) )
			{
				PGPAddKeys( keySetSingle, keySetError );
				partialFailure = TRUE;

				if ( err == kPGPError_ServerKeyAlreadyExists )
				{
					keyAlreadyExists = TRUE;
				}

			}
			PGPCloseSocket( keyServer->http );
			keyServer->http = kPGPInvalidSocket;

			pgpContextMemFree( keyServer->myContext, postBuffer );
			postBuffer = NULL;
		}
		if (ErrBuffer)
		{
			pgpContextMemFree( keyServer->myContext, ErrBuffer );
			ErrBuffer = NULL;
		}

		pgpContextMemFree( keyServer->myContext, keyBuffer );
		keyBuffer = NULL;

		pgpContextMemFree( keyServer->myContext, tempBuffer );
		tempBuffer = NULL;

		PGPFreeKeySet( keySetSingle );
		keySetSingle = NULL;

		currentRecord++;
	} 

	if ( err == kPGPError_EndOfIteration )
	{
		err = kPGPError_NoErr;
	}

done:

	if ( keyList != NULL )
	{
		PGPFreeKeyList( keyList );
	}

	if ( keyIter != NULL )
	{
		PGPFreeKeyIter( keyIter );
	}

	if ( keySetSingle != NULL )
	{
		PGPFreeKeySet( keySetSingle );
	}

	if ( keyServer->http != kPGPInvalidSocket ) 
	{
		PGPCloseSocket(keyServer->http);
		keyServer->http = kPGPInvalidSocket;
	}

	if ( postBuffer != NULL )
	{
		pgpContextMemFree( keyServer->myContext, postBuffer );
	}
	if ( tempBuffer != NULL )
	{
		pgpContextMemFree( keyServer->myContext, tempBuffer );
	}
	if ( keyBuffer != NULL )
	{
		PGPFreeData( keyBuffer );
	}

	if ( partialFailure == TRUE )
	{
		*keysThatFailed = keySetError;

		if ( keyAlreadyExists == TRUE )
		{
			return kPGPError_ServerKeyAlreadyExists;
		}
		else
		{
			return kPGPError_ServerPartialAddFailure;
		}
	}
	else
	{
		PGPFreeKeySet( keySetError );
		*keysThatFailed = NULL;
		return err;
	}
}
		
static 
PGPError
pgpUrlEncode( PGPContextRef context, char **Dest, char *Source)
{
    PGPError	err = kPGPError_NoErr;
    char		*pSource, *pDest, *pLimit;

    /*Absolute worst case scenario is three times the source size.  Rather
     *than getting too precise, we'll allocate that much initially, and then
     *realloc it down to actuality later.
     */

    pgpAssert( Source );
    pgpAssert( Dest );

    if ( Source && Dest ) 
	{
		*Dest = pgpContextMemAlloc( context, 
							(strlen(Source) * 3) + 1,
							kPGPMemoryFlags_Clear );
		if ( *Dest != NULL )
		{
			pSource = Source;
			pDest = *Dest;
			pLimit = *Dest + (strlen(pSource) * 3) + 1;
			
			while(pSource && *pSource) 
			{
				pgpAssert( pDest < pLimit );
				/*Zeroth case:  it's an alphabetic or numeric character*/
				if (!isalpha(*pSource) && 
				    !isdigit(*pSource) && 
				    *pSource != '-') 
				{
					/*First case:  Turn spaces into pluses.*/
					if(*pSource == ' ') 
					{
						*pDest = '+';
					}
#if PGP_MACINTOSH
					else if(*pSource == '\r')
					{
						sprintf(pDest, "%0D%0A");
						pDest += 5;
					}
#endif
					else 
					{
						/*This is overkill, but works for our purposes*/
						*pDest = '%';
						++pDest;
						sprintf(pDest, "%02X", *pSource);
						++pDest;
					}
				}
				else
				{
					*pDest = *pSource;
				}
				
				++pDest;
				++pSource;
			}

			*pDest = '\0';
			
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}
	}
    else
	{
		err = kPGPError_BadParams;
	}
    return err;
}


/*____________________________________________________________________________
	Receive data from server.
	
	Each time we receive another bufferfull, append it to main buffer.
	The main buffer is grown proportionally to its existing size in order to
	avoid n^2 behavior.  For the same reason, we do *not* use strcat when
	appending to it, but rather maintain a count of how much data is already
	present.
____________________________________________________________________________*/
static 
PGPError 
pgpReceiveBuffer( PGPKeyServerRef keyServer, 
		 	PGPEventHandlerProcPtr callBack,
			PGPUserValue		callBackArg,
			char **bufferOut )
{
	PGPUInt32	bufferSize	  = 0;	/* size of buffer >= dataSize + 1 */
	PGPUInt32	dataSize	  = 0;	/* amount of data in buffer */
	PGPError	err			  = kPGPError_NoErr;
#define kTempBufferSize			4096
    char		*tempBuffer	  = NULL;
	PGPUInt16	tempBufferSize = kTempBufferSize;
	PGPOptionListRef newOptionList;
	char *		buffer	= NULL;

	PGPValidatePtr( bufferOut );
    *bufferOut = NULL;

	tempBuffer = (char*) pgpContextMemAlloc( keyServer->myContext, 
										tempBufferSize,
										kPGPMemoryFlags_Clear );
	if ( IsNull( tempBuffer ) )
	{
		return kPGPError_OutOfMemory;
	} 

    do
    {
		PGPInt16	charsReceived = 0;
    
		err = pgpEventKeyServer( keyServer->myContext, 
					&newOptionList, callBack, callBackArg, 
					kPGPKeyServerStateReceive,
					MAX_PGPUInt32,  MAX_PGPUInt32 ); 
		if ( IsPGPError(err) )
		{
			break;
		}

		charsReceived = PGPReceive( keyServer->http, 
									tempBuffer, 
									(short)tempBufferSize, 
									(ushort)0);

		if ( charsReceived > 0 ) 
		{
			PGPSize		startDataSize;
			
			startDataSize	= dataSize;
			dataSize		+= charsReceived;
			/* maintain the buffer at least as large as the data plus
			a trailing NULL we'll want to append later */
			if ( bufferSize < dataSize + sizeof(char) )
			{
				/* need to make buffer larger */
				/* grow size proportionally to avoid n^2 behavior */
				/* include enough space for trailing NULL */
				bufferSize	= bufferSize + ( bufferSize / 2 );
				if ( bufferSize <= dataSize )
					bufferSize	= dataSize + sizeof( char );
					
				err = pgpContextMemRealloc( keyServer->myContext, 
							(void**)&buffer, 
							bufferSize * sizeof( char ), 0);

				if ( IsPGPError(err) )
				{
					err	= kPGPError_OutOfMemory;
					break;
				}
			}
			
			/* append new data .  Using strcat() makes this loop
			an n^2 algorithm so just copy the data to the end */
			pgpCopyMemory( tempBuffer,
				buffer + startDataSize, charsReceived );
		} 
		else if ( charsReceived == 0 && IsNull( buffer ) )
		{
		    /*
			 * No data has been received on first attempt.
		     */
			err = kPGPError_ServerSocketError;
			break;
     	}
		else
		{
			/*
			 * we've either had a socket error (-1) or the socket
			 * was closed on the other side (0).  Since we have a
			 * valid buffer, we'll check for an error, and continue
			 */

			err = PGPGetLastSocketsError();
			if (err == kPGPSocketsErrorNotConn )
			{
				if ( IsntNull( buffer ) )
					err = kPGPError_NoErr;
			}
			else if (err == kPGPSocketsErrorUserCanceled )
			{
				err = kPGPError_UserAbort;
			}
			else if (err != 0)
			{
				err = kPGPError_ServerSocketError;
			} 
			else 
			{
				/*
				 * translate 0 into SDK no error
				 */
				err = kPGPError_NoErr;
			}
			break;
		}

    } while ( TRUE );
	
	pgpContextMemFree( keyServer->myContext, tempBuffer );
	if ( IsntNull( buffer ) && IsPGPError( err ) )
	{
		pgpContextMemFree( keyServer->myContext, buffer );
		buffer	= NULL;
	}
	
	if ( IsntPGPError( err ) )
	{
		pgpAssert( IsntNull( buffer ) );
		pgpAssert( bufferSize >= dataSize + 1 );
		
		/* shrink buffer to include just data and trailing NULL */
		if ( bufferSize != dataSize + 1 )
		{
			err = pgpContextMemRealloc( keyServer->myContext, 
						(void**)&buffer, 
						(dataSize + 1) * sizeof( char ), 0 );
			pgpAssertNoErr( err );
		}
		/* always null-terminate buffer */
		buffer[ dataSize ]	= '\0';
	}
	
	*bufferOut	= buffer;
	pgpAssertErrWithPtr( err, *bufferOut );
	
    return err;
}

static PGPError 
pgpHKPGetKeyInfo(
	PGPKeyServerRef		keyServer, 
	char				**keyBuffer,
	char				*query,
	PGPEventHandlerProcPtr callBack,
	PGPUserValue		callBackArg)
{
	PGPError					err = kPGPError_NoErr;
	PGPHostEntry				*pHostEnt;
	PGPSocketAddressInternet	sockAddr;
	short						result;
	char						*buffer	= NULL;
	PGPSize						bufferSize	= 0;
	PGPOptionListRef			newOptionList;
	PGPKeyServerCallbackData	callBackData;
	char const			*prefix = "GET /pks/lookup?op=get&";
	char const			*suffix = " HTTP/1.0\n";
	char const			*userAgent = "User-Agent: PGP/5.5 (Unix)\n\r\n\r";

	PGPValidatePtr( keyBuffer );
	*keyBuffer	= NULL;
	
	err = pgpEventKeyServer( keyServer->myContext, &newOptionList, 
						callBack, callBackArg, 
						kPGPKeyServerStateConnect, 0, 0 );

	if ( IsPGPError(err) )
	{
		return err;
	}

	callBackData.callBack	= callBack;
	callBackData.callBackArg	= callBackArg;
	callBackData.callBackState = kPGPKeyServerStateConnect;
	callBackData.socket		= NULL;
	callBackData.soFar		= 0;
	callBackData.total		= 0;

	PGPSetDefaultCallback(pgpKeyServerSocketCallback, (void*) &callBackData);

	pHostEnt = PGPGetHostByName(keyServer->host);
	if ( pHostEnt == NULL )
	{
		PGPSetDefaultCallback(NULL, NULL);
		return kPGPError_ServerUnknownHost;
	}

	keyServer->http = PGPOpenSocket( kPGPAddressFamilyInternet, 
								kPGPSocketTypeStream, kPGPTCPProtocol );

	if ( keyServer->http == kPGPInvalidSocket )
	{
		PGPSetDefaultCallback(NULL, NULL);
		return kPGPError_ServerSocketError;
	}

	PGPSetDefaultCallback(NULL, NULL);

	callBackData.socket		= keyServer->http;

	PGPSetCallback( keyServer->http, 
			pgpKeyServerSocketCallback, 
			(void*)&callBackData);

	sockAddr.sin_family = kPGPAddressFamilyInternet;
	sockAddr.sin_port   = (ushort)PGPHostToNetShort((short)keyServer->port);
	sockAddr.sin_addr   = *((PGPInternetAddress*)*pHostEnt->h_addr_list);

	result = PGPConnect(keyServer->http, 
					(PGPSocketAddress*)&sockAddr, sizeof(sockAddr));

	if ( result != 0 )
	{
		PGPCloseSocket( keyServer->http );
		keyServer->http = kPGPInvalidSocket;

		return kPGPError_ServerConnectFailed;
	}

	/*
	 * Create the CGI request
	 */
	bufferSize	= strlen(prefix) + strlen(query) +
					strlen(suffix) + strlen(userAgent) + 1;
#if PGP_DEBUG
	bufferSize	+= 1;	/* sentinel to verify sprintf doesn't overwrite */
#endif
	buffer = (char *) pgpContextMemAlloc( keyServer->myContext,
							bufferSize, kPGPMemoryFlags_Clear );

	if ( buffer == NULL )
	{
		PGPCloseSocket( keyServer->http );
		keyServer->http = kPGPInvalidSocket;

		return kPGPError_OutOfMemory;
	}
#if PGP_DEBUG
	buffer[ bufferSize - 1 ]	= 'X';	/* set sentinel value */
#endif
	sprintf( buffer, "%s%s%s%s", prefix, query, suffix, userAgent );
	pgpAssert( buffer[ bufferSize - 1 ] == 'X' );
#if PGP_DEBUG
	--bufferSize;	/* remove sentinel */
#endif
	pgpAssert( strlen( buffer ) + 1 <= bufferSize );

	callBackData.callBackState = kPGPKeyServerStateSend;
	callBackData.soFar		= 0;
	callBackData.total		= 0;

	PGPSend( keyServer->http, buffer, (short) strlen(buffer), 0);

	err = pgpEventKeyServer( keyServer->myContext, &newOptionList, 
						callBack, callBackArg, 
						kPGPKeyServerStateWait, 0, 0 );

	callBackData.callBackState = kPGPKeyServerStateReceive;
	callBackData.soFar		= 0;
	callBackData.total		= 0;

	err = pgpReceiveBuffer( keyServer, 
					callBack,
					callBackArg,
					keyBuffer );
	pgpAssertErrWithPtr( err, *keyBuffer );

	if ( IsntPGPError(err) ) 
	{
		pgpAssert( IsntNull( *keyBuffer ) );
		
	    if( (strstr( *(keyBuffer), "BEGIN PGP") == NULL) ||
			(strstr( *(keyBuffer),  "No matching keys in database") != NULL))
		{
			err = kPGPError_NoErr;
		}
	}

	PGPCloseSocket( keyServer->http );
	keyServer->http = kPGPInvalidSocket;

	if ( buffer != NULL )
	{
		pgpContextMemFree( keyServer->myContext, buffer );
	}
	
	pgpAssertErrWithPtr( err, *keyBuffer );
    return err;
}

static 
PGPError
pgpHKPProcessResult( PGPContextRef context, char *buffer )
{
    char		*p;
    PGPInt32	x;
    char		*TempBuffer;
	PGPBoolean	FoundError	= FALSE;
    PGPError	err	= kPGPError_NoErr;

    pgpAssert( buffer );

    TempBuffer = pgpContextMemAlloc( context, 
							(strlen(buffer) + 1) * sizeof(char),
							kPGPMemoryFlags_Clear );

    if ( TempBuffer == NULL ) 
	{
		return kPGPError_OutOfMemory;
	}

	strcpy( TempBuffer, buffer );
	p = TempBuffer;

	/*The first step is to skip over the HTTP headers.*/
	p = strstr(buffer, "\r\n\r\n");
	if( p ) 
	{
	    /*Next, skip the title*/
	    p = strstr( p, "<p>\r\n" );
	    if ( p ) 
		{
			++p;
			/*Next, skip the header (<h1>)*/
			p = strstr(p, "<p>\r\n");
			if(p) 
			{
		    /*Last, jump the <p>\r\n we just found*/
				p += 5;
				/*Sometimes, it has a "<pre>...</pre>" tag, so let's
				 *clean those up.  Ideally, if we ever have to show this
				 *to the user, we don't want the raw HTML to pop out.
				 */
				if(strncmp(p, "<pre>", 5) == 0) {
					char *p2;
		
					p += 5;
					p2 = strstr(p, "</pre>");
					if(p2)
						*p2 = '\0';
				}
				while(isspace(*p))
					++p;
				/*God willing and the creek don't rise, p now points to
				 *the actual error message.  See if we can look it up...
				 */

				for ( x = 0; 
					svr[x].serverMsg && !FoundError; 
					++x) 
				{
					if ( strncmp( svr[x].serverMsg, 
							p, strlen(svr[x].serverMsg)) == 0)
					{
						FoundError = TRUE;
						err = svr[x].err;
					}
			    }
				if ( FoundError == FALSE )
				{
					err = kPGPError_ServerUnknownResponse;
				}
			}
		}
	}
	else
	{
		err = kPGPError_ServerUnknownResponse;
	}

	pgpContextMemFree( context, TempBuffer );
    return err;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
