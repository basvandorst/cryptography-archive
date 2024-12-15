/*____________________________________________________________________________
	pgpPubTypes.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Opaque types for various modules go in this file.

	$Id: pgpPubTypes.h,v 1.31 1997/10/11 00:55:12 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpPubTypes_h	/* [ */
#define Included_pgpPubTypes_h


#include "pgpConfig.h"
#include "pgpBase.h"


/*____________________________________________________________________________
	General data types used by PGPsdk
____________________________________________________________________________*/
typedef struct PGPContext *			PGPContextRef;


typedef struct PGPFileSpec *		PGPFileSpecRef;
typedef const struct PGPFileSpec *	PGPConstFileSpecRef;

typedef struct PGPOptionList * 			PGPOptionListRef;
typedef const struct PGPOptionList * 	PGPConstOptionListRef;

/*____________________________________________________________________________
	Data types used by the key manipulation functions
____________________________________________________________________________*/

typedef struct PGPKeyDB *			PGPKeyDBRef;
typedef const struct PGPKeyDB *		PGPConstKeyDBRef;

typedef struct PGPKey *				PGPKeyRef;
typedef const struct PGPKey *		PGPConstKeyRef;

typedef struct PGPSubKey *  		PGPSubKeyRef;
typedef const struct PGPSubKey *	PGPConstSubKeyRef;

typedef struct PGPUserID *			PGPUserIDRef;
typedef const struct PGPUserID *	PGPConstUserIDRef;

typedef struct PGPSig *				PGPSigRef;
typedef const struct PGPSig *		PGPConstSigRef;

typedef struct PGPKeySet *			PGPKeySetRef;
typedef const struct PGPKeySet *	PGPConstKeySetRef;

typedef struct PGPKeyList *			PGPKeyListRef;
typedef const struct PGPKeyList *	PGPConstKeyListRef;

typedef struct PGPKeyIter *			PGPKeyIterRef;
typedef const struct PGPKeyIter *	PGPConstKeyIterRef;

typedef struct PGPFilter *			PGPFilterRef;
typedef const struct PGPFilter *	PGPConstFilterRef;

typedef struct PGPKeyID
{
	/* do not attempt to interpret these bytes; they *will* change */
	PGPByte		opaqueBytes[ 34 ];
} PGPKeyID;


/*____________________________________________________________________________
	Data types used by symmetric ciphers, cipher modes, hashing
____________________________________________________________________________*/
typedef struct PGPHashContext *						PGPHashContextRef;
typedef struct PGPHashContext const *				PGPConstHashContextRef;

typedef struct PGPPublicKeyContext *			PGPPublicKeyContextRef;
typedef struct PGPPublicKeyContext const *		PGPConstPublicKeyContextRef;
typedef struct PGPPrivateKeyContext *			PGPPrivateKeyContextRef;
typedef struct PGPPrivateKeyContext const *		PGPConstPrivateKeyContextRef;

typedef struct PGPCBCContext *					PGPCBCContextRef;
typedef struct PGPCBCContext const *			PGPConstCBCContextRef;

typedef struct PGPCFBContext *					PGPCFBContextRef;
typedef struct PGPCFBContext const *
												PGPConstCFBContextRef;
typedef struct PGPSymmetricCipherContext *
											PGPSymmetricCipherContextRef;
typedef struct PGPSymmetricCipherContext const *
											PGPConstSymmetricCipherContextRef;


/*____________________________________________________________________________
	Invalid values for each of the "ref" data types. Use these for assignment
	and initialization only. Use the PGPXXXRefIsValid macros (below) to test
	for valid/invalid values.
____________________________________________________________________________*/

#define	kInvalidPGPContextRef		((PGPContextRef) NULL)
#define	kInvalidPGPFileSpecRef		((PGPFileSpecRef) NULL)
#define	kInvalidPGPOptionListRef	((PGPOptionListRef) NULL)

#define	kInvalidPGPKeyDBRef			((PGPKeyDBRef) NULL)
#define	kInvalidPGPKeyRef			((PGPKeyRef) NULL)
#define	kInvalidPGPSubKeyRef		((PGPSubKeyRef) NULL)
#define	kInvalidPGPUserIDRef		((PGPUserIDRef) NULL)
#define	kInvalidPGPSigRef			((PGPSigRef) NULL)
#define	kInvalidPGPKeySetRef		((PGPKeySetRef) NULL)
#define	kInvalidPGPKeyListRef		((PGPKeyListRef) NULL)
#define	kInvalidPGPKeyIterRef		((PGPKeyIterRef) NULL)
#define	kInvalidPGPFilterRef		((PGPFilterRef) NULL)

#define	kInvalidPGPHashContextRef					\
			((PGPHashContextRef) NULL)
#define	kInvalidPGPCFBContextRef			\
			((PGPCFBContextRef) NULL)
#define	kInvalidPGPCBCContextRef			\
			((PGPCBCContextRef) NULL)
#define	kInvalidPGPSymmetricCipherContextRef		\
			((PGPSymmetricCipherContextRef) NULL)

/*	kPGPInvalidRef is deprecated. Please use a type-specific version  */
#define kPGPInvalidRef				NULL

/*____________________________________________________________________________
	Macros to test for ref validity. Use these in preference to comparing
	directly with the kInvalidXXXRef values.
____________________________________________________________________________*/

#define PGPContextRefIsValid( ref )		( (ref) != kInvalidPGPContextRef )
#define PGPFileSpecRefIsValid( ref )	( (ref) != kInvalidPGPFileSpecRef )
#define PGPOptionListRefIsValid( ref )	( (ref) != kInvalidPGPOptionListRef )

#define PGPKeyDBRefIsValid( ref )		( (ref) != kInvalidPGPKeyDBRef )
#define PGPKeyRefIsValid( ref )			( (ref) != kInvalidPGPKeyRef )
#define PGPSubKeyRefIsValid( ref )		( (ref) != kInvalidPGPSubKeyRef )
#define PGPUserIDRefIsValid( ref )		( (ref) != kInvalidPGPUserIDRef )
#define PGPSigRefIsValid( ref )			( (ref) != kInvalidPGPSigRef )
#define PGPKeySetRefIsValid( ref )		( (ref) != kInvalidPGPKeySetRef )
#define PGPKeyListRefIsValid( ref )		( (ref) != kInvalidPGPKeyListRef )
#define PGPKeyIterRefIsValid( ref )		( (ref) != kInvalidPGPKeyIterRef )
#define PGPFilterRefIsValid( ref )		( (ref) != kInvalidPGPFilterRef )

#define PGPHashContextRefIsValid( ref )							\
			( (ref) != kInvalidPGPHashContextRef )
#define PGPCFBContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPCFBContextRef )
#define PGPCBCContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPCBCContextRef )
#define PGPSymmetricCipherContextRefIsValid( ref )				\
			( (ref) != kInvalidPGPSymmetricCipherContextRef )

/*	PGPRefIsValid() is deprecated. Please use a type-specific version  */
#define PGPRefIsValid( ref )			( (ref) != kPGPInvalidRef )

/*____________________________________________________________________________
	Symmetric Ciphers
____________________________________________________________________________*/
enum PGPCipherAlgorithm_
{
	/* do NOT change these values */
	kPGPCipherAlgorithm_None	= 0,
	kPGPCipherAlgorithm_IDEA	= 1,
	kPGPCipherAlgorithm_3DES	= 2,
	kPGPCipherAlgorithm_CAST5	= 3,
	
	
	kPGPNumPossibleCipherAlgorithms = 3,

	PGP_ENUM_FORCE( PGPCipherAlgorithm_ )
};
PGPENUM_TYPEDEF( PGPCipherAlgorithm_, PGPCipherAlgorithm );




/*____________________________________________________________________________
	Public/Private key algorithms
____________________________________________________________________________*/
enum PGPPublicKeyAlgorithm_
{
	/* note: do NOT change these values */
	kPGPPublicKeyAlgorithm_Invalid			= 0xFFFFFFFF,
	kPGPPublicKeyAlgorithm_RSA				= 1,
	kPGPPublicKeyAlgorithm_RSAEncryptOnly	= 2,
	kPGPPublicKeyAlgorithm_RSASignOnly		= 3,
	kPGPPublicKeyAlgorithm_ElGamal			= 0x10, /* A.K.A.Diffie-Hellman */
	kPGPPublicKeyAlgorithm_DSA				= 0x11,
	
	kPGPPublicKeyAlgorithm_First			= kPGPPublicKeyAlgorithm_RSA,
	kPGPPublicKeyAlgorithm_Last				= kPGPPublicKeyAlgorithm_DSA,
	
	PGP_ENUM_FORCE( PGPPublicKeyAlgorithm_ )
};
PGPENUM_TYPEDEF( PGPPublicKeyAlgorithm_, PGPPublicKeyAlgorithm );


/* max length is 255; the +1 is for the trailing \0 */
#define kPGPMaxUserIDSize		( (PGPSize)255 + 1 )


#endif /* ] Included_pgpPubTypes_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
