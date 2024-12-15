/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.


	$Id: pgpFeatures.c,v 1.27.2.2.2.1 1998/11/12 03:23:06 heller Exp $



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

#include "pgpPubKey.h"
#include "pgpDES3.h"
#include "pgpCAST5.h"

#if PGP_IDEA
#include "pgpIDEA.h"
#endif

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
			flags	=	0
#if !PGP_ENCRYPT_DISABLE
						| kPGPFeatureMask_CanEncrypt
#endif
#if !PGP_DECRYPT_DISABLE
						| kPGPFeatureMask_CanDecrypt
#endif
#if !PGP_SIGN_DISABLE
						| kPGPFeatureMask_CanSign
#endif
#if !PGP_VERIFY_DISABLE
						| kPGPFeatureMask_CanVerify
#endif
						;
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
		"PGPsdk version 1.5.2 (C) 1997-1998 Network Associates, Inc. and its "
		"affiliated companies."
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


static PGPPublicKeyAlgorithmInfo	sPublicKeyAlgs[] =
{

#if PGP_RSA
	#if PGP_USECAPIFORRSA
		#define kRSALongVersionString	"RSA (CAPI)"
	#elif PGP_USEBSAFEFORRSA
		#define kRSALongVersionString	"RSA (BSAFE)"
	#elif PGP_USERSAREF
		#define kRSALongVersionString	"RSA (RSAREF)"
	#elif PGP_USEPGPFORRSA
		#define kRSALongVersionString	"RSA (PGP)"
	#else
		#error Unknown RSA implementation
	#endif
	
/* Must be first one, we skip it if not available at run time */
	{{ "RSA", kRSALongVersionString, "", 0, {0,} },
		kPGPPublicKeyAlgorithm_RSA,
	 	!PGP_ENCRYPT_DISABLE, !PGP_DECRYPT_DISABLE,
	 	!PGP_SIGN_DISABLE, !PGP_VERIFY_DISABLE, TRUE,
	    0, 0, 0 },
#endif /* PGP_RSA */
		
	{{ "ElGamal",	"ElGamal",	"", 0, {0,} },
		kPGPPublicKeyAlgorithm_ElGamal,
	 	!PGP_ENCRYPT_DISABLE, !PGP_DECRYPT_DISABLE, FALSE, FALSE, TRUE,
	    0, 0, 0 },
		
	{{ "DSA",		"Digital Signature Standard",	"", 0, {0,} },
		kPGPPublicKeyAlgorithm_DSA,
	 	FALSE, FALSE, !PGP_SIGN_DISABLE, !PGP_VERIFY_DISABLE, TRUE,
        0, 0, 0 },
};
#define kNumPublicKeyAlgs	\
( sizeof( sPublicKeyAlgs ) / sizeof( sPublicKeyAlgs[ 0 ] ) )


	PGPError
PGPCountPublicKeyAlgorithms( PGPUInt32 *numAlgs )
{
	PGPValidatePtr( numAlgs );
	
	*numAlgs	= kNumPublicKeyAlgs;
#if PGP_RSA
	if( pgpKeyUse( pgpPkalgByNumber( kPGPPublicKeyAlgorithm_RSA ) ) == 0 )
		*numAlgs -= 1;
#endif
	return( kPGPError_NoErr );
}


	PGPError
PGPGetIndexedPublicKeyAlgorithmInfo(
	PGPUInt32					idx,
	PGPPublicKeyAlgorithmInfo *	info)
{
	PGPValidatePtr( info );
	pgpClearMemory( info, sizeof( *info ) );

#if PGP_RSA
	{
		int rsause =
			pgpKeyUse( pgpPkalgByNumber( kPGPPublicKeyAlgorithm_RSA ) );
		if (rsause == 0)
			idx += 1;
		if( idx == 0 ) {
#if PGP_ENCRYPT_DISABLE
			sPublicKeyAlgs[0].canEncrypt	= FALSE;
#else
			sPublicKeyAlgs[0].canEncrypt	= !!(rsause & PGP_PKUSE_ENCRYPT);
#endif
#if PGP_DECRYPT_DISABLE
			sPublicKeyAlgs[0].canDecrypt	= FALSE;
#else
			sPublicKeyAlgs[0].canDecrypt	= !!(rsause & PGP_PKUSE_ENCRYPT);
#endif
#if PGP_SIGN_DISABLE
			sPublicKeyAlgs[0].canSign		= FALSE;
#else
			sPublicKeyAlgs[0].canSign		= !!(rsause & PGP_PKUSE_SIGN);
#endif
#if PGP_VERIFY_DISABLE
			sPublicKeyAlgs[0].canVerify	= 	FALSE;
#else
			sPublicKeyAlgs[0].canVerify		= !!(rsause & PGP_PKUSE_SIGN);
#endif
		}
	}
#endif	

	PGPValidateParam( idx < kNumPublicKeyAlgs );

	*info	= sPublicKeyAlgs[ idx ];
	
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
	PGPUInt32					idx,
	PGPSymmetricCipherInfo *	info)
{
	PGPValidatePtr( info );
	pgpClearMemory( info, sizeof( *info ) );
	PGPValidateParam( idx < kNumSymmetricCipherAlgs );

	*info	= sSymmetricCipherAlgs[ idx ];
	
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
