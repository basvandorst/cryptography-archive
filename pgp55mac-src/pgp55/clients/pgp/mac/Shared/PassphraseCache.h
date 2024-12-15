/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include <Timer.h>


#include "pgpPubTypes.h"
#include "pgpEncode.h"

class CPassphraseCache
{
	ulong	mEraseDelayMilliseconds;
	
	enum { kDefaultEraseDelaysSeconds	= 2 * 60 };	// 2 minutes;

	typedef struct
	{
		TMTask				tmTask;
		CPassphraseCache *	thisObject;
		TimerUPP			upp;
	#if PGP_DEBUG
		enum { kMagic	= 'ETSK' };
		OSType				magic;
	#endif
	} MyTMTask;
	
protected:
	char *		mPassphrase;
	Boolean		mPassphraseValid;

	Boolean		mEraseTaskIsInstalled;
	MyTMTask	mEraseTask;
	
	void		RemoveEraseTask( void );
	void		InstallEraseTask( void );
	
	void		EraseTask( void );
	#if GENERATINGCFM
	static pascal void	sEraseTask( MyTMTask *task );
	#else
	static pascal void	sEraseTask( void );
	#endif
	
public:
			CPassphraseCache( void );
	virtual	~CPassphraseCache( void );
	
	void		Remember( const char *	passphrase );
	void		Forget( void );
	Boolean		GetPassphrase( char *	passphrase );
	
	void		SetCacheSeconds( ulong numSeconds )
					{ mEraseDelayMilliseconds = numSeconds * 1000; }
};


class CSignPassphraseCache : public CPassphraseCache
{
private:
	PGPKeyID				mKeyID;
	PGPPublicKeyAlgorithm	mPublicKeyAlg;
	
public:
	CSignPassphraseCache( void );
	
	/* key is no longer needed after remembering */
	void		Remember( const char *passphrase, PGPKeyRef key );
	
	/* key set MUST be valid */
	Boolean		GetPassphrase( PGPKeySetRef allKeys,
					char *passphrase, PGPKeyRef *key);
};


#ifdef __cplusplus
extern "C" {
#endif

 
extern CSignPassphraseCache *	gSigningPassphraseCache;
extern CPassphraseCache *		gEncryptionPassphraseCache;


void		InitPassphraseCaches( void );
void		DisposePassphraseCaches( void );



#ifdef __cplusplus
}
#endif


