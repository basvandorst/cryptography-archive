/*____________________________________________________________________________
	pgpKeyServerPrefs.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	This file contains functions for KeyServer Preference Management.

	$Id: pgpKeyServerPrefs.h,v 1.4 1997/09/18 02:27:54 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeyServerPrefs_h	/* [ */
#define Included_pgpKeyServerPrefs_h

#include "pgpBase.h"
#include "pgpPFLErrors.h"
#include "pflPrefs.h"

#define	kMaxServerNameLength	255

enum    PGPKeyServerEntryFlags
{
	kKeyServerListed	= 1,
	kKeyServerDefault	= 2,
	kKeyServerAdmin		= 4
};
 
typedef struct PGPKeyServerEntry
{
	char		domain[kMaxServerNameLength + 1];	/* these are C strings */
	char		serverURL[kMaxServerNameLength + 1];
	PGPUInt32	flags;
} PGPKeyServerEntry;

#define IsKeyServerDefault(x)	(x & kKeyServerDefault)
#define IsKeyServerListed(x)	(x & kKeyServerListed)
#define IsKeyServerAdmin(x)		(x & kKeyServerAdmin)


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
	extracts the default keyserver from the keyserver array.
	this is a convenience function. stringSize points to the
	size of the passed in buffer. If it is large enough
	the default keyserver URL is copied into it and the
	number of characters copied is assigned to stringSize.
	If it is not large enough, the function returns
	kPGPError_BadParams and the size needed (INCLUDING
	terminating NULL) is returned in stringSize.

*/
	PGPError
PGPGetDefaultKeyServer( PGPPrefRef	prefRef,
						PGPSize		*stringSize,
						char		*string);

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