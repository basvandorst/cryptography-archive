//////////////////////////////////////////////////////////////////////////////
// CipherUtils.cpp
//
// Various cipher utility functions.
//////////////////////////////////////////////////////////////////////////////

// $Id: CipherUtils.cpp,v 1.1.2.15 1998/07/20 08:20:33 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#if defined(PGPDISK_MFC)

#include "StdAfx.h"
#include <new.h>

#elif defined(PGPDISK_95DRIVER)

#include <vtoolscp.h>
#include "PlacementNew.h"

#include LOCKED_CODE_SEGMENT
#include LOCKED_DATA_SEGMENT

#elif defined(PGPDISK_NTDRIVER)

#include <vdw.h>
#include <stdio.h>
#include <stdlib.h>

#else
#error Define PGPDISK_MFC, PGPDISK_95DRIVER, or PGPDISK_NTDRIVER.
#endif	// PGPDISK_MFC

#include "Required.h"
#include "PGPdiskFileFormat.h"
#include "SecureMemory.h"

#include "Cast5.h"
#include "CipherUtils.h"
#include "SHA.h"


//////////////////////////
// Ciper utility functions
//////////////////////////

// ContinueCRC32 generates a CRC.

CRC32
ComputeCRC32(const PGPUInt32 *theDWords, PGPUInt32 nDWords)
{
	return (ContinueCRC32(theDWords, nDWords, 0));
}

// ContinueCRC32 continues a CRC.
	
CRC32 
ContinueCRC32(
	const PGPUInt32 *theDWords, 
	PGPUInt32		nDWords, 
	CRC32			startCRC)
{
	PGPUInt32		k4C11DB7	= 0x04C11DB7;
	CRC32	 		crc			= startCRC;
	PGPUInt32 const	*curDWord 	= theDWords;
	PGPUInt32 const	*lastDWord	= theDWords + (nDWords - 1);

	while (curDWord <= lastDWord )
	{
		PGPUInt8		bytes[4];
		const PGPUInt8	*curByte;
		PGPInt32		remaining;
		PGPUInt32		theDWord;
		
		// We want an endian-independent CRC - place the bytes in big-endian
		// order.

		theDWord = (* curDWord++);

		bytes[0]	= (PGPUInt8) (theDWord >> 24) & 0xFF;
		bytes[1]	= (PGPUInt8) (theDWord >> 16) & 0xFF;
		bytes[2]	= (PGPUInt8) (theDWord >> 8) & 0xFF;
		bytes[3]	= (PGPUInt8) theDWord & 0xFF;
		
		remaining = 4;
		curByte = bytes;

		while (remaining-- != 0)
		{
			PGPInt32 check;

			crc ^= ((PGPInt32 ) (* curByte)) << 8;
			++curByte;

			check = crc; crc <<= 1; if (check < 0) crc ^= k4C11DB7;
			check = crc; crc <<= 1; if (check < 0) crc ^= k4C11DB7;
			check = crc; crc <<= 1; if (check < 0) crc ^= k4C11DB7;
			check = crc; crc <<= 1; if (check < 0) crc ^= k4C11DB7;
			check = crc; crc <<= 1; if (check < 0) crc ^= k4C11DB7;
			check = crc; crc <<= 1; if (check < 0) crc ^= k4C11DB7;
			check = crc; crc <<= 1; if (check < 0) crc ^= k4C11DB7;
			check = crc; crc <<= 1; if (check < 0) crc ^= k4C11DB7;
		}
	}

	return crc;
}

// HashBuf hashes the given buffer in locked memory and stores the result in
// the key parameter.

DualErr
HashBuf(const PGPUInt8 *buf, PGPUInt16 length, EncryptedCASTKey *key)
{
	DualErr			derr;
	SHA				*hash	= NULL;
	SHA::Digest		hashResult;
	SecureMemory	smHash(sizeof(SHA));

	pgpAssertAddrValid(buf, PGPUInt8);
	pgpAssertAddrValid(key, EncryptedCASTKey);

	// Did we get our memory?
	derr = smHash.mInitErr;

	if (derr.IsntError())
	{
		hash = new (smHash.GetPtr()) SHA;		// <new.h>

		hash->Update(buf, length);
		hash->Final(&hashResult);

		pgpCopyMemory(hashResult.bytes, key->keyBytes, sizeof(key->keyBytes));
	}

	if (hash)
	{
		hash->~SHA();
	}

	return derr;
}

// SaltPassphrase hashes a passphrase with the given salt bytes in a locked
// memory buffer.

DualErr
SaltPassphrase(
	const EncryptedCASTKey	*keyin,
	EncryptedCASTKey		*keyout,
	const PassphraseSalt	*salt,
	PGPUInt16				*hashReps)
{
	DualErr			derr;
	PGPUInt8		j;
	PGPUInt16		i, k;
	SHA				*hash	= NULL;
	SHA::Digest		hashResult;
	SecureMemory	smHash(sizeof(SHA));

	pgpAssertAddrValid(keyin, EncryptedCASTKey);
	pgpAssertAddrValid(keyout, EncryptedCASTKey);
	pgpAssertAddrValid(salt, PassphraseSalt);
	pgpAssertAddrValid(hashReps, PGPUInt16);

	// Did we get our locked memory?
	derr = smHash.mInitErr;

	if (derr.IsntError())
	{
		hash = new (smHash.GetPtr()) SHA;		// placement new
		hash->Update(salt->saltBytes, sizeof(salt->saltBytes));

		if ((* hashReps) != 0)
		{
			// Hash the passphrase and a rotating byte counter the specified
			// number of times into the hash field with the 8 bytes of salt
			// from above in order to form the session key to decrypt the
			// master key.

			k = (* hashReps);

			for (i=0, j=0; i<k; i++, j++)
			{
				hash->Update(keyin->keyBytes, sizeof(keyin->keyBytes));
				hash->Update(&j, 1);
			}
		}
		else
		{
			PGPUInt64 endTicks;

			// This is a new disk or we are changing the passphrase.  We hash
			// the passphrase in with a rotating counter byte an arbitrary
			// number of times based on the processing power of the computer
			// we're running on up to a maximum of 16000.

			endTicks = PGPdiskGetTicks() + 500;	// 500 ms

			for (i=0, j=0; (PGPdiskGetTicks() < endTicks) && (i < 16000); 
				i++, j++)
			{
				hash->Update(keyin->keyBytes, sizeof(keyin->keyBytes));
				hash->Update(&j, 1);
			}

			(* hashReps) = i;
		}
	}

	if (derr.IsntError())
	{
		hash->Final(&hashResult);

		pgpCopyMemory(hashResult.bytes, keyout->keyBytes, 
			sizeof(keyout->keyBytes));
	}

	if (hash)
	{
		hash->~SHA();
	}

	return derr;
}

// HashSaltAndSchedule hashes, salts, and expands the given passphrase.

DualErr
HashSaltAndSchedule(
	const SecureString		*passphrase, 
	ExpandedCASTKey			*expandedKey, 
	const PassphraseSalt	*salt, 
	PGPUInt16				*hashReps, 
	EncryptedCASTKey		*outKey)
{
	DualErr				derr;
	EncryptedCASTKey	*key;
	PGPUInt8			*secureBuf;

	SecureMemory	smKey(sizeof(EncryptedCASTKey));
	SecureMemory	smSecureBuf(kMaxPassphraseLength);

	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertAddrValid(expandedKey, ExpandedCASTKey);
	pgpAssertAddrValid(salt, PassphraseSalt);
	pgpAssertAddrValid(hashReps, PGPUInt16);

	// Did we get our locked memory?
	derr = smKey.mInitErr;

	if (derr.IsntError())
	{
		derr = smSecureBuf.mInitErr;
	}

	// Set up our pointers.
	if (derr.IsntError())
	{
		key			= (EncryptedCASTKey *) smKey.GetPtr();
		secureBuf	= (PGPUInt8 *) smSecureBuf.GetPtr();

		// Hash the passphrase.
		passphrase->GetString((LPSTR) secureBuf, kMaxStringSize);
		derr = HashBuf(secureBuf, (PGPUInt16) passphrase->GetLength(), key);
	}

	// Salt the passphrase.
	if (derr.IsntError())
	{
		derr = SaltPassphrase(key, key, salt, hashReps);
	}

	if (derr.IsntError())
	{
		// Expand the key.
		CAST5schedule(expandedKey->keyDWords, key->keyBytes);

		// Feed the caller the encrypted key if he provided a pointer.
		if (IsntNull(outKey))
		{
			(* outKey) = (* key);
		}
	}

	return derr;
}

// DecryptPassphraseKey decrypts a header key using a passphrase.

DualErr 
DecryptPassphraseKey(
	const PassphraseKey		*passKey,
	const PassphraseSalt 	*salt,
	SecureString			*passphrase,
	CASTKey					*decryptedKey)
{
	CheckBytes			checkBytes;
	DualErr				derr;
	EncryptedCASTKey	*key;
	ExpandedCASTKey		*expandedKey;
	PGPUInt16			hashReps;

	SecureMemory	smEncryptionKey(sizeof(CASTKey));
	SecureMemory	smExpandedKey(sizeof(ExpandedCASTKey));

	pgpAssertAddrValid(passKey, PassphraseKey);
	pgpAssertAddrValid(salt, PassphraseSalt);
	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertAddrValid(decryptedKey, CASTKey);

	// Did we get our locked memory?
	derr = smEncryptionKey.mInitErr;	

	if (derr.IsntError())
	{
		derr = smExpandedKey.mInitErr;
	}

	if (derr.IsntError())
	{
		// Initialize pointer to our locked memory.
		key			= (EncryptedCASTKey *) smEncryptionKey.GetPtr();
		expandedKey = (ExpandedCASTKey *) smExpandedKey.GetPtr();

		hashReps = passKey->hashReps;

		// Hash, salt, and expand the passphrase.
		derr = HashSaltAndSchedule(passphrase, expandedKey, salt, &hashReps, 
			key);
	}

	if (derr.IsntError())
	{
		(* decryptedKey) = passKey->encryptedKey;
			
		// Decrypt the encrypted key.
		CAST5decrypt(&decryptedKey->keyBytes[0], 
			&decryptedKey->keyBytes[0], expandedKey->keyDWords);

		CAST5decrypt(&decryptedKey->keyBytes[8], 
			&decryptedKey->keyBytes[8], expandedKey->keyDWords);

		// Get the ckeck bytes
		checkBytes = passKey->checkBytes;
	
		// Decrypt the check bytes using the expanded key
		CAST5decrypt(checkBytes.theBytes, checkBytes.theBytes, 
			expandedKey->keyDWords);

		// Compare the check bytes against the encrypted key for equality.
		if (!pgpMemoryEqual(&checkBytes.theBytes[0], &key->keyBytes[0], 
			sizeof(checkBytes)))
		{
			derr = DualErr(kPGDMinorError_IncorrectPassphrase);
		}
	}
	
	return derr;
}

// EncryptPassphraseKey encrypts a header key using a passphrase.

DualErr 
EncryptPassphraseKey(
	CASTKey					*decryptedKey,
	const PassphraseSalt 	*salt,
	SecureString			*passphrase,
	PassphraseKey			*passKey)
{
	DualErr				derr;
	EncryptedCASTKey	*key;
	ExpandedCASTKey		*expandedKey;

	SecureMemory	smEncryptionKey(sizeof(CASTKey));
	SecureMemory	smExpandedKey(sizeof(ExpandedCASTKey));

	pgpAssertAddrValid(passKey, PassphraseKey);
	pgpAssertAddrValid(salt, PassphraseSalt);
	pgpAssertAddrValid(passphrase, SecureString);
	pgpAssertAddrValid(decryptedKey, CASTKey);

	// Did we get our locked memory?
	derr = smEncryptionKey.mInitErr;	

	if (derr.IsntError())
	{
		derr = smExpandedKey.mInitErr;
	}

	if (derr.IsntError())
	{
		// Initialize pointer to our locked memory.
		key			= (EncryptedCASTKey *) smEncryptionKey.GetPtr();
		expandedKey = (ExpandedCASTKey *) smExpandedKey.GetPtr();

		passKey->hashReps = 0;

		// Hash, salt, and expand the passphrase.
		derr = HashSaltAndSchedule(passphrase, expandedKey, salt, 
			&passKey->hashReps, key);
	}

	if (derr.IsntError())
	{
		passKey->encryptedKey = (* decryptedKey);

		// Encrypt the key with the new salted passphrase.
		CAST5encrypt(&passKey->encryptedKey.keyBytes[0], 
			&passKey->encryptedKey.keyBytes[0], expandedKey->keyDWords);

		CAST5encrypt(&passKey->encryptedKey.keyBytes[8], 
			&passKey->encryptedKey.keyBytes[8], expandedKey->keyDWords);

		// Encrypt the new key itself for checkbytes purposes.
		CAST5encrypt(key->keyBytes, key->keyBytes, expandedKey->keyDWords);

		for (PGPUInt16 i=0; i < sizeof(passKey->checkBytes); i++)
		{
			passKey->checkBytes.theBytes[i] = key->keyBytes[i];
		}
	}

	return derr;
}
