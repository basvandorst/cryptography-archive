/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpShare.c,v 1.9.8.1 1998/11/12 03:11:12 heller Exp $
____________________________________________________________________________*/
#include "pgpSharePriv.h"
#include "pgpUtilities.h"
#include "pgpHash.h"
#include "pgpMemoryMgr.h"
#include "pgpMem.h"
#include "pgpDebug.h"

typedef struct PGPShare		PGPShare;

struct PGPShare
{
	PGPContextRef	context;
	PGPMemoryMgrRef	memoryMgr;
	PGPKeyID		keyID;
	PGPShareID		shareID;
	PGPUInt32		threshold;
	PGPUInt32		numShares;
	PGPUInt32		totalShares;
	PGPSize			shareDataSize;
	PGPByte *		shareData;
	PGPSize			shareHashSize;
	PGPByte *		shareHash;
};

#define PGPValidateShare(ref)	PGPValidateParam(ref != NULL)

static PGPError sCalculateShareHash(PGPShareRef share);


PGPError PGPCreateShares(PGPContextRef context, PGPKeyRef key, 
				PGPUInt32 threshold, PGPUInt32 numShares, 
				PGPShareRef *share)
{
	PGPMemoryMgrRef		memoryMgr = NULL;
	PGPShareRef			newShare = NULL;
	PGPInt32			lockBits = 0;
	PGPSize				passkeySize = 0;
	PGPSize				shareSize = 0;
	PGPSize				totalSharesSize = 0;
	PGPByte *			passkey	= NULL;
	PGPError			err	= kPGPError_NoErr;

	if (IsntNull(share))
		*share = NULL;

	PGPValidateParam(PGPContextRefIsValid(context));
	PGPValidateParam(PGPKeyRefIsValid(key));
	PGPValidateParam(threshold > 0);
	PGPValidateParam(numShares > 0);
	PGPValidateParam(threshold <= numShares);
	PGPValidatePtr(share);

	memoryMgr = PGPGetContextMemoryMgr(context);
	
	pgpAssert(PGPMemoryMgrIsValid(memoryMgr));
	if (!PGPMemoryMgrIsValid(memoryMgr))
		return kPGPError_BadParams;

	newShare = (PGPShareRef) PGPNewData(memoryMgr, sizeof(PGPShare), 
								kPGPMemoryMgrFlags_Clear);

	pgpAssert(IsntNull(newShare));
	if (IsNull(newShare))
		return kPGPError_OutOfMemory;

	newShare->context		= context;
	newShare->memoryMgr		= memoryMgr;
	newShare->threshold		= threshold;
	newShare->numShares		= numShares;
	newShare->totalShares	= numShares;
	newShare->shareDataSize	= 0;
	newShare->shareData		= NULL;
	newShare->shareHashSize = 0;
	newShare->shareHash		= NULL;

	err = PGPGetKeyIDFromKey(key, &(newShare->keyID));

	if (IsntPGPError(err))
		err = PGPGetKeyNumber(key, kPGPKeyPropLockingBits, &lockBits);
	
	if (IsntPGPError(err))
	{
		passkeySize = lockBits / 8;
		passkey = (PGPByte *)PGPNewSecureData(memoryMgr, passkeySize, 
					kPGPMemoryMgrFlags_Clear);
	}

	if (IsNull(passkey))
		err = kPGPError_OutOfMemory;

	if (IsntPGPError(err))
		err = PGPContextGetRandomBytes(context, passkey, passkeySize);

	if (IsntPGPError(err))
	{
		shareSize = kPGPShareHeaderSize + passkeySize;
		totalSharesSize = shareSize * numShares;

		newShare->shareData = (PGPByte *)PGPNewSecureData(memoryMgr,
											totalSharesSize, 
											kPGPMemoryMgrFlags_Clear);
	}

	if (IsNull(newShare->shareData))
		err = kPGPError_OutOfMemory;

	if (IsntPGPError(err))
	{
		newShare->shareDataSize = totalSharesSize;

		err = PGPSecretShareData(context, passkey, passkeySize, threshold,
				numShares, newShare->shareData);
	}

	if (IsntPGPError(err))
		err = sCalculateShareHash(newShare);

	if (IsntPGPError(err))
		err = PGPContextGetRandomBytes(context, newShare->shareID.data, 
				sizeof(newShare->shareID.data));

	if (IsntNull(passkey))
		PGPFreeData(passkey);

	if (IsPGPError(err))
	{
		PGPFreeShares(newShare);
		newShare = NULL;
	}

	*share = newShare;
	return err;		
}


PGPError pgpCreateShares(PGPContextRef context, PGPKeyID keyID, 
				PGPShareID shareID, PGPUInt32 threshold, PGPUInt32 numShares, 
				PGPUInt32 totalShares, PGPSize shareSize,
				const PGPByte *shareData, PGPShareRef *shares)
{
	PGPMemoryMgrRef		memoryMgr = NULL;
	PGPShareRef			newShare = NULL;
	PGPError			err	= kPGPError_NoErr;

	if (IsntNull(shares))
		*shares = NULL;

	PGPValidateParam(PGPContextRefIsValid(context));
	PGPValidateParam(threshold > 0);
	PGPValidateParam(numShares > 0);
	PGPValidateParam(totalShares > 0);
	PGPValidateParam(threshold <= totalShares);
	PGPValidateParam(numShares <= totalShares);
	PGPValidateParam(shareSize > 0);
	PGPValidatePtr(shareData);
	PGPValidatePtr(shares);

	memoryMgr = PGPGetContextMemoryMgr(context);
	
	pgpAssert(PGPMemoryMgrIsValid(memoryMgr));
	if (!PGPMemoryMgrIsValid(memoryMgr))
		return kPGPError_BadParams;

	newShare = (PGPShareRef) PGPNewData(memoryMgr, sizeof(PGPShare), 
								kPGPMemoryMgrFlags_Clear);

	pgpAssert(IsntNull(newShare));
	if (IsNull(newShare))
		return kPGPError_OutOfMemory;

	newShare->context		= context;
	newShare->memoryMgr		= memoryMgr;
	newShare->threshold		= threshold;
	newShare->numShares		= numShares;
	newShare->totalShares	= totalShares;
	newShare->shareDataSize	= shareSize;
	newShare->shareData		= NULL;
	newShare->shareHashSize = 0;
	newShare->shareHash		= NULL;

	newShare->shareData = (PGPByte *) PGPNewSecureData(memoryMgr, shareSize, 
							kPGPMemoryMgrFlags_Clear);

	if (IsNull(newShare->shareData))
		err = kPGPError_OutOfMemory;

	if (IsntPGPError(err))
	{
		pgpCopyMemory(&keyID, &(newShare->keyID), sizeof(PGPKeyID)); 
		pgpCopyMemory(&shareID, &(newShare->shareID), sizeof(PGPShareID)); 
		pgpCopyMemory(shareData, newShare->shareData, shareSize);
	}

	if (IsntPGPError(err))
		err = sCalculateShareHash(newShare);

	if (IsPGPError(err))
	{
		PGPFreeShares(newShare);
		newShare = NULL;
	}

	*shares = newShare;
	return err;
}


/* The passkey needs to be freed with PGPFreeData(passkey) */
PGPError PGPGetPasskeyFromShares(PGPShareRef share, PGPByte **passkey,
				PGPSize *passkeySize)
{
	PGPSize		shareSize = 0;
	PGPSize		newPasskeySize = 0;
	PGPByte *	newPasskey = NULL;
	PGPError	err = kPGPError_NoErr;

	if (passkey != NULL)
		*passkey = NULL;
	if (passkeySize != NULL)
		*passkeySize = 0;

	PGPValidateParam(PGPShareRefIsValid(share));
	PGPValidatePtr(passkey);
	PGPValidatePtr(passkeySize);

	shareSize = share->shareDataSize / share->numShares;
	newPasskeySize = shareSize - kPGPShareHeaderSize;

	newPasskey = (PGPByte *) PGPNewSecureData(share->memoryMgr, newPasskeySize, 
					kPGPMemoryMgrFlags_Clear);

	if (IsNull(newPasskey))
		err = kPGPError_OutOfMemory;

	if (IsntPGPError(err))
		err = PGPSecretReconstructData(share->context, share->shareData, 
				newPasskeySize, share->numShares, newPasskey);

	if (IsPGPError(err))
	{
		PGPFreeData(newPasskey);
		newPasskey = NULL;
		newPasskeySize = 0;
	}

	*passkey = newPasskey;
	*passkeySize = newPasskeySize;
	return err;
}


PGPError PGPSplitShares(PGPShareRef share, PGPUInt32 numShares, 
				PGPShareRef *splitShares)
{
	PGPShareRef			newShare = NULL;
	PGPUInt32			sharesLeft = 0;
	PGPSize				shareSize = 0;
	PGPSize				dataSize = 0;
	PGPError			err = kPGPError_NoErr;

	if (IsntNull(share))
		*splitShares = NULL;

	PGPValidateParam(PGPShareRefIsValid(share));
	PGPValidateParam(numShares > 0);
	PGPValidatePtr(splitShares);

	if (numShares > share->numShares)
		return kPGPClientError_NotEnoughSharesInObject;

	newShare = (PGPShareRef) PGPNewData(share->memoryMgr, sizeof(PGPShare), 
								kPGPMemoryMgrFlags_Clear);

	pgpAssert(IsntNull(newShare));
	if (IsNull(newShare))
		return kPGPError_OutOfMemory;

	newShare->context		= share->context;
	newShare->memoryMgr		= share->memoryMgr;
	newShare->threshold		= share->threshold;
	newShare->totalShares	= share->totalShares;

	pgpCopyMemory(&(share->keyID), &(newShare->keyID), sizeof(PGPKeyID));
	pgpCopyMemory(&(share->shareID), &(newShare->shareID), 
		sizeof(PGPShareID));
	
	shareSize = share->shareDataSize / share->numShares;
	dataSize = numShares * shareSize;
	sharesLeft = share->numShares - numShares;
	
	newShare->shareData	= (PGPByte *) PGPNewSecureData(share->memoryMgr, 
										dataSize, 
										kPGPMemoryMgrFlags_Clear);

	if (IsNull(newShare->shareData))
		err = kPGPError_OutOfMemory;

	if (IsntPGPError(err))
	{
		pgpCopyMemory(&(share->shareData[sharesLeft * shareSize]), 
			newShare->shareData, dataSize);

		newShare->numShares	= numShares;
		newShare->shareDataSize	= dataSize;

		share->numShares = sharesLeft;
		share->shareDataSize = sharesLeft * shareSize;
		
		if (share->shareDataSize > 0)
			err = PGPReallocData(share->memoryMgr, &(share->shareData), 
					share->shareDataSize, 0);
		else
		{
			PGPFreeData(share->shareData);
			share->shareData = NULL;
		}
	}

	PGPFreeData(share->shareHash);

	if (IsntPGPError(err))
		err = sCalculateShareHash(share);

	if (IsntPGPError(err))
		err = sCalculateShareHash(newShare);

	if (IsPGPError(err))
	{
		PGPFreeShares(newShare);
		newShare = NULL;
	}

	*splitShares = newShare;
	return err;
}


/* The share objects being combined are NOT freed by this function */
PGPError PGPCombineShares(PGPShareRef firstShare, PGPShareRef secondShare,
				PGPShareRef *combinedShares)
{
	PGPShareRef	newShare = NULL;
	PGPSize		shareSize = 0;
	PGPSize		dataSize = 0;
	PGPError	err = kPGPError_NoErr;

	if (IsntNull(combinedShares))
		*combinedShares = NULL;

	PGPValidateParam(PGPShareRefIsValid(firstShare));
	PGPValidateParam(PGPShareRefIsValid(secondShare));
	PGPValidatePtr(combinedShares);

	if (PGPCompareKeyIDs(&(firstShare->keyID), &(secondShare->keyID)) != 0)
		return kPGPClientError_DifferentSplitKeys;

	if (PGPCompareShareIDs(firstShare->shareID, secondShare->shareID) != 0)
		return kPGPClientError_DifferentSharePool;

	if (firstShare->threshold != secondShare->threshold)
		return kPGPClientError_DifferentSharePool;

	if (firstShare->totalShares != secondShare->totalShares)
		return kPGPClientError_DifferentSharePool;

	if (IsSamePGPShares(firstShare, secondShare))
		return kPGPClientError_IdenticalShares;

	newShare = (PGPShareRef) PGPNewData(firstShare->memoryMgr, 
								sizeof(PGPShare), 
								kPGPMemoryMgrFlags_Clear);

	pgpAssert(IsntNull(newShare));
	if (IsNull(newShare))
		return kPGPError_OutOfMemory;

	newShare->context		= firstShare->context;
	newShare->memoryMgr		= firstShare->memoryMgr;
	newShare->threshold		= firstShare->threshold;
	newShare->numShares		= firstShare->numShares + secondShare->numShares;
	newShare->totalShares	= firstShare->totalShares;
	newShare->shareDataSize = firstShare->shareDataSize +
								secondShare->shareDataSize;

	pgpCopyMemory(&(firstShare->keyID), &(newShare->keyID), sizeof(PGPKeyID));
	pgpCopyMemory(&(firstShare->shareID), &(newShare->shareID), 
		sizeof(PGPShareID));

	newShare->shareData	= (PGPByte *) PGPNewSecureData(firstShare->memoryMgr, 
											newShare->shareDataSize, 
											kPGPMemoryMgrFlags_Clear);

	if (IsNull(newShare->shareData))
		err = kPGPError_OutOfMemory;

	if (IsntPGPError(err))
	{
		pgpCopyMemory(firstShare->shareData, newShare->shareData, 
			firstShare->shareDataSize);

		pgpCopyMemory(secondShare->shareData, 
			&(newShare->shareData[firstShare->shareDataSize]),
			secondShare->shareDataSize);
	}

	if (IsntPGPError(err))
		err = sCalculateShareHash(newShare);

	if (IsPGPError(err))
	{
		PGPFreeShares(newShare);
		newShare = NULL;
	}

	*combinedShares = newShare;
	return err;
}


PGPError PGPFreeShares(PGPShareRef share)
{
	PGPError err = kPGPError_NoErr;

	PGPValidateParam(PGPShareRefIsValid(share));

	if (IsntNull(share->shareData))
		PGPFreeData(share->shareData);

	if (IsntNull(share->shareHash))
		PGPFreeData(share->shareHash);

	PGPFreeData(share);
	return err;
}


PGPError PGPGetKeyIDFromShares(PGPShareRef share, PGPKeyID *id)
{
	PGPError err = kPGPError_NoErr;

	PGPValidateParam(PGPShareRefIsValid(share));
	PGPValidatePtr(id);

	pgpCopyMemory(&(share->keyID), id, sizeof(PGPKeyID));

	return err;
}


PGPError PGPGetShareID(PGPShareRef share, PGPShareID *id)
{
	PGPError err = kPGPError_NoErr;

	PGPValidateParam(PGPShareRefIsValid(share));
	PGPValidatePtr(id);

	pgpCopyMemory(&(share->shareID), id, sizeof(PGPShareID));

	return err;
}


PGPBoolean IsSamePGPShares(PGPShareRef firstShare, PGPShareRef secondShare)
{
	PGPBoolean	result = FALSE;
	PGPSize		index1;
	PGPSize		index2;
	PGPSize		shareSize;

	if (!PGPShareRefIsValid(firstShare))
		return FALSE;
	if (!PGPShareRefIsValid(secondShare))
		return FALSE;

	shareSize = firstShare->shareDataSize / firstShare->numShares;
	if (shareSize != (secondShare->shareDataSize / secondShare->numShares))
		return FALSE;

	if ((firstShare->numShares == secondShare->numShares) &&
		(firstShare->shareHashSize == secondShare->shareHashSize))
	{
		result = pgpMemoryEqual(firstShare->shareHash, 
					secondShare->shareHash, firstShare->shareHashSize);
	}

	if (result == FALSE)
	{
		for (index1=0; index1<firstShare->numShares; index1++)
			for (index2=0; index2<secondShare->numShares; index2++)
				if (pgpMemoryEqual(&(firstShare->shareData[index1*shareSize]),
						&(secondShare->shareData[index2*shareSize]),
						shareSize))
				{
					result = TRUE;
					index1 = firstShare->numShares;
					index2 = secondShare->numShares;
				}
	}

	return result;
}


/* The share data needs to be freed with PGPFreeData(shareData) */
PGPError pgpGetShareData(PGPShareRef share, PGPByte **shareData, 
				PGPSize *shareDataSize)
{
	PGPError	err = kPGPError_NoErr;

	if (IsntNull(shareData))
		*shareData = NULL;

	if (IsntNull(shareDataSize))
		*shareDataSize = 0;

	PGPValidateParam(PGPShareRefIsValid(share));
	PGPValidatePtr(shareData);
	PGPValidatePtr(shareDataSize);

	*shareData = (PGPByte *) PGPNewSecureData(share->memoryMgr, 
								share->shareDataSize, 
								kPGPMemoryMgrFlags_Clear);

	if (IsNull(*shareData))
		err = kPGPError_OutOfMemory;
	else
	{
		pgpCopyMemory(share->shareData, *shareData, share->shareDataSize);
		*shareDataSize = share->shareDataSize;
	}
	
	return err;
}


/* The share hash needs to be freed with PGPFreeData(shareHash) */
PGPError pgpGetShareHash(PGPShareRef share, PGPByte **shareHash, 
				PGPSize *shareHashSize)
{
	PGPError	err = kPGPError_NoErr;

	if (IsntNull(shareHash))
		*shareHash = NULL;

	if (IsntNull(shareHashSize))
		*shareHashSize = 0;

	PGPValidateParam(PGPShareRefIsValid(share));
	PGPValidatePtr(shareHash);
	PGPValidatePtr(shareHashSize);

	*shareHash = (PGPByte *) PGPNewData(share->memoryMgr, 
								share->shareHashSize, 
								kPGPMemoryMgrFlags_Clear);

	if (IsNull(*shareHash))
		err = kPGPError_OutOfMemory;
	else
	{
		pgpCopyMemory(share->shareHash, *shareHash, share->shareHashSize);
		*shareHashSize = share->shareHashSize;
	}
	
	return err;
}


PGPUInt32 PGPGetShareThreshold(PGPShareRef share)
{
	if (PGPShareRefIsValid(share))
		return share->threshold;
	else
		return 0;
}


/* This is the number of shares contained in the share object */
PGPUInt32 PGPGetNumberOfShares(PGPShareRef share)
{
	if (PGPShareRefIsValid(share))
		return share->numShares;
	else
		return 0;
}


/* The share object may contain less than the total number of shares */
PGPUInt32 PGPGetTotalNumberOfShares(PGPShareRef share)
{
	if (PGPShareRefIsValid(share))
		return share->totalShares;
	else
		return 0;
}


PGPInt32 PGPCompareShareIDs(PGPShareID firstID, PGPShareID secondID)
{
	PGPInt32 index;
	PGPInt32 result = 0;

	for (index=sizeof(firstID.data)-1; index>=0; index--)
	{
		if (firstID.data[index] < secondID.data[index])
		{
			result = -1;
			index = -1;
		}
		else if (firstID.data[index] > secondID.data[index])
		{
			result = 1;
			index = -1;
		}
	}

	return result;
}


static PGPError sCalculateShareHash(PGPShareRef share)
{
	PGPHashContextRef	hash = NULL;
	PGPByte				nullByte = 0;
	PGPError			err = kPGPError_NoErr;

	err = PGPNewHashContext(share->memoryMgr, kPGPHashAlgorithm_SHA, &hash);

	if (IsntPGPError(err))
	{
		PGPGetHashSize(hash, &(share->shareHashSize));

		share->shareHash = (PGPByte *) PGPNewData(share->memoryMgr,
											share->shareHashSize,
											kPGPMemoryMgrFlags_Clear);
	}

	if (IsNull(share->shareHash))
		err = kPGPError_OutOfMemory;

	if (IsntPGPError(err))
	{
		if (IsntNull(share->shareData))
			PGPContinueHash(hash, share->shareData, share->shareDataSize);
		else
			PGPContinueHash(hash, &nullByte, sizeof(nullByte));

		PGPFinalizeHash(hash, share->shareHash);
	}

	if (IsntNull(hash))
		PGPFreeHashContext(hash);

	return err;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


