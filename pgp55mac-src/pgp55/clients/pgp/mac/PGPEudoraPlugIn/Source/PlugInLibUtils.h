/*____________________________________________________________________________
	PluginLibUtils.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PlugInLibUtils.h,v 1.2 1997/07/30 21:56:53 lloyd Exp $
____________________________________________________________________________*/
#pragma once


#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpUtilities.h"
#include "CFree.h"


class CIterateUserIDs
{
protected:
	PGPContextRef	mContext;
	PGPKeySetRef	mKeySet;
	PGPKeyListRef	mKeyList;
	PGPKeyIterRef	mKeyIter;
	
	virtual Boolean		ExamineUserID(  PGPUserID *	userID ) = 0;
	
public:
						CIterateUserIDs( PGPContextRef context );
	virtual				~CIterateUserIDs( void );
	
	virtual PGPError	Init( void );
	virtual PGPError	Iterate( void );
} ;


class CFindUserID : public CIterateUserIDs
{
protected:
	const char *	mSearchName;
	Boolean			mFoundIt;
	
	virtual Boolean	ExamineUserID( PGPUserID *	userID );
	
public:
						CFindUserID( PGPContextRef context )
							: CIterateUserIDs( context )
						{}
						
	void				CIterateUserIDs( PGPContextRef context );
	Boolean				UserIDExists( const char *userID, PGPError *err);
} ;



class CFreePGPFileRef : public CFree
{
public:			
				CFreePGPFileRef( PGPFileSpecRef ref ) : CFree( ref ) {}

	virtual		~CFreePGPFileRef(  )
					{ if ( mMemory != NULL )
						PGPFreeFileSpec( (PGPFileSpecRef) mMemory ); }
};





