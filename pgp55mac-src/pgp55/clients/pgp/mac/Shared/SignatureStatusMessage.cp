/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include "MacStrings.h"

#include "PGPSharedEncryptDecrypt.h"
#include "SignatureStatusMessage.h"
#include "pgpKeys.h"
#include "pgpUtilities.h"


const ResID	kSigStatusStrListID	= 14383;	// STR#
enum
{
	kSigStatusTemplateStrIndex	= 1,
	kSigStatusUnknownStrIndex,
	kSigStatusNotVerifiedStrIndex,
	kSigStatusGoodStrIndex,
	
	kSigStatusGoodButNoValidityStrIndex,
	kSigStatusGoodButMarginalValidityStrIndex,
	kSigStatusGoodButUnknownValidityStrIndex,
	kSigStatusGoodButExpiredStrIndex,
	kSigStatusGoodButDisabledStrIndex,
	kSigStatusGoodButRevokedStrIndex,
	kSigStatusGoodButProblemStrIndex,
	kSigStatusKeyCorruptStrIndex,
	
	kSigStatusBadStrIndex
};



	static short
GetSignatureStatusStrIndex(
	PGPEventSignatureData const * sigData,
	char const *			userID )
{
	short		strIndex	= kSigStatusUnknownStrIndex;
	PGPError	err	= kPGPError_NoErr;
	
	if ( sigData->checked )
	{
		if ( sigData->verified )
		{
			PGPSharedKeyProperties	properties;
			PGPValidity				validity;
			
			/* checked, verified  */
			
			pgpAssert( sigData->signingKey != kPGPInvalidRef );
			validity	= sigData->keyValidity;
			
			strIndex	= kSigStatusUnknownStrIndex;

			// we don't really need all this stuff,
			// but we do need the validity
			err	= PGPSharedGetKeyProperties( 
					sigData->signingKey,
					userID, &properties );
			pgpAssertNoErr( err );
			
			if ( sigData->keyRevoked )
				strIndex	= kSigStatusGoodButRevokedStrIndex;
			else if ( sigData->keyExpired )
				strIndex	= kSigStatusGoodButExpiredStrIndex;
			else if ( sigData->keyDisabled )
				strIndex	= kSigStatusGoodButDisabledStrIndex;
			else if ( ( ! properties.isAxiomatic ) &&
					validity != kPGPValidity_Complete )
			{
				if ( validity == kPGPValidity_Marginal )
				strIndex = kSigStatusGoodButMarginalValidityStrIndex;
				else if ( validity == kPGPValidity_Invalid )
				strIndex = kSigStatusGoodButNoValidityStrIndex;
				else
				strIndex = kSigStatusGoodButUnknownValidityStrIndex;
			}
			else if ( properties.isCorrupt )
			{
				// can this case even occur?  How did we verify things
				// then?
				strIndex	= kSigStatusKeyCorruptStrIndex;
			}
			else
			{
				strIndex	= kSigStatusGoodStrIndex;
			}
		}
		else
		{
			/* checked, ! verified */
			strIndex	= kSigStatusBadStrIndex;
		}
	}
	else
	{
		/* !checked*/
		strIndex	= kSigStatusNotVerifiedStrIndex;
	}
	
	
	return( strIndex );
}

	void
GetSignatureStatusMessage(
	SignatureStatusInfo const *	info,
	StringPtr					msg )
{
	ConstStringPtr	result	= nil;
	ulong			now;
	ushort			strIndex;
	Str255			tempString;
	Str255			statusTemplate;
	char			userName[ 256 ];
	PGPError		err	= kPGPError_NoErr;
	PGPEventSignatureData const *	sigData	= &info->sigData;
	
	GetIndString( statusTemplate, kSigStatusStrListID,
				kSigStatusTemplateStrIndex );
	
	userName[ 0 ]	= 0;
	if ( PGPRefIsValid( sigData->signingKey ) )
	{
		err	= PGPGetPrimaryUserIDNameBuffer( sigData->signingKey,
			sizeof(userName), userName, NULL );
		pgpAssertNoErr( err );
	}
	else
	{
		/* no key, use key ID */
		CopyCString( info->keyIDString, userName );
	}
		
	// *** PGP Signature Status: %s
	strIndex	= GetSignatureStatusStrIndex( sigData, userName);
	GetIndString( tempString, kSigStatusStrListID, strIndex  );
	PrintPString( msg, statusTemplate, tempString );
	
	if ( PGPRefIsValid( sigData->signingKey ) )
	{
		ulong			macTime;

		// *** Signer: %s
		CToPString( userName, tempString );
		PrintPString( msg, msg, tempString );
		
		macTime	= PGPTimeToMacTime( sigData->creationTime );
		// *** Signed: %s, %s
		IUDateString( macTime, shortDate, tempString );
		PrintPString( msg, msg, tempString );
		IUTimeString( macTime, false, tempString );
		PrintPString( msg, msg, tempString );
	}
	else
	{
		const uchar		kNotAvailableString[]	= "\pN/A";
		
		CToPString( userName, tempString );
		PrintPString( msg, msg, tempString );
		PrintPString( msg, msg, kNotAvailableString );
		PrintPString( msg, msg, kNotAvailableString );
	}
		
	// *** Verifed: %s, %s
	GetDateTime( &now );
	IUDateString( now, shortDate, tempString );
	PrintPString( msg, msg, tempString );
	IUTimeString( now, false, tempString );
	PrintPString( msg, msg, tempString );
}




