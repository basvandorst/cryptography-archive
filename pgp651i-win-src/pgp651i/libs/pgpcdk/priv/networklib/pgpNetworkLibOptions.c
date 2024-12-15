/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: pgpNetworkLibOptions.c,v 1.8.6.1 1999/06/04 01:12:04 heller Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpKeyServer.h"
#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpOptionListPriv.h"


	static PGPError
StandardNetworkLibHandler(
	PGPContextRef				context,
	PGPOptionHandlerOperation 	operation,
	PGPOptionType				type,
	PGPOptionValue				inputValue,
	PGPSize 					inputValueSize,
	PGPOptionValue 				*outputValue,
	PGPSize						*outputValueSize)
{
	PGPError	err = kPGPError_NoErr;
	
	(void) context;
	(void) inputValueSize;
	(void) outputValueSize;

	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			switch( type ) {
			case kPGPOptionType_KeyServerSearchFilter:
				PGPFreeFilter( inputValue.asFilterRef );
				break;

			default:
				break;
			}
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			switch( type ) {
			case kPGPOptionType_KeyServerSearchFilter:
				PGPIncFilterRefCount( outputValue->asFilterRef );
				break;
				
			default:
				break;
			}
			break;
		}
		
		default:
		{
			err = kPGPError_UnknownRequest;
			break;
		}
	}

	return( err );
}



	PGPOptionListRef
PGPONetURL(
	PGPContextRef 	context,
	const char 		*url)
{
	PGPOptionListRef	optionList;

	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( url ) );
	
	optionList = pgpCreateBufferOptionList( context,
								kPGPOptionType_URL,
								url, strlen( url ) + 1 );
								
	return( optionList );
}

	static void
FreePGPONetHostNameDesc(
	PGPContextRef		context,
	PGPONetHostNameDesc	*desc)
{
	pgpAssertAddrValid( desc, PGPONetHostNameDesc );
	
	if( IsntNull( desc->hostName ) )
	{
		pgpContextMemFree( context, (void *) desc->hostName );
		desc->hostName = NULL;
	}

	pgpContextMemFree( context, desc );
}

	static PGPError
CopyPGPONetHostNameDesc(
	PGPContextRef				context,
	const PGPONetHostNameDesc	*srcDesc,
	PGPONetHostNameDesc			**destDesc)
{
	PGPError			err = kPGPError_OutOfMemory;
	PGPONetHostNameDesc	*outDesc;
	
	pgpAssertAddrValid( srcDesc, PGPONetHostNameDesc );
	pgpAssertAddrValid( destDesc, PGPONetHostNameDesc * );
	
	*destDesc = NULL;
	
	outDesc = (PGPONetHostNameDesc *) pgpContextMemAlloc( context,
						sizeof( *outDesc ), kPGPMemoryMgrFlags_Clear );
	if( IsntNull( outDesc ) )
	{
		*outDesc = *srcDesc;
		
		pgpAssertAddrValid( srcDesc->hostName, char );
		
		outDesc->hostName = pgpAllocCString( context, srcDesc->hostName );
		if( IsntNull( outDesc->hostName )  )
		{
			err = kPGPError_NoErr;
		}
		else
		{
			FreePGPONetHostNameDesc( context, outDesc );
			outDesc = NULL;
		}
	}
	
	*destDesc = outDesc;

	return( err );
}

	static PGPError
HostNameOptionHandlerProc(
	PGPContextRef				context,
	PGPOptionHandlerOperation 	operation,
	PGPOptionType				type,
	PGPOptionValue				inputValue,
	PGPSize 					inputValueSize,
	PGPOptionValue 				*outputValue,
	PGPSize						*outputValueSize)
{
	PGPError			err = kPGPError_NoErr;
	PGPONetHostNameDesc	*inputDesc;
	
	pgpAssert( inputValueSize == sizeof( *inputDesc ) );

	(void) type;
	(void) inputValueSize;
	
	inputDesc = (PGPONetHostNameDesc *) inputValue.asPtr;
	pgpAssertAddrValid( inputDesc, PGPONetHostNameDesc );
	
	switch( operation )
	{
		case kPGPOptionHandler_FreeDataOperation:
		{
			FreePGPONetHostNameDesc( context, inputDesc );
			break;
		}
			
		case kPGPOptionHandler_CopyDataOperation:
		{
			PGPONetHostNameDesc	*outputDesc;

			pgpAssertAddrValid( outputValue, PGPOptionValue );
			pgpAssertAddrValid( outputValueSize, PGPSize );

			err = CopyPGPONetHostNameDesc( context, inputDesc, &outputDesc );
			if( IsntPGPError( err ) )
			{
				outputValue->asPtr 	= outputDesc;
				*outputValueSize 	= sizeof( *outputDesc );
			}

			break;
		}
		
		default:
		{
			err = kPGPError_UnknownRequest;
			break;
		}
	}

	return( err );
}

	PGPOptionListRef
PGPONetHostName(
	PGPContextRef 	context,
	const char 		*hostName,
	PGPUInt16 		port)
{
	PGPOptionListRef	optionList;
	PGPONetHostNameDesc	descriptor;
	PGPONetHostNameDesc	*allocatedDesc;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( IsntNull( hostName ) );
	
	pgpClearMemory( &descriptor, sizeof( descriptor ) );
	
	descriptor.hostName	= hostName;
	descriptor.port		= port;

	if( IsntPGPError( CopyPGPONetHostNameDesc( context, &descriptor,
				&allocatedDesc ) ) )
	{
		PGPOptionValue		value;
		
		value.asPtr = allocatedDesc;
		
		optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_HostName,
							&value, sizeof( *allocatedDesc ),
							HostNameOptionHandlerProc );
	}
	else
	{
		optionList = kPGPOutOfMemoryOptionListRef;
	}
	
	return( optionList );
}

	PGPOptionListRef
PGPONetHostAddress(
	PGPContextRef 	context,
	PGPUInt32 		hostAddress,
	PGPUInt16 		port)
{
	PGPOptionListRef		optionList;
	PGPONetHostAddressDesc	desc;
	
	pgpValidateOptionContext( context );
	
	desc.hostAddress 	= hostAddress;
	desc.port 			= port;

	optionList = pgpCreateBufferOptionList( context, kPGPOptionType_HostAddress,
								&desc, sizeof( desc ) );
	
	return( optionList );
}

	PGPOptionListRef
PGPOKeyServerProtocol(
	PGPContextRef 			context,
	PGPKeyServerProtocol 	serverProtocol)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( serverProtocol >= kPGPKeyServerProtocol_LDAP &&
			serverProtocol <= kPGPKeyServerProtocol_HTTPS );
			
	value.asUInt = serverProtocol;

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerProtocol,
							&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOKeyServerKeySpace(
	PGPContextRef 			context,
	PGPKeyServerKeySpace 	serverSpace)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( serverSpace >= kPGPKeyServerKeySpace_Default &&
			serverSpace <= kPGPKeyServerKeySpace_Pending );
			
	value.asUInt = serverSpace;

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerKeySpace,
							&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOKeyServerAccessType(
	PGPContextRef 			context,
	PGPKeyServerAccessType 	accessType)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( accessType >= kPGPKeyServerAccessType_Default &&
			accessType <= kPGPKeyServerAccessType_Administrator );
			
	value.asUInt = accessType;

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerAccessType,
							&value, sizeof( PGPUInt32 ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOKeyServerCAKey(
	PGPContextRef	context,
	PGPKeyRef		caKey)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( PGPKeyRefIsValid( caKey ) );
			
	value.asKeyRef = caKey;

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerCAKey,
							&value, sizeof( PGPKeyRef ), NULL );
	
	return( optionList );
}

	PGPOptionListRef
PGPOKeyServerRequestKey(
	PGPContextRef	context,
	PGPKeyRef		requestKey)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( PGPKeyRefIsValid( requestKey ) );
			
	value.asKeyRef = requestKey;

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerRequestKey,
							&value, sizeof( PGPKeyRef ), NULL );
	
	return( optionList );
}



	PGPOptionListRef
PGPOKeyServerSearchKey(
	PGPContextRef	context,
	PGPKeyRef		searchKey)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( PGPKeyRefIsValid( searchKey ) );
			
	value.asKeyRef = searchKey;

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerSearchKey,
							&value, sizeof( PGPKeyRef ), NULL );
	
	return( optionList );
}



	PGPOptionListRef
PGPOKeyServerSearchFilter(
	PGPContextRef	context,
	PGPFilterRef	searchFilter)
{
	PGPOptionListRef	optionList;
	PGPOptionValue		value;
	
	pgpValidateOptionContext( context );
	pgpValidateOptionParam( PGPFilterRefIsValid( searchFilter ) );
	
	PGPIncFilterRefCount(searchFilter);
	value.asFilterRef = searchFilter;

	optionList = pgpCreateStandardValueOptionList( context,  
							kPGPOptionType_KeyServerSearchFilter,
							&value, sizeof( PGPFilterRef ), StandardNetworkLibHandler );
	
	return( optionList );
}
