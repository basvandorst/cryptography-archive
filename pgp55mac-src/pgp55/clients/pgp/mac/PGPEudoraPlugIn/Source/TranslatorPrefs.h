/*____________________________________________________________________________
	TranslatorPrefs.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: TranslatorPrefs.h,v 1.5 1997/10/10 23:20:22 lloyd Exp $
____________________________________________________________________________*/

#pragma once
 
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	PGPUInt32			numAlgs;
	PGPCipherAlgorithm	algs[ 20 ];
} PreferredAlgorithmsStruct;

void		RefreshPrefs( void );
Boolean		PrefShouldUsePGPMimeEncrypt( void );
Boolean		PrefShouldUsePGPMimeSign( void );
Boolean		PrefShouldBreakLines( ulong *lineLength );

ulong		PrefGetSigningPassphraseCacheSeconds( void );
ulong		PrefGetEncryptionPassphraseCacheSeconds( void );

Boolean		PrefGetComment( char comment[ 256 ] );

PGPCipherAlgorithm	PrefGetConventionalCipher( void );

void		PrefGetPreferredAlgorithms( PreferredAlgorithmsStruct *algs );



#ifdef __cplusplus
}
#endif