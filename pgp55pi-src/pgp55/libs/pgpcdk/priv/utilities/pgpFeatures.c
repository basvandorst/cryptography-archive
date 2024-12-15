/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$Id: pgpFeatures.c,v 1.20.2.1 1997/11/08 01:10:37 lloyd Exp $
____________________________________________________________________________*/
#include "pgpSDKBuildFlags.h"
#include "pgpConfig.h"
#include <string.h>
#include "pgpDebug.h"

#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpUtilities.h"

#include "pgpFeatures.h"
#include "pgpTimeBomb.h"

#include "pgpDES3.h"
#include "pgpCAST5.h"
#include "pgpIDEA.h"

#ifndef PGP_RSA
#error "PGP_RSA requires a value"
#endif

	PGPError 
PGPGetSDKVersion( PGPUInt32 *version )
{
	PGPValidatePtr( version );
	*version	= kPGPsdkAPIVersion;
	
	return( kPGPError_NoErr );
}






/*____________________________________________________________________________
	Return a flags word for the feature selector
____________________________________________________________________________*/
	PGPError 
PGPGetFeatureFlags(
	PGPFeatureSelector	selector,
	PGPFlags *			flagsPtr )
{
	PGPError	err		= kPGPError_NoErr;
	PGPFlags	flags	= 0;
	
	PGPValidatePtr( flagsPtr );
	*flagsPtr	= 0;
	
	switch ( selector )
	{
		default:
			/* update comment in pgpFeatures.h if this changes */
			err		= kPGPError_ItemNotFound;
			flags	= 0;
			break;
			
		case kPGPFeatures_GeneralSelector:
		{
			flags	= 	kPGPFeatureMask_CanEncrypt |
						kPGPFeatureMask_CanDecrypt |
						kPGPFeatureMask_CanSign |
						kPGPFeatureMask_CanVerify;
			break;
		}
			
		case kPGPFeatures_ImplementationSelector:
		{
			flags	= 0;
			
		#if PGP_DEBUG
			flags	|= kPGPFeatureMask_IsDebugBuild;
		#endif
		
		#ifndef PGP_TIME_BOMB
		#error PGP_TIME_BOMB must be defined
		#endif
		
		#if PGP_TIME_BOMB
			flags	|= kPGPFeatureMask_HasTimeout;
		#endif
			break;
		}
	}
	
	*flagsPtr	= flags;
	
	return( err );
}



/*____________________________________________________________________________
	Return a C string of the form:
		"PGPsdk version 1.0 (C) 1997 Pretty Good Privacy, Inc"
____________________________________________________________________________*/
	PGPError 
PGPGetSDKString( char theString[ 256 ] )
{
	static const char	kVersionString[]	=
		"PGPsdk version 1.1.1 (C) 1997 Pretty Good Privacy, Inc."
#if ! PGP_RSA
		" (Diffie-Helman/DSS-only version)"
#endif
		;
	
	PGPValidatePtr( theString );
	/* leave this in even in non-debug; enforce having the space */
	pgpClearMemory( theString, 256 );
	
	strcpy( theString, kVersionString );
	
	return( kPGPError_NoErr );
}


static const PGPPublicKeyAlgorithmInfo	sPublicKeyAlgs[] =
{

#if PGP_RSA
	{{ "RSA",		"RSA",		"", 0, {0,} },
		kPGPPublicKeyAlgorithm_RSA, TRUE, TRUE, 0, 0 },
#endif /* PGP_RSA */
		
	{{ "ElGamal",	"ElGamal",	"", 0, {0,} },
		kPGPPublicKeyAlgorithm_ElGamal, TRUE, FALSE, 0, 0 },
		
	{{ "DSA",		"Digital Signature Standard",	"", 0, {0,} },
		kPGPPublicKeyAlgorithm_DSA, FALSE, TRUE, 0, 0 },
};
#define kNumPublicKeyAlgs	\
( sizeof( sPublicKeyAlgs ) / sizeof( sPublicKeyAlgs[ 0 ] ) )


	PGPError
PGPCountPublicKeyAlgorithms( PGPUInt32 *numAlgs )
{
	PGPValidatePtr( numAlgs );
	
	*numAlgs	= kNumPublicKeyAlgs;
	return( kPGPError_NoErr );
}


	PGPError
PGPGetIndexedPublicKeyAlgorithmInfo(
	PGPUInt32					index,
	PGPPublicKeyAlgorithmInfo *	info)
{
	PGPValidatePtr( info );
	pgpClearMemory( info, sizeof( *info ) );
	PGPValidateParam( index < kNumPublicKeyAlgs );

	*info	= sPublicKeyAlgs[ index ];
	
	return( kPGPError_NoErr );
}





static const PGPSymmetricCipherInfo	sSymmetricCipherAlgs[] =
{
#if PGP_IDEA
	{{ "IDEA",	"IDEA",		"",	0, {0,} },		kPGPCipherAlgorithm_IDEA},
#endif

#if PGP_DES3
	{{ "3DES",	"3DES",		"",	0, {0,} },		kPGPCipherAlgorithm_3DES},
#endif
	
#if PGP_CAST5
	{{ "CAST5",	"CAST5",	"",	0, {0,} },		kPGPCipherAlgorithm_CAST5},
#endif
};
#define kNumSymmetricCipherAlgs		\
	( sizeof( sSymmetricCipherAlgs ) / sizeof( sSymmetricCipherAlgs[ 0 ] ) )


/*____________________________________________________________________________
	The call to this routine ensures that these algorithms are actually
	present; it's intended to make the programmer notice if one or more
	of them are not linked in.
____________________________________________________________________________*/
	static const PGPCipherVTBL *
sEnsureSymmetricCipherLinked( PGPCipherAlgorithm	alg)
{
	PGPCipherVTBL const *vtbl	= NULL;
	
	switch( alg )
	{
		default:
			break;
	#if PGP_IDEA
		case kPGPCipherAlgorithm_IDEA:	vtbl	= &cipherIDEA;	break;
	#endif
	
	#if PGP_DES3
		case kPGPCipherAlgorithm_3DES:	vtbl	= &cipher3DES;	break;
	#endif
	
	#if PGP_CAST5
		case kPGPCipherAlgorithm_CAST5:	vtbl	= &cipherCAST5;	break;
	#endif
	}
	
	return( vtbl );
}

	PGPError
PGPCountSymmetricCiphers( PGPUInt32 *numAlgs )
{
	PGPValidatePtr( numAlgs );
	
	*numAlgs	= kNumSymmetricCipherAlgs;
	return( kPGPError_NoErr );
}


	PGPError
PGPGetIndexedSymmetricCipherInfo(
	PGPUInt32					index,
	PGPSymmetricCipherInfo *	info)
{
	PGPValidatePtr( info );
	pgpClearMemory( info, sizeof( *info ) );
	PGPValidateParam( index < kNumSymmetricCipherAlgs );

	*info	= sSymmetricCipherAlgs[ index ];
	
	sEnsureSymmetricCipherLinked( info->algID );
	
	return( kPGPError_NoErr );
}
				



		
/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
