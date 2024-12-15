//////////////////////////////////////////////////////////////////////////////
// PGPdiskPublicKeyUtils.cpp
//
// Utility functions for working with public keys and PGPdisks.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskPublicKeyUtils.cpp,v 1.5 1999/02/26 04:09:59 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"
#include <new.h>

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include "PlacementNew.h"
#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"

#include "PGPclx.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpAdminPrefs.h"
#include "pgpErrors.h"
#include "pgpPublicKey.h"
#include "pgpUtilities.h"

#include "GlobalPGPContext.h"
#include "PGPdiskHighLevelUtils.h"
#include "PGPdiskLowLevelUtils.h"
#include "PGPdiskPublicKeyUtils.h"
#include "SecureMemory.h"
#include "UtilityFunctions.h"

#include "File.h"


////////////
// Constants
////////////

static LPCSTR kPGPdiskPassesWipedId = "WIPED";


///////////////////////////////
// Public key utility functions
///////////////////////////////

// DoesPGPdiskHavePublicKey returns TRUE if the specified PGPdisk has a
// public key passphrase, FALSE otherwise.

PGPBoolean 
DoesPGPdiskHavePublicKey(LPCSTR path)
{
	DualErr					derr;
	File					diskFile;
	PGPBoolean				gotItemList, hasPublicKey;
	PGPdiskFileHeaderItem	*curItem, *itemList;

	gotItemList = hasPublicKey = FALSE;

	pgpAssertStrValid(path);

	// Open the PGPdisk.
	derr = diskFile.Open(path, kOF_MustExist | kOF_ReadOnly);

	// Get list of headers.
	if (derr.IsntError())
	{
		derr = GetHeaderItemList(&diskFile, &itemList);
		gotItemList = derr.IsntError();
	}

	// Look for a public key header.
	if (derr.IsntError())
	{
		curItem = itemList;

		while (IsntNull(curItem))
		{
			if (IsPublicKeyHeader(curItem->hdr))
			{
				hasPublicKey = TRUE;
				break;
			}

			curItem = curItem->next;
		}
	}

	if (gotItemList)
		FreeHeaderItemList(itemList);

	if (diskFile.Opened())
		diskFile.Close();

	if (derr.IsError())
		return FALSE;
	else
		return hasPublicKey;
}

// IsKeyIDMemberOfRecipList returns TRUE if the given key ID corresponds to a
// key in the given recipient list, FALSE otherwise.

DualErr 
IsKeyIDMemberOfRecipList(
	PGPKeyID			goalKeyID, 
	PGPRecipientSpec	*recipList, 
	PGPUInt32			size, 
	PGPBoolean			*isMember)
{
	DualErr		derr;
	PGPUInt32	i;

	pgpAssertAddrValid(recipList, PGPRecipientSpec);
	pgpAssertAddrValid(isMember, PGPBoolean);

	(* isMember) = FALSE;

	// For every recipient spec...
	for (i = 0; i < size; i++)
	{
		PGPKeyID keyID;

		// ... if its key ID equals goal key ID, goal key is a member.
		switch (recipList[i].type)
		{
		case kPGPRecipientSpecType_Key:

			pgpAssert(PGPKeyRefIsValid(recipList[i].u.key));
			derr = PGPGetKeyIDFromKey(recipList[i].u.key, &keyID);

			if (derr.IsntError())
			{
				if (PGPCompareKeyIDs(&keyID, &goalKeyID) == 0)
					(* isMember) = TRUE;
			}

			break;

		case kPGPRecipientSpecType_KeyID:

			if (PGPCompareKeyIDs(&recipList[i].u.id.keyID, &goalKeyID) == 0)
				(* isMember) = TRUE;

			break;

		default:
			break;
		}

		if ((* isMember) || derr.IsError())
			break;
	}

	return derr;
}

// AddRecipientSpecToList adds a single PGPRecipientSpec to the given array.

DualErr 
AddRecipientSpecToList(PGPRecipientSpec **recipList, PGPUInt32 size)
{
	DualErr				derr;
	PGPRecipientSpec	*newList;

	pgpAssertAddrValid(recipList, PGPRecipientSpec *);

	try
	{
		newList = new PGPRecipientSpec[size + 1];
	}
	catch (CMemoryException *ex)
	{
		derr = DualErr(kPGDMinorError_OutOfMemory);
		ex->Delete();
	}

	if (derr.IsntError())
	{
		pgpClearMemory((void *) &newList[size], sizeof(PGPRecipientSpec));

		if (size > 0)
		{
			pgpCopyMemory((* recipList), newList, 
				sizeof(PGPRecipientSpec) * size);

			delete[] (* recipList);
		}

		(* recipList) = newList;
	}

	return derr;
}

// FreeRecipientSpecList frees a list of recipient specs and the keys within.

void 
FreeRecipientSpecList(PGPRecipientSpec *recipList, PGPUInt32 size)
{
	if (size > 0)
	{
		pgpAssertAddrValid(recipList, PGPRecipientSpec);
		delete[] recipList;
	}
}

// GetPGPdiskRecipKeys returns a list of the public keys associated with a
// PGPdisk.

DualErr 
GetPGPdiskRecipKeys(
	LPCSTR				path, 
	PGPKeySetRef		allKeys, 
	PGPRecipientSpec	**pDiskRecipKeys, 
	PGPUInt32			*pNumRecipKeys)
{
	File					diskFile;
	DualErr					derr;
	PGPBoolean				gotAnyRecips, gotItemList;
	PGPdiskFileHeaderItem	*curItem, *itemList;
	PGPdiskPublicKeyHeader	*pubKeyHdr;
	PGPRecipientSpec		*diskRecipKeys;
	PGPUInt32				numRecipKeys;

	gotAnyRecips = gotItemList = FALSE;

	pgpAssertStrValid(path);
	pgpAssert(PGPKeySetRefIsValid(allKeys));
	pgpAssertAddrValid(pDiskRecipKeys, PGPRecipientSpec *);
	pgpAssertAddrValid(pNumRecipKeys, PGPUInt32);

	(* pNumRecipKeys) = numRecipKeys = 0;

	// Open the PGPdisk.
	derr = diskFile.Open(path, kOF_MustExist | kOF_ReadOnly);

	// Get list of headers.
	if (derr.IsntError())
	{
		derr = GetHeaderItemList(&diskFile, &itemList);
		gotItemList = derr.IsntError();
	}

	if (derr.IsntError())
	{
		curItem = itemList;

		// Extract key from each public key header.
		while (derr.IsntError() && IsntNull(curItem))
		{
			PGPByte				*exportedKeyID;
			PGPKeyID			keyID;
			PGPKeyRef			curKey;
			PGPRecipientSpec	*curRecipSpec;

			if (IsPublicKeyHeader(curItem->hdr))
			{
				pubKeyHdr = (PGPdiskPublicKeyHeader *) curItem->hdr;

				exportedKeyID = (PGPByte *) pubKeyHdr + 
					pubKeyHdr->keyIDOffset;

				// Create new recip spec.
				derr = AddRecipientSpecToList(&diskRecipKeys, numRecipKeys);

				if (!gotAnyRecips)
					gotAnyRecips = derr.IsntError();

				if (derr.IsntError())
				{
					// Get pointer to current recip spec.
					curRecipSpec = &diskRecipKeys[numRecipKeys++];

					// Read in key ID.
					derr = PGPImportKeyID(exportedKeyID, &keyID);
				}

				// If actual key found, add key ref to spec, else add ID.
				if (derr.IsntError())
				{
					DualErr keyInRingErr;

					keyInRingErr = PGPGetKeyByKeyID(allKeys, &keyID, 
						pubKeyHdr->algorithm, &curKey);

					if (keyInRingErr.IsntError())
					{
						curRecipSpec->type = kPGPRecipientSpecType_Key;
						curRecipSpec->locked = pubKeyHdr->locked;

						curRecipSpec->u.key = curKey;
					}
					else
					{
						curRecipSpec->type = kPGPRecipientSpecType_KeyID;
						curRecipSpec->locked = pubKeyHdr->locked;

						curRecipSpec->u.id.keyID = keyID;
						curRecipSpec->u.id.algorithm = pubKeyHdr->algorithm;
					}

				#if !PGP_BUSINESS_SECURITY
					// No locked keys in personal build.
					curRecipSpec->locked = FALSE;
				#endif // !PGP_BUSINESS_SECURITY

				}
			}

			curItem = curItem->next;
		}
	}

	if (derr.IsntError())
	{
		(* pDiskRecipKeys) = diskRecipKeys;
		(* pNumRecipKeys) = numRecipKeys;
	}

	// Cleanup on error.
	if (derr.IsError())
	{
		if (gotAnyRecips)
			FreeRecipientSpecList(diskRecipKeys, numRecipKeys);
	}

	if (gotItemList)
		FreeHeaderItemList(itemList);

	if (diskFile.Opened())
		diskFile.Close();

	return derr;
}

// UpdatePGPdiskPublicKeys updates the public keys headers on a PGPdisk.

DualErr 
UpdatePGPdiskPublicKeys(
	LPCSTR				path, 
	SecureString		*masterPassphrase, 
	PGPKeySetRef		allKeys, 
	PGPRecipientSpec	*newRecipKeys, 
	PGPUInt32			numRecipKeys, 
	PGPBoolean			newKeysReadOnly)
{
	File					diskFile;
	CASTKey					*decryptedKey;
	DualErr					derr;
	PGPBoolean				gotItemList	= FALSE;
	PGPdiskFileHeader		*mainHeader;
	PGPdiskFileHeaderItem	*curItem, *itemList;
	PGPdiskPublicKeyHeader	*pubKeyHdr;
	PGPKeyRef				curKey;
	PGPUInt32				i;

	SecureMemory	smDecryptedKey(sizeof(CASTKey));

	pgpAssertStrValid(path);
	pgpAssertAddrValid(masterPassphrase, SecureString);
	pgpAssert(PGPKeySetRefIsValid(allKeys));
	pgpAssertAddrValid(newRecipKeys, PGPRecipientSpec);

	// Did we get our locked memory?
	derr = smDecryptedKey.mInitErr;

	if (derr.IsntError())
	{
		// Set the locked memory pointer.
		decryptedKey = (CASTKey *) smDecryptedKey.GetPtr();

		// Open the PGPdisk.
		derr = diskFile.Open(path, kOF_MustExist);
	}

	// Get list of headers.
	if (derr.IsntError())
	{
		derr = GetHeaderItemList(&diskFile, &itemList);
		gotItemList = derr.IsntError();
	}

	if (derr.IsntError())
	{
		// Get main header pointer.
		mainHeader = (PGPdiskFileHeader *) itemList->hdr;

		// Decrypt the master key
		derr = DecryptPassphraseKey(&mainHeader->masterPassphrase, 
			&mainHeader->salt, masterPassphrase, decryptedKey);
	}

	// Add keys not in the PGPdisk to the PGPdisk.
	if (derr.IsntError())
	{
		// For each recipient spec...
		for (i = 0; i < numRecipKeys; i++)
		{
			// ... if it is a key ref...
			if ((newRecipKeys[i].type) == kPGPRecipientSpecType_Key)
			{
				curKey = newRecipKeys[i].u.key;
				pgpAssert(PGPKeyRefIsValid(curKey));

				// ... and if not in header list...
				derr = FindPublicKeyInHeaderList(&diskFile, curKey, 
					itemList, &curItem);

				if (derr.IsntError() && IsNull(curItem))
				{
					// Create a new header for it.
					derr = CreatePublicKeyHeader(curKey, decryptedKey, 
						FALSE, newKeysReadOnly, &pubKeyHdr);

					// ...insert the new header after the main header.
					if (derr.IsntError())
					{
						derr = InsertPGPdiskHeaderInList(itemList, 
							(PGPdiskFileHeaderInfo *) pubKeyHdr);
					}
				}
			}
		}
	}

	// Remove keys not in the key list from the PGPdisk.
	if (derr.IsntError())
	{
		curItem = itemList;

		// For every header...
		while (derr.IsntError() && IsntNull(curItem))
		{
			PGPBoolean	isMember;
			PGPByte		*exportedKeyID;
			PGPKeyID	keyID;

			// ...if it's a public key header...
			if (IsPublicKeyHeader(curItem->hdr))
			{
				pubKeyHdr = (PGPdiskPublicKeyHeader *) curItem->hdr;

				exportedKeyID = (PGPByte *) pubKeyHdr + 
					pubKeyHdr->keyIDOffset;

				// Read in key ID.
				derr = PGPImportKeyID(exportedKeyID, &keyID);

				// ... and isn't in the list, remove it from PGPdisk.
				if (derr.IsntError())
				{
					derr = IsKeyIDMemberOfRecipList(keyID, newRecipKeys, 
						numRecipKeys, &isMember);
				}

				if (derr.IsntError())
				{
					if (!isMember)
						RemovePGPdiskHeaderFromList(&curItem);
				}
			}

			if (IsntNull(curItem))
				curItem = curItem->next;
		}
	}

	// Write out the updated headers to the PGPdisk.
	if (derr.IsntError())
	{
		derr = UpdateHeaderItemList(&diskFile, itemList);
	}

	if (gotItemList)
		FreeHeaderItemList(itemList);

	if (diskFile.Opened())
		diskFile.Close();

	return derr;
}

// FindPublicKeyInHeaderList finds the header item in the given list, if any, 
// that corresponds to the given public key.

DualErr 
FindPublicKeyInHeaderList(
	File					*diskFile, 
	PGPKeyRef				curKey, 
	PGPdiskFileHeaderItem	*itemList, 
	PGPdiskFileHeaderItem	**keyItem)
{
	DualErr					derr;
	PGPdiskFileHeaderItem	*curItem;
	PGPdiskPublicKeyHeader	*pubKeyHdr;
	PGPKeyID				goalKeyID;

	pgpAssertAddrValid(diskFile, File);
	pgpAssert(diskFile->Opened());
	pgpAssert(PGPKeyRefIsValid(curKey));
	pgpAssertAddrValid(itemList, PGPdiskFileHeaderItem);
	pgpAssertAddrValid(keyItem, PGPdiskFileHeaderItem *);

	(* keyItem) = NULL;

	// Get ID of key to look for.
	derr = PGPGetKeyIDFromKey(curKey, &goalKeyID);

	curItem = itemList;

	while (derr.IsntError() && IsntNull(curItem))
	{
		PGPByte			*exportedKeyID;
		PGPKeyID		keyID;

		if (IsPublicKeyHeader(curItem->hdr))
		{
			pubKeyHdr = (PGPdiskPublicKeyHeader *) curItem->hdr;
			exportedKeyID = (PGPByte *) pubKeyHdr + pubKeyHdr->keyIDOffset;

			// Read in key ID.
			derr = PGPImportKeyID(exportedKeyID, &keyID);

			// Do they match?
			if (PGPCompareKeyIDs(&keyID, &goalKeyID) == 0)
			{
				(* keyItem) = curItem;
				break;
			}
		}

		curItem = curItem->next;
	}

	return derr;
}

// GetHeaderForPublicKey returns a copy of the PGPdisk header that
// corresponds to the given public key.

DualErr 
GetHeaderForPublicKey(
	LPCSTR					path, 
	PGPKeyRef				curKey, 
	PGPdiskPublicKeyHeader	**pubKeyHdr)
{
	File					diskFile;
	DualErr					derr;
	PGPBoolean				gotItemList;
	PGPdiskFileHeaderItem	*itemList, *keyItem;

	pgpAssertStrValid(path);
	pgpAssert(PGPKeyRefIsValid(curKey));

	// Open the PGPdisk.
	derr = diskFile.Open(path, kOF_MustExist | kOF_ReadOnly);

	// Get list of headers.
	if (derr.IsntError())
	{
		derr = GetHeaderItemList(&diskFile, &itemList);
		gotItemList = derr.IsntError();
	}

	// Look for associated header.
	if (derr.IsntError())
	{
		derr = FindPublicKeyInHeaderList(&diskFile, curKey, itemList, 
			&keyItem);
	}

	// Copy the header.
	if (derr.IsntError())
	{
		derr = CopyHeader(keyItem->hdr, (PGPdiskFileHeaderInfo **) pubKeyHdr);
	}

	if (gotItemList)
		FreeHeaderItemList(itemList);

	if (diskFile.Opened())
		diskFile.Close();

	return derr;
}

// CreatePublicKeyHeader creates and returns a new PGPdisk public key header
// using the specified information.

DualErr 
CreatePublicKeyHeader(
	PGPKeyRef				pubKey, 
	const CASTKey			*decryptedKey, 
	PGPBoolean				locked, 
	PGPBoolean				readOnly, 
	PGPdiskPublicKeyHeader	**outPubKeyHdr)
{
	DualErr					derr;
	CheckBytes				checkBytes;
	PGPBoolean				gotEncryptedKey	= FALSE;
	PGPByte					*data, exportedKeyID[kPGPMaxExportedKeyIDSize];
	PGPKeyID				keyID;
	PGPInt32				alg;
	PGPPublicKeyAlgorithm	algorithm;
	PGPdiskPublicKeyHeader	*pubKeyHdr;
	PGPSize					exportedKeyIDSize;
	PGPUInt32				encryptedKeySize, headerSize;
	void					*encryptedKey	= NULL;

	encryptedKeySize = 0;

	pgpAssert(PGPKeyRefIsValid(pubKey));
	pgpAssertAddrValid(decryptedKey, CASTKey);
	pgpAssertAddrValid(outPubKeyHdr, PGPdiskPublicKeyHeader *);

	// Encrypt decypted key using the private key of the public key.
	derr = EncryptPublicKey(decryptedKey, pubKey, &encryptedKey, 
		&encryptedKeySize, &checkBytes);

	gotEncryptedKey = derr.IsntError();

	// Get key ID.
	if (derr.IsntError())
	{
		derr = PGPGetKeyIDFromKey(pubKey, &keyID);
	}

	// Get exported key ID.
	if (derr.IsntError())
	{
		derr = PGPExportKeyID(&keyID, exportedKeyID, &exportedKeyIDSize);
	}

	// Get key algorithm.
	if (derr.IsntError())
	{
		derr = PGPGetKeyNumber(pubKey, kPGPKeyPropAlgID, &alg);
	}

	// Allocate header.
	if (derr.IsntError())
	{		
		algorithm = (PGPPublicKeyAlgorithm) alg;

		headerSize = sizeof(PGPdiskPublicKeyHeader) + exportedKeyIDSize + 
			encryptedKeySize;

		try
		{
			pubKeyHdr = (PGPdiskPublicKeyHeader *) new PGPUInt8[headerSize];
		}
		catch (CMemoryException *ex)
		{
			derr = DualErr(kPGDMinorError_OutOfMemory);
			ex->Delete();
		}
	}

	// Initialize header fields.
	if (derr.IsntError())
	{
		pgpClearMemory(pubKeyHdr, sizeof(PGPdiskPublicKeyHeader));

		strncpy(pubKeyHdr->headerInfo.headerMagic, kPGPdiskHeaderMagic, 4);

		strncpy(pubKeyHdr->headerInfo.headerType, 
			kPGPdiskPublicKeyHeaderType, 4);

		pubKeyHdr->headerInfo.headerSize = headerSize;

		pubKeyHdr->majorVersion	= kPGPdiskPublicKeyHeaderMajorVersion;
		pubKeyHdr->minorVersion	= kPGPdiskPublicKeyHeaderMinorVersion;
									
		pubKeyHdr->readOnly	= readOnly;
		pubKeyHdr->locked	= locked;

		pubKeyHdr->keyIDSize		= exportedKeyIDSize;
		pubKeyHdr->encryptedKeySize	= encryptedKeySize;
		pubKeyHdr->checkBytes		= checkBytes;
		pubKeyHdr->algorithm		= algorithm;
		
		data = (PGPByte *) pubKeyHdr + sizeof(PGPdiskPublicKeyHeader);
		pgpCopyMemory(exportedKeyID, data, exportedKeyIDSize);

		pubKeyHdr->keyIDOffset = data - (PGPByte *) pubKeyHdr;
		
		data += exportedKeyIDSize;
		pgpCopyMemory(encryptedKey, data, encryptedKeySize);

		pubKeyHdr->encryptedKeyOffset = data - (PGPByte *) pubKeyHdr;

		(* outPubKeyHdr) = pubKeyHdr;
	}

	if (gotEncryptedKey)
		delete[] (PGPUInt8 *) encryptedKey;

	return derr;
}

// ArePGPdiskPassesWiped returns TRUE if this PGPdisk has had its passes
// wiped, FALSE otherwise.

PGPBoolean 
ArePGPdiskPassesWiped(LPCSTR path)
{
	DualErr				derr;
	File				diskFile;
	PGPBoolean			arePassesWiped, masterInUse, readHeader;
	PGPdiskFileHeader	*mainHeader;

	arePassesWiped = readHeader = FALSE;

	pgpAssertStrValid(path);

	// Read main header.
	derr = ReadPGPdiskFileMainHeader(path, &mainHeader);
	readHeader = derr.IsntError();

	// If master passphrase not in use and ID matches, then we're wiped.
	if (derr.IsntError())
	{
		masterInUse = (mainHeader->masterPassphrase.inUse ? TRUE : FALSE);
		
		if (!masterInUse)
		{
			arePassesWiped = (strcmp((LPCSTR) 
				&mainHeader->masterPassphrase.checkBytes.theBytes, 
				kPGPdiskPassesWipedId) == 0);
		}
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) mainHeader);

	if (derr.IsError())
		return FALSE;
	else
		return arePassesWiped;
}

// ShowReadOnlyListOfPublicKeys displays the recipient dialog containing a
// read-only list of all the public keys on the specified PGPdisk.

DualErr 
ShowReadOnlyListOfPublicKeys(LPCSTR path)
{
	DualErr				derr;
	PGPBoolean			gotAllKeys, gotExistingKeys, gotNewKeys;
	PGPBoolean			gotOptionsList;
	PGPKeySetRef		allKeys, newKeySet;
	PGPOptionListRef	recipientOptions;
	PGPRecipientSpec	*existingKeys;
	PGPUInt32			numExistKeys;

	gotAllKeys = gotExistingKeys = gotNewKeys = gotOptionsList = FALSE;

	// Open default key rings.
	derr = PGPOpenDefaultKeyRings(GetGlobalPGPContext(), 0, &allKeys);
	gotAllKeys = derr.IsntError();

	// Retrieve already existing keys from the PGPdisk.
	if (derr.IsntError())
	{
		derr = GetPGPdiskRecipKeys(path, allKeys, &existingKeys, 
			&numExistKeys);

		gotExistingKeys = derr.IsntError();
	}

	// Make each key read-only in the dialog.
	if (derr.IsntError())
	{
		PGPUInt32 i;

		for (i = 0; i < numExistKeys; i++)
		{
			existingKeys[i].locked = TRUE;
		}
	}

	// Build option list for the recipient dialog.
	if (derr.IsntError())
	{
		PGPOptionListRef defRecipOption;

		defRecipOption = PGPOUIDefaultRecipients(GetGlobalPGPContext(), 
			numExistKeys, existingKeys);

		derr = PGPBuildOptionList(GetGlobalPGPContext(), &recipientOptions, 
			defRecipOption, PGPOLastOption(GetGlobalPGPContext()));

		gotOptionsList = derr.IsntError();
	}

	// Show the recipient dialog.
	if (derr.IsntError())
	{
		derr = PGPRecipientDialog(GetGlobalPGPContext(), allKeys, TRUE, 
			&newKeySet, recipientOptions, 
			PGPOLastOption(GetGlobalPGPContext()));

		gotNewKeys = derr.IsntError();
	}

	if (gotNewKeys)
		PGPFreeKeySet(newKeySet);

	if (gotOptionsList)
		PGPFreeOptionList(recipientOptions);

	if (gotExistingKeys)
		FreeRecipientSpecList(existingKeys, numExistKeys);

	if (gotAllKeys)
		PGPFreeKeySet(allKeys);

	return derr;
}

// WipeMasterAndAlternatePasses wipes all the master and alternate
// passphrases on a PGPdisk, but leaves the public key passphrases alone.

DualErr 
WipeMasterAndAlternatePasses(LPCSTR path)
{
	DualErr				derr;
	File				diskFile;
	PGPBoolean			readHeader	= FALSE;
	PGPdiskFileHeader	*mainHeader;

	pgpAssertStrValid(path);

	// Read main header.
	derr = ReadPGPdiskFileMainHeader(path, &mainHeader);
	readHeader = derr.IsntError();

	// Wipe master and alternate passes.
	if (derr.IsntError())
	{
		PGPUInt32 i;

		pgpClearMemory(&mainHeader->masterPassphrase, 
			sizeof(mainHeader->masterPassphrase));

		pgpClearMemory(mainHeader->otherPassphrases, 
			sizeof(mainHeader->otherPassphrases));

		mainHeader->masterPassphrase.inUse = FALSE;

		for (i = 0; i < kMaxAlternatePassphrases; i++)
		{
			mainHeader->otherPassphrases[i].inUse = FALSE;
		}

		strcpy((LPSTR) &mainHeader->masterPassphrase.checkBytes.theBytes, 
			kPGPdiskPassesWipedId);

		derr = WritePGPdiskFileMainHeader(path, mainHeader);
	}

	if (readHeader)
		FreePGPdiskFileHeader((PGPdiskFileHeaderInfo *) mainHeader);

	return derr;
}


///////////////////////
// ADK utility routines
///////////////////////

// IsTherePGPdiskADK returns TRUE if a PGPdisk ADK exists, FALSE otherwise.

DualErr 
IsTherePGPdiskADK(PGPBoolean *isThereADK)
{
	DualErr derr;

	pgpAssertAddrValid(isThereADK, PGPBoolean);
	(* isThereADK) = FALSE;

#if PGP_BUSINESS_SECURITY

	PGPBoolean	openedAdminPrefs	= FALSE;
	PGPPrefRef	adminPrefsRef;

	// Open admin prefs file.
	if (derr.IsntError())
	{		
		derr = PGPclOpenAdminPrefs(PGPGetContextMemoryMgr(
			GetGlobalPGPContext()), &adminPrefsRef, TRUE);

		openedAdminPrefs = derr.IsntError();
	}

	// Uses ADK?
	if (derr.IsntError())
	{
		derr = PGPGetPrefBoolean(adminPrefsRef, kPGPPrefUsePGPdiskADK, 
			isThereADK);
	}

	if (openedAdminPrefs)
		PGPclCloseAdminPrefs(adminPrefsRef, FALSE);

#endif	// PGP_BUSINESS_SECURITY

	return derr;
}

// GetADKKey gets the PGPdisk ADK key.

DualErr 
GetADKKey(PGPKeySetRef allKeys, PGPKeyRef *ADKKey)
{
	DualErr					derr;
	PGPBoolean				gotIDData, openedAdminPrefs;
	PGPKeyID				keyID;
	PGPPrefRef				adminPrefsRef;
	PGPPublicKeyAlgorithm	algorithm;
	PGPSize					dataLength;
	PGPUInt32				tempAlg;
	void					*data;

	gotIDData = openedAdminPrefs = FALSE;

	pgpAssert(PGP_BUSINESS_SECURITY);

	pgpAssert(PGPKeySetRefIsValid(allKeys));
	pgpAssertAddrValid(ADKKey, PGPKeyRef);

	// Open admin prefs file.
	derr = PGPclOpenAdminPrefs(PGPGetContextMemoryMgr(
		GetGlobalPGPContext()), &adminPrefsRef, TRUE);

	openedAdminPrefs = derr.IsntError();

	// Get ADK algorithm.
	if (derr.IsntError())
	{
		derr = PGPGetPrefNumber(adminPrefsRef, 
			kPGPPrefPGPdiskADKPublicKeyAlgorithm, &tempAlg);
	}

	// Get ADK algorithm.
	if (derr.IsntError())
	{			
		algorithm = (PGPPublicKeyAlgorithm) tempAlg;
		
		derr = PGPGetPrefData(adminPrefsRef, kPGPPrefPGPdiskADKKeyID, 
			&dataLength, &data);

		gotIDData = derr.IsntError();
	}

	// Get ADK key ID.
	if (derr.IsntError())
	{
		derr = PGPImportKeyID(data, &keyID);
	}

	// Get ADK key.
	if (derr.IsntError())
	{
		derr = PGPGetKeyByKeyID(allKeys, &keyID, algorithm, ADKKey);
	}

	if (gotIDData)
		PGPFreeData(data);

	if (openedAdminPrefs)
		PGPclCloseAdminPrefs(adminPrefsRef, FALSE);

	return derr;
}

// GetADKUserID gets the user ID of the current PGPdisk ADK.

DualErr 
GetADKUserID(PGPKeySetRef allKeys, LPSTR userID, PGPUInt32 sizeUserID)
{
	DualErr		derr;
	PGPKeyRef	ADKKey;

	pgpAssert(PGP_BUSINESS_SECURITY);

	pgpAssert(PGPKeySetRefIsValid(allKeys));
	pgpAssertAddrValid(userID, char);

	// Get ADK key.
	derr = GetADKKey(allKeys, &ADKKey);

	// Get user ID string.
	if (derr.IsntError())
	{
		PGPSize usedLength;

		derr = PGPGetPrimaryUserIDNameBuffer(ADKKey, sizeUserID, userID, 
			&usedLength);
	}

	return derr;
}

// AddADKToPGPdisk adds an existing ADK to a PGPdisk.

DualErr 
AddADKToPGPdisk(LPCSTR path, SecureString *masterPassphrase)
{
	CASTKey					*decryptedKey;
	DualErr					derr;
	File					diskFile;
	PGPBoolean				gotAllKeys, gotItemList;
	PGPdiskFileHeaderItem	*itemList;
	PGPdiskPublicKeyHeader	*pubKeyHdr;
	PGPdiskFileHeader		*mainHeader;
	PGPKeyRef				ADKKey;
	PGPKeySetRef			allKeys;

	SecureMemory	smDecryptedKey(sizeof(CASTKey));

	gotAllKeys = gotItemList = FALSE;
	pgpAssertStrValid(path);

	// Did we get our locked memory?
	derr = smDecryptedKey.mInitErr;

	// Set the locked memory pointer.
	if (derr.IsntError())
	{
		decryptedKey = (CASTKey *) smDecryptedKey.GetPtr();
	}

	// Load default key ring.
	if (derr.IsntError())
	{
		derr = PGPOpenDefaultKeyRings(GetGlobalPGPContext(), 0, &allKeys);
		gotAllKeys = derr.IsntError();
	}

	// Get ADK key.
	if (derr.IsntError())
	{
		derr = GetADKKey(allKeys, &ADKKey);
	}

	// Open the PGPdisk.
	if (derr.IsntError())
	{
		derr = diskFile.Open(path, kOF_MustExist);
	}

	// Get list of headers.
	if (derr.IsntError())
	{
		derr = GetHeaderItemList(&diskFile, &itemList);
		gotItemList = derr.IsntError();
	}

	if (derr.IsntError())
	{
		// Get main header pointer.
		mainHeader = (PGPdiskFileHeader *) itemList->hdr;

		// Decrypt the master key
		derr = DecryptPassphraseKey(&mainHeader->masterPassphrase, 
			&mainHeader->salt, masterPassphrase, decryptedKey);
	}

	// Create a new header for it.
	if (derr.IsntError())
	{
		derr = CreatePublicKeyHeader(ADKKey, decryptedKey, TRUE, TRUE, 
			&pubKeyHdr);
	}

	// Insert the new header after the main header.
	if (derr.IsntError())
	{
		derr = InsertPGPdiskHeaderInList(itemList, 
			(PGPdiskFileHeaderInfo *) pubKeyHdr);
	}

	// Write out the updated headers to the PGPdisk.
	if (derr.IsntError())
	{
		derr = UpdateHeaderItemList(&diskFile, itemList);
	}

	if (gotItemList)
		FreeHeaderItemList(itemList);

	if (gotAllKeys)
		PGPFreeKeySet(allKeys);

	if (diskFile.Opened())
		diskFile.Close();

	return derr;
}


////////////////////////////////////////////
// Public key encryption/decryption routines
////////////////////////////////////////////

// DecryptPublicKey decrypts data using a public/private key pair.

DualErr 
DecryptPublicKey(
	PGPKeyRef			pubKey,
	SecureString		*passphrase,
	const void			*encryptedData,
	PGPSize				encryptedDataSize,
	const CheckBytes	*checkBytes,
	CASTKey				*decryptedKey)
{
	CheckBytes				decryptedCheckBytes;
	DualErr					derr;
	ExpandedCASTKey			*expandedKey;
	PGPBoolean				allocedDataBuf, gotPrivContext;
	PGPByte					*data, fingerPrint[128];
	PGPPrivateKeyContextRef	privContext;
	PGPSize					decryptedDataSize, fingerPrintSize;
	PGPSize					maxDecryptedBufferSize, maxEncryptedBufferSize;
	PGPSize					maxSignatureSize;
	PGPUInt8				*secureBuf;

	SecureMemory	smExpandedKey(sizeof(ExpandedCASTKey));
	SecureMemory	smSecureBuf(kMaxPassphraseLength);
	SecureMemory	*pSmCASTKey;

	allocedDataBuf = gotPrivContext = FALSE;

	pgpAssert(PGPKeyRefIsValid(pubKey));
	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertAddrValid(encryptedData, VoidAlign);
	pgpAssertAddrValid(checkBytes, checkBytes);
	pgpAssertAddrValid(decryptedKey, CASTKey);

	// Did we get our locked memory?
	derr = smExpandedKey.mInitErr;

	if (derr.IsntError())
	{
		derr = smSecureBuf.mInitErr;
	}

	if (derr.IsntError())
	{
		// Get locked memory pointer.
		expandedKey	= (ExpandedCASTKey *) smExpandedKey.GetPtr();
		secureBuf	= (PGPUInt8 *) smSecureBuf.GetPtr();

		// Get passphrase string.
		passphrase->GetString((LPSTR) secureBuf, kMaxStringSize);

		// Get key context.
		PGPContextRef context = PGPGetKeyContext(pubKey);

		// Initialize private context.
		derr = PGPNewPrivateKeyContext(pubKey, 
			kPGPPublicKeyMessageFormat_PGP, &privContext, 
			PGPOPassphrase(context, (LPCSTR) secureBuf), 
			PGPOLastOption(context));

		gotPrivContext = derr.IsntError();
	}

	// Get operation sizes.
	if (derr.IsntError())
	{		
		derr = PGPGetPrivateKeyOperationSizes(privContext, 
			&maxDecryptedBufferSize, &maxEncryptedBufferSize, 
			&maxSignatureSize);
	}

	// Get key properties.
	if (derr.IsntError())
	{
		pgpClearMemory(fingerPrint, sizeof(fingerPrint));			
			
		derr = PGPGetKeyPropertyBuffer(pubKey, kPGPKeyPropFingerprint,
			sizeof(fingerPrint), fingerPrint, &fingerPrintSize);
	}

	// Get space for buffer.
	if (derr.IsntError())
	{
		try
		{
			pSmCASTKey = new SecureMemory(maxDecryptedBufferSize);
			data = (PGPUInt8 *) pSmCASTKey->GetPtr();
		}
		catch (CMemoryException *ex)
		{
			derr = DualErr(kPGDMinorError_OutOfMemory);
			ex->Delete();
		}

		allocedDataBuf = derr.IsntError();
	}

	// Decrypt the data.
	if (derr.IsntError())
	{
		derr = PGPPrivateKeyDecrypt(privContext, encryptedData,
			encryptedDataSize, data, &decryptedDataSize);
	}

	// Check the result.
	if (derr.IsntError())
	{
		if (decryptedDataSize == sizeof(CASTKey))
		{
			pgpCopyMemory(data, decryptedKey, sizeof(CASTKey));

			// Decrypt the check bytes. We encrypted the first eight bytes of
			// the fingerprint.

			CAST5schedule(expandedKey->keyDWords, decryptedKey->keyBytes);

			CAST5decrypt(&checkBytes->theBytes[0], 
				&decryptedCheckBytes.theBytes[0], expandedKey->keyDWords);

			if (!pgpMemoryEqual(&decryptedCheckBytes, fingerPrint, 
				sizeof(decryptedCheckBytes)))
			{
				// Should never get here.
				derr = DualErr(kPGDMinorError_InvalidParameter);
			}						
		}
		else
		{
			derr = DualErr(kPGDMinorError_InvalidParameter);
		}
	}

	if (gotPrivContext)
		PGPFreePrivateKeyContext(privContext);

	if (allocedDataBuf)
		delete pSmCASTKey;

	return derr;
}

// EncryptPublicKey encrypts data using a public/private key pair.

DualErr 
EncryptPublicKey(
	const CASTKey	*decryptedKey,
	PGPKeyRef		pubKey,
	void			**encryptedData,
	PGPUInt32		*encryptedDataSize,
	CheckBytes		*checkBytes)
{
	DualErr					derr;
	ExpandedCASTKey			*expandedKey;
	PGPBoolean				allocedDataBuf, gotPubContext;
	PGPByte					*data, fingerPrint[128];
	PGPPublicKeyContextRef	pubContext;
	PGPSize					dataSize, fingerPrintSize, maxDecryptedBufferSize;
	PGPSize					maxEncryptedBufferSize, maxSignatureSize;

	SecureMemory	smExpandedKey(sizeof(ExpandedCASTKey));

	allocedDataBuf = gotPubContext = FALSE;

	pgpAssert(PGPKeyRefIsValid(pubKey));
	pgpAssertAddrValid(encryptedData, VoidAlign);
	pgpAssertAddrValid(checkBytes, checkBytes);
	pgpAssertAddrValid(decryptedKey, CASTKey);

	// Did we get our locked memory?
	derr = smExpandedKey.mInitErr;

	if (derr.IsntError())
	{
		// Get locked memory pointer.
		expandedKey = (ExpandedCASTKey *) smExpandedKey.GetPtr();

		// Initialize private GetGlobalPGPContext().
		derr = PGPNewPublicKeyContext(pubKey, 
			kPGPPublicKeyMessageFormat_PGP, &pubContext);

		gotPubContext = derr.IsntError();
	}

	// Get operation sizes.
	if (derr.IsntError())
	{		
		derr = PGPGetPublicKeyOperationSizes(pubContext, 
			&maxDecryptedBufferSize, &maxEncryptedBufferSize, 
			&maxSignatureSize);
	}

	// Get key properties.
	if (derr.IsntError())
	{
		pgpClearMemory(fingerPrint, sizeof(fingerPrint));			
			
		derr = PGPGetKeyPropertyBuffer(pubKey, kPGPKeyPropFingerprint,
			sizeof(fingerPrint), fingerPrint, &fingerPrintSize);
	}

	// Get space for buffer.
	if (derr.IsntError())
	{
		try
		{
			data = new PGPByte[maxEncryptedBufferSize];
		}
		catch (CMemoryException *ex)
		{
			derr = DualErr(kPGDMinorError_OutOfMemory);
			ex->Delete();
		}

		allocedDataBuf = derr.IsntError();
	}

	if (derr.IsntError())
	{
		// Compute the check bytes.
		CAST5schedule(expandedKey->keyDWords, decryptedKey->keyBytes);

		CAST5encrypt(fingerPrint, &checkBytes->theBytes[0], 
			expandedKey->keyDWords);

		// Perform the encryption.
		derr = PGPPublicKeyEncrypt(pubContext, decryptedKey, 
			sizeof(CASTKey), data, &dataSize);
	}

	if (derr.IsntError())
	{
		(* encryptedData) 		= data;
		(* encryptedDataSize)	= dataSize;
	}

	if (gotPubContext)
		PGPFreePublicKeyContext(pubContext);

	if (derr.IsError())
	{
		if (allocedDataBuf)
			delete[] data;
	}

	return derr;
}

// GetDecryptedKeyUsingPublicKey decrypts an encrypted session key using info
// from a public key header.

DualErr 
GetDecryptedKeyUsingPublicKey(
	SecureString			*passphrase, 
	PGPdiskPublicKeyHeader	*pubKeyHdr, 
	CASTKey					*decryptedKey)
{
	DualErr			derr;
	PGPBoolean		gotAllKeys	= FALSE;
	PGPByte			*encryptedKey, *exportedKeyID;
	PGPKeyID		keyID;
	PGPKeyRef		pubKey;
	PGPKeySetRef	allKeys;

	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertAddrValid(pubKeyHdr, PGPdiskPublicKeyHeader);
	pgpAssertAddrValid(decryptedKey, CASTKey);

	// Load SDK prefs.
	derr = PGPsdkLoadDefaultPrefs(GetGlobalPGPContext());

	// Load default key ring.
	if (derr.IsntError())
	{
		derr = PGPOpenDefaultKeyRings(GetGlobalPGPContext(), 0, &allKeys);
		gotAllKeys = derr.IsntError();
	}

	// Import key ID.
	if (derr.IsntError())
	{
		exportedKeyID = (PGPByte *) pubKeyHdr + pubKeyHdr->keyIDOffset;
		derr = PGPImportKeyID(exportedKeyID, &keyID);
	}

	// Get actual key.
	if (derr.IsntError())
	{
		derr = PGPGetKeyByKeyID(allKeys, &keyID, pubKeyHdr->algorithm, 
			&pubKey);
	}

	// Decrypt the encrypted session key.
	if (derr.IsntError())
	{
		encryptedKey = (PGPByte *) pubKeyHdr + pubKeyHdr->encryptedKeyOffset;

		derr = DecryptPublicKey(pubKey, passphrase, encryptedKey, 
			pubKeyHdr->encryptedKeySize, &pubKeyHdr->checkBytes, 
			decryptedKey);
	}

	if (gotAllKeys)
		PGPFreeKeySet(allKeys);

	return derr;
}

// FindPublicPGPdiskKeyHeader finds a PGPdisk public key header whose private
// key is unlocked by the specified passphrase.

DualErr 
FindPublicPGPdiskKeyHeader(
	LPCSTR					path, 
	SecureString			*passphrase, 
	PGPdiskPublicKeyHeader	**pubKeyHdr)
{
	DualErr				derr;
	PGPBoolean			anyKeysNotInRing, foundKey, gotAllKeys;
	PGPBoolean			gotDiskKeys;
	PGPKeyRef			curKey;
	PGPKeySetRef		allKeys;
	PGPRecipientSpec	*diskRecipKeys;
	PGPUInt8			*secureBuf;
	PGPUInt32			i, numRecipKeys;

	SecureMemory	smSecureBuf(kMaxPassphraseLength);

	anyKeysNotInRing = foundKey = gotAllKeys = gotDiskKeys = FALSE;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertAddrValid(pubKeyHdr, PGPdiskPublicKeyHeader *);

	// Did we get our locked memory?
	derr = smSecureBuf.mInitErr;

	if (derr.IsntError())
	{
		// Get locked memory pointer.
		secureBuf = (PGPUInt8 *) smSecureBuf.GetPtr();

		// Get passphrase string.
		passphrase->GetString((LPSTR) secureBuf, kMaxStringSize);

		// Load SDK prefs.
		derr = PGPsdkLoadDefaultPrefs(GetGlobalPGPContext());
	}

	// Load default key ring.
	if (derr.IsntError())
	{
		derr = PGPOpenDefaultKeyRings(GetGlobalPGPContext(), 0, &allKeys);
		gotAllKeys = derr.IsntError();
	}

	// Get list of keys from PGPdisk.
	if (derr.IsntError())
	{
		derr = GetPGPdiskRecipKeys(path, allKeys, &diskRecipKeys, 
			&numRecipKeys);

		gotDiskKeys = derr.IsntError();
	}

	// Find the key that corresponds to the passphrase.
	if (derr.IsntError())
	{
		// For every recipient spec in the list...
		for (i = 0; i < numRecipKeys; i++)
		{
			// ... if it has an associated key ref...
			if (diskRecipKeys[i].type == kPGPRecipientSpecType_Key)
			{
				PGPBoolean canDecrypt = FALSE;

				curKey = diskRecipKeys[i].u.key;
				PGPGetKeyBoolean(curKey, kPGPKeyPropCanDecrypt, &canDecrypt);

				//... and if the given passphrase unlocks it, we're done.
				if (canDecrypt && PGPPassphraseIsValid(curKey,
					PGPOPassphrase(GetGlobalPGPContext(), (LPCSTR) secureBuf), 
					PGPOLastOption(GetGlobalPGPContext())))
				{
					foundKey = TRUE;
					break;
				}
			}
			else
			{
				anyKeysNotInRing = TRUE;
			}
		}
	}

	// Get header associated with the key if one was found.
	if (derr.IsntError())
	{
		if (foundKey)
		{
			derr = GetHeaderForPublicKey(path, curKey, pubKeyHdr);
		}
		else
		{
			if (anyKeysNotInRing)
				derr = DualErr(kPGDMinorError_IncPassAndKeyNotInRing);
			else
				derr = DualErr(kPGDMinorError_IncorrectPassphrase);
		}
	}

	if (gotDiskKeys)
		FreeRecipientSpecList(diskRecipKeys, numRecipKeys);

	if (gotAllKeys)
		PGPFreeKeySet(allKeys);

	return derr;
}

// CheckIfLockedKeysOnKeyring checks if all the PGPdisk's locked public keys
// are on the local keyring.

DualErr 
CheckIfLockedKeysOnKeyring(LPCSTR path, PGPBoolean *pLockedKeysOnKeyring)
{
	DualErr					derr;
	File					diskFile;
	PGPBoolean				gotAllKeys, gotItemList;
	PGPBoolean				lockedKeysOnKeyring;
	PGPByte					*exportedKeyID;
	PGPdiskFileHeaderItem	*curItem, *itemList;
	PGPKeyID				keyID;
	PGPKeyRef				pubKey;
	PGPKeySetRef			allKeys;

	gotAllKeys = gotItemList = FALSE;
	lockedKeysOnKeyring = TRUE;

	pgpAssertStrValid(path);
	pgpAssertAddrValid(pLockedKeysOnKeyring, PGPBoolean);

	// Open the PGPdisk.
	if (derr.IsntError())
	{
		derr = diskFile.Open(path, kOF_MustExist | kOF_ReadOnly);
	}
	
	// Get list of headers.
	if (derr.IsntError())
	{
		derr = GetHeaderItemList(&diskFile, &itemList);
		gotItemList = derr.IsntError();
	}

	// Search for locked keys.
	if (derr.IsntError())
	{
		curItem = itemList->next;

		// Open default key rings.
		derr = PGPOpenDefaultKeyRings(GetGlobalPGPContext(), 0, &allKeys);
		gotAllKeys = derr.IsntError();
		
		while (derr.IsntError() && IsntNull(curItem))
		{
			if (IsPublicKeyHeader(curItem->hdr))
			{
				PGPdiskPublicKeyHeader *pubKeyHdr;

				pubKeyHdr = (PGPdiskPublicKeyHeader *) curItem->hdr;

				// Is it locked?
				if (pubKeyHdr->locked)
				{
					// Import key ID.
					exportedKeyID = (PGPByte *) pubKeyHdr + 
						pubKeyHdr->keyIDOffset;

					derr = PGPImportKeyID(exportedKeyID, &keyID);

					// Get actual key.
					if (derr.IsntError())
					{
						derr = PGPGetKeyByKeyID(allKeys, &keyID, 
							pubKeyHdr->algorithm, &pubKey);

						// If failed, then bail.
						if (derr.IsError())
						{
							derr = DualErr::NoError;
							lockedKeysOnKeyring = FALSE;
							break;
						}
					}
				}
			}

			curItem = curItem->next;
		}
	}

	(* pLockedKeysOnKeyring) = lockedKeysOnKeyring;

	if (gotAllKeys)
		PGPFreeKeySet(allKeys);

	if (gotItemList)
		FreeHeaderItemList(itemList);

	if (diskFile.Opened())
		diskFile.Close();

	return derr;
}
