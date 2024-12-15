//////////////////////////////////////////////////////////////////////////////
// CipherUtils.h
//
// Declarations for CipherUtils.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CipherUtils.h,v 1.1.2.6 1998/07/06 08:56:37 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CipherUtils_h	// [
#define Included_CipherUtils_h

#include "SecureString.h"

#include "Cast5.h"


////////
// Types
////////

typedef PGPUInt32 CRC32;
typedef struct PassphraseKey PassphraseKey;


/////////////////////
// Exported Functions
/////////////////////

CRC32	ComputeCRC32(const PGPUInt32 *theDWords, PGPUInt32 nDWords);
CRC32	ContinueCRC32(const PGPUInt32 *theDWords, PGPUInt32 nDWords, 
			CRC32 startCRC);

DualErr	HashBuf(const PGPUInt8 *buf, PGPUInt16 length, EncryptedCASTKey *key);

DualErr	SaltPassphrase(const EncryptedCASTKey *keyin, 
			EncryptedCASTKey *keyout, const PassphraseSalt *salt, 
			PGPUInt16 *hashReps);

DualErr	HashSaltAndSchedule(const SecureString *passphrase, 
			ExpandedCASTKey *expandedKey, const PassphraseSalt *salt, 
			PGPUInt16 *hashReps, EncryptedCASTKey *outKey = NULL);

DualErr	DecryptPassphraseKey(const PassphraseKey *passKey, 
			const PassphraseSalt *salt, SecureString *passphrase, 
			CASTKey *decryptedKey);

DualErr	EncryptPassphraseKey(CASTKey *decryptedKey, 
			const PassphraseSalt *salt, SecureString *passphrase, 
			PassphraseKey *passKey);

#endif	// ] Included_CipherUtils_h
