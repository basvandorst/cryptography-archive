/*____________________________________________________________________________
	PluginLibUtils.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PlugInLibUtils.cp,v 1.5 1997/09/17 16:25:29 heller Exp $
____________________________________________________________________________*/

#include "MacStrings.h"
#include "PluginLibUtils.h"
#include "pgpKeys.h"




CIterateUserIDs::CIterateUserIDs( PGPContextRef context )
{
	mContext	= context;
	mKeySet		= nil;
	mKeyList	= nil;
	mKeyIter	= nil;
}
	
	
CIterateUserIDs::~CIterateUserIDs( )
{
	if ( IsntNull( mKeySet ) )
		PGPFreeKeySet( mKeySet );
		
	if ( IsntNull( mKeyList ) )
		PGPFreeKeyList( mKeyList );
		
	if ( IsntNull( mKeyIter ) )
		PGPFreeKeyIter( mKeyIter );
}


	PGPError
CIterateUserIDs::Init( )
{
	PGPError	err	= kPGPError_LazyProgrammer;
	
	if ( IsNull( mKeySet ) )
	{
		err	= PGPOpenDefaultKeyRings( mContext, FALSE, &mKeySet );
		if ( IsntNull( mKeySet ) )
			err	= kPGPError_NoErr;
	}
		
	if ( IsntNull( mKeySet ) && IsNull( mKeyList ) )
		err	= PGPOrderKeySet( mKeySet, kPGPAnyOrdering, &mKeyList);
		
	if ( IsntPGPError( err ) && IsNull( mKeyIter ) )
		err	= PGPNewKeyIter( mKeyList, &mKeyIter);
	
	return( err );
}


	PGPError
CIterateUserIDs::Iterate( )
{
	PGPError		err	= kPGPError_NoErr;
	
	err	= Init( );
		
	if ( err == kPGPError_NoErr )
	{
		PGPKey *		key	= nil;
		Boolean			keepIterating	= true;
	
		while ( IsntPGPError( err = PGPKeyIterNext( mKeyIter, &key) ) )
		{
			PGPUserID *	userID;
			
			while ( IsntPGPError( err = PGPKeyIterNextUserID( mKeyIter,
					&userID)  ) )
			{
				keepIterating	= ExamineUserID( userID );
				if ( ! keepIterating )
					break;
			}
			if ( ! keepIterating )
				break;
		}
	}
	
	// destroy the iterator, but keep the other stuff, in case caller wants
	// to iterate again
	if ( IsntNull( mKeyIter ) )
	{
		PGPFreeKeyIter( mKeyIter );
		mKeyIter	= nil;
	}
	
	return( err );
}


#pragma mark -



/*____________________________________________________________________________
	user ID will probably be in the form "lloyd <lloyd@pgp.com>"
	
	So we have to look for our desired name
____________________________________________________________________________*/
	Boolean
CFindUserID::ExamineUserID( PGPUserID *	userID )
{
	Boolean		keepGoing	= true;
	
	char				name[ 256 ];
	size_t				totalSize;
	PGPError			err;
	
	// use size -1 so we can append a null
	err		= PGPGetUserIDStringBuffer( userID, kPGPUserIDPropName,
				sizeof( name ), name, &totalSize);
	
	if ( IsntPGPError( err ) )
	{
		if ( FindSubStringNoCase( (const char *)name, mSearchName ) != nil )
		{
			mFoundIt	= true;
			keepGoing	= false;
		}
	}
	
	return( keepGoing );
}


	Boolean
CFindUserID::UserIDExists(
	const char *	userID,
	PGPError *		pgpErrPtr)
{
	pgpAssertAddrValid( userID, char );
	
	mSearchName	= userID;
	mFoundIt	= false;
	
	*pgpErrPtr	= Init();
	if ( *pgpErrPtr == kPGPError_NoErr )
	{
		*pgpErrPtr	= Iterate();
	}
	
	return( mFoundIt );
}



#pragma mark -





