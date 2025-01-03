/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include "MacCursors.h"
#include "MacFiles.h"
#include "MacStrings.h"
#include "pgpMem.h"
#include "pgpUtilities.h"

#include "PGPtoolsEncryptDecrypt.h"
#include "PGPtoolsResources.h"


	void
GetDefaultEncryptSignName(
	ConstStringPtr	srcName,
	Boolean			textOutput,
	Boolean			detachedSignature,
	StringPtr 		destName)
{
	Str255	suffix;
	short	strIndex;
	
	pgpAssertAddrValid( srcName, uchar );
	pgpAssertAddrValid( destName, uchar );
	
	if( textOutput )
	{
		strIndex = kASCIIFileSuffixStrIndex;
	}
	else if( detachedSignature )
	{
		strIndex = kDetachedBinarySigFileSuffixStrIndex;
	}
	else
	{
		strIndex = kBinaryFileSuffixStrIndex;
	}
	
	GetIndString( suffix, kPGPtoolsMiscStringsResID, strIndex );

	CopyPString( srcName, destName );
	if( destName[0] > kMaxHFSFileNameLength - suffix[0] )
	{
		destName[0] = kMaxHFSFileNameLength - suffix[0];
	}
	
	AppendPString( suffix, destName );
}


// Given a source spec, determine the correct default encrypt output spec.
	void
GetDefaultEncryptSignFSSpec(
	const FSSpec 	*sourceSpec,
	Boolean			textOutput,
	Boolean			detachedSignature,
	FSSpec 			*destSpec)
{
	AssertSpecIsValid( sourceSpec, "GetDefaultEncryptSignFSSpec" );
	pgpAssertAddrValid( destSpec, FSSpec );
	
	destSpec->vRefNum	= sourceSpec->vRefNum;
	destSpec->parID		= sourceSpec->parID;
	GetDefaultEncryptSignName( sourceSpec->name,
		textOutput, detachedSignature, destSpec->name );
}



/*____________________________________________________________________________
	Given a source spec, determine the correct default decrypt output spec.
	
	Examples:
	"X.pgp"	=>	"X"
	"X.asc"	=>	"X"
	"X"		=>	"X"
	
	If no suffix is detected, then the name is not changed, and the caller
	has to figure out a name.
____________________________________________________________________________*/

	Boolean
GetDefaultDecryptVerifyName(
	ConstStringPtr	srcName,
	StringPtr		destName )
{
	Str255		suffix;
	Boolean		foundSuffix	= FALSE;

	pgpAssertAddrValid( srcName, uchar );
	pgpAssertAddrValid( destName, uchar );
	
	GetIndString(suffix, kPGPtoolsMiscStringsResID, kBinaryFileSuffixStrIndex);

	CopyPString( srcName, destName );
	
	foundSuffix	= PStringHasSuffix( destName, suffix, FALSE);
	if ( ! foundSuffix )
	{
		GetIndString( suffix, kPGPtoolsMiscStringsResID,
					kASCIIFileSuffixStrIndex );
		
		foundSuffix	= PStringHasSuffix( destName, suffix, FALSE);
	}
	
	if ( foundSuffix )
	{
		destName[0]	-= suffix[0];
	}
	
	return( foundSuffix );
}


	Boolean
GetDefaultDecryptVerifyFSSpec(
	const FSSpec *	srcSpec,
	FSSpec *		destSpec)
{
	Boolean		foundSuffix	= FALSE;
	
	*destSpec	= *srcSpec;
	
	foundSuffix	= GetDefaultDecryptVerifyName( srcSpec->name, destSpec->name );
	
	return( foundSuffix );
}




/*____________________________________________________________________________
	
____________________________________________________________________________*/
	Boolean
GetDetachedSigFileSourceSpec(const FSSpec *sigSpec, FSSpec *sourceSpec)
{
	Str255	suffix;
	Boolean	matchedSuffix;
	
	*sourceSpec		= *sigSpec;
	matchedSuffix	= FALSE;
	
	GetIndString( suffix, kPGPtoolsMiscStringsResID,
				kDetachedBinarySigFileSuffixStrIndex );
	
	if( PStringHasSuffix( sourceSpec->name, suffix, FALSE) )
	{
		sourceSpec->name[0] -= suffix[0];
		matchedSuffix		= TRUE;
	}
	else
	{
		GetIndString( suffix, kPGPtoolsMiscStringsResID,
				kASCIIFileSuffixStrIndex );
		
		if( PStringHasSuffix( sourceSpec->name, suffix, FALSE) )
		{
			sourceSpec->name[0] -= suffix[0];
			matchedSuffix		= TRUE;
		}
		else
		{
			GetIndString( suffix, kPGPtoolsMiscStringsResID,
						kBinaryFileSuffixStrIndex );
			
			if( PStringHasSuffix( sourceSpec->name, suffix, FALSE) )
			{
				sourceSpec->name[0] -= suffix[0];
				matchedSuffix		= TRUE;
			}
		}
	}
	
	return( matchedSuffix );
}
