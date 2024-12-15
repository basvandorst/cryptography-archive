/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpKeyFilterPriv.h,v 1.28 1999/05/18 19:38:45 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeyFilterPriv_h	/* [ */
#define Included_pgpKeyFilterPriv_h

#include <stdio.h>

#include "pgpKeys.h"
#include "pgpBase.h"
#include "pgpErrors.h"
#include "pgpKeyDBOpaqueStructs.h"
#include "pgpRingOpaqueStructs.h"


/* Definitions of key filters */

enum PGPFilterType_
{	
	kPGPFilterTypeStart = 1,

	kPGPFilterTypeKeyKeyID = 2,
	kPGPFilterTypeKeyEncryptAlgorithm = 3,
	kPGPFilterTypeKeyFingerPrint = 4,
	kPGPFilterTypeKeyCreationTime = 5,
	kPGPFilterTypeKeyExpirationTime = 6,
	kPGPFilterTypeKeyRevoked = 7,
	kPGPFilterTypeKeyDisabled = 8,
	kPGPFilterTypeKeySigAlgorithm = 9,
	kPGPFilterTypeKeyEncryptKeySize = 10, 
	kPGPFilterTypeKeySigKeySize = 11,
/*	kPGPFilterTypeUNUSED = 12,			*/
/*	kPGPFilterTypeUNUSED = 13,			*/
	kPGPFilterTypeKeySubKeyID = 14,
	kPGPFilterTypeUserIDString = 15,
	kPGPFilterTypeUserIDEmail = 16,
	kPGPFilterTypeUserIDName = 17,
/*	kPGPFilterTypeUNUSED = 18,			*/

/*	kPGPFilterTypeUNUSED = 19,			*/
/*	kPGPFilterTypeUNUSED = 20,			*/
/*	kPGPFilterTypeUNUSED = 21,			*/
/*	kPGPFilterTypeUNUSED = 22,			*/
	kPGPFilterTypeSigKeyID = 23,
/*	kPGPFilterTypeUNUSED = 24,			*/

	/* Logical operations */
	kPGPFilterTypeNot = 25,
	kPGPFilterTypeAnd = 26,
	kPGPFilterTypeOr = 27,

	/* Generic property filters, to replace most specific ones above */
	kPGPFilterTypeKeyBuffer =		50,
	kPGPFilterTypeKeyNumber =		51,
	kPGPFilterTypeKeyTime =			52,
	kPGPFilterTypeKeyBoolean =		53,
	kPGPFilterTypeUserIDBuffer =	54,
	kPGPFilterTypeUserIDNumber =	55,
	kPGPFilterTypeUserIDTime =		56,
	kPGPFilterTypeUserIDBoolean =	57,
	kPGPFilterTypeSigBuffer =		58,
	kPGPFilterTypeSigNumber =		59,
	kPGPFilterTypeSigTime =			60,
	kPGPFilterTypeSigBoolean =		61,
	kPGPFilterTypeSubKeyBuffer =	62,
	kPGPFilterTypeSubKeyNumber =	63,
	kPGPFilterTypeSubKeyTime =		64,
	kPGPFilterTypeSubKeyBoolean =	65,

	kPGPFilterTypeEnd = 66,

	PGP_ENUM_FORCE( PGPFilterType_ )
};
PGPENUM_TYPEDEF( PGPFilterType_, PGPFilterType );


#define kPGPFilterClassDefault	0x01

typedef PGPUInt16	PGPFilterClass;

#define kPGPFilterMagic		0x46494c54	/* 'FILT' */
struct PGPFilter
{
	PGPUInt32				magic;
	PGPContextRef			context;
	PGPUInt32				refCount;
	PGPMatchCriterion		match;
	PGPFilterClass			filterClass;
	PGPFilterType			filterType;	
	
	/* Select element of union based on type */
	union 
	{
		PGPKeyID			keyKeyID;
		
		PGPPublicKeyAlgorithm	keyEncryptAlgorithm;
		struct
		{
			PGPByte *		keyFingerPrintData;
			PGPUInt32		keyFingerPrintLength;	/* 16 or 20 */
		} keyFingerPrint;
		struct
		{
			PGPByte *		keyX509MD5HashData;
			PGPUInt32		keyX509MD5HashLength;
		} keyX509MD5Hash;
		PGPTime				keyCreationTime;
		PGPTime				keyExpirationTime;
		PGPTime				keyLastModifyTime;
		PGPBoolean			keyRevoked;				
		PGPBoolean			keyDisabled;
		PGPByte				keySigAlgorithm;		
		PGPUInt32			keyEncryptKeySize;
		PGPUInt32			keySigKeySize;
		PGPByte				keyVersion;
		PGPKeyID			keySubKeyID;

		char *				userIDString;
		char *				userIDEmail;
		char *				userIDName;
		PGPByte				userIDVersion;

		PGPByte				sigHashAlgorithm;
		PGPPublicKeyAlgorithm	sigKeyAlgorithm;
		PGPByte				sigClassification;
		PGPTime				sigCreationTime;
		PGPKeyID			sigKeyID;

		PGPByte				sigVersion;

		PGPFilter *			notFilter;
		struct 
		{
			PGPFilter *		andFilter1;
			PGPFilter *		andFilter2;
		} andFilter;
		struct 
		{
			PGPFilter *		orFilter1;
			PGPFilter *		orFilter2;
		} orFilter;

		struct 
		{
			PGPUInt32		prop;
			PGPBoolean		val;
		} propbool;
		struct 
		{
			PGPUInt32		prop;
			PGPUInt32		val;
		} propnum;
		struct 
		{
			PGPUInt32		prop;
			PGPTime			val;
		} proptime;
		struct 
		{
			PGPUInt32		prop;
			void		   *val;
			PGPSize			len;
		} propbuffer;

	} value;
};

	PGPBoolean
pgpKeyObjMatchesFilter(
	PGPFilterRef		filter,
	RingSet const *		ringSet,
	RingObject *		keyObj);

	PGPBoolean
pgpKeyMatchesFilter(
	PGPFilterRef		filter,
	PGPKeyRef			key);


PGPBoolean		PGPFilterIsValid( PGPFilterRef	filter );
#define PGPValidateFilter( filter ) \
	PGPValidateParam( PGPFilterIsValid( filter ) )



#endif /* ] Included_pgpKeyFilterPriv_h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */