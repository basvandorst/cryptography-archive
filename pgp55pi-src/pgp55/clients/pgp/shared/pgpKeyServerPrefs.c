/*____________________________________________________________________________
	pgpKeyServerPrefs.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpKeyServerPrefs.c,v 1.5 1997/09/18 02:27:51 heller Exp $
____________________________________________________________________________*/
#include "pflPrefTypes.h"
#include "pgpPFLErrors.h"
#include "pgpClientPrefs.h"
#include "pgpKeyServerPrefs.h"
#include "pgpDebug.h"
#include "pgpMem.h"

#include <string.h>
#include "pgpStrings.h"

	PGPError
PGPGetKeyServerPrefs(	PGPPrefRef				prefRef,
						PGPKeyServerEntry		**keyserverList,
						PGPUInt32				*keyserverCount)
{
	PGPError	err;
	PGPSize		dataLength;

	PGPValidatePtr(keyserverList);
	PGPValidatePtr(keyserverCount);

	err = PGPGetPrefData(	prefRef, 
							kPGPPrefKeyServerList, 
							&dataLength, 
							keyserverList);

	*keyserverCount = dataLength / sizeof(PGPKeyServerEntry);

	return err;

}

	PGPError
PGPSetKeyServerPrefs(	PGPPrefRef				prefRef,
						PGPKeyServerEntry		*keyserverList,
						PGPUInt32				keyserverCount)
{
	PGPError	err;
	PGPSize		dataLength;

	PGPValidatePtr(keyserverList);

	dataLength = keyserverCount * sizeof(PGPKeyServerEntry);

	err = PGPSetPrefData(	prefRef, 
							kPGPPrefKeyServerList, 
							dataLength, 
							keyserverList);
	return err;
}


	PGPError
PGPGetDefaultKeyServer( PGPPrefRef	prefRef,
						PGPSize		*maxSize,
						char		*string)
{
	PGPError				err;
	PGPKeyServerEntry		*keyserverList;
	PGPSize					dataLength;
	PGPUInt32				count;
	PGPUInt32				index;
	
	PGPValidatePtr(maxSize);
	PGPValidatePtr(string);

	err = PGPGetPrefData(	prefRef, 
							kPGPPrefKeyServerList, 
							&dataLength, 
							&keyserverList);
	if (IsPGPError(err))
		return err;

	count = dataLength / sizeof(PGPKeyServerEntry);

	for(index = 0; index < count; index++)
	{
		if( IsKeyServerDefault(keyserverList[index].flags) )
		{
			PGPSize length;

			length = strlen( keyserverList[index].serverURL );

			if (length >= *maxSize)
			{
				err = kPGPError_BadParams;
				*maxSize = length + 1;
			}

			if( IsntPGPError(err) )
			{				
				strcpy(string, keyserverList[index].serverURL);
				*maxSize = length;
			}

			break;
		}
	}

	PGPDisposePrefData(prefRef, keyserverList);
	
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
	PGPSize					dataLength;
	PGPUInt32				index;
	PGPBoolean				foundDefault = FALSE;
	PGPUInt32				defaultIndex = 0;

	PGPValidatePtr(keyserverPath);
	PGPValidatePtr(keyserverCount);
	PGPValidatePtr(searchDomain);
	
	// init pointers
	*keyserverCount = 0;
	*keyserverPath = pgpAlloc(sizeof(PGPKeyServerEntry));

	err = PGPGetPrefData(	prefRef, 
							kPGPPrefKeyServerList, 
							&dataLength, 
							&keyserverList);
	if (IsPGPError(err))
		return err;

	listCount = dataLength / sizeof(PGPKeyServerEntry);

	for(index = 0; index < listCount; index++)
	{
		// check for a domain match
		if( !pgpCompareStringsIgnoreCase(keyserverList[index].domain,
				searchDomain) )
		{
			// remember if we need to add it to the tail end of the list
			if( IsKeyServerDefault(keyserverList[index].flags) )
				foundDefault = TRUE;

			*keyserverCount = *keyserverCount + 1;
			
			pgpRealloc(	keyserverPath, 
						*keyserverCount * sizeof(PGPKeyServerEntry));

			(*keyserverPath)[*keyserverCount - 1] = keyserverList[index];
		}
		else if( IsKeyServerDefault(keyserverList[index].flags) )
			defaultIndex = index;
	}

	if( !foundDefault ) // add it as the last server to search
	{
		*keyserverCount = *keyserverCount + 1;

		pgpRealloc(	keyserverPath,
					*keyserverCount * sizeof(PGPKeyServerEntry));

		(*keyserverPath)[*keyserverCount - 1] = keyserverList[defaultIndex];
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

	void
PGPFindEmailDomain(const char *emailAddress, char *domain)
{
	char *p = (char *) emailAddress;
	
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

