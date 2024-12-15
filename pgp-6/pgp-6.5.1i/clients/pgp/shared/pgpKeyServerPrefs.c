/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpKeyServerPrefs.c,v 1.21 1999/03/10 02:52:16 heller Exp $
____________________________________________________________________________*/
#include "pflPrefTypes.h"
#include "pgpBuildFlags.h"
#include "pgpPFLErrors.h"
#include "pgpClientPrefs.h"
#include "pgpKeyServerPrefs.h"
#include "pgpDebug.h"
#include "pgpMem.h"

#include <string.h>
#include <stdio.h>
#include "pgpStrings.h"

	PGPError
PGPGetKeyServerPrefs(	PGPPrefRef				prefRef,
						PGPKeyServerEntry		**keyserverList,
						PGPUInt32				*keyserverCount)
{
	PGPError			err;
	PGPSize				dataLength;
	PGPBoolean			badStruct = FALSE;
	PGPUInt32			index;
	PGPKeyServerEntry *	tempList = NULL;

	PGPValidatePtr(keyserverList);
	PGPValidatePtr(keyserverCount);

	err = PGPGetPrefData(	prefRef, 
							kPGPPrefKeyServerList, 
							&dataLength, 
							keyserverList);

	*keyserverCount = dataLength / sizeof(PGPKeyServerEntry);

	if (IsntPGPError(err))
	{
		for (index=0; index<*keyserverCount; index++)
			if ((*keyserverList)[index].structSize != 
				sizeof(PGPKeyServerEntry))
			{
				badStruct = TRUE;
			}
	}

	if (badStruct)
	{
		PGPDisposePrefData(prefRef, *keyserverList);
		tempList = (PGPKeyServerEntry *) PGPNewData( PGPGetDefaultMemoryMgr(),
											sizeof(sDefaultKeyServer),
											kPGPMemoryMgrFlags_Clear);

		pgpCopyMemory(sDefaultKeyServer, tempList, sizeof(sDefaultKeyServer));

		*keyserverCount = sizeof(sDefaultKeyServer) / 
							sizeof(PGPKeyServerEntry);
			
		err = PGPSetKeyServerPrefs(prefRef, tempList, *keyserverCount);
		PGPFreeData(tempList);

		if (IsntPGPError(err))
			err = PGPGetPrefData(prefRef, 
					kPGPPrefKeyServerList, 
					&dataLength, 
					keyserverList);
	}

	return err;

}

	PGPError
PGPSetKeyServerPrefs(
	PGPPrefRef				prefRef,
	PGPKeyServerEntry		*keyserverList,
	PGPUInt32				keyserverCount )
{
	PGPError	err;
	PGPSize		dataLength;
	PGPUInt32	index;

	PGPValidatePtr(keyserverList);

	for (index=0; index<keyserverCount; index++)
		keyserverList[index].structSize = sizeof(PGPKeyServerEntry);

	dataLength = keyserverCount * sizeof(PGPKeyServerEntry);

	err = PGPSetPrefData(	prefRef, 
							kPGPPrefKeyServerList, 
							dataLength, 
							keyserverList);
	return err;
}

	PGPError
PGPCreateKeyServerPath(	PGPPrefRef				prefRef,
						const char*				searchDomain,
						PGPKeyServerEntry		**keyserverPath,
						PGPUInt32				*keyserverCount)
{
	PGPError				err;
	PGPKeyServerEntry		*keyserverList;
	PGPUInt32				listCount;
	PGPUInt32				index;
	char					nullDomain[] = "";
	
	PGPValidatePtr(keyserverPath);
	PGPValidatePtr(keyserverCount);
	
	if( IsNull( searchDomain ) )
		searchDomain = nullDomain;
		
	err = PGPGetKeyServerPrefs(prefRef, &keyserverList, &listCount);
	if( IsPGPError(err) )
		return err;

	// init pointers
	*keyserverCount = 0;
	*keyserverPath = (PGPKeyServerEntry *)pgpAlloc(sizeof(PGPKeyServerEntry));

	for(index = 0; index < listCount; index++)
	{
		// check for a domain match
		if( !pgpCompareStringsIgnoreCase(keyserverList[index].domain,
				searchDomain) )
		{
			*keyserverCount = *keyserverCount + 1;
			
			pgpRealloc(	keyserverPath, 
						*keyserverCount * sizeof(PGPKeyServerEntry));

			(*keyserverPath)[*keyserverCount - 1] = keyserverList[index];
		}
	}

	if( searchDomain[0] ) // add servers that do all domains
	{
		for(index = 0; index < listCount; index++)
		{
			if( !keyserverList[index].domain[0] )
			{
				*keyserverCount = *keyserverCount + 1;
				
				pgpRealloc(	keyserverPath, 
							*keyserverCount * sizeof(PGPKeyServerEntry));

				(*keyserverPath)[*keyserverCount - 1] = keyserverList[index];
			}
		}
	}

	PGPDisposePrefData(prefRef, keyserverList);
	
	return err;
}

	PGPError
PGPDisposeKeyServerPath(PGPKeyServerEntry *keyserverPath)
{
	PGPValidatePtr(keyserverPath);

	if( keyserverPath )
	{
		pgpFree(keyserverPath);
	}
	return kPGPError_NoErr;
}

	PGPError
PGPGetRootKeyServer(
	PGPPrefRef			prefRef,
	PGPKeyServerEntry	*rootServer )
{
	PGPError			err = kPGPError_NoErr;
	PGPKeyServerEntry	*keyserverList;
	PGPSize				dataLength;
	PGPUInt32			count;
	PGPUInt32			index;
	PGPBoolean			found = FALSE;
	
	PGPValidatePtr( rootServer );
	
	err = PGPGetPrefData(	prefRef, 
							kPGPPrefKeyServerList, 
							&dataLength, 
							&keyserverList);
	if (IsPGPError(err))
		return err;

	count = dataLength / sizeof(PGPKeyServerEntry);

	for(index = 0; index < count; index++)
	{
		if( IsKeyServerRoot( keyserverList[index].flags ) )
		{
			*rootServer = keyserverList[index];
			found = TRUE;
			break;
		}
	}
	if( !found )
		for(index = 0; index < count; index++)
		{
			if( keyserverList[index].domain[0] == '\0' )
			{
				*rootServer = keyserverList[index];
				found = TRUE;
				break;
			}
		}

	PGPDisposePrefData(prefRef, keyserverList);
	
	if( !found )
		err = kPGPError_ItemNotFound;
	
	return err;
}

	void
PGPGetKeyServerURL(
	PGPKeyServerEntry	*keyserver,
	char				*url )
{
	switch( keyserver->protocol )
	{
		case kPGPKeyServerType_LDAP:
			strcpy( url, "ldap://" );
			break;
		case kPGPKeyServerType_HTTP:
			strcpy( url, "http://" );
			break;
		case kPGPKeyServerType_LDAPS:
			strcpy( url, "ldaps://" );
			break;
		case kPGPKeyServerType_HTTPS:
			strcpy( url, "https://" );
			break;
	}
	strncat( url, keyserver->serverDNS, 241 );
	if( keyserver->serverPort != 0 )
		sprintf( url, "%s:%d", url, keyserver->serverPort );
}

	void
PGPFindEmailDomain(const char *emailAddress, char *domain)
{
	char *p = (char *) emailAddress;
	
	if (IsNull(emailAddress))
	{
		strcpy(domain, "\0");
		return;
	}

	while(*p && *p != '@')
		p++;
	if(*p)
	{
		p++;
		strcpy(domain, p);
		p = domain;
		while(*p && *p != '>')
			p++;
		if(*p == '>')
			*p = '\0';
	}
}

