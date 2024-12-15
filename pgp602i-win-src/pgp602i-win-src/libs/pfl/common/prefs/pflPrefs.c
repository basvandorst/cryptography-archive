/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pflPrefs.c,v 1.19.14.1 1998/11/12 03:18:35 heller Exp $
____________________________________________________________________________*/

#include <stdio.h>
#include <string.h>

#include "pgpMemoryMgr.h"
#include "pflPrefs.h"
#include "pgpMem.h"
#include "pgpDebug.h"
#include "pgpPFLErrors.h"
#include "pgpFileUtilities.h"
#include "pgpEndianConversion.h"

typedef struct PGPPrefData	PGPPrefData;
typedef struct PGPPref		PGPPref;

#define kPGPPrefsEndian		kPGPBigEndian
#define kPGPPrefsMagic		0x155A1326

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
	PGPMemoryMgrRef	memoryMgr;
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

static PGPError sIORead(PGPIORef fileIO, PGPSize dataSize, 
					PGPBoolean crossPlatform, void *data);

static PGPError sIOWrite(PGPIORef fileIO, PGPSize dataSize, 
					PGPBoolean crossPlatform, const void *data);


PGPError PGPOpenPrefFile(PFLFileSpecRef prefFileSpec, 
						 const PGPPrefDefault *defaults,
						 PGPUInt32 numDefaults,
						 PGPPrefRef *prefRef)
{
	PGPMemoryMgrRef	memoryMgr	= NULL;
	PGPIORef		prefIO;
	PGPPrefIndex	newIndex;
	PGPUInt32		index;
	PGPUInt32		magic;
	PGPSize			dataLength;
	void *			newData		= NULL;
	PGPPrefData *	newPrefData = NULL;
	PGPPrefData *	oldPrefData = NULL;
	PGPSize			bytesRead;
	PGPError		err;

	if (IsntNull(prefRef))
		*prefRef = NULL;

	PGPValidatePtr(prefRef);
	PFLValidateFileSpec(prefFileSpec);

	memoryMgr = PFLGetFileSpecMemoryMgr( prefFileSpec );
	
	pgpAssert(PGPMemoryMgrIsValid(memoryMgr));
	if (!PGPMemoryMgrIsValid(memoryMgr))
		return kPGPError_BadParams;

	*prefRef = (PGPPrefRef) PGPNewData(memoryMgr, 
								sizeof(PGPPref), 
								kPGPMemoryMgrFlags_Clear);

	pgpAssert(IsntNull(*prefRef));
	if (IsNull(*prefRef))
		return kPGPError_OutOfMemory;

	(*prefRef)->memoryMgr		= memoryMgr;
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
	
	if ((err != kPGPError_FileNotFound) && (err != kPGPError_FileOpFailed))
	{
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
		
		if (!PGPIOIsAtEOF(prefIO))
		{
			err = sIORead(prefIO, sizeof(PGPUInt32), TRUE, &magic);
			if (magic != kPGPPrefsMagic)
				err = PGPIOSetPos(prefIO, 0);
		}
		
		while (!PGPIOIsAtEOF(prefIO) && IsntPGPError(err))
		{
			newData = NULL;
			newPrefData = NULL;
			
			if (magic == kPGPPrefsMagic)
				err = sIORead(prefIO, sizeof(PGPPrefIndex), TRUE, &newIndex);
			else
				err = PGPIORead(prefIO, sizeof(PGPPrefIndex), &newIndex, 
				&bytesRead);
			
			if (IsPGPError(err))
				break;
			
			if (magic == kPGPPrefsMagic)
				err = sIORead(prefIO, sizeof(PGPSize), TRUE, &dataLength);
			else
				err = PGPIORead(prefIO, sizeof(PGPSize), &dataLength, 
						&bytesRead);
			
			if (IsPGPError(err))
				break;
			
			newData = PGPNewData(memoryMgr, 
				dataLength, 
				kPGPMemoryMgrFlags_Clear);
			
			if (IsNull(newData))
			{
				err = kPGPError_OutOfMemory;
				break;
			}
			
			if (magic == kPGPPrefsMagic)
				err = sIORead(prefIO, dataLength, FALSE, newData);
			else
				err = PGPIORead(prefIO, dataLength, newData, &bytesRead);
			
			if (IsPGPError(err))
				break;
			
			newPrefData = (PGPPrefData *) PGPNewData(memoryMgr,
											sizeof(PGPPrefData),
											kPGPMemoryMgrFlags_Clear);
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
				PGPFreeData( newData);
			
			err = kPGPError_NoErr;
		}
		else if (IsPGPError(err))
		{
			pgpAssertNoErr(err);
			pgpFreePref(*prefRef);
			*prefRef = NULL;
		}
		
		PGPFreeIO(prefIO);
	}
	else
		err = kPGPError_NoErr;

	if ((numDefaults > 0) && IsntNull(defaults) && IsntPGPError(err))
	{
		for (index=0; index<numDefaults; index++)
		{
			err = pgpFindPrefData(*prefRef, defaults[index].index, 
					&oldPrefData);

			pgpAssertNoErr(err);
			if (IsPGPError(err))
				break;

			if (IsNull(oldPrefData))
			{
				newPrefData = (PGPPrefData *) PGPNewData(memoryMgr,
												sizeof(PGPPrefData),
												kPGPMemoryMgrFlags_Clear);
				
				if (IsNull(newPrefData))
				{
					err = kPGPError_OutOfMemory;
					break;
				}
				
				newPrefData->prefIndex = defaults[index].index;
				
				switch (defaults[index].type)
				{
				case kPGPPrefType_Boolean:
					{
					PGPBoolean	temp;
					
					newPrefData->dataLength = sizeof(PGPBoolean);
					newPrefData->dataBuffer = PGPNewData(memoryMgr,
												newPrefData->dataLength,
												kPGPMemoryMgrFlags_Clear);

					temp = (PGPBoolean) (size_t) defaults[index].data;
					pgpCopyMemory( &temp,
						newPrefData->dataBuffer,
						newPrefData->dataLength);
					break;
					}
				case kPGPPrefType_Number:
					{
					PGPByte data32[sizeof(PGPUInt32)];

					newPrefData->dataLength = sizeof(PGPUInt32);
					newPrefData->dataBuffer = PGPNewData(memoryMgr,
												newPrefData->dataLength,
												kPGPMemoryMgrFlags_Clear);

					PGPUInt32ToEndian((PGPUInt32) defaults[index].data, 
						kPGPPrefsEndian, data32);

					pgpCopyMemory(data32,
						newPrefData->dataBuffer,
						newPrefData->dataLength);
					break;
					}
					
				case kPGPPrefType_String:
					newPrefData->dataLength = 
						strlen((char *) defaults[index].data) + 1;
					newPrefData->dataBuffer = PGPNewData(memoryMgr,
												newPrefData->dataLength,
												kPGPMemoryMgrFlags_Clear);

					pgpCopyMemory(defaults[index].data,
						newPrefData->dataBuffer, 
						newPrefData->dataLength);
					break;
					
				case kPGPPrefType_Byte:
					newPrefData->dataLength = defaults[index].size;
					newPrefData->dataBuffer = PGPNewData(memoryMgr,
												newPrefData->dataLength,
												kPGPMemoryMgrFlags_Clear);

					pgpCopyMemory(defaults[index].data,
						newPrefData->dataBuffer,
						newPrefData->dataLength);
					break;
				}
				
				pgpAddPrefData(*prefRef, newPrefData);
			}
		}
	}

	if ((magic != kPGPPrefsMagic) && IsntPGPError(err))
		PGPSavePrefFile(*prefRef);

	return err;
}


PGPError PGPSavePrefFile(PGPPrefRef prefRef)
{
	PGPPrefData *	currentPrefData;
	PGPIORef		prefIO;
	PGPUInt32		prefFileSize;
	PGPUInt32		magic;
	PGPError		err;
	PGPBoolean		fileExists;
	
	PGPValidatePref(prefRef);

	pgpCalcPrefFileSize(prefRef, &prefFileSize);

	/* Create the file if it does not exist */
	err = PFLFileSpecExists( prefRef->prefFileSpec, &fileExists );
	if( IsntPGPError( err ) && ! fileExists )
	{
		err = PFLFileSpecCreate(prefRef->prefFileSpec);
	}
	
	pgpAssertNoErr(err);
	if (IsPGPError(err))
		return err;

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

	magic = kPGPPrefsMagic;
	err = sIOWrite(prefIO, sizeof(PGPUInt32), TRUE, &magic);

	pgpAssertNoErr(err);
	if (IsPGPError(err))
	{
		PGPFreeIO(prefIO);
		return err;
	}

	currentPrefData = prefRef->prefList;

	while (IsntNull(currentPrefData))
	{
		err = sIOWrite(prefIO, sizeof(PGPPrefIndex), TRUE,
				&(currentPrefData->prefIndex));

		pgpAssertNoErr(err);
		if (IsPGPError(err))
			break;

		err = sIOWrite(prefIO, sizeof(PGPSize), TRUE,
				&(currentPrefData->dataLength));

		pgpAssertNoErr(err);
		if (IsPGPError(err))
			break;

		err = sIOWrite(prefIO, currentPrefData->dataLength, FALSE,
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
		return kPGPError_PrefNotFound;
	}

	*inBuffer	= PGPNewData(prefRef->memoryMgr,
					currentPrefData->dataLength,
					kPGPMemoryMgrFlags_Clear);

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
	PGPValidatePtr(outBuffer);

	err = pgpFindPrefData(prefRef, prefIndex, &currentPrefData);
	pgpAssertNoErr(err);
	if (IsPGPError(err))
		return err;

	if (IsNull(currentPrefData))
	{
		currentPrefData = 
			(PGPPrefData *) PGPNewData(prefRef->memoryMgr,
								sizeof(PGPPrefData),
								kPGPMemoryMgrFlags_Clear);

		if (IsNull(currentPrefData))
			return kPGPError_OutOfMemory;

		currentPrefData->prefIndex	= prefIndex;
		currentPrefData->dataBuffer = NULL;

		pgpAddPrefData(prefRef, currentPrefData);
	}

	currentPrefData->dataLength = dataLength;
	
	if (IsntNull(currentPrefData->dataBuffer))
		PGPFreeData( currentPrefData->dataBuffer);

	currentPrefData->dataBuffer = PGPNewData(prefRef->memoryMgr,
									dataLength,
									kPGPMemoryMgrFlags_Clear);

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

	PGPFreeData( dataBuffer);
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

	*size = sizeof(PGPUInt32);

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
	PGPMemoryMgrRef memoryMgr;

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

	memoryMgr = prefRef->memoryMgr;
	PGPFreeData( prefRef);
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
		PGPFreeData( prefData->dataBuffer);

	if (IsntNull(prefData->nextPrefData))
		prefData->nextPrefData->lastPrefData = prefData->lastPrefData;

	if (IsntNull(prefData->lastPrefData))
		prefData->lastPrefData->nextPrefData = prefData->nextPrefData;

	if (prefRef->prefList == prefData)
		prefRef->prefList = prefData->nextPrefData;
	
	if (prefRef->prefListEnd == prefData)
		prefRef->prefListEnd = prefData->lastPrefData;

	PGPFreeData( prefData);
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


static PGPError sIORead(PGPIORef fileIO, PGPSize dataSize, 
					PGPBoolean crossPlatform, void *data)
{
	PGPByte		tempData16[sizeof(PGPUInt16)];
	PGPByte		tempData32[sizeof(PGPUInt32)];
	PGPUInt32	data32;
	PGPUInt16	data16;
	PGPSize		bytesRead;
	PGPError	err = kPGPError_NoErr;

	PGPValidateIO(fileIO);
	PGPValidateParam(dataSize > 0);
	PGPValidatePtr(data);

	if (crossPlatform)
	{
		if (dataSize == sizeof(PGPUInt16))
			err = PGPIORead(fileIO, dataSize, tempData16, &bytesRead);
		else
			err = PGPIORead(fileIO, dataSize, tempData32, &bytesRead);

		if (IsntPGPError(err))
		{
			if (dataSize == sizeof(PGPUInt16))
			{
				data16 = PGPEndianToUInt16(kPGPPrefsEndian, tempData16);
				pgpCopyMemory(&data16, data, sizeof(PGPUInt16));
			}
			else
			{
				data32 = PGPEndianToUInt32(kPGPPrefsEndian, tempData32);
				pgpCopyMemory(&data32, data, sizeof(PGPUInt32));
			}
		}
	}
	else
		err = PGPIORead(fileIO, dataSize, data, &bytesRead);

	return err;
}


static PGPError sIOWrite(PGPIORef fileIO, PGPSize dataSize, 
					PGPBoolean crossPlatform, const void *data)
{
	PGPByte		tempData16[sizeof(PGPUInt16)];
	PGPByte		tempData32[sizeof(PGPUInt32)];
	PGPUInt32	data32;
	PGPUInt16	data16;
	PGPError	err = kPGPError_NoErr;

	PGPValidateIO(fileIO);
	PGPValidateParam(dataSize > 0);
	PGPValidatePtr(data);

	if (crossPlatform)
	{
		if (dataSize == sizeof(PGPUInt16))
		{
			pgpCopyMemory(data, &data16, sizeof(PGPUInt16));
			PGPUInt16ToEndian(data16, kPGPPrefsEndian, tempData16);
			err = PGPIOWrite(fileIO, dataSize, tempData16);
		}
		else
		{
			pgpCopyMemory(data, &data32, sizeof(PGPUInt32));
			PGPUInt32ToEndian(data32, kPGPPrefsEndian, tempData32);
			err = PGPIOWrite(fileIO, dataSize, tempData32);
		}
	}
	else
		err = PGPIOWrite(fileIO, dataSize, data);

	return err;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/



