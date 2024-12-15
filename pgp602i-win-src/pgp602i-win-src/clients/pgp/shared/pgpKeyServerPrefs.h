/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.

	$Id: pgpKeyServerPrefs.h,v 1.13.8.1 1998/11/12 03:11:09 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeyServerPrefs_h	/* [ */
#define Included_pgpKeyServerPrefs_h

#include "pgpPFLErrors.h"
#include "pflPrefs.h"
#include "pgpKeyServer.h"
#include "pgpKeys.h"

#define	kMaxServerNameLength	255

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

enum    PGPKeyServerEntryFlags
{
	kKeyServerListed		= 1,
	kKeyServerIsRoot		= 2
};

typedef struct PGPKeyServerEntry
{
	PGPSize					structSize;
	PGPKeyServerType		protocol;
	char					domain[kMaxServerNameLength + 1];
	char					serverDNS[kMaxServerNameLength + 1];
	PGPUInt16				serverPort;
	char					authKeyIDString[kPGPMaxKeyIDStringSize];
	PGPPublicKeyAlgorithm	authAlg;
	PGPUInt32				flags;
	char					reserved[64];
} PGPKeyServerEntry;

#define IsKeyServerListed(x)	((x & kKeyServerListed) != 0)
#define IsKeyServerRoot(x)		((x & kKeyServerIsRoot) != 0)
#define KeyServersAreEqual(x,y)	\
(	(x).protocol == (y).protocol && \
	(x).serverPort == (y).serverPort && \
	!strcmp((x).domain, (y).domain)	&& \
	!strcmp((x).serverDNS, (y).serverDNS) )

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/*	
	returns an array of PGPKeyServerEntry structures 
	describing the keyservers in the default keyserver
	path and the number of structures in the array.
*/
	PGPError
PGPGetKeyServerPrefs(	PGPPrefRef				prefRef,
						PGPKeyServerEntry		**keyserverList,
						PGPUInt32				*keyserverCount);

/*	
	given an array of PGPKeyServerEntry structures 
	describing the keyservers in the default keyserver
	path it saves them off for later retrieval.
	All domains should be grouped contiguously in the array.
*/
	PGPError
PGPSetKeyServerPrefs(	PGPPrefRef				prefRef,
						PGPKeyServerEntry		*keyserverList,
						PGPUInt32				keyserverCount);

/*	
	given a domain (i.e. pgp.com), returns an array of 
	PGPKeyServerEntry structures describing the keyservers 
	in the keyserver path and the number of structures in 
	the array. The keyservers should be searched in the order 
	returned by the function.
*/

	PGPError
PGPCreateKeyServerPath(	PGPPrefRef				prefRef,
						const char*				searchDomain,
						PGPKeyServerEntry		**keyserverPath,
						PGPUInt32				*keyserverCount);
/*
	frees the array returned by PGPCreateKeyServerPath()
*/

	PGPError
PGPDisposeKeyServerPath(PGPKeyServerEntry *keyserverPath);

/* Returns root server */
	PGPError
PGPGetRootKeyServer(	PGPPrefRef			prefRef,
						PGPKeyServerEntry	*rootServer );

/* Utility returns DNS name prefixed by protocol URL identifier */

	void
PGPGetKeyServerURL(	PGPKeyServerEntry	*keyserver,
					char				*url );

/* Little utility function to extract the domain from an email address */

	void
PGPFindEmailDomain(const char *emailAddress, char *domain);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpKeyServerPrefs_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/