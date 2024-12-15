/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpShareFile.c,v 1.7.8.1 1998/11/12 03:11:13 heller Exp $
____________________________________________________________________________*/
#include "pgpShareFile.h"
#include "pgpSharePriv.h"
#include "pgpMemoryMgr.h"
#include "pgpFileUtilities.h"
#include "pgpOptionList.h"
#include "pgpEncode.h"
#include "pgpMem.h"
#include "pgpDebug.h"
#include "pgpEndianConversion.h"

#include <string.h>

typedef struct PGPShareFile		PGPShareFile;

struct PGPShareFile
{
	PGPMemoryMgrRef	memoryMgr;
	PFLFileSpecRef	shareFileSpec;

	char *			userID;
	void *			fingerprint;
	PGPSize			fingerprintSize;
	PGPKeyID		keyID;
	PGPKeyID		sharedKeyID;
	PGPShareID		shareID;
	PGPUInt32		threshold;
	PGPUInt32		numShares;
	PGPUInt32		totalShares;
	PGPSize			decryptedShareSize;
	PGPSize			shareDataSize;
	PGPByte	*		shareData;
	PGPSize			shareHashSize;
	PGPByte *		shareHash;
};

#define PGPValidateShareFile(ref)	PGPValidateParam(ref != NULL)

#define kPGPShareFileEndian		kPGPBigEndian
#define kPGPShareFileMagic		0x15607AED

static void sCalcShareFileSize(PGPShareFileRef shareFileRef, 
				PGPUInt32 *fileSize);

static PGPError sIORead(PGPIORef fileIO, PGPSize dataSize, 
					PGPBoolean crossPlatform, void *data);

static PGPError sIOWrite(PGPIORef fileIO, PGPSize dataSize, 
					PGPBoolean crossPlatform, const void *data);


PGPError PGPNewShareFile(PFLFileSpecRef shareFileSpec, 
				PGPShareFileRef *shareFileRef)
{
	PGPMemoryMgrRef	memoryMgr	= NULL;
	PGPError		err			= kPGPError_NoErr;

	if (IsntNull(shareFileRef))
		*shareFileRef = NULL;

	PGPValidatePtr(shareFileRef);
	PFLValidateFileSpec(shareFileSpec);

	memoryMgr = PFLGetFileSpecMemoryMgr(shareFileSpec);
	
	pgpAssert(PGPMemoryMgrIsValid(memoryMgr));
	if (!PGPMemoryMgrIsValid(memoryMgr))
		return kPGPError_BadParams;

	*shareFileRef = (PGPShareFileRef) PGPNewData(memoryMgr, 
										sizeof(PGPShareFile), 
										kPGPMemoryMgrFlags_Clear);

	pgpAssert(IsntNull(*shareFileRef));
	if (IsNull(*shareFileRef))
		return kPGPError_OutOfMemory;

	(*shareFileRef)->memoryMgr			= memoryMgr;
	(*shareFileRef)->shareFileSpec		= NULL;
	(*shareFileRef)->userID				= NULL;
	(*shareFileRef)->fingerprint		= NULL;
	(*shareFileRef)->fingerprintSize	= 0;
	(*shareFileRef)->threshold			= 0;
	(*shareFileRef)->numShares			= 0;
	(*shareFileRef)->totalShares		= 0;
	(*shareFileRef)->decryptedShareSize	= 0;
	(*shareFileRef)->shareDataSize		= 0;
	(*shareFileRef)->shareData			= NULL;
	(*shareFileRef)->shareHashSize		= 0;
	(*shareFileRef)->shareHash			= NULL;

	err = PFLCopyFileSpec(shareFileSpec, &((*shareFileRef)->shareFileSpec));
	if (IsPGPError(err))
	{
		PGPFreeShareFile(*shareFileRef);
		*shareFileRef = NULL;
		return err;
	}

	return err;		
}


PGPError PGPFreeShareFile(PGPShareFileRef shareFileRef)
{
	PGPError	err = kPGPError_NoErr;

	PGPValidateShareFile(shareFileRef);

	if (pflFileSpecIsValid(shareFileRef->shareFileSpec))
		PFLFreeFileSpec(shareFileRef->shareFileSpec);

	if (shareFileRef->userID != NULL)
		PGPFreeData(shareFileRef->userID);

	if (shareFileRef->fingerprint != NULL)
		PGPFreeData(shareFileRef->fingerprint);

	if (shareFileRef->shareData != NULL)
		PGPFreeData(shareFileRef->shareData);

	if (shareFileRef->shareHash != NULL)
		PGPFreeData(shareFileRef->shareHash);

	PGPFreeData(shareFileRef);
	return err;
}


/* The share object needs to be freed with PGPFreeShares(shares) */
PGPError PGPCopySharesFromFile(PGPContextRef context, 
				PGPShareFileRef shareFileRef, PGPOptionListRef optionList,
				PGPShareRef *shares)
{
	PGPError	err	= kPGPError_NoErr;
	PGPShareRef	newShare = NULL;
	PGPByte *	shareData = NULL;
	PGPSize		actualSize = 0;

	if (IsntNull(shares))
		*shares = NULL;

	PGPValidateParam(PGPContextRefIsValid(context));
	PGPValidateShareFile(shareFileRef);
	PGPValidateParam(PGPOptionListRefIsValid(optionList));
	PGPValidatePtr(shares);

	if (shareFileRef->shareDataSize == 0)
		return kPGPError_NoErr;

	shareData = (PGPByte *) PGPNewSecureData(shareFileRef->memoryMgr,
								shareFileRef->decryptedShareSize,
								kPGPMemoryMgrFlags_Clear);

	if (IsNull(shareData))
		err = kPGPError_OutOfMemory;

	if (IsntPGPError(err))
		err = PGPDecode(context,
				PGPOInputBuffer(context, 
					shareFileRef->shareData, 
					shareFileRef->shareDataSize),
				PGPOOutputBuffer(context, 
					shareData, 
					shareFileRef->decryptedShareSize,
					&actualSize),
				optionList,
				PGPOLastOption(context));

	if (IsntPGPError(err))
	{
		pgpAssert(actualSize == shareFileRef->decryptedShareSize);
		
		if (actualSize == 0)
			err = kPGPError_BadPassphrase;
		else if (actualSize != shareFileRef->decryptedShareSize)
			err = kPGPError_CorruptData;
	}

	if (IsntPGPError(err))
		pgpCreateShares(context, shareFileRef->sharedKeyID,
			shareFileRef->shareID, shareFileRef->threshold, 
			shareFileRef->numShares, shareFileRef->totalShares, 
			shareFileRef->decryptedShareSize, shareData, &newShare);

	if (IsPGPError(err))
	{
		if (IsntNull(newShare))
		{
			PGPFreeShares(newShare);
			newShare = NULL;
		}
	}

	if (IsntNull(shareData))
		PGPFreeData(shareData);

	*shares = newShare;
	return err;
}


PGPError PGPCopySharesToFile(PGPContextRef context,
				PGPShareFileRef shareFileRef, PGPOptionListRef optionList,
				PGPShareRef shares)
{
	PGPByte *	shareData = NULL;
	PGPSize		dataSize = 0;
	PGPError	err	= kPGPError_NoErr;

	PGPValidateParam(PGPContextRefIsValid(context));
	PGPValidateShareFile(shareFileRef);
	PGPValidateParam(PGPOptionListRefIsValid(optionList));
	PGPValidateParam(PGPShareRefIsValid(shares));

	if (shareFileRef->shareData != NULL)
	{
		PGPFreeData(shareFileRef->shareData);
		shareFileRef->shareData = NULL;
	}

	err = pgpGetShareData(shares, &shareData, &dataSize);

	if (IsntPGPError(err))
		err = PGPEncode(context,
				PGPOInputBuffer(context, shareData, dataSize),
				PGPOAllocatedOutputBuffer(context, 
					&(shareFileRef->shareData), 
					INT_MAX, 
					&(shareFileRef->shareDataSize)),
				optionList,
				PGPOLastOption(context));

	if (IsntPGPError(err))
	{
		if( IsntNull( shareFileRef->shareHash ) )
		{
			PGPFreeData( shareFileRef->shareHash );
			shareFileRef->shareHash = NULL;
		}
		
		err = pgpGetShareHash(shares, &(shareFileRef->shareHash),
				&(shareFileRef->shareHashSize));
	}

	if (IsPGPError(err))
	{
		if (shareFileRef->shareData != NULL)
		{
			PGPFreeData(shareFileRef->shareData);
			shareFileRef->shareData = NULL;
		}

		if (shareFileRef->shareHash != NULL)
		{
			PGPFreeData(shareFileRef->shareHash);
			shareFileRef->shareHash = NULL;
		}

		shareFileRef->decryptedShareSize = 0;
		shareFileRef->shareDataSize = 0;
		shareFileRef->shareHashSize = 0;
		shareFileRef->threshold = 0;
		shareFileRef->numShares = 0;
		shareFileRef->totalShares = 0;
	}
	else
	{
		shareFileRef->decryptedShareSize = dataSize;
		shareFileRef->threshold = PGPGetShareThreshold(shares);
		shareFileRef->numShares = PGPGetNumberOfShares(shares);
		shareFileRef->totalShares = PGPGetTotalNumberOfShares(shares);
		
		PGPGetKeyIDFromShares(shares, &(shareFileRef->sharedKeyID));
		PGPGetShareID(shares, &(shareFileRef->shareID));
	}

	PGPFreeData(shareData);
	return err;
}


PGPError PGPGetShareFileUserID(PGPShareFileRef shareFileRef,
				PGPSize bufferSize, char *userID, PGPSize *fullSize)
{
	PGPError	err = kPGPError_NoErr;
	PGPSize		userIDSize;

	if (IsntNull(userID))
		*userID = '\0';
	if (IsntNull(fullSize))
		*fullSize = 0;

	PGPValidateShareFile(shareFileRef);
	PGPValidateParam(bufferSize > 0);
	PGPValidatePtr(userID);

	if (IsNull(shareFileRef->userID))
		return kPGPError_NoErr;

	if (bufferSize < strlen(shareFileRef->userID) + 1)
		userIDSize = bufferSize;
	else
		userIDSize = strlen(shareFileRef->userID) + 1;

	pgpCopyMemory(shareFileRef->userID, userID, userIDSize);
	userID[userIDSize-1] = 0;

	if (fullSize != NULL)
		*fullSize = strlen(shareFileRef->userID) + 1;

	if (bufferSize < strlen(shareFileRef->userID) + 1)
		err = kPGPError_BufferTooSmall;

	return err;
}


PGPError PGPSetShareFileUserID(PGPShareFileRef shareFileRef,
				const char *userID)
{
	PGPError	err = kPGPError_NoErr;

	PGPValidateShareFile(shareFileRef);
	PGPValidatePtr(userID);

	if (shareFileRef->userID != NULL)
		PGPFreeData(shareFileRef->userID);

	shareFileRef->userID = (char *) PGPNewData(shareFileRef->memoryMgr,
										strlen(userID)+1,
										kPGPMemoryMgrFlags_Clear);

	if (shareFileRef->userID == NULL)
		return kPGPError_OutOfMemory;

	pgpCopyMemory(userID, shareFileRef->userID, strlen(userID)+1);
	return err;
}


PGPError PGPOpenShareFile(PFLFileSpecRef shareFileSpec, 
				PGPShareFileRef *shareFileRef)
{
	PGPError	err = kPGPError_NoErr;
	PGPIORef	fileIO;
	PGPSize		dataSize;
	PGPUInt32	magic;

	err = PGPNewShareFile(shareFileSpec, shareFileRef);
	if (IsPGPError(err))
		return err;

	err = PGPOpenFileSpec(	(*shareFileRef)->shareFileSpec, 
							kPFLFileOpenFlags_ReadOnly, 
							(PGPFileIORef *) &fileIO);
	if (IsPGPError(err))
		return err;

	err = sIORead(fileIO, sizeof(PGPUInt32), TRUE, &magic);
	if (magic != kPGPShareFileMagic)
		err = kPGPError_CorruptData;

	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPSize), TRUE, &dataSize);

	if (IsntPGPError(err) && (dataSize > 0))
	{
		(*shareFileRef)->userID = (char *) 
								PGPNewData((*shareFileRef)->memoryMgr,
									dataSize,
									kPGPMemoryMgrFlags_Clear);

		if ((*shareFileRef)->userID != NULL)
			err = sIORead(fileIO, dataSize, FALSE, 
					(*shareFileRef)->userID);
		else
			err = kPGPError_OutOfMemory;
	}

	if (IsntPGPError(err))
	{
		err = sIORead(fileIO, sizeof(PGPSize), TRUE, &dataSize);
		(*shareFileRef)->fingerprintSize = dataSize;
	}

	if (IsntPGPError(err) && (dataSize > 0))
	{
		(*shareFileRef)->fingerprint = (char *) 
								PGPNewData((*shareFileRef)->memoryMgr,
									dataSize,
									kPGPMemoryMgrFlags_Clear);

		if ((*shareFileRef)->fingerprint != NULL)
			err = sIORead(fileIO, dataSize, FALSE, 
					(*shareFileRef)->fingerprint);
		else
			err = kPGPError_OutOfMemory;
	}
	
	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPKeyID), FALSE,
				&((*shareFileRef)->keyID));

	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPKeyID), FALSE,
				&((*shareFileRef)->sharedKeyID));

	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPShareID), FALSE,
				&((*shareFileRef)->shareID));

	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPUInt32), TRUE,
				&((*shareFileRef)->threshold));

	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPUInt32), TRUE,
				&((*shareFileRef)->numShares));

	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPUInt32), TRUE,
				&((*shareFileRef)->totalShares));

	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPSize), TRUE,
				&((*shareFileRef)->decryptedShareSize));

	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPSize), TRUE,
				&((*shareFileRef)->shareDataSize));

	if (IsntPGPError(err) && ((*shareFileRef)->shareDataSize > 0) &&
		((*shareFileRef)->numShares > 0))
	{
		(*shareFileRef)->shareData = (PGPByte *) PGPNewData(
										(*shareFileRef)->memoryMgr, 
										(*shareFileRef)->shareDataSize, 
										kPGPMemoryMgrFlags_Clear);
	
		if ((*shareFileRef)->shareData != NULL)
			err = sIORead(fileIO, (*shareFileRef)->shareDataSize, FALSE,
					(*shareFileRef)->shareData);
		else
			err = kPGPError_OutOfMemory;
	}

	if (IsntPGPError(err))
		err = sIORead(fileIO, sizeof(PGPSize), TRUE,
				&((*shareFileRef)->shareHashSize));

	if (IsntPGPError(err) && ((*shareFileRef)->shareHashSize > 0))
	{
		(*shareFileRef)->shareHash = (PGPByte *) PGPNewData(
										(*shareFileRef)->memoryMgr, 
										(*shareFileRef)->shareHashSize, 
										kPGPMemoryMgrFlags_Clear);
	
		if ((*shareFileRef)->shareHash != NULL)
			err = sIORead(fileIO, (*shareFileRef)->shareHashSize, FALSE,
					(*shareFileRef)->shareHash);
		else
			err = kPGPError_OutOfMemory;
	}

	PGPFreeIO(fileIO);
	if (IsPGPError(err))
	{
		PGPFreeShareFile(*shareFileRef);
		*shareFileRef = NULL;
		return err;
	}

	return err;
}


PGPError PGPSaveShareFile(PGPShareFileRef shareFileRef)
{
	PGPError	err = kPGPError_NoErr;
	PGPIORef	fileIO;
	PGPUInt32	fileSize;
	PGPSize		dataSize;
	PGPUInt32	magic;

	PGPValidateShareFile(shareFileRef);

	sCalcShareFileSize(shareFileRef, &fileSize);
	err = PGPSetFileSize(shareFileRef->shareFileSpec, fileSize);

	if (IsPGPError(err))
		return err;

	err = PGPOpenFileSpec(	shareFileRef->shareFileSpec, 
							kPFLFileOpenFlags_ReadWrite, 
							(PGPFileIORef *) &fileIO);
	if (IsPGPError(err))
		return err;

	magic = kPGPShareFileMagic;
	err = sIOWrite(fileIO, sizeof(PGPUInt32), TRUE, &magic);

	if (IsntNull(shareFileRef->userID))
		dataSize = strlen(shareFileRef->userID) + 1;
	else
		dataSize = 0;

	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPSize), TRUE, &dataSize);

	if (IsntPGPError(err) && (dataSize > 0))
		err = sIOWrite(fileIO, dataSize, FALSE,
				shareFileRef->userID);
	
	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPSize), TRUE, 
				&(shareFileRef->fingerprintSize));

	if (IsntPGPError(err) && (shareFileRef->fingerprintSize > 0))
		err = sIOWrite(fileIO, shareFileRef->fingerprintSize, FALSE,
				shareFileRef->fingerprint);
	
	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPKeyID), FALSE,
				&(shareFileRef->keyID));

	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPKeyID), FALSE,
				&(shareFileRef->sharedKeyID));

	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPShareID), FALSE,
				&(shareFileRef->shareID));

	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPUInt32), TRUE,
				&(shareFileRef->threshold));

	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPUInt32), TRUE,
				&(shareFileRef->numShares));

	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPUInt32), TRUE,
				&(shareFileRef->totalShares));

	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPSize), TRUE,
				&(shareFileRef->decryptedShareSize));

	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPSize), TRUE,
				&(shareFileRef->shareDataSize));

	if (IsntPGPError(err) && (shareFileRef->shareDataSize > 0))
		err = sIOWrite(fileIO, shareFileRef->shareDataSize, FALSE,
				shareFileRef->shareData);

	if (IsntPGPError(err))
		err = sIOWrite(fileIO, sizeof(PGPSize), TRUE,
				&(shareFileRef->shareHashSize));

	if (IsntPGPError(err) && (shareFileRef->shareHashSize > 0))
		err = sIOWrite(fileIO, shareFileRef->shareHashSize, FALSE,
				shareFileRef->shareHash);

	PGPFreeIO(fileIO);
	return err;
}


PGPError PGPGetShareFileSpec(PGPShareFileRef shareFileRef,
				PFLFileSpecRef *shareFileSpec)
{
	PGPError err = kPGPError_NoErr;

	if (IsntNull(shareFileSpec))
		*shareFileSpec = NULL;

	PGPValidateShareFile(shareFileRef);
	PGPValidatePtr(shareFileSpec);

	err = PFLCopyFileSpec(shareFileRef->shareFileSpec, shareFileSpec);
	if (IsPGPError(err))
		*shareFileSpec = NULL;

	return err;
}


PGPError PGPGetShareFileShareID(PGPShareFileRef shareFileRef, 
				PGPShareID *id)
{
	PGPError err = kPGPError_NoErr;

	PGPValidateParam(PGPShareFileRefIsValid(shareFileRef));
	PGPValidatePtr(id);

	pgpCopyMemory(&(shareFileRef->shareID), id, sizeof(PGPShareID));

	return err;
}


PGPError PGPGetShareFileOwnerKeyID(PGPShareFileRef shareFileRef, 
				PGPKeyID *id)
{
	PGPError err = kPGPError_NoErr;

	PGPValidateParam(PGPShareFileRefIsValid(shareFileRef));
	PGPValidatePtr(id);

	pgpCopyMemory(&(shareFileRef->keyID), id, sizeof(PGPKeyID));

	return err;
}


PGPError PGPSetShareFileOwnerKeyID(PGPShareFileRef shareFileRef, 
				const PGPKeyID id)
{
	PGPError err = kPGPError_NoErr;

	PGPValidateParam(PGPShareFileRefIsValid(shareFileRef));

	pgpCopyMemory(&id, &(shareFileRef->keyID), sizeof(PGPKeyID));

	return err;
}


PGPError PGPGetShareFileOwnerFingerprint(PGPShareFileRef shareFileRef, 
				PGPSize bufferSize, void *fingerprint, PGPSize *fullSize)
{
	PGPError	err = kPGPError_NoErr;
	PGPSize		fingerprintSize;

	if (IsntNull(fullSize))
		*fullSize = 0;

	PGPValidateShareFile(shareFileRef);
	PGPValidateParam(bufferSize > 0);
	PGPValidatePtr(fingerprint);

	if (IsNull(shareFileRef->fingerprint))
		return kPGPError_NoErr;

	if (bufferSize < shareFileRef->fingerprintSize)
		fingerprintSize = bufferSize;
	else
		fingerprintSize = shareFileRef->fingerprintSize;

	pgpCopyMemory(shareFileRef->fingerprint, fingerprint, fingerprintSize);

	if (fullSize != NULL)
		*fullSize = shareFileRef->fingerprintSize;

	if (bufferSize < shareFileRef->fingerprintSize)
		err = kPGPError_BufferTooSmall;

	return err;
}


PGPError PGPSetShareFileOwnerFingerprint(PGPShareFileRef shareFileRef,
				PGPSize bufferSize, const void *fingerprint)
{
	PGPError	err = kPGPError_NoErr;

	PGPValidateShareFile(shareFileRef);
	PGPValidateParam(bufferSize > 0);
	PGPValidatePtr(fingerprint);

	if (shareFileRef->fingerprint != NULL)
		PGPFreeData(shareFileRef->fingerprint);

	shareFileRef->fingerprint = (char *) PGPNewData(shareFileRef->memoryMgr,
											bufferSize,
											kPGPMemoryMgrFlags_Clear);

	if (shareFileRef->fingerprint == NULL)
		return kPGPError_OutOfMemory;

	pgpCopyMemory(fingerprint, shareFileRef->fingerprint, bufferSize);
	shareFileRef->fingerprintSize = bufferSize;
	return err;
}


PGPError PGPGetShareFileSharedKeyID(PGPShareFileRef shareFileRef, 
				PGPKeyID *id)
{
	PGPError err = kPGPError_NoErr;

	PGPValidateParam(PGPShareFileRefIsValid(shareFileRef));
	PGPValidatePtr(id);

	pgpCopyMemory(&(shareFileRef->sharedKeyID), id, sizeof(PGPKeyID));

	return err;
}


PGPUInt32 PGPGetShareThresholdInFile(PGPShareFileRef shareFileRef)
{
	pgpAssert(PGPShareFileRefIsValid(shareFileRef));

	if (PGPShareFileRefIsValid(shareFileRef))
		return shareFileRef->threshold;
	else
		return 0;
}


PGPUInt32 PGPGetNumSharesInFile(PGPShareFileRef shareFileRef)
{
	pgpAssert(PGPShareFileRefIsValid(shareFileRef));
	
	if (PGPShareFileRefIsValid(shareFileRef))
		return shareFileRef->numShares;
	else
		return 0;
}


PGPBoolean IsSamePGPSharesInFiles(PGPShareFileRef firstFile, 
				PGPShareFileRef secondFile)
{
	PGPBoolean result = FALSE;

	if (!PGPShareFileRefIsValid(firstFile))
		return FALSE;
	if (!PGPShareFileRefIsValid(secondFile))
		return FALSE;

	if (firstFile->shareHashSize == secondFile->shareHashSize)
	{
		if (pgpMemoryEqual(firstFile->shareHash, secondFile->shareHash,
				firstFile->shareHashSize))
		{
			result = TRUE;
		}
	}

	return result;
}


static void sCalcShareFileSize(PGPShareFileRef shareFileRef, 
				PGPUInt32 *fileSize)
{
	pgpAssert(shareFileRef != NULL);
	pgpAssert(fileSize != NULL);

	*fileSize = sizeof(PGPUInt32)*4 + sizeof(PGPSize)*4 + 
					sizeof(PGPKeyID)*2 + shareFileRef->shareDataSize +
					sizeof(PGPShareID) + shareFileRef->shareHashSize;

	if (shareFileRef->userID != NULL)
		*fileSize += strlen(shareFileRef->userID) + 1;

	if (shareFileRef->fingerprint != NULL)
		*fileSize += shareFileRef->fingerprintSize;

	return;
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
				data16 = PGPEndianToUInt16(kPGPShareFileEndian, tempData16);
				pgpCopyMemory(&data16, data, sizeof(PGPUInt16));
			}
			else
			{
				data32 = PGPEndianToUInt32(kPGPShareFileEndian, tempData32);
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
			PGPUInt16ToEndian(data16, kPGPShareFileEndian, tempData16);
			err = PGPIOWrite(fileIO, dataSize, tempData16);
		}
		else
		{
			pgpCopyMemory(data, &data32, sizeof(PGPUInt32));
			PGPUInt32ToEndian(data32, kPGPShareFileEndian, tempData32);
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

