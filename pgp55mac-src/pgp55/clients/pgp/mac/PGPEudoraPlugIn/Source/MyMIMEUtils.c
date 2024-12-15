/*____________________________________________________________________________
	MyMIMEUtils.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MyMIMEUtils.c,v 1.1 1997/07/25 23:47:26 lloyd Exp $
____________________________________________________________________________*/
#include "MacStrings.h"
#include "MyMIMEUtils.h"
#include "mimetype.h"


	static OSErr
CreateMIMEParam(
	ConstStr255Param		nameString,
	ConstStr255Param		valueString,
	emsMIMEParamHandle *	paramHandle )
{
	emsMIMEParamHandle	param;
	Handle				valueHandle;
	OSErr				err	= noErr;
	
	param	= (emsMIMEParamHandle) NewHandleClear( sizeof( **param ) );
	if ( IsntNull( param ) )
	{
		CopyPString( nameString, (**param).name );
		(**param).next	= nil;
		
		err	= PtrToHand( &valueString[ 1 ], &valueHandle, valueString[ 0 ] );
		if ( IsErr( err )  )
		{
			DisposeHandle( (Handle) param );
			param	= nil;
		}
		else
		{
			(**param).value	= valueHandle;
		}
	}
	else
	{
		err	= memFullErr;
	}
	
	*paramHandle	= param;
	return( err );
}


			
	OSErr
AddMIMEParam(
	emsMIMETypeHandle	typeHandle,
	ConstStr255Param	nameString,
	ConstStr255Param	valueString
	)
{
	OSErr				err	= noErr;
	emsMIMEParamHandle	paramHandle;
	
	AssertHandleIsValid( typeHandle, "AddMIMEParam" );
	pgpAssertAddrValid( nameString, uchar );
	pgpAssertAddrValid( valueString, uchar );
	
	err	= CreateMIMEParam( nameString, valueString, &paramHandle );
	if ( IsntErr( err ) )
	{
		if ( IsNull( (**typeHandle).params ) )
		{
			(**typeHandle).params	= paramHandle;
		}
		else
		{
			emsMIMEParamHandle	curParamHandle;
	
			// now find the last param, and add the new param to it
			curParamHandle	= (**typeHandle).params;
			while ( IsntNull( curParamHandle ) )
			{
				if ( IsNull( (**curParamHandle).next ) )
				{
					(**curParamHandle).next	= paramHandle;
					break;
				}
				
				curParamHandle	= (**curParamHandle).next;
			}
		}
	}
	
	return( err );
}


	OSErr
CreateMIMEType(
	ConstStr255Param		typeString,
	ConstStr255Param		subtypeString,
	emsMIMETypeHandle *		typeHandlePtr )
{
	emsMIMEType		type;
	OSErr			err	= noErr;
	
	CopyPString( "\p1.0", type.mime_version );
	CopyPString( typeString, type.mime_type );
	CopyPString( subtypeString, type.sub_type );
	type.params	= nil;
	
	err	= PtrToHand( &type, (Handle *)typeHandlePtr, sizeof( type ) );
	
	return( err );
}

	
	void
DisposeMIMEType( emsMIMETypeHandle	mimeTypeHandle)
{
	emsMIMEParamHandle	params;
	
	params	= (**mimeTypeHandle).params;
	
	while ( IsntNull( params ) )
	{
		emsMIMEParamHandle	next;
		
		next	= (**params).next;
		DisposeHandle( (Handle) params );
		params	= next;
	}
	
	DisposeHandle( (Handle) mimeTypeHandle );
}


/*____________________________________________________________________________
	Build a Eudora data structure representing:
	multipart/encrypted; boundary=foo; protocol="application/pgp-encrypted"
____________________________________________________________________________*/
	OSErr
BuildEncryptedPGPMIMEType(
	emsMIMETypeHandle *	mimeTypeHandlePtr,
	const char *		boundaryCString )
{
	OSErr				err	= noErr;
	emsMIMETypeHandle	mimeTypeHandle = nil;

	err	= CreateMIMEType( "\pmultipart", "\pencrypted", &mimeTypeHandle );
	if ( IsntErr( err ) )
	{
		Str255	tempString;
		
		CToPString( boundaryCString, tempString );
		
		err	= AddMIMEParam( mimeTypeHandle, "\pboundary", tempString );
		
		if ( IsntErr( err ) )
		{
			err	= AddMIMEParam( mimeTypeHandle,
					"\pprotocol", "\papplication/pgp-encrypted" );
		}
		
		
		if ( IsErr( err ) )
		{
			DisposeMIMEType( mimeTypeHandle );
			mimeTypeHandle	= nil;
		}
	}
	
	*mimeTypeHandlePtr	= mimeTypeHandle;
	
	AssertNoErr( err, "BuildEncryptedMIMEType" );
	return( err );
}


/*____________________________________________________________________________
	Build a Eudora data structure representing:
	multipart/signed; boundary=foo; protocol="application/pgp-signature"
____________________________________________________________________________*/
	OSErr
BuildSignedPGPMIMEType(
	emsMIMETypeHandle *	mimeTypeHandlePtr,
	const char *		boundaryCString )
{
	OSErr				err	= noErr;
	emsMIMETypeHandle	mimeTypeHandle = nil;

	err	= CreateMIMEType( "\pmultipart", "\psigned", &mimeTypeHandle );
	if ( IsntErr( err ) )
	{
		Str255	tempString;
		
		CToPString( boundaryCString, tempString );
		
		err	= AddMIMEParam( mimeTypeHandle, "\pboundary", tempString );
		
		if ( IsntErr( err ) )
		{
			err	= AddMIMEParam( mimeTypeHandle,
					"\pprotocol", "\papplication/pgp-signature" );
		}
		
		if ( IsntErr( err ) )
		{
			err	= AddMIMEParam( mimeTypeHandle, "\pmicalg", "\ppgp-md5" );
		}
		
		
		if ( IsErr( err ) )
		{
			DisposeMIMEType( mimeTypeHandle );
			mimeTypeHandle	= nil;
		}
	}
	
	*mimeTypeHandlePtr	= mimeTypeHandle;
	
	AssertNoErr( err, "BuildEncryptedMIMEType" );
	return( err );
}


/*____________________________________________________________________________
	*typePtr is set to NULL if no valid Content-type is found.
____________________________________________________________________________*/
	void
ParseMIMEType(
	Handle				canonicalizedText,
	emsMIMETypeHandle * typePtr)
{
	*typePtr	= nil;
	
	parse_mime_ct( canonicalizedText, typePtr);
	if ( IsntNull( *typePtr ) )
	{
		// parse_mime_ct doesn't fill in mime_version
		CopyPString( "\p1.0", (**(*typePtr)).mime_version );
	}
}



/*____________________________________________________________________________
	Convert a mime type data structure and its parameters to a Handle of data
____________________________________________________________________________*/
	Handle
MIMETypeToHandle( ConstemsMIMETypeHandle typeHandle )
{
	Handle					theResult	= nil;
	ConstemsMIMEParamHandle	curParamHandle;
	OSErr					err	= noErr;

	AssertHandleIsValid( typeHandle, "MIMETypeToString" );

	theResult	= NewHandle( 0 );
	if ( IsntNull( theResult ) )
	{
		emsMIMEType	type;
		
		type	= **typeHandle;
		
		err	= AppendStringToHandle( theResult, type.mime_type );
		if ( IsntErr( err ) )
			err	= AppendStringToHandle( theResult, "\p/" );
		if ( IsntErr( err ) )
			err	= AppendStringToHandle( theResult, type.sub_type );
	}
    
    // now append all the parameters
    curParamHandle	= (**typeHandle).params;
    while ( IsntNull( curParamHandle ) && IsntErr( err ) )
    {
    	Str255	tempStr;
    	
		err	= AppendStringToHandle( theResult, "\p; " );
		
		if ( IsntErr( err ) )
		{
			CopyPString( (**curParamHandle).name, tempStr );
			err	= AppendStringToHandle( theResult, tempStr );
		}
		
		if ( IsntErr( err ) )
		{
			CopyPString( "\p=\"", tempStr );
			AppendPString( *(StringHandle)((**curParamHandle).value), tempStr );
			AppendPString( "\p\"",  tempStr);
			err	= AppendStringToHandle( theResult, tempStr);
		}
		
        curParamHandle	= (*curParamHandle)->next;
    }
		
	return(theResult);
}



	Boolean
MatchMIMEType(
	ConstemsMIMETypeHandle	typeH,
	ConstStringPtr			typeString,
	ConstStringPtr			subString )
{
    Boolean	matchFound	= false;
    
    AssertHandleIsValid( typeH, "MatchMIMEType");
    pgpAssertAddrValid( typeString, uchar);
    pgpAssertAddrValid( subString, uchar);
    
    if ( IsntNull( typeString ) )
    {
 	    matchFound	= EqualString( (**typeH).mime_type,
 	    				typeString, false, true );
 	}
 	
 	// nil substring means wildcard match; anything matches
    if( matchFound && IsntNull( subString ) )
    {
        matchFound = EqualString( (**typeH).sub_type, subString, false, true );
  	}
   
    return( matchFound );
}





	Boolean
GetMIMEParameter(
	ConstemsMIMETypeHandle	mimeStuff,
	ConstStr255Param		paramName,
	StringPtr				outValue )
{
	ConstemsMIMEParamHandle	param;
	
	outValue[ 0 ]	= 0;
	
	param	= (**mimeStuff).params;
	while ( IsntNull( param ) )
	{
		if ( EqualString( (**param).name, paramName, FALSE, TRUE) )
		{
			ulong	length	= GetHandleSize( (**param).value );
			
			if ( length > 255 )
				length	= 255;
			
			outValue[ 0 ]	= length;
			BlockMoveData( *(**param).value, &outValue[ 1 ], length );
			break;
		}
		param	= (**param).next;
	}

    return( StrLength( outValue ) != 0 );
}				














