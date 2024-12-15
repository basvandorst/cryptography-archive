/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pflPrefs.c,v 1.13 1997/09/07 00:28:45 lloyd Exp $
____________________________________________________________________________*/
#include "pflContext.h"
#include "pflPrefs.h"
#include "pgpMem.h"
#include "pgpDebug.h"
#include "pgpPFLErrors.h"
#include "pgpFileUtilities.h"
#include <stdio.h>

typedef struct PGPPrefData	PGPPrefData;
typedef struct PGPPref		PGPPref;

struct PGPPrefData
{
	PGPPrefIndex	prefIndex;
	PGPSize			dataLength;
	void *			dataBuffer;

	PGPPrefData *	nextPrefData;
	PGPPrefData *	lastPrefData;
};

struct PGPPref 
{
	PFLContextRef	prefContext;
	PFLFileSpecRef	prefFileSpec;

	PGPPrefData *	prefList;
	PGPPrefData *	prefListEnd;
};


#define PGPValidatePref(prefRef)		PGPValidateParam(prefRef != NULL)

static PGPError pgpFindPrefData(PGPPrefRef prefRef, 
								PGPPrefIndex prefIndex, 
								PGPPrefData **prefData);

static PGPError pgpCalcPrefFileSize(PGPPrefRef prefRef, PGPUInt32 *size);

static PGPError pgpFreePref(PGPPrefRef prefRef);

static PGPError pgpAddPrefData(PGPPrefRef prefRef, 
							   PGPPrefData *prefData);

static PGPError pgpRemovePrefData(PGPPrefRef prefRef, 
								  PGPPrefData *prefData);

static PGPError pgpGetNextPrefData(const PGPPrefData *currentPrefData, 
								   PGPPrefData **nextPrefData);


PGPError PGPOpenPrefFile(PFLFileSpecRef prefFileSpec, 
						 PGPPrefRef *prefRef)
{
	PFLContextRef	context;
	PGPIORef		prefIO;
	PGPPrefIndex	newIndex;
	PGPSize			dataLength;
	void *			newData		= NULL;
	PGPPrefData *	newPrefData = NULL;
	PGPSize			bytesRead;
	PGPError		err;

	if (IsntNull(prefRef))
		*prefRef = NULL;

	PGPValidatePtr(prefRef);
	PFLValidateFileSpec(prefFileSpec);

	context = PFLGetFileSpecContext( prefFileSpec );
	
	pgpAssert(PFLContextIsValid(context));
	if (!PFLContextIsValid(context))
		return kPGPError_BadParams;

	*prefRef = (PGPPrefRef) PFLContextMemAlloc(context, 
								sizeof(PGPPref), 
								kPFLMemoryFlags_Clear);

	pgpAssert(IsntNull(*prefRef));
	if (IsNull(*prefRef))
		return kPGPError_OutOfMemory;

	(*prefRef)->prefContext		= context;
	(*prefRef)->prefFileSpec	= NULL;
	(*prefRef)->prefList		= NULL;
	(*prefRef)->prefListEnd		= NULL;

	err = PFLCopyFileSpec(prefFileSpec, &((*prefRef)->prefFileSpec));
	if (IsPGPError(err))
	{
		pgpFreePref(*prefRef);
		*prefRef = NULL;
		return err;
	}

	err = PGPOpenFileSpec(	(*prefRef)->prefFileSpec, 
							kPFLFileOpenFlags_ReadOnly, 
							(PGPFileIORef *) &prefIO);
	if (IsPGPError(err))
	{
		pgpFreePref(*prefRef);
		*prefRef = NULL;
		return err;
	}

	pgpAssert(PGPIOIsValid(prefIO));
	if (!PGPIOIsValid(prefIO))
	{
		pgpFreePref(*prefRef);
		*prefRef = NULL;
		return kPGPError_OutOfMemory;
	}

	while (!PGPIOIsAtEOF(prefIO))
	{
		newData = NULL;
		newPrefData = NULL;

		err = PGPIORead(prefIO, sizeof(PGPPrefIndex), &newIndex, &bytesRead);
		if (IsPGPError(err))
			break;

		err = PGPIORead(prefIO, sizeof(PGPSize), &dataLength, &bytesRead);
		if (IsPGPError(err))
			break;

		newData = PFLContextMemAlloc(context, 
					dataLength, 
					kPFLMemoryFlags_Clear);

		if (IsNull(newData))
		{
			err = kPGPError_OutOfMemory;
			break;
		}

		err = PGPIORead(prefIO, dataLength, newData, &bytesRead);
		if (IsPGPError(err))
			break;

		newPrefData = (PGPPrefData *) PFLContextMemAlloc(context,
										sizeof(PGPPrefData),
										kPFLMemoryFlags_Clear);
		if (IsNull(newPrefData))
		{
			err = kPGPError_OutOfMemory;
			break;
		}

		newPrefData->prefIndex	= newIndex;
		newPrefData->dataLength = dataLength;
		newPrefData->dataBuffer = newData;

		pgpAddPrefData(*prefRef, newPrefData);
	}

	if (err == kPGPError_EOF)
	{
		if (IsntNull(newData))
			PFLContextMemFree(context, newData);
	}
	else if (IsPGPError(err))
	{
		pgpAssertNoErr(err);
		pgpFreePref(*prefRef);
		*prefRef = NULL;
	}

	PGPFreeIO(prefIO);
	return err;
}


PGPError PGPSavePrefFile(PGPPrefRef prefRef)
{
	PGPPrefData *	currentPrefData;
	PGPIORef		prefIO;
	PGPUInt32		prefFileSize;
	PGPError		err;

	PGPValidatePref(prefRef);

	pgpCalcPrefFileSize(prefRef, &prefFileSize);
	err = PGPSetFileSize(prefRef->prefFileSpec, prefFileSize);

	pgpAssertNoErr(err);
	if (IsPGPError(err))
		return err;

	err = PGPOpenFileSpec(	prefRef->prefFileSpec, 
							kPFLFileOpenFlags_ReadWrite, 
							(PGPFileIORef *) &prefIO);
	pgpAssertNoErr(err);
	if (IsPGPError(err))
		return err;

	currentPrefData = prefRef->prefList;

	while (IsntNull(currentPrefData))
	{
		err = PGPIOWrite(prefIO, 
				sizeof(PGPPrefIndex), 
				&(currentPrefData->prefIndex));

		pgpAssertNoErr(err);
		if (IsPGPError(err))
			break;

		err = PGPIOWrite(prefIO, 
				sizeof(PGPSize), 
				&(currentPrefData->dataLength));

		pgpAssertNoErr(err);
		if (IsPGPError(err))
			break;

		err = PGPIOWrite(prefIO, 
				currentPrefData->dataLength, 
				currentPrefData->dataBuffer);

		pgpAssertNoErr(err);
		if (IsPGPError(err))
			break;

		pgpGetNextPrefData(currentPrefData, &currentPrefData);
	}

	PGPFreeIO(prefIO);
	return err;
}


PGPError PGPClosePrefFile(PGPPrefRef prefRef)
{
	PGPValidatePref(prefRef);

	pgpFreePref(prefRef);
	return kPGPError_NoErr;
}


PGPError PGPGetPrefFileSpec(PGPPrefRef prefRef,
								PFLFileSpecRef *prefFileSpec)
{
	PGPError err = kPGPError_NoErr;

	if (IsntNull(prefFileSpec))
		*prefFileSpec = NULL;

	PGPValidatePref(prefRef);
	PGPValidatePtr(prefFileSpec);

	err = PFLCopyFileSpec(prefRef->prefFileSpec, prefFileSpec);
	if (IsPGPError(err))
		*prefFileSpec = NULL;

	return err;
}


PGPError PGPGetPrefData(PGPPrefRef prefRef, 
						PGPPrefIndex prefIndex, 
						PGPSize *dataLength, 
						void **inBuffer)
{
	PGPPrefData *	currentPrefData;
	PGPError		err;

	if (IsntNull(dataLength))
		*dataLength = 0;
	if (IsntNull(inBuffer))
		*inBuffer = NULL;

	PGPValidatePtr(inBuffer);
	PGPValidatePtr(dataLength);
	PGPValidatePref(prefRef);

	err = pgpFindPrefData(prefRef, prefIndex, &currentPrefData);
	pgpAssertNoErr(err);
	if (IsPGPError(err))
		return err;

	if (IsNull(currentPrefData))
	{
		return kPGPError_ItemNotFound;
	}

	*inBuffer	= PFLContextMemAlloc(prefRef->prefContext,
					currentPrefData->dataLength,
					kPFLMemoryFlags_Clear);

	if (IsNull(*inBuffer))
	{
		return kPGPError_OutOfMemory;
	}

	*dataLength	= currentPrefData->dataLength;
	pgpCopyMemory(currentPrefData->dataBuffer, 
		*inBuffer, 
		currentPrefData->dataLength);

	return kPGPError_NoErr;
}


PGPError PGPSetPrefData(PGPPrefRef prefRef, 
						PGPPrefIndex prefIndex, 
						PGPSize dataLength, 
						const void *outBuffer)
{
	PGPPrefData *	currentPrefData;
	PGPError		err;
	
	PGPValidatePref(prefRef);
	PGPValidateParam(dataLength > 0);
	PGPValidatePtr(outBuffer);

	err = pgpFindPrefData(prefRef, prefIndex, &currentPrefData);
	pgpAssertNoErr(err);
	if (IsPGPError(err))
		return err;

	if (IsNull(currentPrefData))
	{
		currentPrefData = 
			(PGPPrefData *) PFLContextMemAlloc(prefRef->prefContext,
								sizeof(PGPPrefData),
								kPFLMemoryFlags_Clear);

		if (IsNull(currentPrefData))
			return kPGPError_OutOfMemory;

		currentPrefData->prefIndex	= prefIndex;
		currentPrefData->dataBuffer = NULL;

		pgpAddPrefData(prefRef, currentPrefData);
	}

	currentPrefData->dataLength = dataLength;
	
	if (IsntNull(currentPrefData->dataBuffer))
		PFLContextMemFree(prefRef->prefContext, currentPrefData->dataBuffer);

	currentPrefData->dataBuffer = PFLContextMemAlloc(prefRef->prefContext,
									dataLength,
									kPFLMemoryFlags_Clear);

	if (IsNull(currentPrefData->dataBuffer))
	{
		pgpRemovePrefData(prefRef, currentPrefData);
		return kPGPError_OutOfMemory;
	}

	pgpCopyMemory(outBuffer, currentPrefData->dataBuffer, dataLength);

	return kPGPError_NoErr;
}


PGPError PGPRemovePref(PGPPrefRef prefRef, 
					   PGPPrefIndex prefIndex)
{
	PGPPrefData *	currentPrefData;
	PGPError		err;
	
	PGPValidatePref(prefRef);

	err = pgpFindPrefData(prefRef, prefIndex, &currentPrefData);
	pgpAssertNoErr(err);
	if (IsPGPError(err))
		return err;

	pgpAssert(IsntNull(currentPrefData));
	if (IsNull(currentPrefData))
		return kPGPError_BadParams;

	pgpRemovePrefData(prefRef, currentPrefData);
	return kPGPError_NoErr;
}


PGPError PGPDisposePrefData(PGPPrefRef prefRef, 
							void *dataBuffer)
{
	PGPValidatePref(prefRef);
	PGPValidatePtr(dataBuffer);

	PFLContextMemFree(prefRef->prefContext, dataBuffer);
	return kPGPError_NoErr;
}


static PGPError pgpFindPrefData(PGPPrefRef prefRef, 
								PGPPrefIndex prefIndex, 
								PGPPrefData **prefData)
{
	PGPPrefData *searchPrefData;

	if (IsntNull(prefData))
		*prefData = NULL;

	PGPValidatePtr(prefData);
	PGPValidatePref(prefRef);

	searchPrefData = prefRef->prefList;
	while (IsntNull(searchPrefData))
	{
		if (searchPrefData->prefIndex == prefIndex)
			break;

		pgpGetNextPrefData(searchPrefData, &searchPrefData);
	}

	*prefData = searchPrefData;
	return kPGPError_NoErr;
}


static PGPError pgpCalcPrefFileSize(PGPPrefRef prefRef, PGPUInt32 *size)
{
	PGPPrefData *currentPrefData;

	if (IsntNull(size))
		*size =  0;

	PGPValidatePtr(size);
	PGPValidatePref(prefRef);

	currentPrefData = prefRef->prefList;
	while (IsntNull(currentPrefData))
	{
		*size += sizeof(PGPPrefIndex) + sizeof(PGPSize) +
					currentPrefData->dataLength;
		pgpGetNextPrefData(currentPrefData, &currentPrefData);
	}

	return kPGPError_NoErr;
}


static PGPError pgpFreePref(PGPPrefRef prefRef)
{
	PGPPrefData *currentPrefData;
	PGPPrefData *tempPrefData;
	PFLContextRef context;

	PGPValidatePref(prefRef);

	currentPrefData = prefRef->prefList;
	while (IsntNull(currentPrefData))
	{
		pgpGetNextPrefData(currentPrefData, &tempPrefData);
		pgpRemovePrefData(prefRef, currentPrefData);
		currentPrefData = tempPrefData;
	}

	if (pflFileSpecIsValid(prefRef->prefFileSpec))
		PFLFreeFileSpec(prefRef->prefFileSpec);

	context = prefRef->prefContext;
	PFLContextMemFree(context, prefRef);
	return kPGPError_NoErr;
}


static PGPError pgpAddPrefData(PGPPrefRef prefRef, 
							   PGPPrefData *prefData)
{
	PGPValidatePref(prefRef);
	PGPValidatePtr(prefData);

	if (IsNull(prefRef->prefList))
		prefRef->prefList = prefData;

	if (IsntNull(prefRef->prefListEnd))
		prefRef->prefListEnd->nextPrefData = prefData;

	prefData->lastPrefData	= prefRef->prefListEnd;
	prefData->nextPrefData	= NULL;
	prefRef->prefListEnd	= prefData;
	return kPGPError_NoErr;
}


static PGPError pgpRemovePrefData(PGPPrefRef prefRef, 
								  PGPPrefData *prefData)
{
	PGPValidatePref(prefRef);
	PGPValidatePtr(prefData);

	if (IsntNull(prefData->dataBuffer))
		PFLContextMemFree(prefRef->prefContext, prefData->dataBuffer);

	if (IsntNull(prefData->nextPrefData))
		prefData->nextPrefData->lastPrefData = prefData->lastPrefData;

	if (IsntNull(prefData->lastPrefData))
		prefData->lastPrefData->nextPrefData = prefData->nextPrefData;

	if (prefRef->prefList == prefData)
		prefRef->prefList = prefData->nextPrefData;
	
	if (prefRef->prefListEnd == prefData)
		prefRef->prefListEnd = prefData->lastPrefData;

	PFLContextMemFree(prefRef->prefContext, prefData);
	return kPGPError_NoErr;
}


static PGPError pgpGetNextPrefData(const PGPPrefData *currentPrefData, 
								   PGPPrefData **nextPrefData)
{
	if (IsntNull(nextPrefData))
		*nextPrefData = NULL;

	PGPValidatePtr(nextPrefData);
	PGPValidatePtr(currentPrefData);

	*nextPrefData = currentPrefData->nextPrefData;
	return kPGPError_NoErr;
}




