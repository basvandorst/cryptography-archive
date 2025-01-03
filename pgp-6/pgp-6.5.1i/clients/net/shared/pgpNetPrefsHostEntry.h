/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: pgpNetPrefsHostEntry.h,v 1.4 1999/03/28 20:48:05 wprice Exp $
____________________________________________________________________________*/
#ifndef Included_pgpNetPrefsHostEntry_h	/* [ */
#define Included_pgpNetPrefsHostEntry_h

#include "pgpIKE.h"

#define	kMaxNetHostNameLength			255
#define	kMaxNetHostSharedSecretLength	255
#define kMaxNetHostIdentityLength		255
#define kMaxNetHostIASNLength			192
#define kMaxNetHostKeyIDSize			34

enum PGPNetHostType_
{
	kPGPnetInsecureHost			= 0,
	kPGPnetSecureHost			= 1,
	kPGPnetSecureGateway		= 2,

	PGP_ENUM_FORCE( PGPNetHostType_ )
};
PGPENUM_TYPEDEF( PGPNetHostType_, PGPNetHostType );

typedef struct PGPNetPrefHostEntry
{
	PGPNetHostType			hostType;
	PGPUInt32				ipAddress;
	PGPUInt32				ipMask;
	PGPInt32				childOf;
	char					hostName[kMaxNetHostNameLength + 1];
	PGPipsecIdentity		identityType;
	PGPByte					identity[kMaxNetHostIdentityLength + 1];
	PGPSize					identityLen;
	char					sharedSecret[kMaxNetHostSharedSecretLength + 1];

	PGPPublicKeyAlgorithm	authKeyAlg;
	PGPByte					authKeyExpKeyID[kMaxNetHostKeyIDSize];
	PGPByte					authCertIASN[kMaxNetHostIASNLength];
	PGPUInt32				authCertIASNLength;
} PGPNetPrefHostEntry;


#endif /* ] Included_pgpNetPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
