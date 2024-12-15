/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <Timer.h>
#include <string.h>
 
#include "pgpKeys.h"
#include "pgpUtilities.h"
 
#include "PassphraseCache.h"
#include "MacMemory.h"
#include "MacStrings.h"
#include "MacEnvirons.h"

#include "SetupA4.h"
#include "A4Stuff.h"
 
 
CPassphraseCache::CPassphraseCache( )
{
	RememberA4();
	
	const ulong	kDefaultEraseDelayMilliseconds	=
				kDefaultEraseDelaysSeconds * 1000;
	
	mPassphrase				= nil;
	mPassphraseValid		= false;
	mEraseTaskIsInstalled	= false;
	mEraseDelayMilliseconds	= kDefaultEraseDelayMilliseconds;
	
	mEraseTask.upp	= NewTimerProc( sEraseTask );
	pgpAssert( IsntNull( mEraseTask.upp ) );
	
	mPassphrase	= NewPtrSys( 256 );
	pgpAssert( IsntNull( mPassphrase ) );
	if ( VirtualMemoryIsOn() && IsntNull( mPassphrase ) )
	{
		HoldMemory( mPassphrase, sizeof( mPassphrase ) );
	}
}


CPassphraseCache::~CPassphraseCache( )
{
	RemoveEraseTask();
	
	if ( IsntNull( mEraseTask.upp ) )
	{
		DisposeRoutineDescriptor( mEraseTask.upp );
	}
	
	if ( VirtualMemoryIsOn() && IsntNull( mPassphrase ) )
	{
		UnholdMemory( mPassphrase, sizeof( mPassphrase ) );
		DisposePtr( mPassphrase );
	}
}


	void
CPassphraseCache::EraseTask( void )
{
	Forget();
}


#if GENERATINGCFM	// [

	pascal void
CPassphraseCache::sEraseTask( MyTMTask *myTask )
{
	pgpAssert( myTask->magic == MyTMTask::kMagic );
	
	myTask->thisObject->EraseTask();
}

#else		// ][

	pascal void
CPassphraseCache::sEraseTask( void )
{
	MyTMTask *	myTask	= (MyTMTask *)GetA1();
	EnterCodeResource();

	pgpAssert( myTask->magic == MyTMTask::kMagic );
	myTask->thisObject->EraseTask();
	
	ExitCodeResource();
}


#endif		// ]


	void
CPassphraseCache::RemoveEraseTask(  )
{
	if ( mEraseTaskIsInstalled )
	{
		RmvTime( (QElemPtr) &mEraseTask.tmTask );
		mEraseTaskIsInstalled	= false;
	}
}


	void
CPassphraseCache::InstallEraseTask(  )
{
	pgpAssert( ! mEraseTaskIsInstalled );
	
	if ( ! mEraseTaskIsInstalled )
	{
		TMTask *		task	= &mEraseTask.tmTask;
		
		pgpClearMemory( task, sizeof( *task ) );
		
		mEraseTask.thisObject	= this;
	#if PGP_DEBUG
		mEraseTask.magic		= MyTMTask::kMagic;
	#endif
		
		task->tmAddr	= mEraseTask.upp;
		InsTime( (QElemPtr) task );
		PrimeTime( (QElemPtr) task, mEraseDelayMilliseconds );
		
		mEraseTaskIsInstalled	= true;
	}
}


	void
CPassphraseCache::Remember( const char *	passphrase )
{
	if ( IsNull( mPassphrase ) )
		return;
		
	RemoveEraseTask();
	
	if ( mEraseDelayMilliseconds != 0 )
	{
		mPassphraseValid	= true;
		CopyCString( passphrase, mPassphrase );
		
		// garble it to make it harder to find
		char *cur	= mPassphrase;
		while ( *cur != '\0' )
			{
			*cur	= ~ *cur;
			++cur;
			}
		
		InstallEraseTask();
	}
	else
	{
		mPassphraseValid	= false;
		pgpClearMemory( mPassphrase, sizeof( mPassphrase ) );
	}
}


	void
CPassphraseCache::Forget( )
{
	if ( IsNull( mPassphrase ) )
		return;
		
	RemoveEraseTask();
	
	pgpClearMemory( mPassphrase, sizeof( mPassphrase ) );
	mPassphraseValid	= false;
}



	Boolean
CPassphraseCache::GetPassphrase( char *	passphrase )
{
	Boolean	passphraseOK	= false;
	
	if ( mPassphraseValid )
	{
		CopyCString( mPassphrase, passphrase );
		
		// ungarble it
		char *cur	= passphrase;
		while ( *cur != '\0' )
			{
			*cur	= ~ *cur;
			++cur;
			}
		
		passphraseOK	= mPassphrase[ 0 ] != '\0';
	}
	
	return( passphraseOK );
}

#pragma mark -


CSignPassphraseCache::CSignPassphraseCache()
	: CPassphraseCache()
{
	MacDebug_FillWithGarbage( &mKeyID, sizeof( mKeyID ) );
}


/*____________________________________________________________________________
	Retrieve a cached passphrase, together with its corresponding key.
	Fetch the key using its key ID and its PGPPublicKeyAlgorithm
	
	Caller must call PGPFreeKey() on *signingKeyOut
____________________________________________________________________________*/
	Boolean
CSignPassphraseCache::GetPassphrase(
	PGPKeySetRef	allKeys,
	char *			passphrase,
	PGPKeyRef *		keyRefOut )
{
	Boolean		haveCache	= FALSE;
	PGPError	err	= kPGPError_NoErr;
	
	pgpAssertAddrValid( passphrase, char );
	pgpAssertAddrValid( keyRefOut, PGPKeyRef );
	
	*keyRefOut	= kPGPInvalidRef;
	passphrase[ 0 ]	= '\0';
	
	haveCache	= inherited::GetPassphrase( passphrase );
	if ( haveCache )
	{
		err	= PGPGetKeyByKeyID( allKeys,
					&mKeyID, mPublicKeyAlg, keyRefOut );
		
		haveCache	= IsntPGPError( err );
	}
	
	return( haveCache );
}

/*____________________________________________________________________________
	Remember a passphrase, together with its corresponding key.
	Remember the key by turning its key ID into a string and also remembering
	its PGPPublicKeyAlgorithm
____________________________________________________________________________*/
	void
CSignPassphraseCache::Remember(
	const char *		passphrase,
	PGPKeyRef			key)
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpAssertAddrValid( passphrase, char );
	
	err	= PGPGetKeyIDFromKey( key, &mKeyID );
	if ( IsntPGPError( err ) )
	{
		PGPInt32	number;
		
		err	= PGPGetKeyNumber( key, kPGPKeyPropAlgID, &number );
		mPublicKeyAlg	= (PGPPublicKeyAlgorithm)number;
	}
	
	pgpAssertNoErr( err );
	
	inherited::Remember( passphrase );
}

#pragma mark -




CSignPassphraseCache *	gSigningPassphraseCache		= nil;
CPassphraseCache *		gEncryptionPassphraseCache	= nil;
	
	void
InitPassphraseCaches( void )
{
	if ( IsNull( gSigningPassphraseCache ) )
	{
		gSigningPassphraseCache	= new CSignPassphraseCache();
	}
	
	if ( IsNull( gEncryptionPassphraseCache ) )
	{
		gEncryptionPassphraseCache	= new CPassphraseCache();
	}
}

	void
DisposePassphraseCaches( void )
{
	delete gSigningPassphraseCache;
	gSigningPassphraseCache	= nil;
	
	delete gEncryptionPassphraseCache;
	gEncryptionPassphraseCache	= nil;
}
	












