/*____________________________________________________________________________
	MyMIMEUtils.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MyMIMEUtils.h,v 1.1 1997/07/25 23:47:28 lloyd Exp $
____________________________________________________________________________*/

#pragma once
 
#include "EudoraTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

			
OSErr	AddMIMEParam( emsMIMETypeHandle,
			ConstStr255Param nameString, ConstStr255Param valueString);


OSErr	CreateMIMEType( ConstStr255Param typeString,
			ConstStr255Param subtypeString, emsMIMETypeHandle * typeHandlePtr );
			
			
void	DisposeMIMEType( emsMIMETypeHandle	mimeTypeHandle);


void	ParseMIMEType( Handle canonicalizedText, emsMIMETypeHandle * type);

Handle	MIMETypeToHandle( ConstemsMIMETypeHandle type );


Boolean	MatchMIMEType( ConstemsMIMETypeHandle	MIMEtypeHandle,
			ConstStringPtr mType, ConstStringPtr subtype);




OSErr	BuildEncryptedPGPMIMEType( emsMIMETypeHandle *	mimeTypeHandle,
			const char *	delimiter);
			
OSErr	BuildSignedPGPMIMEType( emsMIMETypeHandle *	mimeTypeHandle,
			const char *	delimiter);


Boolean	GetMIMEParameter(
			ConstemsMIMETypeHandle MIMEtypeHandle, ConstStr255Param	paramName,
			StringPtr value );


#ifdef __cplusplus
}
#endif