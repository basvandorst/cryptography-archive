//////////////////////////////////////////////////////////////////////////////
// PGPdiskPublicKeyUtils.h
//
// Declarations for PGPdiskPublicKeyUtils.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskPublicKeyUtils.h,v 1.4 1999/02/26 04:10:00 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef	Included_PGPdiskPublicKeyUtils_h	// [
#define	Included_PGPdiskPublicKeyUtils_h

#include "pgpGroups.h"
#include "pgpKeys.h"
#include "pgpUserInterface.h"

#include "FatUtils.h"
#include "PGPdiskFileFormat.h"
#include "PGPdiskLowLevelUtils.h"

#include "Cast5.h"
#include "CipherUtils.h"


////////
// Types
////////

class File;


/////////////////////
// Exported functions
/////////////////////

PGPBoolean	DoesPGPdiskHavePublicKey(LPCSTR path);

DualErr		IsKeyIDMemberOfRecipList(PGPKeyID goalKeyID, 
				PGPRecipientSpec *recipList, PGPUInt32 size, 
				PGPBoolean *isMember);

DualErr		AddRecipientSpecToList(PGPRecipientSpec **recipList, 
				PGPUInt32 size);

void		FreeRecipientSpecList(PGPRecipientSpec *recipList, 
				PGPUInt32 size);

DualErr		GetPGPdiskRecipKeys(LPCSTR path, PGPKeySetRef allKeys, 
				PGPRecipientSpec **pDiskRecipKeys, PGPUInt32 *pNumRecipKeys);

DualErr		UpdatePGPdiskPublicKeys(LPCSTR path, 
				SecureString *masterPassphrase, PGPKeySetRef allKeys, 
				PGPRecipientSpec *newRecipKeys, PGPUInt32 numRecipKeys, 
				PGPBoolean newKeysReadOnly);

DualErr		FindPublicKeyInHeaderList(File *diskFile, PGPKeyRef curKey, 
				PGPdiskFileHeaderItem *itemList, 
				PGPdiskFileHeaderItem **keyItem);

DualErr		GetHeaderForPublicKey(LPCSTR path, PGPKeyRef curKey, 
				PGPdiskPublicKeyHeader **pubKeyHdr);

DualErr		CreatePublicKeyHeader(PGPKeyRef pubKey, 
				const CASTKey *decryptedKey, PGPBoolean locked, 
				PGPBoolean readOnly, PGPdiskPublicKeyHeader **outPubKeyHdr);

PGPBoolean	ArePGPdiskPassesWiped(LPCSTR path);

DualErr		ShowReadOnlyListOfPublicKeys(LPCSTR path);
DualErr		WipeMasterAndAlternatePasses(LPCSTR path);

DualErr		IsTherePGPdiskADK(PGPBoolean *isThereADK);

DualErr		GetADKUserID(PGPKeySetRef allKeys, LPSTR userID, 
				PGPUInt32 sizeUserID);

DualErr		GetADKKey(PGPKeySetRef allKeys, PGPKeyRef *ADKKey);

DualErr		AddADKToPGPdisk(LPCSTR path, SecureString *masterPassphrase);

DualErr		DecryptPublicKey(PGPKeyRef pubKey, SecureString *passphrase, 
				const void *encryptedData, PGPSize encryptedDataSize, 
				const CheckBytes *checkBytes, CASTKey *decryptedKey);

DualErr		EncryptPublicKey(const CASTKey *decryptedKey, PGPKeyRef pubKey, 
				void **encryptedData, PGPUInt32 *encryptedDataSize, 
				CheckBytes *checkBytes);

DualErr		GetDecryptedKeyUsingPublicKey(SecureString *passphrase, 
				PGPdiskPublicKeyHeader *pubKeyHdr, CASTKey *decryptedKey);

DualErr		FindPublicPGPdiskKeyHeader(LPCSTR path, SecureString *passphrase, 
				PGPdiskPublicKeyHeader **pubKeyHdr);

DualErr		CheckIfLockedKeysOnKeyring(LPCSTR path, 
				PGPBoolean *pLockedKeysOnKeyring);

#endif	// ] Included_PGPdiskPublicKeyUtils_h
