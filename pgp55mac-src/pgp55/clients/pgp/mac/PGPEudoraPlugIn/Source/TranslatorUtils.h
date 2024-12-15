/*____________________________________________________________________________
	TranslatorUtils.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: TranslatorUtils.h,v 1.10 1997/09/18 23:30:27 lloyd Exp $
____________________________________________________________________________*/
#pragma once

#include "EudoraTypes.h"

#include "pgpTypes.h"

#include "CComboError.h"

#include "pgpEncode.h"

#ifdef __cplusplus
extern "C" {
#endif




/* we use this to see if we've already processed a message
when Eudora calls CanTranslate file */
const uchar	kPGPAlreadyProcessedParamName[]		= "\pPGPProcessed";
const uchar	kPGPAlreadyProcessedParamValue[]	= "\pyes";

			
			
long	SignatureDataToEudoraResult( PGPEventSignatureData const *sigData );


OSErr	ParseFileForMIMEType( const FSSpec *spec,
			emsMIMETypeHandle *typeHandlePtr );




OSErr	CopyFileToFileRef( const FSSpec * fileToCopy, short destFileRef );

					
					
#if PGP_DEBUG
	void	DebugCopyToRAMDisk( const FSSpec *spec, ConstStr255Param name );
#else
	#define DebugCopyToRAMDisk( spec, name )
#endif



OSErr	FSpReadFileIntoBuffer( const FSSpec * spec,
			void ** bufferOut, ulong *	 bufferSizeOut );


void			MyGetIndCString( ushort whichString, char *theString );
StringHandle	GetIndStringHandle( ushort whichString );


long			CComboErrorToEudoraError( CComboError &err );



OSErr	EliminateMIMEHeader( const FSSpec *spec );


#include "PGPUserInterface.h"
CComboError	GetRecipientList( PGPContextRef context,
				const StringHandle ** addresses,
				PGPKeySetRef allKeys,
				PGPKeySetRef * keySet,
				PGPRecipientSettings *	settings,
				PGPKeySetRef *	keyToAdd);




#ifdef __cplusplus
}
#endif